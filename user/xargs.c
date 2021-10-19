#include "kernel/types.h"
#include "kernel/fs.h"
#include "kernel/param.h"
#include "user/user.h"


//MAXARG 32 
int main(int argc, char* argv[]){
    //xargs command args...
    if(argc - 1 > MAXARG){
        fprintf(2, "xargs: too many args\n");
        exit(1);
    }

    char* args[MAXARG];
    
    char* command = argv[1];
    int idx = 0;
    for(int i = 1;i < argc;i++){
        args[idx++] = argv[i];
    }
    char buf[512];
	while(1){
        char* ch = buf;
        int len;
        int argIdx = idx;
        while((len = read(0, ch, 1)) > 0 && *ch != '\n') ch++;
        
        if(len == 0)break;

        *ch = 0;
        args[argIdx++] = buf;

        if(fork() == 0){
            exec(command, args);
            exit(0);
        }else{
            wait(0);
        }

    }
    exit(0);
}
