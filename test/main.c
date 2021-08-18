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

uint64_t hash_data(char *key) {
    // int str_len = strnlen(key, INTMAX_MAX);

    // return data_hash64(key, sizeof(*key));
    return integer_hash64(*key);
}

bool comp_data_func(char *key_1, char *key_2) {
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
    HashMapStr *map = init_map();

    if (map == NULL) {
        printf("did not allocate memory\n");
        return 1;
    }

    enum HashMapResult result = Success;

    for (int i = 0x21; i <= 0x7E && result == Success; ++i) {
        char *test_key = malloc(sizeof(*test_key));

        char *test_value = malloc(sizeof(*test_value));

        *test_key = (char)i;
        *test_value = (char)i;

        insert_hashmap(map, test_key, test_value, result);

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
        } else {
            printf("> char is %c \n", *iter_key);
        }
    }

    printf("%c\n", *iter_key);

    iter_key = NULL;
    value = NULL;

    for_each_drop(iter, iter_key, value) {
        printf("char %c\n", *iter_key);
        free(iter_key);
        free(value);
    }

    drop_hashmap(map);

    printf(">>>>>>>>>>> \n");

    printf("done\n");

    return 0;
}
