#include <stdio.h>
#include "sfs_api.h"

int main() {
    mksfs(1);
    int f = sfs_fopen("hello");
    sfs_fseek(f, 0);
    int result = sfs_fwrite(f, "bye", 3);
    sfs_fwrite(f, "bye", 3);
    return 0;
}