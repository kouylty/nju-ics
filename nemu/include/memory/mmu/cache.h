#ifndef __CACHE_H__
#define __CACHE_H__

#include "nemu.h"
#include "memory/memory.h"
#include <stdlib.h>
#include <string.h>

#ifdef CACHE_ENABLED

typedef struct CacheLine {
	uint8_t v;
	int tag;
	int t;
	uint8_t *data;
} line_t;
typedef struct Cache{
	int S,B,E;
	line_t **line;
} cache_t;

cache_t cache;

// init the cache
void init_cache();

// write data to cache
void cache_write(paddr_t paddr, size_t len, uint32_t data);

// read data from cache
uint32_t cache_read(paddr_t paddr, size_t len);

#endif

#endif
