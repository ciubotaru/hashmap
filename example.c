#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <sys/time.h>
#include <time.h>
#include "hashmap.h"
#include "hashmap-internals.h"

char key_buf[1024];
char data_buf[1024];

#if (defined(DEBUG) && DEBUG == 1)
#define debug_msg printf
#else
#define debug_msg(...)
#endif

void *hash(void *data, size_t data_size, size_t key_size) {
	void *key = calloc(key_size, 1);
	int i;
	unsigned char *key_ptr = (unsigned char *)key;
	unsigned char *data_ptr = (unsigned char *)data;
	for (i = 0; i < data_size; i++) {
		key_ptr[i % key_size] *= 37;
		key_ptr[i % key_size] += data_ptr[i];
	}
	return key;
}

int iterator(const void *key,
	     const size_t key_size,
	     const void *value,
	     const size_t value_size,
	     const char *context) {
	printf("Hash key: 0x");
	int i;
	char *ptr = (char *)key;
	for (i = 0; i < key_size; i++)
		printf("%02x", (int)ptr[i]);
	printf(", data: 0x");
	ptr = (char *)value;
	for (i = 0; i < value_size; i++)
		printf("%02x", (int)ptr[i]);
	printf("\n");
	return 0;
}

void insert_n_random(hashmap_t *map,
		     size_t key_size,
		     size_t data_size,
		     size_t count) {
	char *key = malloc(key_size);
	char *data = malloc(data_size);
	size_t i, j;
	for (i = 0; i < count; i++) {
		snprintf(key, key_size, "%ld", i);
		for (j = 0; j < data_size; j++)
			data[j] = rand() % (unsigned char)~0;	//(UCHAR_MAX + 1);
		hash_insert(&map, key, key_size, data, data_size);
	}
	free(key);
	free(data);
}

void delete_n_random(hashmap_t *map, size_t count) {
	debug_msg("Starting batch delete...\n");
	if (!map) {
		debug_msg("No hashmap found. Exiting.\n");
		return;
	}
	if (count > map->nr_entries + map->nr_entries_old) {
		debug_msg("Delete request too big. Reducing count.\n");
		count = map->nr_entries + map->nr_entries_old;
	}
	int i = 0;
	int j = 0;
	hashmap_entry_t  *current;
	if (map->resize_in_progress) {
		while (i < count && map->nr_entries_old > 0) {
			while (!map->buckets_old[j])
				j++;
			current = map->buckets_old[j];
			map->buckets_old[j] = current->next;
			free(current->key);
			free(current->data);
			free(current);
			map->nr_entries_old--;
			i++;
		}
	}
	map->resize_in_progress = 0;
	j = 0;
	while (i < count) {
		while (!map->buckets[j])
			j++;
		current = map->buckets[j];
		map->buckets[j] = current->next;
		free(current->key);
		free(current->data);
		free(current);
		map->nr_entries--;
		i++;
	}
	debug_msg("Batch delete complete.\n");
}

struct benchmark_results {
	unsigned long start;
	unsigned long end;
	unsigned long diff;
};

unsigned long get_microseconds()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return 1000000 * tv.tv_sec + tv.tv_usec;
}

static struct benchmark_results benchmark_results;

void reset_timer()
{
	benchmark_results.start = 0;
	benchmark_results.end = 0;
	benchmark_results.diff = 0;
}

void start_timer()
{
	benchmark_results.start = get_microseconds();
}

void stop_timer()
{
	benchmark_results.end = get_microseconds();
	benchmark_results.diff =
	    benchmark_results.end - benchmark_results.start;
}

void print_results()
{
	printf("----\nBenchmark duration: %lu\n----\n", benchmark_results.diff);
}

int main() {
	srand(time(NULL));
	int sample_size = 10000;
	void *key_array[sample_size];
	void *data_array[sample_size];
	char *key, *data;
	int i, j;
	size_t key_size = 10;
	size_t data_size= 10;
	hashmap_t *map = hashmap_create();
//	hashmap_opt(map, HASHMAP_SHRINK_TRIGGER, 0.1);
//	hashmap_opt(map, HASHMAP_MIN_BUCKETS, 2);
//	hashmap_opt(map, HASHMAP_RESIZE_FACTOR, 2);
//	hashmap_opt(map, HASHMAP_GROW_TRIGGER, 2);

//	hashmap_info(map);

	printf("Random insert\n");
	for (i = 0; i < sample_size; i++) {
		data = malloc(data_size);
		for (j = 0; j < data_size; j++)
			data[j] = (unsigned char)rand();
		key = hash(data, data_size, key_size);
		key_array[i] = key;
		data_array[i] = data;
	}
	start_timer();
	int rc;
	for (i = 0; i < sample_size; i++) {
		rc = hash_insert(&map, key_array[i], key_size, data_array[i], data_size);
	}
	stop_timer();
	print_results();
	reset_timer();


//	hashmap_info(map);

	printf("Random update\n");
	start_timer();
	for (i = 0; i < sample_size; i++) {
		rc = hash_update(map, key_array[i], key_size, data_array[i], data_size);
	}
	stop_timer();
	print_results();
	reset_timer();

//	hashmap_info(map);

	printf("Random populate\n");
	start_timer();
	insert_n_random(map, key_size, data_size, sample_size);
	stop_timer();
	print_results();
	reset_timer();
/*
	for (i = 0; i < sample_size; i++) {
//              sprintf(key_buf, "%d",);
		if (rand() % 2) {
			hash_delete(sample_array[i], 10);
		} else {
			hash_delete(sample_array[i], 10);
		}
	}
	hashmap_info();
	hashmap_iterate(*iterator, NULL);
*/

//	hashmap_info(map);

	printf("Serial delete\n");
	start_timer();
	for (i = 0; i < sample_size; i++) {
		hash_delete(&map, key_array[i], key_size);
	}
	stop_timer();
	print_results();
	reset_timer();

//	hashmap_info(map);

	printf("Random depopulate\n");
	start_timer();
	delete_n_random(map, hashmap_getsize(map));
	//hashmap_clear(&msp);
	stop_timer();
	print_results();

//	hashmap_info(map);

	printf("%zu entries.\n", hashmap_getsize(map));
	hashmap_clear(&map);
	return 0;
}

