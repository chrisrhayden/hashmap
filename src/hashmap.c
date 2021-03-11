#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "spookyhash_api.h"

#include "hashmap.h"

#define STARTING_SIZE 1024
#define GROWTH_FACTOR 2
#define MAX_LOAD_FACTOR 1

// a wrapper to get a valid hash for the current table
uint64_t hash_64(HashMap *map, uint64_t key) {
    return spookyhash_64(&key, 1, map->seed) & (map->table_size - 1);
}

/** the internal create_hashmap func */
HashMap *_create_hashmap(int table_size) {
    HashMap *map = (HashMap *)malloc(sizeof(HashMap));

    if (map == NULL) {
        return NULL;
    }

    map->table = (Entry **)calloc(table_size, sizeof(Entry *));
    // map->table = (Entry **)malloc(sizeof(Entry *) * table_size);

    if (map->table == NULL) {
        free(map);

        return NULL;
    }

    // NOTE: this is kinda whatever
    map->seed = rand();

    map->table_size = table_size;

    map->current_size = 0;

    return map;
}

/* a wrapper to get a default sized table */
HashMap *create_hashmap() {
    return _create_hashmap(STARTING_SIZE);
}

/** drop the hashmap table, entrys and values
 *
 * the values will not be freed if there is no drop_func
 *
 * NOTE: the value check could be enforced from the drop_func but it
 * probably is safer to do that here and let the drop_func assume it
 * will always have valid data
 */
void drop_table(HashMap *map, DropValueFunc drop_func) {
    Entry *entry;
    Entry *temp;

    for (int i = 0; i < map->table_size; ++i) {
        entry = map->table[i];

        while (entry != NULL) {
            if (drop_func && entry->value) {
                drop_func(entry->value);
            }

            temp = entry->next;

            free(entry);

            entry = temp;
        }
    }

    free(map->table);
}

/** drop the whole hashmap */
void drop_hashmap(HashMap *map, DropValueFunc drop_func) {
    if (map->table) {
        drop_table(map, drop_func);
    }

    free(map);
}

/** create an entry struct */
Entry *create_entry(uint64_t key, void *value) {
    Entry *entry = (Entry *)malloc(sizeof(Entry));

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
 * this is where entrys will acutely be entered in to the table/linked list
 */
enum HashMapResult _insert_hashmap(HashMap *map, uint64_t key, void *value) {
    uint64_t key_hash = hash_64(map, key);

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

    /* if there is data in a given bucket see if there is a matching key
     * if not then add the new entry to the end of the buckets list
     * else just return false
     */
    bool found = false;

    Entry *table_entry = map->table[key_hash];

    // check all entrys in the list
    while (table_entry->next != NULL && !found) {
        if (table_entry->key == key) {
            found = true;
        } else {
            table_entry = table_entry->next;
        }
    }

    if (found || table_entry->key == key) {
        return FailedToInsertDuplicate;
    }

    // just add entry to the end of the list
    table_entry->next = create_entry(key, value);

    if (table_entry->next == NULL) {
        return FailedToInsertNoMemory;
    }

    return Success;
}

/** rehash the whole table
 *
 * this will rehash the whole table to a bigger size based on the GROWTH_FACTOR
 */
enum HashMapResult rehash_hashmap(HashMap *map) {
    enum HashMapResult result = Success;

    int new_table_size = map->table_size * GROWTH_FACTOR;

    HashMap *temp_map = _create_hashmap(new_table_size);

    if (temp_map == NULL) {
        return FailedToInsertNoMemory;
    }

    for (int i = 0; i < map->table_size && result == Success; ++i) {

        for (Entry *entry = map->table[i]; entry && result == Success;
             entry = entry->next) {

            result = _insert_hashmap(temp_map, entry->key, entry->value);
        }
    }

    if (result != Success) {
        drop_hashmap(temp_map, NULL);

        return result;

    } else { // assign the new table to the user's hashmap

        // drop the old table but dont drop the values
        drop_table(map, NULL);

        // set the new table to the old hashmap
        map->table = temp_map->table;
        map->table_size = temp_map->table_size;
        map->seed = temp_map->seed;

        // this should be the same no mater what
        map->current_size = temp_map->current_size;

        // drop the temp map
        temp_map->table = NULL;

        drop_hashmap(temp_map, NULL);

        return Success;
    }
}

/** the insert entry point
 *
 * this will rehash the table when the current size is the same as the table
 * length * MAX_LOAD_FACTOR
 */
enum HashMapResult insert_hashmap(HashMap *map, uint64_t key, void *value) {
    enum HashMapResult result = Success;

    if (map->current_size + 1 == (map->table_size * MAX_LOAD_FACTOR)) {
        result = rehash_hashmap(map);
    }

    if (result != Success) {
        return result;
    }

    result = _insert_hashmap(map, key, value);

    if (result != Success) {
        return result;
    }

    ++map->current_size;

    return Success;
}

// check is a key is in the table
bool contains_key_hashmap(HashMap *map, uint64_t key) {
    uint64_t key_hash = hash_64(map, key);

    bool found = false;

    Entry *entry = map->table[key_hash];

    while (entry != NULL && !found) {
        if (entry->key == key) {
            found = true;
        } else {
            entry = entry->next;
        }
    }

    return found;
}

/** get an iterator for a hashmap
 *
 * this is definitely not thread safe
 */
IterHashMap *get_iter_hashmap(HashMap *map) {
    IterHashMap *iter = (IterHashMap *)malloc(sizeof(IterHashMap));

    if (iter == NULL) {
        return NULL;
    }

    iter->current_index = -1;

    iter->current_entry = NULL;

    iter->table_size = map->table_size;

    iter->iter_table = map->table;

    return iter;
}

/** get the next key from ether a bucket or from the linked list */
void iter_next_hashmap(IterHashMap *iter, uint64_t *key, void **value) {
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
