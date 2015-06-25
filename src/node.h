#include <stdlib.h>
#include <unistd.h>

#ifndef __NODE_H__

struct node_s {
    uint32_t hash;
    time_t timeout;
    size_t size;
    char *data;
    char key[];
};
typedef struct node_s node_t;

#endif
