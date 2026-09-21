#include <stdio.h>
#include <assert.h>
#include <stddef.h>

#include <hashmap.h>

static void test_opt_null_map(void) {
	assert(hashmap_opt(NULL, HASHMAP_MIN_BUCKETS, 32.0f) == HASHMAP_INVALID_ARG);
}

static void test_opt_invalid_key(void) {
	hashmap_t *map;
	map = hashmap_create();
	assert(map != NULL);
	assert(hashmap_opt(map, -1, 32.0f) == HASHMAP_INVALID_ARG);
	assert(hashmap_opt(map, 999, 32.0f) == HASHMAP_INVALID_ARG);
	hashmap_clear(&map);
	assert(map == NULL);
}

static void test_opt_default_values(void) {
	hashmap_t *map;
	hashmap_info_t info;
	map = hashmap_create();
	assert(map != NULL);
	assert(hashmap_info(map, &info) == HASHMAP_OK);
	assert(info.nr_buckets == 16);
	assert(hashmap_opt(map, HASHMAP_GROW_THRESHOLD, 1.2f) == HASHMAP_OK);
	assert(hashmap_opt(map, HASHMAP_SHRINK_THRESHOLD, 0.3f) == HASHMAP_OK);
	assert(hashmap_opt(map, HASHMAP_MIN_BUCKETS, 16.0f) == HASHMAP_OK);
	hashmap_clear(&map);
	assert(map == NULL);
}

static void test_opt_grow_threshold(void) {
	hashmap_t *map;
	map = hashmap_create();
	assert(map != NULL);
	assert(hashmap_opt(map, HASHMAP_GROW_THRESHOLD, 1.5f) == HASHMAP_OK);
	assert(hashmap_opt(map, HASHMAP_GROW_THRESHOLD, 2.0f) == HASHMAP_OK);
	hashmap_clear(&map);
	assert(map == NULL);
}

static void test_opt_shrink_threshold(void) {
	hashmap_t *map;
	map = hashmap_create();
	assert(map != NULL);
	assert(hashmap_opt(map, HASHMAP_SHRINK_THRESHOLD, 0.2f) == HASHMAP_OK);
	assert(hashmap_opt(map, HASHMAP_SHRINK_THRESHOLD, 0.1f) == HASHMAP_OK);
	hashmap_clear(&map);
	assert(map == NULL);
}

static void test_opt_min_buckets(void) {
	hashmap_t *map;
	hashmap_info_t info;
	map = hashmap_create();
	assert(map != NULL);
	assert(hashmap_opt(map, HASHMAP_MIN_BUCKETS, 32.0f) == HASHMAP_OK);
	assert(hashmap_info(map, &info) == HASHMAP_OK);
	assert(info.nr_buckets == 32);
	assert(!info.resize_in_progress);
	hashmap_clear(&map);
	assert(map == NULL);
}

static void test_opt_invalid_grow_threshold(void) {
	hashmap_t *map;
	hashmap_info_t info;
	map = hashmap_create();
	assert(map != NULL);
	assert(hashmap_opt(map, HASHMAP_GROW_THRESHOLD, 0.0f) == HASHMAP_INVALID_ARG);
	assert(hashmap_opt(map, HASHMAP_GROW_THRESHOLD, -1.0f) == HASHMAP_INVALID_ARG);
	assert(hashmap_info(map, &info) == HASHMAP_OK);
	assert(info.nr_buckets == 16);
	hashmap_clear(&map);
	assert(map == NULL);
}

static void test_opt_invalid_shrink_threshold(void) {
	hashmap_t *map;
	map = hashmap_create();
	assert(map != NULL);
	assert(hashmap_opt(map, HASHMAP_SHRINK_THRESHOLD, -1.0f) == HASHMAP_INVALID_ARG);
	hashmap_clear(&map);
	assert(map == NULL);
}

static void test_opt_threshold_order(void) {
	hashmap_t *map;
	map = hashmap_create();
	assert(map != NULL);
	assert(hashmap_opt(map, HASHMAP_GROW_THRESHOLD, 1.0f) == HASHMAP_INVALID_ARG);
	assert(hashmap_opt(map, HASHMAP_SHRINK_THRESHOLD, 1.2f) == HASHMAP_INVALID_ARG);
	hashmap_clear(&map);
	assert(map == NULL);
}

static void test_opt_two_move_constraint(void) {
	hashmap_t *map;
	map = hashmap_create();
	assert(map != NULL);
	assert(hashmap_opt(map, HASHMAP_SHRINK_THRESHOLD, 0.3f) == HASHMAP_OK);
	assert(hashmap_opt(map, HASHMAP_GROW_THRESHOLD, 1.2f) == HASHMAP_OK);
	assert(hashmap_opt(map, HASHMAP_SHRINK_THRESHOLD, 0.31f) == HASHMAP_INVALID_ARG);
	assert(hashmap_opt(map, HASHMAP_GROW_THRESHOLD, 1.19f) == HASHMAP_INVALID_ARG);
	hashmap_clear(&map);
	assert(map == NULL);
}

static void test_opt_invalid_min_buckets(void) {
	hashmap_t *map;
	map = hashmap_create();
	assert(map != NULL);
	assert(hashmap_opt(map, HASHMAP_MIN_BUCKETS, 0.0f) == HASHMAP_INVALID_ARG);
	assert(hashmap_opt(map, HASHMAP_MIN_BUCKETS, -1.0f) == HASHMAP_INVALID_ARG);
	hashmap_clear(&map);
	assert(map == NULL);
}

static void test_opt_min_buckets_with_entries(void) {
	hashmap_t *map;
	hashmap_info_t info;
	int key;
	int data;
	map = hashmap_create();
	assert(map != NULL);
	key = 1;
	data = 100;
	assert(hash_insert(&map, &key, sizeof(key), &data, sizeof(data)) == HASHMAP_OK);
	assert(map != NULL);
	assert(hashmap_opt(map, HASHMAP_MIN_BUCKETS, 32.0f) == HASHMAP_OK);
	assert(hashmap_info(map, &info) == HASHMAP_OK);
	assert(info.nr_buckets == 32);
	assert(!info.resize_in_progress);
	assert(info.nr_entries == 1);
	assert(info.nr_entries_old == 0);
	hashmap_clear(&map);
	assert(map == NULL);
}

static void test_opt_min_buckets_shrink(void) {
	hashmap_t *map;
	hashmap_info_t info;
	int key;
	int data;
	map = hashmap_create();
	assert(map != NULL);
	for (int i = 0; i < 8; i++) {
		key = i;
		data = i * 10;
		assert(hash_insert
		       (&map, &key, sizeof(key), &data, sizeof(data)) == HASHMAP_OK);
	}
	assert(hashmap_opt(map, HASHMAP_MIN_BUCKETS, 4.0f) == HASHMAP_OK);
	assert(hashmap_info(map, &info) == HASHMAP_OK);
	assert(info.nr_buckets >= 4);
	assert(!info.resize_in_progress);
	assert(info.nr_entries_old == 0);
	assert(hashmap_getsize(map) == 8);
	size_t data_size;
	for (int i = 0; i < 8; i++) {
		key = i;
		assert(hash_search(map, &key, sizeof(key),
				   (const void **) &data, &data_size) == HASHMAP_OK);
	}
	hashmap_clear(&map);
	assert(map == NULL);
}

static void test_opt_preserves_entries(void) {
	hashmap_t *map;
	int key;
	int data;
	size_t data_size = sizeof(data);
	size_t i;
	map = hashmap_create();
	assert(map != NULL);
	for (i = 0; i < 20; i++) {
		key = (int) i;
		data = (int) i + 100;
		assert(hash_insert
		       (&map, &key, sizeof(key), (const void *)&data, data_size) == HASHMAP_OK);
	}
	assert(hashmap_opt(map, HASHMAP_GROW_THRESHOLD, 1.5f) == HASHMAP_OK);
	assert(hashmap_getsize(map) == 20);
	const void *data_ptr;
	for (i = 0; i < 20; i++) {
		key = (int) i;
		assert(hash_search(map, &key, sizeof(key),
				   &data_ptr, &data_size) == HASHMAP_OK);
		assert(*(int *) data_ptr == (int) i + 100);
	}
	hashmap_clear(&map);
	assert(map == NULL);
}

static void test_opt_finishes_regular_resize(void) {
	hashmap_t *map;
	hashmap_info_t info;
	int key;
	int data;
	size_t data_size = sizeof(data);
	size_t i;
	map = hashmap_create();
	assert(map != NULL);
	for (i = 0; i < 20; i++) {
		key = (int) i;
		data = (int) i;
		assert(hash_insert
		       (&map, &key, sizeof(key), &data, data_size) == HASHMAP_OK);
	}
	assert(hashmap_info(map, &info) == HASHMAP_OK);
	assert(info.resize_in_progress);
	assert(hashmap_opt(map, HASHMAP_GROW_THRESHOLD, 1.5f) == HASHMAP_OK);
	assert(hashmap_info(map, &info) == HASHMAP_OK);
	assert(!info.resize_in_progress);
	assert(info.nr_entries_old == 0);
	assert(info.resize_bucket == 0);
	assert(hashmap_getsize(map) == 20);
	const void *data_ptr;
	for (i = 0; i < 20; i++) {
		key = (int) i;
		assert(hash_search(map, &key, sizeof(key),
				   (const void **) &data_ptr, &data_size) == HASHMAP_OK);
		assert(*(int *) data_ptr == (int) i);
	}
	hashmap_clear(&map);
	assert(map == NULL);
}

static void test_opt_invalid_change_does_not_modify_config(void) {
	hashmap_t *map;
	hashmap_info_t info;
	map = hashmap_create();
	assert(map != NULL);
	assert(hashmap_opt(map, HASHMAP_GROW_THRESHOLD, 1.5f) == HASHMAP_OK);
	assert(hashmap_opt(map, HASHMAP_GROW_THRESHOLD, 0.5f) == HASHMAP_INVALID_ARG);
	assert(hashmap_opt(map, HASHMAP_SHRINK_THRESHOLD, 0.4f) == HASHMAP_INVALID_ARG);
	assert(hashmap_info(map, &info) == HASHMAP_OK);
	assert(info.nr_buckets == 16);
	hashmap_clear(&map);
	assert(map == NULL);
}

static void test_opt_multiple_changes(void) {
	hashmap_t *map;
	hashmap_info_t info;
	map = hashmap_create();
	assert(map != NULL);
	assert(hashmap_opt(map, HASHMAP_GROW_THRESHOLD, 1.6f) == HASHMAP_OK);
	assert(hashmap_opt(map, HASHMAP_SHRINK_THRESHOLD, 0.2f) == HASHMAP_OK);
	assert(hashmap_opt(map, HASHMAP_MIN_BUCKETS, 32.0f) == HASHMAP_OK);
	assert(hashmap_info(map, &info) == HASHMAP_OK);
	assert(info.nr_buckets == 32);
	assert(!info.resize_in_progress);
	hashmap_clear(&map);
	assert(map == NULL);
}

int main(void) {
	test_opt_null_map();
	test_opt_invalid_key();
	test_opt_default_values();
	test_opt_grow_threshold();
	test_opt_shrink_threshold();
	test_opt_min_buckets();
	test_opt_invalid_grow_threshold();
	test_opt_invalid_shrink_threshold();
	test_opt_threshold_order();
	test_opt_two_move_constraint();
	test_opt_invalid_min_buckets();
	test_opt_min_buckets_with_entries();
	test_opt_min_buckets_shrink();
	test_opt_preserves_entries();
	test_opt_finishes_regular_resize();
	test_opt_invalid_change_does_not_modify_config();
	test_opt_multiple_changes();
	printf("All tests passed.\n");
	return 0;
}
