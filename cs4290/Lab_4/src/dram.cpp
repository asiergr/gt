#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "dram.h"

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

}

//////////////////////////////////////////////////////////////////////////////
// You may update the statistics here, and also call dram_access_mode_CDE()
//////////////////////////////////////////////////////////////////////////////

uint64_t dram_access(DRAM* dram, Addr lineaddr, bool is_dram_write) {

}

//////////////////////////////////////////////////////////////////////////////
// Modify the function below only for Parts C,D,E
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Assume a mapping with consecutive cache lines in consecutive DRAM banks
// Assume a mapping with consecutive rowbufs in consecutive DRAM rows
//////////////////////////////////////////////////////////////////////////////

uint64_t dram_access_mode_CDE(DRAM* dram, Addr lineaddr, bool is_dram_write) {

}

