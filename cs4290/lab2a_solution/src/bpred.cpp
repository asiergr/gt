#include "bpred.h"

#define TAKEN   true
#define NOTTAKEN false

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////

BPRED::BPRED(uint32_t policy) {
  // TODO: UPDATE HERE (Part B)  
  BPRED_TYPE policies[NUM_BPRED_TYPE] = {BPRED_PERFECT, BPRED_ALWAYS_TAKEN,
     BPRED_GSELECT, BPRED_GSHARE};

  this->policy = policies[policy];
  this->stat_num_branches = 0;
  this->stat_num_mispred = 0;

  for (size_t i = 0; i < 16384; i++) {
    this->pattern_hist_table[i] = 2; // init all to weakly taken
  }
}

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////

bool BPRED::GetPrediction(uint32_t PC){
  // TODO: UPDATE HERE (Part B)
  if (this->policy != BPRED_GSELECT) {
    return TAKEN;
  }
  // GSELECT logic
  
}


/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////

void  BPRED::UpdatePredictor(uint32_t PC, bool resolveDir, bool predDir) {
  // TODO: UPDATE HERE (Part B)
  if (this->policy != BPRED_GSELECT) return;
  
  // GSELECT LOGIC
}

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////

