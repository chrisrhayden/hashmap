#include <stdio.h>

#include "../src/hashmap.h"

typedef struct {
    uint64_t x;
    uint64_t y;
} MapPoint;

HAHSMAP(HashMapPoint, MapPoint, uint64_t);

uint64_t key_hash(const MapPoint *point) {
    return data_hash64(point, sizeof(MapPoint));
}

void drop_value_func(uint64_t *value) {
    return;
}

int main() {
    HashMapPoint *map;

    init_hashmap(map, key_hash, drop_value_func);

    if (map == NULL) {
        printf("did not allocate memory\n");

        return 1;
    }

    MapPoint key = {.x = 10, .y = 11};

    uint64_t *value = malloc(sizeof(uint64_t));
    *value = 10;

    insert_hashmap(map, &key, value);

    bool success = false;

    contains_key_hashmap(map, &key, &success);

    if (success == true) {
        printf("i dont believe you\n");
    }

    return 0;
}
