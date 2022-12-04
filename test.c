#include <stdio.h>
#include <string.h>
#include "sfs_api.h"

int main() {
    mksfs(1);       /* Initialize the file system. */
    int f = sfs_fopen("test.txt");
    char my_data[] = "The quick brown fox jumps over the lazy dog";
    char out_data[1024];
    printf("test1\n");
    sfs_fwrite(f, my_data, sizeof(my_data)+1);
    sfs_fseek(f, 0);
    sfs_fread(f, out_data, sizeof(out_data)+1);
    if (strcmp(out_data,"The quick brown fox jumps over the lazy dog") != 0) {
        printf("ERROR: sfs_fseek and sfs_fread failed. Expected 'The quick brown fox jumps over the lazy dog' but got '%s'\n",out_data);
    }
    return 0;
}