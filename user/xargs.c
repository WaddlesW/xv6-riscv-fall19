#include "kernel/types.h"
#include "user/user.h"

int main(int argc, char *argv[]){
    int  cnt , n, m = 0;
    char block[32], buf[32];
    char *p = buf;
    char *lineSplit[32];
	
    for(cnt = 0; cnt < argc - 1; cnt++){
        lineSplit[cnt] = argv[cnt+1];
    }
    while((n = read(0, block, sizeof(block))) > 0){
        for(int i = 0; i < n; i++){
            if(block[i] == '\n'){
                buf[m] = 0;
                m = 0;
                lineSplit[cnt++] = p;
                p = buf;
                lineSplit[cnt] = 0;
                cnt = argc - 1;
                if(fork() == 0){
                    exec(argv[1], lineSplit);
                }                
                wait(0);
            }else if(block[i] == ' ') {
                buf[m++] = 0;
                lineSplit[cnt++] = p;
                p = &buf[m];
            }else {
                buf[m++] = block[i];
            }
        }
    }
    exit();
}