#include <stdio.h>
#include <assert.h>
#include <stddef.h>
#include <string.h>

#include <hashmap.h>

static void test_insert_into_null_map(void) {
	hashmap_t *map = NULL;
	int key = 42;
	int data = 1234;
	hashmap_info_t info;
	assert(hash_insert(&map,
			   &key, sizeof(key),
			   &data, sizeof(data)) == HASHMAP_OK);
	assert(map != NULL);
	assert(hashmap_info(map, &info) == HASHMAP_OK);
	assert(info.nr_entries == 1);
	hashmap_clear(&map);
	assert(map == NULL);
}

static void test_insert_single_entry(void) {
	hashmap_t *map = NULL;
	int key = 42;
	int data = 1234;
	const void *found_data;
	size_t found_size;
	assert(hash_insert(&map,
			   &key, sizeof(key),
			   &data, sizeof(data)) == HASHMAP_OK);
	assert(hash_search(map,
			   &key, sizeof(key),
			   &found_data, &found_size) == HASHMAP_OK);
	assert(found_size == sizeof(data));
	assert(memcmp(found_data, &data, sizeof(data)) == 0);
	hashmap_clear(&map);
	assert(map == NULL);
}

static void test_insert_multiple_entries(void) {
	hashmap_t *map = NULL;
	int keys[] = { 1, 2, 3 };
	int data[] = { 10, 20, 30 };
	hashmap_info_t info;
	for (size_t i = 0; i < 3; i++) {
		assert(hash_insert(&map,
				   &keys[i], sizeof(keys[i]),
				   &data[i],
				   sizeof(data[i])) == HASHMAP_OK);
	}
	assert(hashmap_info(map, &info) == HASHMAP_OK);
	assert(info.nr_entries == 3);
	for (size_t i = 0; i < 3; i++)
		assert(hash_probe(map, &keys[i], sizeof(keys[i])) == HASHMAP_OK);
	hashmap_clear(&map);
	assert(map == NULL);
}

static void test_insert_duplicate_key(void) {
	hashmap_t *map = NULL;
	int key = 42;
	int first_data = 1234;
	int second_data = 5678;
	hashmap_info_t info;
	const void *found_data;
	size_t found_size;
	assert(hash_insert(&map,
			   &key, sizeof(key),
			   &first_data,
			   sizeof(first_data)) == HASHMAP_OK);
	assert(hash_insert(&map,
			   &key, sizeof(key),
			   &second_data, sizeof(second_data)) == HASHMAP_ERROR);
	assert(hashmap_info(map, &info) == HASHMAP_OK);
	assert(info.nr_entries == 1);
	assert(hash_search(map,
			   &key, sizeof(key),
			   &found_data, &found_size) == HASHMAP_OK);
	assert(found_size == sizeof(first_data));
	assert(memcmp(found_data, &first_data, sizeof(first_data)) == 0);
	hashmap_clear(&map);
	assert(map == NULL);
}

static void test_insert_zero_size_key(void) {
	hashmap_t *map = NULL;
	int key = 42;
	int other_key = 1234;
	int data = 5678;

	/*
	 * key_size == 0 means the key pointer itself is ignored.
	 */
	assert(hash_insert(&map,
			   &key, 0, &data, sizeof(data)) == HASHMAP_OK);
	assert(hash_probe(map, NULL, 0) == HASHMAP_OK);
	assert(hash_probe(map, &other_key, 0) == HASHMAP_OK);
	hashmap_clear(&map);
	assert(map == NULL);
}

static void test_insert_zero_size_data(void) {
	hashmap_t *map = NULL;
	int key = 42;
	hashmap_info_t info;
	const void *data;
	size_t data_size;
	assert(hash_insert(&map,
			   &key, sizeof(key), NULL, 0) == HASHMAP_OK);
	assert(hashmap_info(map, &info) == HASHMAP_OK);
	assert(info.nr_entries == 1);
	assert(hash_search(map,
			   &key, sizeof(key),
			   &data, &data_size) == HASHMAP_OK);
	assert(data == NULL);
	assert(data_size == 0);
	hashmap_clear(&map);
	assert(map == NULL);
}

static void test_insert_nonnull_key_with_zero_size(void) {
	hashmap_t *map = NULL;
	int key = 42;
	int other_key = 1234;
	int data = 5678;

	/*
	 * A non-NULL key pointer must be ignored when key_size == 0.
	 */
	assert(hash_insert(&map,
			   &key, 0, &data, sizeof(data)) == HASHMAP_OK);

	/*
	 * Therefore another pointer with size zero refers to
	 * exactly the same key.
	 */
	assert(hash_insert(&map,
			   &other_key, 0,
			   &data, sizeof(data)) == HASHMAP_ERROR);
	hashmap_clear(&map);
	assert(map == NULL);
}

static void test_insert_null_key_with_nonzero_size(void) {
	hashmap_t *map = NULL;
	int data = 1234;
	assert(hash_insert(&map,
			   NULL, sizeof(int),
			   &data, sizeof(data)) == HASHMAP_INVALID_ARG);

	/*
	 * The invalid insertion must not create a hashmap.
	 */
	assert(map == NULL);
}

static void test_insert_null_data_with_nonzero_size(void) {
	hashmap_t *map = NULL;
	int key = 42;
	assert(hash_insert(&map,
			   &key, sizeof(key),
			   NULL, sizeof(int)) == HASHMAP_INVALID_ARG);

	/*
	 * The invalid insertion must not create a hashmap.
	 */
	assert(map == NULL);
}

static void test_insert_null_map_pointer(void) {
	int key = 42;
	int data = 1234;

	/*
	 * The hashmap_t ** argument itself is NULL.
	 * This must not be dereferenced.
	 */
	assert(hash_insert(NULL,
			   &key, sizeof(key),
			   &data, sizeof(data)) == HASHMAP_INVALID_ARG);
}

static void test_insert_binary_key_and_data(void) {
	hashmap_t *map = NULL;
	const unsigned char key[] = {
		0x00, 0xff, 0x01, 0x00
	};
	const unsigned char data[] = {
		0xff, 0x00, 0x7f, 0x80, 0x00
	};
	const void *found_data;
	size_t found_size;
	assert(hash_insert(&map,
			   key, sizeof(key),
			   data, sizeof(data)) == HASHMAP_OK);
	assert(hash_search(map,
			   key, sizeof(key),
			   &found_data, &found_size) == HASHMAP_OK);
	assert(found_size == sizeof(data));
	assert(memcmp(found_data, data, sizeof(data)) == 0);
	hashmap_clear(&map);
	assert(map == NULL);
}

static void test_insert_copies_key(void) {
	hashmap_t *map = NULL;
	unsigned char key[] = {
		0x01, 0x02, 0x03, 0x04
	};
	unsigned char original_key[] = {
		0x01, 0x02, 0x03, 0x04
	};
	int data = 1234;
	assert(hash_insert(&map,
			   key, sizeof(key),
			   &data, sizeof(data)) == HASHMAP_OK);

	/*
	 * Modify the caller's key after insertion.
	 */
	key[0] = 0xff;

	/*
	 * The hashmap owns its own copy, so the original key must
	 * still be searchable.
	 */
	assert(hash_probe(map, original_key, sizeof(original_key)) == HASHMAP_OK);
	assert(hash_probe(map, key, sizeof(key)) == HASHMAP_ERROR);
	hashmap_clear(&map);
	assert(map == NULL);
}

static void test_insert_copies_data(void) {
	hashmap_t *map = NULL;
	int key = 42;
	int data = 1234;
	const void *found_data;
	size_t found_size;
	assert(hash_insert(&map,
			   &key, sizeof(key),
			   &data, sizeof(data)) == HASHMAP_OK);

	/*
	 * Modify the caller's data after insertion.
	 */
	data = 5678;
	assert(hash_search(map,
			   &key, sizeof(key),
			   &found_data, &found_size) == HASHMAP_OK);
	assert(found_size == sizeof(int));
	assert(memcmp(found_data, &(int) { 1234 }, sizeof(int)) == 0);
	hashmap_clear(&map);
	assert(map == NULL);
}

static void test_insert_starts_resize(void) {
	hashmap_t *map = NULL;
	hashmap_info_t info;
	int key;
	int data;

	/*
	 * The default table has 16 buckets and the growth threshold
	 * is 1.2. Therefore 20 entries cross the threshold.
	 */
	for (int i = 0; i < 20; i++) {
		key = i;
		data = i * 10;
		assert(hash_insert(&map,
				   &key, sizeof(key),
				   &data, sizeof(data)) == HASHMAP_OK);
	}
	assert(hashmap_info(map, &info) == HASHMAP_OK);
	assert(info.nr_entries + info.nr_entries_old == 20);
	assert(info.resize_in_progress == true);
	assert(info.nr_buckets_old == 16);
	assert(info.nr_entries_old > 0);
	hashmap_clear(&map);
	assert(map == NULL);
}

static void test_insert_during_resize(void) {
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
		assert(hash_insert(&map,
				   &key, sizeof(key),
				   &data, sizeof(data)) == HASHMAP_OK);
	}
	assert(hashmap_info(map, &info) == HASHMAP_OK);
	assert(info.resize_in_progress == true);

	/*
	 * A new entry must be inserted while the resize is active.
	 */
	key = 1000;
	data = 10000;
	assert(hash_insert(&map,
			   &key, sizeof(key),
			   &data, sizeof(data)) == HASHMAP_OK);
	assert(hashmap_info(map, &info) == HASHMAP_OK);
	assert(info.nr_entries + info.nr_entries_old == 21);
	assert(hash_probe(map, &key, sizeof(key)) == HASHMAP_OK);
	hashmap_clear(&map);
	assert(map == NULL);
}

static void test_insert_duplicate_during_resize(void) {
	hashmap_t *map = NULL;
	hashmap_info_t info;
	int key;
	int data;
	const void *found_data;
	size_t found_size;

	/*
	 * Start a resize.
	 */
	for (int i = 0; i < 20; i++) {
		key = i;
		data = i * 10;
		assert(hash_insert(&map,
				   &key, sizeof(key),
				   &data, sizeof(data)) == HASHMAP_OK);
	}
	assert(hashmap_info(map, &info) == HASHMAP_OK);
	assert(info.resize_in_progress == true);

	/*
	 * Key 0 may still be in the old table. Regardless of which
	 * table contains it, insertion must detect the duplicate.
	 */
	key = 0;
	data = 9999;
	assert(hash_insert(&map,
			   &key, sizeof(key),
			   &data, sizeof(data)) == HASHMAP_ERROR);
	assert(hash_search(map,
			   &key, sizeof(key),
			   &found_data, &found_size) == HASHMAP_OK);
	assert(found_size == sizeof(int));
	assert(memcmp(found_data, &(int) { 0 }, sizeof(int)) == 0);
	hashmap_clear(&map);
	assert(map == NULL);
}

static void test_insert_does_not_change_existing_entries(void) {
	hashmap_t *map = NULL;
	int key1 = 1;
	int key2 = 2;
	int data1 = 10;
	int data2 = 20;
	const void *found_data;
	size_t found_size;
	assert(hash_insert(&map,
			   &key1, sizeof(key1),
			   &data1, sizeof(data1)) == HASHMAP_OK);
	assert(hash_insert(&map,
			   &key2, sizeof(key2),
			   &data2, sizeof(data2)) == HASHMAP_OK);
	assert(hash_search(map,
			   &key1, sizeof(key1),
			   &found_data, &found_size) == HASHMAP_OK);
	assert(found_size == sizeof(data1));
	assert(memcmp(found_data, &data1, sizeof(data1)) == 0);
	assert(hash_search(map,
			   &key2, sizeof(key2),
			   &found_data, &found_size) == HASHMAP_OK);
	assert(found_size == sizeof(data2));
	assert(memcmp(found_data, &data2, sizeof(data2)) == 0);
	hashmap_clear(&map);
	assert(map == NULL);
}

int main(void) {
	test_insert_into_null_map();
	test_insert_single_entry();
	test_insert_multiple_entries();
	test_insert_duplicate_key();
	test_insert_zero_size_key();
	test_insert_zero_size_data();
	test_insert_nonnull_key_with_zero_size();
	test_insert_null_key_with_nonzero_size();
	test_insert_null_data_with_nonzero_size();
	test_insert_null_map_pointer();
	test_insert_binary_key_and_data();
	test_insert_copies_key();
	test_insert_copies_data();
	test_insert_starts_resize();
	test_insert_during_resize();
	test_insert_duplicate_during_resize();
	test_insert_does_not_change_existing_entries();
	printf("All tests passed.\n");
	return 0;
}
