#include "kernel/types.h"
#include "user/user.h"

#define R 0
#define W 1
 
int
main(int argc, char *argv[])
{
  int numbers[100], cnt = 0;
  int p[2];
  
  //number数组存储2-35
  for (cnt = 0; cnt <= 33; cnt++) {
    numbers[cnt] = cnt + 2;
  }
  
  while (cnt > 0) {
    pipe(p);	//创建一个管道
	
	//子进程
    if (fork() == 0) {
		int buf, temp = 0;
		close(p[W]);	//关闭上一写端
		cnt = -1;
		while (read(p[R], &buf, sizeof(buf)) != 0) {
			if (cnt == -1) {
				temp = buf;
				cnt = 0;
			} else {
				if (buf % temp != 0) numbers[cnt++] = buf;
			}
		  }
		  printf("prime %d\n",temp);
		  close(p[R]);
		} else {
		  close(p[R]);
		  for (int i = 0; i < cnt; i++) {
			write(p[W], &numbers[i], sizeof(numbers[0]));
		  }
		  close(p[W]);
		  wait();
		  break;
    }
  }
  exit();
}
