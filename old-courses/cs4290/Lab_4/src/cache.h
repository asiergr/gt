#ifndef CACHE_H
#define CACHE_H
#define MAX_WAYS 16

#include <stdint.h>

#include "types.h"

/////////////////////////////////////////////////////////////////////////////////////////////
// Define the necessary data structures here (Look at Appendix A for more details)
/////////////////////////////////////////////////////////////////////////////////////////////

typedef struct Cache Cache;


typedef struct Cache_Line {
	bool valid;
	bool dirty;
	Addr tag;
	uint16_t core_id;
	uint64_t insertion_time;
	uint64_t umon_counter[2];
} Cache_Line;

typedef struct Cache_Set {
	Cache_Line cache_line[MAX_WAYS];
	// uns64 umon_counter[MAX_WAYS][2]; // Utility monitor counter
	uint16_t core0_quota;
	uint64_t misses_core0;
	uint64_t misses_core1;
} Cache_Set;

typedef struct Cache {
	uint64_t num_sets;
	uint64_t num_ways;
	uint64_t repl_policy;

	Cache_Set *cache_sets;
	Cache_Line last_evicted;

	uint64_t stat_read_access;
	uint64_t stat_write_access;
	uint64_t stat_read_miss;
	uint64_t stat_write_miss;
	uint64_t stat_dirty_evicts;

	uint64_t partition_freq;
	uint64_t counter;
} Cache;

/////////////////////////////////////////////////////////////////////////////////////////////
// Mandatory variables required for generating the desired final reports as necessary
// Used by cache_print_stats()
/////////////////////////////////////////////////////////////////////////////////////////////

// stat_read_access: Number of read (lookup accesses do not count as READ accesses)
//                   accesses made to the cache
// stat_write_access: Number of write accesses made to the cache
// stat_read_miss: Number of READ requests that lead to a MISS at the respective cache
// stat_write_miss: Number of WRITE requests that lead to a MISS at the respective cache
// stat_dirty_evicts: Count of requests to evict DIRTY lines

/////////////////////////////////////////////////////////////////////////////////////////////
// Functions to be implemented
/////////////////////////////////////////////////////////////////////////////////////////////

Cache* cache_new(uint64_t size, uint64_t assocs, uint64_t linesize, uint64_t repl_policy);
bool cache_access(Cache* c, Addr lineaddr, uint32_t is_write, uint32_t core_id);
void cache_install(Cache* c, Addr lineaddr, uint32_t is_write, uint32_t core_id);
uint32_t cache_find_victim(Cache* c, uint32_t set_index, uint32_t core_id);

void cache_partition_algo(Cache *c);

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////

void cache_print_stats(Cache* c, char* header);


#endif // CACHE_H
