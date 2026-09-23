#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "hashmap.h"

int callback(const void *key, size_t key_size, const void *data,
	     size_t data_size, const void *context) {
	const char *key_byte = key;
	printf("Key: ");
	for (int i = 0; i < key_size; i++) {
		printf("%02X ", key_byte[i]);
	}
	printf("\n");
	const char *data_byte = data;
	printf("Data: ");
	for (int i = 0; i < data_size; i++) {
		printf("%02X ", data_byte[i]);
	}
	printf("\n");
	return 0;
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
	hashmap_iterate(map, &callback, NULL);
	return 0;
}
