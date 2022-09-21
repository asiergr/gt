#include "bpred.h"

#define TAKEN   true
#define NOTTAKEN false

#define SEVEN_BIT_MASK (1 << 7) - 1

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////

BPRED::BPRED(uint32_t policy) {
  // TODO: UPDATE HERE (Part B)  
  this->policy = (BPRED_TYPE) policy;
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
  if (this->policy != BPRED_GSELECT) return TAKEN;

  // GSELECT LOGIC
  uint32_t idx = ((PC & SEVEN_BIT_MASK) << 7) | (global_hist_table & SEVEN_BIT_MASK);
  uint32_t pred = pattern_hist_table[idx];
  return pred & 2? TAKEN: NOTTAKEN;
}


/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////

void  BPRED::UpdatePredictor(uint32_t PC, bool resolveDir, bool predDir) {
  // TODO: UPDATE HERE (Part B)
  if (this->policy != BPRED_GSELECT) return;
  // GSELECT LOGIC
  uint32_t idx = ((PC & SEVEN_BIT_MASK) << 7) | (global_hist_table & SEVEN_BIT_MASK);
  global_hist_table = (global_hist_table << 1) | resolveDir;
  if (resolveDir == NOTTAKEN) {
    pattern_hist_table[idx] = SatDecrement(pattern_hist_table[idx]);
  } else {
    pattern_hist_table[idx] = SatIncrement(pattern_hist_table[idx], 3);
  }
}

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////

