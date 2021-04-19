#include "protocol_support.h"

void Getaddrinfo(const char *hostname, const char *servname,
                 const struct addrinfo *hints, struct addrinfo **res)
{
    int errcode = getaddrinfo(hostname, servname, hints, res);
    if (errcode != 0)
    {
        fprintf(stderr, "Error in Getaddrinfo(): %s\n", gai_strerror(errcode));
        freeaddrinfo(*res);
        exit(1);
    }
}

int Recvfrom(int socket, void *restrict buffer, size_t length, int flags,
             struct sockaddr *restrict address, socklen_t *restrict address_len)
{
    int n = recvfrom(socket, buffer, length, flags, address, address_len);
    if (n == -1)
    {
        fprintf(stderr, "Error in Recvfrom(): %s\n", strerror(errno));
        Close(socket);
        exit(1);
    }

    return n;
}

int Socket(int domain, int type, int protocol)
{
    int fd = socket(domain, type, protocol); //UDP socket
    if (fd == -1)
    {
        fprintf(stderr, "Error in Socket(): %s\n", strerror(errno));
        exit(1);
    }
    return fd;
}

// sudo lsof -i :port - https://stackoverflow.com/questions/3855127/find-and-kill-process-locking-port-3000-on-mac : 1. kill -15 <PID>, 2. kill -9 <PID>
void Bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
    int n = bind(sockfd, addr, addrlen);
    if (n == -1)
    {
        fprintf(stderr, "Error in Bind(): %s\n", strerror(errno));
        Close(sockfd);
        exit(1);
    }
}

int Accept(int socket, struct sockaddr *restrict address, socklen_t *restrict address_len)
{
    int newfd;
    if ((newfd = accept(socket, address, address_len)) == -1)
    {
        fprintf(stderr, "Error in Accept(): %s\n", strerror(errno));
        Close(socket);
        exit(1);
    }
    return newfd;
}

void Connect(int sockfd, const struct sockaddr *addr,socklen_t addrlen)
{
    int n = connect(sockfd, addr, addrlen);
    if (n == -1)
    {
        fprintf(stderr, "Error in Connect(): %s\n", strerror(errno));
        Close(sockfd);
        exit(1);
    }
}

// TODO 
void Close(int fildes)
{
    int n = close(fildes);
    if (n == -1)
    {
        fprintf(stderr, "Error in Close(%d): %s\n",fildes, strerror(errno));
        exit(1);
    }
}

int Select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout)
{
    int n = select(nfds, readfds, writefds, exceptfds, timeout);

    if (n == -1)
    {
        fprintf(stderr, "Error in Select(): %s\n", strerror(errno));
        // Close(nfds); TODO
        exit(1);
    }
    return n;
}

int Listen(int sockfd)
{
    int n = listen(sockfd, 5);
    if (n == -1)
    {
        fprintf(stderr, "Error in Listen(): %s\n", strerror(errno));
        Close(sockfd);
        exit(1);
    }
    return n;
}

ssize_t Read(int fd, void *buf, size_t count)
{
    char character[2]; count = 1;
    int n, n_total = 0;
    memset(buf, '\0', MESSAGE_SIZE);

    do
    {
        n = read(fd, character, 1);
        if (n == -1)
        {
            fprintf(stderr, "Error in Read(): %s\n", strerror(errno));
            Close(fd);
            exit(1);
        }
        n_total = n_total+n;

        sprintf(buf, "%s%c", buf, character[0]);

    } while (character[0]!='\n' && n!=0);

    printf("You received from %d a %d:%d Bytes message: %s\n", fd, n_total, (int)strlen(buf), buf);
    return n_total;
}

ssize_t Write(int fd, const void *buf, size_t count)
{   
    int n = write(fd, buf, count);
    if (n == -1)
    {
        fprintf(stderr, "Error in Write(): %s\n", strerror(errno));
        Close(fd);
        exit(1);
    }
    printf("You wrote to %d a %d:%d Bytes message: %s\n", fd, n, (int)strlen(buf), buf);
    return n;
}

