#include "inode.h"

/**
 * init_inode_table -- Initializes the INode table In-Memory and 
 *                     writes it on the disk. Furthermore, all
 *                     pointers are set to -1 so that we may
 *                     check which pointer is available and 0 points to
 *                     the first block, i.e., the super block. The first
 *                     INode has a link counter set to 1 since it represents
 *                     the root directory where the directory entries will
 *                     be referred from.
 * 
 * inode_table: INode Table in memory
*/
void init_inode_table(inode_t* inode_table) {
    /* Initialize the root directory INode */
    inode_table[0].mode = 1;
    inode_table[0].link_cnt = 1;
    inode_table[0].size = 0;
    inode_table[0].ind_pointer = -1;
    for (int pt = 0; pt < INODE_POINTER_SIZE; pt++)
            inode_table[0].pointers[pt] = -1;

    /* Initialize the other INodes */
    for (int index = 1; index < INODE_LENGTH; index++) {
        inode_table[index].mode = 0;
        inode_table[index].link_cnt = 0;
        inode_table[index].size = -1;
        inode_table[index].ind_pointer = -1;
        for (int pt = 0; pt < INODE_POINTER_SIZE; pt++)
            inode_table[index].pointers[pt] = -1;
    }

    /* Write the INode In-Memory to the disk */
    write_blocks(SUPERBLOCK_SIZE, INODE_TABLE_SIZE, inode_table);
}

/**
 * set_inode_table -- Initializes the INode table In-Memory with the
 *                    values from the INode table on the disk.
 * 
 * inode_table: INode Table in memory
*/
void set_inode_table(inode_t* inode_table) {
    block_t blocks[INODE_TABLE_SIZE];

    /* Read the INode on the disk to a temporary set of blocks */
    read_blocks(SUPERBLOCK_SIZE, INODE_TABLE_SIZE, blocks);

    inode_t *inodes = (inode_t *) &blocks;

    /* Initializes all INodes In-Memory */
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

/**
 * find_free_inode -- Finds the first available INode.
 * 
 * inode_table: INode Table in memory
 * 
 * returns: index of the available INode
*/
int find_free_inode(inode_t* inode_table) {
    for (int index = 0; index < INODE_LENGTH; index++)
        if (inode_table[index].link_cnt == 0) return index;
    return -1;
}

/**
 * init_inode -- Sets the requested INode to used by changing the link counter.
 * 
 * inode_table: INode table in memory
 * index: Index of the INode to be set to under used
*/
void init_inode(inode_t* inode_table, int index) {
    inode_table[index].mode = 1;
    inode_table[index].link_cnt = 1;
    inode_table[index].size = 0;

    write_blocks(SUPERBLOCK_SIZE, INODE_TABLE_SIZE, inode_table);
}

/**
 * remove_entry_inode -- Decrements the size property of the root directory INode
 *                       since a directory entry has been removed.
 * 
 * inode_table: INode table in memory
*/
void remove_entry_inode(inode_t* inode_table) {
    inode_table[0].size -= DIR_PER_BLOCK;

    write_blocks(SUPERBLOCK_SIZE, INODE_TABLE_SIZE, inode_table);
}