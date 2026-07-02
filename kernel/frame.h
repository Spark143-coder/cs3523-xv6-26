#ifndef FRAME_H
#define FRAME_H
#include "types.h"
#include "spinlock.h"
#include "memlayout.h"

struct proc;

struct FrameTableEntry{
    int inUse;
    int pid;
    int reference;
    uint64 virtualAddress;
    int isPageTable;
    struct spinlock lock;
    struct proc* owner;
};

extern struct spinlock ftable_lock;
extern struct FrameTableEntry frameTable[MAX_PHYS_PAGES];

#endif