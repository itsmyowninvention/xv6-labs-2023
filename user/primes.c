#include <kernel/types.h>
#include <user/user.h>

int get_number(int reader, int* status)
{
  int n;
  char* buf = (char*)&n;
  *status = 1; // indicate that there is no number read

  for(int total_nbytesread = 0; total_nbytesread < sizeof(n);) {
    int nbytesread = read(reader, buf + total_nbytesread, sizeof(n) - total_nbytesread);
    if(nbytesread <= 0) return n;
    else total_nbytesread += nbytesread;
  }

  *status = 0;
  return n;
}

void send_number(int writer, int n, int* status)
{
  *status = 1;
  int nbyteswrite = write(writer, &n, sizeof(int));
  if(nbyteswrite < sizeof(n)) {
    return;
  }

  *status = 0;
}

void child(int reader)
{
  int read_error = 0;
  int write_error = 0;

  int p = get_number(reader, &read_error);  
  if(read_error) return;
  printf("prime %d\n", p);

  int right_pipe[2];
  pipe(right_pipe);
  int pid = fork();
  if(pid > 0) {
    close(right_pipe[0]);
    int writer = right_pipe[1];
    for(;;) {
      int n = get_number(reader, &read_error);
      if(read_error) break;

      if(n % p) send_number(writer, n, &write_error); // maybe prime, send to next proc
      if(write_error) break;
    }
    close(reader);
    close(writer);
    int wait_status;
    wait(&wait_status);
  } else if(pid == 0) {
    close(right_pipe[1]);
    child(right_pipe[0]);
  }
}

void parent(int writer)
{
  int write_error = 0;
  int wait_status;
  for(int i = 2; i <= 35 && !write_error; i++)
    send_number(writer, i, &write_error);
  close(writer);
  wait(&wait_status);
}

int main(void)
{
  int p[2];
  pipe(p);
  int pid = fork();
  if(pid > 0) {
    close(p[0]);
    parent(p[1]);
  } else if(pid == 0) {
    close(p[1]);
    child(p[0]);
  } else {
    exit(1);
  }
  
  exit(0);
}
