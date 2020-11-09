#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"


void find(char* path, char* file) {
  int fd;
  struct dirent de;
  struct stat st;
  
  if((fd = open(path, 0)) < 0){
    fprintf(2, "find: cannot open %s\n", path);
    return;
  }

  if(fstat(fd, &st) < 0){
    fprintf(2, "ls: cannot stat %s\n", path);
    close(fd);
    return;
  }

  switch (st.type) {
    case T_DIR:
    while(read(fd, &de, sizeof(de)) == sizeof(de)) {
      if (de.inum == 0) continue;
      if (strcmp(de.name, ".") == 0 || strcmp(de.name, "..") == 0) continue;
      find(path, de.name, file, 0);
    }
    break;
    case T_FILE:
    if (strcmp(file, next) == 0) {
      printf("%s\n", path);
    }
    break;
  }

  close(fd);
}

int
main(int argc, char *argv[])
{
  if(argc != 3){
    fprintf(2, "Usage: find path filename\n");
    exit();
  }


  find(argv[1], argv[2]);

  exit();
}
