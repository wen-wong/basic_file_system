#include "directory.h"
#include <string.h>
#include <stdio.h>

void init_dir_entry_table(dirent_t* dir_table) {
    for (int i = 1; i < DIR_ENTRY_SIZE; i++)
        dir_table[i].inode = -1;

    write_blocks(SUPERBLOCK_SIZE + INODE_TABLE_SIZE + DATA_BLOCK_SIZE, DIR_BLOCK_SIZE, dir_table);
}

int find_inode_with_filename(char *name, dirent_t* dir_table) {
    for (int i = 0; i < DIR_ENTRY_SIZE; i++)
        if (strcmp(dir_table[i].filename, name) == 0)
            return dir_table[i].inode;
    return -1;
}

int find_free_entry(dirent_t* dir_table) {
    for (int i = 0; i < DIR_ENTRY_SIZE; i++)
        if (dir_table[i].inode == -1) return i;
    return -1;
}

void insert_dir_entry(dirent_t* dir_table, int index, char *name, int inode) {
    if (index < 0) return;

    strcpy(dir_table[index].filename, name);
    dir_table[index].inode = inode;
}

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

void remove_dir_entry_disk(dirent_t* dir_table, int dir_block_index, int dir_index) {
    block_t block;
    read_blocks(dir_block_index, 1, &block);

    dirent_t *dir = (dirent_t *) &block;
    memcpy(dir[dir_index % DIR_PER_BLOCK].filename, "", DIR_PER_BLOCK - sizeof(int));
    dir[dir_index % DIR_PER_BLOCK].inode = -1;
    
    write_blocks(dir_block_index, 1, &block);
}