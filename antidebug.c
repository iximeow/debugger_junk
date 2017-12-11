#include <sys/ptrace.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>

int breakpoint_me() {
  asm("nop");
  printf("did you breakpoint this function?\n");
  return 1;
}

int singlestep_me() {
  long long flags;
  asm("xor %%rax, %%rax; pushfq; pop %%rax; mov %%rax, %0" : "=r" (flags) : : "rax", "cc");
  printf("did you singlestep?\n");
  // this could also be a check like:
  // flags & 0x100 == 0x100, but all flags after xor rax, rax should come out to 0x346.
  // ... unless the direction flag was set. but that's not something i'd expect :)
  if (flags == 0x346) {
    printf("now put a breakpoint on putchar :)\n");
    // this will only be reached if we saw 
    asm("int3");
    putchar('y'); fflush(stdout);
    putchar('o'); fflush(stdout);
    putchar('u'); fflush(stdout);
    putchar(' '); fflush(stdout);
    putchar('a'); fflush(stdout);
    putchar('r'); fflush(stdout);
    putchar('e'); fflush(stdout);
    putchar(' '); fflush(stdout);
    putchar('b'); fflush(stdout);
    putchar('e'); fflush(stdout);
    putchar('i'); fflush(stdout);
    putchar('n'); fflush(stdout);
    putchar('g'); fflush(stdout);
    putchar(' '); fflush(stdout);
    putchar('w'); fflush(stdout);
    putchar('a'); fflush(stdout);
    putchar('t'); fflush(stdout);
    putchar('c'); fflush(stdout);
    putchar('h'); fflush(stdout);
    putchar('e'); fflush(stdout);
    putchar('d'); fflush(stdout);
    putchar('\n'); fflush(stdout);
  }
  return 1;
}

int check_breakpoint_on_fn() {
  int breakpointed = 0;

  unsigned char fnentry = 0xff & *((unsigned char*)breakpoint_me);
  // 0x55 being the encoding of "push rbp" - testing that the entrypoint of this function
  // is still "push rbp"
  if (fnentry != 0x55) {
    printf("[!] I see your breakpoint: `b *breakpoint_me`\n");
    breakpointed = 1;
    printf("[!] byte at breakpoint_me: %02x, but it should have been 0x55\n", fnentry);
  }

  // Also check at breakpoint_me + 4 because gdb may have breakpointed *after* `push rbp; mov rbp, rsp`
  unsigned char breakpoint_meplus4 = 0xff & *((unsigned char*)breakpoint_me + 4);
  // 0x90 being the encoding of nop - testing that this instruction is still 'nop'
  if (breakpoint_meplus4 != 0x90) {
    printf("[!] I see your breakpoint: `b breakpoint_me + 4`\n");
    breakpointed = 1;
    printf("[!] byte at breakpoint_me + 4: %02x, but it should have been 0x90\n", breakpoint_meplus4);
  }

  return breakpointed;
}

void unbreakpoint_fn() {
  printf("un-breakpointing breakpoint_me...\n");
  printf("resetting bytes for expected breakpoints.\n");
  *((char *)breakpoint_me) = 0x55;
  *((char *)breakpoint_me + 4) = 0x90;
  printf("[+] fixed obvious breakpoint locations. glhf\n");
}

int main(int argc, char** argv) {
  printf("hello!\n");
  int err = mprotect((long*)((long)breakpoint_me & ~(4095)), 0x100, PROT_READ | PROT_WRITE | PROT_EXEC);
  if (err) {
    perror("mprotect");
    exit(1);
  }
  int breakpointed = check_breakpoint_on_fn();

  if (breakpointed) unbreakpoint_fn();

  breakpoint_me();
  singlestep_me();

  printf("bye!\n");
}
