#include "tree.h"

#define ALLOCATION_OFFSET 2
#define CACHE_SIZE 5

// camada da topologia
struct contact
{
    char ip[16];
    char tcp[6];
};

struct cache_node
{
    char subname[BUFFER_SIZE];
    struct cache_node *next;
    struct cache_node *previous;
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

struct message_path
{
    int socket;
    char name[BUFFER_SIZE];
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
    struct cache_node *head;
    struct cache_node *tail;
    struct resizable_vect *income_messages; 
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
    size_t n = 0;

    printf("Adjacency Table\n\tid\t|\tsocket\t  |\n");
    for (unsigned int i = 0; i < NODE.table->occupancy; i++)
    {
        if ((n = strlen(((struct table *)NODE.table->item)[i].id)) != 0) // knows all information about the node
        {
            // personalise personal socket
            sprintf(socket_, "%d", ((struct table *)NODE.table->item)[i].socket);
            if (strcmp(socket_, "-1") == 0) // found personal socket
                sprintf(socket_, "-");

            printf("\t%s\t|\t  %s\t  |\n",
                   ((struct table *)NODE.table->item)[i].id,
                   socket_);
        }
    }
    printf("OCCUPANCY: %d\t ALLOCATED_SIZE: %d\n", NODE.table->occupancy, NODE.table->max_size);
}

/**
 * 
*/
void advertise_update_table(int fd, char *id, bool *advertise_connecting)
{
    char message[MESSAGE_SIZE];
    int *diferent_sockets = checked_calloc(NODE.table->occupancy - 1, sizeof(int)); // worst case
    int number_diferent_sockets = 0;
    bool send_advertise = true;

    // iterates over the adjacency table
    for (unsigned int i = 0; i < NODE.table->occupancy; i++)
    {
        // whether its me or the income socket
        if ((((struct table *)NODE.table->item)[i].socket == -1) || (((struct table *)NODE.table->item)[i].socket == fd))
            continue;

        send_advertise = true;
        // iterates over connections who were already sent a message
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

        sprintf(message, "ADVERTISE %s\n", id);
        Write(((struct table *)NODE.table->item)[i].socket, message, strlen(message));

        // save the connection
        diferent_sockets[number_diferent_sockets++] = ((struct table *)NODE.table->item)[i].socket;
    }
    free(diferent_sockets);

    // check if I know the id, and if a new neighbour is found I will send my table
    bool direct_neighbour = false;
    for (unsigned int i = 0; i < NODE.table->occupancy; i++)
    {
        if ((fd != ((struct table *)NODE.table->item)[i].socket) || (strlen(((struct table *)NODE.table->item)[i].id) != 0))
            continue;
        // found new neighbour, gonna send my table

        for (unsigned int j = 0; j < NODE.table->occupancy; j++)
        {
            // whether its me or the income socket
            if (fd == ((struct table *)NODE.table->item)[j].socket || *advertise_connecting) // i == j
                continue;

            sprintf(message, "ADVERTISE %s\n", ((struct table *)NODE.table->item)[j].id);
            Write(fd, message, strlen(message));
        }
        if (*advertise_connecting)
            *advertise_connecting = false;
        direct_neighbour = true;
        sprintf(((struct table *)NODE.table->item)[i].id, "%s", id);
        break;
    }

    if (direct_neighbour)
        return;

    // add node to my table
    NODE.table = add_item_checkup(NODE.table, sizeof(struct table));
    ((struct table *)NODE.table->item)[NODE.table->occupancy - 1].socket = fd;
    sprintf(((struct table *)NODE.table->item)[NODE.table->occupancy - 1].id, "%s", id);
}

void withdraw_update_table(int fd, char *id, bool detected_withdraw)
{
    // get the closed socket

    char message[MESSAGE_SIZE];
    char(*exit_id)[MESSAGE_SIZE];
    exit_id = checked_calloc(NODE.table->occupancy - 1, sizeof(exit_id)); // worst case
    int number_exit_id = 0;
    int *diferent_sockets = checked_calloc(NODE.table->occupancy - 1, sizeof(int)); // worst case
    int number_diferent_sockets = 0;
    bool send_withdraw = false;

    if (detected_withdraw)
    {
        // get and delete the nodes that have the same socket of the lost node in my table
        for (unsigned int i = 0; i < NODE.table->occupancy; i++)
        {
            if (((struct table *)NODE.table->item)[i].socket != fd)
                continue;

            sprintf(exit_id[number_exit_id++], "%s", ((struct table *)NODE.table->item)[i].id);

            // put the last element in the new free place
            ((struct table *)NODE.table->item)[i].socket = ((struct table *)NODE.table->item)[NODE.table->occupancy - 1].socket;
            sprintf(((struct table *)NODE.table->item)[i].id, "%s", ((struct table *)NODE.table->item)[NODE.table->occupancy - 1].id);

            // reset the id of the last position
            sprintf(((struct table *)NODE.table->item)[NODE.table->occupancy - 1].id, "");
            NODE.table->occupancy--;
            i--;
        }

        // send withdraw message to all the remaining sockets
        for (unsigned int i = 0; i < NODE.table->occupancy; i++)
        {
            // local socket
            if (((struct table *)NODE.table->item)[i].socket == -1)
                continue;

            send_withdraw = true;
            // iterates over connections who were already sent a message
            for (int j = 0; j < number_diferent_sockets; j++)
            {
                if (((struct table *)NODE.table->item)[i].socket == diferent_sockets[j])
                {
                    send_withdraw = false;
                    break;
                }
            }
            if (!send_withdraw)
                continue;

            // send withdraw messaeges
            for (int e = 0; e < number_exit_id; e++)
            {
                sprintf(message, "WITHDRAW %s\n", exit_id[e]);
                Write(((struct table *)NODE.table->item)[i].socket, message, strlen(message));
            }

            // save the connection
            diferent_sockets[number_diferent_sockets++] = ((struct table *)NODE.table->item)[i].socket;
        }
    }
    else
    {
        // iterates over the adjacency table
        for (unsigned int i = 0; i < NODE.table->occupancy; i++)
        {
            // owm socket
            if (((struct table *)NODE.table->item)[i].socket == -1)
                continue;

            if (strcmp(((struct table *)NODE.table->item)[i].id, id) == 0)
            {
                // put the last element in the new free place
                ((struct table *)NODE.table->item)[i].socket = ((struct table *)NODE.table->item)[NODE.table->occupancy - 1].socket;
                sprintf(((struct table *)NODE.table->item)[i].id, "%s", ((struct table *)NODE.table->item)[NODE.table->occupancy - 1].id);

                // reset the id of the last position
                sprintf(((struct table *)NODE.table->item)[NODE.table->occupancy - 1].id, "");
                NODE.table->occupancy--;
                i--;
            }
            else if (((struct table *)NODE.table->item)[i].socket != fd) // update the other connections
            {
                send_withdraw = true;
                // iterates over connections who were already sent a message
                for (int j = 0; j < number_diferent_sockets; j++)
                {
                    if (((struct table *)NODE.table->item)[i].socket == diferent_sockets[j])
                    {
                        send_withdraw = false;
                        break;
                    }
                }
                if (!send_withdraw)
                    continue;

                sprintf(message, "WITHDRAW %s\n", id);
                Write(((struct table *)NODE.table->item)[i].socket, message, strlen(message));

                // save the connection
                diferent_sockets[number_diferent_sockets++] = ((struct table *)NODE.table->item)[i].socket;
            }
        }
    }
    free(diferent_sockets);
    free(exit_id);
}

void node_init(char *id)
{
    if (NODE.table != NULL)
        return;

    NODE.table = add_item_checkup(NODE.table, sizeof(struct table));

    ((struct table *)NODE.table->item)[0].socket = -1;
    sprintf(((struct table *)NODE.table->item)[0].id, "%s", id);

    init_cache();
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
    if (NODE.table == NULL) // does not have a table...
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
        if (FD_ISSET(((struct table *)NODE.table->item)[i].socket, rfds) != 0)
            return ((struct table *)NODE.table->item)[i].socket;
    }
    return -1;
}

void init_cache()
{
    //head vai ser o mais recentemente utilizado
    NODE.head = NULL;
    //tail vai ser o que não é utilizado há mais tempo
    NODE.tail = NULL;

    for (int i = 0; i < CACHE_SIZE; i++)
    {
        struct cache_node *new_node = (struct cache_node *)checked_calloc(1, sizeof(struct cache_node));
        sprintf(new_node->subname, "");
        new_node->next = NODE.head;
        new_node->previous = NULL;
        if (NODE.head != NULL)
            NODE.head->previous = new_node;
        NODE.head = new_node;
        //como estou a adicionar no início a tail vai ser o primeiro elemento a ser adicionado
        if (i == 0)
            NODE.tail = NODE.head;
    }
}

// LSR
void update_cache(char *subname)
{       
    struct cache_node *ptr = search_my_cache(subname);
    if(ptr==NULL)   // object does not exist in my cache
    {
        sprintf(NODE.tail->subname, "%s", subname);
        // criar ligações
        NODE.tail->next = NODE.head;
        NODE.head->previous=NODE.tail;
        // atualizar ponteiros
        NODE.tail = NODE.tail->previous;
        NODE.head=NODE.head->previous;
        // quebrar ligações
        NODE.tail->next=NULL;
        NODE.head->previous=NULL;
    }
    else    
    {   
        // Remove from the middle
        if(ptr->previous==NULL) // it already is the recently
            return;

        ptr->previous->next = ptr->next;
        ptr->next->previous = ptr->previous;
        // According to the Least Recently Used(LRU)
        ptr->next = NODE.head;
        ptr->previous = NULL;
        // update HEAD
        NODE.head->previous = ptr;
        NODE.head = ptr;
    }
}

void search_object(char *name, int socket, bool waiting_for_object)
{   
    char id[BUFFER_SIZE];
    char subname[BUFFER_SIZE];
    char message[MESSAGE_SIZE];

    bool valid_name = break_name_into_id_and_subname(name, id, subname);
    if(!valid_name)
    {
        printf("The name you entered is not valid, try again soldier.\n");
        return;
    }

    // search in my own cache
    struct cache_node * ptr = search_my_cache(subname);
    if(ptr != NULL)
    {
        if(waiting_for_object)
        {
            sprintf(message, "DATA %s.%s\n", id, subname);
            Write(socket, message, strlen(message));
        }
        else
        {
            sprintf(message, "%s", subname);
            printf("I received an object titled: '%s'.\n", subname);
        }
        update_cache(message);
    }

    // I am the one to whom the message was intended, if I did not find the object the search will end unsuccessfully
    if(strcmp(id, ((struct table *)NODE.table->item)[0].id)==0)
    {
        sprintf(message, "NODATA %s.%s\n", id, subname);
        Write(socket, message, strlen(message));
        return;
    }

    for (unsigned int i = 0; i < NODE.table->occupancy; i++)
    {   
        if(strcmp(((struct table *)NODE.table->item)[i].id, id != 0))
            continue;

        sprintf(message, "INTEREST %s.%s\n", id, subname);
        Write(((struct table *)NODE.table->item)[i].socket, message, strlen(message));

        NODE.income_messages = add_item_checkup(NODE.income_messages, sizeof(struct message_path));
        ((struct message_path *)NODE.income_messages->item)[NODE.income_messages->occupancy - 1].socket = socket;
        sprintf(((struct message_path *)NODE.income_messages->item)[NODE.income_messages->occupancy - 1].name, "%s", subname);   
    }
        
}

void show_cache()
{
    struct cache_node* ptr=NODE.head;
    int count = 0;

    printf("Cache\n");
    while(ptr!=NULL)
    {   
        if(strlen(ptr->subname)!=0)
            printf("%d. %s\n", ++count, ptr->subname);

        ptr=ptr->next;
    }
    printf("OCCUPANCY: %d\t CACHE_SIZE: %d\n", count, CACHE_SIZE);
}

struct cache_node *search_my_cache(char *subname)
{
    struct cache_node* ptr=NODE.head;

    while(ptr!=NULL)
    {   
        if(strcmp(subname, ptr->subname)==0)    // found subname
            return ptr;
        ptr=ptr->next;
    }
    return NULL;
}

int get_waiting_node(char* name)
{
    for (unsigned int i = 0; i < NODE.income_messages->occupancy; i++)
    {
        if(strcmp(name, ((struct message_path *)NODE.income_messages->item)[i].name))
            continue;
        
        return i;
    }
    return -1;
}

void close_node()
{

    // close all sockets
    int *diferent_sockets = checked_calloc(NODE.table->occupancy - 1, sizeof(int)); // worst case
    int number_diferent_sockets = 0;

    for (unsigned int i = 0; i < NODE.table->occupancy; i++)
    {
        for (int j = 0; j < number_diferent_sockets; j++)
        {
            if (((struct table *)NODE.table->item)[i].socket == diferent_sockets[j])
            {
                Close(((struct table *)NODE.table->item)[i].socket);
            }
        }
    }
    free(diferent_sockets);
    free((struct table *)NODE.table->item);
    free(NODE.table);
    NODE.table = NULL;

    // free cache
   struct cache_node* tmp;

   while (NODE.head != NULL)
    {
       tmp = NODE.head;
       NODE.head = NODE.head->next;
       free(tmp);
    }
}