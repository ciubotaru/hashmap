#ifndef HASHMAP_H
#define HASHMAP_H

enum option_keys {
	HASHMAP_MIN_BUCKETS
};

typedef struct hashmap hashmap_t;

hashmap_t *hashmap_create(void);

int hashmap_opt(hashmap_t *map, const int option, float value);

#endif /* HASHMAP_H */
