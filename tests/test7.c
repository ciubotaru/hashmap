#include <stdio.h>
#include <assert.h>
#include <stddef.h>
#include <string.h>

#include <hashmap.h>

static void test_put_null_map_pointer(void) {
	int key = 42;
	int data = 1234;
	assert(hash_put(NULL, &key, sizeof(key), &data, sizeof(data)) == HASHMAP_INVALID_ARG);
}

static void test_put_into_null_map(void) {
	hashmap_t *map = NULL;
	int key = 42;
	int data = 1234;
	hashmap_info_t info;
	assert(hash_put(&map, &key, sizeof(key), &data, sizeof(data)) == HASHMAP_OK);
	assert(map != NULL);
	assert(hashmap_info(map, &info) == HASHMAP_OK);
	assert(info.nr_entries == 1);
	assert(hash_probe(map, &key, sizeof(key)) == HASHMAP_OK);
	hashmap_clear(&map);
	assert(map == NULL);
}

static void test_put_missing_key_inserts(void) {
	hashmap_t *map = NULL;
	int key = 42;
	int data = 1234;
	const void *found_data;
	size_t found_size;
	assert(hash_put(&map, &key, sizeof(key), &data, sizeof(data)) == HASHMAP_OK);
	assert(hash_search(map,
			   &key, sizeof(key), &found_data, &found_size) == HASHMAP_OK);
	assert(found_size == sizeof(data));
	assert(memcmp(found_data, &data, sizeof(data)) == 0);
	hashmap_clear(&map);
	assert(map == NULL);
}

static void test_put_existing_key_updates(void) {
	hashmap_t *map = NULL;
	int key = 42;
	int old_data = 1234;
	int new_data = 5678;
	const void *found_data;
	size_t found_size;
	assert(hash_insert(&map,
			   &key, sizeof(key),
			   &old_data, sizeof(old_data)) == HASHMAP_OK);
	assert(hash_put(&map,
			&key, sizeof(key), &new_data, sizeof(new_data)) == HASHMAP_OK);
	assert(hash_search(map,
			   &key, sizeof(key), &found_data, &found_size) == HASHMAP_OK);
	assert(found_size == sizeof(new_data));
	assert(memcmp(found_data, &new_data, sizeof(new_data)) == 0);
	hashmap_clear(&map);
	assert(map == NULL);
}

static void test_put_existing_key_does_not_change_entry_count(void) {
	hashmap_t *map = NULL;
	hashmap_info_t info;
	int key = 42;
	int old_data = 1234;
	int new_data = 5678;
	assert(hash_insert(&map,
			   &key, sizeof(key),
			   &old_data, sizeof(old_data)) == HASHMAP_OK);
	assert(hashmap_info(map, &info) == HASHMAP_OK);
	assert(info.nr_entries == 1);
	assert(hash_put(&map,
			&key, sizeof(key), &new_data, sizeof(new_data)) == HASHMAP_OK);
	assert(hashmap_info(map, &info) == HASHMAP_OK);
	assert(info.nr_entries == 1);
	hashmap_clear(&map);
	assert(map == NULL);
}

static void test_put_multiple_entries(void) {
	hashmap_t *map = NULL;
	int keys[] = { 1, 2, 3 };
	int data[] = { 10, 20, 30 };
	hashmap_info_t info;
	for (size_t i = 0; i < 3; i++) {
		assert(hash_put(&map,
				&keys[i], sizeof(keys[i]),
				&data[i], sizeof(data[i])) == HASHMAP_OK);
	}
	assert(hashmap_info(map, &info) == HASHMAP_OK);
	assert(info.nr_entries == 3);
	for (size_t i = 0; i < 3; i++) {
		assert(hash_probe(map, &keys[i], sizeof(keys[i])) == HASHMAP_OK);
	}
	hashmap_clear(&map);
	assert(map == NULL);
}

static void test_put_existing_and_new_keys(void) {
	hashmap_t *map = NULL;
	int key1 = 1;
	int key2 = 2;
	int data1 = 10;
	int data2 = 20;
	int new_data1 = 100;
	hashmap_info_t info;
	const void *found_data;
	size_t found_size;
	assert(hash_put(&map,
			&key1, sizeof(key1), &data1, sizeof(data1)) == HASHMAP_OK);
	assert(hash_put(&map,
			&key2, sizeof(key2), &data2, sizeof(data2)) == HASHMAP_OK);
	assert(hash_put(&map,
			&key1, sizeof(key1),
			&new_data1, sizeof(new_data1)) == HASHMAP_OK);
	assert(hashmap_info(map, &info) == HASHMAP_OK);
	assert(info.nr_entries == 2);
	assert(hash_search(map,
			   &key1, sizeof(key1),
			   &found_data, &found_size) == HASHMAP_OK);
	assert(found_size == sizeof(new_data1));
	assert(memcmp(found_data, &new_data1, sizeof(new_data1)) == 0);
	assert(hash_search(map,
			   &key2, sizeof(key2),
			   &found_data, &found_size) == HASHMAP_OK);
	assert(found_size == sizeof(data2));
	assert(memcmp(found_data, &data2, sizeof(data2)) == 0);
	hashmap_clear(&map);
	assert(map == NULL);
}

static void test_put_zero_size_key(void) {
	hashmap_t *map = NULL;
	int key = 42;
	int other_key = 1234;
	int old_data = 100;
	int new_data = 200;
	hashmap_info_t info;
	const void *found_data;
	size_t found_size;
	assert(hash_put(&map, &key, 0, &old_data, sizeof(old_data)) == HASHMAP_OK);
	assert(hash_put(&map,
			&other_key, 0, &new_data, sizeof(new_data)) == HASHMAP_OK);
	assert(hashmap_info(map, &info) == HASHMAP_OK);
	assert(info.nr_entries == 1);
	assert(hash_search(map, NULL, 0, &found_data, &found_size) == HASHMAP_OK);
	assert(found_size == sizeof(new_data));
	assert(memcmp(found_data, &new_data, sizeof(new_data)) == 0);
	hashmap_clear(&map);
	assert(map == NULL);
}

static void test_put_zero_size_data(void) {
	hashmap_t *map = NULL;
	int key = 42;
	int old_data = 1234;
	const void *found_data;
	size_t found_size;
	assert(hash_put(&map,
			&key, sizeof(key), &old_data, sizeof(old_data)) == HASHMAP_OK);
	assert(hash_put(&map, &key, sizeof(key), NULL, 0) == HASHMAP_OK);
	assert(hash_search(map,
			   &key, sizeof(key), &found_data, &found_size) == HASHMAP_OK);
	assert(found_data == NULL);
	assert(found_size == 0);
	hashmap_clear(&map);
	assert(map == NULL);
}

static void test_put_nonnull_data_with_zero_size(void) {
	hashmap_t *map = NULL;
	int key = 42;
	int old_data = 1234;
	int ignored_data = 5678;
	const void *found_data;
	size_t found_size;
	assert(hash_put(&map,
			&key, sizeof(key), &old_data, sizeof(old_data)) == HASHMAP_OK);
	assert(hash_put(&map, &key, sizeof(key), &ignored_data, 0) == HASHMAP_OK);
	assert(hash_search(map,
			   &key, sizeof(key), &found_data, &found_size) == HASHMAP_OK);
	assert(found_data == NULL);
	assert(found_size == 0);
	hashmap_clear(&map);
	assert(map == NULL);
}

static void test_put_null_key_with_nonzero_size(void) {
	hashmap_t *map = NULL;
	int data = 1234;
	assert(hash_put(&map, NULL, sizeof(int), &data, sizeof(data)) == HASHMAP_INVALID_ARG);
	assert(map == NULL);
}

static void test_put_null_data_with_nonzero_size(void) {
	hashmap_t *map = NULL;
	int key = 42;
	assert(hash_put(&map, &key, sizeof(key), NULL, sizeof(int)) == HASHMAP_INVALID_ARG);
	assert(map == NULL);
}

static void test_put_binary_key_and_data(void) {
	hashmap_t *map = NULL;
	const unsigned char key[] = {
		0x00, 0xff, 0x01, 0x00
	};
	const unsigned char old_data[] = {
		0x10, 0x20, 0x00
	};
	const unsigned char new_data[] = {
		0xff, 0x00, 0x7f, 0x80
	};
	const void *found_data;
	size_t found_size;
	assert(hash_put(&map,
			key, sizeof(key), old_data, sizeof(old_data)) == HASHMAP_OK);
	assert(hash_put(&map,
			key, sizeof(key), new_data, sizeof(new_data)) == HASHMAP_OK);
	assert(hash_search(map,
			   key, sizeof(key), &found_data, &found_size) == HASHMAP_OK);
	assert(found_size == sizeof(new_data));
	assert(memcmp(found_data, new_data, sizeof(new_data)) == 0);
	hashmap_clear(&map);
	assert(map == NULL);
}

static void test_put_copies_data_on_insert(void) {
	hashmap_t *map = NULL;
	int key = 42;
	int data = 1234;
	const void *found_data;
	size_t found_size;
	assert(hash_put(&map, &key, sizeof(key), &data, sizeof(data)) == HASHMAP_OK);
	data = 5678;
	assert(hash_search(map,
			   &key, sizeof(key), &found_data, &found_size) == HASHMAP_OK);
	assert(found_size == sizeof(int));
	assert(memcmp(found_data, &(int) { 1234 }, sizeof(int)) == 0);
	hashmap_clear(&map);
	assert(map == NULL);
}

static void test_put_copies_data_on_update(void) {
	hashmap_t *map = NULL;
	int key = 42;
	int data = 1234;
	const void *found_data;
	size_t found_size;
	assert(hash_put(&map,
			&key, sizeof(key), &(int) { 1 }, sizeof(int)) == HASHMAP_OK);
	assert(hash_put(&map, &key, sizeof(key), &data, sizeof(data)) == HASHMAP_OK);
	data = 5678;
	assert(hash_search(map,
			   &key, sizeof(key), &found_data, &found_size) == HASHMAP_OK);
	assert(found_size == sizeof(int));
	assert(memcmp(found_data, &(int) { 1234 }, sizeof(int)) == 0);
	hashmap_clear(&map);
	assert(map == NULL);
}

static void test_put_during_resize(void) {
	hashmap_t *map = NULL;
	hashmap_info_t info;
	int key;
	int data;
	for (int i = 0; i < 20; i++) {
		key = i;
		data = i * 10;
		assert(hash_put(&map,
				&key, sizeof(key), &data, sizeof(data)) == HASHMAP_OK);
	}
	assert(hashmap_info(map, &info) == HASHMAP_OK);
	assert(info.resize_in_progress == true);
	key = 1000;
	data = 10000;
	assert(hash_put(&map, &key, sizeof(key), &data, sizeof(data)) == HASHMAP_OK);
	assert(hash_probe(map, &key, sizeof(key)) == HASHMAP_OK);
	assert(hashmap_info(map, &info) == HASHMAP_OK);
	assert(info.nr_entries + info.nr_entries_old == 21);
	hashmap_clear(&map);
	assert(map == NULL);
}

static void test_put_update_during_resize(void) {
	hashmap_t *map = NULL;
	hashmap_info_t info;
	int key;
	int data;
	const void *found_data;
	size_t found_size;
	for (int i = 0; i < 20; i++) {
		key = i;
		data = i * 10;
		assert(hash_put(&map,
				&key, sizeof(key), &data, sizeof(data)) == HASHMAP_OK);
	}
	assert(hashmap_info(map, &info) == HASHMAP_OK);
	assert(info.resize_in_progress == true);
	key = 0;
	data = 9999;
	assert(hash_put(&map, &key, sizeof(key), &data, sizeof(data)) == HASHMAP_OK);
	assert(hash_search(map,
			   &key, sizeof(key), &found_data, &found_size) == HASHMAP_OK);
	assert(found_size == sizeof(data));
	assert(memcmp(found_data, &data, sizeof(data)) == 0);
	assert(hashmap_info(map, &info) == HASHMAP_OK);
	assert(info.nr_entries + info.nr_entries_old == 20);
	assert(info.resize_in_progress == true);
	hashmap_clear(&map);
	assert(map == NULL);
}

static void test_put_duplicate_during_resize(void) {
	hashmap_t *map = NULL;
	hashmap_info_t info;
	int key;
	int data;
	for (int i = 0; i < 20; i++) {
		key = i;
		data = i * 10;
		assert(hash_put(&map,
				&key, sizeof(key), &data, sizeof(data)) == HASHMAP_OK);
	}
	assert(hashmap_info(map, &info) == HASHMAP_OK);
	assert(info.resize_in_progress == true);
	key = 0;
	data = 9999;
	assert(hash_put(&map, &key, sizeof(key), &data, sizeof(data)) == HASHMAP_OK);
	assert(hashmap_info(map, &info) == HASHMAP_OK);
	assert(info.nr_entries + info.nr_entries_old == 20);
	assert(hash_search(map,
			   &key, sizeof(key),
			   (const void **) &(const void *) { 0 },
			   &(size_t) { 0 }) == HASHMAP_OK);
	hashmap_clear(&map);
	assert(map == NULL);
}

int main(void) {
	test_put_null_map_pointer();
	test_put_into_null_map();
	test_put_missing_key_inserts();
	test_put_existing_key_updates();
	test_put_existing_key_does_not_change_entry_count();
	test_put_multiple_entries();
	test_put_existing_and_new_keys();
	test_put_zero_size_key();
	test_put_zero_size_data();
	test_put_nonnull_data_with_zero_size();
	test_put_null_key_with_nonzero_size();
	test_put_null_data_with_nonzero_size();
	test_put_binary_key_and_data();
	test_put_copies_data_on_insert();
	test_put_copies_data_on_update();
	test_put_during_resize();
	test_put_update_during_resize();
	test_put_duplicate_during_resize();
	printf("All tests passed.\n");
	return 0;
}
