#include "node.h"

node_t* node_alloc(char *key, char *data, size_t size, time_t timeout) {
    node_t *n = malloc(sizeof(node_t) + strlen(key));
    strcpy(n->key, key);
    n->timeout = timeout;
    n->size = size;
    n->data = malloc(size);
    memcpy(n->data, data, size);
    return n;
}

void node_free(node_t* n) {
    free(n->data);
    free(n);
}

int node_compare_key(node_t *n, char *key) {
    return strcmp(n->key, key);
}

int node_compare(node_t *n, node_t *m) {
    if(n->hash == m->hash) return 0;
    return strcmp(n->key, m->key);
}
