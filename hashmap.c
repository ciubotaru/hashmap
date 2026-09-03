#include <stdlib.h>
#include <math.h>
#include "hashmap.h"
#include "hashmap-internals.h"

static const hashmap_options_t default_options = {
	.min_buckets = 16
};

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
