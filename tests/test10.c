#include <stdio.h>
#include <assert.h>
#include <stddef.h>

#include <hashmap.h>

static void test_info_null_map(void) {
	hashmap_info_t info;
	assert(hashmap_info(NULL, &info) == HASHMAP_INVALID_ARG);
}

static void test_info_null_info(void) {
	hashmap_t *map;
	map = hashmap_create();
	assert(map != NULL);
	assert(hashmap_info(map, NULL) == HASHMAP_INVALID_ARG);
	hashmap_clear(&map);
	assert(map == NULL);
}

static void test_info_null_map_and_info(void) {
	assert(hashmap_info(NULL, NULL) == HASHMAP_INVALID_ARG);
}

static void test_info_empty_map(void) {
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

static void test_info_one_entry(void) {
	hashmap_t *map = NULL;
	hashmap_info_t info;
	int key = 42;
	int data = 1234;
	assert(hash_insert(&map,
			   &key, sizeof(key), &data, sizeof(data)) == HASHMAP_OK);
	assert(hashmap_info(map, &info) == HASHMAP_OK);
	assert(info.nr_buckets == 16);
	assert(info.nr_entries == 1);
	assert(info.load_factor == 1.0f / 16.0f);
	assert(info.resize_in_progress == false);
	assert(info.nr_buckets_old == 0);
	assert(info.nr_entries_old == 0);
	assert(info.resize_bucket == 0);
	hashmap_clear(&map);
	assert(map == NULL);
}

static void test_info_multiple_entries(void) {
	hashmap_t *map = NULL;
	hashmap_info_t info;
	int keys[] = { 1, 2, 3, 4 };
	int data[] = { 10, 20, 30, 40 };
	for (size_t i = 0; i < 4; i++) {
		assert(hash_insert(&map,
				   &keys[i], sizeof(keys[i]),
				   &data[i], sizeof(data[i])) == HASHMAP_OK);
	}
	assert(hashmap_info(map, &info) == HASHMAP_OK);
	assert(info.nr_buckets == 16);
	assert(info.nr_entries == 4);
	assert(info.load_factor == 4.0f / 16.0f);
	assert(info.resize_in_progress == false);
	assert(info.nr_buckets_old == 0);
	assert(info.nr_entries_old == 0);
	assert(info.resize_bucket == 0);
	hashmap_clear(&map);
	assert(map == NULL);
}

static void test_info_after_update(void) {
	hashmap_t *map = NULL;
	hashmap_info_t info;
	int key = 42;
	int old_data = 1234;
	int new_data = 5678;
	assert(hash_insert(&map,
			   &key, sizeof(key),
			   &old_data, sizeof(old_data)) == HASHMAP_OK);
	assert(hash_update(map,
			   &key, sizeof(key),
			   &new_data, sizeof(new_data)) == HASHMAP_OK);
	assert(hashmap_info(map, &info) == HASHMAP_OK);
	assert(info.nr_entries == 1);
	assert(info.load_factor == 1.0f / 16.0f);
	assert(info.resize_in_progress == false);
	assert(info.nr_entries_old == 0);
	hashmap_clear(&map);
	assert(map == NULL);
}

static void test_info_after_delete(void) {
	hashmap_t *map = NULL;
	hashmap_info_t info;
	int keys[] = { 1, 2, 3 };
	int data[] = { 10, 20, 30 };
	for (size_t i = 0; i < 3; i++) {
		assert(hash_insert(&map,
				   &keys[i], sizeof(keys[i]),
				   &data[i], sizeof(data[i])) == HASHMAP_OK);
	}
	assert(hash_delete(&map, &keys[1], sizeof(keys[1])) == HASHMAP_OK);
	assert(map != NULL);
	assert(hashmap_info(map, &info) == HASHMAP_OK);
	assert(info.nr_entries + info.nr_entries_old == 2);
	assert(info.load_factor == (float) info.nr_entries /
	       (float) info.nr_buckets);
	hashmap_clear(&map);
	assert(map == NULL);
}

static void test_info_after_put(void) {
	hashmap_t *map = NULL;
	hashmap_info_t info;
	int key1 = 1;
	int key2 = 2;
	int data1 = 10;
	int data2 = 20;
	assert(hash_put(&map,
			&key1, sizeof(key1), &data1, sizeof(data1)) == HASHMAP_OK);
	assert(hashmap_info(map, &info) == HASHMAP_OK);
	assert(info.nr_entries + info.nr_entries_old == 1);
	assert(hash_put(&map,
			&key1, sizeof(key1), &data2, sizeof(data2)) == HASHMAP_OK);
	assert(hashmap_info(map, &info) == HASHMAP_OK);
	assert(info.nr_entries + info.nr_entries_old == 1);
	assert(hash_put(&map,
			&key2, sizeof(key2), &data2, sizeof(data2)) == HASHMAP_OK);
	assert(hashmap_info(map, &info) == HASHMAP_OK);
	assert(info.nr_entries + info.nr_entries_old == 2);
	hashmap_clear(&map);
	assert(map == NULL);
}

static void test_info_during_resize(void) {
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
	assert(info.nr_buckets == 32);
	assert(info.nr_buckets_old == 16);
	assert(info.nr_entries + info.nr_entries_old == 20);
	assert(info.nr_entries_old > 0);
	assert(info.resize_bucket < info.nr_buckets_old);
	assert(info.load_factor == (float) info.nr_entries /
	       (float) info.nr_buckets);
	hashmap_clear(&map);
	assert(map == NULL);
}

static void test_info_during_resize_after_insert(void) {
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
	assert(hashmap_info(map, &info) == HASHMAP_OK);
	assert(info.nr_entries + info.nr_entries_old == 21);
	assert(info.nr_buckets == 32);
	assert(info.nr_buckets_old == 16);
	assert(info.load_factor == (float) info.nr_entries /
	       (float) info.nr_buckets);
	hashmap_clear(&map);
	assert(map == NULL);
}

static void test_info_during_resize_after_delete(void) {
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
	key = 0;
	assert(hash_delete(&map, &key, sizeof(key)) == HASHMAP_OK);
	assert(hashmap_info(map, &info) == HASHMAP_OK);
	assert(info.nr_entries + info.nr_entries_old == 19);
	assert(info.nr_buckets == 32);
	assert(info.nr_buckets_old == 16);
	assert(info.load_factor == (float) info.nr_entries /
	       (float) info.nr_buckets);
	hashmap_clear(&map);
	assert(map == NULL);
}

static void test_info_resize_cursor_changes(void) {
	hashmap_t *map = NULL;
	hashmap_info_t info_before;
	hashmap_info_t info_after;
	int key;
	int data;
	for (int i = 0; i < 20; i++) {
		key = i;
		data = i * 10;
		assert(hash_insert(&map,
				   &key, sizeof(key),
				   &data, sizeof(data)) == HASHMAP_OK);
	}
	assert(hashmap_info(map, &info_before) == HASHMAP_OK);
	assert(info_before.resize_in_progress == true);
	assert(info_before.nr_entries + info_before.nr_entries_old == 20);
	key = 1000;
	data = 10000;
	assert(hash_insert(&map,
			   &key, sizeof(key), &data, sizeof(data)) == HASHMAP_OK);
	assert(hashmap_info(map, &info_after) == HASHMAP_OK);
	assert(info_after.resize_in_progress == true);
	assert(info_after.nr_entries + info_after.nr_entries_old == 21);
	assert(info_after.resize_bucket >= info_before.resize_bucket);
	assert(info_after.nr_entries_old <= info_before.nr_entries_old);
	hashmap_clear(&map);
	assert(map == NULL);
}

static void test_info_after_resize_completion(void) {
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
	while (map != NULL) {
		assert(hashmap_info(map, &info) == HASHMAP_OK);
		if (!info.resize_in_progress)
			break;
		key = 1000 + (int) info.nr_entries_old;
		data = key * 10;
		assert(hash_insert(&map,
				   &key, sizeof(key),
				   &data, sizeof(data)) == HASHMAP_OK);
	}
	assert(map != NULL);
	assert(hashmap_info(map, &info) == HASHMAP_OK);
	assert(info.resize_in_progress == false);
	assert(info.nr_buckets_old == 0);
	assert(info.nr_entries_old == 0);
	assert(info.resize_bucket == 0);
	assert(info.nr_entries > 0);
	assert(info.load_factor == (float) info.nr_entries /
	       (float) info.nr_buckets);
	hashmap_clear(&map);
	assert(map == NULL);
}

static void test_info_does_not_modify_map(void) {
	hashmap_t *map = NULL;
	hashmap_info_t info1;
	hashmap_info_t info2;
	int key = 42;
	int data = 1234;
	assert(hash_insert(&map,
			   &key, sizeof(key), &data, sizeof(data)) == HASHMAP_OK);
	assert(hashmap_info(map, &info1) == HASHMAP_OK);
	assert(hashmap_info(map, &info2) == HASHMAP_OK);
	assert(info1.nr_buckets == info2.nr_buckets);
	assert(info1.nr_entries == info2.nr_entries);
	assert(info1.load_factor == info2.load_factor);
	assert(info1.resize_in_progress == info2.resize_in_progress);
	assert(info1.nr_buckets_old == info2.nr_buckets_old);
	assert(info1.nr_entries_old == info2.nr_entries_old);
	assert(info1.resize_bucket == info2.resize_bucket);
	hashmap_clear(&map);
	assert(map == NULL);
}

int main(void) {
	test_info_null_map();
	test_info_null_info();
	test_info_null_map_and_info();
	test_info_empty_map();
	test_info_one_entry();
	test_info_multiple_entries();
	test_info_after_update();
	test_info_after_delete();
	test_info_after_put();
	test_info_during_resize();
	test_info_during_resize_after_insert();
	test_info_during_resize_after_delete();
	test_info_resize_cursor_changes();
	test_info_after_resize_completion();
	test_info_does_not_modify_map();
	printf("All tests passed.\n");
	return 0;
}
