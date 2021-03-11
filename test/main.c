#include <stdio.h>
#include <stdlib.h>

#include "../src/hashmap.h"

void drop_value(void *value) {
    return;
}

int main() {
    HashMap *map = create_hashmap();

    uint64_t iter_count = 10000;

    for (uint64_t i = 0; i < iter_count; ++i) {
        uint64_t *value = (uint64_t *)malloc(sizeof(uint64_t));

        *value = i;

        if (insert_hashmap(map, i, value) != Success) {
            printf("bad insert\n");
        }
    }

    bool not_found = false;

    printf("checking all keys \n");
    for (int i = 0; i < iter_count && !not_found; ++i) {
        if (contains_key_hashmap(map, i) == false) {
            not_found = true;
        }
    }

    IterHashMap *iter = get_iter_hashmap(map);
    uint64_t key = 0;
    void *value = NULL;
    for_each(iter, key, value) {
        printf("in main key %lu -- value %d\n", key, *(int *)value);
    }

    drop_hashmap(map, drop_value);

    if (not_found) {
        printf("could not find all keys\n");
    } else {
        printf("found all keys\n");
    }
}
