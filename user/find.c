#include "kernel/types.h"
#include "kernel/fcntl.h"
#include "kernel/stat.h"
#include "kernel/fs.h"
#include "user/user.h"

void find(char *path, char *fileName){
	int fd;
	struct stat st;	
	
	//文件夹无法打开
	if((fd = open(path, O_RDONLY)) < 0){
		printf("find: cannot open %s\n", path);
		return;
	}
	
	//文件无法打开
	if(fstat(fd, &st) < 0){
		printf("find: cannot stat %s\n", path);
		close(fd);
		return;
	}
	
	char buf[512], *p;	
	struct dirent de;
	static char cmp[DIRSIZ+1];
	
	for(p=path+strlen(path); p >= path && *p != '/'; p--);
	p++;
	memmove(cmp, p, strlen(p)+1);
	
	switch(st.type){	
		case T_FILE:
		//若为文件，判断是否相符
			if(strcmp(cmp, fileName) == 0){
				printf("%s\n", path);
			}			
			break;

		case T_DIR:
		//若为文件夹，判断文件夹内文件，对文件夹内文件夹递归
			if(strlen(path) + 1 + DIRSIZ + 1 > sizeof buf){
				printf("find: path too long\n");
				break;
			}
			strcpy(buf, path);
			p = buf+strlen(buf);
			*p++ = '/';
			while(read(fd, &de, sizeof(de)) == sizeof(de)){
				if(de.inum == 0 || de.inum == 1 || strcmp(de.name, ".")==0 || strcmp(de.name, "..")==0)	continue;				
				memmove(p, de.name, strlen(de.name));
				p[strlen(de.name)] = 0;
				find(buf, fileName);
			}
			break;
	}
	close(fd);	
}

int main(int argc, char *argv[]){
	if(argc < 3){
		printf("Usage:find path flieName\n");
		exit();
	}
	find(argv[1], argv[2]);
	exit();
}