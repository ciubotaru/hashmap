#include <stdio.h>
#include <assert.h>
#include <stddef.h>

#include <hashmap.h>

static void test_getsize_null_map(void) {
	assert(hashmap_getsize(NULL) == 0);
}

static void test_getsize_empty_map(void) {
	hashmap_t *map;
	map = hashmap_create();
	assert(map != NULL);
	assert(hashmap_getsize(map) == 0);
	hashmap_clear(&map);
	assert(map == NULL);
}

static void test_getsize_one_entry(void) {
	hashmap_t *map = NULL;
	int key = 42;
	int data = 1234;
	assert(hash_insert(&map,
			   &key, sizeof(key), &data, sizeof(data)) == 0);
	assert(hashmap_getsize(map) == 1);
	hashmap_clear(&map);
	assert(map == NULL);
}

static void test_getsize_multiple_entries(void) {
	hashmap_t *map = NULL;
	int keys[] = { 1, 2, 3, 4, 5 };
	int data[] = { 10, 20, 30, 40, 50 };
	assert(hashmap_getsize(map) == 0);
	for (size_t i = 0; i < 5; i++) {
		assert(hash_insert(&map,
				   &keys[i], sizeof(keys[i]),
				   &data[i], sizeof(data[i])) == 0);
		assert(hashmap_getsize(map) == i + 1);
	}
	hashmap_clear(&map);
	assert(map == NULL);
}

static void test_getsize_after_delete(void) {
	hashmap_t *map = NULL;
	int keys[] = { 1, 2, 3 };
	int data[] = { 10, 20, 30 };
	assert(hash_insert(&map,
			   &keys[0], sizeof(keys[0]),
			   &data[0], sizeof(data[0])) == 0);
	assert(hash_insert(&map,
			   &keys[1], sizeof(keys[1]),
			   &data[1], sizeof(data[1])) == 0);
	assert(hash_insert(&map,
			   &keys[2], sizeof(keys[2]),
			   &data[2], sizeof(data[2])) == 0);
	assert(hashmap_getsize(map) == 3);
	assert(hash_delete(&map, &keys[1], sizeof(keys[1])) == 0);
	assert(hashmap_getsize(map) == 2);
	assert(hash_delete(&map, &keys[0], sizeof(keys[0])) == 0);
	assert(hashmap_getsize(map) == 1);
	assert(hash_delete(&map, &keys[2], sizeof(keys[2])) == 0);
	assert(map == NULL);
	assert(hashmap_getsize(map) == 0);
}

static void test_getsize_duplicate_insert(void) {
	hashmap_t *map = NULL;
	int key = 42;
	int data1 = 1234;
	int data2 = 5678;
	assert(hash_insert(&map,
			   &key, sizeof(key), &data1, sizeof(data1)) == 0);
	assert(hashmap_getsize(map) == 1);
	assert(hash_insert(&map,
			   &key, sizeof(key), &data2, sizeof(data2)) == -1);
	assert(hashmap_getsize(map) == 1);
	hashmap_clear(&map);
	assert(map == NULL);
}

static void test_getsize_update(void) {
	hashmap_t *map = NULL;
	int key = 42;
	int old_data = 1234;
	int new_data = 5678;
	assert(hash_insert(&map,
			   &key, sizeof(key),
			   &old_data, sizeof(old_data)) == 0);
	assert(hashmap_getsize(map) == 1);
	assert(hash_update(map,
			   &key, sizeof(key),
			   &new_data, sizeof(new_data)) == 0);
	assert(hashmap_getsize(map) == 1);
	hashmap_clear(&map);
	assert(map == NULL);
}

static void test_getsize_put(void) {
	hashmap_t *map = NULL;
	int key1 = 1;
	int key2 = 2;
	int data1 = 10;
	int data2 = 20;
	assert(hash_put(&map,
			&key1, sizeof(key1), &data1, sizeof(data1)) == 0);
	assert(hashmap_getsize(map) == 1);
	assert(hash_put(&map,
			&key1, sizeof(key1), &data2, sizeof(data2)) == 0);
	assert(hashmap_getsize(map) == 1);
	assert(hash_put(&map,
			&key2, sizeof(key2), &data2, sizeof(data2)) == 0);
	assert(hashmap_getsize(map) == 2);
	hashmap_clear(&map);
	assert(map == NULL);
}

static void test_getsize_during_resize(void) {
	hashmap_t *map = NULL;
	hashmap_info_t info;
	int key;
	int data;
	for (int i = 0; i < 20; i++) {
		key = i;
		data = i * 10;
		assert(hash_insert(&map,
				   &key, sizeof(key),
				   &data, sizeof(data)) == 0);
	}
	assert(hashmap_info(map, &info) == 0);
	assert(info.resize_in_progress == true);
	assert(hashmap_getsize(map) == 20);
	assert(info.nr_entries + info.nr_entries_old == 20);
	hashmap_clear(&map);
	assert(map == NULL);
}

static void test_getsize_during_resize_after_insert(void) {
	hashmap_t *map = NULL;
	hashmap_info_t info;
	int key;
	int data;
	for (int i = 0; i < 20; i++) {
		key = i;
		data = i * 10;
		assert(hash_insert(&map,
				   &key, sizeof(key),
				   &data, sizeof(data)) == 0);
	}
	assert(hashmap_info(map, &info) == 0);
	assert(info.resize_in_progress == true);
	key = 1000;
	data = 10000;
	assert(hash_insert(&map,
			   &key, sizeof(key), &data, sizeof(data)) == 0);
	assert(hashmap_getsize(map) == 21);
	assert(hashmap_info(map, &info) == 0);
	assert(info.nr_entries + info.nr_entries_old == 21);
	hashmap_clear(&map);
	assert(map == NULL);
}

static void test_getsize_during_resize_after_delete(void) {
	hashmap_t *map = NULL;
	hashmap_info_t info;
	int key;
	int data;
	for (int i = 0; i < 20; i++) {
		key = i;
		data = i * 10;
		assert(hash_insert(&map,
				   &key, sizeof(key),
				   &data, sizeof(data)) == 0);
	}
	assert(hashmap_info(map, &info) == 0);
	assert(info.resize_in_progress == true);
	key = 0;
	assert(hash_delete(&map, &key, sizeof(key)) == 0);
	assert(hashmap_getsize(map) == 19);
	assert(hashmap_info(map, &info) == 0);
	assert(info.nr_entries + info.nr_entries_old == 19);
	hashmap_clear(&map);
	assert(map == NULL);
}

static void test_getsize_zero_size_key_and_data(void) {
	hashmap_t *map = NULL;
	int key = 42;
	assert(hash_insert(&map, &key, 0, NULL, 0) == 0);
	assert(hashmap_getsize(map) == 1);
	assert(hash_delete(&map, &key, 0) == 0);
	assert(map == NULL);
	assert(hashmap_getsize(map) == 0);
}

int main(void) {
	test_getsize_null_map();
	test_getsize_empty_map();
	test_getsize_one_entry();
	test_getsize_multiple_entries();
	test_getsize_after_delete();
	test_getsize_duplicate_insert();
	test_getsize_update();
	test_getsize_put();
	test_getsize_during_resize();
	test_getsize_during_resize_after_insert();
	test_getsize_during_resize_after_delete();
	test_getsize_zero_size_key_and_data();
	printf("All tests passed.\n");
	return 0;
}
