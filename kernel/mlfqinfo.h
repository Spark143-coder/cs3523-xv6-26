#ifndef MLFQINFO_H
#define MLFQINFO_H

struct mlfqinfo {
    int level;            // current queue level
    int ticks[4];         // total ticks consumed at each level
    int times_scheduled;  // number of times the process has been scheduled
    int total_syscalls;   // total system calls made (from PA1);
    int ticks_in_slice;
    int syscalls_at_start;
};


#endif