#ifndef MY_HASHMAP_BASE
#define MY_HASHMAP_BASE

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

typedef uint64_t (*HashFunc)(const void *key);

/* a entry in the hashmap */
typedef struct Entry {
    void *value;
    uint64_t key;
    struct Entry *next;
} Entry;

/* the main hashmap */
typedef struct {
    int table_size;
    int current_size;
    Entry **table;
    HashFunc hash_func;
} HashMapBase;

/* the iteration data */
typedef struct {
    int current_index;
    int prev_index;
    int table_size;
    Entry *prev_entry;
    Entry *current_entry;
    Entry **iter_table;
} IterHashMap;

// uint64_t hash64(HashMap *map, uint64_t key);

/* the function signature to pass a drop function for values
 *
 * if the function is null then the value is not dropped
 */

bool init_hashmap_base(HashMapBase *map, HashFunc hash_func);

// typedef void (*DropValueFunc)(void *value);

// void drop_hashmap(HashMap *map, DropValueFunc drop_func);

// enum HashMapResult insert_hashmap(HashMap *map, uint64_t key, void *value);

// bool remove_hashmap(HashMap *map, uint64_t key);

// bool contains_key_hashmap(HashMap *map, uint64_t key);

/** delete an entry and return the value */
// void *delete_entry_hashmap(HashMap *map, uint64_t key);
#endif
