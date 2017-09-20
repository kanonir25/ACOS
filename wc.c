#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>

#define NUMBERS_COUNT 100000
#define MAXIMUM_NUMBER 10
#define THREADS_COUNT 5

void * addr;
pthread_mutex_t mutex;
size_t file_size;
int * answer;

void thread_func(void * arg) {
    /**/
    int thread_index = *(int*)arg;
    char * char_num = malloc(5);
    if (char_num == NULL) {
        perror("Error");
        exit(1);
    }
    char * char_addr = (char*)addr;
    int num, cur_len = 0;
    int len = file_size / THREADS_COUNT;
    int left_side = thread_index * len;
    int right_side;
    if (thread_index == THREADS_COUNT - 1)
        right_side = file_size;
    else
        right_side = left_side + len;
    while (left_side < file_size) {
        if (char_addr[left_side] == ' ' && cur_len != 0) {
            num = atoi(char_num);
            pthread_mutex_lock(&mutex);
            answer[num]++;
            pthread_mutex_unlock(&mutex);
            cur_len = 0;
        }
        if (cur_len == 0 && left_side >= right_side)
            break;
        if (char_addr[left_side] != ' ')
            char_num[cur_len++] = char_addr[left_side];
        left_side++;
    }

    free(char_num);
    pthread_exit((void *)answer);
	free(char_addr);
}

struct numbers {
    int a, b;
};

struct numbers order_numbers[MAXIMUM_NUMBER];

int comp(void * _left, void * _right) {
    int left = ((struct numbers *)_left)->a;
    int right = ((struct numbers *)_right)->a;
    if (left < right)
        return 1;
    if (left > right)
        return -1;
    return 0;
}

int main()
{
    FILE * f = fopen("input.txt", "w");
    if (f == NULL) {
        perror("Error");
        return 1;
    }
    
    int i, j;
	srand(time(0));
    for (i = 0; i < NUMBERS_COUNT; i++)
        fprintf(f, "%d ", rand() % MAXIMUM_NUMBER);
    fprintf(f, "\n");
    fclose(f);

    int fd = open("input.txt", O_RDWR, 0777);
    if (fd == -1) {
        perror("Error");
        return 1;
    }
    struct stat file_stat;
    if (stat("input.txt", &file_stat) == -1) {
        perror("Error");
        return 1;
    }
    file_size = file_stat.st_size;
   // int answer[MAXIMUM_NUMBER];
    answer = (int *)malloc(MAXIMUM_NUMBER * sizeof(int));
    if (answer == NULL) {
        perror("Error");
        exit(1);
    }
    for (i = 0; i < MAXIMUM_NUMBER; i++)
        answer[i] = 0;

    addr = mmap(NULL, file_size, PROT_READ, MAP_PRIVATE,fd, 0);
    if (addr == (void*)-1) {
        perror("Error");
        return 1;
    }
    pthread_t threads[THREADS_COUNT];
    int thread_args[THREADS_COUNT];
    for (i = 0; i < THREADS_COUNT; i++) {
        thread_args[i] = i;
        if (pthread_create(&threads[i], NULL, (void *)thread_func, (void *)&thread_args[i])) {
            perror("Error");
            return 1;
        }
    }
    for (i = 0; i < THREADS_COUNT; i++) {
        int * thread_result;
        if (pthread_join(threads[i], (void *)&thread_result)) {
            perror("Error");
            return 1;
        }
     /*  for (j = 0; j < MAXIMUM_NUMBER; j++)
            answer[j] += thread_result[j];
        free(thread_result);*/
    }
    for (i = 0; i < MAXIMUM_NUMBER; i++) {
        order_numbers[i].a = answer[i];
        order_numbers[i].b = i;
    }
	int sum = 0;
    qsort(order_numbers, MAXIMUM_NUMBER, sizeof(struct numbers), comp);
    for (i = 0; i < 10; i++)
        if (order_numbers[i].a > 0){
             printf("%d: count[%d] = %d\n", i + 1, order_numbers[i].b, order_numbers[i].a);
             sum += order_numbers[i].a;
		 }
	printf("%d\n", sum);
    munmap(addr, file_size);
    close(fd);
    free(answer);
    return 0;
}
