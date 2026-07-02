#include "kernel/types.h"
#include "user/user.h"

int main() {
    struct vmstats stats;
    int pid = getpid();
    uint64 size = 31000 * 4096; // 100 pages

    printf("Test 1: Lazy Allocation\n");
    sbrk(size);
    getvmstats(pid, &stats);
    printf("After sbrk: Resident=%d, Faults=%d\n", stats.resident_pages, stats.page_faults);

    char *p = sbrk(0) - size;
    for(int i = 0; i < 31000; i++) {
        p[i * 4096] = 'a'; // Trigger fault
    }

    getvmstats(pid, &stats);
    printf("After touching 100 pages: Resident=%d, Faults=%d\n", stats.resident_pages, stats.page_faults);
    exit(0);
}