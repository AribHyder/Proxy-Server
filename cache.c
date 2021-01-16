#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "csapp.h"
#include "cache.h"


/* cache_init initializes the input cache linked list. */
void cache_init(CacheList *list) {
}

/* cache_URL adds a new cached item to the linked list. It takes the
 * URL being cached, a link to the content, the size of the content, and
 * the linked list being used. It creates a struct holding the metadata
 * and adds it at the beginning of the linked list.
 */
void cache_URL(const char *URL, const char *headers, void *item, size_t size, CacheList *list) {
}


/* find iterates through the linked list and returns a pointer to the
 * struct associated with the requested URL. If the requested URL is
 * not cached, it returns null.
 */
CachedItem *find(const char *URL, CacheList *list) {
  return NULL;
}


/* frees the memory used to store each cached object, and frees the struct
 * used to store its metadata. */
void cache_destruct(CacheList *list) {
}
