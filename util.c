#include "util.h"

/**
 * This function splits the name into id and subname
 * e.g. : node35.project_assignment -> (id,subname) = (node35,project_assignment)
 * @param   name        TCP message used in data transfering
 * @param   id          Node identifier                         @return
 * @param   subname     Transfered data information             @return
*/
bool break_name_into_id_and_subname(char *name, char *id, char *subname)
{
    int i = 0;
    memset(id, '\0', BUFFER_SIZE);
    memset(subname, '\0', BUFFER_SIZE);
    for (int k = 0; k < (int)strlen(name); k++)
    {
        if (name[k] == '.') // moves to the subname when the delimeter '.' is found
            i = 1;
        else if (i == 0)
            id[strlen(id)] = name[k];
        else
            subname[strlen(subname)] = name[k];
    }
    return (strlen(id) != 0 && strlen(subname) != 0);
}

/**
 * Limited memory allocation and inicializaation
 * @param  nitems − This is the number of elements to be allocated.
 * @param  size − This is the size of elements.
 * @return
 */
void *checked_calloc(size_t nitems, size_t size)
{
    void *ptr = calloc(nitems, size);

    if (ptr == NULL)
    {
        fprintf(stderr, "Error allocating memory\n");
        exit(1);
    }

    return ptr;
}

/**
 * Limited memory allocation
 * @param   size    This is the size of elements.
 * @return          Pointer to the allocated memory.
 */
void *checked_realloc(void *ptr, size_t size)
{

    ptr = realloc(ptr, size);

    if (ptr == NULL)
    {
        fprintf(stderr, "Error allocating memory\n");
        exit(1);
    }

    return ptr;
}

/**
 * Validates the IPv4 only if it has the syntax: byte.byte.byte.byte
 * @param   IPv4    IPv4 to be validated.
 * @return          Bollean wheter the IPv4 is valid or not.
*/
bool check_IP(char *IPv4)
{
    int a = -1, b = -1, c = -1, d = -1;
    sscanf(IPv4, "%d.%d.%d.%d", &a, &b, &c, &d);
    if (0 <= a && a <= 255 && 0 <= b && b <= 255 && 0 <= c && c <= 255 && 0 <= d && d <= 255)
        return true;

    fprintf(stderr, "IP: \"%s\" is not valid.\n", IPv4);
    return false;
}

/**
 * Validates the port only if it in the range (49152:65535)
 * @param   port    Port to be validated.
 * @return          Bollean wheter the port is valid or not.
 * Dynamic ports (also called private ports) are 49152 to 65535.
*/
bool check_port(char *port)
{
    int port_int = atoi(port);
    if (49152 <= port_int && port_int <= 65535)
        return true;

    fprintf(stderr, "Port: \"%s\" is not valid.\n", port);
    return false;
}

/**
 * Computes the number of LF in a string
 * @param   string   Income message from UDP or TCP connections.
 * @return           Number of Line Feed that the message has before the first '\0'.
*/
int get_number_of_LF(char *string)
{
    int count = 0;
    for (int i = 0; string[i] != '\0'; ++i)
    {
        if ('\n' == string[i])
            ++count;
    }
    return count;
}
