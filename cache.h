#ifndef __CACHE_H__
#define __CACHE_H__

#include <stdlib.h>
#include "csapp.h"
/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 4000000 
#define MAX_OBJECT_SIZE 1000000 
typedef struct CachedItem CachedItem;

// When creating each CachedItem, the string url, headers
// should both be properly null terminated strings stored
// in dynamically allocated memory.
// The cache owns the memory for url, headers, item_p
// and should free the memory for these fields every time 
// when a CachedItem is to be deleted.
struct CachedItem {
  char *url;
  char *headers;
  void *item_p;
  size_t size;
  CachedItem *prev;
  CachedItem *next;
};

typedef struct {
  size_t size;
  CachedItem* first;
  CachedItem* last;
} CacheList;

extern void cache_init(CacheList *list);
extern void cache_URL(const char *URL, const char *headers, void *item, size_t size, CacheList *list);
extern CachedItem *find(const char *url, CacheList *list);
extern void cache_destruct(CacheList *list);

#endif
