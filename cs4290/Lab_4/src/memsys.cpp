#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include "memsys.h"

#define PAGE_SIZE 4096

//---- Cache Latencies  ------

#define DCACHE_HIT_LATENCY   1
#define ICACHE_HIT_LATENCY   1
#define L2CACHE_HIT_LATENCY  10

extern MODE   SIM_MODE;
extern uint64_t  CACHE_LINESIZE;
extern uint64_t  REPL_POLICY;

extern uint64_t  DCACHE_SIZE;
extern uint64_t  DCACHE_ASSOC;
extern uint64_t  ICACHE_SIZE;
extern uint64_t  ICACHE_ASSOC;
extern uint64_t  L2CACHE_SIZE;
extern uint64_t  L2CACHE_ASSOC;
extern uint64_t  L2CACHE_REPL;
extern uint64_t  NUM_CORES;

////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////


Memsys* memsys_new(void){
	Memsys* sys = (Memsys*)calloc(1, sizeof (Memsys));

	switch(SIM_MODE) {
		case SIM_MODE_A:
			sys->dcache = cache_new(DCACHE_SIZE, DCACHE_ASSOC, CACHE_LINESIZE, REPL_POLICY);
			break;  

		case SIM_MODE_B:
		case SIM_MODE_C:
			sys->dcache = cache_new(DCACHE_SIZE, DCACHE_ASSOC, CACHE_LINESIZE, REPL_POLICY);
			sys->icache = cache_new(ICACHE_SIZE, ICACHE_ASSOC, CACHE_LINESIZE, REPL_POLICY);
			sys->l2cache = cache_new(L2CACHE_SIZE, L2CACHE_ASSOC, CACHE_LINESIZE, REPL_POLICY);
			sys->dram = dram_new();
			break;

		case SIM_MODE_D:
		case SIM_MODE_E:
			for (int i=0; i<NUM_CORES; i++) {
				sys->dcache_coreid[i] = cache_new(DCACHE_SIZE, DCACHE_ASSOC, CACHE_LINESIZE, REPL_POLICY);
				sys->icache_coreid[i] = cache_new(ICACHE_SIZE, ICACHE_ASSOC, CACHE_LINESIZE, REPL_POLICY);
			}
			sys->l2cache = cache_new(L2CACHE_SIZE, L2CACHE_ASSOC, CACHE_LINESIZE, L2CACHE_REPL);
			sys->dram = dram_new();
			break;
		default:
			break;
	}

	return sys;
}


////////////////////////////////////////////////////////////////////
// Return the latency of a memory operation
////////////////////////////////////////////////////////////////////

uint64_t memsys_access(Memsys* sys, Addr addr, Access_Type type, uint32_t core_id){
	uint32_t delay = 0;

	// all cache transactions happen at line granularity, so get lineaddr
	Addr lineaddr = addr / CACHE_LINESIZE;

	switch (SIM_MODE) {
		case SIM_MODE_A:
			delay = memsys_access_modeA(sys,lineaddr,type,core_id);
			break;

		case SIM_MODE_B:
		case SIM_MODE_C:
			delay = memsys_access_modeBC(sys,lineaddr,type,core_id);
			break;

		case SIM_MODE_D:
		case SIM_MODE_E:
			delay = memsys_access_modeDE(sys,lineaddr,type,core_id);
			break;
		default:
			break;
	}

	//update the stats

	switch (type) {
		case ACCESS_TYPE_IFETCH: 
			sys->stat_ifetch_access++;
			sys->stat_ifetch_delay += delay;
			break;
		case ACCESS_TYPE_LOAD:
			sys->stat_load_access++;
			sys->stat_load_delay += delay;
			break;
		case ACCESS_TYPE_STORE:
			sys->stat_store_access++;
			sys->stat_store_delay += delay;
			break;
		default:
			break;
	}

	return delay;
}



////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////

void memsys_print_stats(Memsys* sys){
	char header[256];
	sprintf(header, "MEMSYS");

	double ifetch_delay_avg=0, load_delay_avg=0, store_delay_avg=0;

	if (sys->stat_ifetch_access) {
		ifetch_delay_avg = (double)(sys->stat_ifetch_delay) / (double)(sys->stat_ifetch_access);
	}

	if (sys->stat_load_access) {
		load_delay_avg = (double)(sys->stat_load_delay) / (double)(sys->stat_load_access);
	}

	if (sys->stat_store_access) {
		store_delay_avg = (double)(sys->stat_store_delay) / (double)(sys->stat_store_access);
	}


	printf("\n");
	printf("\n%s_IFETCH_ACCESS  \t\t : %10llu",  header, sys->stat_ifetch_access);
	printf("\n%s_LOAD_ACCESS    \t\t : %10llu",  header, sys->stat_load_access);
	printf("\n%s_STORE_ACCESS   \t\t : %10llu",  header, sys->stat_store_access);
	printf("\n%s_IFETCH_AVGDELAY\t\t : %10.3f",  header, ifetch_delay_avg);
	printf("\n%s_LOAD_AVGDELAY  \t\t : %10.3f",  header, load_delay_avg);
	printf("\n%s_STORE_AVGDELAY \t\t : %10.3f",  header, store_delay_avg);
	printf("\n");

	switch (SIM_MODE) {
		case SIM_MODE_A:
			sprintf(header, "DCACHE");
			cache_print_stats(sys->dcache, header);
			break;
		case SIM_MODE_B:
		case SIM_MODE_C:
			sprintf(header, "ICACHE");
			cache_print_stats(sys->icache, header);
			sprintf(header, "DCACHE");
			cache_print_stats(sys->dcache, header);
			sprintf(header, "L2CACHE");
			cache_print_stats(sys->l2cache, header);
			dram_print_stats(sys->dram);
			break;

		case SIM_MODE_D:
		case SIM_MODE_E:
			assert(NUM_CORES==2); //Hardcoded
			sprintf(header, "ICACHE_0");
			cache_print_stats(sys->icache_coreid[0], header);
			sprintf(header, "DCACHE_0");
			cache_print_stats(sys->dcache_coreid[0], header);
			sprintf(header, "ICACHE_1");
			cache_print_stats(sys->icache_coreid[1], header);
			sprintf(header, "DCACHE_1");
			cache_print_stats(sys->dcache_coreid[1], header);
			sprintf(header, "L2CACHE");
			cache_print_stats(sys->l2cache, header);
			dram_print_stats(sys->dram);
			break;
		default:
			break;
	}
}


////////////////////////////////////////////////////////////////////
// Used by Part A to access the L1 cache
////////////////////////////////////////////////////////////////////

uint64_t memsys_access_modeA(Memsys* sys, Addr lineaddr, Access_Type type, uint32_t core_id){
  
	// IFETCH accesses go to icache, which we don't have in part A
	bool needs_dcache_access = !(type == ACCESS_TYPE_IFETCH);

	// Stores write to the caches
	bool is_write = (type == ACCESS_TYPE_STORE);

	if (needs_dcache_access) {
		// Miss
		if (!cache_access(sys->dcache,lineaddr,is_write,core_id)) {
			// Install the new line in L1
			cache_install(sys->dcache,lineaddr,is_write,core_id);
		}
	}

	// Timing is not simulated in Part A
	return 0;
}

////////////////////////////////////////////////////////////////////
// Used by Parts B,C to access the L1 icache + L1 dcache
// Returns the access latency
////////////////////////////////////////////////////////////////////

uint64_t memsys_access_modeBC(Memsys* sys, Addr lineaddr, Access_Type type, uint32_t core_id){
	uint64_t delay = 0;

	// Perform the icache/dcache access

	// On dcache miss, access the L2 + install the new line + if needed, perform writeback

  if(type == ACCESS_TYPE_IFETCH){
    delay += ICACHE_HIT_LATENCY;

    if (cache_access(sys->icache, lineaddr, FALSE, core_id) == MISS) {
      uint64_t delay_l2 = memsys_L2_access(sys, lineaddr, FALSE, core_id);
      cache_install(sys->icache, lineaddr, FALSE, core_id);
      delay += delay_l2;
    }
  }
    
  if(type == ACCESS_TYPE_LOAD){
    delay += DCACHE_HIT_LATENCY;

    if (cache_access(sys->dcache, lineaddr, FALSE, core_id) == MISS) {
      uint64_t l2_del = memsys_L2_access(sys, lineaddr, FALSE, core_id);
      cache_install(sys->dcache, lineaddr, FALSE, core_id);
      delay += l2_del;

      if (sys->dcache->last_evicted.dirty && sys->dcache->last_evicted.valid) {
        uint32_t n_idx_bits = ceil(log(sys->dcache->num_sets)/log(2));
        uint32_t mask = (1 << n_idx_bits) - 1;
        uint32_t idx = (lineaddr & mask);
        uint32_t dest_addr = (sys->dcache->last_evicted.tag << n_idx_bits) | idx;

        memsys_L2_access(sys, dest_addr, TRUE, core_id);
      }
    }
  }
  

  if(type == ACCESS_TYPE_STORE){
    delay += DCACHE_HIT_LATENCY;

    if (cache_access(sys->dcache, lineaddr, TRUE, core_id) == MISS) {
      uint64_t l2_del = memsys_L2_access(sys, lineaddr, FALSE, core_id);
      cache_install(sys->dcache, lineaddr, TRUE, core_id);
      delay += l2_del;

      if (sys->dcache->last_evicted.dirty && sys->dcache->last_evicted.valid) {
        uint32_t n_idx_bits = ceil(log(sys->dcache->num_sets)/log(2));
        uint32_t mask = (1 << n_idx_bits) - 1;
        uint32_t idx = (lineaddr & mask);
        uint32_t dest_addr = (sys->dcache->last_evicted.tag << n_idx_bits) | idx;
        memsys_L2_access(sys, dest_addr, TRUE, core_id);
      }
    }
  }


	return delay;
}

////////////////////////////////////////////////////////////////////
// Used by Parts B,C to access the L2
// Returns the access latency
////////////////////////////////////////////////////////////////////

uint64_t memsys_L2_access(Memsys* sys, Addr lineaddr, bool is_writeback, uint32_t core_id){ 
	uint64_t delay = L2CACHE_HIT_LATENCY;

	if (cache_access(sys->l2cache, lineaddr, is_writeback, core_id) == HIT) return delay;

    if (is_writeback == FALSE) {
      delay += dram_access(sys->dram, lineaddr, FALSE);
    }
    cache_install(sys->l2cache, lineaddr, is_writeback, core_id);
	bool cond = sys->l2cache->last_evicted.dirty && sys->l2cache->last_evicted.valid;
    if (cond) {
        uint64_t n_idx_bits = ceil(log(sys->dcache->num_sets)/log(2));
        uint64_t mask = (1 << n_idx_bits) - 1;
        uint64_t idx = (lineaddr & mask);
        uint64_t dest_address = (sys->dcache->last_evicted.tag << n_idx_bits) | idx;
      dram_access(sys->dram, dest_address, TRUE);
    }
	return delay;
}

/////////////////////////////////////////////////////////////////////
// Converts virtual page number (VPN) to physical frame number (PFN)
// Note, you will need additional operations to obtain
// VPN from lineaddr and to get physical lineaddr using PFN.
/////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
// ------------- DO NOT CHANGE THE CODE OF THIS FUNCTION ----------
////////////////////////////////////////////////////////////////////

uint64_t memsys_convert_vpn_to_pfn(Memsys *sys, uint64_t vpn, uint32_t core_id){
	uint64_t tail = vpn & 0x000fffff;
	uint64_t head = vpn >> 20;
	uint64_t pfn  = tail + (core_id << 21) + (head << 21); 
	assert(NUM_CORES==2);
	return pfn;
}


/////////////////////////////////////////////////////////////////////
// Used by Parts D,E to access per-core L1 icache + L1 dcache
/////////////////////////////////////////////////////////////////////

uint64_t memsys_access_modeDE(Memsys* sys, Addr v_lineaddr, Access_Type type, uint32_t core_id){
	uint64_t delay = 0;
	Addr p_lineaddr;

	// First convert lineaddr from virtual (v) to physical (p) using the
	// function memsys_convert_vpn_to_pfn(). Page size is defined to be 4KB.
	// NOTE: VPN_to_PFN operates at page granularity and returns page addr

  uint64_t n_offset_b = ceil(log(PAGE_SIZE)/log(2)) - ceil(log(CACHE_LINESIZE)/log(2));
  uint64_t mask = (1 << n_offset_b) - 1;
  uint64_t offset = (v_lineaddr & mask);

  uint64_t vpn = v_lineaddr >> n_offset_b;
  uint64_t pfn = memsys_convert_vpn_to_pfn(sys, vpn, core_id);

  p_lineaddr = (pfn << n_offset_b) | offset;

  if(type == ACCESS_TYPE_IFETCH){
    bool is_hit = cache_access(sys->icache_coreid[core_id], p_lineaddr, FALSE, core_id);
    delay += ICACHE_HIT_LATENCY;

    if (is_hit == MISS) {
      uint64_t delay_l2 = memsys_L2_access_multicore(sys, p_lineaddr, FALSE, core_id);
      cache_install(sys->icache_coreid[core_id], p_lineaddr, FALSE, core_id);
      delay += delay_l2;
    }
  }

  if(type == ACCESS_TYPE_LOAD){
    bool is_hit = cache_access(sys->dcache_coreid[core_id], p_lineaddr, FALSE, core_id);
    delay += DCACHE_HIT_LATENCY;
    if (is_hit == MISS) {
      uint64_t delay_l2 = memsys_L2_access_multicore(sys, p_lineaddr, FALSE, core_id);

      cache_install(sys->dcache_coreid[core_id], p_lineaddr, FALSE, core_id);
      delay += delay_l2;

      if (sys->dcache_coreid[core_id]->last_evicted.dirty && sys->dcache_coreid[core_id]->last_evicted.valid) {
        uint64_t n_idx_bits = ceil(log(sys->dcache_coreid[core_id]->num_sets)/log(2));
        uint64_t mask = (1 << n_idx_bits) - 1;
        uint64_t idx = (p_lineaddr & mask);
        uint64_t dest_address = (sys->dcache_coreid[core_id]->last_evicted.tag << n_idx_bits) | idx;
        memsys_L2_access_multicore(sys, dest_address, TRUE, core_id);
      }
    }
  }
  
  if(type == ACCESS_TYPE_STORE){
    bool is_hit = cache_access(sys->dcache_coreid[core_id], p_lineaddr, TRUE, core_id);
    delay += DCACHE_HIT_LATENCY;

    if (is_hit == MISS) {
      uint64_t delay_l2 = memsys_L2_access_multicore(sys, p_lineaddr, FALSE, core_id);
      cache_install(sys->dcache_coreid[core_id], p_lineaddr, TRUE, core_id);
      delay += delay_l2;
      if (sys->dcache_coreid[core_id]->last_evicted.dirty && sys->dcache_coreid[core_id]->last_evicted.valid) {
        uint64_t n_idx_b = ceil(log(sys->dcache_coreid[core_id]->num_sets)/log(2));
        uint64_t mask = (1 << n_idx_b) - 1;
        uint64_t idx = (p_lineaddr & mask);
        uint64_t dest_address = (sys->dcache_coreid[core_id]->last_evicted.tag << n_idx_b) | idx;
        memsys_L2_access_multicore(sys, dest_address, TRUE, core_id);
      }
    }
  }
	return delay;
}


/////////////////////////////////////////////////////////////////////
// Used by Parts D,E to access the L2
/////////////////////////////////////////////////////////////////////

uint64_t memsys_L2_access_multicore(Memsys* sys, Addr v_lineaddr, bool is_write, uint32_t core_id){
	uint64_t delay = L2CACHE_HIT_LATENCY;

  if (cache_access(sys->l2cache, v_lineaddr, is_write, core_id) == HIT) return delay;
    uint64_t dram_delay = dram_access(sys->dram, v_lineaddr, FALSE);
    cache_install(sys->l2cache, v_lineaddr, is_write, core_id);
    delay += dram_delay;
    if (sys->l2cache->last_evicted.dirty && sys->l2cache->last_evicted.valid) {
      uint64_t numIdxBits = ceil(log(sys->l2cache->num_sets)/log(2));
      uint64_t idxMask = (1 << numIdxBits) - 1;
      uint64_t idx = (v_lineaddr & idxMask);
      uint64_t dest_address = (sys->l2cache->last_evicted.tag << numIdxBits) | idx;

      dram_access(sys->dram, dest_address, TRUE);
    }

	return delay;
}
