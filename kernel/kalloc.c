// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"
#include "frame.h"
#include "swap.h"

struct swap_slot swap_space[SWAP_SIZE];
struct spinlock swap_lock;

struct spinlock ftable_lock;
struct FrameTableEntry frameTable[MAX_PHYS_PAGES];

void freerange(void *pa_start, void *pa_end);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  struct run *freelist;
} kmem;

void swapinit(void){
  initlock(&swap_lock,"swap");
  for(int i=0;i<SWAP_SIZE;i++)swap_space[i].available=1;
}

void
kinit(){

  swapinit();
  initlock(&ftable_lock,"ftable");
  for(int i = 0;i < MAX_PHYS_PAGES;i++){
    memset(&frameTable[i],0,sizeof(struct FrameTableEntry));
    initlock(&frameTable[i].lock,"frame_entry");
  }
  initlock(&kmem.lock,"kmem");
  freerange(end, (void*)(KERNBASE + (MAX_PHYS_PAGES * PGSIZE)));

  // swapinit();
  // initlock(&ftable_lock, "ftable");

  // for(int i = 0; i < MAX_PHYS_PAGES; i++){
  //   memset(&frameTable[i], 0, sizeof(struct FrameTableEntry));
  //   initlock(&frameTable[i].lock, "frame_entry");
  // }

  // initlock(&kmem.lock, "kmem");
  // uint64 fake_limit = (uint64)end + (1024 * PGSIZE);

  // freerange(end, (void*)fake_limit);
}

void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE)
    kfree(p);
}

// Free the page of physical memory pointed at by pa,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa)
{
  struct run *r;

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  // Fill with junk to catch dangling refs.
  memset(pa, 1, PGSIZE);

  r = (struct run*)pa;

  //Update the frametable
  int idx = ((uint64)pa - KERNBASE)/PGSIZE;
  acquire(&frameTable[idx].lock);
  frameTable[idx].inUse=0;
  frameTable[idx].pid=0;
  frameTable[idx].reference=0;
  frameTable[idx].virtualAddress=0;
  frameTable[idx].owner=0;
  frameTable[idx].isPageTable=0;
  release(&frameTable[idx].lock);

  acquire(&kmem.lock);
  r->next = kmem.freelist;
  kmem.freelist = r;
  release(&kmem.lock);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  struct run *r;

  acquire(&kmem.lock);
  r = kmem.freelist;
  if(r)
    kmem.freelist = r->next;
  release(&kmem.lock);

  if(r == 0){
    int victim_idx = clock();
    if(evict_page(victim_idx)==0){
      r = (struct run*)(KERNBASE + (victim_idx*PGSIZE));
    }
    else{
        printf("KALLOC: Eviction FAILED!\n");
        return 0;
    }
  }

  if(r){
    memset((char*)r, 5, PGSIZE); // fill with junk
    //Mark the frame's inUse bit
    int idx = ((uint64)r-KERNBASE)/(PGSIZE);
    acquire(&frameTable[idx].lock);
    frameTable[idx].inUse=1;
    frameTable[idx].pid=0;
    frameTable[idx].virtualAddress=0;
    frameTable[idx].reference=1;
    frameTable[idx].owner=0;
    frameTable[idx].isPageTable=0;
    release(&frameTable[idx].lock);
  }

  return (void*)r;
}
