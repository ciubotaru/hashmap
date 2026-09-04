#include <stdlib.h>
#include <math.h>
#include "hashmap.h"
#include "hashmap-internals.h"

static const hashmap_options_t default_options = {
	.min_buckets = 16
};

static size_t hash_function(const char *s, const size_t size) {
	size_t rand1 = 31307;
	size_t rand2 = 19451;
	size_t hash = 0;
	for (size_t i = 0; i < size; i++, rand1 *= rand2)
		hash = rand1 * hash + s[i];
	return hash;
}

static int hash_compare(const void *key1,
			size_t key1_size,
			const void *key2,
			size_t key2_size) {
	if (key1_size == 0 && key2_size == 0)
		return 0;
	if (key1_size == 0)
		return -1;
	if (key2_size == 0)
		return 1;
	size_t min_size = key1_size < key2_size ? key1_size : key2_size;
	int rc = memcmp(key1, key2, min_size);
	if (rc != 0)
		return rc;
	if (key1_size < key2_size)
		return -1;
	if (key1_size > key2_size)
		return 1;
	return 0;
}

static size_t hash_bucket(size_t nr_buckets,
			  const void *key,
			  size_t key_size) {
	size_t hash = hash_function(key, key_size);
	return hash % nr_buckets;
}

static struct hashmap *hashmap_create_(size_t nr_buckets) {
	struct hashmap *newmap = malloc(sizeof(hashmap_t));
	if (!newmap)
		return NULL;
	newmap->buckets = calloc(nr_buckets, sizeof(void *));
	if (!newmap->buckets) {
		free(newmap);
		return NULL;
	}
	newmap->nr_buckets = nr_buckets;
	newmap->options = default_options;
	return newmap;
}

hashmap_t *hashmap_create(void) {
	return hashmap_create_(default_options.min_buckets);
}

static hashmap_entry_t *hash_search_(const hashmap_t *map,
			      const void *key,
			      const size_t key_size) {
	size_t bucket_nr = hash_bucket(map->nr_buckets, key, key_size);
	hashmap_entry_t *current = map->buckets[bucket_nr];
	while (current) {
		if (hash_compare(key, key_size, current->key, current->key_size) == 0)
			break;
		current = current->next;
	}
	return current;
}

int hashmap_opt(hashmap_t *map, const int key, float value) {
	if (!map)
		return -1;
	switch (key) {
	case HASHMAP_MIN_BUCKETS:
		if (!isfinite(value))
			return -1;
		if (value < 1.0f)
			return -1;
		if (value != floorf(value))
			return -1;
		if (value >= (float)SIZE_MAX)
			return -1;
		if (map->options.min_buckets == (size_t) value)
			return 0;
		map->options.min_buckets = (size_t) value;
		break;
	default:
		return -1;
	}
	return 0;
}
