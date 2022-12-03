#include "inode.h"
#include <stdio.h>

void init_inode_table(inode_t* inode_table) {
    inode_table[0].mode = 1;
    inode_table[0].link_cnt = 1;
    inode_table[0].size = 0;
    inode_table[0].ind_pointer = -1;
    for (int pt = 0; pt < INODE_POINTER_SIZE; pt++)
            inode_table[0].pointers[pt] = -1;

    for (int index = 1; index < INODE_LENGTH; index++) {
        inode_table[index].mode = 0;
        inode_table[index].link_cnt = 0;
        inode_table[index].size = 0;
        inode_table[index].ind_pointer = -1;
        for (int pt = 0; pt < INODE_POINTER_SIZE; pt++)
            inode_table[index].pointers[pt] = -1;
    }


    write_blocks(SUPERBLOCK_SIZE, INODE_TABLE_SIZE, inode_table);
}

void set_inode_table(inode_t* inode_table) {
    block_t block[INODE_TABLE_SIZE];
    read_blocks(SUPERBLOCK_SIZE, INODE_TABLE_SIZE, block);

    inode_t *inodes = (inode_t *) block;

    for (int index = 0; index < INODE_LENGTH; index++) {
        inode_table[index] = (inode_t) {
            .mode = inodes[index].mode,
            .link_cnt = inodes[index].link_cnt,
            .size = inodes[index].size,
            .ind_pointer = inodes[index].ind_pointer
        };
        for (int pt = 0; pt < INODE_POINTER_SIZE; pt++)
            inode_table[index].pointers[pt] = inodes[index].pointers[pt];
    }
}

int find_free_inode(inode_t* inode_table) {
    for (int index = 0; index < INODE_LENGTH; index++)
        if (inode_table[index].link_cnt == 0) return index;
    return -1;
}

void init_inode(inode_t* inode_table, int index) {
    inode_table[index].mode = 1;
    inode_table[index].link_cnt = 1;
    inode_table[index].size = 0;

    write_blocks(SUPERBLOCK_SIZE, INODE_TABLE_SIZE, inode_table);
}

void remove_entry_inode(inode_t* inode_table) {
    inode_table[0].size -= DIR_PER_BLOCK;

    write_blocks(SUPERBLOCK_SIZE, INODE_TABLE_SIZE, inode_table);
}

void remove_inode(inode_t* inode_table, int index) {
    block_t temp_block;

    inode_table[index].mode = 0;
    inode_table[index].link_cnt = 0;
    inode_table[index].size = 0;

    printf("%d\n", inode_table[index].pointers[0]);

    for (int i = 0; i < INODE_POINTER_SIZE; i++) {
        if (inode_table[index].pointers[i] < 0) break;
        int block_index = inode_table[index].pointers[i];
        memcpy(&temp_block, "", BLOCK_SIZE);
        write_blocks(block_index, 1, &temp_block);
        inode_table[index].pointers[i] = -1;
    }

    // ! Clear Indirect pointer
    write_blocks(SUPERBLOCK_SIZE, INODE_TABLE_SIZE, inode_table);
}