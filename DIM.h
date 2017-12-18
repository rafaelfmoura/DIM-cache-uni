#ifndef DIM_H
#define DIM_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "memory_interface.h"
#include "array.h"
#include "configuration.h"
#include "types.h"
#include "sparcv8_isa.h"
#include "results.h"
#include "micro_instruction.h"

//#define PRINT_DEBUG_DIM

//Return values form decode_instruction function
#define NORMAL_INSTRUCTION 0
#define BARRIER_INSTRUCTION 1
#define NOT_INSTRUCTION 2

//FSM' STATES
#define SIMPLE_INSTRUCTION_STATE 1
#define PREVIOUS_WAS_BRANCH_INSTRUCTION_STATE 2
#define PREVIOUS_WAS_BRANCH_DELAY_SLOT_INSTRUCTION_STATE 3

#define ARRAY_MIN_SIZE 5                                   //numero minimo de instrucoes para ser contada como uma configuração no array

//mask for the branch instruction type
#define ITS_BRANCH true
#define NOT_BRANCH false

//mask for barrier
#define ITS_BARRIER true
#define NOT_BARRIER false
	
//mask for the suported mode on the DIM.CPP
#define SUPPORTED true
#define NOT_SUPPORTED false

class DIM 
{
    public:
        /*VARIAVEIS UTILIZADAS PELO ALGORITMO E PELA FSM*/        
        uint32_t state;
        uint32_t spec_deepness;       
        bool detecting;       
        bool executing;
        configuration* current_config;

        /*PONTEIROS PARA O ARRAY, AS CACHES E O RESULTS*/
        memory_interface* memories;
        array* dim_array;
        results* dim_results;
        
        DIM(); 
        DIM(memory_interface* memories_argv,array *array_argv,results *results_argv);                        	
        void DIM_decode_instruction(INSTRUCTION_T instr,ADDRESS_T address,micro_instruction_t *uins);        
        void DIM_fsm_memory_access(micro_instruction_t *uins);
        void DIM_fsm_execution(micro_instruction_t *uins);
        void DIM_fsm_detection(micro_instruction_t *uins);
        void DIM_effectivate_configuration(void);
};

#endif

