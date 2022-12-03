#include "free_bitmap.h"

void init_fbm() {
    block_t block[FREE_BITMAP_SIZE];
    uint8_t *free_bitmap = (uint8_t *) block; 

    for (int i = SUPERBLOCK_SIZE + INODE_TABLE_SIZE; i < SUPERBLOCK_SIZE + INODE_TABLE_SIZE + DATA_BLOCK_SIZE; i++)
        free_bitmap[i] = 1;
    
    write_blocks(SUPERBLOCK_SIZE + INODE_TABLE_SIZE + DATA_BLOCK_SIZE + DIR_BLOCK_SIZE, FREE_BITMAP_SIZE, free_bitmap);
}

int find_free_block() {
    int index = -1;
    block_t block[FREE_BITMAP_SIZE];
    read_blocks(SUPERBLOCK_SIZE + INODE_TABLE_SIZE + DATA_BLOCK_SIZE + DIR_BLOCK_SIZE, FREE_BITMAP_SIZE, &block);

    for (int i = SUPERBLOCK_SIZE + INODE_TABLE_SIZE; i < SUPERBLOCK_SIZE + INODE_TABLE_SIZE + DATA_BLOCK_SIZE; i++) {
        if (((uint8_t *) block)[i] == 1) {
            ((uint8_t *) block)[i] = 0;
            index = i;
            break;
        }
    }
    write_blocks(SUPERBLOCK_SIZE + INODE_TABLE_SIZE + DATA_BLOCK_SIZE + DIR_BLOCK_SIZE, FREE_BITMAP_SIZE, block);
    return index;
}