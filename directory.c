#include "directory.h"
#include <string.h>

/**
 * init_dir_entry_table -- Initializes the directory table where all inode properties
 *                         are set to -1 since they are unused.
 * 
 * dir_table: directory table in memory
*/
void init_dir_entry_table(dirent_t* dir_table) {
    for (int i = 1; i < DIR_ENTRY_SIZE; i++)
        dir_table[i].inode = -1;

    write_blocks(SUPERBLOCK_SIZE + INODE_TABLE_SIZE + DATA_BLOCK_SIZE, DIR_BLOCK_SIZE, dir_table);
}

/**
 * find_inode_with_filename -- Finds the inode related to the filename. It returns -1 if it cannot be found.
 * 
 * name: filename
 * dir_table: directory table in memory
 * 
 * returns the inode index
*/
int find_inode_with_filename(char *name, dirent_t* dir_table) {
    for (int i = 0; i < DIR_ENTRY_SIZE; i++)
        if (strcmp(dir_table[i].filename, name) == 0)
            return dir_table[i].inode;
    return -1;
}

/**
 * find_inode_with_path -- Finds the inode related to the path. It returns -1 if it cannot be found.
 * 
 * name: filename
 * dir_table: directory table in memory
 * 
 * returns the inode index
*/
int find_inode_with_path(const char *name, dirent_t* dir_table) {
    for (int i = 0; i < DIR_ENTRY_SIZE; i++)
        if (strcmp(dir_table[i].filename, name) == 0)
            return dir_table[i].inode;
    return -1;
}

/**
 * find_free_entry -- Finds the first available directory entry.
 * 
 * dir_table: directory table in memory
 * 
 * returns the directory entry index
*/
int find_free_entry(dirent_t* dir_table) {
    for (int i = 0; i < DIR_ENTRY_SIZE; i++)
        if (dir_table[i].inode == -1) return i;
    return -1;
}

/**
 * insert_dir_entry -- Inserts the directory entry of the requested index.
 * 
 * dir_table: directory table in memory
 * index: directory entry index
 * name: filename
 * inode: inode index
*/
void insert_dir_entry(dirent_t* dir_table, int index, char *name, int inode) {
    if (index < 0) return;

    strcpy(dir_table[index].filename, name);
    dir_table[index].inode = inode;
}

/**
 * remove_dir_entry_mem -- Removes the directory entry in memory.
 * 
 * dir_table: directory table in memory
 * name: filename
 * 
 * returns directory entry index
*/
int remove_dir_entry_mem(dirent_t* dir_table, char *name) {
    for (int i = 0; i < DIR_ENTRY_SIZE; i++) {
        if (strcmp(dir_table[i].filename, name) == 0) {
            strcpy(dir_table[i].filename, "");
            dir_table[i].inode = -1;
            return i;
        }
    }
    return -1;
}

/**
 * remove_dir_entry_disk -- Removes the directory entry on the disk.
 * 
 * dir_block_index: block index of the directory entry
 * dir_index: index of the directory entry
*/
void remove_dir_entry_disk(int dir_block_index, int dir_index) {
    block_t block;
    read_blocks(dir_block_index, 1, &block);

    dirent_t *dir = (dirent_t *) &block;
    memset(dir[dir_index % DIR_PER_BLOCK].filename, 0, DIR_PER_BLOCK - sizeof(int));
    dir[dir_index % DIR_PER_BLOCK].inode = -1;
    
    write_blocks(dir_block_index, 1, &block);
}