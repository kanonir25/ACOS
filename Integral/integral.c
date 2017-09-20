#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <unistd.h>
#include "function.h"
#include <errno.h>

static int procesesNumber = 4;
static int M = 10000;

void debug_mistakes(int err, const char* text){
	if (err != 0){
		perror(text);
		exit(err);
	}
}

int main(int argc, char** argv) {
	setbuf(stdout, 0);
	
	void *lib = dlopen("./libfoo.so", RTLD_LAZY);
	if (lib == NULL)
	{
		perror("Can't open libfoo.so");
		return 0;

	}
	
	double (*foo)(double) = dlsym(lib, argv[1]);
	char * error = dlerror();
	
	if (error != NULL)
	{
		fprintf(stderr, "%s\n", error);
		return 0;
	}
	
	
	double Gleft = atof(argv[2]);
	double Gright = atof(argv[3]);
	if (argc >= 6){
		procesesNumber = atoi(argv[5]);
	}
	
	M = atoi(argv[4]);
	double step = (Gright-Gleft)/procesesNumber;
	double left = Gleft;
	double right = Gleft;
	
	int io[2];
	pipe(io);
	debug_mistakes(errno,"pipe failed");
	
	pid_t my_pid = fork();
	right = left + step;
	int i;
	for (i = 0; (i < procesesNumber - 1) && (my_pid != 0); i++){
		left = right;
		right = right + step;
		my_pid = fork();
		debug_mistakes(errno,"fork failed");
	}
	
	if (my_pid == 0){
		double step = (right-left) / M;
		double sum = 0;
		int i;
		double cur_left = left;
		for (i = 0; i < M; i++){
			sum += step*foo(cur_left);
			cur_left = left + i*step; 
		}
		write(io[1], &sum, sizeof(double));
		debug_mistakes(errno,"write failed");
		return 0;
	} else {
		double sum = 0;
		int i;
		for (i = 0; i < procesesNumber; i++) {
			double temp;
			read(io[0], &temp, sizeof(double));
			debug_mistakes(errno, "read failed");
			sum += temp;
		}
		printf("%f\n", sum);
		dlclose(lib);
		debug_mistakes(errno, "dlclose failed");
	}
	return 0;
}
