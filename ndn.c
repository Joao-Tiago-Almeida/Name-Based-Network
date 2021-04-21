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
    pos_LF_vect = NULL;
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

    char buffer[MESSAGE_SIZE], command[MESSAGE_SIZE], net[BUFFER_SIZE], local_id[BUFFER_SIZE], bootIP[BUFFER_SIZE], bootTCP[BUFFER_SIZE], message[MESSAGE_SIZE + 1], id_or_name[BUFFER_SIZE];
    char new_node_ip[BUFFER_SIZE], new_node_port[BUFFER_SIZE]; // IP and TCP for a node that is already in the tree
    int number_of_line_feed = 0, number_of_bytes_read=0;
    int fd_server, fd_client, fd_neighbour, newfd, n_select, maxfd=0;
    fd_set rfds;
    struct addrinfo hints, *res_server, *res_client;
    struct sockaddr_in addr_client;
    socklen_t addrlen;
    struct timeval timeout = {2 /*seconds*/, 0 /*milliseconds*/}; // TODO alterar, pq nao sabemos onde a sessão está
    bool needs_to_connect=false;

    fd_server = Socket(AF_INET, SOCK_STREAM, 0); //TCP socket

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;       //IPv4
    hints.ai_socktype = SOCK_STREAM; //TCP socket
    hints.ai_flags = AI_PASSIVE;

    Getaddrinfo(NULL, port, &hints, &res_server);

    Bind(fd_server, res_server->ai_addr, res_server->ai_addrlen);
    addrlen = (socklen_t)sizeof(addr_client);

    bool is_connected = false;
    bool machine_online = true;

    while (machine_online)
    {
        switch (state)
        {
        case lobby:
        {
            printf("[LOBBY] Should I stay or should I go? Available commands:\n");
            printf("\t>\tjoin net id\n");
            printf("\t>\tjoin net id bootIP bootTCP\n");
            printf("\t>\texit\n\n");
            memset(buffer, '\0', MESSAGE_SIZE);
            fgets(buffer, MESSAGE_SIZE, stdin);
            sscanf(buffer, "%s", command);
            if (strcmp(command, "exit") == 0)
            {
                state = disconnecting;
                break;
            }
            else if (strcmp(command, "join") == 0)
            {
                int n = sscanf(buffer, "%*s %s %s %s %s", net, local_id, bootIP, bootTCP);
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
            fd_client = Socket(AF_INET, SOCK_STREAM, 0); //TCP socket
            Getaddrinfo(bootIP, bootTCP, &hints, &res_client);
            Connect(fd_client, res_client->ai_addr, res_client->ai_addrlen);
            sprintf(message, "NEW %s %s\n", ip, port);
            Write(fd_client, message, strlen(message)); // NEW IP TCP<LF>
            FD_ZERO(&rfds);
            FD_SET(fd_client, &rfds);
            if (Select(fd_client + 1, &rfds, (fd_set *)NULL, (fd_set *)NULL, &timeout)) // if time reach to the end, a = is returned
            {   
                Read(fd_client, message); // EXTERN IP TCP<LF>
                node_init(local_id, ip, port);
                set_external_and_recovery(bootIP, bootTCP, message, fd_client);
                // Check if message received is OK  TODO
                send_my_table(fd_client);
                state = connected;
            }
            else
                state = lobby;
            break;
        }
        case connected:
        {
            if (!is_connected)
            {
                node_init(local_id, ip, port);

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

                printf("[CONNECTED] You are connected. Available commands:\n");
                printf("\t>\tcreate subname\n");
                printf("\t>\tget name\n");
                printf("\t>\tshow topology (st)\n");
                printf("\t>\tshow routing  (sr)\n");
                printf("\t>\tshow cache    (sc)\n");
                printf("\t>\tleave\n");
                printf("\t>\texit\n\n");
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
                    memset(buffer, '\0', MESSAGE_SIZE);
                    fgets(buffer, BUFFER_SIZE, stdin);
                    buffer[strlen(buffer)-1] = '\0';    // replace \n
                    if (strncmp(buffer, "create subname", 7) == 0) // É criado um objeto cujo nome será da forma id.subname, em que id é o identificador do nó.
                    {   
                        sscanf(buffer, "%*s %s", message);
                        update_cache(message);
                    }
                    else if (strncmp(buffer, "get name", 4) == 0) // Inicia-se a pesquisa do objeto com o nome name. Este nome será da forma id.subname, em que id é o identificador de um nó e subname é o sub-nome do objeto atribuído pelo nó com identificador id.
                    {
                        sscanf(buffer, "%*s %s", message);
                        search_object(message, STDOUT_FILENO, false);
                    }
                    else if (strcmp(buffer, "show topology") == 0 || strcmp(buffer, "st") == 0) // Mostra os contactos do vizinho externo e do vizinho de recuperação.
                    {
                        show_topology();
                    }
                    else if (strcmp(buffer, "show routing") == 0 || strcmp(buffer, "sr") == 0) // Mostra a tabela de expedição do nó.
                    {
                        show_table();
                    }
                    else if (strcmp(buffer, "show cache") == 0 || strcmp(buffer, "sc") == 0) // Mostra os nomes dos objetos guardados na cache.
                    {
                        show_cache();
                    }
                    else if (strcmp(buffer, "leave") == 0) // Saída do nó da rede.
                    {
                        sprintf(message, "UNREG %s %s %s", net, ip, port);
                        send_udp_message(message);
                        receive_udp_message(message);
                        close_node();
                        is_connected = false;
                        state=lobby;
                    }
                    else if (strcmp(buffer, "exit") == 0) // Fecho da aplicação.
                    {
                        sprintf(message, "UNREG %s %s %s", net, ip, port);
                        send_udp_message(message);
                        receive_udp_message(message);
                        close_node();
                        state = disconnecting;
                    }
                    else
                    {
                        printf("Invalid command: '%s'.\n", buffer);
                    }
                }
                else if (FD_ISSET(fd_server, &rfds)) // fresh connections
                {
                    FD_CLR(fd_server, &rfds);

                    newfd = Accept(fd_server, (struct sockaddr *)&addr_client, &addrlen);

                    Read(newfd, message);
                    sscanf(message, "NEW %s %s\n", new_node_ip, new_node_port); // neighbour information

                    if (strlen(get_external_neighbour_ip()) == 0) // node without neighbours
                    {   
                        // set my contacts
                        sprintf(message, "EXTERN %s %s\n", ip, port); // my recovery contact is ... myself
                        set_external_and_recovery(new_node_ip, new_node_port, message, newfd);
                    }
                    else
                    {
                        add_internal_neighbour(new_node_ip, new_node_port, newfd);
                    }
                    // send my external contact
                    sprintf(message, "EXTERN %s %s\n", get_external_neighbour_ip(), get_external_neighbour_tcp()); // envia o proprio contacto
                    Write(newfd, message, strlen(message));
                    send_my_table(newfd);
                    // end of topology process

                }
                else if((fd_neighbour = FD_ISKNOWN(&rfds)) != -1)   // known connection
                {
                    FD_CLR(fd_neighbour, &rfds);
                    number_of_bytes_read = Read(fd_neighbour, message);
                    if(number_of_bytes_read==0) 
                    {   
                        remove_direct_neighbour(fd_neighbour);
                        withdraw_update_table(fd_neighbour, "\0", true);
                        needs_to_connect = reconnect_network(fd_neighbour, bootIP, bootTCP);
                        Close(fd_neighbour);
                        fd_neighbour = -1;
                        if(needs_to_connect)
                            state = connecting;
                        continue;
                    }

                    sscanf(message, "%s %s", command, id_or_name); 
                    if(strcmp(command, "ADVERTISE") == 0)   //  Anúncio do nó com identificador id.
                    {
                        broadcast_advertise(fd_neighbour, id_or_name);
                    }
                    else if(strcmp(command, "WITHDRAW") == 0)   //  Retirada do nó com identificador id.
                    {
                        withdraw_update_table(fd_neighbour, id_or_name, false);
                    }
                    else if(strcmp(command, "INTEREST") == 0)   //  Pesquisa do objeto com nome name.
                    {
                        search_object(id_or_name, fd_neighbour, true);
                    }
                    else if(strcmp(command, "DATA") == 0 || strcmp(command, "NODATA") == 0)   //  Entrega do objeto com nome name.
                    {
                        return_search(command, id_or_name);
                    }
                    else if(strcmp(command, "EXTERN") == 0) // fomos promovidos a vizinhos externos, atualizo o meu vizinho exetrno que se tudo correr so eu próprio
                    {   
                        sscanf(message, "%*s %s %s", new_node_ip, new_node_port); 
                        update_recovery_contact(new_node_ip, new_node_port);
                    }
                }
                else
                {
                    printf("Entrei no Else no Select() principal\n");
                }
            }
            break;
        }
        case disconnecting:
        {
            close_UDP();
            Close(fd_server);
            fd_server = -1;
            Close(fd_client);
            fd_client = -1;
            freeaddrinfo(res_server);
            res_server = NULL;
            printf("See you later alligator!\n");
            machine_online = false;
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

    exit(0);

    return 0;
}