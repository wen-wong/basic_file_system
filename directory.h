#include "disk_emu.h"
#include "constant.h"
#include "block.h"

#define ENTRY_SIZE 32

typedef struct _dirent_t {
    char filename[ENTRY_SIZE - sizeof(int)];
    int inode;
} dirent_t;

/**
 * init_dir_entry_table -- Initializes the directory table where all inode properties
 *                         are set to -1 since they are unused.
 * 
 * dir_table: directory table in memory
*/
void init_dir_entry_table(dirent_t* dir_table);

/**
 * find_inode_with_filename -- Finds the inode related to the filename. It returns -1 if it cannot be found.
 * 
 * name: filename
 * dir_table: directory table in memory
 * 
 * returns the inode index
*/
int find_inode_with_filename(char *name, dirent_t* dir_table);

/**
 * find_inode_with_path -- Finds the inode related to the path. It returns -1 if it cannot be found.
 * 
 * name: filename
 * dir_table: directory table in memory
 * 
 * returns the inode index
*/
int find_inode_with_path(const char *name, dirent_t* dir_table);

/**
 * find_free_entry -- Finds the first available directory entry.
 * 
 * dir_table: directory table in memory
 * 
 * returns the directory entry index
*/
int find_free_entry(dirent_t* dir_table);

/**
 * insert_dir_entry -- Inserts the directory entry of the requested index.
 * 
 * dir_table: directory table in memory
 * index: directory entry index
 * name: filename
 * inode: inode index
*/
void insert_dir_entry(dirent_t* dir_table, int index, char *name, int inode);

/**
 * remove_dir_entry_mem -- Removes the directory entry in memory.
 * 
 * dir_table: directory table in memory
 * name: filename
 * 
 * returns directory entry index
*/
int remove_dir_entry_mem(dirent_t* dir_table, char *name);