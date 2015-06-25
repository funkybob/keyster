/*

*/

#include "node.h"
#include "fnv.h"


struct trie_s {
    struct trie_s* link[16];
};
typedef struct trie_s trie_t;

#define LINK_IS_NODE(x) ((x) && (trie_t*)0x01 == 1)
#define LINK_IS_CHILD(x) ((x) && (trie_t*)0x01 == 0)
#define LINK_MASK(x) ((x) && (trie_t*)~1)

#define NODE_AS_TRIE ((node_t*)((x) | 0x01))


static uint32_t trie_hash_key(const char *key) {
    uint32_t value;
    FNV_hash32(key, value);
    return value;
}

trie_t* trie_alloc(void) {
    return calloc(sizeof(trie_t), 1);
}

#define HBITS(hash, step) ((hash >> ((8-step) * 4)) & 0x0f)

node_t** trie_find(trie_t *root, char *key) {
    uint32_t hash;
    trie_t *n;
    int step;
    int idx;

    FNV_hash32(key, hash);

    if(root == NULL) {
        root = trie_alloc();
        return (node_t**)&root[HBITS(hash, 0)];
    }

    for(step = 0; step < 8; step++) {
        idx = HBITS(hash, step);
        //n = root[idx];
        if(root[idx] == NULL) {
            // Found an empty slot
            return (node_t **)&root[idx];
        }
        if(LINK_IS_NODE(n)) {
            // Collission
            // Create new trie layer
            trie_t* t_new = trie_alloc();
            root[idx] = t_new;
            // move n down into it
            // n is already marked as a node
            t_new[HBITS(LINK_MASK(n)->hash, step)] = n
            // re-loop.
            root = t_new;
        } else {
            // Another layer -- step into it.
            root = LINK_MASK(n);
        }
    }

    // if we got here, we've found a 32bit collission!
}
