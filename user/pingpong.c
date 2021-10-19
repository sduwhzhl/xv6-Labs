#include "kernel/types.h"
#include "kernel/kernel.h"
#include "user/user.h"

#define R 0
#define W 1

int main(int argc, char* argv[])
{
  int f2s[2], s2f[2]; //father to son, son to father
  pipe(f2s); pipe(s2f);
  int pid;
  char buf[1];
  
  if((pid = fork()) < 0)
    exit(1);

  if(pid == 0){
    //son
    close(s2f[R]);
    close(f2s[W]);
    read(f2s[R], buf, 1);
    printf("%d: received ping\n", getpid());
    write(s2f[W], buf, 1);

    close(s2f[W]);
    close(f2s[R]);

    exit(0);
  }else{
    //parent
    close(s2f[W]);
    close(f2s[R]);

    write(f2s[W], buf, 1);
    read(s2f[R], buf, 1);

    printf("%d: received pong\n", getpid());

    close(s2f[R]);
    close(f2s[W]);
    exit(0);
    
  }
  

}

