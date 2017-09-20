#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define BUFFER_SIZE 100

char * cur_str, *buffer;
char * str;
int cur_length = 0;
int str_len;
int cur_max = 1;
int readen;

void add(char c) {
    if (cur_length == cur_max) {
        cur_max *= 2;
        cur_str = (char*)realloc(cur_str, cur_max);
        if (cur_str == NULL) {
            perror("ERROR");
            exit(1);
        }
    }
    cur_str[cur_length] = c;
    ++cur_length;
}

 int min(int a, int b) {
    if (a < b)
        return a;
    else
        return b;
}

void check_str() {
    add('\n');
    int is_prefix = 0;
    int word;
    int i, j;
    for (i = 0; i < cur_length - str_len; i++) {
        word = 1;
        for (j = 0; j < str_len; j++)
            if (cur_str[i + j] != str[j])
                word = 0;
        if (word) {
            is_prefix = 1;
            break;
        }
    }
    if (is_prefix)
        for (i = 0; i < cur_length; i += BUFFER_SIZE)
            if (write(1, cur_str + i, min(BUFFER_SIZE, cur_length - i)) == -1) {
                perror("ERROR");
                return;
            }
    cur_length = 0;
}

int main(int argc, char ** argv)
{
    if (argc < 2)
        return 0;
    buffer = (char*)malloc(sizeof(char) * BUFFER_SIZE);
    cur_str = (char*)malloc(sizeof(char) * 1);
    str = argv[1];
    str_len = strlen(str);
    while ( (readen = read(0, buffer, BUFFER_SIZE)) > 0) {
			int i;
            for (i = 0; i < readen; i++)
                if (buffer[i] != '\n')
                    add(buffer[i]);
                else {
                    check_str();
                }
    }
    if (readen == -1) {
        perror("ERROR");
        return 1;
    }
    else
        check_str();
    free(buffer);
    free(cur_str);
    return 0;
}
