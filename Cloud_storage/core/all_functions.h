

#ifndef OUR_CLOUD_ALL_FUNCTIONS_H
#define OUR_CLOUD_ALL_FUNCTIONS_H

#include <stdint.h>
#include <stdio.h>

static int i;

int main(int argc, char** argv);

// Cloud Functions
typedef struct node node;
void run_cloud(int number, char** destinations, uint16_t port);
void start_handling_client_requests(int sock);;

// Server
int prepare_server(uint16_t port);
void run_server(int sock);
int* run_threads(int threads_number, int* our_pipe);
void* thread_processing(void* our_pipe);

// Networking
int recv_all(int sfd, void* buf, long len, int flags);
int send_all(int sfd, void* buf, long len, int flags);
int receive_message(char* str, int sock);
int send_message(char* str, int sock);
long receive_file(int sock, void** pointer_to_alloc);
long send_file(int sock, void* data, long file_size);

// Files
int is_dir(char* filename, char* path);
// Возвращает список файлов в папке, без внутренних папок
void get_dir_list(char* future_dir_list, const char* current_path_);
// Возвращает список файлов по всем узлам хранилища
void get_full_dir_list(char * future_full_list);
long get_file_size(const char* full_path);
void create_duplicates_directories();
// copy взял со stack_overflow
int copy(const char *to, const char *from);
int remove_file(char* path, char* filename);
// Копирует все файлы из списка(раделенных\n) из from в to
void copy_all(const char* from, const char* to, const char* file_list);
// Со stackoverflow
int clean_directory(const char *path);

// Storage
struct node;
unsigned int HashFAQ6(const char * str);
void update_node(node* node_, char* dest, int id, int pair_id);
void remove_node(int node_id);
void init_storage(int number, char** destinations);
int chose_node(const char *filename);

// Error function
void throw_error(char* message, int sock) {
    fprintf(stderr, "Error : %s (sock = %d)", message, sock);
    return;
}
#endif //OUR_CLOUD_ALL_FUNCTIONS_H
