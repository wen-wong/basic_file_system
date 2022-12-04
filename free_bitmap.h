#include <stdint.h>
#include <inttypes.h>
#include "disk_emu.h"
#include "constant.h"
#include "block.h"

/**
 * init_fbm -- Initializes all 1500 bit to 1 to represent all 1500 data blocks are available.
 *             The first 11 blocks are taken by the (1) super block and (10) INode table.
*/
void init_fbm();

/**
 * find_free_block -- Finds the first available block that can be used, and sets it to used.
 * 
 * returns the index of the data block
*/
int find_free_block();

/**
 * reset_free_block -- Resets the requested block to a free available block.
 * 
 * index: index of the data block
*/
void reset_free_block(int index);