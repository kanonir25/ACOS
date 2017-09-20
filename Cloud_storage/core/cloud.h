
#ifndef OUR_CLOUD_CLOUD_H
#define OUR_CLOUD_CLOUD_H

#include <stdio.h>
#include <string.h>

#include "all_functions.h"
#include "storage.h"
#include "server.h"
#include "cloud_interface.h"

void start_handling_client_requests(int sock) {
    while (1) {
        char handled_message[BUF_SIZE];
        receive_message(handled_message, sock);

        if (handled_message == NULL || !strcmp("", handled_message)) {
            // Клиент отключился?
            printf("Клиент (sock_fd = %d) отключился?\n", sock);
            return;
        }

        if (!strcasecmp(get_list, handled_message)) { // getlist

            perform_getlist(sock);

        } else if (!strcasecmp(download, handled_message)) { // download

            char filename[1024];
            receive_message(filename, sock);
            perform_download(sock, filename);

        } else if (!strcasecmp(upload, handled_message)) { // upload

            char filename[1024];
            receive_message(filename, sock);
            perform_upload(sock, filename);

        } else if (!strcasecmp(finish, handled_message)) { // exit
            // С этим клиентом - всё.
            // Будем подбирать следующего в thread_processing
            return;
        } else if (!strcasecmp(delete_, handled_message)) { // delete

            char filename[1024];
            receive_message(filename, sock);
            perform_delete(sock, filename);

        } else if (!strcasecmp(deactivate, handled_message)) {

            char node_id[1024];
            receive_message(node_id, sock);
            perform_deactivate(node_id, sock);

        } else {
            fprintf(stderr, "Недопустимое действие (\"%s\") со стороны sock_fd = %d.\n", handled_message, sock);
        }
    }
}

void run_cloud(int number, char** destinations, uint16_t port) {
    setbuf(stdout, 0);
    setbuf(stderr, 0);

    init_storage(number, destinations);
    int sock = prepare_server(port);
    run_server(sock);
}



#endif //OUR_CLOUD_CLOUD_H
