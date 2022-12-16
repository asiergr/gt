#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "cache.h"

extern uint64_t cycle;
extern uint64_t SWP_CORE0_WAYS;


/////////////////////////////////////////////////////////////////////////////////////
// ---------------------- DO NOT MODIFY THE PRINT STATS FUNCTION --------------------
/////////////////////////////////////////////////////////////////////////////////////


void cache_print_stats(Cache* c, char* header){
	double read_mr = 0;
	double write_mr = 0;

	if (c->stat_read_access) {
		read_mr = (double)(c->stat_read_miss) / (double)(c->stat_read_access);
	}

	if (c->stat_write_access) {
		write_mr = (double)(c->stat_write_miss) / (double)(c->stat_write_access);
	}

	printf("\n%s_READ_ACCESS    \t\t : %10llu", header, c->stat_read_access);
	printf("\n%s_WRITE_ACCESS   \t\t : %10llu", header, c->stat_write_access);
	printf("\n%s_READ_MISS      \t\t : %10llu", header, c->stat_read_miss);
	printf("\n%s_WRITE_MISS     \t\t : %10llu", header, c->stat_write_miss);
	printf("\n%s_READ_MISS_PERC  \t\t : %10.3f", header, 100*read_mr);
	printf("\n%s_WRITE_MISS_PERC \t\t : %10.3f", header, 100*write_mr);
	printf("\n%s_DIRTY_EVICTS   \t\t : %10llu", header, c->stat_dirty_evicts);

	printf("\n");
}


/////////////////////////////////////////////////////////////////////////////////////
// Allocate memory for the data structures 
// Initialize the required fields 
/////////////////////////////////////////////////////////////////////////////////////


Cache* cache_new(uint64_t size, uint64_t assoc, uint64_t linesize, uint64_t repl_policy){
  Cache *c = (Cache *) calloc(1, sizeof(Cache));
  assert(assoc <= MAX_WAYS);

  uint16_t sets = size / (linesize * assoc);

  c->num_sets = sets;
  c->num_ways = assoc;
  c->repl_policy = repl_policy;
  c->cache_sets = (Cache_Set *) calloc(sets, sizeof(Cache_Set));

//   for (int i = 0; i < sets; ++i) {
//     c->cache_sets[i].core0_quota = assoc / 2;
//   }

//   c->partition_freq = 1000000;

  return c;
}


/////////////////////////////////////////////////////////////////////////////////////
// Return HIT if access hits in the cache, MISS otherwise 
// Also if is_write is TRUE, then mark the resident line as dirty
// Update appropriate stats
/////////////////////////////////////////////////////////////////////////////////////


bool cache_access(Cache* c, Addr lineaddr, uint32_t is_write, uint32_t core_id){
  uint64_t n_idx_bits = ceil(log(c->num_sets)/log(2));
  uint64_t idx_mask = (1 << n_idx_bits) - 1;
  uint64_t idx = (lineaddr & idx_mask);
  uint64_t tag = (lineaddr >> n_idx_bits);

  bool res = MISS;

  for (uint16_t i = 0; i < c->num_ways; ++i) { // iterate thru ways
	if (!(c->cache_sets[idx].cache_line[i].valid && c->cache_sets[idx].cache_line[i].tag == tag)) {
		// not here
		continue;
	}
	// we hit
  	res = HIT;

    if (is_write) {
		c->cache_sets[idx].cache_line[i].dirty = TRUE;
	}
    break;
  } 

  if (is_write) {
    c->stat_write_access++;
    if (res == MISS) {
      c->stat_write_miss++;
    }
  } else {
    c->stat_read_access++;
    if (res == MISS) {
      c->stat_read_miss++;
    }
  }
  return res;
}


/////////////////////////////////////////////////////////////////////////////////////
// Install the line: determine victim using replacement policy
// Copy victim into last_evicted_line for tracking writebacks
/////////////////////////////////////////////////////////////////////////////////////


void cache_install(Cache* c, Addr lineaddr, uint32_t is_write, uint32_t core_id){

  // Find victim using cache_find_victim()
  
  uint64_t n_idx_bits = ceil(log(c->num_sets)/log(2));
  uint64_t idx_mask = (1 << n_idx_bits) - 1;
  uint64_t idx = (lineaddr & idx_mask);
  uint64_t tag = (lineaddr >> n_idx_bits);

  uint32_t victim = cache_find_victim(c, idx, core_id);

  c->last_evicted = c->cache_sets[idx].cache_line[victim];
  if (c->last_evicted.valid && c->last_evicted.dirty) c->stat_dirty_evicts++;

  c->cache_sets[idx].cache_line[victim].tag = tag;
  c->cache_sets[idx].cache_line[victim].dirty = (is_write == TRUE);
  c->cache_sets[idx].cache_line[victim].valid = TRUE;
  c->cache_sets[idx].cache_line[victim].core_id = core_id;
  c->cache_sets[idx].cache_line[victim].insertion_time = cycle;
}


/////////////////////////////////////////////////////////////////////////////////////
// You may find it useful to split victim selection from install
/////////////////////////////////////////////////////////////////////////////////////


uint32_t cache_find_victim(Cache *c, uint32_t set_index, uint32_t core_id){

  for (uint16_t i = 0; i < c->num_ways; i++) { // check if empty space exists
    if (!c->cache_sets[set_index].cache_line[i].valid) return i;
  }

  // FIFO
  if (c->repl_policy == 0) {
    uint64_t victim = 0;
    uint64_t victim_last_access = c->cache_sets[set_index].cache_line[victim].insertion_time;
    for (uint16_t i = 1; i < c->num_ways; i++) {
      if (c->cache_sets[set_index].cache_line[i].insertion_time < victim_last_access) {
        victim = i;
        victim_last_access = c->cache_sets[set_index].cache_line[i].insertion_time;
      }
    }
    return victim;
  }

  // SWP
  else if (c->repl_policy == 1) {
    uint32_t n_core0 = 0;
    for (int i = 0; i < c->num_ways; i++) {
      if (c->cache_sets[set_index].cache_line[i].core_id != 0) continue;
      n_core0++;
    }
    uint32_t victim_core = 0;
    if (n_core0 > SWP_CORE0_WAYS) {
      victim_core = 0;
    } else if (n_core0 < SWP_CORE0_WAYS) {
      victim_core = 1;
    } else {
      victim_core = core_id;
    }

    uint32_t victim_line = 0;
    uint32_t victim_last_access = -1;
    for (int i = 0; i < c->num_ways; i++) {
      if (c->cache_sets[set_index].cache_line[i].core_id != victim_core) continue;
      victim_line = i;
      victim_last_access = c->cache_sets[set_index].cache_line[i].insertion_time;
      break;
    }
    for (int i = victim_line + 1; i < c->num_ways; i++) {
      if (!(c->cache_sets[set_index].cache_line[i].core_id == victim_core && 
        c->cache_sets[set_index].cache_line[i].insertion_time < victim_last_access)) continue;
        victim_line = i;
        victim_last_access = c->cache_sets[set_index].cache_line[i].insertion_time;
    }
    return victim_line; 
  }
  return 0;
}