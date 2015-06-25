/*

Block Chain array

Each Block links to the next.

Each Block is an array of N elements.
-> N is calculated to make sizeof(Block) ~ 1 PAGE

Each Block keeps a count of how many elements are used.

Elements in a block are sorted by key, and always packed [no gaps].

To find an element:
1. start at first block.
2. test if our key is < element[0].key
3. If so, we are before this - fail
4. test if our key is > element[used].key
5. If so, move to next block. If next block is NULL, fail.
6. binary search elements to find matching key, or fail.

To add an elemnt:
1. find which block has a larger key than us.
2. if none, append a new block ,and become first element. END.
3. if block is full, spill into next block.
3a. if next block is full, insert a new block, and become first element. END.
4. move "larger" elements up one space.
5. insert our details in new gap
6. incremend 'used' counter. END

To remove an element:
1. find which block the element is in.
2. if is the only element, remove from block chain and free. END
3. move "larger" elements down one space.
4. decremend 'used' counter. END

Qualities:
1. Dynamically grow/shrink capacity.
2. Fast skip N elements with only 2 compares.
3. Allocate PAGE sized chunks of memory at a time for Blocks+Elements
4. Easy to adapt to portable memory space.

*/

struct kv_node_s {
    char *key;
    char *data;
    size_t data_len;
};
typedef struct kv_node_s kv_node_t;

struct bc_block_s {
    struct bc_block_s *next;
    int last;
    // This claculation assumes the above fields are smaller than 1 kv_node_t
    struct node item[4095 / sizeof(struct node)];
};
typedef struct bc_block_s bc_block_t;


// Insert values in a new position
void node_add(bc_block_t *root, char *key, char *data, size_t data_len) {
}

// Returns a pointer to the kv node, if it exists.
void node_get(bc_block_t *root, char *key) {
}

// Returns the block and index matching the key
void node_find(bc_block_t *root, char *key, bc_block_t **block, int *index) {
}

// Removes 
void node_remove(bc_block_t *block, int index) {
}
