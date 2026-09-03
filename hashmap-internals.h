#ifndef HASHMAP_INTERNALS_H
#define HASHMAP_INTERNALS_H

typedef struct hashmap_entry {
	void *key;
	size_t key_size;
	void *data;
	size_t data_size;
	struct hashmap_entry *next;
} hashmap_entry_t;

typedef struct {
	size_t min_buckets;
} hashmap_options_t;

struct hashmap {
	size_t nr_buckets;
	hashmap_entry_t **buckets;
	hashmap_options_t options;
};

#endif /* HASHMAP_INTERNALS_H */
