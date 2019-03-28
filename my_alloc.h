//
// Created by Reyes on 28.03.2019.
//

#ifndef BLATT8_MY_ALLOC_H
#define BLATT8_MY_ALLOC_H

#include <stddef.h>
#include <stdbool.h>

#define POW2(X) (1 << (X))

typedef unsigned char uchar;

typedef struct cell8_t {
    uchar right : 4;
    uchar left : 4;
} cell8_t;

typedef struct cell16_t {
    uchar rSize : 3;
    uchar rSet : 1;
    uchar lSize : 3;
    uchar lSet : 1;
} cell16_t;

typedef union cell_u {
    cell8_t nibbles8;
    cell16_t nibbles16;
} cell_u;

typedef struct node_t {
    size_t index;
    bool leftNibble;
    bool leftHalfNibble;
} node_t;

cell_u *data;


cell_u *makeNewBlock();

void setNext(uchar *cur, uchar *next);

cell_u *getNext(cell_u *block);

cell_u *appendNew(cell_u *block);

bool sond8(cell_u *block, node_t *node);

bool sond16(cell_u *block, node_t *node);

bool sond(cell_u *block, node_t *node, uchar size);

void *toAddress(cell_u *block, node_t *node);

uchar normalize(size_t size);

void init_malloc();

void *my_malloc(size_t size);

void free(void *ptr);

void printBlock(cell_u *block);

void printData();


#endif //BLATT8_MY_ALLOC_H
