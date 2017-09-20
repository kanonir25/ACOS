

#ifndef OUR_CLOUD_STORAGE_H
#define OUR_CLOUD_STORAGE_H

#include <stdlib.h>

typedef struct node {
    int id;
    int pair_id;
    char* destination;
} node;

node* storage; // Динамическая память, выделяется в init_storage
int storage_size = -1;

// Файл, который будет загружаться на сервер, будет попадать
// в узел с номером, который будет вычислен хэш-функцией.
unsigned int HashFAQ6(const char * str) {
    unsigned int hash = 0;

    for (; *str; str++)
    {
        hash += (unsigned char)(*str);
        hash += (hash << 10);
        hash ^= (hash >> 6);
    }
    hash += (hash << 3);
    hash ^= (hash >> 11);
    hash += (hash << 15);

    return hash;
}

void update_node(node* node_, char* dest, int id, int pair_id) {
    node_->destination = dest;
    node_->id = id;
    node_->pair_id = pair_id;
}
void remove_node(int node_id) {
    for (i = node_id; i < storage_size - 1; i++){
        (storage + i)->destination = (storage + i + 1)->destination;
    }
    storage = realloc(storage, (storage_size - 1) * sizeof(node));
    --storage_size;
}
void init_storage(int number, char** destinations) {
    storage = (node*)malloc(number*sizeof(node));
    for (i = 0; i < number; ++i) {
        update_node(storage+i, destinations[i+1], i, (i+1) % number);
    }
    storage_size = number-1;
    create_duplicates_directories();
}
// Возвращает id узла по хэшу
int chose_node(const char *filename) {
    return HashFAQ6(filename) % storage_size;
}

#endif //OUR_CLOUD_STORAGE_H
