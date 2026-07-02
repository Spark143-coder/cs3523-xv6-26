#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"
#include "mlfqinfo.h"
#include "vm.h"

uint64
sys_exit(void)
{
  int n;
  argint(0, &n);
  kexit(n);
  return 0;  // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return kfork();
}

uint64
sys_wait(void)
{
  uint64 p;
  argaddr(0, &p);
  return kwait(p);
}

uint64
sys_sbrk(void)
{
  uint64 addr;
  int n;
  int t;
  struct proc *p = myproc();

  argint(0, &n);
  argint(1, &t); // This will read garbage if only 1 arg is passed

  addr = p->sz;

  // The "Dumbass-Proof" Logic:
  // Only go Eager if 't' specifically matches your EAGER constant.
  // Otherwise, if n > 0, ALWAYS go Lazy.
  if(n > 0 && t == SBRK_EAGER) {
      if(growproc(n) < 0) return -1;
  } else if(n < 0) {
      // Negative n must always be eager to free memory immediately
      if(growproc(n) < 0) return -1;
  } else {
      // This is the Lazy Path for sbrk(n) where t is garbage or 0
      if(addr + n < addr || addr + n > TRAPFRAME) return -1;
      p->sz += n;
  }

  return addr;
}

uint64
sys_pause(void)
{
  int n;
  uint ticks0;

  argint(0, &n);
  if(n < 0)
    n = 0;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(killed(myproc())){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

uint64
sys_kill(void)
{
  int pid;

  argint(0, &pid);
  return kkill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

uint64 sys_hello(){
  printf("Hello from the kernel!\n");
  return 0;
}

uint64 sys_getpid2(){
  return myproc()->pid;
}

uint64 sys_getppid(){
  int ppid=-1;
  acquire(&myproc()->lock);
  if(myproc()->parent)ppid=myproc()->parent->pid;
  release(&myproc()->lock);
  return ppid;
}

uint64 sys_getnumchild(){
  return kGetnumchild();
}

uint64 sys_getsyscount(){
  return (myproc()->syscalls);
}

uint64 sys_getchildsyscount(){
  int pid;
  argint(0,&pid);
  if(pid < 0)return -1;
  return kGetChildsyscount(pid);
}

uint64 sys_getlevel(){
  return (myproc()->queueInfo.level);
}

uint64 sys_getmlfqinfo(){
  int pid;
  argint(0,&pid);
  uint64 addr;
  argaddr(1,&addr);
  if(pid < 0)return -1;
  return ksys_getmlfqinfo(pid,addr);
}

uint64 sys_getvmstats(void){
  int pid;
  uint64 addr;
  argint(0,&pid);
  argaddr(1,&addr);
  if(pid < 0)return -1;
  return ksys_getvmstats(pid,addr);
}