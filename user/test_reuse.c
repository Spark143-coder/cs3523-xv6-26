#include "kernel/types.h"
#include "user/user.h"

// Note: Ensure struct vmstats is defined in your user.h
void print_vm_info(char* msg) {
    struct vmstats stats;
    if(getvmstats(getpid(), &stats) == 0) {
        printf("%s: Resident=%d, In=%d, Out=%d\n",
                msg, stats.resident_pages, stats.pages_swapped_in, stats.pages_swapped_out);
    }
}

int main() {
    int num_pages = 10500; // Adjust based on your xv6 RAM limit
    char *p = sbrk(num_pages * 4096);
    
    if(p == (char*)-1) {
        printf("sbrk failed\n");
        exit(1);
    }

    printf("1. Writing unique patterns to %d pages...\n", num_pages);
    for(int i = 0; i < num_pages; i++) {
        p[i * 4096] = (char)(i % 256);
    }
    print_vm_info("After Initial Write");

    printf("\n2. Allocating massive extra memory to force EVICTION of original pages...\n");
    // Allocate another large chunk to push the first 512 pages to swap
    sbrk(num_pages * 2 * 4096);
    char *p2 = p + (num_pages * 4096);
    for(int i = 0; i < num_pages * 2; i++) {
        p2[i * 4096] = 0xFF;
    }
    print_vm_info("After Pressure");

    printf("\n3. Accessing original pages (Reusing evicted memory)...\n");
    int errors = 0;
    for(int i = 0; i < num_pages; i++){
        if(p[i * 4096] != (char)(i % 256)){
            errors++;
        }
    }

    if(errors == 0) {
        printf("SUCCESS: All evicted data recovered correctly!\n");
    } else {
        printf("FAILURE: %d pages contained corrupted data.\n", errors);
    }
    
    print_vm_info("Final Stats");
    exit(0);
}