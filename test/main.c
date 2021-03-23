#include "../src/hashmap.h"

typedef struct {
    uint64_t x;
    uint64_t y;
} MapPoint;

uint64_t key_hash(const MapPoint *point) {
    return data_hash64(point, sizeof(MapPoint));
}

int main() {
    HAHSMAP(MapPoint, uint64_t *) map;

    init_hashmap(map, key_hash);

    return 0;
}
