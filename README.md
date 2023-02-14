# Simple File System

The following documentation covers
- The File System structure
- The Project Structure
- The Execution Instructions
- The Simple File System limitations
- The modifications to certain files

## File System Structure

The file system has four main components on the disk
- Superblock
- INode Table
- Free Bitmap
- Data Blocks

and it has three main components in memory
- File Descriptor Table
- Directory Table
- INode Table

The system is organize in terms of **blocks** where each block represents **1024 bytes**, and it has a maximum size of **1500 data blocks** where a file requires a minimum of **10 blocks**.

Firstly, to accommodate **1500 data blocks**, there will be at least 150 INodes to have each file associated to one INode. It is important to note that an INode has a size of 64 bytes. One block can have a maximum of 16 INodes `(1024 / 64 = 16)`. So the INode table will have a size of **10 blocks** with 160 INodes to cover the minimum requirement of 150 INodes.

It is important to note that the INode is of size 64 bytes since the `uid` and `gid` properties have been removed to accommodate the size of a single block.

Secondly, the free bitmap will need to represent the following:
- Superblock: 1 block
- INode Table: 10 blocks
- Data Blocks: 1500 blocks

So in total, it will need at least 1511 bytes to represent each byte as a block. Thus, 2 blocks are required.

Thirdly, the directory table are stored inside of the root directory (the first INode), and it has a size of 32 bytes where one block will have 32 directory entries.

In conclusion, the File System has the following order and size

1. Superblock: 1 block
2. INode Table: 10 blocks
3. Data Blocks: 1500 blocks
4. Free Bitmap: 2 blocks

## Project Structure
The project is divided into three layers
1. `block.h`, `constant.h` and `disk_emu.h`
2. `super_block.h`, `free_bitmap.h`, `inode.h`, `directory.h` and `fdt.h`
3. `sfs_api.h`

Note that each **header** file except for `block.h` and `constant.h` has a `.c` file with its implementation.

### First Layer
- `block.h` - Structure of a block

- `constant.h` - Constants used between files in the second and third layer

### Second Layer
- `super_block.h` - API to initialize the Superblock
- `free_bitmap.h` - API to initialize and edit the Free Bitmap
- `inode.h` - API to initialize and edit the INode table in-memory and on the disk
- `directory.h` - API to initialize and edit the directory table in-memory and on the disk
- `fdt.h` - API to initialize and edit the file descriptor table

### Third Layer
- `sfs_api.h` - API to initialize and edit the file system

## Execution Instructions

1. Go to the `Makefile` and uncomment the following `SOURCES` to run sfs_test0
```
SOURCES = sfs_test0.c disk_emu.c sfs_api.c sfs_api.h super_block.c super_block.h inode.c inode.h free_bitmap.c free_bitmap.h directory.c directory.h fdt.c fdt.h constant.h
```

2. Remove previous executable files
```
make clean
```

3. Compile the source code
```
make
```

4. Execute the executable
```
./sfs
```

5. Redo step 1 - 4 and uncomment the other `SOURCES` to run sfs_test3

## SFS Limitations
- The API has only been tested with `sfs_test0.c` and `sfs_test3.c`
- Read and Write has only been tested to read/write on the first block.
- Read and Write does not check/allocate blocks past the 12 direct pointers.
- It only writes characters using strcpy and not individual bytes using memcpy.

## Modifications to Certain Files
### `Makefile`
The `Makefile` has been edited to execute the files `sfs_test1.c` and `sfs_test2.c`. The **fuse** options have also been removed since they are not implemented in this API. Lastly, There are multiple files that havce been added to be compiled since they are external APIs that are used in the API. You may read the *Project Structure* to see the list of external APIs that were written.

### `sfs_test0.c`
The line `sfs_remove("some_name.txt");` has been uncommented to verify if the file has been removed from the disk. You may comment it to verify that the content is indeed valid.

### `sfs_test3.c`
A constant `MAXFILENAME` has been added to run the program. It has been set to 28 to represent the number of characters a filename can have. It is set to 28 since the directory entry of this API has a size of 32 bytes where there's an int of 4 bytes and a string of 28 bytes.
