#ifndef HASHMAP_INTERNALS_H
#define HASHMAP_INTERNALS_H

#define HASHMAP_DEFAULT_BUCKETS 16

struct hashmap {
	size_t nr_buckets;
	void **buckets;
};

#endif /* HASHMAP_INTERNALS_H */
