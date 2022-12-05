/**
 * Simple File System API
 * ------------------------------
 * The following implementation provides a set of functions to create, write, read and remove files
 * into a disk. The API has multiple files. You may find the structure of the file system and
 * the limitations of the following API in the README.md file.
*/
#ifndef SFS_API_H
#define SFS_API_H

/**
 * mksfs -- Initializes the disk and the disk information in-memory.
 * 
 * fresh: indication if the user would like to create a new disk (1) or use an existing disk (0)
*/
void mksfs(int);

/**
 * sfs_getnextfilename -- Gets the name of the next file in the directory table.
 *                        There's also a global counter that keeps track of
 *                        the current index of the directory table.
 *        
 * fname: the buffer where the next filename will be copied to
 * 
 * returns 1 if a new filename has been found and returns 0 when
 * the counter has reached the end of the directory table.
*/
int sfs_getnextfilename(char*);

/**
 * sfs_getfilesize -- Returns the size of the specific file given a path.
 *                    The size of the specific file is found in the INode.
 * 
 * path: file path
 * 
 * returns the size of the file found in the given INode
*/
int sfs_getfilesize(const char*);

/**
 * sfs_fopen -- Opens the given filename or creates a new file with the filename
 *              and returns the index of the file descriptor related to the file.
 * 
 * name: filename to be opened and to be added to the file descriptor
 * 
 * returns the file descriptor index of the newly opened file
*/
int sfs_fopen(char*);

/**
 * sfs_fclose -- Closes the file in the file descriptor table using its index.
 * 
 * fileID: file descriptor table index
 * 
 * returns -1 or 0 if its a success
*/
int sfs_fclose(int);

/**
 * sfs_fwrite -- Writes the given buffer to file.
 * 
 * limitations:
 * - It has only been tested to write on the first block
 * - It does not allocate blocks past the 12 direct pointers
 * - It only writes characters and not individual bytes
 * 
 * fileID: file descriptor index
 * buf: buffer that will be written onto the file
 * length: size of the buffer
*/
int sfs_fwrite(int, const char*, int);

/**
 * sfs_fread -- Reads the file and copies it to the given buffer.
 * 
 * limitations:
 * - It has only been tested to read on the first block
 * - It only writes characters and not individual bytes
 * 
 * fileID: file descriptor index
 * buf: buffer to be written on with the file's data
 * length: size of the buffer
*/
int sfs_fread(int, char*, int);

/**
 * sfs_fseek -- Changes the read/write pointer of a file descriptor entry.
 * 
 * fileID: file descriptor entry index
 * loc: new location of the read/write pointer
 * 
 * returns -1 or 0 if its a success
*/
int sfs_fseek(int, int);

/**
 * sfs_remove -- Removes the file from its allocated data blocks, INode, and directory entry.
 * 
 * file: filename to be removed
 * 
 * returns -1 or 0 if its a success
*/
int sfs_remove(char*);

#endif