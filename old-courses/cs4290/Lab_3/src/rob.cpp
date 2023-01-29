#include <stdio.h>
#include <assert.h>

#include "rob.h"


extern int32_t NUM_ROB_ENTRIES;

/////////////////////////////////////////////////////////////
// Init function initializes the ROB
/////////////////////////////////////////////////////////////

ROB* ROB_init(void){
  int ii;
  ROB *t = (ROB *) calloc (1, sizeof (ROB));
  for(ii=0; ii<MAX_ROB_ENTRIES; ii++){
    t->ROB_Entries[ii].valid=false;
    t->ROB_Entries[ii].ready=false;
    t->ROB_Entries[ii].exec=false;
  }
  t->head_ptr=0;
  t->tail_ptr=0;
  return t;
}

/////////////////////////////////////////////////////////////
// Print State
/////////////////////////////////////////////////////////////
void ROB_print_state(ROB *t){
 int ii = 0;
  printf("Printing ROB \n");
  printf("Entry  Inst   Valid   Ready Exec\n");
  for(ii = 0; ii < NUM_ROB_ENTRIES; ii++) {
    printf("%5d ::  %d\t", ii, (int)t->ROB_Entries[ii].inst.inst_num);
    printf(" %5d\t", t->ROB_Entries[ii].valid);
    printf(" %5d\n", t->ROB_Entries[ii].ready);
    printf(" %5d\n", t->ROB_Entries[ii].exec);
  }
  printf("\n");
}

/////////////////////////////////////////////////////////////
// If there is space in ROB return true, else false
/////////////////////////////////////////////////////////////

bool ROB_check_space(ROB *t){
  return !(t->ROB_Entries[t->tail_ptr].valid);
}

/////////////////////////////////////////////////////////////
// insert entry at tail, increment tail (do check_space first)
/////////////////////////////////////////////////////////////

int ROB_insert(ROB *t, Inst_Info inst){
  if (!ROB_check_space(t)) return -1;

  int prev_tail = t->tail_ptr;
  t->ROB_Entries[prev_tail].inst = inst;
  t->ROB_Entries[prev_tail].valid = true;

  t->tail_ptr = (t->tail_ptr + 1) % (NUM_ROB_ENTRIES - 1);
  return prev_tail;
}

/////////////////////////////////////////////////////////////
// When an inst gets scheduled for execution, mark exec
/////////////////////////////////////////////////////////////

void ROB_mark_exec(ROB *t, Inst_Info inst){
  for (int i = 0; i < NUM_ROB_ENTRIES; i++) {
    if (t->ROB_Entries[i].valid && (t->ROB_Entries[i].inst.inst_num == inst.inst_num)) {
      t->ROB_Entries[i].exec = t->ROB_Entries[i].inst.src1_ready && t->ROB_Entries[i].inst.src2_ready;
      break;
    }
  }
}


/////////////////////////////////////////////////////////////
// Once an instruction finishes execution, mark rob entry as done
/////////////////////////////////////////////////////////////

void ROB_mark_ready(ROB *t, Inst_Info inst){
  for (int i = 0; i < NUM_ROB_ENTRIES; i++) {
    if (t->ROB_Entries[i].valid && (t->ROB_Entries[i].inst.inst_num == inst.inst_num)) {
      t->ROB_Entries[i].ready = true;
      break;
    }
  }
}

/////////////////////////////////////////////////////////////
// Find whether the prf (rob entry) is ready
/////////////////////////////////////////////////////////////

bool ROB_check_ready(ROB *t, int tag){
  return t->ROB_Entries[tag].ready && t->ROB_Entries[tag].valid;
}


/////////////////////////////////////////////////////////////
// Check if the oldest ROB entry is ready for commit
/////////////////////////////////////////////////////////////

bool ROB_check_head(ROB *t){
  return ROB_check_ready(t, t->head_ptr);
}

/////////////////////////////////////////////////////////////
// For writeback of freshly ready tags, wakeup waiting inst
/////////////////////////////////////////////////////////////

void  ROB_wakeup(ROB *t, int tag){
  for (int i = 0; i < NUM_ROB_ENTRIES; i++) {
    if (!t->ROB_Entries[i].valid) continue;

    if (t->ROB_Entries[i].inst.src1_tag == tag) {
      t->ROB_Entries[i].inst.src1_tag = -1; // -1 means ready
      t->ROB_Entries[i].inst.src1_ready = true;
    }
    if (t->ROB_Entries[i].inst.src2_tag == tag) {
      t->ROB_Entries[i].inst.src2_tag = -1;
      t->ROB_Entries[i].inst.src2_ready = true;
    }
  }
}


/////////////////////////////////////////////////////////////
// Remove oldest entry from ROB (after ROB_check_head)
/////////////////////////////////////////////////////////////

Inst_Info ROB_remove_head(ROB *t){
  if (!ROB_check_head(t)) return {.inst_num = (uint64_t) -1};

  Inst_Info ret_val = t->ROB_Entries[t->head_ptr].inst;
  t->ROB_Entries[t->head_ptr].valid = false;
  t->head_ptr = (t->head_ptr + 1) % (NUM_ROB_ENTRIES - 1);
  return ret_val;

}

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
