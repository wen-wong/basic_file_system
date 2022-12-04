#include "disk_emu.h"
#include "constant.h"
#include "block.h"
#include <string.h>

#define INODE_POINTER_SIZE 12
#define INODE_LENGTH 160

/**
 * _inode_t -- Note that the uid and gid have been removed since they are not used
 *             in the sfs_api and omitting them gives a size of 64 bytes in total
 *             which gives a total of 16 inodes per block of 1024 bytes.
*/
typedef struct _inode_t {
    int mode;
    int link_cnt;
    int size;
    int pointers[INODE_POINTER_SIZE];
    int ind_pointer;
} inode_t;

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
void init_inode_table(inode_t* inode_table);

/**
 * set_inode_table -- Initializes the INode table In-Memory with the
 *                    values from the INode table on the disk.
 * 
 * inode_table: INode Table in memory
*/
void set_inode_table(inode_t* inode_table);

/**
 * find_free_inode -- Finds the first available INode.
 * 
 * inode_table: INode Table in memory
 * 
 * returns: index of the available INode
*/
int find_free_inode(inode_t* inode_table);

/**
 * init_inode -- Sets the requested INode to used by changing the link counter.
 * 
 * inode_table: INode table in memory
 * index: Index of the INode to be set to under used
*/
void init_inode(inode_t* inode_table, int index);

/**
 * remove_entry_inode -- Decrements the size property of the root directory INode
 *                       since a directory entry has been removed.
 * 
 * inode_table: INode table in memory
*/
void remove_entry_inode(inode_t* inode_table);

/**
 * remove_inode -- Removes all data to the requested INode In-Memory and on the disk.
 * 
 * inode_table: INode table in memory
 * index: Index of the INode to be reset
*/
void remove_inode(inode_t* inode_table, int index);