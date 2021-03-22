#ifndef MY_HASHMAP
#define MY_HASHMAP

#include "hashmap_base.h"

/* a way to signal what went wrong */
enum HashMapResult {
    FailedToInsert,
    FailedToInsertNoMemory,
    FailedToInsertDuplicate,
    Success,
};

#define HAHSMAP(key_type, data_type)                                           \
    struct {                                                                   \
        HashMapBase *map_base;                                                 \
        struct {                                                               \
            const key_type *key_t;                                             \
            data_type *data_t;                                                 \
        } data_types;                                                          \
    }

#define init_hashmap(hashmap, hash_func)                                       \
    do {                                                                       \
        init_hashmap_base(*(hashmap.map_base), (HashFunc)hash_func);           \
    } while (0)

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

uint64_t integer_hash64(uint64_t x);
size_t data_hash64(const void *data, size_t len);

#endif
