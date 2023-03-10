/***********************************************************************
 * File         : pipeline.cpp
 * Author       : Soham J. Desai
 * Date         : 14th January 2014
 * Description  : Superscalar Pipeline for Lab2 ECE 6100
 * 
 * Version 2.0 (Updates for 6-stage pipeline)
 * Author       : Geonhwa Jeong
 * Date         : 30th August 2022 
 * Description  : Superscalar Pipeline for Lab2 CS4290/CS6290/ECE4100/ECE6100 [Fall 2022]
 * 
 **********************************************************************/

#include "pipeline.h"
#include <cstdlib>

extern int32_t PIPE_WIDTH;
extern int32_t ENABLE_MEM_FWD;
extern int32_t ENABLE_EXE_FWD;
extern int32_t BPRED_POLICY;

bool VERBOSE = false;

/**********************************************************************
 * Support Function: Read 1 Trace Record From File and populate Fetch Op
 **********************************************************************/

void pipe_get_fetch_op(Pipeline *p, Pipeline_Latch* fetch_op){
  uint8_t bytes_read = 0;
  bytes_read = fread(&fetch_op->tr_entry, 1, sizeof(Trace_Rec), p->tr_file);

  // check for end of trace
  if(bytes_read < sizeof(Trace_Rec)) {
    fetch_op->valid=false;
    p->halt_op_id=p->op_id_tracker;
    return;
  }

  // got an instruction ... hooray!
  fetch_op->valid=true;
  fetch_op->stall=false;
  fetch_op->is_mispred_cbr=false;
  p->op_id_tracker++;
  fetch_op->op_id=p->op_id_tracker;
  
  return; 
}

bool is_longalu(uint64_t inst_addr, uint8_t op_type){
  if(op_type == OP_ALU){
    if(inst_addr & 1)
      return true;
  }

  return false;
}

// This fn is fine
bool has_raw_hazard(Pipeline_Latch reader, Pipeline_Latch writer) {
  if (!reader.valid || !writer.valid) {
    return false;
  }
  
  uint8_t reader_src1_reg = reader.tr_entry.src1_reg;
  uint8_t reader_src2_reg = reader.tr_entry.src2_reg;
  uint8_t reader_src1_needed = reader.tr_entry.src1_needed;
  uint8_t reader_src2_needed = reader.tr_entry.src2_needed;
  
  uint8_t writer_dest = writer.tr_entry.dest;
  uint8_t writer_dest_needed = writer.tr_entry.dest_needed;
  
  bool reg1_conflict = (reader_src1_needed && writer_dest_needed) && (reader_src1_reg == writer_dest);
  bool reg2_conflict = (reader_src2_needed && writer_dest_needed) && (reader_src2_reg == writer_dest);
  bool cc_hazard = reader.tr_entry.cc_read && writer.tr_entry.cc_write;
  bool has_hazard = reg1_conflict || reg2_conflict || cc_hazard;

  if (VERBOSE && has_hazard)
    printf("\n HAZARD between %u and %u\n", reader.op_id, writer.op_id);

  return has_hazard;
}

void sort_pipes(Pipeline* p){
  // Sorts the pipes by validity and then by op_id
  for (int i = 0; i < PIPE_WIDTH - 1; i++) {
    for (int j = 0; j < PIPE_WIDTH - i - 1; j++) {
      if (!p->pipe_latch[IF_LATCH][j].valid) {
        if (p->pipe_latch[IF_LATCH][j + 1].valid && !p->pipe_latch[IF_LATCH][j].valid) {
          Pipeline_Latch temp = p->pipe_latch[IF_LATCH][j];
          p->pipe_latch[IF_LATCH][j] = p->pipe_latch[IF_LATCH][j+1];
          p->pipe_latch[IF_LATCH][j+1] = temp;
        }
      } else {
        if ((p->pipe_latch[IF_LATCH][j].op_id > p->pipe_latch[IF_LATCH][j+1].op_id) && p->pipe_latch[IF_LATCH][j+1].valid) {
          Pipeline_Latch temp = p->pipe_latch[IF_LATCH][j];
          p->pipe_latch[IF_LATCH][j] = p->pipe_latch[IF_LATCH][j+1];
          p->pipe_latch[IF_LATCH][j+1] = temp;
        }
      }
    }
  }
}

void stall_if_younger_stalled(Pipeline* p, int ii){
  for (int j = 0; j < PIPE_WIDTH; j++) {
    if (p->pipe_latch[ID_LATCH][j].stall){
      p->pipe_latch[ID_LATCH][ii].stall = true;
      break;
    }
  }
}


/**********************************************************************
 * Pipeline Class Member Functions 
 **********************************************************************/

Pipeline * pipe_init(FILE *tr_file_in){
  printf("\n** PIPELINE IS %d WIDE **\n\n", PIPE_WIDTH);
  // Initialize Pipeline Internals
  Pipeline *p = (Pipeline *) calloc (1, sizeof (Pipeline));
  p->tr_file = tr_file_in;
  p->halt_op_id = ((uint64_t)-1) - 3; 

  // Allocated Branch Predictor
  p->fetch_cbr_stall = false;
	
	if(BPRED_POLICY != -1){
    p->b_pred = new BPRED(BPRED_POLICY);
    p->b_pred->stat_num_branches = 0;
    p->b_pred->stat_num_mispred = 0;
	}
    return p;
}


/**********************************************************************
 * Print the pipeline state (useful for debugging)
 **********************************************************************/

void pipe_print_state(Pipeline *p){
  std::cout << "--------------------------------------------" << std::endl;
  std::cout <<"cycle count : " << p->stat_num_cycle << " retired_instruction : " << p->stat_retired_inst << std::endl;

  uint8_t latch_type_i = 0;   // Iterates over Latch Types
  uint8_t width_i      = 0;   // Iterates over Pipeline Width
  for(latch_type_i = 0; latch_type_i < NUM_LATCH_TYPES; latch_type_i++) {
    switch(latch_type_i) {
      case 0:
          printf("\t IF: ");
          break;
      case 1:
          printf("\t ID: ");
          break;
      case 2:
          printf("\t EX1: ");
          break;
      case 3:
          printf("\t EX2: ");
          break;
      case 4:
          printf("\t MA: ");
          break;
      default:
          printf("\t ---- ");
    }
  }
  printf("\n");
  for(width_i = 0; width_i < PIPE_WIDTH; width_i++) {
      for(latch_type_i = 0; latch_type_i < NUM_LATCH_TYPES; latch_type_i++) {
        if(latch_type_i == 0)
          printf("\t");
          if(p->pipe_latch[latch_type_i][width_i].valid == true) {
            printf(" %6u ",(uint32_t)( p->pipe_latch[latch_type_i][width_i].op_id));
          } else {
              printf(" ------ ");
          }
      }
      if(VERBOSE){
        printf("\n");
        printf("op_type: %u ",(uint32_t)( p->pipe_latch[0][width_i].tr_entry.op_type));
        printf(",");
        printf("dest: %u ",(uint32_t)( p->pipe_latch[0][width_i].tr_entry.dest));
        printf(",");
        printf("dest_needed: %u ",(uint32_t)( p->pipe_latch[0][width_i].tr_entry.dest_needed));
        printf("<-");
        printf("src1: %u ",(uint32_t)( p->pipe_latch[0][width_i].tr_entry.src1_reg));
        printf(",");
        printf("src2: %u ",(uint32_t)( p->pipe_latch[0][width_i].tr_entry.src2_reg));
        printf(",");
        printf("src1_needed %u ",(uint32_t)( p->pipe_latch[0][width_i].tr_entry.src1_needed));
        printf(",");
        printf("src2_needed %u ",(uint32_t)( p->pipe_latch[0][width_i].tr_entry.src2_needed));
        printf(",");
        printf("cc_read: %u ",(uint32_t)( p->pipe_latch[0][width_i].tr_entry.cc_read));
        printf(",");
        printf("cc_write %u ",(uint32_t)( p->pipe_latch[0][width_i].tr_entry.cc_write));

        if(p->pipe_latch[0][width_i].tr_entry.op_type == 0){
          printf(" %u", p->pipe_latch[0][width_i].tr_entry.inst_addr);
          if(is_longalu(p->pipe_latch[0][width_i].tr_entry.inst_addr, p->pipe_latch[0][width_i].tr_entry.op_type)){
              printf(" Long ALU");
          }
          else
            printf(" ALU");
        }
        printf("\n");
      }
  }
  printf("\n");
}


/**********************************************************************
 * Pipeline Main Function: Every cycle, cycle the stage 
 **********************************************************************/

void pipe_cycle(Pipeline *p)
{
  if(VERBOSE)
    pipe_print_state(p);

  p->stat_num_cycle++;

  pipe_cycle_WB(p);
  pipe_cycle_MA(p);
  pipe_cycle_EX2(p);
  pipe_cycle_EX1(p);
  pipe_cycle_ID(p);
  pipe_cycle_IF(p);

  if(VERBOSE){
    if(p->stat_num_cycle == 1000000)
      exit(-1);
  }
}
/**********************************************************************
 * -----------  DO NOT MODIFY THE CODE ABOVE THIS LINE ----------------
 **********************************************************************/

void pipe_cycle_WB(Pipeline *p){
  // TODO: UPDATE HERE (Part B)
  int ii;
  for(ii=0; ii<PIPE_WIDTH; ii++){
	  if(p->pipe_latch[MA_LATCH][ii].valid){
      p->stat_retired_inst++;
      if(p->pipe_latch[MA_LATCH][ii].op_id >= p->halt_op_id){
		    p->halt=true;
      }
      if (p->pipe_latch[MA_LATCH][ii].is_mispred_cbr) {
        p->fetch_cbr_stall = false;
      }
    }
  }
}

//--------------------------------------------------------------------//

void pipe_cycle_MA(Pipeline *p){
  int ii;
  for(ii=0; ii<PIPE_WIDTH; ii++){
	  p->pipe_latch[MA_LATCH][ii]=p->pipe_latch[EX2_LATCH][ii];
  }
}

//--------------------------------------------------------------------//

void pipe_cycle_EX2(Pipeline *p){
  int ii;
  for(ii=0; ii<PIPE_WIDTH; ii++){
    p->pipe_latch[EX2_LATCH][ii]=p->pipe_latch[EX1_LATCH][ii];
  }
}

//--------------------------------------------------------------------//

void pipe_cycle_EX1(Pipeline *p){
  int ii;
  for(ii=0; ii<PIPE_WIDTH; ii++){
    p->pipe_latch[EX1_LATCH][ii]=p->pipe_latch[ID_LATCH][ii];
  }
}

//--------------------------------------------------------------------//

void pipe_cycle_ID(Pipeline *p){
  // TODO: UPDATE HERE (Part A, B)
  int ii;
  sort_pipes(p);

  //initialize everything to false before setting stalls
  for (ii = 0; ii < PIPE_WIDTH; ii++)
    p->pipe_latch[ID_LATCH][ii].stall = false;

  for(ii=0; ii<PIPE_WIDTH; ii++){
    stall_if_younger_stalled(p, ii);
    
    Pipeline_Latch if_latch = p->pipe_latch[IF_LATCH][ii];

    for (int j = 0; j < PIPE_WIDTH; j++) {
      Pipeline_Latch id_latch = p->pipe_latch[ID_LATCH][j];
      Pipeline_Latch ex1_latch = p->pipe_latch[EX1_LATCH][j];
      Pipeline_Latch ex2_latch = p->pipe_latch[EX2_LATCH][j];
      Pipeline_Latch ma_latch = p->pipe_latch[MA_LATCH][j];

      bool is_ex1_fwdable = false;
      bool is_ex2_fwdable = false;
      bool is_id_fwdable_at_ex1 = false;

      if (ENABLE_EXE_FWD) {
        bool is_id_longalu = is_longalu(id_latch.tr_entry.inst_addr, id_latch.tr_entry.op_type);
        bool is_ex1_longalu = is_longalu(ex1_latch.tr_entry.inst_addr, ex1_latch.tr_entry.op_type);
        
        bool is_id_ld = id_latch.tr_entry.op_type == OP_LD;
        bool is_ex1_ld = ex1_latch.tr_entry.op_type == OP_LD;
        bool is_ex2_ld = ex2_latch.tr_entry.op_type == OP_LD;
        
        bool is_ex1_ex2_same_dest = ex1_latch.tr_entry.dest == ex2_latch.tr_entry.dest;
        
        is_id_fwdable_at_ex1 = !is_id_longalu && !is_id_ld;
        is_ex1_fwdable = !is_ex1_longalu && !is_ex1_ld;
        is_ex2_fwdable = !is_ex2_ld || is_ex1_ex2_same_dest;
      }
      
      bool is_ma_fwdable = false;
      if (ENABLE_MEM_FWD) {
        is_ma_fwdable = true;
      }

      if ((has_raw_hazard(if_latch, id_latch) && !is_id_fwdable_at_ex1)
        || (has_raw_hazard(if_latch, ex1_latch) && !is_ex1_fwdable)
        || (has_raw_hazard(if_latch, ex2_latch) && !is_ex2_fwdable)
        || (has_raw_hazard(if_latch, ma_latch) && !is_ma_fwdable)) {
          p->pipe_latch[ID_LATCH][ii].stall = true;
        }
    }
    if (!p->pipe_latch[IF_LATCH][ii].valid && p->fetch_cbr_stall) {
      p->pipe_latch[ID_LATCH][ii].stall = true;
    }

    if (p->pipe_latch[ID_LATCH][ii].stall) { 
      p->pipe_latch[ID_LATCH][ii].valid = false;
    }
    else { // not stalling, move inst along
      p->pipe_latch[ID_LATCH][ii] = p->pipe_latch[IF_LATCH][ii];
    }
  }
}	

//--------------------------------------------------------------------//

void pipe_cycle_IF(Pipeline *p){
  // TODO: UPDATE HERE (Part A, B)
  int ii,jj;
  Pipeline_Latch fetch_op;
  bool tr_read_success;
  

  for(ii=0; ii<PIPE_WIDTH; ii++){
    bool is_next_stalled = p->pipe_latch[ID_LATCH][ii].stall;
    
    if (!p->fetch_cbr_stall)
      p->pipe_latch[IF_LATCH][ii].valid = true;
    else
      p->pipe_latch[IF_LATCH][ii].valid &= is_next_stalled;

    if (is_next_stalled || p->fetch_cbr_stall) {
      continue;
    }
    pipe_get_fetch_op(p, &fetch_op);
    
    if(BPRED_POLICY != -1){
      pipe_check_bpred(p, &fetch_op);
    }
    p->pipe_latch[IF_LATCH][ii]=fetch_op;
  }
}


//--------------------------------------------------------------------//

void pipe_check_bpred(Pipeline *p, Pipeline_Latch *fetch_op){
  // call branch predictor here, if mispred then mark in fetch_op
  // update the predictor instantly
  // stall fetch using the flag p->fetch_cbr_stall

  // TODO: UPDATE HERE (Part B)
  if (fetch_op->tr_entry.op_type != OP_CBR) return;
  p->b_pred->stat_num_branches++;
  if (BPRED_POLICY == BPRED_PERFECT) return;

  bool pred = p->b_pred->GetPrediction(fetch_op->tr_entry.inst_addr);
  p->b_pred->UpdatePredictor(fetch_op->tr_entry.inst_addr, fetch_op->tr_entry.br_dir, pred);

  if (pred == fetch_op->tr_entry.br_dir) return; // predicted right, woo!

  if (VERBOSE) printf("\n Misprediction on: , pred: %d.\n", fetch_op->op_id, pred);

  p->b_pred->stat_num_mispred++;
  p->fetch_cbr_stall = true;
  fetch_op->is_mispred_cbr = true;
}


//--------------------------------------------------------------------//
