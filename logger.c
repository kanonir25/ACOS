#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <string.h>

#define STDOUT 1

static int N = 10;

void debug_mistakes(int err, char* msg){
	if (err != 0){
		perror(msg);
		exit(err);
	}
}

void printTime(int oldstdout) {
	time_t rawtime;
	struct tm * timeinfo;
	time ( &rawtime );
	timeinfo = localtime ( &rawtime );
	write(oldstdout, asctime(timeinfo), strlen(asctime(timeinfo)) * sizeof(char) - 1);
	write(oldstdout, " ", 1);
}

int main(int argc, char** argv) {	
	
	int oldstdout = dup(STDOUT);	
	int io[2];
	pipe(io);
	debug_mistakes(errno,"pipe failed");
	char* file = argv[1];
	int id = fork();
	debug_mistakes(errno, "fork  failed");
	if ( id ==0 ) {
		dup2(io[1],STDOUT);
		close(io[0]);
		int er = execl(file, NULL);
		if (er == -1) {
			perror("exec error occured\n");
		}
	} else {
		int i;
		close(io[1]);
		for (i = 0; i < N; ++i) {
			char c;
			char str[1000];
			int j = 0;
			do {
				read(io[0], &c, sizeof(char));
				str[j++] = c;
			} while ( c != '\n');
			printTime(oldstdout);
			write(oldstdout, str, j*sizeof(char));
		}
	}
	return 0;
}
