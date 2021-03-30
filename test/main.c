#include <stdio.h>

#include <time.h>

#include "../src/hashmap.h"

HAHSMAP(HashMapInt, int, int);

uint64_t key_hash_func(const int *point) {
    return integer_hash64(*point);
}

bool key_comp_func(const int *data_1, const int *data_2) {
    return (*data_1 == *data_2);
}

void kye_drop_func(int *key, int *value) {
    if (key != NULL) {
        free(key);
    }

    if (value != NULL) {
        free(value);
    }
}

HashMapInt *init_map() {
    HashMapInt *map;

    init_hashmap(map, key_hash_func, kye_drop_func, key_comp_func);

    return map;
}

int main() {
    int iter_count = 10000;
    // int iter_count = 100;

    time_t before = clock();

    HashMapInt *map = init_map();

    if (map == NULL) {
        printf("did not allocate memory\n");
        return 1;
    }

    bool success = false;
    for (int i = 0; i < iter_count; ++i) {
        int *key = malloc(sizeof(*key));
        int *value = malloc(sizeof(*value));

        *key = i;
        *value = i;

        insert_hashmap(map, key, value, success);

        if (success == false) {
            break;
        }
    }

    if (success == false) {
        printf("could not insert in to hashmap n");
        drop_hashmap(map);
        return 1;
    }

    bool contains = false;
    IterHashMap *iter;

    get_iter_hashmap(map, iter);

    int *iter_key;
    int *value;

    contains = false;

    for_each(iter, iter_key, value) {
        contains_key_hashmap(map, iter_key, contains);

        if (contains == false) {
            break;
        }
    }

    time_t now = clock();
    double delta = (double)(now - before) / CLOCKS_PER_SEC;

    contains = false;
    for (int i = 0; i < iter_count; ++i) {
        contains_key_hashmap(map, &i, contains);

        if (contains == false) {
            break;
        }
    }

    if (contains == true) {
        printf("found all\n");
    }

    printf("> delta %f\n", delta);

    iter_key = NULL;
    value = NULL;
    contains = false;

    for_each_safe(iter, iter_key, value) {
        contains_key_hashmap(map, iter_key, contains);

        if (contains == false) {
            break;
        }
    }

    if (contains == true) {
        printf("found all safe\n");
    }

    if (contains != false) {
        contains = false;

        int *value_to_fill = NULL;

        int new_key = 99;

        remove_entry_hashmap(map, &new_key, value_to_fill);

        if (value_to_fill != NULL) {
            printf("found value %d\n", *(int *)value_to_fill);
        }

        free(value_to_fill);

        contains_key_hashmap(map, &new_key, contains);

        if (contains == false) {
            printf("key was removed\n");
        }
    }

    drop_iter_hashmap(iter);
    drop_hashmap(map);
    printf("done\n");

    return 0;
}
