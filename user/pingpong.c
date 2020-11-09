#include "kernel/types.h"
#include "user/user.h"

#define R 0
#define W 1
int
main(int argc, char *argv[])
{
	int parent_p[2], child_p[2];
	pipe(child_p);
	pipe(parent_p);
	char buf[10];
	
	if (fork() == 0) {
		close(parent_p[W]);
		close(child_p[R]);
		read(parent_p[R], &buf, 4);
		printf("%d: received %s\n", getpid(), buf);
		write(child_p[W], "pong", 4);
		close(parent_p[R]);
		close(child_p[W]);
	} 
	else {
		close(parent_p[R]);
		close(child_p[W]);
		write(parent_p[W], "ping", 4);
		read(child_p[R], &buf, 4);
		printf("%d: received %s\n", getpid(), buf);
		close(parent_p[W]);
		close(child_p[R]);
		wait();
	}
	
	exit();
}
