#include "super_block.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void init_superblock() {
    block_t block[SUPERBLOCK_SIZE];
    superblock_t *super_block = (superblock_t *) block;
    
    strcpy(super_block -> magic, MAGIC);
    super_block -> block_size = BLOCK_SIZE;
    super_block -> fs_size = FILE_SIZE;
    super_block -> inode_length = INODE_TABLE_SIZE;
    super_block -> root_dir = 0;
    super_block -> dir_length = DIR_BLOCK_SIZE;
    super_block -> dir_root_dir = 0;
    super_block -> fbm_length = FREE_BITMAP_SIZE;
    super_block -> fbm_root_dir = 0;

    write_blocks(0, SUPERBLOCK_SIZE, &block);
}

void check_valid_disk() {
    block_t block[SUPERBLOCK_SIZE];
    read_blocks(0, SUPERBLOCK_SIZE, block);

    superblock_t *super_block = (superblock_t *) block;
    if (strcmp(super_block -> magic, MAGIC) != 0) {
        printf("Invalid File Format -- Cannot Open the file system.\n");
        exit(EXIT_FAILURE);
    }
}