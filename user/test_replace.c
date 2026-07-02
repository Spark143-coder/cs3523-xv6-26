#include "kernel/types.h"
#include "user/user.h"

// Assuming PGSIZE is 4096
#define PGSIZE 4096

void print_eviction_stats(int pid, char* phase) {
    struct vmstats stats;
    if (getvmstats(pid, &stats) == 0) {
        printf("[%s] Resident: %d, Evicted: %d, Swapped Out: %d\n",
            phase, stats.resident_pages, stats.pages_evicted, stats.pages_swapped_out);
    }
}

int main() {
    int pid = getpid();
    
    // 1. Determine how many pages to allocate.
    // If your xv6 RAM is 128MB, you need > 32768 pages.
    // If it's 8MB (common in labs), you need > 2048 pages.
    int num_pages = 31500;
    uint64 size = (uint64)num_pages * PGSIZE;

    printf("Step 1: Initial State\n");
    print_eviction_stats(pid, "START");

    // 2. Allocate memory
    char *p = sbrk(size);
    if(p == (char*)-1) {
        printf("sbrk failed\n");
        exit(1);
    }

    // 3. Touch pages to fill physical RAM
    // As you loop, eventually the kernel will run out of free frames
    // and the EVICT: logs should start appearing in your console.
    printf("Step 2: Touching %d pages to force replacement...\n", num_pages);
    for(int i = 0; i < num_pages; i++) {
        p[i * PGSIZE] = (char)(i % 256);
        
        // Print progress every 500 pages to see the "Resident" count stall
        if(i > 0 && i % 500 == 0) {
            print_eviction_stats(pid, "PROGRESS");
        }
    }

    printf("Step 3: Final State after forcing replacement\n");
    print_eviction_stats(pid, "END");

    // 4. Verification logic
    struct vmstats final_stats;
    getvmstats(pid, &final_stats);
    if(final_stats.pages_evicted > 0) {
        printf("\nSUCCESS: Page replacement was triggered (%d evictions).\n",
            final_stats.pages_evicted);
    } else {
        printf("\nFAILURE: No pages were evicted. Increase num_pages.\n");
    }

    exit(0);
}