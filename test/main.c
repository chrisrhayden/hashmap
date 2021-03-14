#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "../src/hashmap.h"

void drop_value(void *value) {
    return;
}

int main() {
    HashMap *map = create_hashmap();

    uint64_t whatever = 10;
    uint64_t iter_count = 10000;

    for (uint64_t i = 0; i < iter_count; ++i) {

        if (insert_hashmap(map, i, &whatever) != Success) {
            printf("bad insert\n");
        }
    }

    bool error = false;

    for (int i = 0; i < iter_count && !error; ++i) {
        if (contains_key_hashmap(map, i) == false) {
            error = true;
        }
    }

    if (error == false) {
        printf("all keys found\n");

        IterHashMap *iter = get_iter_hashmap(map);
        uint64_t key = 0;
        void *value = NULL;

        for_each(iter, key, value) {
            printf("in main key %lu -- value %lu -- hash %lu\n", key,
                   *(uint64_t *)value, hash64(map, key));
        }

        drop_iter_hashmap(iter);
    }

    drop_hashmap(map, drop_value);

    if (error) {
        printf("error\n");
    } else {
        printf("good bye\n");
    }
}
