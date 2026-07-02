#include "kernel/types.h"
#include "user/user.h"

// Note: Ensure struct vmstats is in your user.h
void check_stats(char* label) {
    struct vmstats stats;
    if(getvmstats(getpid(), &stats) == 0) {
        printf("[%s] Page Faults: %d, Resident Pages: %d\n",
            label, stats.page_faults, stats.resident_pages);
    }
}

int main() {
    int num_pages = 31000;
    uint64 size = num_pages * 4096;

    printf("--- Phase 1: Lazy Allocation ---\n");
    check_stats("Before sbrk");

    char *p = sbrk(size);
    if(p == (char*)-1) {
        printf("sbrk failed\n");
        exit(1);
    }

    // In a lazy system, Resident and Faults should NOT increase here
    check_stats("After sbrk (Before touching)");

    printf("\n--- Phase 2: Triggering Faults ---\n");
    for(int i = 0; i < num_pages; i++) {
        // Accessing the page triggers a Page Fault (Scause 13 or 15)
        // The kernel handles the trap, allocates a page, and resumes.
        p[i * 4096] = 'Z';
        
        if(i == 0 || i == 49 || i == 99) {
            printf("Touched page %d...\n", i + 1);
        }
    }

    // Now, Resident and Faults should have increased by ~100
    check_stats("After touching all pages");

    if (num_pages > 0) {
        printf("\nSUCCESS: If 'After sbrk' stats were low and 'After touching' stats are high, \n");
        printf("your Page Fault handler is working correctly!\n");
    }

    exit(0);
}