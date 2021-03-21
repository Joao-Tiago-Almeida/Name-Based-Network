#ifndef UDP_H
#define UDP_H

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

#include "util.h"

void set_parameters(int argc, char *argv[]);
void get_info();
bool send_udp_message(char *message);
char *receive_udp_message();
void open_UDP();
void close_UDP();

#endif