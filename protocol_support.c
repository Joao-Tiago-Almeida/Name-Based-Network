#include "protocol_support.h"

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

void Close(int fildes)
{
    int n = close(fildes);
    if (n == -1)
    {
        fprintf(stderr, "Error in Close(%d): %s\n", fildes, strerror(errno));
        exit(1);
    }
    if (DEBUG)
        printf("Closed(%d) successfully\n", fildes);
}

void Connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
    int n = connect(sockfd, addr, addrlen);
    if (n == -1)
    {
        fprintf(stderr, "Error in Connect(): %s\n", strerror(errno));
        Close(sockfd);
        exit(1);
    }
}

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

int Listen(int sockfd)
{
    int n = listen(sockfd, 5);
    if (n == -1)
    {
        fprintf(stderr, "Error in Listen(%d): %s\n", sockfd, strerror(errno));
        Close(sockfd);
        exit(1);
    }
    return n;
}

ssize_t Read(int fd, char *buf)
{
    char character[2] = {'\0'};
    int n = 0, n_total = 0;
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
        n_total = n_total + n;

        buf[n_total - 1] = character[0];

    } while (character[0] != '\n' && n != 0);

    if (DEBUG)
        printf("You received from %d a %d:%d Bytes message: %s\n", fd, n_total, (int)strlen(buf), (char *)buf);
    return n_total;
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

int Select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout)
{
    int n = select(nfds, readfds, writefds, exceptfds, timeout);

    if (n == -1)
    {
        fprintf(stderr, "Error in Select(%d): %s\n", n, strerror(errno));
        // Close(nfds); TODO
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

ssize_t Write(int fd, const void *buf, size_t count)
{
    int n = write(fd, buf, count);
    if (n == -1)
    {
        fprintf(stderr, "Error in Write(%d): %s\n", fd, strerror(errno));
        exit(1);
    }
    if (DEBUG)
        printf("You wrote to %d a %d:%d Bytes message: %s\n", fd, n, (int)strlen(buf), (char *)buf);
    return n;
}