#include "kernel/types.h"
#include "user/user.h"

#define PAGES 31500 // Set this to be > your MAX_PHYS_PAGES

int main() {
    struct vmstats stats;

    printf("Before Sbrk\n");
    char *p = sbrk(PAGES * 4096);
    
    printf("Test 2 & 3: Swapping and Persistence\n");

    // Write unique patterns
    for(int i = 0; i < PAGES; i++) {
        p[i * 4096] = (char)(i % 255);
    }

    getvmstats(getpid(), &stats);
    printf("\n\nStats: Evicted=%d, Swapped Out=%d\n\n", stats.pages_evicted, stats.pages_swapped_out);

    // Read back and verify
    for(int i = 0; i < PAGES; i++) {
        if(p[i * 4096] != (char)(i % 255)) {
        printf("DATA CORRUPTION at page %d!\n", i);
        exit(1);
        }
    }

    getvmstats(getpid(), &stats);
    printf("\nFinal Stats: Swapped In=%d, Resident=%d\n", stats.pages_swapped_in, stats.resident_pages);
    printf("Persistence Test Passed!\n");
    exit(0);
}