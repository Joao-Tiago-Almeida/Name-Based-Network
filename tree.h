#ifndef TREE_H
#define TREE_H

#include "protocol_support.h"

#define max(A,B) ((A)>=(B)?(A):(B))

// typedef struct resizable_vect RESIZABLE_VECT;
// typedef struct cache_node CACHE_NODE;

void set_external_and_recovery(char *ip, char *tcp, char *recovery, int socket);
void add_internal_neighbour(char *ip, char *tcp, int socket);
char *get_external_neighbour_ip();
char *get_external_neighbour_tcp();
char *get_recovery_contact_ip();
char *get_recovery_contact_tcp();
void show_topology();
void show_table();
void show_cache();
void advertise_update_table(int fd, char* id, bool *advertise_connecting);
void withdraw_update_table(int fd, char *id, bool detected_withdraw);
void init_cache();
void update_cache(char* subname);
void search_object(char *subname, int socket, bool waiting_for_object);
int get_waiting_node(char* name);
struct cache_node *search_my_cache(char* subname);
void node_init(char* id);
int set_sockets(fd_set* rfds);
int FD_ISKNOWN(fd_set *rfds);
struct resizable_vect *add_item_checkup(struct resizable_vect *ptr, ssize_t size);
void close_node();

#endif