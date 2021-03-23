#include "server.h"

struct server
{
    int fd_server, errcode;
    ssize_t n_transfer;
    struct addrinfo hints, *res;
    struct sockaddr_in addr;
    socklen_t addrlen;
    char income_message[BUFFER_SIZE];
};

struct server SERVER; // pivate struct of this file containing information about the TCP server

void server_up(char *port)
{
    SERVER.fd_server = socket(AF_INET, SOCK_STREAM, 0); //TCP socket
    if (SERVER.fd_server == -1)
    {
        fprintf(stderr, "Cannot create server file descriptor.\n");
        exit(1);
    }

    memset(&SERVER.hints, 0, sizeof(SERVER.hints));
    SERVER.hints.ai_family = AF_INET;       //IPv4
    SERVER.hints.ai_socktype = SOCK_STREAM; //TCP socket
    SERVER.hints.ai_flags = AI_PASSIVE;

    SERVER.errcode = getaddrinfo(NULL, port, &SERVER.hints, &SERVER.res);
    if ((SERVER.errcode) != 0)
    {
        fprintf(stderr, "Error: getaddrinfo: %s\n", gai_strerror(SERVER.errcode));
        exit(1);
    }

    SERVER.n_transfer = bind(SERVER.fd_server, SERVER.res->ai_addr, SERVER.res->ai_addrlen);
    if (SERVER.n_transfer == -1)
    {
        fprintf(stderr, "Error at bind function in the server.\n");
        exit(1);
    }
    SERVER.addrlen = (socklen_t)sizeof(SERVER.addr);

    // apagar
    printf("ola");
    int n;
    int newfd;
    char buffer[128];
    struct sockaddr_in addr;
    socklen_t addrlen;

    while (1)
    {
        addrlen = sizeof(addr);
        if ((newfd = accept(SERVER.fd_server, (struct sockaddr *)&addr, &addrlen)) == -1) /*error*/
            exit(1);

        n = read(newfd, buffer, 128);

        if (n == -1) /*error*/
            //exit(1);

        write(1, "received: ", 10);
        write(1, buffer, n);
        n = write(newfd, buffer, n);
        if (n == -1) /*error*/
           //exit(1);
        close(newfd);
        printf("ola\n");
    }
}

void server_down()
{
    freeaddrinfo(SERVER.res);
    close(SERVER.fd_server);
}