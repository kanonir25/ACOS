

#ifndef OUR_CLOUD_NETWORKING_H
#define OUR_CLOUD_NETWORKING_H

// networking
#include <sys/socket.h>
#include <netinet/in.h>

#define BUF_SIZE 1024

// получение заранее известного числа байт
int recv_all(int sfd, void* buf, long len, int flags) {
    int total = 0;
    while(total < len)
    {
        ssize_t n = recv(sfd, buf + total, len - total, flags);
        if (n == -1)
            return -1;
        if (n == 0)
            return total;
        total += n;
    }
    return total;
}

// отправка заранее известного числа байт
int send_all(int sfd, void* buf, long len, int flags)
{
    int total = 0;
    while(total < len)
    {
        ssize_t n = send(sfd, buf + total, len - total, flags);
        if(n == -1)
            return -1;
        total += n;
    }
    return total;
}

int receive_message(char* str, int sock) {
    int size;

    int bytes_received = recv_all(sock, &size, sizeof(int), 0);
    if (bytes_received == -1) {
        return -1;
    }

    bytes_received = recv_all(sock, str, size, 0);
    if (bytes_received == -1) {
        return -1;
    }

    str[bytes_received] = '\0';
    return bytes_received;
}

int send_message(char* str, int sock) {
    size_t size = strlen(str);
    int bytes_sent = send_all(sock, &size, sizeof(int), 0);
    if (bytes_sent == -1) {
        return -1;
    }

    bytes_sent = send_all(sock, str, size, 0);
    if (bytes_sent == -1) {
        return -1;
    }

    return bytes_sent;
}

long receive_file(int sock, void** pointer_to_alloc) {
    long file_size;
    long received_bytes = recv_all(sock, &file_size, sizeof(long), 0);
    if (received_bytes != sizeof(long)) {
        throw_error("receiving file_size", sock);
        return -1;
    }
    assert(file_size > 0);

    *pointer_to_alloc = malloc((unsigned long)file_size);
    received_bytes = recv_all(sock, *pointer_to_alloc, file_size, 0);
    if (received_bytes < file_size) {
        throw_error("receiving file", sock);
        free(*pointer_to_alloc);
        return -1;
    }


    return received_bytes;
}

long send_file(int sock, void* data, long file_size) {
    long sent_bytes = send_all(sock, &file_size, sizeof(long), 0);
    if (sent_bytes != sizeof(long)) {
        throw_error("sending file_size", sock);
        return -1;
    }
    assert(file_size > 0);

    sent_bytes = send_all(sock, data, file_size, 0);
    if (sent_bytes < file_size) {
        throw_error("sending file", sock);
        return -1;
    }

    return sent_bytes;
}


#endif //OUR_CLOUD_NETWORKING_H
