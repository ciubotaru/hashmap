#include <stdio.h>
#include <assert.h>
#include <stddef.h>
#include <string.h>

#include <hashmap.h>

static void test_search_null_map(void) {
	int key = 42;
	const void *data;
	size_t data_size;
	assert(hash_search(NULL,
			   &key, sizeof(key),
			   &data, &data_size) == HASHMAP_ERROR);
}

static void test_search_empty_map(void) {
	hashmap_t *map;
	const void *data;
	size_t data_size;
	map = hashmap_create();
	assert(map != NULL);
	assert(hash_search(map,
			   "key", 3, &data, &data_size) == HASHMAP_ERROR);
	hashmap_clear(&map);
	assert(map == NULL);
}

static void test_search_existing_key(void) {
	hashmap_t *map = NULL;
	int key = 42;
	int value = 1234;
	const void *data;
	size_t data_size;
	assert(hash_insert(&map,
			   &key, sizeof(key),
			   &value, sizeof(value)) == HASHMAP_OK);
	assert(hash_search(map,
			   &key, sizeof(key),
			   &data, &data_size) == HASHMAP_OK);
	assert(data != NULL);
	assert(data_size == sizeof(value));
	assert(memcmp(data, &value, sizeof(value)) == 0);
	hashmap_clear(&map);
	assert(map == NULL);
}

static void test_search_missing_key(void) {
	hashmap_t *map = NULL;
	int key = 42;
	int missing_key = 43;
	int value = 1234;
	const void *data;
	size_t data_size;
	assert(hash_insert(&map,
			   &key, sizeof(key),
			   &value, sizeof(value)) == HASHMAP_OK);
	assert(hash_search(map,
			   &missing_key, sizeof(missing_key),
			   &data, &data_size) == HASHMAP_ERROR);
	hashmap_clear(&map);
	assert(map == NULL);
}

static void test_search_null_data_output(void) {
	hashmap_t *map = NULL;
	int key = 42;
	int value = 1234;
	size_t data_size;
	assert(hash_insert(&map,
			   &key, sizeof(key),
			   &value, sizeof(value)) == HASHMAP_OK);
	assert(hash_search(map,
			   &key, sizeof(key),
			   NULL, &data_size) == HASHMAP_INVALID_ARG);
	hashmap_clear(&map);
	assert(map == NULL);
}

static void test_search_null_size_output(void) {
	hashmap_t *map = NULL;
	int key = 42;
	int value = 1234;
	const void *data;
	assert(hash_insert(&map,
			   &key, sizeof(key),
			   &value, sizeof(value)) == HASHMAP_OK);
	assert(hash_search(map,
			   &key, sizeof(key),
			   &data, NULL) == HASHMAP_INVALID_ARG);
	hashmap_clear(&map);
	assert(map == NULL);
}

static void test_search_null_outputs(void) {
	hashmap_t *map = NULL;
	int key = 42;
	int value = 1234;
	assert(hash_insert(&map,
			   &key, sizeof(key),
			   &value, sizeof(value)) == HASHMAP_OK);
	assert(hash_search(map,
			   &key, sizeof(key),
			   NULL, NULL) == HASHMAP_INVALID_ARG);
	hashmap_clear(&map);
	assert(map == NULL);
}

static void test_search_null_key_with_nonzero_size(void) {
	hashmap_t *map = NULL;
	int existing_key = 42;
	int value = 1234;
	const void *data;
	size_t data_size;
	assert(hash_insert(&map,
			   &existing_key, sizeof(existing_key),
			   &value, sizeof(value)) == HASHMAP_OK);
	assert(hash_search(map,
			   NULL, sizeof(existing_key),
			   &data, &data_size) == HASHMAP_INVALID_ARG);

	/*
	 * The invalid search must not affect the map.
	 */
	assert(hash_search(map,
			   &existing_key, sizeof(existing_key),
			   &data, &data_size) == HASHMAP_OK);
	hashmap_clear(&map);
	assert(map == NULL);
}

static void test_search_zero_size_key(void) {
	hashmap_t *map = NULL;
	int key = 42;
	int other_key = 1234;
	int value = 5678;
	const void *data;
	size_t data_size;
	assert(hash_insert(&map,
			   &key, 0,
			   &value, sizeof(value)) == HASHMAP_OK);

	/*
	 * The key pointer must be ignored when key_size == 0.
	 */
	assert(hash_search(map,
			   &other_key, 0,
			   &data, &data_size) == HASHMAP_OK);
	assert(data != NULL);
	assert(data_size == sizeof(value));
	assert(memcmp(data, &value, sizeof(value)) == 0);
	hashmap_clear(&map);
	assert(map == NULL);
}

static void test_search_zero_size_data(void) {
	hashmap_t *map = NULL;
	int key = 42;
	const void *data;
	size_t data_size;
	assert(hash_insert(&map,
			   &key, sizeof(key), NULL, 0) == HASHMAP_OK);
	assert(hash_search(map,
			   &key, sizeof(key),
			   &data, &data_size) == HASHMAP_OK);

	/*
	 * NULL data is valid when data_size == 0.
	 */
	assert(data == NULL);
	assert(data_size == 0);
	hashmap_clear(&map);
	assert(map == NULL);
}

static void test_search_binary_data(void) {
	hashmap_t *map = NULL;
	const unsigned char key[] = {
		0x00, 0xff, 0x01
	};
	const unsigned char value[] = {
		0x00, 0x7f, 0xff, 0x00
	};
	const void *data;
	size_t data_size;
	assert(hash_insert(&map,
			   key, sizeof(key),
			   value, sizeof(value)) == HASHMAP_OK);
	assert(hash_search(map,
			   key, sizeof(key),
			   &data, &data_size) == HASHMAP_OK);
	assert(data != NULL);
	assert(data_size == sizeof(value));
	assert(memcmp(data, value, sizeof(value)) == 0);
	hashmap_clear(&map);
	assert(map == NULL);
}

static void test_search_data_is_copied(void) {
	hashmap_t *map = NULL;
	int key = 42;
	int value = 1234;
	const void *data;
	size_t data_size;
	assert(hash_insert(&map,
			   &key, sizeof(key),
			   &value, sizeof(value)) == HASHMAP_OK);

	/*
	 * Modify the original object after insertion.
	 */
	value = 5678;
	assert(hash_search(map,
			   &key, sizeof(key),
			   &data, &data_size) == HASHMAP_OK);
	assert(data_size == sizeof(value));
	assert(memcmp(data, &(int) { 1234 }, sizeof(int)) == 0);
	hashmap_clear(&map);
	assert(map == NULL);
}

static void test_search_returns_borrowed_data(void) {
	hashmap_t *map = NULL;
	int key = 42;
	int value = 1234;
	const void *data;
	size_t data_size;
	assert(hash_insert(&map,
			   &key, sizeof(key),
			   &value, sizeof(value)) == HASHMAP_OK);
	assert(hash_search(map,
			   &key, sizeof(key),
			   &data, &data_size) == HASHMAP_OK);

	/*
	 * The returned pointer refers to hashmap-owned storage.
	 * It must not be freed by the caller.
	 */
	assert(data != NULL);
	assert(data_size == sizeof(value));
	hashmap_clear(&map);
	assert(map == NULL);
}

static void test_search_during_resize(void) {
	hashmap_t *map = NULL;
	hashmap_info_t info;
	int key;
	int value;
	const void *data;
	size_t data_size;

	/*
	 * Start a resize.
	 */
	for (int i = 0; i < 20; i++) {
		key = i;
		value = i * 10;
		assert(hash_insert(&map,
				   &key, sizeof(key),
				   &value, sizeof(value)) == HASHMAP_OK);
	}
	assert(hashmap_info(map, &info) == HASHMAP_OK);
	assert(info.resize_in_progress == true);

	/*
	 * Every entry must remain searchable while migration
	 * is taking place.
	 */
	for (int i = 0; i < 20; i++) {
		key = i;
		assert(hash_search(map,
				   &key, sizeof(key),
				   &data, &data_size) == HASHMAP_OK);
		assert(data != NULL);
		assert(data_size == sizeof(value));
		value = i * 10;
		assert(memcmp(data, &value, sizeof(value)) == 0);
	}
	key = 1000;
	assert(hash_search(map,
			   &key, sizeof(key),
			   &data, &data_size) == HASHMAP_ERROR);
	hashmap_clear(&map);
	assert(map == NULL);
}

static void test_search_key_in_new_table(void) {
	hashmap_t *map = NULL;
	hashmap_info_t info;
	int key;
	int value;
	const void *data;
	size_t data_size;

	/*
	 * Start a resize.
	 */
	for (int i = 0; i < 20; i++) {
		key = i;
		value = i * 10;
		assert(hash_insert(&map,
				   &key, sizeof(key),
				   &value, sizeof(value)) == HASHMAP_OK);
	}
	assert(hashmap_info(map, &info) == HASHMAP_OK);
	assert(info.resize_in_progress == true);

	/*
	 * New entries go into the current/new table.
	 */
	key = 1000;
	value = 10000;
	assert(hash_insert(&map,
			   &key, sizeof(key),
			   &value, sizeof(value)) == HASHMAP_OK);
	assert(hash_search(map,
			   &key, sizeof(key),
			   &data, &data_size) == HASHMAP_OK);
	assert(data_size == sizeof(value));
	assert(memcmp(data, &value, sizeof(value)) == 0);
	hashmap_clear(&map);
	assert(map == NULL);
}

static void test_search_key_in_old_table(void) {
	hashmap_t *map = NULL;
	hashmap_info_t info;
	int key;
	int value;
	const void *data;
	size_t data_size;

	/*
	 * Start a resize.
	 */
	for (int i = 0; i < 20; i++) {
		key = i;
		value = i * 10;
		assert(hash_insert(&map,
				   &key, sizeof(key),
				   &value, sizeof(value)) == HASHMAP_OK);
	}
	assert(hashmap_info(map, &info) == HASHMAP_OK);
	assert(info.resize_in_progress == true);
	assert(info.nr_entries_old > 0);

	/*
	 * We cannot identify a particular old-table entry through
	 * the public API, but all entries must remain searchable
	 * while old entries still exist.
	 */
	for (int i = 0; i < 20; i++) {
		key = i;
		value = i * 10;
		assert(hash_search(map,
				   &key, sizeof(key),
				   &data, &data_size) == HASHMAP_OK);
		assert(data_size == sizeof(value));
		assert(memcmp(data, &value, sizeof(value)) == 0);
	}
	hashmap_clear(&map);
	assert(map == NULL);
}

int main(void) {
	test_search_null_map();
	test_search_empty_map();
	test_search_existing_key();
	test_search_missing_key();
	test_search_null_data_output();
	test_search_null_size_output();
	test_search_null_outputs();
	test_search_null_key_with_nonzero_size();
	test_search_zero_size_key();
	test_search_zero_size_data();
	test_search_binary_data();
	test_search_data_is_copied();
	test_search_returns_borrowed_data();
	test_search_during_resize();
	test_search_key_in_new_table();
	test_search_key_in_old_table();
	printf("All tests passed\n");
	return 0;
}
