#ifndef UTIL_H
#define UTIL_H

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#define MESSAGE_SIZE 1023
#define BUFFER_SIZE 255

#define DEBUG 0 // wheter it prints auxiliar messages in the terminal

bool break_name_into_id_and_subname(char *str, char *id, char* subname);
void *checked_calloc(size_t nitems, size_t size);
void *checked_realloc(void *ptr, size_t size);
bool check_IP(char *IPv4);
bool check_port(char *port);
int get_number_of_LF(char *string);

#endif