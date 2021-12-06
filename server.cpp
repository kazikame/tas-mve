#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/mman.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <netdb.h>
#include <stdio.h>

/**
 * @brief Create a tcp connection object
 * 
 * @param port
 * @param ip
 * @param is_client
 * @return int 0 if success, < 0 if error
 */
static int create_tcp_connection(char *port, char *ip, bool is_client) {
    int ret;
    int sock;
    // Create the socket.
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("socket");
        goto fail;
    }
    // Convert the hostname to a sockaddr.
    struct addrinfo hints;
    bzero(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    struct addrinfo *res;
    if (getaddrinfo(ip, port, &hints, &res)) {
        perror("getaddrinfo");
        goto fail;
    }
    if (is_client) {
        // Running the client, so connect to server.
        printf("Connecting to server at %s:%s ...", ip, port);
        if (connect(sock, res->ai_addr, res->ai_addrlen) == -1) {
            perror("connect");
            goto fail_freeaddrinfo;
        }
    } else {
        // Running the server, so bind and accept a connection.
            if (bind(sock, res->ai_addr, res->ai_addrlen)) {
                perror("bind");
                goto fail_freeaddrinfo;
            }
            // Listen for connections.
            if (listen(sock, 1)) {
                perror("listen");
                goto fail_freeaddrinfo;
            }
        // Accept a connection;
        printf("Waiting for client to connect on port %s ... ", port);
        sock = accept(sock, 0, 0);    
        if (sock == -1) {
            perror("accept");
            goto fail_freeaddrinfo;
        }
    }
    freeaddrinfo(res);
    return sock;
fail_freeaddrinfo:
    freeaddrinfo(res);
fail:
    close(sock);
    return -1;
}

static void sync_with_remote(int sync_sock) {
    uint64_t dummy = 0;
    send(sync_sock, &dummy, sizeof(dummy), 0);
    recv(sync_sock, &dummy, sizeof(dummy), 0);
}

int main(int argc, char **argv)
{
    char *port = argv[2];
    char *ip = argv[1];
    int sock = create_tcp_connection(port, ip, false);
    if (sock < 0) return -1;

    printf("TCP socket created!");
    
    for (int i = 0; i < 1000; i++)
    {
        printf("Iter %d\n", i);
        sync_with_remote(sock);
    }
}
