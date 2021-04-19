#include "nodes_list.h"

struct net
{
    char IP[BUFFER_SIZE];       //   IP address of local machine
    char TCP[BUFFER_SIZE];    //   TCP port of local machine
    char regIP[BUFFER_SIZE];    //   IP address of remote list
    char regUDP[BUFFER_SIZE]; //   UDP port of remote List
};
struct net NET;             // pivate struct of this file containing information about the remote UDP server

// Set UDP connection
struct addrinfo hints, *res;
int fd_udp;
ssize_t n_sent, n_received;
struct sockaddr_in addr;
socklen_t addrlen = sizeof(addr);
char buffer[BUFFER_SIZE];

/*
Initialise the parameters related to the connection with the remote UDP server
*/
bool set_parameters(int argc, char *argv[])
{
    if (check_IP(argv[1]))
        sprintf(NET.IP, "%s", argv[1]);
    else
        return false;

    if (check_port(argv[2]))
        sprintf(NET.TCP, "%s", argv[2]);
    else
        return false;

    if (argc >= 4)
        if (check_IP(argv[3]))
            sprintf(NET.regIP, "%s", argv[3]);
        else
            return false;
    else
        sprintf(NET.regIP, "193.136.138.142");

    if (argc == 5)
        if (check_port(argv[4]))
            sprintf(NET.regUDP, "%s", argv[4]);
        else
            return false;
    else
        sprintf(NET.regUDP, "59000");

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
message to the remote UDP server.
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
    Getaddrinfo(NET.regIP, NET.regUDP, &hints, &res);

    n_sent = sendto(fd_udp, message, strlen(message), 0, res->ai_addr, res->ai_addrlen);
    printf("Sent UDP to %s:%s -> %s\n",NET.regIP, NET.regUDP, message);

    return true;
}

/*
Receives a message to the remote UDP server.
*/
void receive_udp_message(char *message)
{
    fd_set rfds;

    FD_ZERO(&rfds);
    FD_SET(0, &rfds);
    FD_SET(fd_udp, &rfds);
    struct timeval timeout = {2 /*seconds*/, 0 /*milliseconds*/};

    int n = Select(fd_udp + 1, &rfds, (fd_set *)NULL, (fd_set *)NULL, &timeout);
    if (n == 0)
        printf("Time exceeded during an incoming UDP message\n");

    else if (FD_ISSET(fd_udp, &rfds)) // stdin
    {
        memset(buffer, '\0', BUFFER_SIZE);
        n_received = Recvfrom(fd_udp, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&addr, &addrlen);

        // desencrypt
        memset(message, '\0', MESSAGE_SIZE);
        for(int j=0; j<MESSAGE_SIZE && j<n_received; j++)
            message[j] = buffer[j];

    }
    // write(1, buffer, n_received);
    // printf("\n\n");
}

void open_UDP()
{
    fd_udp = Socket(AF_INET, SOCK_DGRAM, 0); //UDP socket

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;      //IPv4
    hints.ai_socktype = SOCK_DGRAM; //UDP socket
}

void close_UDP()
{
    close(fd_udp);
    freeaddrinfo(res);
}
