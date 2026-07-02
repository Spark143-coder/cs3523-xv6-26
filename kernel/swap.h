#ifndef SWAP_H
#define SWAP_H

#include "types.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

#define SWAP_SIZE 2048

struct swap_slot{
    char pg[PGSIZE];
    int available;
};

extern struct swap_slot swap_space[SWAP_SIZE];
extern struct spinlock swap_lock;

void swapinit(void);

#endif