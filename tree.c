#include "tree.h"

#define ALLOCATION_OFFSET 2

struct contact
{
    char ip[16];
    char tcp[6];
};

struct table
{
    int socket;
    char *id;
    enum
    {
        external,
        internal,
        unknown
    } neighbour;
};

struct resizable_vect
{
    void *vector;
    unsigned int occupancy;
    unsigned int max_size;
};

struct node
{
    struct contact external_neighbour;
    struct contact recovery_contact;
    struct contact *list_internal_neighbours;
    unsigned short n_internal;

    struct resizable_vect *table;
};
struct node NODE; // only one node per application

void set_external_and_recovery(char *ip, char *tcp, char *recovery, int socket)
{
    // External neighbour
    sprintf(NODE.external_neighbour.ip, "%s", ip);
    sprintf(NODE.external_neighbour.tcp, "%s", tcp);
    // recovery contact
    sscanf(recovery, "EXTERN %s %s\n", NODE.recovery_contact.ip, NODE.recovery_contact.tcp);

    NODE.table = add_item_checkup(NODE.table, sizeof(struct table));
    ((struct table *)NODE.table->vector)[NODE.table->occupancy - 1].socket = socket;
    ((struct table *)NODE.table->vector)[NODE.table->occupancy - 1].neighbour = external;

}

void add_internal_neighbour(char *neighbour, int socket)
{
    // Internal neighbour
    if (strcmp(neighbour, "void") == 0) // there isnt any internal neighbour
        return;

    if (NODE.list_internal_neighbours == NULL)
    {
        NODE.list_internal_neighbours = (struct contact *)checked_calloc(1, sizeof(struct contact));
        NODE.n_internal = 0;
    }
    else
    {
        NODE.list_internal_neighbours = (struct contact *)checked_realloc(NODE.list_internal_neighbours, sizeof(struct contact) * (NODE.n_internal + 1));
    }
    sscanf(neighbour, "NEW %s %s\n",
           NODE.list_internal_neighbours[NODE.n_internal].ip,
           NODE.list_internal_neighbours[NODE.n_internal++].tcp);

    NODE.table = add_item_checkup(NODE.table, sizeof(struct table));
    ((struct table *)NODE.table->vector)[NODE.table->occupancy - 1].socket = socket;
    ((struct table *)NODE.table->vector)[NODE.table->occupancy - 1].neighbour = internal;

}

// Displays the external and recovery contacts
void show_topology()
{
    printf("[TOPOLOGY] Here are your contacts:\n");

    if (strlen(NODE.external_neighbour.ip) == 0)
    {
        printf("Sniff Sniff, I am alone on the network. I could call my recovery contact ... but that is me. Wanna join me and watch Peaky Blinders?\n");
        return;
    }

    printf("\t*\tRecovery Neighbour:\t%s:%s\n", NODE.recovery_contact.ip, NODE.recovery_contact.tcp);
    printf("\t*\tExternal Neighbour:\t%s:%s\n\n", NODE.external_neighbour.ip, NODE.external_neighbour.tcp);
}

char *get_external_neighbour_ip()
{
    return NODE.external_neighbour.ip;
}
char *get_external_neighbour_tcp()
{
    return NODE.external_neighbour.tcp;
}
char *get_recovery_contact_ip()
{
    return NODE.external_neighbour.ip;
}
char *get_recovery_contact_tcp()
{
    return NODE.external_neighbour.tcp;
}

// ensures that the vector has space
struct resizable_vect *add_item_checkup(struct resizable_vect *ptr, ssize_t size)
{
    if (ptr == NULL)
        ptr = (struct resizable_vect *)checked_calloc(1, sizeof(struct resizable_vect));

    if (ptr->vector == NULL)
    {
        ptr->vector = checked_calloc(ALLOCATION_OFFSET, size);
        ptr->max_size = ALLOCATION_OFFSET;
        ptr->occupancy = 0;
    }
    else if (ptr->occupancy == ptr->max_size)
    {
        ptr->max_size = ptr->max_size + ALLOCATION_OFFSET; // increase max
        ptr->vector = checked_realloc(ptr->vector, size * (ptr->max_size));
    }
    ptr->occupancy++;
    return ptr;
}