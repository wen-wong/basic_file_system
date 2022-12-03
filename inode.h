#include "disk_emu.h"
#include "constant.h"
#include "block.h"
#include <string.h>

#define INODE_POINTER_SIZE 12
#define INODE_LENGTH 160

typedef struct _inode_t {
    int mode;
    int link_cnt;
    int size;
    int pointers[INODE_POINTER_SIZE];
    int ind_pointer;
} inode_t;

void init_inode_table(inode_t* inode_table);

void set_inode_table(inode_t* inode_table);

int find_free_inode(inode_t* inode_table);

void init_inode(inode_t* inode_table, int index);

void remove_entry_inode(inode_t* inode_table);

void remove_inode(inode_t* inode_table, int index);