#ifndef LIST_NODES
#define LIST_NODES

#include "protocol_support.h"


bool set_parameters(int argc, char *argv[]);
void get_info();
bool send_udp_message(char *message);
void receive_udp_message(char *message);
void open_UDP();
void close_UDP();

#endif