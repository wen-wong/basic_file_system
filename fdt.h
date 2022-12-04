#define FDT_SIZE 320

typedef struct _fdt_t {
    int inum;
    int foffset;
} fdt_t;

/**
 * init_fdt -- Initializes the file descriptor table, and
 *             and sets all inum properties to -1 so that
 *             we can parse through the table to find the
 *             available entries.
 * 
 * fdt: file descriptor table in memory
*/
void init_fdt(fdt_t* fdt); 

/**
 * find_free_fdt_entry -- Finds the first available file descriptor entry.
 * 
 * fdt: file descriptor table in memory.
*/
int find_free_fdt_entry(fdt_t* fdt);

/**
 * insert_fdt_entry -- Inserts the file descriptor entry with its data to
 *                     the specified index.
 * 
 * fdt: file descriptor table in memory
 * index: index of the file descriptor entry to be added
 * inode: INode of the entry
 * offset: read/write pointer of the entry
*/
void insert_fdt_entry(fdt_t* fdt, int index, int inode, int offset);

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
int seek_fdt_entry(fdt_t* fdt, int index, int loc);

/**
 * close_fdt_entry -- Closes the file descriptor entry.
 * 
 * fdt: file descriptor table in memory
 * fileIndex: index of the file descriptor entry
 * 
 * returns 0 or -1 to show if the action was successful
*/
int close_fdt_entry(fdt_t* fdt, int fileIndex);