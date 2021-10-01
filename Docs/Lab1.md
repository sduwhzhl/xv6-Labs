

## sleep



可以直接进行系统调用， 最开始的时候不知道要做啥， 看了下别人写的，其实就是简单的进行一下 `sleep` 的调用



```c
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char* argv[])
{
    if(argc <= 1){
        fprintf(2, "usage: sleep [integer]...\n");
        exit(1);
    }	    
    sleep(atoi(argv[1]));
    exit(0);
}

```



## pingpong



```c
/**
 *    pingpong.c 
**/
#include <kernel/types.h>
#include <user/user.h>

int main(){
	
	int pipeToChild[2];
	int pipeToParent[2];

	pipe(pipeToChild);
	pipe(pipeToParent);
	

	int pid = fork();
	char c;
	int n;
	if(pid == 0){ //child
		close(pipeToChild[1]);
		close(pipeToParent[0]);
		
		n = read(pipeToChild[0], &c, 1);
		/*
			don't call write here to make output order correct
			first ping
			then pong
			
			call it after output ping
		*/

		if(n != 1){
			fprintf(2, "child does not recevie 1 byte\n");
			exit(1);
		}
		printf("%d: received ping\n", getpid());
		write(pipeToParent[1], &c, 1);

		close(pipeToChild[0]);
		close(pipeToParent[1]);

	}else if(pid > 0){ //parent
		close(pipeToChild[0]);
		close(pipeToParent[1]);
		
		write(pipeToChild[1], &c, 1);
		n = read(pipeToParent[0], &c, 1);

		if(n != 1){
			fprintf(2, "parent does not recevie 1 byte\n");
			exit(1);
		}

		printf("%d: received pong\n", getpid());

		close(pipeToChild[1]);
		close(pipeToParent[0]);
		wait(0);
	}else{
		write(2, "fork fail", 10);
		exit(1);
	}

	exit(0);
}

```



## primes

需要递归



```c
#include "kernel/types.h"
#include "user/user.h"

void getChild(int p[]){
	close(p[1]); //read only
	int pp[2];

	int x, y;
	if(read(p[0], &x, sizeof(int))){
		fprintf(1, "prime %d\n", x);
		pipe(pp);
		
		if(fork() == 0){
			getChild(pp);	
		}else{
			close(pp[0]); //write only
			while(read(p[0], &y, sizeof(int))){
				if(y % x != 0){
					write(pp[1], &y, sizeof(int));
				}
			}
			//wait(0);
			close(p[0]);
			close(pp[1]);
			wait(0);
		}
	}
	exit(0);
}

int main(){
	
	int p[2];
	pipe(p);

	if(fork() == 0){
		//child
		getChild(p);
	}else{
		close(p[0]); //write only

		for(int i = 2;i <= 35;i++){
			write(p[1], &i, sizeof(int));
		}
		close(p[1]);
		wait(0);
	}
	//return 0;
    exit(0);
}

```





## find



`struct stat` 和 `struct dirent` 

```cpp
struct stat {
  int dev;     // File system's disk device
  uint ino;    // Inode number
  short type;  // Type of file
  short nlink; // Number of links to file
  uint64 size; // Size of file in bytes
};
```



```cpp
struct dirent {
  ushort inum;
  char name[DIRSIZ];
};
```



**什么是 inode** 

参考上篇博客



`ls.c` 源码分析

```c
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

char*
fmtname(char *path)
{
  static char buf[DIRSIZ+1];
  char *p;

  // Find first character after last slash.
  for(p=path+strlen(path); p >= path && *p != '/'; p--)
    ;
  p++;

  // Return blank-padded name.
  if(strlen(p) >= DIRSIZ)
    return p;
  memmove(buf, p, strlen(p));
  memset(buf+strlen(p), ' ', DIRSIZ-strlen(p));
  return buf;
}

void
ls(char *path)
{
  char buf[512], *p;
  int fd;
  struct dirent de;
  struct stat st;

  if((fd = open(path, 0)) < 0){ // O_RDONLY : 0 , O_WRONLY : 1 , O_RDWR : 2
    fprintf(2, "ls: cannot open %s\n", path);
    return;
  }

  if(fstat(fd, &st) < 0){
    fprintf(2, "ls: cannot stat %s\n", path);
    close(fd);
    return;
  }

  switch(st.type){
  case T_FILE:// ls file
    printf("%s %d %d %l\n", fmtname(path), st.type, st.ino, st.size);
    break;

  case T_DIR: // ls dir
    if(strlen(path) + 1 + DIRSIZ + 1 > sizeof buf){
      printf("ls: path too long\n");
      break;
    }
    strcpy(buf, path);
    p = buf+strlen(buf);
    *p++ = '/';
    while(read(fd, &de, sizeof(de)) == sizeof(de)){
      if(de.inum == 0)
        continue;
      memmove(p, de.name, DIRSIZ);
      p[DIRSIZ] = 0;
      if(stat(buf, &st) < 0){
        printf("ls: cannot stat %s\n", buf);
        continue;
      }
      printf("%s %d %d %d\n", fmtname(buf), st.type, st.ino, st.size);
    }
    break;
  }
  close(fd);
}

int
main(int argc, char *argv[])
{
  int i;

  if(argc < 2){
    ls(".");
    exit(0);
  }
  for(i=1; i<argc; i++)
    ls(argv[i]);
  exit(0);
}

```



打开文件的步骤

1. `fd = open(fileName, xxx)`   获取文件描述符
2. `fstat(fd, &st)` 获取文件的 `stat` 信息
3. 如果 `st.type == T_DIR` 是文件夹， 则遍历该文件的 `dirent` 信息 `while(read(fd, &de, sizeof(de)))` 
4. 如果 `st.type == T_FILE` 则直接进行处理



`fd` 成功打开后要记得关闭 `close(fd)` 

进行递归的时候要忽略掉 `.` 和 `..` 目录

由此改编 `find.c` 



```c
#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fs.h"
#include "user/user.h"

char* getFileName(char* path){
    char* p;
    for(p = path + strlen(path); p >= path && *p != '/';p--)
        ;
    return ++p;
}

void find(char* path, char* target)
{
    // find user ls.c
    char buf[512], *p;
    int fd;
    struct dirent de;
    struct stat st;

    if((fd = open(path, 0)) < 0){
        fprintf(2, "find: cannot open %s\n", path);
        return;
    }

    if(fstat(fd, &st) < 0){
        fprintf(2, "find: cannot stat %s\n", path);
        close(fd);
        return;
    }

    switch (st.type)
    {
    case T_FILE:
        if(strcmp(getFileName(path), target) == 0){
            printf("%s\n", path);
        }
        break;

    case T_DIR:
        if(strlen(path) + 1 + DIRSIZ + 1 > sizeof buf){
            printf("find: path too long\n");
            break;
        }
        strcpy(buf, path);
        p = buf + strlen(buf);
        *p++ = '/';
        while(read(fd, &de, sizeof(de)) == sizeof(de)){
            if(de.inum == 0)
                continue;
            if (strcmp(de.name, ".") == 0 || strcmp(de.name, "..") == 0) { // 跳过子目录的.和..
                continue;
            }
            memmove(p, de.name, DIRSIZ);
            p[DIRSIZ] = 0;
            if(stat(buf, &st) < 0){
                printf("find: cannot stat %s\n", buf);
                continue;
            }
            find(buf, target);
        }
        break;
    default:
        break;
    }
    close(fd);
}

int main(int argc, char* argv[]){
    if(argc < 3){
        // find
        fprintf(2, "find usage: find path target\n");
        exit(1);
    }
    find(argv[1], argv[2]);
    exit(0);
}
```



## xargs



linux 中的 `xargs` 命令用法介绍， [xargs 命令教程](http://www.ruanyifeng.com/blog/2019/08/xargs-tutorial.html)



`include` 的顺序会有影响， 当最后 `#include "kernel/types.h"` 的时候会导致 CE







## Lab1结束



![image-20210921151343380](https://img-bucket-zhl.oss-cn-shanghai.aliyuncs.com/image-20210921151343380.png)

拖拖拉拉做了非常久的 lab1， `Test time` 还是手动建的 `time.txt` ， 99分看的太难受了



