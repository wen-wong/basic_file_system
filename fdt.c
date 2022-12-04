#include "fdt.h"

/**
 * init_fdt -- Initializes the file descriptor table, and
 *             and sets all inum properties to -1 so that
 *             we can parse through the table to find the
 *             available entries.
 * 
 * fdt: file descriptor table in memory
*/
void init_fdt(fdt_t* fdt) {
    for (int i = 0; i < FDT_SIZE; i++)
        fdt[i].inum = -1;
}

/**
 * find_free_fdt_entry -- Finds the first available file descriptor entry.
 * 
 * fdt: file descriptor table in memory.
*/
int find_free_fdt_entry(fdt_t* fdt) {
    for (int i = 0; i < FDT_SIZE; i++)
        if (fdt[i].inum == -1) return i;
    return -1;
}

/**
 * insert_fdt_entry -- Inserts the file descriptor entry with its data to
 *                     the specified index.
 * 
 * fdt: file descriptor table in memory
 * index: index of the file descriptor entry to be added
 * inode: INode of the entry
 * offset: read/write pointer of the entry
*/
void insert_fdt_entry(fdt_t* fdt, int index, int inode, int offset) {
    if (index < 0) return;

    fdt[index].inum = inode;
    fdt[index].foffset = offset;
}

/**
 * seek_fdt_entry -- Changes the location of the read/write pointer of the
 *                   specified file descriptor entry.
 * 
 * fdt: file descriptor table in memory
 * index: index of the file descriptor entry
 * loc: new location of the read/write pointer
 * 
 * returns 0 or -1 to show if the action was successful
*/
int seek_fdt_entry(fdt_t* fdt, int index, int loc) {
    if (index < 0) return -1;
    fdt[index].foffset = loc;
    return 0;
}

/**
 * close_fdt_entry -- Closes the file descriptor entry.
 * 
 * fdt: file descriptor table in memory
 * fileIndex: index of the file descriptor entry
 * 
 * returns 0 or -1 to show if the action was successful
*/
int close_fdt_entry(fdt_t* fdt, int fileIndex) {
    if (fileIndex < 0) return -1;
    if (fdt[fileIndex].inum < 0) return -1;

    fdt[fileIndex].inum = -1;
    fdt[fileIndex].foffset = 0;
    
    return 0;
}