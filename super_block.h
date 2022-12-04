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

/**
 * init_superblock -- Initializes the super block with default values
 *                    and writes it to the disk.
*/
void init_superblock();

/**
 * check_valid_dish -- Verifies that the disk is of a valid format
 *                     by checking the MAGIC value of it's superblock.
 *                     If it is invalid, the whole program will be exited.
*/
void check_valid_disk();