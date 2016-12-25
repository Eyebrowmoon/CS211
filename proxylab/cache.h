#ifndef _CACHE_H
#define _CACHE_H

#include "csapp.h"

/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

struct cache_object {
    int chance;
    size_t size;
    char url[MAXLINE];
    char *data;

    struct cache_object *prev;
    struct cache_object *next;
};

typedef struct {
    size_t total_size;     /* Global sum of size */
    sem_t mutex;        /* Mutex to protect linked list */

    struct cache_object *head;  /* First cached object */
    struct cache_object *tail;  /* Last cached object */
} cache_t;

void init_cache(cache_t *cache);
void free_cache(cache_t *cache);

void add_object(cache_t *cache, char *url, char *data, size_t size);
struct cache_object *find_object(cache_t *cache, char *url);

#endif
