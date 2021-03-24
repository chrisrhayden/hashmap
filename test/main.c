#include <stdio.h>

#include "../src/hashmap.h"

typedef struct {
    uint64_t x;
    uint64_t y;
} MapPoint;

HAHSMAP(HashMapPoint, MapPoint, uint64_t);

uint64_t key_hash_func(const MapPoint *point) {
    return data_hash64(point, sizeof(MapPoint));
}

bool key_comp_func(const MapPoint *data_1, const MapPoint *data_2) {
    if (data_1->x == data_2->x && data_1->y == data_2->y) {
        return true;
    }

    return false;
}

void drop_value_func(uint64_t *value) {
    free(value);

    return;
}

MapPoint *create_map_point(int x, int y) {
    MapPoint *key = malloc(sizeof(MapPoint));

    key->x = x;
    key->y = y;

    return key;
}

HashMapPoint *init_map() {
    HashMapPoint *map;

    init_hashmap(map, key_hash_func, drop_value_func, key_comp_func);

    return map;
}

int main() {
    // int iter_count = 10000;
    int iter_count = 100;

    HashMapPoint *map = init_map();

    if (map == NULL) {
        printf("did not allocate memory\n");
        return 1;
    }

    bool success = false;
    for (int i = 0; i < iter_count; ++i) {
        MapPoint *key = create_map_point(i, i + i);

        uint64_t *value = malloc(sizeof(uint64_t));
        *value = (uint64_t)i;

        insert_hashmap(map, key, value, success);

        if (success == false) {
            break;
        }
    }

    if (success == false) {
        printf("could not insert in to hashmap n");
        drop_hashmap(map);
        return 1;
    } else {
        printf("inserted all\n");
    }

    bool contains = false;

    for (int i = 1; i < iter_count; ++i) {
        MapPoint *key = create_map_point(i, i + i);

        contains_key_hashmap(map, key, contains);

        if (contains == false) {
            break;
        }
    }

    if (contains != false) {
        contains = false;

        uint64_t *value_to_fill = NULL;

        MapPoint *key = create_map_point(99, 198);

        remove_entry_hashmap(map, key, value_to_fill);

        if (value_to_fill != NULL) {
            printf("found value\n");
        }

        contains_key_hashmap(map, key, contains);

        if (contains == false) {
            printf("key was removed\n");
        }
    }

    drop_hashmap(map);

    return 0;
}
