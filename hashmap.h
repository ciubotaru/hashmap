#ifndef HASHMAP_H
#define HASHMAP_H

#include <stdbool.h>

#define HASHMAP_OK 0
#define HASHMAP_ERROR -1
#define HASHMAP_INVALID_ARG -2

enum option_keys {
	HASHMAP_GROW_THRESHOLD,
	HASHMAP_SHRINK_THRESHOLD,
	HASHMAP_MIN_BUCKETS
};

typedef struct {
	size_t nr_buckets;
	size_t nr_entries;
	float load_factor;
	bool resize_in_progress;
	size_t nr_buckets_old;
	size_t nr_entries_old;
	size_t resize_bucket;
} hashmap_info_t;

typedef struct hashmap hashmap_t;

hashmap_t *hashmap_create(void);

void hashmap_clear(hashmap_t **map);

int hash_probe(const hashmap_t *map,
	       const void *key,
	       size_t key_size);

int hash_search(hashmap_t *map,
		const void *key,
		size_t key_size,
		const void **data,
		size_t *data_size);

int hash_insert(hashmap_t **map,
		const void *key,
		size_t key_size,
		const void *data,
		size_t data_size);

int hash_update(hashmap_t *map,
		const void *key,
		const size_t key_size,
		const void *data,
		const size_t data_size);

int hash_put(hashmap_t **map,
	     const void *key,
	     const size_t key_size,
	     const void *data,
	     const size_t data_size);

int hash_delete(hashmap_t **map, const void *key, size_t key_size);

size_t hashmap_getsize(hashmap_t  *map);

int hashmap_info(const hashmap_t *map, hashmap_info_t *info);

int hashmap_opt(hashmap_t *map, const int option, float value);

#endif /* HASHMAP_H */
