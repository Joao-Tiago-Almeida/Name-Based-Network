#include "list_nodes.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

struct net
{
    char IP[IP_SIZE];       //   IP address of local machine
    char TCP[PORT_SIZE];    //   TCP port of local machine
    char regIP[IP_SIZE];    //   IP address of remote list
    char regUDP[PORT_SIZE]; //   UDP port of remote List
};

struct net NET;             // pivate struct of this file containing information about the remote UDP server
bool is_registered = false; // If I am registered in the list

// Set UDP connection
struct addrinfo hints, *res;
int fd_udp, errcode;
ssize_t n_sent, n_received;
struct sockaddr_in addr;
socklen_t addrlen = sizeof(addr);
char buffer[UDP_SIZE];

/*
Initialise the parameters related to the connection with the remote UDP server
*/
bool set_parameters(int argc, char *argv[])
{
    if (check_IP(argv[1]))
        strcpy(NET.IP, argv[1]);
    else
        return false;

    if (check_port(argv[2]))
        strcpy(NET.TCP, argv[2]);
    else
        return false;

    if (argc >= 4)
        if (check_IP(argv[3]))
            strcpy(NET.regIP, argv[3]);
        else
            return false;
    else
        strcpy(NET.regIP, "193.136.138.142");

    if (argc == 5)
        if (check_port(argv[3]))
            strcpy(NET.regUDP, argv[4]);
        else
            return false;
    else
        strcpy(NET.regUDP, "59000");

    open_UDP();

    return true;
}

/*
Print the parameters related to the connection with the remote UDP server
*/
void get_info()
{
    printf("\n[INFO]\n\a");
    printf("IP: %s\n", NET.IP);
    printf("TCP: %s\n", NET.TCP);
    printf("regIP: %s\n", NET.regIP);
    printf("regUDP: %s\n\n", NET.regUDP);
}

/*
Sends a message to the remote UDP server.
Types of message and responses

- [CLIENT] REG net IP TCP   :   Um nó regista o seu contacto no servidor de nós e associa-o à rede net.
- [SERVER] OKREG            :   O servidor de nós confirma o registo de um contacto.

- [CLIENT] UNREG net IP TCP :   O nó retira o seu contacto associado à rede net.
- [SERVER] OKUNREG          :   O servidor de nós confirma a retirada de um contacto.

- [CLIENT] NODES net        :   Um nó pede ao servidor de nós os contactos associados à rede net.
- [SERVER] NODESLIST net<LF>
                IP1 TCP1<LF>    
                IP2 TCP2<LF>    :   O servidor de nós confirma a retirada de um contacto.
*/
bool send_udp_message(char *message)
{
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

/*
Receives a message to the remote UDP server.
*/
char *receive_udp_message()
{
    n_received = recvfrom(fd_udp, buffer, UDP_SIZE, 0, (struct sockaddr *)&addr, &addrlen);
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
