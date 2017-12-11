#include <sys/ptrace.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>

void peek(int fd);
void poke(int fd);

int prompt(char* message) {
  printf("%s (Y/n): ", message);
  char answer[16];
  scanf("%s", answer);
  return answer[0] != 'n';
}

int main(int argc, char** argv) {
  puts("PID of target process: ");
  char mem_path[32];
  int pid;
  scanf("%d", &pid);
  snprintf(mem_path, 32, "/proc/%d/mem", pid);
  int fd = open(mem_path, O_CREAT | O_RDWR);
  if (fd < 0) {
    perror("open");
    exit(1);
  }
  unsigned int addr = 0x4005f6;
  int err = lseek(fd, addr, SEEK_SET);
  if (err < 0) {
    perror("lseek");
    exit(1);
  }
  char buf[0x1000];
  int read_size = read(fd, buf, 32);
  if (read_size < 0) {
    perror("read");
    exit(1);
  }

  printf("0x%x: ", addr);
  for (int i = 0; i < 32; i++) {
    printf("%02x", 0xff & buf[i]);
  }
  puts("");

  while (1) {
    if (prompt("poke?")) {
      poke(fd);
    } else if (prompt("peek?")) {
      peek(fd);
    } else if (!prompt("again?")) {
      exit(0);
    }
  }
}

void peek(int fd) {
  printf("Address? ");
  unsigned int address;
  scanf("%x", &address);

  int err = lseek(fd, address, SEEK_SET);
  if (err < 0) { perror("lseek"); return; }

  char value;
  err = read(fd, &value, 1);
  if (err < 0) { perror("read"); puts("Read failed..."); return; }

  printf("0x%08x: %02hhx\n", address, value);
}

void poke(int fd) {
  printf("Address? ");
  unsigned int address;
  scanf("%x", &address);

  printf("Value? ");
  char value;
  scanf("%hhx", &value);

  printf("Going to write 0x%hhx to 0x%x. ", value, address);
  if (!prompt("Ok?")) return;

  int err = lseek(fd, address, SEEK_SET);
  if (err < 0) { perror("lseek"); return; }

  err = write(fd, &value, 1);
  if (err < 0) { perror("write"); puts("Write failed..."); return; }

  lseek(fd, address, SEEK_SET);
  unsigned char rd_buf[4];
  int read_size = read(fd, rd_buf, 4);
  if (read_size < 0) { perror("read-after-write"); exit(0); }

  printf("0x%x: ", address);
  for (int i = 0; i < 4; i++) {
    printf("%02hhx", rd_buf[i]);
  }
  puts("");
}
