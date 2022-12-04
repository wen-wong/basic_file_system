CFLAGS = -c -g -ansi -pedantic -Wall -std=gnu99 `pkg-config --cflags --libs`

LDFLAGS = `pkg-config fuse --cflags --libs`

# Uncomment on of the following three lines to compile
# SOURCES = sfs_test0.c disk_emu.c sfs_api.c sfs_api.h super_block.c super_block.h inode.c inode.h free_bitmap.c free_bitmap.h directory.c directory.h fdt.c fdt.h constant.h
SOURCES = sfs_test3.c disk_emu.c sfs_api.c sfs_api.h super_block.c super_block.h inode.c inode.h free_bitmap.c free_bitmap.h directory.c directory.h fdt.c fdt.h constant.h

OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=sfs

all: $(SOURCES) $(HEADERS) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	gcc $(OBJECTS) -o $@

.c.o:
	gcc $(CFLAGS) $< -o $@

clean:
	rm -rf *.o *~ $(EXECUTABLE)
	rm -rf file_sys
