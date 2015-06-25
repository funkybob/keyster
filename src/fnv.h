#ifndef __FNV_H__

#define FNV_prime32 16777619
#define FNV_offset32 2166136261

#define FNV_hash32(x, y)   { \
    uint8_t *key = (uint8_t*)x; \
    uint32_t hash = FNV_offset32; \
    while(*key) { \
        hash *= FNV_prime32; \
        hash ^= *key++; \
    } \
    y = hash; \
}

# endif
