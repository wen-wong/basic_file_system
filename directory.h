#include "disk_emu.h"
#include "constant.h"
#include "block.h"

#define ENTRY_SIZE 32

typedef struct _dirent_t {
    char filename[ENTRY_SIZE - sizeof(int)];
    int inode;
} dirent_t;

void init_dir_entry_table(dirent_t* dir_table);

int find_inode_with_filename(char *name, dirent_t* dir_table);

int find_free_entry(dirent_t* dir_table);

void insert_dir_entry(dirent_t* dir_table, int index, char *name, int inode);

int remove_dir_entry_mem(dirent_t* dir_table, char *name);

void remove_dir_entry_disk(dirent_t* dir_table, int dir_block_index, int dir_index);