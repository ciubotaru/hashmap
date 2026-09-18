#include <stdio.h>
#include <assert.h>
#include <stddef.h>

#include <hashmap.h>

static void test_probe_null_map(void) {
	int key = 42;
	assert(hash_probe(NULL, &key, sizeof(key)) == -1);
}

static void test_probe_empty_map(void) {
	hashmap_t *map;
	map = hashmap_create();
	assert(map != NULL);
	assert(hash_probe(map, "key", 3) == 0);
	hashmap_clear(&map);
	assert(map == NULL);
}

static void test_probe_existing_key(void) {
	hashmap_t *map = NULL;
	int key = 42;
	int data = 1234;
	assert(hash_insert(&map, &key, sizeof(key),
			   &data, sizeof(data)) == 0);
	assert(hash_probe(map, &key, sizeof(key)) == 1);
	hashmap_clear(&map);
	assert(map == NULL);
}

static void test_probe_missing_key(void) {
	hashmap_t *map = NULL;
	int key1 = 42;
	int key2 = 43;
	int data = 1234;
	assert(hash_insert(&map, &key1, sizeof(key1),
			   &data, sizeof(data)) == 0);
	assert(hash_probe(map, &key2, sizeof(key2)) == 0);
	hashmap_clear(&map);
	assert(map == NULL);
}

static void test_probe_zero_size_key(void) {
	hashmap_t *map = NULL;
	int data = 1234;

	/*
	 * A zero-size key is valid even when its pointer is NULL.
	 */
	assert(hash_insert(&map, NULL, 0, &data, sizeof(data)) == 0);
	assert(hash_probe(map, NULL, 0) == 1);
	hashmap_clear(&map);
	assert(map == NULL);
}

static void test_probe_binary_key(void) {
	hashmap_t *map = NULL;
	const unsigned char key[] = {
		0x00, 0xff, 0x01, 0x00, 0x7f
	};
	const unsigned char missing_key[] = {
		0x00, 0xff, 0x01, 0x00, 0x7e
	};
	int data = 1234;
	assert(hash_insert(&map, key, sizeof(key), &data, sizeof(data)) == 0);
	assert(hash_probe(map, key, sizeof(key)) == 1);
	assert(hash_probe(map, missing_key, sizeof(missing_key)) == 0);
	hashmap_clear(&map);
	assert(map == NULL);
}

static void test_probe_null_key_with_nonzero_size(void) {
	hashmap_t *map = NULL;
	assert(hash_probe(map, NULL, 1) == -1);

	/*
	 * The map should not have been created or modified by probe.
	 */
	assert(map == NULL);
}

static void test_probe_null_key_with_nonzero_size_nonempty_map(void) {
	hashmap_t *map = NULL;
	int key = 42;
	int data = 1234;
	assert(hash_insert(&map, &key, sizeof(key),
			   &data, sizeof(data)) == 0);
	assert(hash_probe(map, NULL, sizeof(key)) == -1);

	/*
	 * The invalid probe must not affect the existing map.
	 */
	assert(hash_probe(map, &key, sizeof(key)) == 1);
	hashmap_clear(&map);
	assert(map == NULL);
}

static void test_probe_nonnull_key_with_zero_size_nonempty_map(void) {
	hashmap_t *map = NULL;
	int key = 42;
	int other_key = 1234;
	int data = 5678;

	/*
	 * The key pointer is non-NULL, but its size is zero.
	 * Therefore the key itself is an empty byte sequence.
	 */
	assert(hash_insert(&map, &key, 0,
			   &data, sizeof(data)) == 0);

	/*
	 * The actual pointer value must be ignored when key_size == 0.
	 */
	assert(hash_probe(map, &other_key, 0) == 1);
	hashmap_clear(&map);
	assert(map == NULL);
}

static void test_probe_during_resize(void) {
	hashmap_t *map = NULL;
	hashmap_info_t info;
	int key;
	int data;

	/*
	 * Insert enough entries to start a resize.
	 */
	for (int i = 0; i < 20; i++) {
		key = i;
		data = i * 10;
		assert(hash_insert(&map, &key, sizeof(key),
				   &data, sizeof(data)) == 0);
	}
	assert(hashmap_info(map, &info) == 0);
	assert(info.resize_in_progress == true);

	/*
	 * At this point entries can exist in both the current
	 * and old tables.
	 */
	for (int i = 0; i < 20; i++) {
		key = i;
		assert(hash_probe(map, &key, sizeof(key)) == 1);
	}
	key = 1000;
	assert(hash_probe(map, &key, sizeof(key)) == 0);
	hashmap_clear(&map);
	assert(map == NULL);
}

static void test_probe_key_in_old_table(void) {
	hashmap_t *map = NULL;
	hashmap_info_t info;
	int key;
	int data;

	/*
	 * Start a resize.
	 */
	for (int i = 0; i < 20; i++) {
		key = i;
		data = i * 10;
		assert(hash_insert(&map, &key, sizeof(key),
				   &data, sizeof(data)) == 0);
	}

	assert(hashmap_info(map, &info) == 0);
	assert(info.resize_in_progress == true);
	assert(info.nr_entries_old > 0);

	/*
	 * The first entries have not necessarily remained in the old
	 * table, so use the public state to wait until there is still
	 * an old table and probe all known keys. At least one must be
	 * found regardless of which table contains it.
	 */
	for (int i = 0; i < 20; i++) {
		key = i;
		assert(hash_probe(map, &key, sizeof(key)) == 1);
	}
	hashmap_clear(&map);
	assert(map == NULL);
}

static void test_probe_key_in_new_table(void) {
	hashmap_t *map = NULL;
	hashmap_info_t info;
	int key;
	int data;

	/*
	 * Start a resize.
	 */
	for (int i = 0; i < 20; i++) {
		key = i;
		data = i * 10;
		assert(hash_insert(&map, &key, sizeof(key),
				   &data, sizeof(data)) == 0);
	}
	assert(hashmap_info(map, &info) == 0);
	assert(info.resize_in_progress == true);

	/*
	 * Insert another key while resizing. New entries must go
	 * into the current/new table.
	 */
	key = 1000;
	data = 10000;
	assert(hash_insert(&map, &key, sizeof(key),
			   &data, sizeof(data)) == 0);
	assert(hash_probe(map, &key, sizeof(key)) == 1);
	hashmap_clear(&map);
	assert(map == NULL);
}

int main(void) {
	test_probe_null_map();
	test_probe_empty_map();
	test_probe_existing_key();
	test_probe_missing_key();
	test_probe_zero_size_key();
	test_probe_null_key_with_nonzero_size_nonempty_map();
	test_probe_nonnull_key_with_zero_size_nonempty_map();
	test_probe_binary_key();
	test_probe_null_key_with_nonzero_size();
	test_probe_during_resize();
	test_probe_key_in_old_table();
	test_probe_key_in_new_table();
	printf("All tests passed.\n");
	return 0;
}
