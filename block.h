#ifndef BLOCK_H
#define BLOCK_H

#define BLOCK_SIZE 1024

typedef struct _block_t {
    char data [BLOCK_SIZE];
} block_t;

#endif