#include <stdlib.h>
#include <stdio.h>

#include "list_nodes.h"

void print_usage(char *program_name)
{
    printf("Usage: %s IP TCP regIP regUDP\n", program_name);
}

void choose_neighbor(char message[], char bootIP[], char bootTCP[], int n_neighbor)
{
    int *pos_LF_vect = (int *)checked_calloc(n_neighbor, sizeof(int));
    int neighbor = 1; // init for the situation when there is only one neighbor

    int count = 0;
    char buffer[BUFFER_SIZE];

    // get the position of the '\n'
    for (int i = 0; message[i] != '\0'; ++i)
    {
        if ('\n' == message[i])
            pos_LF_vect[count++] = i;
    }

    if (n_neighbor > 2) // in case there is more than one neighbors
    {
        printf("\nInsert the number of the neighboor that you want to connect to.\n");

        // print option list
        for (int i = 1; i < n_neighbor; ++i)
        {
            printf("%d. ", i);
            for (int j = pos_LF_vect[i - 1] + 1; j <= pos_LF_vect[i]; j++)
                printf("%c", message[j]);
        }

        neighbor = 0;
        do
        {
            printf("Neihgbor: ");
            fgets(buffer, BUFFER_SIZE, stdin);
            sscanf(buffer, "%d", &neighbor);
        } while (neighbor < 1 || neighbor >= n_neighbor);
    }

    memset(buffer, '\0', BUFFER_SIZE);
    for (int i = pos_LF_vect[neighbor - 1]; i < pos_LF_vect[neighbor]; i++)
    {
        sprintf(buffer, "%s%c", buffer, message[i]);
    }
    sscanf(buffer, "%s %s", bootIP, bootTCP);
    //printf("bootIP: %s\nbootTCP: %s\n", bootIP, bootTCP);

    free(pos_LF_vect);
}

void network_interaction(char *ip, char *port)
{
    enum
    {
        lobby,
        connecting,
        connected,
        disconnecting
    } state;
    state = lobby; // initial state

    char buffer[BUFFER_SIZE], command[BUFFER_SIZE], net[BUFFER_SIZE], id[BUFFER_SIZE], bootIP[BUFFER_SIZE], bootTCP[BUFFER_SIZE], message[BUFFER_SIZE + 1];
    int number_of_line_feed = 0;
    int fd_server, fd_client, newfd, n_select;
    fd_set rfds;
    struct addrinfo hints, *res_server, *res_client;
    struct sockaddr_in addr_client;
    socklen_t addrlen;

    fd_server = Socket(AF_INET, SOCK_STREAM, 0); //TCP socket
    fd_client = Socket(AF_INET, SOCK_STREAM, 0); //TCP socket

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;       //IPv4
    hints.ai_socktype = SOCK_STREAM; //TCP socket
    hints.ai_flags = AI_PASSIVE;

    Getaddrinfo(NULL, port, &hints, &res_server);

    Bind(fd_server, res_server->ai_addr, res_server->ai_addrlen);
    addrlen = (socklen_t)sizeof(addr_client);

    while (1)
    {
        switch (state)
        {
        case lobby:
        {
            printf("[LOBBY] Should I stay or should I go? Available commands:\n");
            printf("\t>\tjoin net id\n");
            printf("\t>\tjoin net id bootIP bootTCP\n");
            printf("\t>\texit\n\n");
            fgets(buffer, BUFFER_SIZE, stdin);
            sscanf(buffer, "%s", command);
            if (strcmp(command, "exit") == 0)
            {
                freeaddrinfo(res_server);
                Close(fd_server);
                printf("See you later alligator!\n");
                exit(0);
            }
            else if (strcmp(command, "join") == 0)
            {
                int n = sscanf(buffer, "%*s %s %s %s %s", net, id, bootIP, bootTCP);
                //if(check_net(net)){ break; };

                sprintf(message, "NODES %s", net);
                send_udp_message(message);
                receive_udp_message(message);
                number_of_line_feed = get_number_of_LF(message);

                if (n == 2) // ask for a neighbor
                {
                    if (number_of_line_feed == 1) // no one is on the net yet, I will be the first
                    {
                        state = connected;
                        break;
                    }
                    else
                        choose_neighbor(message, bootIP, bootTCP, number_of_line_feed);
                }
                else if (n == 4)
                    ; // ip:port already speacified
                else
                {
                    printf("Invalid command. \n");
                    break;
                }
                if (check_IP(bootIP) && check_port(bootTCP))
                {
                    state = connecting;
                }
            }

            break;
        }
        case connecting:
        {
            printf("[CONNECTING] Waiting for connection confirmation.\n");
            Getaddrinfo(bootIP, bootTCP, &hints, &res_client);
            Connect(fd_client,res_client->ai_addr,res_client->ai_addrlen);
            Write(fd_client, "hey new buddy!!", 16);
            state = connected;
            break;
        }
        case connected:
        {
            printf("[CONNECTED] You are connected. Available commands:\n");
            printf("\t>\tcreate subname\n");
            printf("\t>\tget name\n");
            printf("\t>\tshow topology (st)\n");
            printf("\t>\tshow routing  (sr)\n");
            printf("\t>\tshow cache    (sc)\n");
            printf("\t>\tleave\n");
            printf("\t>\texit\n\n");

            // Starting accepting incoming connections
            Listen(fd_server);

            // Tell the UDP server I am on!
            sprintf(message, "REG %s %s %s", net, ip, port);
            send_udp_message(message);
            receive_udp_message(message);
            if (strcmp(message, "OKREG")) // not register
            {
                printf("Could not register on the UDP list. Try again.\n");
                break;
            }

            FD_ZERO(&rfds);
            FD_SET(0, &rfds);
            FD_SET(fd_server, &rfds);
            n_select = Select(fd_server + 1, &rfds, (fd_set *)NULL, (fd_set *)NULL, (struct timeval *)NULL);
            if (FD_ISSET(0, &rfds)) // stdin
            {
                fgets(buffer, BUFFER_SIZE, stdin);
                sscanf(buffer, "%s", command);
                if (strcmp(command, "exit") == 0) // not register
                {
                    sprintf(message, "UNREG %s %s %s", net, ip, port);
                    send_udp_message(message);
                    exit(0);
                }
            }
            else if (FD_ISSET(fd_server, &rfds))
            {
                newfd = Accept(fd_server, (struct sockaddr *)&addr_client, &addrlen);
                Read(newfd, buffer, BUFFER_SIZE);
            }
            break;
        }
        case disconnecting:
        {
            break;
        }
        }
    }
}

int main(int argc, char *argv[])
{

    if (argc < 3 || argc > 5)
    {
        fprintf(stderr, "Invalid number of arguments\n");
        print_usage(argv[0]);
        return 1;
    }

    if (!set_parameters(argc, argv))
        exit(1);
    get_info();

    network_interaction(argv[1], argv[2]);

    // char message[BUFFER_SIZE+1];
    // send_udp_message("UNREG 35 192.168.1.75 56787");
    // receive_udp_message(message);
    // // // send_udp_message("UNREG 35 192.168.1.75 56789");
    // // // receive_udp_message(message);
    // // // send_udp_message("UNREG 35 192.168.1.75 56790");
    // // // receive_udp_message(message);
    // send_udp_message("NODES 35");
    // receive_udp_message(message);


    return 0;
}