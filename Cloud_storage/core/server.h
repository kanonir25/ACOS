

#ifndef OUR_CLOUD_SERVER_H
#define OUR_CLOUD_SERVER_H

#define THREADS_NUMBER 2

#include <pthread.h>
#include <netinet/in.h>
#include <unistd.h>
#include <assert.h>
#include "cloud.h"


void* thread_processing(void* our_pipe) {
//Здесь описано ожидание потоком момента, пока в пайп
//не будут записаны данные, и соответственно чтение этих данных
    int read_fd = ((int*)our_pipe)[0];

    while(1)
    {
        int client_socket = -1;
        ssize_t bytes_received = 0;

        // Здесь получим сокет клиента для обработки
        //pthread_mutex_lock(&mtx);
        bytes_received = read(read_fd, &client_socket, sizeof(int));
        //pthread_mutex_unlock(&mtx);
        if (bytes_received == 0) continue;

        assert(bytes_received == sizeof(int));
        assert(client_socket != -1);
        // Тут начнём выполнять его задания
        start_handling_client_requests(client_socket);
    }
}

int* run_threads(int threads_number, int* our_pipe) {
    assert(threads_number > 0);
    pipe(our_pipe);

    for (i = 0; i < threads_number; ++i)
    {
        pthread_t thread_id;
        pthread_create(&thread_id, NULL, thread_processing, (void*)our_pipe);
        printf("Поток id%ld запущен...\n", thread_id);
    }
    return our_pipe;
}

int prepare_server(uint16_t port) {
    // Инициализация параметров инициализации сокета
    struct sockaddr_in sa;
    sa.sin_family = AF_INET;
    sa.sin_port = htons(port);
    sa.sin_addr.s_addr = htonl(INADDR_ANY);

    // Запускаем сервер и начинаем прослушивание
    int server_socket;
    server_socket = socket(AF_INET, SOCK_STREAM, 0);							//Creating socket
    int optval;
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)); // Чтобы переиспользовать адрес - чтоб не было проблемы подключения сразу после отключения сервера
    bind(server_socket, (struct sockaddr *)&sa, sizeof(struct sockaddr));		//Binding - association server_socket_fd with sa?
    listen(server_socket, THREADS_NUMBER);										//listening with maximum 1 requests in queue
    printf("Сервер запущен...\n");
    return server_socket;
}

void run_server(int sock) {
    int our_pipe[2];
    run_threads(THREADS_NUMBER, our_pipe);
    // Бесконечный цикл прослушивания клиентов - подсоединения.
    while (0 == 0)
    {
        int client = accept(sock, NULL, 0);
        assert(client > 0);
        printf("Client (sock_fd = %d) connected...\n", client);
        write(our_pipe[1], &client, sizeof(int));
        // Сунули клиента в очередь, подбирать будут потоки
    }
}

#endif //OUR_CLOUD_SERVER_H
