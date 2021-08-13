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

// uint64_t key_hash_func(const int *point) {
//     return integer_hash64(*point);
// }

// bool key_comp_func(const int *data_1, const int *data_2) {
//     return (*data_1 == *data_2);
// }

// void key_drop_func(int *key, int *value) {
//     if (key != NULL) {
//         free(key);
//     }

//     if (value != NULL) {
//         free(value);
//     }
// }

// HashMapInt *init_map() {
//     HashMapInt *map;

//     init_hashmap(map, key_hash_func, key_comp_func, key_drop_func);

//     return map;
// }

// uint64_t hash_data(const struct TestStruct *key) {
//     return data_hash64(key, sizeof(*key));
// }

// bool comp_data_func(const struct TestStruct *key_1,
//                     const struct TestStruct *key_2) {
//     return ((key_1->x == key_2->x) && (key_1->y == key_2->y));
// }

// void drop_data_func(struct TestStruct *key, struct TestStruct *data) {
//     free(data);
// }

uint64_t hash_data(const char *key) {
    int str_len = strnlen(key, INTMAX_MAX);

    return data_hash64(key, sizeof(*key) * str_len);
}

bool comp_data_func(const char *key_1, const char *key_2) {
    return (strcmp(key_1, key_2) == 0);
}

void drop_data_func(char *key, char *data) {
    free(data);
}

HashMapStr *init_map() {
    HashMapStr *map;

    init_hashmap(map, hash_data, comp_data_func, drop_data_func);

    return map;
}

// int main() {
//     // ProfilerStart("./out/gperf_tools.log");

//     // int iter_count = 900000;
//     // int iter_count = 100000;
//     int iter_count = 10000;
//     // int iter_count = 100;

//     time_t before = clock();

//     HashMapInt *map = init_map();

//     if (map == NULL) {
//         printf("did not allocate memory\n");
//         return 1;
//     }

//     enum HashMapResult result = Success;

//     for (int i = 0; i < iter_count && result == Success; ++i) {
//         int *key = malloc(sizeof(*key));
//         int *value = malloc(sizeof(*value));
//         *key = i;
//         *value = i;

//         insert_hashmap(map, key, value, result);

//         if (result != Success) {
//             printf("bad insert %d\n", i);
//             break;
//         }
//     }

//     if (result != Success) {
//         printf("could not insert in to hashmap\n");
//         print_hashmap_error(result);

//         drop_hashmap(map);

//         return 1;
//     }

//     bool contains = false;
//     IterHashMap *iter;
//     get_iter_hashmap(map, iter);
//     int *iter_key;
//     int *value;

//     // this is silly but its just to have something to do with the hashmap
//     for_each(iter, iter_key, value) {
//         contains_key_hashmap(map, iter_key, contains);

//         if (contains == false) {
//             printf("could not find key\n");
//             break;
//         }
//     }

//     time_t now = clock();
//     double delta = (double)(now - before) / CLOCKS_PER_SEC;

//     int key = 10;
//     contains = false;
//     for (int i = 0; i < iter_count; ++i) {
//         contains_key_hashmap(map, &key, contains);

//         if (contains == false) {
//             printf("did not find %d\n", key);
//             break;
//         }
//     }

//     printf("> delta %f\n", delta);

//     if (contains != true) {
//         printf("did not find all\n");
//     }

//     // iter_key = NULL;
//     // value = NULL;
//     // contains = false;

//     // for_each_safe(iter, iter_key, value) {
//     //     contains_key_hashmap(map, iter_key, contains);

//     //     if (contains == false) {
//     //         break;
//     //     }
//     // }

//     // if (contains == false) {
//     //     printf("could not find all safe\n");
//     // }

//     // if (contains != false) {
//     //     contains = false;

//     //     int *value_to_fill = NULL;

//     //     int new_key = 99;

//     //     remove_entry_hashmap(map, &new_key, value_to_fill);

//     //     if (value_to_fill == NULL) {
//     //         printf("could not find value\n");
//     //     }

//     //     free(value_to_fill);

//     //     contains_key_hashmap(map, &new_key, contains);

//     //     if (contains != false) {
//     //         printf("key was not removed\n");
//     //     }
//     // }

//     // drop_iter_hashmap(iter);
//     drop_hashmap(map);

//     // ProfilerStop();
//     return 0;
// }

int main() {
    int iter_count = 1000;
    int str_len = 5;
    HashMapStr *map = init_map();

    if (map == NULL) {
        printf("did not allocate memory\n");
        return 1;
    }

    enum HashMapResult result = Success;

    for (int i = 0; i < iter_count && result == Success; ++i) {
        char *test = malloc(sizeof(*test));

        snprintf(test, sizeof(char) * str_len, "%d", i);

        printf("%s\n", test);

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

    char *key = malloc(sizeof(*key));
    contains = false;
    for (int i = 0; i < iter_count; ++i) {
        snprintf(key, sizeof(char) * str_len, "%d", i);

        contains_key_hashmap(map, key, contains);

        if (contains == false) {
            printf("did not find %s\n", key);
            break;
        }
    }

    void *return_value = NULL;

    char get_key = '+';

    get_value_hashmap(map, get_key, value);

    int longest = get_longest_chain(map);

    printf("longest chain %d\n", longest);

    drop_hashmap(map);
    printf("done\n");

    return 0;
}
