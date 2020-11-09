#include "kernel/types.h"
#include "user/user.h"

#define R 0
#define W 1
 
int
main(int argc, char *argv[])
{
  int numbers[100], cnt = 0;
  int p[2];
  for (cnt = 0; cnt <= 33; cnt++) {
    numbers[cnt] = cnt + 2;
  }
  while (cnt > 0) {
    pipe(p);
    if (fork() == 0) {
		int right, left = 0;
		close(p[W]);
		cnt = -1;
		while (read(p[R], &right, sizeof(right)) != 0) {
			if (cnt == -1) {
				left = right;
				cnt = 0;
			} else {
				if (right % left != 0) numbers[cnt++] = right;
			}
		  }
		  printf("prime %d\n",left);
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
