#ifndef HASHMAP_INTERNALS_H
#define HASHMAP_INTERNALS_H

#include <stdbool.h>

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

#define HASHMAP_RESIZE_MOVES 2

struct hashmap {
	size_t nr_buckets;
	size_t nr_entries;
	hashmap_entry_t **buckets;
	bool resize_in_progress;
	size_t nr_buckets_old;
	size_t nr_entries_old;
	hashmap_entry_t **buckets_old;
	size_t resize_bucket;
	hashmap_options_t options;
};

#endif /* HASHMAP_INTERNALS_H */
