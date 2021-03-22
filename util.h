#ifndef UTIL_H
#define UTIL_H

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#define NET_SIZE 3 //  00->99

bool check_IP(char *IP);
bool check_port(char *port);
bool check_net(char *net);
void *checked_calloc(size_t size, size_t n);

#endif