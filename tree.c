#include "tree.h"

#define ALLOCATION_OFFSET 2

// camada da topologia
struct contact
{
    char ip[16];
    char tcp[6];
};

// camada do encaminhamento
struct table
{
    int socket;
    char id[BUFFER_SIZE];
};

struct resizable_vect
{
    void *item;
    unsigned int occupancy;
    unsigned int max_size;
};

struct node
{
    // camada da topologia
    struct contact external_neighbour;
    struct contact recovery_contact;
    struct resizable_vect *list_internal_neighbours;
    unsigned short n_internal;

    // camda do encaminhamento
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
    ((struct table *)NODE.table->item)[NODE.table->occupancy - 1].socket = socket;
}

void add_internal_neighbour(char *ip, char *tcp, int socket)
{

    NODE.list_internal_neighbours = add_item_checkup(NODE.list_internal_neighbours, sizeof(struct contact));
    sprintf(((struct contact *)NODE.list_internal_neighbours->item)[NODE.list_internal_neighbours->occupancy - 1].ip, "%s", ip);
    sprintf(((struct contact *)NODE.list_internal_neighbours->item)[NODE.list_internal_neighbours->occupancy - 1].tcp, "%s", tcp);

    NODE.table = add_item_checkup(NODE.table, sizeof(struct table));
    ((struct table *)NODE.table->item)[NODE.table->occupancy - 1].socket = socket;
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

void show_table()
{
    char socket_[BUFFER_SIZE]; // auxiliar struct to remove -1 and replace to -
    // size_t n = 0;

    printf("Adjacency Table\n\tid\t|\tsocket\t  |\n");
    for (unsigned int i = 0; i < NODE.table->occupancy; i++)
    {
        // if ((n = strlen(((struct table *)NODE.table->item)[i].id)) != 0) // knows all information about the node
        // {
            // personalise personal socket
            sprintf(socket_, "%d", ((struct table *)NODE.table->item)[i].socket);
            if (strcmp(socket_, "-1") == 0) // found personal socket
                sprintf(socket_, "-");

            printf("\t%s\t|\t  %s\t  |\n",
                   ((struct table *)NODE.table->item)[i].id,
                   socket_);
        // }
    }
}

/**
 * 
*/
void update_table(int fd, char *id)
{
    char message[MESSAGE_SIZE];
    NODE.table = add_item_checkup(NODE.table, sizeof(struct table));
    int *diferent_sockets = checked_calloc(NODE.table->occupancy - 1, sizeof(int)); // worst case
    int number_diferent_sockets = 0;
    bool send_advertise = true;

    // iterates over the adjacency table
    for (unsigned int i = 0; i < NODE.table->occupancy; i++)
    {
        send_advertise = true;
        // iterates over connections who was already sent a message
        for (int j = 0; j < number_diferent_sockets; j++)
        {
            if (((struct table *)NODE.table->item)[i].socket == diferent_sockets[j])
            {
                send_advertise = false;
                break;
            }
        }
        if (!send_advertise)
            continue;

        // whether its me or the income socket
        if ((((struct table *)NODE.table->item)[i].socket == -1) || (((struct table *)NODE.table->item)[i].socket == fd))
            continue;

        sprintf(message, "ADVERTISE %s\n", id);
        Write(((struct table *)NODE.table->item)[i].socket, message, strlen(message));

        // save the connection
        diferent_sockets[number_diferent_sockets++] = ((struct table *)NODE.table->item)[i].socket;
    }
    free(diferent_sockets);

    // check if I know the id, and if a new neighbour is found, I will send my table
    bool direct_neighbour = false;
    for (unsigned int i = 0; i < NODE.table->occupancy; i++)
    {
        if ((fd == ((struct table *)NODE.table->item)[i].socket) && (strlen(((struct table *)NODE.table->item)[i].id) == 0))
        {
            // found new neighbour, gonna send my table
            for (unsigned int j = 0; j < NODE.table->occupancy; j++)
            {
                // whether its me or the income socket
                if ((((struct table *)NODE.table->item)[j].socket == -1) || (i == j))
                    continue;

                sprintf(message, "ADVERTISE %s\n", ((struct table *)NODE.table->item)[i].id);
                Write(fd, message, strlen(message));
            }
            direct_neighbour = true;
            sprintf(((struct table *)NODE.table->item)[i].id, "%s", id);
            break;
        }
    }

    if(direct_neighbour)
        return;

    // add node to my table
    NODE.table = add_item_checkup(NODE.table, sizeof(struct table));
    ((struct table *)NODE.table->item)[NODE.table->occupancy - 1].socket = fd;
    sprintf(((struct table *)NODE.table->item)[NODE.table->occupancy - 1].id, "%s", id);
    
}

void init(char *id)
{
    NODE.table = add_item_checkup(NODE.table, sizeof(struct table));

    ((struct table *)NODE.table->item)[0].socket = -1;
    sprintf(((struct table *)NODE.table->item)[0].id, "%s", id);
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

/** get the different chanels around the node
FD_SET()
              This macro adds the file descriptor fd to set.  Adding a
              file descriptor that is already present in the set is a
              no-op, and does not produce an error.
 */
int set_sockets(fd_set *rfds)
{
    if(NODE.table == NULL)  // does not have a table...
        return -1;

    int maxfd = 0;
    for (unsigned int i = 0; i < NODE.table->occupancy; i++)
    {
        if (((struct table *)NODE.table->item)[i].socket == -1) // personal connection in the adjancency table
           continue;

        FD_SET(((struct table *)NODE.table->item)[i].socket, rfds);
        maxfd = max(maxfd, ((struct table *)NODE.table->item)[i].socket);
    }
    return maxfd;
}

// ensures that the vector has space
struct resizable_vect *add_item_checkup(struct resizable_vect *ptr, ssize_t size)
{
    if (ptr == NULL)
        ptr = (struct resizable_vect *)checked_calloc(1, sizeof(struct resizable_vect));

    if (ptr->item == NULL)
    {
        ptr->item = checked_calloc(ALLOCATION_OFFSET, size);
        ptr->max_size = ALLOCATION_OFFSET;
        ptr->occupancy = 0;
    }
    else if (ptr->occupancy == ptr->max_size)
    {
        ptr->max_size = ptr->max_size + ALLOCATION_OFFSET; // increase max
        ptr->item = checked_realloc(ptr->item, size * (ptr->max_size));
    }
    ptr->occupancy++;
    return ptr;
}

// Iterates through the hash table and returns the set socket if a known connection was set, and -1 otherwise
int FD_ISKNOWN(fd_set *rfds)
{
    for (unsigned int i = 0; i < NODE.table->occupancy; i++)
    {
        if (FD_ISSET(((struct table *)NODE.table->item)[i].socket, rfds)!=0)
            return ((struct table *)NODE.table->item)[i].socket;
    }
    printf("Não está nada set né Zé\n");
    return -1;
}

void close_node()
{
    for (unsigned int i = 0; i < NODE.table->occupancy; i++)
    {
        if (((struct table *)NODE.table->item)[i].socket == -1) // personal connection in the adjancency table
           continue;

        Close(((struct table *)NODE.table->item)[i].socket);
    }
    free((struct table *)NODE.table->item);
    free(NODE.table);
}