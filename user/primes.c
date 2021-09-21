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
