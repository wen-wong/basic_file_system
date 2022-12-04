#include "super_block.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * init_superblock -- Initializes the super block with default values
 *                    and writes it to the disk.
*/
void init_superblock() {
    block_t block;
    superblock_t *super_block = (superblock_t *) &block;
    
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

/**
 * check_valid_dish -- Verifies that the disk is of a valid format
 *                     by checking the MAGIC value of it's superblock.
 *                     If it is invalid, the whole program will be exited.
*/
void check_valid_disk() {
    block_t block;
    read_blocks(0, SUPERBLOCK_SIZE, &block);

    if (strcmp(((superblock_t *) &block) -> magic, MAGIC) != 0) {
        printf("Invalid File Format -- Cannot Open the file system.\n");
        exit(EXIT_FAILURE);
    }
}