#define FDT_SIZE 320

typedef struct _fdt_t {
    int inum;
    int foffset;
} fdt_t;

void init_fdt(fdt_t* fdt); 

int find_free_fdt_entry(fdt_t* fdt);

void insert_fdt_entry(fdt_t* fdt, int index, int inode, int offset);

int seek_fdt_entry(fdt_t* fdt, int index, int loc);

int close_fdt_entry(fdt_t* fdt, int fileIndex);