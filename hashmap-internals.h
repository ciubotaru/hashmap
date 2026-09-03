#ifndef HASHMAP_INTERNALS_H
#define HASHMAP_INTERNALS_H

typedef struct {
	size_t min_buckets;
} hashmap_options_t;

struct hashmap {
	size_t nr_buckets;
	void **buckets;
	hashmap_options_t options;
};

#endif /* HASHMAP_INTERNALS_H */
