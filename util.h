#ifndef UTIL_H
#define UTIL_H

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#define MESSAGE_SIZE 1023
#define BUFFER_SIZE 255


bool check_IP(char *IP);
bool check_port(char *port);
bool check_net(char *net);
int get_number_of_LF(char *string);
void *checked_calloc(size_t nitems, size_t size);
void *checked_realloc(void *ptr, size_t size);

#endif