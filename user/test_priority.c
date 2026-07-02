#include "kernel/types.h"
#include "user/user.h"
#include "kernel/mlfqinfo.h"

// Ensure these structures match your kernel definitions exactly

void print_status(int pid, char* name) {
    struct vmstats v;
    struct mlfqinfo m;
    if (getvmstats(pid, &v) == 0 && getmlfqinfo(pid, &m) == 0) {
        printf("[%s PID %d] MLFQ Level: %d | Resident: %d | Evicted: %d | Syscalls: %d\n",
            name, pid, m.level, v.resident_pages, v.pages_evicted, m.total_syscalls);
    }
}

int main() {
    int parent_pid = getpid();
    printf("Step 1: Parent allocating baseline memory...\n");
    char *p_mem = sbrk(10000 * 4096);
    for(int i=0; i<10000; i++) p_mem[i*4096] = 'P';

    int child_pid = fork();

    if (child_pid == 0) {
        // --- CHILD PROCESS (LOW PRIORITY) ---
        int my_pid = getpid();
        printf("Child (PID %d): Allocating memory then going CPU-bound...\n", my_pid);
        char *c_mem = sbrk(1000 * 4096);
        for(int i=0; i<1000; i++) c_mem[i*4096] = 'C';

        // Burn CPU cycles to sink to lowest MLFQ level
        // We do this for a long time to ensure the scheduler demotes us
        int temp=0;
        for(volatile int i = 0; i < 50000000; i++) {
            for(volatile int j=0;j<1000;j++)temp*=1;
            if(i % 500000 == 0) temp++; // Yield occasionally to let Parent run
        }
        printf("Temp: %d\n",temp);
        printf("Child: Should be lowest priority now. Waiting for eviction...\n");
        while(1) {temp /= 1;}
        printf("Temp: %d\n",temp);
        exit(0);
    }

    // --- PARENT PROCESS (HIGH PRIORITY) ---
    // Give child time to sink in priority
    pause(500);

    struct mlfqinfo m;
    getmlfqinfo(child_pid,&m);
    printf("The level of child process: %d",m.level);

    printf("\nStep 2: Parent boosting priority via syscalls and forcing eviction...\n");
    for (int i = 0; i < 10600; i++) {
        // Spam syscalls to stay at MLFQ Level 0
        for(int k = 0; k < 100; k++) {
            getpid();
        }

        // Allocate new pages to trigger eviction
        char *extra = sbrk(4096);
        if(extra != (char*)-1) *extra = 'V';

        if (i % 500 == 0) {
            printf("\n--- Mid-Test Snapshot ---\n");
            print_status(parent_pid, "PARENT");
            print_status(child_pid, "CHILD");
        }
    }

    printf("\nStep 3: Final Analysis\n");
    print_status(parent_pid, "PARENT");
    print_status(child_pid, "CHILD");

    kill(child_pid);
    exit(0);
}