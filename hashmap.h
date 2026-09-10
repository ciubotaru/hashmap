#ifndef HASHMAP_H
#define HASHMAP_H

enum option_keys {
	HASHMAP_MIN_BUCKETS
};

typedef struct hashmap hashmap_t;

hashmap_t *hashmap_create(void);

int hash_probe(const hashmap_t *map,
	       const void *key,
	       size_t key_size);

int hash_insert(hashmap_t **map,
		const void *key,
		size_t key_size,
		const void *data,
		size_t data_size);

int hash_delete(hashmap_t **map, const void *key, size_t key_size);

int hashmap_opt(hashmap_t *map, const int option, float value);

#endif /* HASHMAP_H */
