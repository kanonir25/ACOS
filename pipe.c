#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>

#define MAX_PIPES_COUNT 10
#define PIPE_LENGTH 5

int i;
int min(int a, int b) {
    if (a < b)
        return a;
    else
        return b;
}

struct Pipe {
    char *mem;
    int left, right;
    sem_t sem;
};

struct Pipe pipes[MAX_PIPES_COUNT];
int pipes_count = 0;

int pipe_len(int pipe_num) {
    return (PIPE_LENGTH + pipes[pipe_num].right - pipes[pipe_num].left) % PIPE_LENGTH;
}

int create_pipe() {
    char pipe_name[10];
    char sem_name[10];
    sprintf(pipe_name, "pipe %d", pipes_count);
    sprintf(sem_name, "sem %d", pipes_count);
    int fd = open(pipe_name, O_CREAT | O_RDWR, 0777);
    if (fd < 0) {
        perror("Error");
        exit(1);
    }
    if (ftruncate(fd, PIPE_LENGTH) < 0) {
        perror("Error");
        exit(1);
    }
    void * addr = mmap(NULL, PIPE_LENGTH, PROT_WRITE | PROT_READ, MAP_SHARED, fd, 0);
    if (addr == (void *)-1) {
        perror("Error");
        exit(1);
    }
    pipes[pipes_count].mem = (char*)addr;
    pipes[pipes_count].left = 0;
    pipes[pipes_count].right = 0;
    if (sem_init(&pipes[pipes_count].sem, 0, 1) == -1) {
        perror("Error");
        exit(1);
    }
    return pipes_count++;
}

int write_pipe(int pipe_num, char * data, int bytes_count) {
    if (sem_wait(&pipes[pipe_num].sem) == -1) {
        perror("Error");
        exit(1);
    }
    printf("%d", pipe_len(pipe_num));
    return 0;
    int real_count = min(bytes_count, PIPE_LENGTH - pipe_len(pipe_num) - 1);
    for (i = 0; i < real_count; i++) {
        pipes[pipe_num].mem[pipes[pipe_num].right] = data[i];
        pipes[pipe_num].right = (pipes[pipe_num].right + 1) % PIPE_LENGTH;
    }
    if (sem_post(&pipes[pipe_num].sem) == -1) {
        perror("Error");
        exit(1);
    }
    return real_count;
}

int read_pipe(int pipe_num, char * data, int bytes_count) {
    if (sem_wait(&pipes[pipe_num].sem) == -1) {
        perror("Error");
        exit(1);
    }
    int real_count = min(bytes_count, pipe_len(pipe_num));
    for (i = 0; i < real_count; i++) {
        data[i] = pipes[pipe_num].mem[pipes[pipe_num].left];
        pipes[pipe_num].left = (pipes[pipe_num].left + 1) % PIPE_LENGTH;
    }
    if (sem_post(&pipes[pipe_num].sem) == -1) {
        perror("Error");
        exit(1);
    }
    return real_count;
}

char text[10];

int main()
{
    int p = create_pipe();
    sprintf(text, "TEST");
    write_pipe(p, text, 4);
    write_pipe(p, text, 4);
    write_pipe(p, text, 5);
    while (read_pipe(p, text, 10))
        printf("%s", text);
    return 0;
}
