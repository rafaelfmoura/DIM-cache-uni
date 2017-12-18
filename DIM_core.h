#ifndef DIM_CORE_H
#define DIM_CORE_H
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <iostream>
#include "types.h"
#include "cache.h"
#include "memory_interface.h"
#include "array.h"
#include "results.h"
#include "DIM.h"
using namespace std;
class DIM_core
{
    public:
        array* dim_array;
        results* dim_results;
        DIM *dim;
        /*===========MEMORIAS CACHE============*/
        cache *cache_uni_dim;        
        memory_interface* memories;
        /*=====================================*/
        
        INSTRUCTION_COUNTER_T count_instr;
        uint32_t core_number; 
        uint32_t thread_number; 
        micro_instruction_t uins;      
    
        char trace_name[1024];
        char type[200],reg[200],garbage[200],address_v[250],address_temp[250],instr_name[200],status[200],instruction[200],exce[200],address_p[250],field1[250], field2[250],field3[250],field4[250],field5[250],field6[250],field7[250],field8[250],field9[250],field10[250];    
        INSTRUCTION_T instruction_hex;
        ADDRESS_T address_hex;
        uint32_t trace_select;       
    
        DIM_core();
        DIM_core(char *argv,cache* ul2,pthread_mutex_t* bus_l1_to_l2,cache* ul3,pthread_mutex_t* bus_l2_to_l3);
        ~DIM_core();
        void DIM_core_set_trace_name(char *trace_name_argv);
        uint32_t DIM_core_process_trace_line(char *line);        
};
#endif
