#include <stdlib.h>
#include <math.h>
#include "hashmap.h"
#include "hashmap-internals.h"

static const hashmap_options_t default_options = {
	.grow_threshold = 1.2,
	.shrink_threshold = 0.3,
	.resize_shift = 1,
	.min_buckets = 16
};

static size_t hash_function(const void *key, size_t key_size) {
	size_t rand1 = 31307;
	size_t rand2 = 19451;
	size_t hash = 0;
	const char *s = key;
	for (size_t i = 0; i < key_size; i++, rand1 *= rand2)
		hash = rand1 * hash + s[i];
	return hash;
/*
	int i;
	int len = strlen(s);
	unsigned char ptr[sizeof(size_t)] = {0};
	for (i = 0; i < len; i++) {
		ptr[i % sizeof(size_t)] *= 37;
		ptr[i % sizeof(size_t)] += (unsigned char)s[i];
	}
	return *(size_t *) ptr;
*/
/*
	unsigned const char *us = (unsigned const char *) s;
	size_t hash = 0;
	while (*us) {
		hash = hash * 37 + *us;
		us++;
	}
	return hash;
*/
/*
	size_t hash = 0;
	int i = 0;
	int len = strlen(s);
	for (i = 0; i < len; i++) {
		hash <<= 8;
		hash += s[i];
	}
	return hash;
*/
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
			  size_t key_size,
			  size_t (*hash_function) (const void *key, size_t key_size)) {
	size_t hash = hash_function(key, key_size);
	return hash & (nr_buckets - 1);
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
	newmap->nr_entries = 0;
	newmap->nr_buckets_old = 0;
	newmap->nr_entries_old = 0;
	newmap->buckets_old = NULL;
	newmap->resize_bucket = 0;
	newmap->options = default_options;
	return newmap;
}

hashmap_t *hashmap_create(void) {
	return hashmap_create_(default_options.min_buckets);
}

static void hashmap_free_(hashmap_t **map) {
	if ((*map)->buckets)
		free((*map)->buckets);
	if ((*map)->buckets_old)
		free((*map)->buckets_old);
	free(*map);
	*map = NULL;
}

static void hash_free_(hashmap_entry_t *entry) {
	if (entry->key)
		free(entry->key);
	if (entry->data)
		free(entry->data);
	free(entry);
}

void hashmap_clear(hashmap_t **map) {
	if (!map || !*map)
		return;
	hashmap_entry_t **current;
	hashmap_entry_t *tmp;
	for (size_t i = 0; i < (*map)->nr_buckets; i++) {
		current = &(*map)->buckets[i];
		while (*current) {
			tmp = *current;
			*current = (*current)->next;
			hash_free_(tmp);
		}
	}
	(*map)->nr_buckets = 0;
	(*map)->nr_entries = 0;
	if ((*map)->resize_in_progress) {
		for (size_t i = (*map)->resize_bucket; i < (*map)->nr_buckets_old; i++) {
			current = &(*map)->buckets_old[i];
			while (*current) {
				tmp = *current;
				*current = (*current)->next;
				hash_free_(tmp);
			}
		}
		(*map)->nr_buckets_old = 0;
		(*map)->nr_entries_old = 0;
		(*map)->resize_in_progress = false;
		(*map)->resize_bucket = 0;
	}
	hashmap_free_(map);
}

static hashmap_entry_t **hash_search_table_(hashmap_entry_t **buckets,
					   size_t nr_buckets,
					   const void *key,
					   size_t key_size,
					   size_t (* hash_function) (const void *key, size_t key_size)) {
	size_t bucket_nr = hash_bucket(nr_buckets, key, key_size, hash_function);
	hashmap_entry_t **current = &buckets[bucket_nr];
	while (*current) {
		if (hash_compare(key, key_size, (*current)->key, (*current)->key_size) == 0)
			break;
		current = &(*current)->next;
	}
	return current;
}

static hashmap_entry_t *hash_search_(const hashmap_t *map,
			      const void *key,
			      const size_t key_size) {
	if (!map)
		return NULL;
	hashmap_entry_t **current = hash_search_table_(map->buckets,
						       map->nr_buckets,
						       key,
						       key_size,
						       map->options.hash_function);
	if (!*current && map->resize_in_progress)
		current = hash_search_table_(map->buckets_old,
					     map->nr_buckets_old,
					     key,
					     key_size,
					     map->options.hash_function);
	return *current;
}

int hash_probe(const hashmap_t *map, const void *key, const size_t key_size) {
	if (!map)
		return HASHMAP_INVALID_ARG;
	if (!key && key_size)
		return HASHMAP_INVALID_ARG;
	hashmap_entry_t *output = hash_search_(map, key, key_size);
	if (output)
		return HASHMAP_OK;
	else
		return HASHMAP_ERROR;
}

int hash_search(hashmap_t *map,
		const void *key,
		size_t key_size,
		const void **data,
		size_t *data_size) {
	if (!data || !data_size)
		return HASHMAP_INVALID_ARG;
	if (!key && key_size)
		return HASHMAP_INVALID_ARG;
	if (!map)
		return HASHMAP_ERROR;
	hashmap_entry_t *entry = hash_search_(map, key, key_size);
	if (!entry)
		return HASHMAP_ERROR;
	*data = entry->data;
	*data_size = entry->data_size;
	return HASHMAP_OK;
}

static void move_on_resize(hashmap_t *map) {
	hashmap_entry_t *tmp = NULL;
	size_t nr_moves = HASHMAP_RESIZE_MOVES;
	size_t new_bucket_nr;
	while (nr_moves && map->nr_entries_old) {
		while (!map->buckets_old[map->resize_bucket])
			map->resize_bucket++;
		tmp = map->buckets_old[map->resize_bucket];
		new_bucket_nr = hash_bucket(map->nr_buckets, tmp->key, tmp->key_size, map->options.hash_function);
		map->buckets_old[map->resize_bucket] = tmp->next;
		tmp->next = map->buckets[new_bucket_nr];
		map->buckets[new_bucket_nr] = tmp;
		map->nr_entries++;
		map->nr_entries_old--;
		nr_moves--;
	}
	if (map->nr_entries_old == 0) {
		free(map->buckets_old);
		map->buckets_old = NULL;
		map->resize_in_progress = false;
		map->nr_buckets_old = 0;
		map->resize_bucket = 0;
	}
}

static int hashmap_resize(hashmap_t *map, size_t new_size) {
	hashmap_entry_t **tmp = calloc(new_size, sizeof(hashmap_entry_t *));
	if (tmp == NULL)
		return HASHMAP_ERROR;
	map->buckets_old = map->buckets;
	map->nr_buckets_old = map->nr_buckets;
	map->nr_entries_old = map->nr_entries;
	map->buckets = tmp;
	map->nr_buckets = new_size;
	map->nr_entries = 0;
	map->resize_bucket = 0;
	map->resize_in_progress = true;
	return HASHMAP_OK;
}

static int hash_insert_(hashmap_t *map,
			const void *key,
			size_t key_size,
			const void *data,
			size_t data_size) {
	hashmap_entry_t *new = malloc(sizeof(hashmap_entry_t));
	if (!new)
		return HASHMAP_ERROR;
	if (key_size) {
		new->key = malloc(key_size);
		if (!new->key) {
			hash_free_(new);
			return HASHMAP_ERROR;
		}
		memcpy(new->key, key, key_size);
	}
	else
		new->key = NULL;
	if (data_size) {
		new->data = malloc(data_size);
		if (!new->data) {
			hash_free_(new);
			return HASHMAP_ERROR;
		}
		memcpy(new->data, data, data_size);
	}
	else
		new->data = NULL;
	new->key_size = key_size;
	new->data_size = data_size;
	size_t bucket_nr = hash_bucket(map->nr_buckets, key, key_size, map->options.hash_function);
	new->next = map->buckets[bucket_nr];
	map->buckets[bucket_nr] = new;
	map->nr_entries++;
	return HASHMAP_OK;
}

int hash_insert(hashmap_t **map,
		 const void *key,
		 const size_t key_size,
		 const void *data,
		 const size_t data_size) {
	if (!map)
		return HASHMAP_INVALID_ARG;
	if (key == NULL && key_size != 0)
		return HASHMAP_INVALID_ARG;
	if (data == NULL && data_size != 0)
		return HASHMAP_INVALID_ARG;
	if (hash_search_(*map, key, key_size))
		return HASHMAP_ERROR;
	if (!*map) {
		*map = hashmap_create();
		if (!*map)
			return HASHMAP_ERROR;
	}
	int rc = hash_insert_(*map, key, key_size, data, data_size);
	if (rc != HASHMAP_OK)
		return rc;
	if (!(*map)->resize_in_progress) {
		if ((float)(*map)->nr_entries >= (*map)->options.grow_threshold
				* (float)(*map)->nr_buckets)
			hashmap_resize(*map, (*map)->nr_buckets << (*map)->options.resize_shift);
	}
	if ((*map)->resize_in_progress)
		move_on_resize(*map);
	return HASHMAP_OK;
}

int hash_update_(hashmap_entry_t *entry,
		 const void *data,
		 size_t data_size) {
	if (!data && data_size)
		return HASHMAP_INVALID_ARG;
	void *new_data = NULL;
	if (data_size) {
		new_data = malloc(data_size);
		if (!new_data)
			return HASHMAP_ERROR;
		memcpy(new_data, data, data_size);
	}
	free(entry->data);
	entry->data = new_data;
	entry->data_size = data_size;
	return HASHMAP_OK;
}

int hash_update(hashmap_t *map,
		 const void *key,
		 const size_t key_size,
		 const void *data,
		 const size_t data_size) {
	if (!map)
		return HASHMAP_ERROR;
	if (key == NULL && key_size != 0)
		return HASHMAP_INVALID_ARG;
	if (data == NULL && data_size != 0)
		return HASHMAP_INVALID_ARG;
	hashmap_entry_t *entry = hash_search_(map, key, key_size);
	if (!entry)
		return HASHMAP_ERROR;
	return hash_update_(entry, data, data_size);;
}

int hash_put(hashmap_t **map,
		 const void *key,
		 const size_t key_size,
		 const void *data,
		 const size_t data_size) {
	if (!map)
		return HASHMAP_INVALID_ARG;
	if (key == NULL && key_size != 0)
		return HASHMAP_INVALID_ARG;
	if (data == NULL && data_size != 0)
		return HASHMAP_INVALID_ARG;
	if (!*map) {
		*map = hashmap_create();
		if (!*map)
			return HASHMAP_ERROR;
	}
	hashmap_entry_t *entry = hash_search_(*map, key, key_size);
	if (entry)
		return hash_update_(entry, data, data_size);
	int rc = hash_insert_(*map, key, key_size, data, data_size);
	if (rc != HASHMAP_OK)
		return rc;
	if (!(*map)->resize_in_progress) {
		if ((float)(*map)->nr_entries >= (*map)->options.grow_threshold
				* (float)(*map)->nr_buckets)
			hashmap_resize(*map, (*map)->nr_buckets << (*map)->options.resize_shift);
	}
	if ((*map)->resize_in_progress)
		move_on_resize(*map);
	return HASHMAP_OK;
}

int hash_delete(hashmap_t **map, const void *key, size_t key_size) {
	if (!map)
		return HASHMAP_INVALID_ARG;
	if (!*map)
		return HASHMAP_ERROR;
	if (!key && key_size)
		return HASHMAP_INVALID_ARG;
	hashmap_entry_t **entry_ptr = hash_search_table_((*map)->buckets,
							 (*map)->nr_buckets,
							 key,
							 key_size,
							 (*map)->options.hash_function);
	hashmap_entry_t *tmp;
	if (*entry_ptr) {
		tmp = *entry_ptr;
		*entry_ptr = (*entry_ptr)->next;
		hash_free_(tmp);
		(*map)->nr_entries--;
	}
	else if ((*map)->resize_in_progress) {
		entry_ptr = hash_search_table_((*map)->buckets_old,
					       (*map)->nr_buckets_old,
					       key,
					       key_size,
					       (*map)->options.hash_function);
		if (*entry_ptr) {
			tmp = *entry_ptr;
			*entry_ptr = (*entry_ptr)->next;
			hash_free_(tmp);
			(*map)->nr_entries_old--;
		}
		else
			return HASHMAP_ERROR;
	}
	else
		return HASHMAP_ERROR;
	if (!(*map)->resize_in_progress) {
		if ((*map)->nr_buckets > (*map)->options.min_buckets
				&& (float)(*map)->nr_entries
				<= (*map)->options.shrink_threshold
				* (float)(*map)->nr_buckets)
			hashmap_resize(*map, (*map)->nr_buckets
				>> (*map)->options.resize_shift);
	}
	if ((*map)->resize_in_progress)
		move_on_resize(*map);
	if ((*map)->nr_entries == 0)
		hashmap_free_(map);
	return HASHMAP_OK;
}

size_t hashmap_getsize(hashmap_t *map) {
	if (!map)
		return 0;
	return (map->nr_entries + map->nr_entries_old);
}

int hashmap_info(const hashmap_t *map, hashmap_info_t *info) {
	if (map == NULL || info == NULL)
		return HASHMAP_INVALID_ARG;
	info->nr_buckets = map->nr_buckets;
	info->nr_entries = map->nr_entries;
	info->load_factor =
		(float)map->nr_entries / (float)map->nr_buckets;
	info->resize_in_progress = map->resize_in_progress;
	info->nr_buckets_old = map->nr_buckets_old;
	info->nr_entries_old = map->nr_entries_old;
	info->resize_bucket = map->resize_bucket;
	return HASHMAP_OK;
}

int hashmap_opt(hashmap_t *map, const int key, float value) {
	if (!map)
		return HASHMAP_INVALID_ARG;
	switch (key) {
	case HASHMAP_GROW_THRESHOLD:
		if (value < 0.0f)
			return HASHMAP_INVALID_ARG;
		if (value <
		    2.0f * map->options.shrink_threshold *
		    (1 << map->options.resize_shift))
			return HASHMAP_INVALID_ARG;
		if (map->options.grow_threshold == value)
			return HASHMAP_OK;
		map->options.grow_threshold = value;
		break;
	case HASHMAP_SHRINK_THRESHOLD:
		if (value < 0.0f)
			return HASHMAP_INVALID_ARG;
		if (map->options.grow_threshold <
		    2.0f * value * (float) (1 << map->options.resize_shift))
			return HASHMAP_INVALID_ARG;
		if (map->options.shrink_threshold == value)
			return HASHMAP_OK;
		map->options.shrink_threshold = value;
		break;
	case HASHMAP_MIN_BUCKETS:
		if (!isfinite(value))
			return HASHMAP_INVALID_ARG;
		if (value < 1.0f)
			return HASHMAP_INVALID_ARG;
		if (value != floorf(value))
			return HASHMAP_INVALID_ARG;
		if (value >= (float)SIZE_MAX)
			return HASHMAP_INVALID_ARG;
		if (map->options.min_buckets == (size_t) value)
			return HASHMAP_OK;
		map->options.min_buckets = (size_t) value;
		break;
	default:
		return HASHMAP_INVALID_ARG;
	}
	size_t nr_buckets_new =
		map->nr_buckets >
		map->options.min_buckets ? map->nr_buckets : map->
		options.min_buckets;
	while ((float) map->nr_entries >=
	       map->options.grow_threshold * (float) nr_buckets_new)
		nr_buckets_new <<= map->options.resize_shift;
	while (nr_buckets_new > map->options.min_buckets
	       && (float) map->nr_entries <=
	       map->options.shrink_threshold * (float) nr_buckets_new)
		nr_buckets_new >>= map->options.resize_shift;
	size_t new_bucket_nr;
	hashmap_entry_t *tmp;
	if (nr_buckets_new != map->nr_buckets) {
		hashmap_entry_t **buckets_new = calloc(sizeof(hashmap_entry_t *), nr_buckets_new);
		size_t nr_entries_new = 0;
		size_t resize_bucket = 0;
		while (map->nr_entries) {
			while (!map->buckets[resize_bucket])
				resize_bucket++;
			tmp = map->buckets[resize_bucket];
			new_bucket_nr =
				hash_bucket(nr_buckets_new, tmp->key, tmp->key_size, map->options.hash_function);
			map->buckets[resize_bucket] = tmp->next;
			tmp->next = buckets_new[new_bucket_nr];
			buckets_new[new_bucket_nr] = tmp;
			nr_entries_new++;
			map->nr_entries--;
		}
		free(map->buckets);
		map->buckets = buckets_new;
		map->nr_entries = nr_entries_new;
		map->nr_buckets = nr_buckets_new;
	}
	if (map->resize_in_progress) {
		while (map->nr_entries_old) {
			while (!map->buckets_old[map->resize_bucket])
				map->resize_bucket++;
			tmp = map->buckets_old[map->resize_bucket];
			new_bucket_nr =
				hash_bucket(map->nr_buckets, tmp->key,
					    tmp->key_size);
			map->buckets_old[map->resize_bucket] = tmp->next;
			tmp->next = map->buckets[new_bucket_nr];
			map->buckets[new_bucket_nr] = tmp;
			map->nr_entries++;
			map->nr_entries_old--;
		}
		free(map->buckets_old);
		map->buckets_old = NULL;
		map->resize_in_progress = false;
		map->nr_buckets_old = 0;
		map->resize_bucket = 0;
	}
	return HASHMAP_OK;
}
