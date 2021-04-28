#ifndef LIST_NODES
#define LIST_NODES

#include "protocol_support.h"

void close_UDP();
void get_info();
void open_UDP();
void receive_udp_message(char *message);
void send_udp_message(char *message);
bool set_parameters(int argc, char *argv[]);

#endif