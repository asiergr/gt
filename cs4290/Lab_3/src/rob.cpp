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
    printf(" %5d\t", t->ROB_Entries[ii].ready);
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

  int curr_tail = t->tail_ptr;
  t->ROB_Entries[curr_tail].valid = true;
  t->ROB_Entries[curr_tail].inst = inst;

  t->tail_ptr = (curr_tail + 1) % (MAX_ROB_ENTRIES - 1);
  return curr_tail;
}

/////////////////////////////////////////////////////////////
// When an inst gets scheduled for execution, mark exec
/////////////////////////////////////////////////////////////

void ROB_mark_exec(ROB *t, Inst_Info inst){
  for (int ii = 0; ii < MAX_ROB_ENTRIES; ii++) {
    ROB_Entry entry = t->ROB_Entries[ii];
    if (entry.inst.inst_num == inst.inst_num && entry.valid) {
      entry.exec = entry.inst.src1_ready && entry.inst.src2_ready;
      return;
    }
  }

}


/////////////////////////////////////////////////////////////
// Once an instruction finishes execution, mark rob entry as done
/////////////////////////////////////////////////////////////

void ROB_mark_ready(ROB *t, Inst_Info inst){
  for (int ii = 0; ii < MAX_ROB_ENTRIES; ii++) {
    ROB_Entry entry = t->ROB_Entries[ii];
    if (entry.inst.inst_num == inst.inst_num && entry.valid) {
      entry.ready = true;
      return;
    }
  }
}

/////////////////////////////////////////////////////////////
// Find whether the prf (rob entry) is ready
/////////////////////////////////////////////////////////////

bool ROB_check_ready(ROB *t, int tag){
  ROB_Entry entry = t->ROB_Entries[tag];
  return entry.valid && entry.ready;
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
  for (int ii = 0; ii < MAX_ROB_ENTRIES; ii++) {
    ROB_Entry entry = t->ROB_Entries[ii];
    if (!entry.valid) continue;
    
    Inst_Info inst = entry.inst;
    if (inst.src1_tag == tag) {
      inst.src1_ready = true;
      inst.src1_tag = -1;
    }
    if (inst.src2_tag == tag) {
      inst.src2_ready = true;
      inst.src2_tag = -1;
    }
  }
}


/////////////////////////////////////////////////////////////
// Remove oldest entry from ROB (after ROB_check_head)
/////////////////////////////////////////////////////////////

Inst_Info ROB_remove_head(ROB *t){
  // check if it's valid
  if (!ROB_check_head(t)) return {.inst_num=(uint64_t) -1};
  ROB_Entry entry = t->ROB_Entries[t->head_ptr];
  entry.valid = false;
  t->head_ptr = (t->head_ptr + 1) % (NUM_ROB_ENTRIES - 1);
  return entry.inst;
}

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////
// Invalidate all entries in ROB
/////////////////////////////////////////////////////////////

void ROB_flush(ROB *t){
  for(int ii=0; ii<MAX_ROB_ENTRIES; ii++){
    t->ROB_Entries[ii].valid=false;
    t->ROB_Entries[ii].ready=false;
    t->ROB_Entries[ii].exec=false;
  }
  t->head_ptr=0;
  t->tail_ptr=0;
}

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////