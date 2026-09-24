#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "hashmap.h"

size_t hash_function(const void *key, size_t key_size) {
	size_t hash = 0;
	char *s = (char *) key;
	int len = strlen(s);
	for (int i = 0; i < len; i++) {
		hash <<= 8;
		hash += s[i];
	}
	return hash;
}

int main(void) {
	srand((unsigned int) time(NULL));
	hashmap_t *map = NULL;
	unsigned char *data;
	size_t data_size = 10;
	for (int i = 0; i < 20; i++) {
		data = calloc(1, data_size);
		for (int j = 0; j < data_size; j++)
			data[j] = (unsigned char) rand();
		hash_insert(&map, &i, sizeof(i), data, data_size);
		free(data);
	}
	hashmap_info_t info;
	hashmap_info(map, &info);
	printf("Resize in progress: %s\n", info.resize_in_progress ? "yes" : "no");
	hashmap_set_hash_function(map, &hash_function);
	hashmap_info(map, &info);
	printf("Resize in progress: %s\n", info.resize_in_progress ? "yes" : "no");
	for (int i = 0; i < 20; i++) {
		if (hash_probe(map, &i, sizeof(i)) != HASHMAP_OK)
			printf("Fail!\n");;
	}
	return 0;
}
