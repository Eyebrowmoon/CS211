#include "cache.h"
#include <stdlib.h>
#include <string.h>

/* Initialize the cache */
void init_cache(cache_t *cache)
{
    struct cache_object *head, *tail;

    head = (struct cache_object *) Calloc(1, sizeof(struct cache_object));
    tail = (struct cache_object *) Calloc(1, sizeof(struct cache_object));
    head->next = tail;
    tail->prev = head;

    total_size = 0;
    Sem_init(&mutex, 0, 1);     
    cache->head = head;
    cache->tail = tail;
    next_victim = NULL;
}

/* Free all the object of cache */
void free_cache(cache_t *cache)
{
    struct cache_object *cur_object, *next_object;
    
    cur_object = cache->head;
    while (cur_object) {
        next_object = cur_object->next;

        if (cur_object->data)
            Free(cur_object->data);
        Free(cur_object);

        cur_object = next_object;
    }    
}

/* Add new object to memory */
int add_object(cache_t *cache, char *url, char *data, size_t size)
{
    struct cache_object *object;
    struct cache_object *head, *head_next;

    /* Allocate memory dynamically */
    object = (struct cache_object *) Malloc(sizeof(struct cache_object));
    object->data = (char *) Malloc(size); 

    /* Set the object */
    object->chance = 1;
    object->size = size;
    strcpy(object->url, url);
    memcpy(object->data, data, size);

    /* Enter critical section */
    P(&cache->mutex);

    head = cache->head;
    head_next = head->next;

    object->next = head_next;
    object->prev = head;
    head_next->prev = object;
    head->next = object;

    cache->total_size += size;

    V(&cache->mutex);
}


