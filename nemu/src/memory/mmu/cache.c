#include "memory/mmu/cache.h"
#include <stdlib.h>
#include <string.h>
#include "memory/memory.h"

void init_cache()
{
	cache.S = 1<<3;
	cache.B = 1<<6;
	cache.E = 128;
	cache.line = (line_t **)malloc(sizeof(line_t *) * cache.S);
	for(int i = 0; i < cache.S; i++) {
		cache.line[i] = (line_t *)malloc(sizeof(line_t) * cache.E);
		for(int j = 0; j < cache.E; j++) {
			cache.line[i][j].v = 0;
			cache.line[i][j].tag = -1;
			cache.line[i][j].t = 0;
			cache.line[i][j].data = (uint8_t *)malloc(sizeof(uint8_t) * cache.B);
			memset(cache.line[i][j].data, 0, sizeof(uint8_t) * cache.B);
		}
	}
}

uint32_t cache_read(paddr_t paddr, size_t len)
{
	int tag = paddr / (cache.B * cache.S);
	int s = (paddr / cache.B) % cache.S;
	int b = paddr % cache.B;
	int u = -1;
	uint32_t ret = 0;
	if(b+len>cache.B) {
		for(int i=0;i<len;i++)
			ret |= ((uint32_t)cache_read(paddr+i, 1)) << (i*8);
		return ret;
	}
	for(int i=0;i<cache.E;i++) {
		if(cache.line[s][i].v && cache.line[s][i].tag == tag) {
			u = i;
			break;
		}
	}
	if(u == -1) {
		int mx = -1;
		for(int i=0;i<cache.E;i++) {
			if(!cache.line[s][i].v) {
				u = i;
				break;
			}
			if(cache.line[s][i].t > mx)
				mx = cache.line[s][i].t, u = i;
		}
		memcpy(cache.line[s][u].data, hw_mem + (paddr / cache.B) * cache.B, cache.B);
	}
	cache.line[s][u].v = 1;
	cache.line[s][u].t = 0;
	cache.line[s][u].tag = tag;
	for(int i=0;i<cache.E;i++)
		if(i != u && cache.line[s][i].v)
			cache.line[s][i].t++;
	memcpy(&ret, cache.line[s][u].data + b, len);
	return ret;
}

void cache_write(paddr_t paddr, size_t len, uint32_t data)
{
	int tag = paddr / (cache.B * cache.S);
	int s = (paddr / cache.B) % cache.S;
	int b = paddr % cache.B;
	if(b+len>cache.B) {
		for(int i=0;i<len;i++)
			cache_write(paddr+i, 1, (uint32_t)((uint8_t *)&data)[i]);
		return;
	}
	int u = -1;
	for(int i=0;i<cache.E;i++) {
		if(cache.line[s][i].v && cache.line[s][i].tag == tag) {
			u = i;
			break;
		}
	}
	for(int i=0;i<cache.E;i++)
		if(i != u && cache.line[s][i].v)
			cache.line[s][i].t++;
	if(u == -1) {
		hw_mem_write(paddr, len, data);
	} else {
		cache.line[s][u].v = 1;
		cache.line[s][u].t = 0;
		cache.line[s][u].tag = tag;
		memcpy(cache.line[s][u].data + b, &data, len);
		hw_mem_write(paddr, len, data);
	}
}