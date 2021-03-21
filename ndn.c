#include <stdlib.h>
#include <stdio.h>

#include "udp.h"

void print_usage(char *program_name)
{
    printf("Usage: %s IP TCP regIP regUDP\n", program_name);
}

int main(int argc, char *argv[])
{

    if (argc < 3 || argc > 5)
    {
        fprintf(stderr, "Invalid number of arguments\n");
        print_usage(argv[0]);
        return 1;
    }

    set_parameters(argc, argv);
    get_info();

    char net[] = "35";
    char message[100];

    strcpy(message, "NODES ");
    strcat(message, net);
    send_udp_message(message);
    receive_udp_message();

    strcpy(message, "REG ");
    strcat(message, net);
    strcat(message, " ");
    strcat(message, argv[1]);
    strcat(message, " ");
    strcat(message, argv[2]);
    send_udp_message(message);
    receive_udp_message();

    strcpy(message, "NODES ");
    strcat(message, net);
    send_udp_message(message);
    receive_udp_message();

    strcpy(message, "UNREG ");
    strcat(message, net);
    strcat(message, " ");
    strcat(message, argv[1]);
    strcat(message, " ");
    strcat(message, argv[2]);
    send_udp_message(message);
    receive_udp_message();

    strcpy(message, "NODES ");
    strcat(message, net);
    send_udp_message(message);
    receive_udp_message();

    return 0;
}