#include "util.h"

bool check_IP(char *IP)
{
    int a = -1, b = -1, c = -1, d = -1;
    sscanf(IP, "%d.%d.%d.%d", &a, &b, &c, &d);
    if (0 <= a && a <= 255 && 0 <= b && b <= 255 && 0 <= c && c <= 255 && 0 <= d && d <= 255)
        return true;

    fprintf(stderr, "IP: \"%s\" is not valid.\n", IP);
    return false;
}

// Dynamic ports (also called private ports) are 49152 to 65535.
bool check_port(char *port)
{
    int port_int = atoi(port);
    if (49152 <= port_int && port_int <= 65535)
        return true;

    fprintf(stderr, "Port: \"%s\" is not valid.\n", port);
    return false;
}

bool check_net(char *net)
{
    int net_int = atoi(net);
    if (0 <= net_int && net_int <= 99)
        return true;

    fprintf(stderr, "Net: \"%s\" is not valid.\n", net);
    return false;
}

/**
 * Alocação e inicialização limitada de memória
 * @param  nitems − This is the number of elements to be allocated.
 * @param  size − This is the size of elements.
 * @return      ponteiro para o espaço de memória alocado
 */
void *checked_calloc(size_t nitems, size_t size)
{
    void *ptr = calloc(nitems, size);

    if (ptr == NULL)
    {
        fprintf(stderr, "Error allocating memory\n");
        exit(0);
    }

    return ptr;
}
