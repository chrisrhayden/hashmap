#include <math.h>
#include <stdio.h>

#include "hashmap_base.h"

/* init/create the hashmap base
 */
HashMapBase *init_hashmap_base(HashFunc hash_func, DropFunc drop_func,
                               CompFunc comp_func, uint64_t size) {
    HashMapBase *map = malloc(sizeof(*map));

    map->table_size = size;
    map->table = (Entry **)calloc(sizeof(Entry *), size);

    if (map->table == NULL) {
        return false;
    }

    map->current_size = 0;

    map->hash_func = hash_func;
    map->drop_func = drop_func;
    map->comp_func = comp_func;

    return map;
}

/** drop the hashmap table, entrys and values
 *
 * the values will not be freed if there is no drop_func
 *
 * NOTE: the value check could be enforced from the drop_func but it
 * probably is safer to do that here and let the drop_func assume it
 * will always have valid data
 */
void drop_table(HashMapBase *map, DropFunc drop_func) {
    Entry *entry;
    Entry *temp;

    for (int i = 0; i < map->table_size; ++i) {
        entry = map->table[i];

        while (entry != NULL) {
            if (drop_func) {
                map->drop_func((void *)entry->key, entry->value);
            }

            temp = entry->next;

            free(entry);

            entry = temp;
        }
    }

    free(map->table);
}

/** drop the whole hashmap */
void drop_hashmap_base(HashMapBase *map) {
    if (map->table) {
        drop_table(map, map->drop_func);
    }

    free(map);
}

/** create an entry struct */
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

/** the actual insert function
 *
 * this is where entrys will acutely be entered in to the table/linked
 list
 */
enum HashMapResult _insert_hashmap(HashMapBase *map, const void *key,
                                   void *value) {
    uint64_t key_hash = map->hash_func(key) & (map->table_size - 1);

    // if the bucket entry is null then set the new entry as the start of the
    // linked list for the given bucket
    if (map->table[key_hash] == NULL) {
        Entry *entry = create_entry(key, value);

        if (entry == NULL) {
            return FailedToInsertNoMemory;
        }

        map->table[key_hash] = entry;

        return Success;
    }

    // if there is data in a given bucket see if there is a matching key
    // if not then add the new entry to the end of the buckets list
    bool found = false;

    Entry *table_entry = map->table[key_hash];

    // check all entrys in the list
    //
    // if we are at the first entry and next is null the we skip the loop and go
    // to the last key check
    // else we check the key in the loop
    while (table_entry->next != NULL && !found) {
        if (map->comp_func(table_entry->key, key)) {
            found = true;
        } else {
            table_entry = table_entry->next;
        }
    }

    // check the last (or first) entry in the list
    if (found || map->comp_func(table_entry->key, key)) {
        return FailedToInsertDuplicate;
    }

    // just add the entry to the end of the list
    table_entry->next = create_entry(key, value);

    if (table_entry->next == NULL) {
        return FailedToInsertNoMemory;
    }

    return Success;
}

/** rehash the whole table
 *
 * this will rehash the whole table to a bigger size based on the GROWTH_FACTOR
 *
 * it would be faster to just give the new table the old entrys but this is
 * fine for now
 */
enum HashMapResult rehash_hashmap(HashMapBase *map) {
    enum HashMapResult result = Success;

    int new_table_size = map->table_size * GROWTH_FACTOR;

    HashMapBase *temp_map = init_hashmap_base(map->hash_func, map->drop_func,
                                              map->comp_func, new_table_size);

    if (temp_map == NULL) {
        // this is technically true i guess
        return FailedToInsertNoMemory;
    }

    // go though the enter table rehashing all the entrys
    for (int i = 0; i < map->table_size && result == Success; ++i) {
        // iterate over the buckets linked list
        Entry *entry = map->table[i];

        while (entry && result == Success) {

            result = _insert_hashmap(temp_map, entry->key, entry->value);

            entry = entry->next;
        }
    }

    // if there was an error drop the temp and let the user know
    if (result != Success) {
        drop_hashmap_base(temp_map);

    } else { // assign the new table to the user's hashmap
        // drop the old table but dont drop the values

        drop_table(map, NULL);

        // set the new table to the old hashmap
        map->table = temp_map->table;
        map->table_size = temp_map->table_size;

        // ignore the temp table when dropping as its now the new table for the
        // users map
        temp_map->table = NULL;

        // drop the temp map
        free(temp_map);

        // this is probably redundant
        result = Success;
    }

    return result;
}

/** the insert entry point
 *
 * this will rehash the table when the current size is the same as the table
 * length * MAX_LOAD_FACTOR
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

    result = _insert_hashmap(map, key, value);

    if (result != Success) {
        return result;
    }

    // only increment if we know _insert_hashmap succeeded
    ++map->current_size;

    return Success;
}

// check is a key is in the table
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

/** delete the entry for the given key */
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

    return value;
}

/** get an iterator for a hashmap
 *
 * this is definitely not thread safe
 */
IterHashMap *get_iter_hashmap_base(HashMapBase *map) {
    IterHashMap *iter = malloc(sizeof(IterHashMap));

    if (iter == NULL) {
        return NULL;
    }

    iter->current_index = -1;
    iter->current_entry = NULL;

    iter->prev_index = -2;
    iter->prev_entry = NULL;

    iter->table_size = map->table_size;

    iter->iter_table = map->table;

    return iter;
}

void drop_iter_hashmap(IterHashMap *iter) {
    free(iter);
}

/** next unsafe
 *
 * it is not safe to mutate the table while iterating with this function
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

/** next safe
 *
 * this will keep track of a previous entry and make sure that we get to the
 * right entry even if the user deletes an entry
 */
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
    // memory using sizeof() to find the length, sizeof(THING) will return the
    // amount of 8bit blocks for THING on most platforms as far as i know
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
