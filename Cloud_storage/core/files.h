

#ifndef OUR_CLOUD_FILES_H
#define OUR_CLOUD_FILES_H


#include <sys/stat.h>
#include <assert.h>
#include <memory.h>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include "storage.h"

#define DIR_LIST_MAX_BUF_SIZE 16384
#define FULL_PATH_MAX_SIZE 1024

int is_dir(char* filename, char* path) {
    char full_path[1024];
    sprintf(full_path, "%s/%s", path, filename);

    struct stat item_info;
    int result = lstat(full_path, &item_info);
    assert(result != -1);

    return S_ISDIR(item_info.st_mode);
}

// Возвращает список файлов в папке, без внутренних папок
void get_dir_list(char* future_dir_list, const char* current_path_) {
    char current_path[FULL_PATH_MAX_SIZE];
    sprintf(current_path, "%s", current_path_);
// get_current_dir_path(current_path);
    strcpy(future_dir_list, "");

    DIR* directory = opendir(current_path);
    struct dirent* item;
    while(NULL != (item = readdir(directory))) {
// ^^^
// |||
// Следующий элемент папки

        char* item_name = item->d_name;
        if(!strcmp(item_name, ".") ||
           !strcmp(item_name, "..") || (item_name[0] == '.')) {
// Если это текущая директория (.),
// либо родительская (..),
// либо начинается с точки - значит скрытая (.*),
// то её показывать никому не надо.
            continue;
        }

        if (!is_dir(item_name, current_path)) {
            strcat(future_dir_list, item_name);
            strcat(future_dir_list, "\n");
        }
    }
    closedir(directory);
}

// Возвращает список файлов по всем узлам хранилища
void get_full_dir_list(char * future_full_list) {
//int i;
    strcpy(future_full_list, "");
    char part_full_list[DIR_LIST_MAX_BUF_SIZE];
    int num_nodes = storage_size;
    for (i = 0; i < num_nodes; i++){
        char* cur_dir = (storage + i)->destination;
        get_dir_list(part_full_list, cur_dir);
        strcat(future_full_list, part_full_list);
    }
}

long get_file_size(const char* full_path) {
    struct stat file_info;
    int result = lstat(full_path, &file_info);
    if (result == -1) {
        fprintf(stderr, "Can't get file size : %s\n", full_path);
        return -1;
    }
    return file_info.st_size;
}

void create_duplicates_directories() {
    int num_nodes = storage_size;
    char* duplicate = "/duplicates";
    for (i = 0; i < num_nodes; i++){
        char duplicate_dir[1000];
        char* cur_dir = (storage + i)->destination;
        sprintf(duplicate_dir, "%s%s", cur_dir, duplicate);
        mkdir(duplicate_dir, 0777);
    }
}
// copy взял со stack_overflow
int copy(const char *to, const char *from) {
    int fd_to, fd_from;
    char buf[4096];
    ssize_t nread;
    int saved_errno;

    fd_from = open(from, O_RDONLY);
    if (fd_from < 0)
        return -1;

    fd_to = open(to, O_WRONLY | O_CREAT | O_EXCL, 0666);
    if (fd_to < 0)
        goto out_error;

    while (nread = read(fd_from, buf, sizeof buf), nread > 0)
    {
        char *out_ptr = buf;
        ssize_t nwritten;

        do {
            nwritten = write(fd_to, out_ptr, nread);

            if (nwritten >= 0)
            {
                nread -= nwritten;
                out_ptr += nwritten;
            }
            else if (errno != EINTR)
            {
                goto out_error;
            }
        } while (nread > 0);
    }

    if (nread == 0)
    {
        if (close(fd_to) < 0)
        {
            fd_to = -1;
            goto out_error;
        }
        close(fd_from);

        /* Success! */
        return 0;
    }

    out_error:
    saved_errno = errno;

    close(fd_from);
    if (fd_to >= 0)
        close(fd_to);

    errno = saved_errno;
    return -1;
}

int remove_file(char* path, char* filename) {
    char temp[FULL_PATH_MAX_SIZE];
    sprintf(temp, "%s/%s", path, filename);
    return remove(temp);
}

// Копирует все файлы из списка(раделенных\n) из from в to
void copy_all(const char* from, const char* to, const char* file_list) {
    char temp[DIR_LIST_MAX_BUF_SIZE];
    strcpy(temp, file_list);
    char* filename = strtok(temp, "\n\0");
    while (filename != NULL) {
        char old[FULL_PATH_MAX_SIZE];
        char new_[FULL_PATH_MAX_SIZE];
        sprintf(old, "%s/%s", from, filename);
        sprintf(new_, "%s/%s", to, filename);
        copy(new_, old);
        filename = strtok(NULL, "\n\0");
    }
}

// Со stackoverflow
int clean_directory(const char *path) {
    DIR *d = opendir(path);
    size_t path_len = strlen(path);
    int r = -1;

    if (d) {
        struct dirent *p;
        r = 0;
        while (!r && (p=readdir(d))) {
            int r2 = -1;
            char *buf;
            size_t len;

            /* Skip the names "." and ".." as we don't want to recurse on them. */
            if (!strcmp(p->d_name, ".") || !strcmp(p->d_name, "..") || !strcmp(p->d_name, "duplicates") ) { //todo ?
                continue;
            }

            len = path_len + strlen(p->d_name) + 2;
            buf = malloc(len);

            if (buf) {
                struct stat statbuf;
                snprintf(buf, len, "%s/%s", path, p->d_name);
                if (!stat(buf, &statbuf)) {
                    if (S_ISDIR(statbuf.st_mode)) {
                        r2 = clean_directory(buf);
                    } else {
                        r2 = unlink(buf);
                    }
                }
                free(buf);
            }
            r = r2;
        }
        closedir(d);
    }

    return r;
}

void make_duplicates_for_all() {
	char full_dir_list[FULL_PATH_MAX_SIZE];
	get_full_dir_list(full_dir_list);
	const char * duplicate = "duplicates";
	for (i = 0; i < storage_size; i++) {
		char cur_dup_dir[FULL_PATH_MAX_SIZE];
		char cur_dir[FULL_PATH_MAX_SIZE];
		sprintf(cur_dir, "%s", (storage + i)->destination);
		sprintf(cur_dup_dir, "%s/%s", (storage + (i + 1)%storage_size)->destination, duplicate);
		clean_directory(cur_dup_dir);
		copy_all(cur_dir, cur_dup_dir, full_dir_list);
	}
}
void rehash_after_remove(int node_id) {
    if (node_id == 0) {
        int last_id = storage_size - 1;

        char* last_node_path = storage[last_id].destination; // откуда
        char first_node_dup_path[FULL_PATH_MAX_SIZE]; // куда
        sprintf(first_node_dup_path, "%s/duplicates", storage[0].destination);

        char last_node_dir_list[DIR_LIST_MAX_BUF_SIZE];
        get_dir_list(last_node_dir_list, last_node_path); // что

        copy_all(last_node_path, first_node_dup_path, last_node_dir_list);
    }
    // Будет храниться список файлов в текущей папке дубликатов.
		char current_dup_dir_list[FULL_PATH_MAX_SIZE];
		for (i = node_id; i < storage_size; i++) {
            // Очистим папку
            clean_directory(storage[i].destination);
            // Построили путь к текущим копиям
            char current_dup_path[FULL_PATH_MAX_SIZE];
            sprintf(current_dup_path, "%s/duplicates", storage[i].destination);
            // Получили список текущих копий
            get_dir_list(current_dup_dir_list, current_dup_path);
			// Копируем из копий в НЕкопии
			copy_all(current_dup_path, storage[i].destination, current_dup_dir_list);
            // Почистим папку дубликатов
		}
        if (node_id == 0) {
            make_duplicates_for_all();
            return;
        }
        // Отдельно обработаем нулевой узел
        char current_dup_path[FULL_PATH_MAX_SIZE];
        sprintf(current_dup_path, "%s/duplicates", storage[0].destination);
        // Получили список текущих копий
        get_dir_list(current_dup_dir_list, current_dup_path);
        // Копируем из копий в НЕкопии
        copy_all(current_dup_path, storage[0].destination, current_dup_dir_list);
		make_duplicates_for_all();
}

#endif //OUR_CLOUD_FILES_H
