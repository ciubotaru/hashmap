#include <stdio.h>
#include <assert.h>
#include <stddef.h>
#include <string.h>

#include <hashmap.h>

static void test_delete_null_map_pointer(void) {
	int key = 42;
	assert(hash_delete(NULL, &key, sizeof(key)) == HASHMAP_INVALID_ARG);
}

static void test_delete_null_map(void) {
	hashmap_t *map = NULL;
	int key = 42;
	assert(hash_delete(&map, &key, sizeof(key)) == HASHMAP_ERROR);
	assert(map == NULL);
}

static void test_delete_from_empty_map(void) {
	hashmap_t *map = NULL;
	int key = 42;
	map = hashmap_create();
	assert(map != NULL);
	assert(hash_delete(&map, &key, sizeof(key)) == HASHMAP_ERROR);
	assert(map != NULL);
	hashmap_clear(&map);
	assert(map == NULL);
}

static void test_delete_missing_key(void) {
	hashmap_t *map = NULL;
	int key = 42;
	int missing_key = 43;
	int data = 1234;
	hashmap_info_t info;
	assert(hash_insert(&map,
			   &key, sizeof(key), &data, sizeof(data)) == HASHMAP_OK);
	assert(hash_delete(&map, &missing_key, sizeof(missing_key)) == HASHMAP_ERROR);
	assert(map != NULL);
	assert(hashmap_info(map, &info) == HASHMAP_OK);
	assert(info.nr_entries + info.nr_entries_old == 1);
	assert(hash_probe(map, &key, sizeof(key)) == HASHMAP_OK);
	hashmap_clear(&map);
	assert(map == NULL);
}

static void test_delete_existing_key(void) {
	hashmap_t *map = NULL;
	int key = 42;
	int data = 1234;
	assert(hash_insert(&map,
			   &key, sizeof(key), &data, sizeof(data)) == HASHMAP_OK);
	assert(hash_delete(&map, &key, sizeof(key)) == HASHMAP_OK);
	assert(hashmap_getsize(map) == 0);
	assert(map == NULL);
}

static void test_delete_one_of_multiple_entries(void) {
	hashmap_t *map = NULL;
	int keys[] = { 1, 2, 3 };
	int data[] = { 10, 20, 30 };
	hashmap_info_t info;
	for (size_t i = 0; i < 3; i++) {
		assert(hash_insert(&map,
				   &keys[i], sizeof(keys[i]),
				   &data[i], sizeof(data[i])) == HASHMAP_OK);
	}
	assert(hashmap_info(map, &info) == HASHMAP_OK);
	assert(info.nr_entries + info.nr_entries_old == 3);
	assert(hash_delete(&map, &keys[1], sizeof(keys[1])) == HASHMAP_OK);
	assert(map != NULL);
	assert(hashmap_info(map, &info) == HASHMAP_OK);
	assert(info.nr_entries + info.nr_entries_old == 2);
	assert(hash_probe(map, &keys[0], sizeof(keys[0])) == HASHMAP_OK);
	assert(hash_probe(map, &keys[1], sizeof(keys[1])) == HASHMAP_ERROR);
	assert(hash_probe(map, &keys[2], sizeof(keys[2])) == HASHMAP_OK);
	hashmap_clear(&map);
	assert(map == NULL);
}

static void test_delete_all_entries(void) {
	hashmap_t *map = NULL;
	int keys[] = { 1, 2, 3 };
	int data[] = { 10, 20, 30 };
	for (size_t i = 0; i < 3; i++) {
		assert(hash_insert(&map,
				   &keys[i], sizeof(keys[i]),
				   &data[i], sizeof(data[i])) == HASHMAP_OK);
	}
	for (size_t i = 0; i < 3; i++) {
		assert(hash_delete(&map, &keys[i], sizeof(keys[i])) == HASHMAP_OK);
	}
	assert(map == NULL);
}

static void test_delete_null_key_with_zero_size(void) {
	hashmap_t *map = NULL;
	int data = 1234;
	assert(hash_insert(&map, NULL, 0, &data, sizeof(data)) == HASHMAP_OK);
	assert(hash_delete(&map, NULL, 0) == HASHMAP_OK);
	assert(map == NULL);
}

static void test_delete_nonnull_key_with_zero_size(void) {
	hashmap_t *map = NULL;
	int key = 42;
	int other_key = 1234;
	int data = 5678;
	assert(hash_insert(&map, &key, 0, &data, sizeof(data)) == HASHMAP_OK);
	assert(hash_delete(&map, &other_key, 0) == HASHMAP_OK);
	assert(map == NULL);
}

static void test_delete_null_key_with_nonzero_size(void) {
	hashmap_t *map = NULL;
	int key = 42;
	int data = 1234;
	assert(hash_insert(&map,
			   &key, sizeof(key), &data, sizeof(data)) == HASHMAP_OK);
	assert(hash_delete(&map, NULL, sizeof(key)) == HASHMAP_INVALID_ARG);
	assert(map != NULL);
	assert(hash_probe(map, &key, sizeof(key)) == HASHMAP_OK);
	hashmap_clear(&map);
	assert(map == NULL);
}

static void test_delete_binary_key(void) {
	hashmap_t *map = NULL;
	const unsigned char key[] = {
		0x00, 0xff, 0x01, 0x00, 0x7f
	};
	const unsigned char missing_key[] = {
		0x00, 0xff, 0x01, 0x00, 0x7e
	};
	int data = 1234;
	assert(hash_insert(&map, key, sizeof(key), &data, sizeof(data)) == HASHMAP_OK);
	assert(hash_delete(&map, missing_key, sizeof(missing_key)) == HASHMAP_ERROR);
	assert(hash_probe(map, key, sizeof(key)) == HASHMAP_OK);
	assert(hash_delete(&map, key, sizeof(key)) == HASHMAP_OK);
	assert(map == NULL);
}

static void test_delete_during_resize(void) {
	hashmap_t *map = NULL;
	hashmap_info_t info;
	int key;
	int data;
	for (int i = 0; i < 20; i++) {
		key = i;
		data = i * 10;
		assert(hash_insert(&map,
				   &key, sizeof(key),
				   &data, sizeof(data)) == HASHMAP_OK);
	}
	assert(hashmap_info(map, &info) == HASHMAP_OK);
	assert(info.resize_in_progress == true);
	assert(info.nr_entries + info.nr_entries_old == 20);
	key = 0;
	assert(hash_delete(&map, &key, sizeof(key)) == HASHMAP_OK);
	assert(map != NULL);
	assert(hashmap_info(map, &info) == HASHMAP_OK);
	assert(info.nr_entries + info.nr_entries_old == 19);
	assert(hash_probe(map, &key, sizeof(key)) == HASHMAP_ERROR);
	for (int i = 1; i < 20; i++) {
		key = i;
		assert(hash_probe(map, &key, sizeof(key)) == HASHMAP_OK);
	}
	hashmap_clear(&map);
	assert(map == NULL);
}

static void test_delete_key_from_old_table(void) {
	hashmap_t *map = NULL;
	hashmap_info_t info;
	int key;
	int data;
	for (int i = 0; i < 20; i++) {
		key = i;
		data = i * 10;
		assert(hash_insert(&map,
				   &key, sizeof(key),
				   &data, sizeof(data)) == HASHMAP_OK);
	}
	assert(hashmap_info(map, &info) == HASHMAP_OK);
	assert(info.resize_in_progress == true);
	assert(info.nr_entries_old > 0);
	key = 0;
	assert(hash_delete(&map, &key, sizeof(key)) == HASHMAP_OK);
	assert(map != NULL);
	assert(hashmap_info(map, &info) == HASHMAP_OK);
	assert(info.nr_entries + info.nr_entries_old == 19);
	assert(hash_probe(map, &key, sizeof(key)) == HASHMAP_ERROR);
	hashmap_clear(&map);
	assert(map == NULL);
}

static void test_delete_key_from_new_table(void) {
	hashmap_t *map = NULL;
	hashmap_info_t info;
	int key;
	int data;
	for (int i = 0; i < 20; i++) {
		key = i;
		data = i * 10;
		assert(hash_insert(&map,
				   &key, sizeof(key),
				   &data, sizeof(data)) == HASHMAP_OK);
	}
	assert(hashmap_info(map, &info) == HASHMAP_OK);
	assert(info.resize_in_progress == true);
	key = 1000;
	data = 10000;
	assert(hash_insert(&map,
			   &key, sizeof(key), &data, sizeof(data)) == HASHMAP_OK);
	assert(hash_probe(map, &key, sizeof(key)) == HASHMAP_OK);
	assert(hash_delete(&map, &key, sizeof(key)) == HASHMAP_OK);
	assert(map != NULL);
	assert(hash_probe(map, &key, sizeof(key)) == HASHMAP_ERROR);
	assert(hashmap_info(map, &info) == HASHMAP_OK);
	assert(info.nr_entries + info.nr_entries_old == 20);
	hashmap_clear(&map);
	assert(map == NULL);
}

static void test_delete_missing_key_during_resize(void) {
	hashmap_t *map = NULL;
	hashmap_info_t info;
	int key;
	int data;
	int missing_key = 1000;
	for (int i = 0; i < 20; i++) {
		key = i;
		data = i * 10;
		assert(hash_insert(&map,
				   &key, sizeof(key),
				   &data, sizeof(data)) == HASHMAP_OK);
	}
	assert(hashmap_info(map, &info) == HASHMAP_OK);
	assert(info.resize_in_progress == true);
	assert(hash_delete(&map, &missing_key, sizeof(missing_key)) == HASHMAP_ERROR);
	assert(map != NULL);
	assert(hashmap_info(map, &info) == HASHMAP_OK);
	assert(info.nr_entries + info.nr_entries_old == 20);
	for (int i = 0; i < 20; i++) {
		key = i;
		assert(hash_probe(map, &key, sizeof(key)) == HASHMAP_OK);
	}
	hashmap_clear(&map);
	assert(map == NULL);
}

static void test_delete_multiple_during_resize(void) {
	hashmap_t *map = NULL;
	hashmap_info_t info;
	int key;
	int data;
	for (int i = 0; i < 20; i++) {
		key = i;
		data = i * 10;
		assert(hash_insert(&map,
				   &key, sizeof(key),
				   &data, sizeof(data)) == HASHMAP_OK);
	}
	assert(hashmap_info(map, &info) == HASHMAP_OK);
	assert(info.resize_in_progress == true);
	assert(info.nr_entries + info.nr_entries_old == 20);
	for (int i = 0; i < 10; i++) {
		key = i;
		assert(hash_delete(&map, &key, sizeof(key)) == HASHMAP_OK);
	}
	assert(map != NULL);
	assert(hashmap_info(map, &info) == HASHMAP_OK);
	assert(info.nr_entries + info.nr_entries_old == 10);
	for (int i = 0; i < 10; i++) {
		key = i;
		assert(hash_probe(map, &key, sizeof(key)) == HASHMAP_ERROR);
	}
	for (int i = 10; i < 20; i++) {
		key = i;
		assert(hash_probe(map, &key, sizeof(key)) == HASHMAP_OK);
	}
	hashmap_clear(&map);
	assert(map == NULL);
}

static void test_delete_then_insert(void) {
	hashmap_t *map = NULL;
	hashmap_info_t info;
	int key1 = 1;
	int key2 = 2;
	int data1 = 10;
	int data2 = 20;
	assert(hash_insert(&map,
			   &key1, sizeof(key1), &data1, sizeof(data1)) == HASHMAP_OK);
	assert(hash_delete(&map, &key1, sizeof(key1)) == HASHMAP_OK);
	assert(map == NULL);
	assert(hash_insert(&map,
			   &key2, sizeof(key2), &data2, sizeof(data2)) == HASHMAP_OK);
	assert(map != NULL);
	assert(hashmap_info(map, &info) == HASHMAP_OK);
	assert(info.nr_entries + info.nr_entries_old == 1);
	assert(hash_probe(map, &key2, sizeof(key2)) == HASHMAP_OK);
	hashmap_clear(&map);
	assert(map == NULL);
}

static void test_delete_and_reinsert_same_key(void) {
	hashmap_t *map = NULL;
	int key = 42;
	int old_data = 1234;
	int new_data = 5678;
	const void *found_data;
	size_t found_size;
	assert(hash_insert(&map,
			   &key, sizeof(key),
			   &old_data, sizeof(old_data)) == HASHMAP_OK);
	assert(hash_delete(&map, &key, sizeof(key)) == HASHMAP_OK);
	assert(map == NULL);
	assert(hash_insert(&map,
			   &key, sizeof(key),
			   &new_data, sizeof(new_data)) == HASHMAP_OK);
	assert(hash_search(map,
			   &key, sizeof(key), &found_data, &found_size) == HASHMAP_OK);
	assert(found_size == sizeof(new_data));
	assert(memcmp(found_data, &new_data, sizeof(new_data)) == 0);
	hashmap_clear(&map);
	assert(map == NULL);
}

int main(void) {
	test_delete_null_map_pointer();
	test_delete_null_map();
	test_delete_from_empty_map();
	test_delete_missing_key();
	test_delete_existing_key();
	test_delete_one_of_multiple_entries();
	test_delete_all_entries();
	test_delete_null_key_with_zero_size();
	test_delete_nonnull_key_with_zero_size();
	test_delete_null_key_with_nonzero_size();
	test_delete_binary_key();
	test_delete_during_resize();
	test_delete_key_from_old_table();
	test_delete_key_from_new_table();
	test_delete_missing_key_during_resize();
	test_delete_multiple_during_resize();
	test_delete_then_insert();
	test_delete_and_reinsert_same_key();
	printf("All tests passed.\n");
	return 0;
}
