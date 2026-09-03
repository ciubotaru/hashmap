#include <stdlib.h>
#include "hashmap.h"
#include "hashmap-internals.h"

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
	return newmap;
}

hashmap_t *hashmap_create(void) {
	return hashmap_create_(HASHMAP_DEFAULT_BUCKETS);
}
