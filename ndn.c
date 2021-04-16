#include <stdlib.h>
#include <stdio.h>

#include "nodes_list.h"
#include "tree.h"

void print_usage(char *program_name)
{
    printf("Usage: %s IP TCP regIP regUDP\n", program_name);
}

void choose_neighbour(char message[], char bootIP[], char bootTCP[], int n_neighbour)
{
    int *pos_LF_vect = (int *)checked_calloc(n_neighbour, sizeof(int));
    int neighbour = 1; // init for the situation when there is only one neighbour

    int count = 0;
    char buffer[BUFFER_SIZE];

    // get the position of the '\n'
    for (int i = 0; message[i] != '\0'; ++i)
    {
        if ('\n' == message[i])
            pos_LF_vect[count++] = i;
    }

    if (n_neighbour > 2) // in case there is more than one neighbour
    {
        printf("\nInsert the number of the neighboor that you want to connect to.\n");

        // print option list
        for (int i = 1; i < n_neighbour; ++i)
        {
            printf("%d. ", i);
            for (int j = pos_LF_vect[i - 1] + 1; j <= pos_LF_vect[i]; j++)
                printf("%c", message[j]);
        }

        neighbour = 0;
        do
        {
            printf("Neihgbour: ");
            fgets(buffer, BUFFER_SIZE, stdin);
            sscanf(buffer, "%d", &neighbour);
        } while (neighbour < 1 || neighbour >= n_neighbour);
    }

    memset(buffer, '\0', BUFFER_SIZE);
    for (int i = pos_LF_vect[neighbour - 1]; i < pos_LF_vect[neighbour]; i++)
    {
        sprintf(buffer, "%s%c", buffer, message[i]);
    }
    sscanf(buffer, "%s %s", bootIP, bootTCP);

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

    char buffer[MESSAGE_SIZE], command[MESSAGE_SIZE], net[BUFFER_SIZE], id[BUFFER_SIZE], bootIP[BUFFER_SIZE], bootTCP[BUFFER_SIZE], message[MESSAGE_SIZE + 1];
    char new_node_ip[BUFFER_SIZE], new_node_port[BUFFER_SIZE]; // IP and TCP for a node that is already in the tree
    int number_of_line_feed = 0;
    int fd_server, fd_client, fd_neighbour, newfd, n_select, maxfd=0;
    fd_set rfds;
    struct addrinfo hints, *res_server, *res_client;
    struct sockaddr_in addr_client;
    socklen_t addrlen;
    struct timeval timeout = {2 /*seconds*/, 0 /*milliseconds*/}; // TODO alterar, pq nao sabemos onde a sessão está

    fd_server = Socket(AF_INET, SOCK_STREAM, 0); //TCP socket
    fd_client = Socket(AF_INET, SOCK_STREAM, 0); //TCP socket

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;       //IPv4
    hints.ai_socktype = SOCK_STREAM; //TCP socket
    hints.ai_flags = AI_PASSIVE;

    Getaddrinfo(NULL, port, &hints, &res_server);

    Bind(fd_server, res_server->ai_addr, res_server->ai_addrlen);
    addrlen = (socklen_t)sizeof(addr_client);

    bool is_connected = false;

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

                if (n == 2) // ask for a neighbour
                {
                    if (number_of_line_feed == 1) // no one is on the net yet, I will be the first
                    {
                        state = connected;
                        break;
                    }
                    else
                        choose_neighbour(message, bootIP, bootTCP, number_of_line_feed);
                }
                else if (n == 4)
                    ; // ip:port already specified
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
            Connect(fd_client, res_client->ai_addr, res_client->ai_addrlen);
            sprintf(message, "NEW %s %s\n", ip, port);
            Write(fd_client, message, strlen(message)); // NEW IP TCP<LF>
            FD_ZERO(&rfds);
            FD_SET(fd_client, &rfds);
            if (Select(fd_client + 1, &rfds, (fd_set *)NULL, (fd_set *)NULL, &timeout)) // if time reach to the end, a = is returned
            {   
                Read(fd_client, message, BUFFER_SIZE); // EXTERN IP TCP<LF>
                init(id);
                set_external_and_recovery(bootIP, bootTCP, message, fd_client);
                // Check if message received is OK  TODO
                state = connected;
                sprintf(message, "ADVERTISE %s\n", id);
                Write(fd_client, message, strlen(message)); // ADVERTISE IP TCP<LF>
            }
            else
                state = lobby;
            break;
        }
        case connected:
        {
            if (!is_connected)
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
                is_connected = true;
            }

            FD_ZERO(&rfds);
            FD_SET(STDIN_FILENO, &rfds);    // keyboard
            maxfd = set_sockets(&rfds);     // existing sockets
            FD_SET(fd_server, &rfds);       // incoming sockets
            maxfd = max(maxfd, fd_server);
            n_select = Select(maxfd+1 , &rfds, (fd_set *)NULL, (fd_set *)NULL, (struct timeval *)NULL);
            for(;n_select;n_select--)
            {
                if (FD_ISSET(STDIN_FILENO, &rfds)) // stdin
                {
                    FD_CLR(STDIN_FILENO, &rfds);
                    fgets(buffer, BUFFER_SIZE, stdin);
                    memset(command, '\0', MESSAGE_SIZE);
                    sscanf(buffer, "%s", command);
                    if (strcmp(command, "create subname") == 0) // É criado um objeto cujo nome será da forma id.subname, em que id é o identificador do nó.
                    {
                        printf("Querias querias ... batatinhas com enguias\n");
                    }
                    else if (strcmp(command, "get name") == 0) // Inicia-se a pesquisa do objeto com o nome name. Este nome será da forma id.subname, em que id é o identificador de um nó e subname é o sub-nome do objeto atribuído pelo nó com identificador id.
                    {
                        printf("Querias querias ... batatinhas com enguias\n");
                    }
                    else if (strcmp(command, "show topology") == 0 || strcmp(command, "st") == 0) // Mostra os contactos do vizinho externo e do vizinho de recuperação.
                    {
                        show_topology();
                    }
                    else if (strcmp(command, "show routing") == 0 || strcmp(command, "sr") == 0) // Mostra a tabela de expedição do nó.
                    {
                        show_table();
                    }
                    else if (strcmp(command, "show cache") == 0 || strcmp(command, "sc") == 0) // Mostra os nomes dos objetos guardados na cache.
                    {
                        printf("Querias querias ... batatinhas com enguias\n");
                    }
                    else if (strcmp(command, "leave") == 0) // Saída do nó da rede.
                    {
                        printf("Querias querias ... batatinhas com enguias\n");
                    }
                    else if (strcmp(command, "exit") == 0) // Fecho da aplicação.
                    {
                        sprintf(message, "UNREG %s %s %s", net, ip, port);
                        send_udp_message(message);
                        close_node();
                        Close(fd_server);
                        exit(0);
                    }
                    else
                    {
                        printf("Invalid command. \n");
                    }
                }
                else if (FD_ISSET(fd_server, &rfds)) // fresh connections
                {
                    FD_CLR(fd_server, &rfds);

                    newfd = Accept(fd_server, (struct sockaddr *)&addr_client, &addrlen);

                    Read(newfd, message, MESSAGE_SIZE);
                    sscanf(message, "NEW %s %s\n", new_node_ip, new_node_port); // neighbour information

                    if (strlen(get_external_neighbour_ip()) == 0) // node without neighbours
                    {   
                        init(id);
                        // set my contacts
                        sprintf(message, "EXTERN %s %s\n", ip, port); // my recovery contact is ... myself
                        set_external_and_recovery(new_node_ip, new_node_port, message, newfd);
                    }
                    else
                    {
                        add_internal_neighbour(new_node_ip, new_node_port, newfd);
                    }
                    // send my external contact
                    sprintf(message, "EXTERN %s %s\n", get_external_neighbour_ip(), get_external_neighbour_tcp()); //envia o proprio contacto
                    Write(newfd, message, strlen(message));
                    // end of topology process

                }
                else if((fd_neighbour = FD_ISKNOWN(&rfds)) != -1)   // known connection
                {
                    FD_CLR(fd_neighbour, &rfds);
                    Read(fd_neighbour, message, BUFFER_SIZE);
                    sscanf(message, "ADVERTISE %s", id);
                    update_table(fd_neighbour, id);
                }
                else
                {
                    printf("Entrei no Else no Select() princial\n");
                }
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

    //  send_udp_message("UNREG 35 192.168.1.81 50000");
    // send_udp_message("UNREG 35 192.168.1.81 50001");
    network_interaction(argv[1], argv[2]);

    // char message[MESSAGE_SIZE+1];
    // send_udp_message("REG 35 192.168.1.81 50000");
    // // // receive_udp_message(message);
    // // // // send_udp_message("UNREG 35 192.168.1.81 50001");
    // // // // receive_udp_message(message);
    // // // // // send_udp_message("UNREG 35 192.168.1.75 56790");
    // // // // // receive_udp_message(message);
    // send_udp_message("NODES 35");
    // receive_udp_message(message);

    return 0;
}