#include <stdio.h>
#include <string.h>

#include "constant.h"
#include "disk_emu.h"
#include "block.h"
#include "sfs_api.h"
#include "super_block.h"
#include "inode.h"
#include "free_bitmap.h"
#include "directory.h"
#include "fdt.h"


block_t inode_table[INODE_TABLE_SIZE];
block_t dir_table[DIR_BLOCK_SIZE];
block_t fd_table[DIR_BLOCK_SIZE];

void set_dir_entry_table(inode_t inode, dirent_t* dir_table) {
    block_t block[1];
    for (int index = 0; index < DIR_BLOCK_SIZE; index++) {
        if (inode.pointers[index] < 0) return;
        read_blocks(inode.pointers[index], 1, &block);
        for (int dir_index = 0; dir_index < DIR_PER_BLOCK; dir_index++)
            dir_table[dir_index + index * DIR_PER_BLOCK] = ((dirent_t *) &block)[dir_index];
    }
}

void mksfs(int fresh) {
    if (fresh == 1) {
        /* To setup a new disk */
        /* Initialize a fresh disk */
        init_fresh_disk(DISK_NAME, BLOCK_SIZE, SUPERBLOCK_SIZE + INODE_TABLE_SIZE + 
            DATA_BLOCK_SIZE + DIR_BLOCK_SIZE + FREE_BITMAP_SIZE);
        /* Initialize the super block */
        init_superblock();
        /* Initialize the inode table and the inode cache */
        init_inode_table((inode_t *) &inode_table);
        /* Initialize the directory table and the directory cache */
        init_dir_entry_table((dirent_t *) &dir_table);
        /* Initialize the free bitmap */
        init_fbm();
    } else {
        /* To setup an existing disk */
        /* Initialize the disk */
        init_disk(DISK_NAME, BLOCK_SIZE, SUPERBLOCK_SIZE + INODE_TABLE_SIZE + 
            DATA_BLOCK_SIZE + DIR_BLOCK_SIZE + FREE_BITMAP_SIZE);
        /* Check if the disk has a valid format */
        check_valid_disk();
        /* Copy the inode table to the inode cache */
        set_inode_table((inode_t *) &inode_table);
        /* Copy the directory table to the directory cache */
        set_dir_entry_table(((inode_t *) &inode_table)[0], (dirent_t *) &dir_table);
    }
    /* Initialize the file descriptor table */
    init_fdt((fdt_t *) &fd_table);
}

void create_file(char* name, inode_t* inode_table, int inode_index, dirent_t* dir_table, int dir_index) {
    int db_index = -1;
    block_t block[1];

    for (int index = 0; index < INODE_POINTER_SIZE + 1; index++) {
        if (inode_table[0].pointers[index] > -1) {
            db_index = inode_table[0].pointers[index];
            read_blocks(inode_table[0].pointers[index], 1, &block);
        } else {
            db_index = find_free_block();
            
            inode_table[0].pointers[index] = db_index;
        }

        inode_table[0].size += sizeof(dirent_t);
        write_blocks(SUPERBLOCK_SIZE, 1, inode_table);

        int dir_blk_index = dir_index % (BLOCK_SIZE / sizeof(dirent_t));
        strcpy(((dirent_t *) block)[dir_blk_index].filename, name);
        ((dirent_t *) block)[dir_blk_index].inode = inode_index;

        write_blocks(db_index, 1, &block);
        break;
    }
}

int sfs_fopen(char *name) {
    int fdt_index = -1;
    int inode = -1;
    int size = 0;

    inode = find_inode_with_filename(name, (dirent_t *) &dir_table);

    if (inode > 0) {
        // If the file exists
        size = ((inode_t *) &inode_table)[inode].size;
    } else {
        // If the file does not exist
        // return available inode and initialize inode (size = 0 and link_cnt = 1)
        inode = find_free_inode((inode_t *) &inode_table);
        int dir_index = find_free_entry((dirent_t *) &dir_table);

        create_file(name, (inode_t *) &inode_table, inode, (dirent_t *) &dir_table, dir_index);

        // find available directory entry and set filename with inode index
        init_inode((inode_t *) &inode_table, inode);
        insert_dir_entry((dirent_t *) &dir_table, dir_index, name, inode);
    }

    fdt_index = find_free_fdt_entry((fdt_t *) &fd_table);

    insert_fdt_entry((fdt_t *) &fd_table, fdt_index, inode, size);

    return fdt_index;
}

int sfs_fclose(int fileID) {
    return close_fdt_entry((fdt_t *) &fd_table, fileID);
}

// TODO
int sfs_fwrite(int fileID, const char *buf, int length) {
    block_t temp;

    /* Get File Descriptor Table information */
    int inode = ((fdt_t *) &fd_table)[fileID].inum;
    int size = ((fdt_t *) &fd_table)[fileID].foffset;

    /* Check which inode pointer to find the starting block */
    int pointer_index = size / BLOCK_SIZE;
    /* Get the starting block to write */
    int block_index = ((inode_t *) &inode_table)[inode].pointers[pointer_index];

    if (block_index < 0) {
        /* If the pointer does not have a block assigned */
        block_index = find_free_block();
    } else {
        /* If the pointer has a block */
        read_blocks(block_index, 1, &temp);
    }

    int starting_point = size - (pointer_index * BLOCK_SIZE);
    int buf_done = 0;
    int buf_left = length;

    while (1) {
        int index;
        for (index = 0; (index + starting_point < BLOCK_SIZE && index < buf_left - 1); index++) {
            temp.data[starting_point + index] = buf[index];
        }
        write_blocks(block_index, 1, &temp);

        buf_done = buf_left - (index + 1);
        if (buf_done < 1) break;
        buf_left = buf_left - buf_done;
        starting_point = 0;
        printf("done: %d\n", buf_done);
        printf("left: %d\n", buf_left);
        // ! recount size if it overwrites existing data
        ((inode_t *) &inode_table)[inode].size += buf_done;

        pointer_index++;
        int block_index = ((inode_t *) &inode_table)[inode].pointers[pointer_index];

        if (block_index < 0) {
            /* If the pointer does not have a block assigned */
            block_index = find_free_block();
        } else {
            /* If the pointer has a block */
            read_blocks(block_index, 1, &temp);
        }
    }

    write_blocks(SUPERBLOCK_SIZE, INODE_TABLE_SIZE, &inode_table);

    ((fdt_t *) &fd_table)[fileID].foffset += length;

    return -1;
}

// TODO
int sfs_fread(int fileID, char *buf, int length) {
    block_t temp[1];

    int inode = ((fdt_t *) &fd_table)[fileID].inum;
    int size = ((fdt_t *) &fd_table)[fileID].foffset;

    int pointer_index = size / BLOCK_SIZE;
    int block_index = ((inode_t *) &inode_table)[inode].pointers[pointer_index];
    if (block_index < 0) {
        block_index = find_free_block();
    } else {
        read_blocks(block_index, 1, &temp);
    }
    memcpy(buf, &temp, length);

    ((fdt_t *) &fd_table)[fileID].foffset += length;

    return -1;
}

int sfs_fseek(int fileID, int loc) {
    return seek_fdt_entry((fdt_t *) &fd_table, fileID, loc);
}

// ! REDO
int sfs_remove(char *file) {
    int inode_index = find_inode_with_filename(file, (dirent_t *) &dir_table);
    int dir_index = remove_dir_entry_mem((dirent_t *) &dir_table, file);
    if (dir_index < 0) return -1;

    int dir_block_index = ((inode_t *) &inode_table)[0].pointers[dir_index / DIR_PER_BLOCK];

    remove_dir_entry_disk((dirent_t *) &dir_table, dir_block_index, dir_index);
    remove_entry_inode((inode_t *) &inode_table);

    remove_inode((inode_t *) &inode_table, inode_index);

    return 0;
}