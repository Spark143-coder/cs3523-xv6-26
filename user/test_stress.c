#include "kernel/types.h"
#include "user/user.h"

int main() {
    printf("Starting memory stress test...\n");
    for(int i = 0; i < 31000; i++) {
        char *p = sbrk(4096); // Request one page
        if(p == (char*)-1) {
        printf("Allocation failed at iteration %d\n", i);
        break;
        }
        *p = 1;
    }
    exit(0);
}