#ifndef UTIL_H
#define UTIL_H

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#define BUFFER_SIZE 1023


bool check_IP(char *IP);
bool check_port(char *port);
bool check_net(char *net);
int get_number_of_LF(char *string);
void *checked_calloc(size_t size, size_t n);
void *checked_realloc(void *ptr, size_t size);

#endif