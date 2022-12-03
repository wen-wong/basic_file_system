#include "disk_emu.h"
#include "constant.h"
#include "block.h"

#define MAGIC "0xACBD0005"

typedef struct _superblock_t {
    char magic[10];
    int block_size;
    int fs_size;
    int inode_length;
    int root_dir;
    int dir_length;
    int dir_root_dir;
    int fbm_length;
    int fbm_root_dir;
} superblock_t;

void init_superblock();

void check_valid_disk();