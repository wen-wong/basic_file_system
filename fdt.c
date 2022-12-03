#include "fdt.h"

void init_fdt(fdt_t* fdt) {
    for (int i = 0; i < FDT_SIZE; i++)
        fdt[i].inum = -1;
}

int find_free_fdt_entry(fdt_t* fdt) {
    for (int i = 0; i < FDT_SIZE; i++)
        if (fdt[i].inum == -1) return i;
    return -1;
}

void insert_fdt_entry(fdt_t* fdt, int index, int inode, int offset) {
    if (index < 0) return;

    fdt[index].inum = inode;
    fdt[index].foffset = offset;
}

int seek_fdt_entry(fdt_t* fdt, int index, int loc) {
    if (index < 0) return -1;
    fdt[index].foffset = loc;
    return 0;
}

int close_fdt_entry(fdt_t* fdt, int fileIndex) {
    if (fileIndex < 0) return -1;
    if (fdt[fileIndex].inum < 0) return -1;

    fdt[fileIndex].inum = -1;
    fdt[fileIndex].foffset = 0;
    
    return 0;
}