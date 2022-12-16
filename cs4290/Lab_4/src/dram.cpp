#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "dram.h"
#include "types.h"

#define ROW_SIZE 1024
#define NUM_BANKS 16

#define DRAM_FIXED_L 100
#define DRAM_ACT_L  45
#define DRAM_CAS_L  45
#define DRAM_PRE_L  45
#define DRAM_BUS_L  10

extern MODE SIM_MODE;
extern uint64_t CACHE_LINESIZE;
extern bool DRAM_PAGE_POLICY;

////////////////////////////////////////////////////////////////////
// ------------- DO NOT MODIFY THE PRINT STATS FUNCTION ------------
////////////////////////////////////////////////////////////////////

void dram_print_stats(DRAM* dram){
	double rddelay_avg=0, wrdelay_avg=0;
	char header[256];
	sprintf(header, "DRAM");

	if (dram->stat_read_access) {
		rddelay_avg = (double)(dram->stat_read_delay) / (double)(dram->stat_read_access);
	}

	if (dram->stat_write_access) {
		wrdelay_avg = (double)(dram->stat_write_delay) / (double)(dram->stat_write_access);
	}

	printf("\n%s_READ_ACCESS\t\t : %10llu", header, dram->stat_read_access);
	printf("\n%s_WRITE_ACCESS\t\t : %10llu", header, dram->stat_write_access);
	printf("\n%s_READ_DELAY_AVG\t\t : %10.3f", header, rddelay_avg);
	printf("\n%s_WRITE_DELAY_AVG\t\t : %10.3f", header, wrdelay_avg);

}

//////////////////////////////////////////////////////////////////////////////
// Allocate memory to the data structures and initialize the required fields
//////////////////////////////////////////////////////////////////////////////

DRAM* dram_new() {
  DRAM *d = (DRAM *) calloc(1, sizeof(DRAM));
  return d;
}

//////////////////////////////////////////////////////////////////////////////
// You may update the statistics here, and also call dram_access_mode_CDE()
//////////////////////////////////////////////////////////////////////////////

uint64_t dram_access(DRAM* dram, Addr lineaddr, bool is_dram_write) {
 uint64_t delay;
  if (SIM_MODE != SIM_MODE_B) {
    delay = dram_access_mode_CDE(dram, lineaddr, is_dram_write);
  } else {
    delay = DRAM_FIXED_L;
  }

  if (is_dram_write) {
    dram->stat_write_delay += delay;
    dram->stat_write_access++;
  } else {
    dram->stat_read_delay += delay;
    dram->stat_read_access++;
  }
  return delay; 
}

//////////////////////////////////////////////////////////////////////////////
// Modify the function below only for Parts C,D,E
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Assume a mapping with consecutive cache lines in consecutive DRAM banks
// Assume a mapping with consecutive rowbufs in consecutive DRAM rows
//////////////////////////////////////////////////////////////////////////////

uint64_t dram_access_mode_CDE(DRAM* dram, Addr lineaddr, bool is_dram_write) {
  uint64_t bank_idx = (lineaddr / (ROW_SIZE / CACHE_LINESIZE)) % NUM_BANKS;
  uint64_t row_idx = lineaddr / (NUM_BANKS * ROW_SIZE / CACHE_LINESIZE);

  uint64_t delay = 0;

  // Open Page
  if (DRAM_PAGE_POLICY == 0) {
    // If row buffer empty: need to activate row, select col, put data on bus
    if (!dram->banks[bank_idx].valid) {
      delay += DRAM_ACT_L + DRAM_CAS_L + DRAM_BUS_L;
      dram->banks[bank_idx].valid = TRUE;
      dram->banks[bank_idx].row_id = row_idx;
    }
    // If row buffer hit: select column, put data on bus
    else if (dram->banks[bank_idx].row_id == row_idx) {
      delay += DRAM_CAS_L + DRAM_BUS_L;
    }
    // If row buffer miss: precharge, activate row, select column, put data on bus
    else {
      delay += DRAM_PRE_L + DRAM_ACT_L + DRAM_CAS_L + DRAM_BUS_L;
      dram->banks[bank_idx].valid = TRUE;
      dram->banks[bank_idx].row_id = row_idx;
    }
  } else { // Closed Pg
    delay += DRAM_ACT_L + DRAM_CAS_L + DRAM_BUS_L;
    dram->banks[bank_idx].valid = FALSE;
    dram->banks[bank_idx].row_id = row_idx;
  } 
  
  return delay;
}

