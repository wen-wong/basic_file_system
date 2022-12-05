/**
 * Simple File System API
 * ------------------------------
 * The following implementation provides a set of functions to create, write, read and remove files
 * into a disk. The API has multiple files. You may find the structure of the file system and
 * the limitations of the following API in the README.md file.
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

/**
 * mksfs -- Initializes the disk and the disk information in-memory.
 * 
 * fresh: indication if the user would like to create a new disk (1) or use an existing disk (0)
*/
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

/**
 * sfs_getnextfilename -- Gets the name of the next file in the directory table.
 *                        There's also a global counter that keeps track of
 *                        the current index of the directory table.
 *        
 * fname: the buffer where the next filename will be copied to
 * 
 * returns 1 if a new filename has been found and returns 0 when
 * the counter has reached the end of the directory table.
*/
int sfs_getnextfilename(char* fname) {
    int dir_length = 1;

    /* Get the length of the directory table */
    for (int i = 1; i < DIR_ENTRY_SIZE; i++) {
        if (((dirent_t *) &dir_table)[i].inode < 0) break;
        dir_length++;
    }

    /* Check if the current directory index has reached the end of the directoru table */
    if (current_dir >= dir_length) return 0;

    /* Set the filename of the current index to the buffer (fname) */
    strcpy(fname, ((dirent_t *) &dir_table)[current_dir].filename);
    /* Increment the current directory index for the next sfs_getnextfilename */
    current_dir++;

    return 1;
}

/**
 * sfs_getfilesize -- Returns the size of the specific file given a path.
 *                    The size of the specific file is found in the INode.
 * 
 * path: file path
 * 
 * returns the size of the file found in the given INode
*/
int sfs_getfilesize(const char* path) {
    /* Get the INode that corresponds to the file in the path */
    int inode = find_inode_with_path(path, (dirent_t *) dir_table);

    /* Check if the inode has been found */
    if (inode < 0) {
        printf("Invalid Read -- Cannot fin %s", path);
        return -1;
    }
    /* Returns the size of the file in the path */
    return ((inode_t *) &inode_table)[inode].size;
}

/**
 * sfs_fopen -- Opens the given filename or creates a new file with the filename
 *              and returns the index of the file descriptor related to the file.
 * 
 * name: filename to be opened and to be added to the file descriptor
 * 
 * returns the file descriptor index of the newly opened file
*/
int sfs_fopen(char *name) {
    int fdt_index = -1;
    int inode = -1;
    int size = 0;

    /* Get the inode corresponding to the filename from the directory table */
    inode = find_inode_with_filename(name, (dirent_t *) &dir_table);

    if (inode > 0) {
        /* If the file exists in the disk */
        /* Get the size of the file */
        size = ((inode_t *) &inode_table)[inode].size;
    } else {
        /* If the file does not exist in the disk */
        /* Find the first available INode from the INode table */
        inode = find_free_inode((inode_t *) &inode_table);
        /* Find the first available directory entry from the directory table */
        int dir_index = find_free_entry((dirent_t *) &dir_table);
        /* Create the file in the disk */
        create_file(name, (inode_t *) &inode_table, inode, (dirent_t *) &dir_table, dir_index);

        /* Update the INode table */
        init_inode((inode_t *) &inode_table, inode);
        /* Update the directory table */
        insert_dir_entry((dirent_t *) &dir_table, dir_index, name, inode);
    }

    /* Find the first available file descriptor entry */
    fdt_index = find_free_fdt_entry((fdt_t *) &fd_table);
    /* Add file descriptor entry */
    insert_fdt_entry((fdt_t *) &fd_table, fdt_index, inode, size);

    return fdt_index;
}

/**
 * sfs_fclose -- Closes the file in the file descriptor table using its index.
 * 
 * fileID: file descriptor table index
 * 
 * returns -1 or 0 if its a success
*/
int sfs_fclose(int fileID) {
    return close_fdt_entry((fdt_t *) &fd_table, fileID);
}

/**
 * sfs_fwrite -- Writes the given buffer to file.
 * 
 * limitations:
 * - It has only been tested to write on the first block
 * - It does not allocate blocks past the 12 direct pointers
 * - It only writes characters and not individual bytes
 * 
 * fileID: file descriptor index
 * buf: buffer that will be written onto the file
 * length: size of the buffer
*/
int sfs_fwrite(int fileID, const char *buf, int length) {
    if (fileID < 0) return -1;
    block_t temp;

    /* Gets File Descriptor Table information */
    int inode = ((fdt_t *) &fd_table)[fileID].inum;
    int size = ((fdt_t *) &fd_table)[fileID].foffset;

    /* Checks if the file descriptor entry has a file */
    if (inode < 0) return -1;

    /* Checks which inode pointer to find the starting block */
    int pointer_index = size / BLOCK_SIZE;
    /* Gets the starting block to write */
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
            /* If the length of the buffer to write exceeds the block size from the read/write pointer */
            /* Gets how many bytes will be written in the current block */
            current_length -= BLOCK_SIZE - size;

            strcpy(((char *) &temp) + size, buf);
            write_blocks(block_index, 1, &temp);

            /* Checks if we have written the whole buffer */
            if (current_length <= 0) break;

            /* If not, we reset the values and write to the next pointer */
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
            /* If the length of the buffer to write can be done in same block */
            /* Writing from the buffer thats into account the read/write pointer */
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

/**
 * sfs_fread -- Reads the file and copies it to the given buffer.
 * 
 * limitations:
 * - It has only been tested to read on the first block
 * - It only writes characters and not individual bytes
 * 
 * fileID: file descriptor index
 * buf: buffer to be written on with the file's data
 * length: size of the buffer
*/
int sfs_fread(int fileID, char *buf, int length) {
    if (fileID < 0) return -1;
    block_t temp;

    /* Gets File Descriptor Table information */
    int inode = ((fdt_t *) &fd_table)[fileID].inum;
    int size = ((fdt_t *) &fd_table)[fileID].foffset;

    /* Checks if the file descriptor entry has a file */
    if (inode < 0) return -1;

    /* Checks which inode pointer to find the starting block */
    int pointer_index = size / BLOCK_SIZE;
    /* Gets the starting block to write */
    int block_index = ((inode_t *) &inode_table)[inode].pointers[pointer_index];

    if (block_index < 0) {
        /* If the pointer does not have a block assigned */
        return -1;
    } else {
        /* If the pointer has a block */
        read_blocks(block_index, 1, &temp);
    }

    int current_length = length;

    while (true) {
        if (length > BLOCK_SIZE - size) {
            /* If the length of the buffer to read exceeds the block size from the read/write pointer */
            /* Gets how many bytes will be read in the current block */
            current_length -= BLOCK_SIZE - size;

            strcpy(buf, ((char *) &temp) + size);

            /* Checks if we have written the whole buffer */
            if (current_length <= 0) break;

            /* If not, we reset the values and write to the next pointer */
            size = 0;

            pointer_index++;
            int block_index = ((inode_t *) &inode_table)[inode].pointers[pointer_index];

            if (block_index < 0) {
                /* If the pointer does not have a block assigned */
                return -1;
            } else {
                /* If the pointer has a block */
                read_blocks(block_index, 1, &temp);
            }
        } else {
            /* If the length of the buffer to read can be done in same block */
            /* Reading from the file thats into account the read/write pointer */
            strcpy(buf, ((char *) &temp) + size);
            break;
        }
    }
    ((inode_t *) &inode_table)[inode].size = length;

    ((fdt_t *) &fd_table)[fileID].foffset += length;

    return length;
}

/**
 * sfs_fseek -- Changes the read/write pointer of a file descriptor entry.
 * 
 * fileID: file descriptor entry index
 * loc: new location of the read/write pointer
 * 
 * returns -1 or 0 if its a success
*/
int sfs_fseek(int fileID, int loc) {
    return seek_fdt_entry((fdt_t *) &fd_table, fileID, loc);
}

/**
 * sfs_remove -- Removes the file from its allocated data blocks, INode, and directory entry.
 * 
 * file: filename to be removed
 * 
 * returns -1 or 0 if its a success
*/
int sfs_remove(char *file) {
    /* Get INode of the file from the directory table in memory */
    int inode_index = find_inode_with_filename(file, (dirent_t *) &dir_table);
    /* Get directory entry index of the file from the directory table in memory */
    int dir_index = remove_dir_entry_mem((dirent_t *) &dir_table, file);

    /* Checks if directory entry has been found */
    if (dir_index < 0) return -1;

    /* Get the block index of the directory entry */
    int dir_block_index = ((inode_t *) &inode_table)[0].pointers[dir_index / DIR_PER_BLOCK];

    /* Remove the directory entry from the specific block */
    remove_dir_entry_disk(dir_block_index, dir_index);
    /* Decrement the size of the inode since a directory entry has been removed */
    remove_entry_inode((inode_t *) &inode_table);

    /* Reset the INode and remove all data that have been assigned to each respective pointer */
    remove_inode((inode_t *) &inode_table, inode_index);

    return 0;
}

/**
 * set_dire_entry_table -- Sets the directory entries on the disk to the one in-memory.
 * 
 * inode: INode linked to the directory table
 * dir_table: directory table in memory
*/
void set_dir_entry_table(inode_t inode, dirent_t* dir_table) {
    block_t block[1];
    for (int index = 0; index < DIR_BLOCK_SIZE; index++) {
        if (inode.pointers[index] < 0) return;
        read_blocks(inode.pointers[index], 1, &block);
        for (int dir_index = 0; dir_index < DIR_PER_BLOCK; dir_index++)
            dir_table[dir_index + index * DIR_PER_BLOCK] = ((dirent_t *) &block)[dir_index];
    }
}

/**
 * create_file -- Creates a file and places it on the disk.
 * 
 * name: filename to be created
 * inode_table: INode table in memory
 * inode_index: index of the INode entry where the file will be referred to
 * dir_table: directory table in memory
 * dir_index: index of the directory entry where the filename will be referred to
*/
void create_file(char* name, inode_t* inode_table, int inode_index, dirent_t* dir_table, int dir_index) {
    int disk_block_index = -1;
    block_t block;

    /* Initializes the INode related to the file to the first available pointer */
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

        /* Initializes the directory entry to the defined position */
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