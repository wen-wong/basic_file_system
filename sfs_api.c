/**
 * README: Simple File System API
 * ------------------------------
 * The following implementation provides a set of functions to create, write, read and remove files
 * into a disk. The API has multiple files.
 * 
*/
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "constant.h"
#include "disk_emu.h"
#include "block.h"
#include "sfs_api.h"
#include "super_block.h"
#include "inode.h"
#include "free_bitmap.h"
#include "directory.h"
#include "fdt.h"

/* In-Memory Data */
block_t inode_table[INODE_TABLE_SIZE]; /* Inode Table */
block_t dir_table[DIR_BLOCK_SIZE];     /* Directory Table */
block_t fd_table[DIR_BLOCK_SIZE];      /* File Descriptor Table */

int current_dir = 1;

/* Helper Functions */
void set_dir_entry_table(inode_t inode, dirent_t* dir_table);
void create_file(char* name, inode_t* inode_table, int inode_index, dirent_t* dir_table, int dir_index);
void remove_inode(inode_t* inode_table, int index);

void mksfs(int fresh) {
    current_dir = 1;
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

int sfs_getnextfilename(char* fname) {
    int dir_length = 1;
    for (int i = 1; i < DIR_ENTRY_SIZE; i++) {
        if (((dirent_t *) &dir_table)[i].inode < 0) break;
        dir_length++;
    }
    if (current_dir >= dir_length) return 0;
    strcpy(fname, ((dirent_t *) &dir_table)[current_dir].filename);
    current_dir++;
    return 1;
}

int sfs_getfilesize(const char* path) {
    int inode = find_inode_with_path(path, (dirent_t *) dir_table);
    return ((inode_t *) &inode_table)[inode].size;
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

int sfs_fwrite(int fileID, const char *buf, int length) {
    if (fileID < 0) return -1;
    block_t temp;

    /* Get File Descriptor Table information */
    int inode = ((fdt_t *) &fd_table)[fileID].inum;
    int size = ((fdt_t *) &fd_table)[fileID].foffset;

    if (inode < 0) return -1;

    /* Check which inode pointer to find the starting block */
    int pointer_index = size / BLOCK_SIZE;
    /* Get the starting block to write */
    int block_index = ((inode_t *) &inode_table)[inode].pointers[pointer_index];

    if (block_index < 0) {
        /* If the pointer does not have a block assigned */
        block_index = find_free_block();
        ((inode_t *) &inode_table)[inode].pointers[pointer_index] = block_index;
    } else {
        /* If the pointer has a block */
        read_blocks(block_index, 1, &temp);
    }

    int current_length = length;

    while (true) {
        if (length > BLOCK_SIZE - size) {
            current_length -= BLOCK_SIZE - size;

            strcpy(((char *) &temp) + size, buf);
            write_blocks(block_index, 1, &temp);

            if (current_length <= 0) break;
            size = 0;

            pointer_index++;
            int block_index = ((inode_t *) &inode_table)[inode].pointers[pointer_index];

            if (block_index < 0) {
                /* If the pointer does not have a block assigned */
                block_index = find_free_block();
                ((inode_t *) &inode_table)[inode].pointers[pointer_index] = block_index;
            } else {
                /* If the pointer has a block */
                read_blocks(block_index, 1, &temp);
            }
        } else {
            strcpy(((char *) &temp) + size, buf);
            write_blocks(block_index, 1, &temp);
            break;
        }
    }

    ((inode_t *) &inode_table)[inode].size = size + length;
    write_blocks(SUPERBLOCK_SIZE, INODE_TABLE_SIZE, &inode_table);

    ((fdt_t *) &fd_table)[fileID].foffset += length;

    return length;
}

int sfs_fread(int fileID, char *buf, int length) {
    if (fileID < 0) return -1;
    block_t temp;

    /* Get File Descriptor Table information */
    int inode = ((fdt_t *) &fd_table)[fileID].inum;
    int size = ((fdt_t *) &fd_table)[fileID].foffset;

    if (inode < 0) return -1;

    /* Check which inode pointer to find the starting block */
    int pointer_index = size / BLOCK_SIZE;
    /* Get the starting block to write */
    int block_index = ((inode_t *) &inode_table)[inode].pointers[pointer_index];

    if (block_index < 0) {
        return -1;
    } else {
        /* If the pointer has a block */
        read_blocks(block_index, 1, &temp);
    }

    int current_length = length;

    while (true) {
        if (length > BLOCK_SIZE - size) {
            current_length -= BLOCK_SIZE - size;

            strcpy(buf, ((char *) &temp) + size);

            if (current_length <= 0) break;
            size = 0;

            pointer_index++;
            int block_index = ((inode_t *) &inode_table)[inode].pointers[pointer_index];

            if (block_index < 0) {
                return -1;
            } else {
                /* If the pointer has a block */
                read_blocks(block_index, 1, &temp);
            }
        } else {
            strcpy(buf, ((char *) &temp) + size);
            break;
        }
    }
    ((inode_t *) &inode_table)[inode].size = length;

    ((fdt_t *) &fd_table)[fileID].foffset += length;

    return length;
}

int sfs_fseek(int fileID, int loc) {
    return seek_fdt_entry((fdt_t *) &fd_table, fileID, loc);
}

int sfs_remove(char *file) {
    /* Get INode of the file from the directory table in memory */
    int inode_index = find_inode_with_filename(file, (dirent_t *) &dir_table);
    /* Get directory entry index of the file from the directory table in memory */
    int dir_index = remove_dir_entry_mem((dirent_t *) &dir_table, file);
    if (dir_index < 0) return -1;

    int dir_block_index = ((inode_t *) &inode_table)[0].pointers[dir_index / DIR_PER_BLOCK];

    remove_dir_entry_disk(dir_block_index, dir_index);
    remove_entry_inode((inode_t *) &inode_table);

    remove_inode((inode_t *) &inode_table, inode_index);

    return 0;
}

void set_dir_entry_table(inode_t inode, dirent_t* dir_table) {
    block_t block[1];
    for (int index = 0; index < DIR_BLOCK_SIZE; index++) {
        if (inode.pointers[index] < 0) return;
        read_blocks(inode.pointers[index], 1, &block);
        for (int dir_index = 0; dir_index < DIR_PER_BLOCK; dir_index++)
            dir_table[dir_index + index * DIR_PER_BLOCK] = ((dirent_t *) &block)[dir_index];
    }
}

void create_file(char* name, inode_t* inode_table, int inode_index, dirent_t* dir_table, int dir_index) {
    int disk_block_index = -1;
    block_t block;

    for (int index = 0; index < INODE_POINTER_SIZE + 1; index++) {
        if (inode_table[0].pointers[index] > -1) {
            disk_block_index = inode_table[0].pointers[index];
            read_blocks(inode_table[0].pointers[index], 1, &block);
        } else {
            disk_block_index = find_free_block();
            
            inode_table[0].pointers[index] = disk_block_index;
        }

        inode_table[0].size += sizeof(dirent_t);
        write_blocks(SUPERBLOCK_SIZE, 1, inode_table);

        int dir_blk_index = dir_index % (BLOCK_SIZE / sizeof(dirent_t));
        strcpy(((dirent_t *) &block)[dir_blk_index].filename, name);
        ((dirent_t *) &block)[dir_blk_index].inode = inode_index;

        write_blocks(disk_block_index, 1, &block);
        break;
    }
}

/**
 * remove_inode -- Removes all data to the requested INode In-Memory and on the disk.
 * 
 * inode_table: INode table in memory
 * index: Index of the INode to be reset
*/
void remove_inode(inode_t* inode_table, int index) {
    block_t temp_block;

    inode_table[index].mode = 0;
    inode_table[index].link_cnt = 0;
    inode_table[index].size = 0;

    /* Clear all data in each pointer */
    for (int i = 0; i < INODE_POINTER_SIZE; i++) {
        if (inode_table[index].pointers[i] < 0) break;

        int block_index = inode_table[index].pointers[i];
        memset(&temp_block, 0, BLOCK_SIZE);
        write_blocks(block_index, 1, &temp_block);
        reset_free_block(block_index);
        inode_table[index].pointers[i] = -1;
    }

    /* Clear all data in the indirect pointer */
    if (inode_table[index].ind_pointer != -1) {

        block_t ind_block, ind_inner_block;
        read_blocks(inode_table[index].ind_pointer, 1, &ind_block);
        memset(&ind_inner_block, 0, BLOCK_SIZE);
        for (int i = 0; i < BLOCK_SIZE / sizeof(int); i++) {
            int block_id = ((int *) &ind_block)[i];
            if (block_id < 0) break;
            write_blocks(block_id, 1, &ind_inner_block);
            reset_free_block(block_id);
        }
        reset_free_block(inode_table[index].ind_pointer);
        inode_table[index].ind_pointer = -1;
    }

    write_blocks(SUPERBLOCK_SIZE, INODE_TABLE_SIZE, inode_table);
}