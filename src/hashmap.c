#include <math.h>
#include <stdio.h>

#include "hashmap_base.h"

/** init the hashmap base
 *
 * this will allocate memory for the base struct and the table
 *
 * @param hash_func
 *  the hash function should return a 64bit int aka uint64_t
 *
 *  @param comp_func
 *   a function that should return true if the keys match and false if they dont
 *
 *  @param drop_func
 *   a function that will receive the key and value for each entry
 *
 *  @param size
 *   the size of the hash table
 */
HashMapBase *init_hashmap_base(HashFunc hash_func, CompFunc comp_func,
                               DropFunc drop_func, size_t size) {
    HashMapBase *map = malloc(sizeof(HashMapBase));

    if (map == NULL) {
        return NULL;
    }

    map->table = calloc(size, sizeof(Entry *));

    if (map->table == NULL) {
        return NULL;
    }

    map->table_size = size;
    map->current_size = 0;

    map->hash_func = hash_func;
    map->comp_func = comp_func;
    map->drop_func = drop_func;

    return map;
}

/** drop the hashmap table, entrys and values
 *
 * the values will not be freed if there is no drop_func attached to the hashmap
 *
 * NOTE: the value check could be enforced from the drop_func but it
 * probably is safer to do that here and let the drop_func assume it
 * will always have valid data
 *
 * @param map
 *  the hashmap base
 */
void drop_table(HashMapBase *map) {
    Entry *entry;
    Entry *temp;

    for (int i = 0; i < map->table_size; ++i) {
        entry = map->table[i];

        while (entry != NULL) {
            if (map->drop_func) {
                map->drop_func((void *)entry->key, entry->value);
            }

            temp = entry->next;

            free(entry);

            entry = temp;
        }
    }

    free(map->table);
}

/** drop the whole hashmap
 *
 * this will try and free the table and ultimately the HashMapBase struct
 *
 * @param map
 *  the hashmap base
 */
void drop_hashmap_base(HashMapBase *map) {
    if (map->table) {
        drop_table(map);
    }

    free(map);
}

/** create an entry struct
 *
 * @param key
 *  a pointer to the key
 *
 * @param value
 *  a pointer the value
 */
Entry *create_entry(const void *key, void *value) {
    Entry *entry = malloc(sizeof(Entry));

    if (entry == NULL) {
        return NULL;
    }

    entry->next = NULL;

    entry->key = key;

    entry->value = value;

    return entry;
}

/** the internal insert function
 *
 * this is where entrys will acutely be entered in to the table/linked list
 *
 * @param map
 *  the hashmap base
 *
 * @param entry
 *  a pointer to an Entry struct, these will not be reallocate
 */
enum HashMapResult _insert_hashmap(HashMapBase *map, Entry *entry) {
    uint64_t key_hash = map->hash_func(entry->key) & (map->table_size - 1);

    // if the bucket entry is null then set the new entry as the start of the
    // linked list for the given bucket
    if (map->table[key_hash] == NULL) {
        map->table[key_hash] = entry;

        return Success;
    }

    /*
     * if there is data in a given bucket see if there is a matching key if not
     * then add the new entry to the end of the buckets list
     */

    bool found = false;

    Entry *table_entry = map->table[key_hash];

    // check all entrys in the list
    //
    // if we are at the first entry and next is null the we skip the loop and go
    // to the last key check
    // else we check the key in the loop
    while (table_entry->next != NULL && !found) {
        // check if key is already in the table
        if (map->comp_func(table_entry->key, entry->key)) {
            found = true;
        } else {
            table_entry = table_entry->next;
        }
    }

    // now check the last (or first) entry in the list for duplicates
    if (found || map->comp_func(table_entry->key, entry->key)) {
        return FailedToInsertDuplicate;
    }

    // just add the entry to the end of the list if there isn't a duplicate
    table_entry->next = entry;

    // let the user know it was successful
    return Success;
}

/** rehash the whole table
 *
 * this will rehash the whole table to a bigger size based on the GROWTH_FACTOR
 *
 * this will not reallocate entrys, this i kind of dangerous as it will leave
 * chains/linked lists half made if there is ever a problem
 *
 * @param map
 *  the hashmap base
 */
enum HashMapResult rehash_hashmap(HashMapBase *map) {
    enum HashMapResult result = Success;

    // get the new table size
    int new_table_size = map->table_size * GROWTH_FACTOR;

    // get a new base map with a larger table to insert in to
    HashMapBase *temp_map = init_hashmap_base(map->hash_func, map->comp_func,
                                              map->drop_func, new_table_size);
    if (temp_map == NULL) {
        return FailedToRehashNoMemory;
    }

    Entry *entry = NULL;
    Entry *temp_entry = NULL;

    // go though the entry table rehashing all the entrys
    for (int i = 0; i < map->table_size && result == Success; ++i) {
        // iterate over the buckets linked list
        entry = map->table[i];

        while (entry != NULL && result == Success) {
            temp_entry = entry->next;

            // this is necessary else the insert function might/will find the
            // same entry its trying to rehash
            //
            // though this is also dangerous as it will leave the old table
            // incomplete if there is an issue inserting or rehashing
            entry->next = NULL;

            result = _insert_hashmap(temp_map, entry);

            entry = temp_entry;
        }
    }

    // if there was an error drop the temp and let the user know
    if (result != Success) {
        // dont free the entrys as data will still be in the original table and
        // the user might want to do something with them
        //
        // TODO: this wont work right now, but I might switch to open addressing
        // so the table wont be left in an incomplete state if there are errors
        // during the rehashing
        free(temp_map->table);
        free(temp_map);

    } else { // assign the new table to the user's hashmap

        // drop the old table but dont drop the values
        free(map->table);

        // set the new table to the old hashmap
        map->table = temp_map->table;
        map->table_size = temp_map->table_size;

        // drop the temp map
        free(temp_map);
    }

    return result;
}

/** the insert entry point
 *
 * this will rehash when the table when it reaches the MAX_LOAD_FACTOR
 *
 * @param map
 *  the hashmap base
 *
 * @param key
 *  the new key to be inserted in to the hashmap
 *
 * @param value
 *  the new value to be inserted in to the hasmap
 */
enum HashMapResult insert_hashmap_base(HashMapBase *map, const void *key,
                                       void *value) {
    enum HashMapResult result = Success;

    // check if we need to resize
    if ((map->current_size + 1) ==
        (int)floor(map->table_size * MAX_LOAD_FACTOR)) {

        result = rehash_hashmap(map);
    }

    // if resizing failed
    if (result != Success) {
        return result;
    }

    Entry *entry = create_entry(key, value);

    if (entry == NULL) {
        return FailedToInsertNoMemory;
    }

    result = _insert_hashmap(map, entry);

    if (result != Success) {
        return result;
    }

    // only increment if we know _insert_hashmap succeeded
    ++map->current_size;

    return Success;
}

/** check is a key is in the table
 *
 * @param map
 *  the hashmap base
 *
 * @param key
 *  the key to check
 */
bool contains_key_hashmap_base(HashMapBase *map, const void *key) {
    uint64_t key_hash = map->hash_func(key) & (map->table_size - 1);

    bool found = false;

    Entry *entry = map->table[key_hash];

    while (entry != NULL && !found) {
        if (map->comp_func(entry->key, key)) {
            found = true;
        } else {
            entry = entry->next;
        }
    }

    return found;
}

/** delete the entry for the given key
 *
 * @param map
 *  the hashmap base
 *
 * @param key
 *  the key to find and remove
 */
// TODO: this could probably be improved but will also change a lot if i switch
// to open addressing
void *remove_entry_hashmap_base(HashMapBase *map, const void *key) {
    uint64_t key_hash = map->hash_func(key) & (map->table_size - 1);

    bool stop = false;

    void *value = NULL;

    Entry *prev;
    Entry *entry = map->table[key_hash];

    if (entry && map->comp_func(entry->key, key)) {

        stop = true;

        map->table[key_hash] = entry->next;

        value = entry->value;

        if (map->drop_func) {
            map->drop_func((void *)entry->key, NULL);
        }
        free(entry);

        entry = NULL;

        --map->current_size;
        return value;
    }

    prev = entry;
    entry = NULL;

    while (entry && !stop) {
        if (map->comp_func(entry->key, key)) {
            stop = true;

            value = entry->value;
            prev->next = entry->next;

            map->drop_func((void *)entry->key, NULL);
            free(entry);

            entry = NULL;
        } else {
            prev = entry;

            entry = entry->next;
        }
    }

    --map->current_size;
    return value;
}

/** get an iterator struct for a hashmap
 *
 * @param map
 *  the hashmap base
 */
IterHashMap *get_iter_hashmap_base(HashMapBase *map) {
    IterHashMap *iter = malloc(sizeof(IterHashMap));

    if (iter == NULL) {
        return NULL;
    }

    iter->current_index = -1;
    iter->current_entry = NULL;

    // set prev_index to something other then what current_index is being set to
    // as to not match for the iter_next_safe_hashmap check at the begging
    iter->prev_index = -2;
    iter->prev_entry = NULL;

    iter->table_size = map->table_size;

    iter->iter_table = map->table;

    return iter;
}

/** free the hashmap struct
 *
 * @param iter
 *  the hashmap iter struct
 */
void drop_iter_hashmap(IterHashMap *iter) {
    free(iter);
}

/** next (unsafe)
 *
 * it is not safe to mutate the table while iterating with this function
 *
 * this function will assign the current key and value to the given pointers
 *
 * @param iter
 *  the hashmap iter struct
 *
 *  @param key
 *   a pointer to another pointer that will hold each new key that gets iterated
 *   over in the for each loop
 *
 *  @param value
 *   a pointer to another pointer that will hold each new value that gets
 *   iterated over during the for each loop
 */
void iter_next_hashmap(IterHashMap *iter, const void **key, void **value) {
    // if current entry is not null then try and get the next Entry from the
    // linked list
    if (iter->current_entry) {
        iter->current_entry = iter->current_entry->next;
    }

    // if there was nothing in the next spot find the next bucket
    if (iter->current_entry == NULL) {
        // get to the next table index so we dont hit the current entry again
        ++iter->current_index;

        // find the next full bucket
        while (iter->current_index < iter->table_size &&
               iter->iter_table[iter->current_index] == NULL) {

            ++iter->current_index;
        }

        // set the new current_entry if we are in bounds
        if (iter->current_index < iter->table_size) {
            // set the new current_entry
            iter->current_entry = iter->iter_table[iter->current_index];
        }
    }

    // if entry is value then set the given ptrs to the current key and value
    // else do nothing as entry will be empty and thus stop the iteration
    if (iter->current_entry) {
        *key = iter->current_entry->key;

        *value = iter->current_entry->value;
    }
}

/** next (remove safe)
 *
 * this will keep track of a previous entry and make sure that we get to the
 * right entry even if the user deletes an entry, though its not "safe" in any
 * other way
 */
// NOTE: this is languishing as i dont care about it right now
void iter_next_safe_hashmap(IterHashMap *iter, const void **key, void **value) {
    // if there is a previous entry and it has the same index as the current
    // entry then reset current to the last entry so we get to the right entry
    // in the next if even if the user removed an entry during iteration
    //
    // this is bad and i feel bad
    if (iter->prev_entry && iter->prev_index == iter->current_index) {
        iter->current_entry = iter->prev_entry;
    }

    // if current entry is not null then try and get the next Entry from the
    // linked list
    if (iter->current_entry) {
        iter->prev_entry = iter->current_entry;

        iter->current_entry = iter->current_entry->next;
    }

    // if there was nothing in the next spot find the next bucket
    if (iter->current_entry == NULL) {
        iter->prev_index = iter->current_index;

        // get to the next table index so we dont hit the current entry again
        ++iter->current_index;

        // find the next full bucket
        while (iter->current_index < iter->table_size &&
               iter->iter_table[iter->current_index] == NULL) {

            ++iter->current_index;
        }

        // set the new current_entry if we are in bounds
        //
        // the last current_entry was from a different bucket so we dont need to
        // bother with the previous entry, just the index for above
        if (iter->current_index < iter->table_size) {
            // set the new current_entry
            iter->prev_index = iter->current_index;

            iter->current_entry = iter->iter_table[iter->current_index];
        }
    }

    // if entry is value then set the given ptrs to the current key and value
    // else do nothing as entry will be empty and thus stop the iteration
    if (iter->current_entry) {
        *key = iter->current_entry->key;

        *value = iter->current_entry->value;
    }
}

/* this was taken from
 * https://github.com/DavidLeeds/hashmap/
 * blob/137d60b3818c22c79d2be5560150eb2eff981a68/src/hashmap.c#L601
 *
 * Recommended hash function for data keys.
 *
 * This is an implementation of the well-documented Jenkins one-at-a-time
 * hash function. See https://en.wikipedia.org/wiki/Jenkins_hash_function
 */
size_t data_hash64(const void *data, size_t len) {
    // cast the data to an 8bit pointer so we can iterate over all the data in
    // memory
    //
    // use sizeof() to find the length of an object
    //
    // sizeof(THING) will return the amount of 8bit blocks for THING on most
    // platforms as far as i can tell
    const uint8_t *byte = (const uint8_t *)data;
    size_t hash = 0;

    for (size_t i = 0; i < len; ++i) {
        hash += *byte++;
        hash += (hash << 10);
        hash ^= (hash >> 6);
    }

    hash += (hash << 3);
    hash ^= (hash >> 11);
    hash += (hash << 15);
    return hash;
}

/** integer hash function
 *
 * this is taken from this stackoverflow
 * https://stackoverflow.com/a/12996028
 * and this article i guess
 * https://zimbry.blogspot.com/2011/09/better-bit-mixing-improving-on.html
 */
uint64_t integer_hash64(uint64_t x) {
    x = (x ^ (x >> 30)) * UINT64_C(0xbf58476d1ce4e5b9);
    x = (x ^ (x >> 27)) * UINT64_C(0x94d049bb133111eb);
    x = x ^ (x >> 31);
    return x;
}

/* taken from
 * https://github.com/skeeto/hash-prospector
 */
uint64_t integer_hash32(uint32_t x) {
    x ^= x >> 16;
    x *= 0x7feb352d;
    x ^= x >> 15;
    x *= 0x846ca68b;
    x ^= x >> 16;
    return (uint64_t)x;
}

/** print a hasmap error to the console
 *
 * a simple function to print out what a HashMapResult is
 * NOTE: it could be named better
 */
void print_hashmap_error(enum HashMapResult h_result) {
    switch (h_result) {
        case FailedToInsert: {
            printf("failed to insert\n");
            break;
        }
        case FailedToInsertDuplicate: {
            printf("failed to insert -- duplicate\n");
            break;
        }
        case FailedToInsertNoMemory: {
            printf("failed to insert -- no memory\n");
            break;
        }
        case FailedToRehashNoMemory: {
            printf("failed to rehash -- no memory\n");
            break;
        }
        case Success: {
            printf("success\n");
            break;
        }
    }
}
