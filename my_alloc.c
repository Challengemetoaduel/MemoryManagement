//
// Created by Reyes on 28.03.2019.
//

#include <stdio.h>
#include "my_alloc.h"
#include "my_system.h"

uchar S32 =3;
uchar S256 = 6;

cell_u *makeNewBlock() {
    cell_u *block = get_block_from_system();
    for (int i = 0; i < 248; i+=8) {
        block[i].nibbles8.left = S256;
    }
    return block;
}

void setNext(uchar *cur, uchar *next) {
    uchar **cur_p = (uchar **) (cur + 248);
    *cur_p = next;
}

cell_u *getNext(cell_u *block) {
    cell_u **block_p = (cell_u **) (block + 248);
    return *block_p;
}

cell_u *appendNew(cell_u *block) {
    cell_u *new = makeNewBlock();
    cell_u **block_p = (cell_u **) (block + 248);
    cell_u *next = *block_p;

    *block_p = new;
    block_p = (cell_u **) (new + 248);
    *block_p = next;

    return new;
}

bool sond8(cell_u *block, node_t *node) {
    for (size_t index= 0; index < 248;) {
        cell16_t cur = block[index].nibbles16;

        if(cur.lSize >= S32) {
            ////32 - 256
            if(cur.lSet) {////1011 0000-1110 0000
                ////X32 - X256
                index += POW2(cur.lSize - S32);
            } else {////0011 0000 - 0110 0000
                ////F32 - F256
                block[index].nibbles8.left = 8;  //XF8
                block[index].nibbles8.right = 2; //F16
                node->index = index;
                node->leftNibble = true;
                node->leftHalfNibble = true;

                uchar prevSize = cur.lSize;
                index ++;
                for (uchar i = S32; i < prevSize; ++i) {
                    block[index].nibbles8.left = i;
                    index += POW2(i - S32);
                }

                for (uchar i = 3; i < 6; ++i) {
                    i += POW2(i - 3);
                    block[i].nibbles8.left = i;
                }
                return true;
            };
        } else if(cur.lSet) {
            ////X16, XF8, XX8
            if(cur.lSize > 0) {
                ////X16, XX8

                ////Second half:
                if(cur.rSet) {
                    ////XF8, XX8
                    if(cur.rSize) {
                        ////XX8
                        index += 1;
                    } else {
                        ////XF8
                        block[index].nibbles8.right = 9; //XX8
                        node->index = index;
                        node->leftNibble = false;
                        node->leftHalfNibble = false;
                        return true;
                    }
                } else {
                    ////F16, FX8
                    if(cur.rSize == 1) {
                        ////FX8
                        block[index].nibbles8.right = 9; //XX8
                        node->index = index;
                        node->leftNibble = false;
                        node->leftHalfNibble = true;
                        return true;
                    } else {
                        ////F16
                        block[index].nibbles8.right = 8; //XF8
                        node->index = index;
                        node->leftNibble = false;
                        node->leftHalfNibble = false;
                        return true;
                    }
                }
            } else {
                ////XF8
                block[index].nibbles8.left = 9; //XX8
                node->index = index;
                node->leftNibble = true;
                node->leftHalfNibble = false;
                return true;
            }
        } else {
            /////F16, FX8
            if(cur.lSize == 1) {
                ////FX8
                block[index].nibbles8.left = 9; //XX8
                node->index = index;
                node->leftNibble = true;
                node->leftHalfNibble = true;
                return true;
            } else {
                ////F16
                block[index].nibbles8.left = 8; //XF8
                node->index = index;
                node->leftNibble = true;
                node->leftHalfNibble = true;
                return true;
            }
        }
    }
    return false;
}

bool sond16(cell_u *block, node_t *node) {
    node->leftNibble = true;
    for (size_t index= 0; index < 248;) {
        cell16_t cur = block[index].nibbles16;
        if (cur.lSize >= S32) {
            if(cur.lSet) {
                index += POW2(cur.lSize - S32);
            } else {
                uchar prevSize = cur.lSize;

                block[index].nibbles8.left = 10; //X16
                node->index = index;
                node->leftHalfNibble = true;
                index++;
                for (uchar i = S32; i < prevSize; ++i) {
                    block[index].nibbles8.left = i;
                    index += POW2(i - S32);
                }
                return true;
            }
        } else if(cur.lSet || cur.lSize < 2) {
            if(cur.rSet || cur.rSize < 2) {
                index+=1;
            } else {
                block[index].nibbles8.right = 10;
                node->index = index;
                node->leftHalfNibble = false;
                return true;
            }
        } else {
            block[index].nibbles8.left = 10;
            node->index = index;
            node->leftHalfNibble = true;
            return true;
        }
    }
    return false;
}

bool sond(cell_u *block, node_t *node, uchar size) {
    if(size == 1) {
        return sond8(block, node);
    }
    if(size == 2) {
        return sond16(block, node);
    }

    node->leftNibble = true;
    node->leftHalfNibble = true;
    for(size_t index = 0; index<248;) {
        cell16_t cur = block[index].nibbles16;
        if(cur.lSize < size) {
            index += POW2(size - S32);
        } else if(cur.lSet){
            index += POW2(cur.lSize - S32);
        } else {
            node->index = index;
            node->leftHalfNibble = true;

            block[index].nibbles16.lSize = size; //X16
            block[index].nibbles16.lSet = 1;

            uchar prevSize = cur.lSize;

            index += POW2(size - S32);
            for (uchar i = size; i < prevSize; ++i) {
                block[index].nibbles8.left = i;
                index += POW2(i - S32);
            }
            return true;
        }
    }
    return false;

}

void *toAddress(cell_u *block, node_t *node) {
    size_t index = node->index * 32;
    if(!node->leftNibble)
        index += 16;
    if(!node->leftHalfNibble)
        index += 8;
    return block + 256 + index;
}

uchar normalize(size_t size) {
    size--;
    uchar newSize = 0;
    while (size >>= 1) newSize++;
    return (uchar) (newSize - 1);
}

void init_malloc() {
}

void *my_malloc(size_t size) {
    if(!data) {
        data = makeNewBlock();
        cell_u **data_p = (cell_u **) (data + 248);
        *data_p = data;
    }

    uchar normalizedSize = normalize(size);

    node_t node;
    cell_u *current = data;
    do {
        if(sond(current, &node, normalizedSize)) {
            data = current;
            return toAddress(current, &node);
        }
        current = getNext(current);
    } while (current != data);

    while (true) {
        current = appendNew(current);
        if(sond(current, &node, normalizedSize)) {
            data = current;
            return toAddress(current, &node);
        }
    }
}

void free(void *ptr) {
}

void printBlock(cell_u *block) {
    printf("%p: \n", (void *) block);

    printf(" ");
    for (int j = 0; j < 32; ++j) {
        printf(" [%6d]", j);
        for (int k = 0; k < 16; ++k) {
            printf("  0x%x", k);
        }
    }
    printf("\n");
    for (int i = 0; i < 16; ++i) {
        printf(" ");
        for (int j = 0; j < 32; ++j) {
            printf(" |0x%03xx|", j*16 + i);
            for (int k = 0; k < 16; ++k) {
                uchar *c_p = (uchar *) (block + (256 * j + 16 * i + k));
                printf(" 0x%02x", *c_p);
            }
        }
        printf("\n");
    }
}

void printData() {
    cell_u *current = data;
    if(!data)
        return;

    do{
        printBlock(current);
        current = getNext(current);
    } while (current != data);

    printf(" ");
    for (int j = 0; j < 32; ++j) {
        printf(" [%6d]", j);
        for (int k = 0; k < 16; ++k) {
            printf("  0x%x", k);
        }
    }
    printf("\n");
}