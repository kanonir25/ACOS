
#include "client.h"

int main(int argc, char** argv) {
    setbuf(stdout, 0);
    setbuf(stderr, 0);
    int sock = run_client(argc, argv);
    client_processing(sock);
    return 0;
}
