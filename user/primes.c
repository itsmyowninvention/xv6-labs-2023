#include <kernel/types.h>
#include <user/user.h>

const int stdin = 0;
const int stdout = 1;

int get_number(int* status)
{
  int n;
  char* buf = (char*)&n;
  *status = 1; // indicate that there is no number read

  for(int total_nbytesread = 0; total_nbytesread < sizeof(n);) {
    int nbytesread = read(stdin, buf + total_nbytesread, sizeof(n) - total_nbytesread);
    if(nbytesread <= 0) return n;
    else total_nbytesread += nbytesread;
  }

  *status = 0;
  return n;
}

void send_number(int n, int* status)
{
  *status = 1;
  int nbyteswrite = write(stdout, &n, sizeof(int));
  if(nbyteswrite < sizeof(n)) {
    return;
  }

  *status = 0;
}

void child()
{
  int read_error = 0;
  int write_error = 0;

  int p = get_number(&read_error);  
  if(read_error) return;

  int right[2];
  pipe(right);
  int pid = fork();
  if(pid > 0) {
    close(stdout);
    dup(right[1]);
    close(right[0]);
    close(right[1]);

    for(;;) {
      int n = get_number(&read_error);
      if(read_error) break;

      if(n % p) send_number(n, &write_error); // maybe prime, send to next proc
      if(write_error) break;
    }
    close(stdin);
    close(stdout);
    int wait_status;
    wait(&wait_status);
  } else if(pid == 0) {
    close(stdin);
    dup(right[0]);
    close(right[0]);
    close(right[1]);
    child();
  }
}

void parent()
{
  int write_error = 0;
  int wait_status;
  for(int i = 2; i <= 35 && write_error; i++) {
    send_number(i, &write_error);
  }
  close(stdout);
  wait(&wait_status);
}

int main(void)
{
  int p[2];
  pipe(p);
  int pid = fork();
  if(pid > 0) {
    close(stdout);
    dup(p[1]);
    close(p[0]);
    close(p[1]);
    parent();
  } else if(pid == 0) {
    close(stdin);
    dup(p[0]);
    close(p[0]);
    close(p[1]);
    child();
  } else {
    exit(1);
  }
  
  exit(0);
}
