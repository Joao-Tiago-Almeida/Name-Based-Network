#ifndef PROTOCOL_SUPPORT
#define PROTOCOL_SUPPORT

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>

#include "util.h"


void Getaddrinfo(const char *hostname, const char *servname,
                    const struct addrinfo *hints, struct addrinfo **res);

int Recvfrom(int socket, void *restrict buffer, size_t length, int flags,
                struct sockaddr *restrict address, socklen_t *restrict address_len);

int Socket(int domain, int type, int protocol);

void Bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);

int Accept(int socket, struct sockaddr *restrict address, socklen_t *restrict address_len);

void Connect(int sockfd, const struct sockaddr *addr,socklen_t addrlen);

void Close(int fildes);

int Select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout);

int Listen(int sockfd);

ssize_t Read(int fd, void *buf);

ssize_t Write(int fd, const void *buf, size_t count);
 
#endif