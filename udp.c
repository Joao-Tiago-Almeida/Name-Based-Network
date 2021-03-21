#include "udp.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

#define SIZE_UDP 128

struct net
{
    char IP[16];    //   IP address of local machine
    char TCP[6];    //   TCP port of local machine
    char regIP[16]; //   IP address of remote list
    char regUDP[6]; //   UDP port of remote List
};

struct net NET;                  // pivate struct of this file containing information about the remote UDP server
bool parameters_are_set = false; // If I already know the parameters
bool is_registered = false;      // If I am registered in the list

// Set UDP connection
struct addrinfo hints, *res;
int fd_udp, errcode;
ssize_t n_sent, n_received;
struct sockaddr_in addr;
socklen_t addrlen = sizeof(addr);
char buffer[SIZE_UDP];

/*
Initialise the parameters related to the connection with the remote UDP server
*/
void set_parameters(int argc, char *argv[])
{
    strcpy(NET.IP, argv[1]);
    strcpy(NET.TCP, argv[2]);
    if (argc >= 4)
        strcpy(NET.regIP, argv[3]);
    else
        strcpy(NET.regIP, "193.136.138.142");
    if (argc == 5)
        strcpy(NET.regUDP, argv[4]);
    else
        strcpy(NET.regUDP, "59000");

    open_UDP();

    parameters_are_set = true;
}

/*
Print the parameters related to the connection with the remote UDP server
*/
void get_info()
{   
    printf("\n[INFO]\a");
    printf("IP: %s\n", NET.IP);
    printf("TCP: %s\n", NET.TCP);
    printf("regIP: %s\n", NET.regIP);
    printf("regUDP: %s\n\n", NET.regUDP);
}

bool send_udp_message(char *message)
{
    if (!parameters_are_set)
    {
        fprintf(stderr, "The UDP parameters are not set yet.\n");
        return false;
    }

    errcode = getaddrinfo(NET.regIP, NET.regUDP, &hints, &res);
    if (errcode != 0)
    {
        fprintf(stderr, "Error in the UDP function _getaddrinfo_ in Sending message.\n");
        exit(1);
    }

    n_sent = sendto(fd_udp, message, strlen(message), 0, res->ai_addr, res->ai_addrlen);
    printf("%s\n", message);
    
    return true;
}

char *receive_udp_message()
{
    n_received = recvfrom(fd_udp, buffer, SIZE_UDP, 0, (struct sockaddr *)&addr, &addrlen);
    if (n_received == -1)
    {
        fprintf(stderr, "Error in the UDP receiving message.\n");
        exit(1);
    }

    write(1, buffer, n_received);
    printf("\n\n");
    return " ";
}

void open_UDP()
{
    fd_udp = socket(AF_INET, SOCK_DGRAM, 0); //UDP socket
    if (fd_udp == -1)
    {
        fprintf(stderr, "Error opening UDP connection\n");
        exit(1);
    }

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;      //IPv4
    hints.ai_socktype = SOCK_DGRAM; //UDP socket
}

void close_UDP()
{
    close(fd_udp);
    freeaddrinfo(res);
}

// Sending message

// // Receiving echo
// n = recvfrom(fd, buffer, 128, 0, (struct sockaddr *)&addr, &addrlen);
// if (n == -1) /*error*/ exit(1);

// write(1, "echo: ", 6); //stdout
// write(1, buffer, n);
