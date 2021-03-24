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

/* the function signature to pass a drop function for values
 *
 * if the function is null then the value is not dropped
 */
typedef void (*DropValueFunc)(void *value);

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
    const void *key;
    struct Entry *next;
} Entry;

/* the main hashmap */
typedef struct {
    int table_size;
    int current_size;
    Entry **table;
    HashFunc hash_func;
    DropValueFunc drop_func;
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

HashMapBase *init_hashmap_base(HashFunc hash_func, DropValueFunc drop_func,
                               uint64_t size);

void drop_hashmap_base(HashMapBase *map);

enum HashMapResult insert_hashmap_base(HashMapBase *map, const void *,
                                       void *value);

void *remove_entry_hashmap_base(HashMapBase *map, const void *key);

bool contains_key_hashmap_base(HashMapBase *map, const void *key);

#endif
