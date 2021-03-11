#ifndef MY_HASHMAP
#define MY_HASHMAP

#include <stdbool.h>
#include <stdint.h>

enum HashMapResult {
    FailedToInsert,
    FailedToInsertNoMemory,
    FailedToInsertDuplicate,
    CouldNotFind,
    Success,
};

typedef struct Entry {
    struct Entry *next;
    uint64_t key;
    void *value;
} Entry;

typedef struct {
    int table_size;
    int current_size;
    uint64_t seed;
    Entry **table;
} HashMap;

typedef struct {
    int current_index;
    int table_size;
    Entry *current_entry;
    Entry **iter_table;
} IterHashMap;

HashMap *create_hashmap();

typedef void (*DropValueFunc)(void *value);

void drop_hashmap(HashMap *hashmap, DropValueFunc drop_func);

enum HashMapResult insert_hashmap(HashMap *map, uint64_t key, void *value);

bool remove_hashmap(HashMap *map, uint64_t key);

bool contains_key_hashmap(HashMap *map, uint64_t key);

IterHashMap *get_iter_hashmap(HashMap *map);

void iter_next_hashmap(IterHashMap *iter, uint64_t *key, void **value);

/** iterate over the hash map (unsafe)
 *
 * call iter_next_hashmap() in a loop until the iter index is the same or larger
 * the table size
 *
 * iter_next_hashmap will assign the `key` and `value` to the user provided
 * variables
 *
 * restart the iterator every time for_each is called by setting the
 * current_index to zero every time this is called/expanded
 */

#define for_each(iter, key, value)                                             \
                                                                               \
    iter->current_index = 0;                                                   \
                                                                               \
    for (iter_next_hashmap(iter, &key, &value);                                \
         iter->current_index < iter->table_size &&                             \
         iter->current_entry != NULL;                                          \
         iter_next_hashmap(iter, &key, &value))

#endif
