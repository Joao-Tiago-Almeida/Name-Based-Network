#ifndef TREE_H
#define TREE_H

#include "protocol_support.h"

typedef struct resizable_vect RESIZABLE_VECT;

void set_external_and_recovery(char *ip, char *tcp, char *recovery, int socket);
void add_internal_neighbour(char* internal, int socket);
char *get_external_neighbour_ip();
char *get_external_neighbour_tcp();
char *get_recovery_contact_ip();
char *get_recovery_contact_tcp();
void show_topology();

struct resizable_vect *add_item_checkup(struct resizable_vect *ptr, ssize_t size);

#endif