/***********************************************************************
 * File         : pipeline.cpp
 * Author       : Soham J. Desai
 * Date         : 14th January 2014
 * Description  : Superscalar Pipeline for Lab2 ECE 6100
 * 
 * Version 2.0 (Updates for solution to Part A)
 * Author       : Kartikay Garg
 * Date         : 1st September 2016
 * Description  : Superscalar Pipeline for Lab2 ECE 6100 [Fall 2016]
 * 
 * Version 3.0 (A "cleaner" solution to Part A)
 * Author       : Marina Vemmou
 * Date         : 10th September 2020 
 * Description  : Superscalar Pipeline for Lab2 CS4290/CS6290/ECE4100/ECE6100 [Fall 2020]
 * 
 * Version 4.0 (Updates for 6-stage pipeline)
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

bool VERBOSE = true;

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

// Use this function to check whether the operation is long alu or not.
bool is_longalu(uint64_t inst_addr, uint8_t op_type){
  if(op_type == OP_ALU){
    if(inst_addr & 1)
      return true;
  }

  return false;
}

/**********************************************************************
 * Pipeline Class Member Functions 
 **********************************************************************/

Pipeline * pipe_init(FILE *tr_file_in){
  printf("\n** PIPELINE IS %d WIDE **\n\n", PIPE_WIDTH);
  // Initialize pipeline internals.
  Pipeline *p = (Pipeline *) calloc (1, sizeof (Pipeline));
  p->tr_file = tr_file_in;
  p->halt_op_id = ((uint64_t)-1) - 3; 

  // Allocated branch predictor.
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
          printf("IF: ");
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
        if(p->pipe_latch[latch_type_i][width_i].valid == true) {
          printf(" %6u ",(uint32_t)( p->pipe_latch[latch_type_i][width_i].op_id));
        } else {
            printf(" ------ ");
        }
      }
      if(VERBOSE){
        printf("\n");
        printf(" %u ",(uint32_t)( p->pipe_latch[0][width_i].tr_entry.op_type));
        printf(",");
        printf(" %u ",(uint32_t)( p->pipe_latch[0][width_i].tr_entry.dest));
        printf(",");
        printf(" %u ",(uint32_t)( p->pipe_latch[0][width_i].tr_entry.dest_needed));
        printf("<-");
        printf(" %u ",(uint32_t)( p->pipe_latch[0][width_i].tr_entry.src1_reg));
        printf(",");
        printf(" %u ",(uint32_t)( p->pipe_latch[0][width_i].tr_entry.src2_reg));
        printf(",");
        printf(" %u ",(uint32_t)( p->pipe_latch[0][width_i].tr_entry.src1_needed));
        printf(",");
        printf(" %u ",(uint32_t)( p->pipe_latch[0][width_i].tr_entry.src2_needed));
        printf(",");
        printf(" %u ",(uint32_t)( p->pipe_latch[0][width_i].tr_entry.cc_read));
        printf(",");
        printf(" %u ",(uint32_t)( p->pipe_latch[0][width_i].tr_entry.cc_write));

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
typedef struct dependence_tracker{
	uint8_t op_type;
	uint64_t op_id;
	uint8_t stage;
  bool is_longalu; 
	void init();
} Depend_track;

void Depend_track::init(){
	this->op_type=NUM_OP_TYPE+1;
	this->op_id=0;
	this->stage=0;
  bool is_longalu=0; 
}

void pipe_cycle_ID(Pipeline *p){
  // TODO: UPDATE HERE (Part A, B)
  int ii,jj;

  /**************************************************/
  /* The dep_track structure is used to track three kinds of RAW hazards:
  * You can use this structure if you want. 
  * dep_track[][0]: cc_write-cc_read dependency
  * dep_track[][1]: dst-src1 dependency
  * dep_track[][2]: dst-src2 dependency */
  /**************************************************/

  Depend_track dep_track[PIPE_WIDTH][3];

  // Reset ID's state - initially we assume no stalls 
  for(ii=0; ii<PIPE_WIDTH; ii++){
    p->pipe_latch[ID_LATCH][ii]=p->pipe_latch[IF_LATCH][ii];
    p->pipe_latch[IF_LATCH][ii].stall = 0;
    for(int i=0;i<3;i++)
      dep_track[ii][i].init();
  }

  for(ii=0; ii<PIPE_WIDTH; ii++){ 	

    Pipeline_Latch id_latch = p->pipe_latch[ID_LATCH][ii];
    Trace_Rec id_entry = id_latch.tr_entry;

    if((id_latch.valid)){
      for (jj=0;jj<PIPE_WIDTH;++jj){
        for (int stage=ID_LATCH;stage<NUM_LATCH_TYPES;stage++) {
          Pipeline_Latch test_latch = p->pipe_latch[stage][jj];
          Trace_Rec test_entry = test_latch.tr_entry;
          
          /**************************************************/
          /* Check for all the types of RAW hazards
           * The (test_latch.op_id > dep_track[][].op_id) check is used to make sure we always keep the "youngest" instruction
           * with a dependency of that type
           * For example:
           * i1: add r1,r2,r2
           * i2: add r1,r3,r3
           * i3: add r5,r1,r1
           * In that case we want the i3<->i2 dependency to be tracked, not the i3<->i1 */
          /**************************************************/

          if (test_latch.valid && id_latch.op_id > test_latch.op_id)
          {
            if (test_entry.cc_write && id_entry.cc_read && test_latch.op_id > dep_track[ii][0].op_id) {
              dep_track[ii][0].op_id = test_latch.op_id;
              dep_track[ii][0].op_type = test_entry.op_type;
              dep_track[ii][0].stage = stage; 
              if(test_entry.op_type == 0)
                dep_track[ii][0].is_longalu = is_longalu(test_entry.inst_addr, test_entry.op_type);
            }
            if (test_entry.dest_needed) {
              if (id_entry.src1_needed && id_entry.src1_reg == test_entry.dest && test_latch.op_id > dep_track[ii][1].op_id) {
                dep_track[ii][1].op_id = test_latch.op_id;
                dep_track[ii][1].op_type = test_entry.op_type;
                dep_track[ii][1].stage = stage;
                if(test_entry.op_type == 0)
                  dep_track[ii][1].is_longalu = is_longalu(test_entry.inst_addr, test_entry.op_type);
              }
              if (id_entry.src2_needed && id_entry.src2_reg == test_entry.dest && test_latch.op_id > dep_track[ii][2].op_id) {
                dep_track[ii][2].op_id = test_latch.op_id;
                dep_track[ii][2].op_type = test_entry.op_type;
                dep_track[ii][2].stage = stage;   
                if(test_entry.op_type == 0)
                  dep_track[ii][2].is_longalu = is_longalu(test_entry.inst_addr, test_entry.op_type);
              }
            }
          }
        }
      }
      
      /**************************************************/
      /* The stall_vector:
       * We use a bit vector to track the latches (ID/EX1/EX2/MA)
       * in which there are instructions with which we have a dependency.
       * There is one bit for each latch, and it is set to 1 if there is a dependency with an instruction in this latch
       * stall_vector[0]: ID
       * stall_vector[1]: EX1
       * stall_vector[2]: EX2
       * stall_vector[3]: MA
       * You can use this structure if you want */
      /**************************************************/

      bool stall_vector[4];
      for (int i=0;i<4;i++)
        stall_vector[i] = 0;

      for(int stage=ID_LATCH; stage<NUM_LATCH_TYPES; stage++) {
        // If we have at least one tracked dependency related to this stage 
        if(dep_track[ii][0].stage == stage || dep_track[ii][1].stage == stage || dep_track[ii][2].stage == stage) 
          stall_vector[stage-1] = 1;
      }

      /**************************************************/
      /* When we use forwarding, all the stalls due to instructions in the MA stage can be removed,
       * as all types of intructions that create RAW hazards have their results ready by MA. */
      /**************************************************/

      if(ENABLE_MEM_FWD){	
          stall_vector[3] = 0;
      }

      /**************************************************/
      /* Similarly, when we use forwarding, all the stalls due to non-load instructions in the EX stage can be removed,
       * as all non-load intructions that create RAW hazards have their results ready by EX.
       * The only case we want to maintain a stall due to a dependency with EX is when the instruction 
       * in EX is a load: loads have their results ready by MA, so we need to stall to get them. */
      /**************************************************/

      if(ENABLE_EXE_FWD){	
        stall_vector[2] = ((dep_track[ii][0].stage == EX2_LATCH && dep_track[ii][0].op_type == OP_LD) || 
                          (dep_track[ii][1].stage == EX2_LATCH && dep_track[ii][1].op_type == OP_LD) ||
                          (dep_track[ii][2].stage == EX2_LATCH && dep_track[ii][2].op_type == OP_LD));

        stall_vector[1] = ((dep_track[ii][0].stage == EX1_LATCH && dep_track[ii][0].op_type == OP_LD) || 
                          (dep_track[ii][1].stage == EX1_LATCH && dep_track[ii][1].op_type == OP_LD) ||
                          (dep_track[ii][2].stage == EX1_LATCH && dep_track[ii][2].op_type == OP_LD)) ||
                          (dep_track[ii][0].stage == EX1_LATCH && dep_track[ii][0].op_type == OP_ALU && dep_track[ii][0].is_longalu) ||
                          (dep_track[ii][1].stage == EX1_LATCH && dep_track[ii][1].op_type == OP_ALU && dep_track[ii][1].is_longalu) ||
                          (dep_track[ii][2].stage == EX1_LATCH && dep_track[ii][2].op_type == OP_ALU && dep_track[ii][2].is_longalu);

        // stall_vector[1] should be set if dep_track[ii][2].stage == EX_LATCH and it's long ALU
      }


      // if there is still a reason to stall, set stall signal in IF latch and send NOPs down te pipeline 
      if(stall_vector[0] || stall_vector[1] || stall_vector[2] || stall_vector[3]) {
        if(VERBOSE)
          std::cout << "stall_vector" << stall_vector[0] << stall_vector[1] << stall_vector[2] << stall_vector[3] << std::endl;
        p->pipe_latch[IF_LATCH][ii].stall = 1;
        p->pipe_latch[ID_LATCH][ii].valid = 0;    
      }
	  } else if (p->fetch_cbr_stall) {
      // id_latch is not valid and we mispredicted. Stall.
      printf("\n latch in ID not valid, and fetch_cbr_stall.\n");
      p->pipe_latch[IF_LATCH][ii].stall = 1;
    }
  }	
  

  /**************************************************/
  /* If there is an instruction stalled in the IF latch due to data dependencies,
   * we need to maintain the in-order nature of the pipeline.
   * So we stall all instructions with an op_id higher than the stalled instruction
   * to make sure that all instructions are executed and completed in-order. */
  /**************************************************/

	for(ii=0;ii<PIPE_WIDTH;++ii){
		if(p->pipe_latch[IF_LATCH][ii].stall){
			for(jj=0;jj<PIPE_WIDTH;++jj){
				if((p->pipe_latch[ID_LATCH][jj].op_id > p->pipe_latch[ID_LATCH][ii].op_id) ){
					p->pipe_latch[IF_LATCH][jj].stall = 1;
          p->pipe_latch[ID_LATCH][jj].valid = 0;
				}
			}
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
    
    if (p->fetch_cbr_stall) {
      p->pipe_latch[IF_LATCH][ii].valid &= p->pipe_latch[ID_LATCH][ii].stall;
    } else {
      p->pipe_latch[IF_LATCH][ii].valid = true;
    }

    if(!p->pipe_latch[IF_LATCH][ii].stall && !p->fetch_cbr_stall){
      //We don't stall and don't have a mispredict
      pipe_get_fetch_op(p, &fetch_op);
      
      if(BPRED_POLICY != -1){
        pipe_check_bpred(p, &fetch_op);
      }
      p->pipe_latch[IF_LATCH][ii]=fetch_op;
    } 
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
  bool pred = p->b_pred->GetPrediction(fetch_op->tr_entry.inst_addr);

  if (pred != fetch_op->tr_entry.br_dir) {
    printf("\n Misprediction on, pred: %d.\n", pred);
    p->b_pred->stat_num_mispred++;
    p->fetch_cbr_stall = true;
    fetch_op->is_mispred_cbr = true;
  }
  p->b_pred->UpdatePredictor(fetch_op->tr_entry.inst_addr, pred, fetch_op->tr_entry.br_dir);

}


//--------------------------------------------------------------------//

