#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"

char* path_to_filename(char* path)
{
  char *p;
  // Find first character after last slash.
  for(p = path + strlen(path); p >= path && *p != '/'; p--)
    ;
  p++;

  // Return the filename
  return p;
}

char* pathcat(char* path, char* filename)
{
  strcat(path, "/");
  strcat(path, filename);
  return path;
}

void find(char* path, char* target, int search_current_path)
{
  int fd = -1;
  char buf[512]  = { 0 };
  char *filename = path_to_filename(path);
  struct dirent de = { 0 };
  struct stat st = { 0 };

  if(strcmp(filename, "..") == 0)
    return;

  if(strcmp(filename, ".") == 0 && !search_current_path)
    return;

  if(strcmp(filename, target) == 0)
    printf("%s\n", path);

  if((fd = open(path, O_RDONLY)) < 0) {
    fprintf(2, "find: can not open %s\n", path);
    return;
  }
  
  if(fstat(fd, &st) < 0) {
    fprintf(2, "find: can not stat %s\n", path);
    close(fd);
    return;
  }

  if(st.type != T_DIR) {
    close(fd);
    return;
  }

  if(strlen(path) + 1 + DIRSIZ + 1 > sizeof(buf)) {
    fprintf(2, "find: path too long\n");
    close(fd);
    return;
  }
  
  while(read(fd, &de, sizeof(de)) == sizeof(de)) {
    if(de.inum == 0)
      continue;
    strcpy(buf, path);
    pathcat(buf, de.name);
    find(buf, target, 0);
  }

  close(fd);
}

int main(int argc, char* argv[])
{
  if(argc < 3) {
    printf("usage: find <path> <target>\n");
    exit(0);
  }
  
  find(argv[1], argv[2], 1);
  exit(0);
}
