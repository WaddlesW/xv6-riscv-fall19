#include "kernel/types.h"
#include "user/user.h"
#include "kernel/fcntl.h"
 
#define R 0
#define W 1
#define MAXARGS 32
#define MAXCHAR 128

char args[MAXARGS][MAXCHAR];
 
int getcmd(char *buf, int nbuf)
{
	printf("@ ");
    memset(buf, 0, nbuf);
    gets(buf, nbuf);
    (buf[0] == 0) ? (return -1:return 0);

}

void setargs(char *cmd, char* argv[],int* argc)
{
	int i = 0;
    for(;i<MAXARGS;i++){
        argv[i]=&args[i][0];
    }
    for (int j=0, i=0; cmd[j] != '\n' && cmd[j] != '\0'; j++)
    {
        while (strchr(" \t\r\n\v", cmd[j])){
            j++;
        }
        argv[i++]=cmd+j;
        while (strchr(" \t\r\n\v", cmd[j])==0){
            j++;
        }
        cmd[j]='\0';
    }
    argv[i]=0;
    *argc=i;
}

void execPipe(char*argv[],int argc); 
void runcmd(char*argv[],int argc)
{
    for(int i=1;i<argc;i++){
        if(!strcmp(argv[i],"|")){
            execPipe(argv,argc);
        }
    }
    for(int i=1;i<argc;i++){
        if(!strcmp(argv[i],">")){
            close(1);
            open(argv[i+1],O_CREATE|O_WRONLY);
            argv[i]=0;
        }
        if(!strcmp(argv[i],"<")){
            close(0);
            open(argv[i+1],O_RDONLY);
            argv[i]=0;
        }
    }
    exec(argv[0], argv);
}
 
void execPipe(char*argv[],int argc){
    int i=0;
	int fd[2];
    pipe(fd);
	
	while(i<argc){
		i++;
		if(!strcmp(argv[i],"|")){
            argv[i]=0;
            break;
        }
	}

    if(fork()==0){
        close(W);
        dup(fd[W]);
        close(fd[R]);
        close(fd[W]);
        runcmd(argv,i);
    }else{
        close(R);
        dup(fd[R]);
        close(fd[R]);
        close(fd[W]);
        runcmd(argv+i+1,argc-i-1);
    }
}

int main()
{
    char buf[MAXCHAR];
    while (getcmd(buf, sizeof(buf)) >= 0)
    {
        if (fork() == 0)
        {
            char* argv[MAXARGS];
            int argc=-1;
            setargs(buf, argv,&argc);
            runcmd(argv,argc);
        }
        wait(0);
    }
    exit(0);
}