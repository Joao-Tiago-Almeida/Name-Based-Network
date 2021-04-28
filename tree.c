#include "tree.h"

#define ALLOCATION_OFFSET 3
#define CACHE_SIZE 2

// topology - contact struct
struct contact
{
    char ip[16];
    char tcp[6];
    int socket;
};

// routing - routing table entry struct
struct table
{
    int socket;
    char id[BUFFER_SIZE];
};

// allocation auxiliar struct
struct resizable_vect
{
    void *item;
    unsigned int occupancy;
    unsigned int max_size;
};

// cache information
struct cache_node
{
    char subname[BUFFER_SIZE];
    struct cache_node *next;
    struct cache_node *previous;
};

// forwarding auxiliar struct
struct message_path
{
    int socket;
    char name[BUFFER_SIZE];
};

// all node information
struct node
{
    // topology
    struct contact me;
    struct contact external_neighbour;
    struct contact recovery_contact;
    struct resizable_vect *list_internal_neighbours;
    struct resizable_vect *direct_neighbours;
    // routing
    struct resizable_vect *table;
    struct cache_node *head;
    struct cache_node *tail;
    struct resizable_vect *income_messages;
};

// only one node declared per application
struct node NODE; 

/**
 * Adds a new connection as am external neighbour.
 * Sets all external neighbour and recovery contact information in the node struct.
 * @param   ip          IPv4 of ones node
 * @param   tcp         TCP port of ones node
 * @param   recovery    Information about my recovery contact
 * @param   socket      End point of the connection
*/
void set_external_and_recovery(char *ip, char *tcp, char *recovery, int socket)
{
    // set external neighbour information
    sprintf(NODE.external_neighbour.ip, "%s", ip);
    sprintf(NODE.external_neighbour.tcp, "%s", tcp);
    NODE.external_neighbour.socket = socket;
    
    // set recovery neighbour information
    sscanf(recovery, "EXTERN %s %s\n", NODE.recovery_contact.ip, NODE.recovery_contact.tcp);

    // insert external neighbour into the routing table, no knowledge of id yet
    NODE.table = add_item_checkup(NODE.table, sizeof(struct table));
    ((struct table *)NODE.table->item)[NODE.table->occupancy - 1].socket = socket;
    
    // insert external neighbour into the list of direct neighbours
    NODE.direct_neighbours = add_item_checkup(NODE.direct_neighbours, sizeof(int));
    ((int *)NODE.direct_neighbours->item)[NODE.direct_neighbours->occupancy - 1] = socket;
}

/**
 * Adds a new connection as an internal neighbour.
 * Sets all internal neighbour information in the node struct.
 * @param   ip          IPv4 of ones node
 * @param   tcp         TCP port of ones node
 * @param   socket      End point of the connection
*/
void add_internal_neighbour(char *ip, char *tcp, int socket)
{
    // insert internal neighbour into the list of internal neighbours
    NODE.list_internal_neighbours = add_item_checkup(NODE.list_internal_neighbours, sizeof(struct contact));
    sprintf(((struct contact *)NODE.list_internal_neighbours->item)[NODE.list_internal_neighbours->occupancy - 1].ip, "%s", ip);
    sprintf(((struct contact *)NODE.list_internal_neighbours->item)[NODE.list_internal_neighbours->occupancy - 1].tcp, "%s", tcp);
    ((struct contact *)NODE.list_internal_neighbours->item)[NODE.list_internal_neighbours->occupancy - 1].socket = socket;

    // insert internal neighbour into the routing table, no knowledge of id yet
    NODE.table = add_item_checkup(NODE.table, sizeof(struct table));
    ((struct table *)NODE.table->item)[NODE.table->occupancy - 1].socket = socket;

    // insert internal neighbour in the list of direct neighbours
    NODE.direct_neighbours = add_item_checkup(NODE.direct_neighbours, sizeof(int));
    ((int *)NODE.direct_neighbours->item)[NODE.direct_neighbours->occupancy - 1] = socket;
}

/**
 * Updates recovery contact information.
 * @param   ip      IPv4 of the recovery contact
 * @param   tcp     TCP port of othe recovery contact
*/
void update_recovery_contact(char *ip, char *tcp)
{
    sprintf(NODE.recovery_contact.ip, "%s", ip);
    sprintf(NODE.recovery_contact.tcp, "%s", tcp);
}

/**
 * Prints the external and recovery contacts on the terminal.
*/
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

/**
 * Prints the adjacency table on the terminal
*/
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
 * Send advertise of every element in my table, most likely when a new conncetion occurs
 * @param   socket      Socket to where the node will send the entirely table
*/
void send_my_table(int socket)
{
    char message[MESSAGE_SIZE];

    for (unsigned int i = 0; i < NODE.table->occupancy; i++)
    {
        if ((socket != ((struct table *)NODE.table->item)[i].socket) || (strlen(((struct table *)NODE.table->item)[i].id) != 0))
            continue;
        for (unsigned int j = 0; j < NODE.table->occupancy; j++)
        {
            // incoming socket
            if (((struct table *)NODE.table->item)[j].socket == socket)
                continue;
            // node not identified yet
            else if(strlen(((struct table *)NODE.table->item)[j].id) == 0)
                continue;
            sprintf(message, "ADVERTISE %s\n", ((struct table *)NODE.table->item)[j].id);
            Write(socket, message, strlen(message));
        }
        break;
    }
}

/**
 * Shares the information about a new know with the neighbour nodes
 * @param  socket   Socket from where the avertise came from
 * @param  id       Id of the new node in the network
*/
void broadcast_advertise(int socket, char *id)
{
    char message[MESSAGE_SIZE];
    bool direct_neighbour = false;

    // iterates over the adjacency table
    for (unsigned int i = 0; i < NODE.direct_neighbours->occupancy; i++)
    {
        // whether its me or the income socket
        if (((int *)NODE.direct_neighbours->item)[i] == -1)
            continue;

        // verify if is a new neighbour and if so, update its contact
        else if (((int *)NODE.direct_neighbours->item)[i] == socket)
        {
            for (unsigned int j = 0; j < NODE.table->occupancy; j++)
            {
                // search for the income sockect
                if (((struct table *)NODE.table->item)[j].socket != socket || strlen(((struct table *)NODE.table->item)[j].id) != 0) // socket_i != socket_j or the id is know
                    continue;

                sprintf(((struct table *)NODE.table->item)[j].id, "%s", id);
                direct_neighbour = true;
                break;
            }
            continue;
        }

        sprintf(message, "ADVERTISE %s\n", id);
        Write(((int *)NODE.direct_neighbours->item)[i], message, strlen(message));
    }

    if (direct_neighbour)
        return;

    // add node to my table
    NODE.table = add_item_checkup(NODE.table, sizeof(struct table));
    ((struct table *)NODE.table->item)[NODE.table->occupancy - 1].socket = socket;
    sprintf(((struct table *)NODE.table->item)[NODE.table->occupancy - 1].id, "%s", id);
}

/**
 * Removes all table entries with the same socket as the leaving node and broadcasts WITHDRAW.
 * @param   socket              Direct end point that was closed
 * @param   id                  Identifier of leaving nodes
 * @param   detected_withdraw   Identifies where it was the one that noticed a closed connection or it was received a WITHDRAW message
 * 
*/
void withdraw_update_table(int socket, char *id, bool detected_withdraw)
{
    char message[MESSAGE_SIZE];
    unsigned int allocation_size = NODE.table->occupancy - 1;
    char **exit_id = checked_calloc(NODE.table->occupancy - 1, sizeof(char *)); // worst case
    for (unsigned int i = 0; i < allocation_size; i++)
    {
        exit_id[i] = checked_calloc(MESSAGE_SIZE, sizeof(char));
    }

    int number_exit_id = 0;

    if (detected_withdraw)
    {
        // get and delete the nodes that have the same socket of the lost node in my table
        for (unsigned int i = 0; i < NODE.table->occupancy; i++)
        {
            if (((struct table *)NODE.table->item)[i].socket != socket)
                continue;
            // stores the id of the nodes that we are removing from our table, to broadcast them next
            sprintf(exit_id[number_exit_id++], "%s", ((struct table *)NODE.table->item)[i].id);

            // put the last element in the new free place
            ((struct table *)NODE.table->item)[i].socket = ((struct table *)NODE.table->item)[NODE.table->occupancy - 1].socket;
            sprintf(((struct table *)NODE.table->item)[i].id, "%s", ((struct table *)NODE.table->item)[NODE.table->occupancy - 1].id);

            // reset the id of the last position
            memset(((struct table *)NODE.table->item)[NODE.table->occupancy - 1].id, '\0', BUFFER_SIZE);
            NODE.table->occupancy--;
            i--;
        }

        // send withdraw message to all the remaining sockets
        for (unsigned int i = 0; i < NODE.direct_neighbours->occupancy; i++)
        {
            // withdraw socket
            if (((int *)NODE.direct_neighbours->item)[i] == socket)
                continue;

            // send withdraw messages
            for (int e = 0; e < number_exit_id; e++)
            {
                sprintf(message, "WITHDRAW %s\n", exit_id[e]);
                Write(((int *)NODE.direct_neighbours->item)[i], message, strlen(message));
            }
        }
    }
    else
    {
        // iterates over the adjacency table
        for (unsigned int i = 0; i < NODE.table->occupancy; i++)
        {
            if (strcmp(((struct table *)NODE.table->item)[i].id, id) != 0)
                continue; // continues if the id is not to be removed

            // put the last element in the new free place
            ((struct table *)NODE.table->item)[i].socket = ((struct table *)NODE.table->item)[NODE.table->occupancy - 1].socket;
            sprintf(((struct table *)NODE.table->item)[i].id, "%s", ((struct table *)NODE.table->item)[NODE.table->occupancy - 1].id);

            // reset the id of the last position
            memset(((struct table *)NODE.table->item)[NODE.table->occupancy - 1].id, '\0', BUFFER_SIZE);
            NODE.table->occupancy--;
            break;
        }

        // send withdraw message to all the remaining sockets
        for (unsigned int i = 0; i < NODE.direct_neighbours->occupancy; i++)
        {
            // withdraw socket
            if (((int *)NODE.direct_neighbours->item)[i] == socket)
                continue;

            // send withdraw messaeges
            sprintf(message, "WITHDRAW %s\n", id);
            Write(((int *)NODE.direct_neighbours->item)[i], message, strlen(message));
        }
    }

    for (unsigned int i = 0; i < allocation_size; i++)
    {
        free(exit_id[i]);
    }
    free(exit_id);
    exit_id = NULL;
}

/**
 * Reconnects the network when a node leaves and separates it in two sub-networks, using the recovery contact
 * @param   fd_neighbour    End point that was closed
 * @param   bootIP          Store the IPv4 that the one will connect if needed          @return
 * @param   bootTCP         Store the TCP port that the one will connect if needed      @return
*/
bool reconnect_network(int fd_neighbour, char *bootIP, char *bootTCP)
{
    char message[MESSAGE_SIZE];
    //  see if the node that left was my internal neighbour
    if (NODE.list_internal_neighbours != NULL)
    {
        for (unsigned int i = 0; i < NODE.list_internal_neighbours->occupancy; i++)
        {
            if (((struct contact *)NODE.list_internal_neighbours->item)[i].socket != fd_neighbour)
                continue;

            // put the last element in the new free place
            ((struct contact *)NODE.list_internal_neighbours->item)[i].socket = ((struct contact *)NODE.list_internal_neighbours->item)[NODE.list_internal_neighbours->occupancy - 1].socket;
            sprintf(((struct contact *)NODE.list_internal_neighbours->item)[i].tcp, "%s", ((struct contact *)NODE.list_internal_neighbours->item)[NODE.list_internal_neighbours->occupancy - 1].tcp);
            sprintf(((struct contact *)NODE.list_internal_neighbours->item)[i].ip, "%s", ((struct contact *)NODE.list_internal_neighbours->item)[NODE.list_internal_neighbours->occupancy - 1].ip);
            NODE.list_internal_neighbours->occupancy--;

            return false;
        }
    }
    //  see if the node that left was my external neighbour
    if (NODE.external_neighbour.socket != fd_neighbour)
    {
        printf("Something went wrong, I detected an absence that I can not track. My external's socket is %d and the old node %d\n", NODE.external_neighbour.socket, fd_neighbour);
        return false;
    }

    if (strcmp(NODE.me.tcp, NODE.recovery_contact.tcp) == 0 && strcmp(NODE.me.ip, NODE.recovery_contact.ip) == 0) // if I am my recovery contact
    {
        if (NODE.list_internal_neighbours == NULL)
        {
            memset(&NODE.external_neighbour, '\0', sizeof(struct contact));
            return false;
        }
        else if (NODE.list_internal_neighbours->occupancy == 0)
        {
            memset(&NODE.external_neighbour, '\0', sizeof(struct contact));
            return false;
        }
        // promote an internal neighbour to external
        sprintf(NODE.external_neighbour.ip, "%s", ((struct contact *)NODE.list_internal_neighbours->item)[NODE.list_internal_neighbours->occupancy - 1].ip);
        sprintf(NODE.external_neighbour.tcp, "%s", ((struct contact *)NODE.list_internal_neighbours->item)[NODE.list_internal_neighbours->occupancy - 1].tcp);
        NODE.external_neighbour.socket = ((struct contact *)NODE.list_internal_neighbours->item)[NODE.list_internal_neighbours->occupancy - 1].socket;
        
        // updates the recovery neighour of the promoted node
        sprintf(message, "EXTERN %s %s\n", NODE.external_neighbour.ip, NODE.external_neighbour.tcp); // envia o proprio contacto
        Write(NODE.external_neighbour.socket, message, strlen(message));
        NODE.list_internal_neighbours->occupancy--;
        return false;
    }

    memset(&NODE.external_neighbour, '\0', sizeof(struct contact));
    sprintf(bootIP, "%s", NODE.recovery_contact.ip);
    sprintf(bootTCP, "%s", NODE.recovery_contact.tcp);
    // alert internal neighbours of my new external neighbour and, therefore, their new recovery contact
    if (NODE.list_internal_neighbours != NULL)
    {
        for (unsigned int i = 0; i < NODE.list_internal_neighbours->occupancy; i++)
        {
            memset(message, '\0', MESSAGE_SIZE);
            sprintf(message, "EXTERN %s %s\n", NODE.recovery_contact.ip, NODE.recovery_contact.tcp); // envia o proprio contacto
            Write(((struct contact *)NODE.list_internal_neighbours->item)[i].socket, message, strlen(message));
        }
    }

    return true;
}

/**
 * Initializes node struct
*/
void node_init(char *id, char *ip, char *port)
{
    if (NODE.table != NULL)
        return;

    NODE.table = add_item_checkup(NODE.table, sizeof(struct table));

    ((struct table *)NODE.table->item)[0].socket = -1;
    sprintf(((struct table *)NODE.table->item)[0].id, "%s", id);
    sprintf(NODE.me.ip, "%s", ip);
    sprintf(NODE.me.tcp, "%s", port);

    init_cache();
}

/**
 * Removes a specific socket from the direct neighbours array if it detects that a connection is closed
*/
void remove_direct_neighbour(int socket)
{
    for (unsigned int i = 0; i < NODE.direct_neighbours->occupancy; i++)
    {
        if (((int *)NODE.direct_neighbours->item)[i] != socket)
            continue;

        // put the last element in the new free place
        Close(((int *)NODE.direct_neighbours->item)[i]);
        ((int *)NODE.direct_neighbours->item)[i] = ((int *)NODE.direct_neighbours->item)[NODE.direct_neighbours->occupancy - 1];
        ((int *)NODE.direct_neighbours->item)[NODE.direct_neighbours->occupancy - 1] = -1;
        NODE.direct_neighbours->occupancy--;

        break;
    }
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

/**
 * Sets the sockets in the adjancency table for Select()
 * @param   rfds    range of file descriptors to be tested
 * @return          number maximum of sockets
*/
int set_sockets(fd_set *rfds)
{
    if (NODE.direct_neighbours == NULL) // does not have a table
        return -1;

    int maxfd = 0;
    for (unsigned int i = 0; i < NODE.direct_neighbours->occupancy; i++)
    {
        // if (((struct table *)NODE.table->item)[i].socket == -1) // personal connection in the adjancency table
        //     continue;

        FD_SET(((int *)NODE.direct_neighbours->item)[i], rfds);
        maxfd = max(maxfd, ((int *)NODE.direct_neighbours->item)[i]);
    }
    return maxfd;
}

/**
 * Allocates and intializes resizable_vect
*/
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
        memset(ptr->item+size*ptr->occupancy, '\0', size*ALLOCATION_OFFSET);
    }
    ptr->occupancy++;
    return ptr;
}

/**
 * Iterates over the adjacency table and returns the set socket if a known connection was set, and -1 otherwise
 * @param   rfds    range of file descriptors to be tested
 * @return          active socket
*/
int FD_ISKNOWN(fd_set *rfds)
{
    for (unsigned int i = 0; i < NODE.direct_neighbours->occupancy; i++)
    {
        if (FD_ISSET(((int *)NODE.direct_neighbours->item)[i], rfds) != 0)
            return ((int *)NODE.direct_neighbours->item)[i];
    }
    return -1;
}

/**
 *  Initializes the cache list
*/
void init_cache()
{
    //head vai ser o mais recentemente utilizado
    NODE.head = NULL;
    //tail vai ser o que não é utilizado há mais tempo
    NODE.tail = NULL;

    for (int i = 0; i < CACHE_SIZE; i++)
    {
        struct cache_node *new_node = (struct cache_node *)checked_calloc(1, sizeof(struct cache_node));
        memset(new_node->subname, '\0', BUFFER_SIZE);
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

/**
 *  
*/
void search_object(char *name, int socket, bool waiting_for_object)
{
    char id[BUFFER_SIZE];
    char subname[BUFFER_SIZE];
    char message[MESSAGE_SIZE];
    bool id_in_my_table = false;

    bool valid_name = break_name_into_id_and_subname(name, id, subname);
    if (!valid_name)
    {
        printf("The name '%s' you entered is not valid, try again soldier.\n", name);
        return;
    }

    // validate the id
    for (unsigned int i = 0; i < NODE.table->occupancy; i++)
    {
        if (strcmp(((struct table *)NODE.table->item)[i].id, id) != 0)
            continue;

        id_in_my_table = true;
        break;
    }
    if (!id_in_my_table)
    {
        printf("The id: '%s' is not in my group chat...\n", id);
        return;
    }

    // search in my own cache
    struct cache_node *ptr = search_my_cache(subname);
    if (ptr != NULL) // found it
    {
        if (waiting_for_object)
        {
            sprintf(message, "DATA %s.%s\n", id, subname);
            Write(socket, message, strlen(message));
        }
        else
        {
            printf("I already have the object titled: '%s'.\n", subname);
        }
        update_cache(subname);
        return;
    }

    // I am the one to whom the message was intended, if I did not find the object the search will end unsuccessfully
    if (strcmp(id, ((struct table *)NODE.table->item)[0].id) == 0)
    {
        sprintf(message, "NODATA %s.%s\n", id, subname);
        Write(socket, message, strlen(message));
        return;
    }

    for (unsigned int i = 0; i < NODE.table->occupancy; i++)
    {
        if (strcmp(((struct table *)NODE.table->item)[i].id, id) != 0)
            continue;

        sprintf(message, "INTEREST %s.%s\n", id, subname);
        Write(((struct table *)NODE.table->item)[i].socket, message, strlen(message));

        NODE.income_messages = add_item_checkup(NODE.income_messages, sizeof(struct message_path));
        ((struct message_path *)NODE.income_messages->item)[NODE.income_messages->occupancy - 1].socket = socket;
        sprintf(((struct message_path *)NODE.income_messages->item)[NODE.income_messages->occupancy - 1].name, "%s", name);
    }
}

void return_search(char *command, char *name)
{
    char id[BUFFER_SIZE];
    char subname[BUFFER_SIZE];
    char message[MESSAGE_SIZE];

    bool valid_name = break_name_into_id_and_subname(name, id, subname);
    if (!valid_name)
    {
        printf("The name '%s' you entered is not valid, try again soldier.\n", name);
        return;
    }

    // save the object if it receives a DATA message
    if (strcmp(command, "DATA") == 0)
    {
        update_cache(subname);
    }

    int socket = -1;

    // search for the loop back path according to the message
    for (unsigned int i = 0; i < NODE.income_messages->occupancy; i++)
    {
        if (strcmp(name, ((struct message_path *)NODE.income_messages->item)[i].name))
            continue;

        NODE.income_messages->occupancy--;
        socket = ((struct message_path *)NODE.income_messages->item)[i].socket;
        break;
    }

    if (socket == -1)
    {
        printf("I do not have any transcript of the message: '%s'.\n", name);
        return;
    }

    if (socket == STDOUT_FILENO)
    {
        if (strcmp(command, "DATA") == 0)
            printf("Gottcha, your Pokemon: '%s' is ready now!\n", subname);
        else
            printf("Even though I had a tough time searching for the needle in the haystack, it is still missing :'(\n");
    }
    else
    {
        sprintf(message, "%s %s.%s\n", command, id, subname);
        Write(socket, message, strlen(message));
    }
}

void show_cache()
{
    struct cache_node *ptr = NODE.head;
    int count = 0;

    printf("Cache\n");
    while (ptr != NULL)
    {
        if (strlen(ptr->subname) != 0)
            printf("%d. %s\n", ++count, ptr->subname);

        ptr = ptr->next;
    }
    printf("OCCUPANCY: %d\t CACHE_SIZE: %d\n", count, CACHE_SIZE);
}

// LSR
void update_cache(char *subname)
{
    struct cache_node *ptr = search_my_cache(subname);
    if (ptr == NULL) // object does not exist in my cache
    {
        sprintf(NODE.tail->subname, "%s", subname);
        // criar ligações
        NODE.tail->next = NODE.head;
        NODE.head->previous = NODE.tail;
        // atualizar ponteiros
        NODE.tail = NODE.tail->previous;
        NODE.head = NODE.head->previous;
        // quebrar ligações
        NODE.tail->next = NULL;
        NODE.head->previous = NULL;
    }
    else
    {
        // Remove from the middle
        if (ptr->previous == NULL) // it already is the recently
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

struct cache_node *search_my_cache(char *subname)
{
    struct cache_node *ptr = NODE.head;

    while (ptr != NULL)
    {
        if (strcmp(subname, ptr->subname) == 0) // found subname
            return ptr;
        ptr = ptr->next;
    }
    return NULL;
}

void close_node()
{
    if (NODE.direct_neighbours != NULL)
    {
        if (NODE.direct_neighbours->item != NULL)
        {
            for (unsigned int i = 0; i < NODE.direct_neighbours->occupancy; i++)
            {
                Close(((int *)NODE.direct_neighbours->item)[i]);
                ((int *)NODE.direct_neighbours->item)[i] = -1;
            }
            free(NODE.direct_neighbours->item);
            NODE.direct_neighbours->item = NULL;
        }
        free(NODE.direct_neighbours);
        NODE.direct_neighbours = NULL;
    }

    if (NODE.table != NULL)
    {
        if (NODE.table->item != NULL)
        {
            free(NODE.table->item);
            NODE.table->item = NULL;
        }
        free(NODE.table);
        NODE.table = NULL;
    }

    if (NODE.list_internal_neighbours != NULL)
    {
        if (NODE.list_internal_neighbours->item != NULL)
        {
            free(NODE.list_internal_neighbours->item);
            NODE.list_internal_neighbours->item = NULL;
        }
        free(NODE.list_internal_neighbours);
        NODE.list_internal_neighbours = NULL;
    }

    if (NODE.income_messages != NULL)
    {
        if (NODE.income_messages->item != NULL)
        {
            free(NODE.income_messages->item);
            NODE.income_messages->item = NULL;
        }
        free(NODE.income_messages);
        NODE.income_messages = NULL;
    }

    // free cache
    struct cache_node *tmp;
    while (NODE.head != NULL)
    {
        tmp = NODE.head;
        NODE.head = NODE.head->next;
        free(tmp);
        tmp = NULL;
    }
}
