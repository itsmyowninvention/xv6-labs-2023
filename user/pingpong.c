#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char* argv[])
{
  int pc[2];
  int cp[2];
  char byte;
  if(pipe(pc) < 0 || pipe(cp) < 0) {
    exit(1);
  }
  int pid = fork();
  if(pid > 0) {
    close(pc[1]);
    close(cp[0]);
    write(cp[1], &byte, 1);
    read(pc[0], &byte, 1);
    printf("%d: received pong\n", getpid());
  } else if(pid == 0) {
    close(pc[0]);
    close(cp[1]);
    read(cp[0], &byte, 1);
    printf("%d: received ping\n",getpid());
    write(pc[1], &byte, 1);
  }
  exit(0);
}
