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
