#include <stdlib.h>
#include <stdio.h>

#include "hashmap.h"

typedef struct user {
	char *name;
	int age;
} user_t;

user_t users[] = {
	{"John Doe", 42},
	{"Jane Doe", 18}
};

#define hash_insert_(map, key, value) hash_insert((map), &(key), sizeof(int), &(user_t *){(value)}, sizeof(user_t *))

int main(void) {
	hashmap_t *map = NULL;
	for (int i = 0; i < sizeof(users) / sizeof(user_t); i++) {
		hash_insert_(&map, i, &users[i]);
		printf("User %i: %s (%i)\n", i, users[i].name, users[i].age);
	}
	printf("hashmap has %zu entries\n", hashmap_getsize(map));
	size_t size;
	user_t *user;
	for (int i = 0; i < sizeof(users) / sizeof(user_t); i++) {
		hash_search(map, &i, sizeof(int), (const void **)&user, &size);
		user = *(user_t **)user;
		printf("User %i: %s (%i)\n", i, user->name, user->age);
	}
	return 0;
}