#ifndef MY_HASHMAP
#define MY_HASHMAP

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

/* a way to signal what went wrong */
enum HashMapResult {
    FailedToInsert,
    FailedToInsertNoMemory,
    FailedToInsertDuplicate,
    Success,
};

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
    uint64_t seed;
    Entry **table;
} HashMap;

/* the iteration data */
typedef struct {
    int current_index;
    int prev_index;
    int table_size;
    Entry *prev_entry;
    Entry *current_entry;
    Entry **iter_table;
} IterHashMap;

uint64_t integer_hash64(uint64_t x);
size_t data_hash64(const void *data, size_t len);
uint64_t hash64(HashMap *map, uint64_t key);

/* the function signature to pass a drop function for values
 *
 * if the function is null then the value is not dropped
 */
typedef void (*DropValueFunc)(void *value);

HashMap *create_hashmap();
HashMap *create_sized_hashmap(int table_size);

void drop_hashmap(HashMap *hashmap, DropValueFunc drop_func);

enum HashMapResult insert_hashmap(HashMap *map, uint64_t key, void *value);

bool remove_hashmap(HashMap *map, uint64_t key);

bool contains_key_hashmap(HashMap *map, uint64_t key);

/** delete an entry and return the value */
void *delete_entry_hashmap(HashMap *map, uint64_t key);

/** make a new iterator
 *
 * this is whats passed to the for_each() macro
 *
 * this only needs to be called once and will be able to iterate many times
 * this might change though
 */
IterHashMap *get_iter_hashmap(HashMap *map);
void drop_iter_hashmap(IterHashMap *iter);

void iter_next_safe_hashmap(IterHashMap *iter, uint64_t *key, void **value);

/** get the next value from the table (not ordered)
 *
 * this is manly for the for_each() macro, it probably should be called out
 * side by the user
 */
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
    iter->current_index = 0;                                                   \
                                                                               \
    for (iter_next_hashmap(iter, &key, &value);                                \
         iter->current_index < iter->table_size &&                             \
         iter->current_entry != NULL;                                          \
         iter_next_hashmap(iter, &key, &value))

#define for_each_safe(iter, key, value)                                        \
    iter->current_index = 0;                                                   \
                                                                               \
    for (iter_next_safe_hashmap(iter, &key, &value);                           \
         iter->current_index < iter->table_size &&                             \
         iter->current_entry != NULL;                                          \
         iter_next_safe_hashmap(iter, &key, &value))

#endif
