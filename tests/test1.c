#include <stdio.h>
#include <assert.h>
#include <stddef.h>

#include <hashmap.h>

static void test_create_returns_map(void) {
	hashmap_t *map;
	map = hashmap_create();
	assert(map != NULL);
	assert(hashmap_getsize(map) == 0);
	hashmap_clear(&map);
	assert(map == NULL);
}

static void test_create_initial_state(void) {
	hashmap_t *map;
	hashmap_info_t info;
	map = hashmap_create();
	assert(map != NULL);
	assert(hashmap_info(map, &info) == HASHMAP_OK);
	assert(info.nr_buckets == 16);
	assert(info.nr_entries == 0);
	assert(info.load_factor == 0.0f);
	assert(info.resize_in_progress == false);
	assert(info.nr_buckets_old == 0);
	assert(info.nr_entries_old == 0);
	assert(info.resize_bucket == 0);
	hashmap_clear(&map);
	assert(map == NULL);
}

int main(void) {
	test_create_returns_map();
	test_create_initial_state();
	printf("All tests passed.\n");
	return 0;
}
