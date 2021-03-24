#ifndef MY_HASHMAP
#define MY_HASHMAP

#include "hashmap_base.h"

/* a macro to define a (kinda) type safe hashmap
 *
 * the map_base  struct is the actual hashmap
 *
 * the _data_types struct is to allow for checking the types in the macro
 * interface and will not receive values
 */
#define HAHSMAP(name, key_type, data_type)                                     \
    typedef struct {                                                           \
        HashMapBase *map_base;                                                 \
        struct {                                                               \
            const key_type *key_t;                                             \
            data_type *value_t;                                                \
            uint64_t (*hash_func_t)(const key_type *);                         \
            void (*drop_func_t)(data_type *);                                  \
        } _data_types;                                                         \
    } name

/* allocate memory for the  given hashmap;
 *
 * this will allocate all the needed memory for a given hashmap
 *
 * use:
 *   HashMap *map;
 *   init_hashmap(map, hash_func, drop_func);
 */
#define init_hashmap(hashmap, hash_func, drop_func)                            \
    do {                                                                       \
        typeof(hashmap->_data_types.hash_func_t) _hash_func = hash_func;       \
                                                                               \
        typeof(hashmap->_data_types.drop_func_t) _drop_func = drop_func;       \
                                                                               \
        hashmap = (typeof(hashmap))malloc(sizeof(*hashmap));                   \
                                                                               \
        if (hashmap != NULL) {                                                 \
            hashmap->map_base =                                                \
                init_hashmap_base((HashFunc)_hash_func,                        \
                                  (DropValueFunc)_drop_func, STARTING_SIZE);   \
        }                                                                      \
    } while (0)

#define insert_hashmap(hashmap, key, value)                                    \
    do {                                                                       \
        typeof(hashmap->_data_types.key_t) _key = key;                         \
        typeof(hashmap->_data_types.value_t) _value = value;                   \
                                                                               \
        insert_hashmap(hashmap->map_base, (const void *)_key, (void *)_value); \
    } while (0)

#define contains_key_hashmap(hashmap, key, success)                            \
    do {                                                                       \
        *success = false;                                                      \
        typeof(hashmap->_data_types.key_t) _key = key;                         \
                                                                               \
        *success =                                                             \
            contains_key_hashmap(hashmap->map_base, (const void *)_key);       \
    } while (0)

/** iterate over the hash map (unsafe)
 *
 * call iter_next_hashmap() in a loop until the iter index is the same or larger
 * the table size iter_next_hashmap will assign the `key` and `value` to the
 * user provided *variables restart the iterator every time for_each is called
 * by setting the *current_index to zero every time this is called expanded
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
