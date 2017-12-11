#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/ptrace.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/user.h>
#include <sys/wait.h>
#include <unistd.h>
#include <inttypes.h>

int singlestep(pid_t target_pid, int target_mem_fd) {
  struct user_regs_struct target_regs;
  ptrace(PTRACE_GETREGS, target_pid, NULL, &target_regs);

  long long unsigned int rip = target_regs.rip;

  printf("16 bytes at rip (%016llx): \n", rip);

  long long err = lseek(target_mem_fd, rip, SEEK_SET);
  if (err < 0) {
    perror("lseek");
    exit(1);
  }
  char buf[0x10];
  int read_size = read(target_mem_fd, buf, 16);
  if (read_size < 0) {
    perror("read");
    printf("\nDetaching...\n");
    ptrace(PTRACE_DETACH, target_pid, NULL, NULL);
    exit(1);
  }

  for (int i = 0; i < 16; i++) {
    printf("%02x", 0xff & buf[i]);
  }
  puts("");
  printf("rax: %016llx\n", target_regs.rax);
  printf("rbx: %016llx\n", target_regs.rbx);
  printf("rcx: %016llx\n", target_regs.rcx);
  printf("rdx: %016llx\n", target_regs.rdx);
  printf("rsp: %016llx\n", target_regs.rsp);
  printf("rbp: %016llx\n", target_regs.rbp);
  printf("rsi: %016llx\n", target_regs.rsi);
  printf("rdi: %016llx\n", target_regs.rdi);
  printf("rflags: %016llx\n", target_regs.eflags);
  printf("and now stepping...\n\n");

  ptrace(PTRACE_SINGLESTEP, target_pid, 0, NULL);
  int status;
  pid_t waitpid_res = waitpid(target_pid, &status, 0);

  if (waitpid_res == -1) {
    perror("waitpid");
    exit(1);
  }
}

int main(int argc, char** argv) {
  printf("hello!\n");
  printf("my pid: %d\n", getpid());

  pid_t target_pid;
  puts("PID of process to debug: ");
  scanf("%d", &target_pid);
  char mem_path[32];
  snprintf(mem_path, 32, "/proc/%d/mem", target_pid);
  int fd = open(mem_path, O_CREAT);
  if (fd < 0) {
    perror("open");
    exit(1);
  }

  // Gonna use ptrace to single step and get registers in the target process
  // so we'll have to attach
  int res = ptrace(PTRACE_ATTACH, target_pid, NULL, NULL);
  if (res) {
    perror("ptrace_attach");
    exit(1);
  }

  printf("Waiting for %d to stop...\n", target_pid);
  int status;
  pid_t waitpid_res = waitpid(target_pid, &status, 0);

  if (waitpid_res == -1) {
    perror("waitpid");
    exit(1);
  }

  // Step 100 instructions and show registers
  for (int i = 0; i < 100; i++) {
    printf("Stepping #%d\n", i);
    singlestep(target_pid, fd);
  }

  // now detach, let the process go be free!
  ptrace(PTRACE_DETACH, target_pid, NULL, NULL);
}
