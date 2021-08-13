#include <stdio.h>
// #include <gperftools/profiler.h>

#include <string.h>
#include <time.h>

#include "../src/hashmap.h"

// struct TestStruct {
//     int x;
//     int y;
// };

// HASHMAP(HashMapData, struct TestStruct, struct TestStruct);
HASHMAP(HashMapStr, char, char);

uint64_t hash_data(const char *key) {
    // int str_len = strnlen(key, INTMAX_MAX);

    return data_hash64(key, sizeof(*key));
}

bool comp_data_func(const char *key_1, const char *key_2) {
    return *key_1 == *key_2;
}

void drop_data_func(char *key, char *data) {
    free(data);
}

HashMapStr *init_map() {
    HashMapStr *map;

    init_hashmap(map, hash_data, comp_data_func, drop_data_func);

    return map;
}

int main() {
    // int iter_count = 1000;
    int iter_count = 100;
    int str_len = 5;
    HashMapStr *map = init_map();

    if (map == NULL) {
        printf("did not allocate memory\n");
        return 1;
    }

    enum HashMapResult result = Success;

    for (int i = 33; i < 44 && result == Success; ++i) {
        char *test = malloc(sizeof(*test));

        *test = (char)i;

        // snprintf(test, sizeof(char) * str_len, "%d", i);

        printf("%c\n", *test);

        insert_hashmap(map, test, test, result);

        if (result != Success) {
            printf("bad insert %d\n", i);
            break;
        }
    }

    printf("inserted\n");

    if (result != Success) {
        printf("could not insert in to hashmap\n");
        print_hashmap_error(result);

        drop_hashmap(map);

        return 1;
    }

    bool contains = false;
    IterHashMap *iter;
    get_iter_hashmap(map, iter);
    char *iter_key;
    char *value;

    // this is silly but its just to have something to do with the hashmap
    for_each(iter, iter_key, value) {
        contains_key_hashmap(map, iter_key, contains);

        if (contains == false) {
            printf("could not find key\n");
            break;
        }
    }

    // char *key = malloc(sizeof(*key));
    // contains = false;
    // for (int i = 0; i < iter_count; ++i) {
    //     snprintf(key, sizeof(char) * str_len, "%d", i);

    //     contains_key_hashmap(map, key, contains);

    //     if (contains == false) {
    //         printf("did not find %s\n", key);
    //         break;
    //     }
    // }

    void *return_value = NULL;

    char get_key = '+';

    get_value_hashmap(map, &get_key, return_value);

    if (return_value == NULL) {
        printf("did not find the key \n");
    } else {
        printf("found %c \n", *(char *)return_value);
    }

    int longest = get_longest_chain(map);

    printf("longest chain %d\n", longest);

    drop_hashmap(map);
    printf("done\n");

    return 0;
}
