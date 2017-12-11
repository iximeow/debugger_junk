#include <sys/ptrace.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <signal.h>

int main(int argc, char** argv) {
  printf("hello!\n");
  printf("my pid: %d\n", getpid());
  char junk[16];
  scanf("%s", junk);

  int i = 0;
  while (1) {
    printf("Iteration %x\n", i++);
  }
}
