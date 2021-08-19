#ifndef MY_HASHMAP_BASE
#define MY_HASHMAP_BASE

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#define STARTING_SIZE 1024
#define GROWTH_FACTOR 2
#define MAX_LOAD_FACTOR 0.7

/* the function signature to hash the key
 *
 * this will be stored with the struct
 */
typedef uint64_t (*HashFunc)(const void *key);

typedef bool (*CompFunc)(const void *key_1, const void *key_2);

/* the function signature to pass a drop function for values
 *
 * if the function is null then the value is not dropped
 */
typedef void (*DropFunc)(void *key, void *value);

/* a way to signal what went wrong */
enum HashMapResult {
    FailedToInsert,
    FailedToInsertNoMemory,
    FailedToInsertDuplicate,
    FailedToRehashNoMemory,
    Success,
};

/* a entry in the hashmap */
typedef struct Entry {
    void *value;
    void *key;
    struct Entry *next;
} Entry;

/* the main hashmap */
typedef struct {
    int table_size;
    int current_size;
    Entry **table;
    HashFunc hash_func;
    DropFunc drop_func;
    CompFunc comp_func;
} HashMapBase;

/* the iteration data */
typedef struct {
    int current_index;
    Entry *current_entry;
    HashMapBase *base;
} IterHashMap;

HashMapBase *init_hashmap_base(HashFunc hash_func, CompFunc comp_func,
                               DropFunc drop_func, uint64_t size);

void drop_hashmap_base(HashMapBase *map);

void drop_entry(Entry *prev_entry, Entry *current_entry);

enum HashMapResult insert_hashmap_base(HashMapBase *map, void *, void *value);

void *remove_entry_hashmap_base(HashMapBase *map, void *key);

bool contains_key_hashmap_base(HashMapBase *map, void *key);

void *get_value_hashmap_base(HashMapBase *map, void *key);

IterHashMap *get_iter_hashmap_base(HashMapBase *map);
void drop_iter_hashmap(IterHashMap *iter);

void _iter_next_base(IterHashMap *iter);

bool iter_next_hashmap(IterHashMap *iter, void **key, void **value);

bool iter_next_drop_hashmap(IterHashMap *iter, void **key, void **value);

// void iter_next_safe_hashmap(IterHashMap *iter, const void **key, void
// **value);

int get_longest_chain_base(HashMapBase *map);
#endif
