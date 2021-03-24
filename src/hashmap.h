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
            data_type *data_t;                                                 \
            uint64_t (*hash_func_t)(const key_type *);                         \
            void (*drop_func_t)(key_type *, data_type *);                      \
            bool (*compare_func_t)(const key_type *, const key_type *);        \
        } _data_types;                                                         \
    } name

/* allocate memory for the  given hashmap;
 *
 * this will allocate all the needed memory for a given hashmap
 *
 * the do {} while(0) is too allow the user to use the macro like a regular
 * function/statement including ending with a semicolon
 *
 * see this stackoverflow https://stackoverflow.com/questions/1067226/
 * c-multi-line-macro-do-while0-vs-scope-block
 *
 * use:
 *   HashMap *map;
 *   init_hashmap(map, hash_func, drop_func);
 */
#define init_hashmap(hashmap, hash_func, drop_func, comp_func)                 \
    do {                                                                       \
        typeof(hashmap->_data_types.hash_func_t) _hash_func = hash_func;       \
                                                                               \
        typeof(hashmap->_data_types.drop_func_t) _drop_func = drop_func;       \
                                                                               \
        typeof(hashmap->_data_types.compare_func_t) _comp_func = comp_func;    \
                                                                               \
        hashmap = (typeof(hashmap))malloc(sizeof(*hashmap));                   \
                                                                               \
        if (hashmap != NULL) {                                                 \
            hashmap->map_base = init_hashmap_base(                             \
                (HashFunc)_hash_func, (DropValueFunc)_drop_func,               \
                (CompFunc)_comp_func, STARTING_SIZE);                          \
        }                                                                      \
    } while (0)

/* drop the hashmap freeing all its memory
 *
 * this will end up calling the drop_func from the hash map to free all the
 * values
 */
#define drop_hashmap(hashmap)                                                  \
    do {                                                                       \
        drop_hashmap_base(hashmap->map_base);                                  \
                                                                               \
        free(hashmap);                                                         \
    } while (0)

/* insert a key and value in to the hashmap
 */
#define insert_hashmap(hashmap, key, value, success)                           \
    do {                                                                       \
        typeof(hashmap->_data_types.key_t) _key = key;                         \
        typeof(hashmap->_data_types.data_t) _value = value;                    \
                                                                               \
        success = insert_hashmap_base(hashmap->map_base, (const void *)_key,   \
                                      (void *)_value);                         \
    } while (0)

#define contains_key_hashmap(hashmap, key, contains)                           \
    do {                                                                       \
        typeof(hashmap->_data_types.key_t) _key = key;                         \
                                                                               \
        contains =                                                             \
            contains_key_hashmap_base(hashmap->map_base, (const void *)_key);  \
    } while (0)

#define remove_entry_hashmap(hashmap, key, value_to_fill)                      \
    do {                                                                       \
        typeof(hashmap->_data_types.key_t) _key = key;                         \
                                                                               \
        typeof(hashmap->_data_types.data_t) *_value = &value_to_fill;          \
                                                                               \
        *_value = remove_entry_hashmap_base(hashmap->map_base, _key);          \
    } while (0)

#define get_iter_hashmap(hashmap, iter)                                        \
    iter = get_iter_hashmap_base(hashmap->map_base);

// #define drop_iter(iter) drop_iter(iter)

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
    for (iter_next_hashmap(iter, (const void **)&key, (void **)&value);        \
         iter->current_index < iter->table_size &&                             \
         iter->current_entry != NULL;                                          \
         iter_next_hashmap(iter, (const void **)&key, (void **)&value))

#define for_each_safe(iter, key, value)                                        \
    iter->current_index = 0;                                                   \
                                                                               \
    for (iter_next_safe_hashmap(iter, (const void **)&key, (void **)&value);   \
         iter->current_index < iter->table_size &&                             \
         iter->current_entry != NULL;                                          \
         iter_next_safe_hashmap(iter, (const void **)&key, (void **)&value))

uint64_t integer_hash64(uint64_t x);
size_t data_hash64(const void *data, size_t len);

#endif
