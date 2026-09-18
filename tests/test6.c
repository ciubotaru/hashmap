#include <stdio.h>
#include <assert.h>
#include <stddef.h>
#include <string.h>

#include <hashmap.h>

static void test_update_null_map(void) {
	int key = 42;
	int data = 1234;
	assert(hash_update(NULL,
			   &key, sizeof(key),
			   &data, sizeof(data)) == -1);
}

static void test_update_missing_key(void) {
	hashmap_t *map = NULL;
	int key = 42;
	int missing_key = 43;
	int data = 1234;
	assert(hash_insert(&map,
			   &key, sizeof(key),
			   &data, sizeof(data)) == 0);
	assert(hash_update(map,
			   &missing_key, sizeof(missing_key),
			   &data, sizeof(data)) == -1);
	assert(hash_probe(map, &key, sizeof(key)) == 1);
	assert(hash_probe(map, &missing_key, sizeof(missing_key)) == 0);
	hashmap_clear(&map);
	assert(map == NULL);
}

static void test_update_existing_key(void) {
	hashmap_t *map = NULL;
	int key = 42;
	int old_data = 1234;
	int new_data = 5678;
	const void *data;
	size_t data_size;
	assert(hash_insert(&map,
			   &key, sizeof(key),
			   &old_data, sizeof(old_data)) == 0);
	assert(hash_update(map,
			   &key, sizeof(key),
			   &new_data, sizeof(new_data)) == 0);
	assert(hash_search(map,
			   &key, sizeof(key),
			   &data, &data_size) == 0);
	assert(data_size == sizeof(new_data));
	assert(memcmp(data, &new_data, sizeof(new_data)) == 0);
	hashmap_clear(&map);
	assert(map == NULL);
}

static void test_update_multiple_times(void) {
	hashmap_t *map = NULL;
	int key = 42;
	int data;
	const void *found_data;
	size_t found_size;
	assert(hash_insert(&map,
			   &key, sizeof(key),
			   &(int) { 1 }, sizeof(int)) == 0);
	for (data = 2; data <= 5; data++) {
		assert(hash_update(map,
				   &key, sizeof(key),
				   &data, sizeof(data)) == 0);
		assert(hash_search(map,
				   &key, sizeof(key),
				   &found_data,
				   &found_size) == 0);
		assert(found_size == sizeof(data));
		assert(memcmp(found_data, &data, sizeof(data)) == 0);
	}
	hashmap_clear(&map);
	assert(map == NULL);
}

static void test_update_larger_data(void) {
	hashmap_t *map = NULL;
	int key = 42;
	unsigned char old_data[] = {
		0x01, 0x02
	};
	unsigned char new_data[] = {
		0x00, 0xff, 0x01, 0x02, 0x03, 0x04
	};
	const void *data;
	size_t data_size;
	assert(hash_insert(&map,
			   &key, sizeof(key),
			   old_data, sizeof(old_data)) == 0);
	assert(hash_update(map,
			   &key, sizeof(key),
			   new_data, sizeof(new_data)) == 0);
	assert(hash_search(map,
			   &key, sizeof(key),
			   &data, &data_size) == 0);
	assert(data_size == sizeof(new_data));
	assert(memcmp(data, new_data, sizeof(new_data)) == 0);
	hashmap_clear(&map);
	assert(map == NULL);
}

static void test_update_smaller_data(void) {
	hashmap_t *map = NULL;
	int key = 42;
	unsigned char old_data[] = {
		0x01, 0x02, 0x03, 0x04, 0x05, 0x06
	};
	unsigned char new_data[] = {
		0xff, 0x00
	};
	const void *data;
	size_t data_size;
	assert(hash_insert(&map,
			   &key, sizeof(key),
			   old_data, sizeof(old_data)) == 0);
	assert(hash_update(map,
			   &key, sizeof(key),
			   new_data, sizeof(new_data)) == 0);
	assert(hash_search(map,
			   &key, sizeof(key),
			   &data, &data_size) == 0);
	assert(data_size == sizeof(new_data));
	assert(memcmp(data, new_data, sizeof(new_data)) == 0);
	hashmap_clear(&map);
	assert(map == NULL);
}

static void test_update_zero_size_data(void) {
	hashmap_t *map = NULL;
	int key = 42;
	int old_data = 1234;
	const void *data;
	size_t data_size;
	assert(hash_insert(&map,
			   &key, sizeof(key),
			   &old_data, sizeof(old_data)) == 0);
	assert(hash_update(map,
			   &key, sizeof(key), NULL, 0) == 0);
	assert(hash_search(map,
			   &key, sizeof(key),
			   &data, &data_size) == 0);
	assert(data == NULL);
	assert(data_size == 0);
	hashmap_clear(&map);
	assert(map == NULL);
}

static void test_update_nonnull_data_with_zero_size(void) {
	hashmap_t *map = NULL;
	int key = 42;
	int old_data = 1234;
	int ignored_data = 5678;
	const void *data;
	size_t data_size;
	assert(hash_insert(&map,
			   &key, sizeof(key),
			   &old_data, sizeof(old_data)) == 0);

	/*
	 * The data pointer must be ignored when data_size == 0.
	 */
	assert(hash_update(map,
			   &key, sizeof(key),
			   &ignored_data, 0) == 0);
	assert(hash_search(map,
			   &key, sizeof(key),
			   &data, &data_size) == 0);
	assert(data == NULL);
	assert(data_size == 0);
	hashmap_clear(&map);
	assert(map == NULL);
}

static void test_update_null_data_with_nonzero_size(void) {
	hashmap_t *map = NULL;
	int key = 42;
	int old_data = 1234;
	const void *data;
	size_t data_size;
	assert(hash_insert(&map,
			   &key, sizeof(key),
			   &old_data, sizeof(old_data)) == 0);
	assert(hash_update(map,
			   &key, sizeof(key),
			   NULL, sizeof(old_data)) == -1);

	/*
	 * The failed update must leave the old data unchanged.
	 */
	assert(hash_search(map,
			   &key, sizeof(key),
			   &data, &data_size) == 0);
	assert(data_size == sizeof(old_data));
	assert(memcmp(data, &old_data, sizeof(old_data)) == 0);
	hashmap_clear(&map);
	assert(map == NULL);
}

static void test_update_null_key_with_nonzero_size(void) {
	hashmap_t *map = NULL;
	int key = 42;
	int data = 1234;
	const void *found_data;
	size_t found_size;
	assert(hash_insert(&map,
			   &key, sizeof(key),
			   &data, sizeof(data)) == 0);
	assert(hash_update(map,
			   NULL, sizeof(key),
			   &data, sizeof(data)) == -1);

	/*
	 * The failed update must not affect the existing entry.
	 */
	assert(hash_search(map,
			   &key, sizeof(key),
			   &found_data, &found_size) == 0);
	assert(found_size == sizeof(data));
	assert(memcmp(found_data, &data, sizeof(data)) == 0);
	hashmap_clear(&map);
	assert(map == NULL);
}

static void test_update_zero_size_key(void) {
	hashmap_t *map = NULL;
	int key = 42;
	int other_key = 1234;
	int old_data = 100;
	int new_data = 200;
	const void *data;
	size_t data_size;
	assert(hash_insert(&map,
			   &key, 0,
			   &old_data, sizeof(old_data)) == 0);

	/*
	 * The key pointer is ignored when key_size == 0.
	 */
	assert(hash_update(map,
			   &other_key, 0,
			   &new_data, sizeof(new_data)) == 0);
	assert(hash_search(map,
			   NULL, 0, &data, &data_size) == 0);
	assert(data_size == sizeof(new_data));
	assert(memcmp(data, &new_data, sizeof(new_data)) == 0);
	hashmap_clear(&map);
	assert(map == NULL);
}

static void test_update_binary_data(void) {
	hashmap_t *map = NULL;
	const unsigned char key[] = {
		0x00, 0xff, 0x01
	};
	const unsigned char old_data[] = {
		0x10, 0x20, 0x00
	};
	const unsigned char new_data[] = {
		0xff, 0x00, 0x7f, 0x80
	};
	const void *data;
	size_t data_size;
	assert(hash_insert(&map,
			   key, sizeof(key),
			   old_data, sizeof(old_data)) == 0);
	assert(hash_update(map,
			   key, sizeof(key),
			   new_data, sizeof(new_data)) == 0);
	assert(hash_search(map,
			   key, sizeof(key),
			   &data, &data_size) == 0);
	assert(data_size == sizeof(new_data));
	assert(memcmp(data, new_data, sizeof(new_data)) == 0);
	hashmap_clear(&map);
	assert(map == NULL);
}

static void test_update_copies_data(void) {
	hashmap_t *map = NULL;
	int key = 42;
	int data = 1234;
	const void *found_data;
	size_t found_size;
	assert(hash_insert(&map,
			   &key, sizeof(key),
			   &(int) { 1 }, sizeof(int)) == 0);
	assert(hash_update(map,
			   &key, sizeof(key),
			   &data, sizeof(data)) == 0);

	/*
	 * Modify the caller's data after the update.
	 */
	data = 5678;
	assert(hash_search(map,
			   &key, sizeof(key),
			   &found_data, &found_size) == 0);
	assert(found_size == sizeof(int));
	assert(memcmp(found_data, &(int) { 1234 }, sizeof(int)) == 0);
	hashmap_clear(&map);
	assert(map == NULL);
}

static void test_update_does_not_change_entry_count(void) {
	hashmap_t *map = NULL;
	hashmap_info_t info;
	int keys[] = { 1, 2, 3 };
	int data = 100;
	for (size_t i = 0; i < 3; i++) {
		assert(hash_insert(&map,
				   &keys[i], sizeof(keys[i]),
				   &data, sizeof(data)) == 0);
	}
	assert(hashmap_info(map, &info) == 0);
	assert(info.nr_entries == 3);
	assert(hash_update(map,
			   &keys[1], sizeof(keys[1]),
			   &(int) { 999 }, sizeof(int)) == 0);
	assert(hashmap_info(map, &info) == 0);
	assert(info.nr_entries == 3);
	hashmap_clear(&map);
	assert(map == NULL);
}

static void test_update_does_not_start_resize(void) {
	hashmap_t *map = NULL;
	hashmap_info_t before;
	hashmap_info_t after;
	int key;
	int data;

	/*
	 * Keep the map below the growth threshold.
	 */
	for (int i = 0; i < 10; i++) {
		key = i;
		data = i * 10;
		assert(hash_insert(&map,
				   &key, sizeof(key),
				   &data, sizeof(data)) == 0);
	}
	assert(hashmap_info(map, &before) == 0);
	assert(before.resize_in_progress == false);
	key = 5;
	data = 9999;
	assert(hash_update(map,
			   &key, sizeof(key),
			   &data, sizeof(data)) == 0);
	assert(hashmap_info(map, &after) == 0);
	assert(after.nr_entries == before.nr_entries);
	assert(after.nr_buckets == before.nr_buckets);
	assert(after.resize_in_progress == false);
	hashmap_clear(&map);
	assert(map == NULL);
}

static void test_update_during_resize(void) {
	hashmap_t *map = NULL;
	hashmap_info_t info;
	int key;
	int old_data;
	int new_data;
	const void *data;
	size_t data_size;

	/*
	 * Start a resize.
	 */
	for (int i = 0; i < 20; i++) {
		key = i;
		old_data = i * 10;
		assert(hash_insert(&map,
				   &key, sizeof(key),
				   &old_data,
				   sizeof(old_data)) == 0);
	}
	assert(hashmap_info(map, &info) == 0);
	assert(info.resize_in_progress == true);

	/*
	 * Updating an existing entry must work regardless of which
	 * table currently contains it.
	 */
	key = 0;
	new_data = 9999;
	assert(hash_update(map,
			   &key, sizeof(key),
			   &new_data, sizeof(new_data)) == 0);
	assert(hash_search(map,
			   &key, sizeof(key),
			   &data, &data_size) == 0);
	assert(data_size == sizeof(new_data));
	assert(memcmp(data, &new_data, sizeof(new_data)) == 0);
	assert(hashmap_info(map, &info) == 0);
	assert(info.resize_in_progress == true);
	assert(info.nr_entries + info.nr_entries_old == 20);
	hashmap_clear(&map);
	assert(map == NULL);
}

int main(void) {
	test_update_null_map();
	test_update_missing_key();
	test_update_existing_key();
	test_update_multiple_times();
	test_update_larger_data();
	test_update_smaller_data();
	test_update_zero_size_data();
	test_update_nonnull_data_with_zero_size();
	test_update_null_data_with_nonzero_size();
	test_update_null_key_with_nonzero_size();
	test_update_zero_size_key();
	test_update_binary_data();
	test_update_copies_data();
	test_update_does_not_change_entry_count();
	test_update_does_not_start_resize();
	test_update_during_resize();
	printf("All tests passed.\n");
	return 0;
}
