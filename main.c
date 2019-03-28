#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "my_alloc.h"

void my_assert(bool p, char *str) {
    if(!p) {
        printf("%s\n", str);
        exit(1);
    }
}

cell_u *block;

void testMakeNewBlock() {
    block = makeNewBlock();
    for (int i = 0; i < 248; i+=8) {
        my_assert(block[i].nibbles8.left == 6, "Should be 6");
        my_assert(block[i].nibbles8.right == 0, "Should be 0");
        for(int j=1; j<8; j++) {
            my_assert(block[i + j].nibbles8.left == 0, "Should be 0");
            my_assert(block[i + j].nibbles8.right == 0, "Should be 0");
        }
    }

//    setNext((uchar *) block, (uchar *) block);
//    printBlock(block);
//    printf("%p\n", getNext(block));
}

void testSetNext() {
    int r = 50;
    setNext((uchar *) block, (uchar *) &r);
    my_assert(*((cell_u **) (block + 248)) == (cell_u *) &r, "Error setting next.");

    setNext((uchar *) block, (uchar *) block);
    my_assert(*((cell_u **) (block + 248)) == block, "Error setting next.");
}

void testGetNext() {
    int bla = 50;
    *((cell_u **) (block + 248)) = (cell_u *) &bla;
    my_assert(getNext(block) == (cell_u *) &bla, "Error getting next");

    *((cell_u **) (block + 248)) = block;
    my_assert(getNext(block) == block, "Error getting next");
}

void testAppendNew() {
    cell_u *new = appendNew(block);
    my_assert(getNext(block) == new, "Error appending");
    my_assert(getNext(new) == block, "Error appenging");
    block = new;
}

void testToAddress() {
    node_t node;
    srand(time(NULL));
    for (int i = 0; i < 200; ++i) {
        node.index = (size_t) (rand() % 248);
        node.leftNibble = !(rand() % 6);
        node.leftHalfNibble = !(rand() % 120);

        cell_u *addr = block + 256 + node.index *32;
        if(!node.leftNibble) addr += 16;
        if(!node.leftHalfNibble) addr += 8;

        my_assert(toAddress(block, &node) == addr, "To address wrong");
        my_assert(8192 >= (int) (addr - 8192), "Addres wronger");
    }
}

void testNormalize() {
    size_t sizes[] = {8, 9,15,16, 17,31,32, 33,63,64, 65,127,128, 129,255,256};
    int nSizes[] = {1, 2,2,2,  3,3,3,    4,4,4,    5,5,5, 6,6,6};
    for (int i = 0; i < 16; ++i) {
        my_assert(nSizes[i] == normalize(sizes[i]), "Normalized dont work none");
    }
}

int main() {
    printf("Hello, World!\n");
    testMakeNewBlock();
    testSetNext();
    testGetNext();
    testAppendNew();
    testToAddress();
    testNormalize();
    printf("Goodbye, World!\n");
    return 0;
}