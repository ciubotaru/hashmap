#include <stdio.h>
#include <assert.h>

#include <hashmap.h>

static void test_clear_null_pointer(void) {
	hashmap_clear(NULL);
}

static void test_clear_null_map(void) {
	hashmap_t *map = NULL;
	hashmap_clear(&map);
	assert(map == NULL);
}

static void test_clear_empty_map(void) {
	hashmap_t *map;
	map = hashmap_create();
	assert(map != NULL);
	hashmap_clear(&map);
	assert(map == NULL);
}

static void test_clear_nonempty_map(void) {
	hashmap_t *map = NULL;
	int key = 42;
	int data = 1234;
	assert(hash_insert(&map, &key, sizeof(key),
			   &data, sizeof(data)) == 0);
	assert(map != NULL);
	hashmap_clear(&map);
	assert(map == NULL);
}

static void test_clear_multiple_entries(void) {
	hashmap_t *map = NULL;
	int keys[] = { 1, 2, 3 };
	int data[] = { 10, 20, 30 };
	hashmap_info_t info;
	for (size_t i = 0; i < 3; i++) {
		assert(hash_insert(&map,
				   &keys[i], sizeof(keys[i]),
				   &data[i],
				   sizeof(data[i])) == 0);
	}
	assert(hashmap_info(map, &info) == 0);
	assert(info.nr_entries == 3);
	hashmap_clear(&map);
	assert(map == NULL);
}

static void test_clear_during_resize(void) {
	hashmap_t *map = NULL;
	hashmap_info_t info;
	int key;
	int data;

	/*
	 * Insert enough entries to start a resize.
	 * The default growth threshold is 1.2, so the 20th
	 * insertion into a 16-bucket table crosses the threshold.
	 */
	for (int i = 0; i < 20; i++) {
		key = i;
		data = i * 10;
		assert(hash_insert(&map, &key, sizeof(key),
				   &data, sizeof(data)) == 0);
	}
	assert(hashmap_info(map, &info) == 0);
	assert(info.resize_in_progress == true);
	hashmap_clear(&map);
	assert(map == NULL);
}

int main(void) {
	test_clear_null_pointer();
	test_clear_null_map();
	test_clear_empty_map();
	test_clear_nonempty_map();
	test_clear_multiple_entries();
	test_clear_during_resize();
	printf("All tests passed.\n");
	return 0;
}
