#include "free_bitmap.h"

/**
 * init_fbm -- Initializes all 1500 bit to 1 to represent all 1500 data blocks are available.
 *             The first 11 blocks are taken by the (1) super block and (10) INode table.
*/
void init_fbm() {
    block_t block[FREE_BITMAP_SIZE];
    uint8_t *free_bitmap = (uint8_t *) block; 

    for (int i = SUPERBLOCK_SIZE + INODE_TABLE_SIZE; i < SUPERBLOCK_SIZE + INODE_TABLE_SIZE + DATA_BLOCK_SIZE; i++)
        free_bitmap[i] = 1;
    
    write_blocks(SUPERBLOCK_SIZE + INODE_TABLE_SIZE + DATA_BLOCK_SIZE + DIR_BLOCK_SIZE, FREE_BITMAP_SIZE, free_bitmap);
}

/**
 * find_free_block -- Finds the first available block that can be used, and sets it to used.
*/
int find_free_block() {
    int index = -1;
    block_t block[FREE_BITMAP_SIZE];
    read_blocks(SUPERBLOCK_SIZE + INODE_TABLE_SIZE + DATA_BLOCK_SIZE + DIR_BLOCK_SIZE, FREE_BITMAP_SIZE, &block);

    for (int i = SUPERBLOCK_SIZE + INODE_TABLE_SIZE; i < SUPERBLOCK_SIZE + INODE_TABLE_SIZE + DATA_BLOCK_SIZE; i++) {
        if (((uint8_t *) block)[i] == 1) {
            /* Set the bit to 0 since the block at i will be used */
            ((uint8_t *) block)[i] = 0;
            index = i;
            break;
        }
    }
    write_blocks(SUPERBLOCK_SIZE + INODE_TABLE_SIZE + DATA_BLOCK_SIZE + DIR_BLOCK_SIZE, FREE_BITMAP_SIZE, block);
    return index;
}