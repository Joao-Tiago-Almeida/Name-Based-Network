#include <stdlib.h>
#include <stdio.h>

#include "list_nodes.h"
#include "server.h"

#define STRING_SIZE 128
#define ID_SIZE 3

void print_usage(char *program_name)
{
    printf("Usage: %s IP TCP regIP regUDP\n", program_name);
}

void get_command()
{
    char buf[STRING_SIZE];
    char command_type[7];

    fgets(buf, STRING_SIZE, stdin);
    sscanf(buf, "%s %[^\n]", command_type, buf);

    printf("command type: %s\n", command_type);
    printf("left info in buf: %s\n", buf);

    if (!strcmp(command_type, "join"))
    {
        char net[NET_SIZE];
        char id[ID_SIZE];
        int buf_len = (int)strlen(buf);
        sscanf(buf, "%s %s %[^\n]", net, id, buf);

        if (!check_net(net))
            exit(1);

        if (buf_len == (int)strlen(buf)) //  command:   join net id
        {
            /*  ask UDP List for a new connection */
        }
        else //  command:    join net id bootIP bootTCP
        {
            char bootIP[IP_SIZE];
            char bootTCP[PORT_SIZE];
            sscanf(buf, "%s %s %[^\n]", bootIP, bootTCP, buf);

            if (!check_IP(bootIP) && !check_port(bootTCP))
                exit(1);

            /*  start a TCP connection */
        }
        
    }
    //  TODO    -   só aplicar estes comandos quando o nó já está registado
    else if (!strcmp(command_type, "create")) //  command:    create subname
    {
        /* char *subname = buf */
        /* Create a subname (file) */
    }
    else if (!strcmp(command_type, "get")) //  command:    get name
    {
        /* char *name = buf */
        /* Search for object name*/
    }
    else if (!strcmp(command_type, "show") || !strcmp(command_type, "st") || !strcmp(command_type, "sr") || !strcmp(command_type, "sc"))
    {
        if (!strcmp(buf, "topology") || !strcmp(command_type, "st"))
        {
            /* show topology */
        }
        else if (!strcmp(buf, "routing") || !strcmp(command_type, "sr"))
        {
            /* show routing */
        }
        else if (!strcmp(buf, "cache") || !strcmp(command_type, "sc"))
        {
            /* show cache */
        }
    }
    else if (!strcmp(command_type, "leave")) //  command:    leave
    {
        /* leave the network */
        printf("Leaving the network safetly! :)\n");
    }
    else if (!strcmp(command_type, "exit")) //  command:    exit
    {
        /* close app */
        server_down();
        printf("Exit the program safetly! :)\n");
    }
    else
    {
        printf("Unkonwn command, maybe in a parallel life I understand black magic ;).\n");
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

    server_up(argv[2]);

    send_udp_message("NODES 35");
    receive_udp_message();

    get_command();

    return 0;
}