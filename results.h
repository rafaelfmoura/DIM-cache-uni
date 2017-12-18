/**
 * @file  results.h
 * @brief Prototype of the class results and its methods.
 * @author Rafael Fao de Moura.
 * @copyright GMICRO - UFSM - 2016.
*/

#ifndef  RESULTS_H
#define  RESULTS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "memory_interface.h"

#define NORMAL_RESULTS 0
#define END_RESULTS 1

#define SPARC_CPI    1.4                                            
#define ALU_POWER    8.5                                           
#define FP_ALU_POWER 25.5                                           
#define MEM_POWER    0.1                                           
#define MUL_POWER    13.6                                          
#define TABLE_PC     12.0                                          
#define MIPS_POWER   47.8                                           

//#define PRINT_DEBUG_RESULTS

class results 
{
    public:        
        FILE* results_trace_file;                             /**<Core results file.*/         
        POWER_T total_power_on_array;                         /**<Dissipated power on array.*/
        uint32_t core_number;                                 /**<Core number.*/
        uint32_t barriers_counter;                            /**<Barriers counter.*/
        CYCLE_COUNTER_T total_cycles_on_array;                /**<Number of cycles executed on array.*/                	       
        INSTRUCTION_COUNTER_T total_instructions;             /**<Number of total instructions executed.*/ 
        INSTRUCTION_COUNTER_T total_instructions_on_array;    /**<Number of total instructions executed on array.*/
        memory_interface* memories;                           /**<Pointer to a memory interface.*/
        
        /**
        * @fn results(). 
        * @brief results' class standard constructor. 
        * Sets the pointers to memories and the results file as NULL. 
        */        
        results();
        
        
        /**
        * @fn results(uint32_t core_number_argv,memory_interface* memories_argv).
        * @brief results class standard constructor. 
        * Updates the pointer to memories, creates the results file and resets the performance metrics counters.
        * @param core_number_argv - Core number.
        * @param memories_argv - Pointer to the memory interface
        * @see results_reset_counters().         
        * @remarks Don't pass as parameter invalid reference to memory interface.
        */        
        results(uint32_t core_number_argv,memory_interface* memories_argv);        
        
        
        /**
        * @fn ~results(). 
        * @brief results class standard destructor. 
        * Closes the file which contains the core results.        
        */         
        ~results();
        
        
        /**
        * @fn results_inc_total_inst_on_array(INSTRUCTION_COUNTER_T instructions). 
        * @brief Increases the number of instructions executed on array.
        * @param instructions - Number of instructions executed on array.           
        * @remarks Don't pass as parameter negative values.
        */          
        void results_inc_total_inst_on_array(INSTRUCTION_COUNTER_T instructions);
        
        
        /**
        * @fn results_inc_total_inst(INSTRUCTION_COUNTER_T instructions). 
        * @brief Increases the number of instructions executed on the simulation. 
        * @param instructions - Number of instructions executed.           
        * @remarks Don't pass as parameter negative values.
        */        
        void results_inc_total_inst(INSTRUCTION_COUNTER_T instructions);
        
        
        /**
        * @fn results_inc_total_cycles_on_array(CYCLE_COUNTER_T cycles). 
        * @brief Increases the number of cycles executed on array. 
        * @param cycles - Number of cycles executed on array.           
        * @remarks Don't pass as parameter negative values.
        */        
        void results_inc_total_cycles_on_array(CYCLE_COUNTER_T cycles);
        
        
        /**
        * @fn results_inc_total_power_on_array(POWER_T power). 
        * @brief Increases the dissipated power on array. 
        * @param power - Dissipated power (mW) on array.           
        */        
        void results_inc_total_power_on_array(POWER_T power);
        
        
        /**
        * @fn results_power_module(). 
        * @brief Performs the calculations of power and energy spent in system
        * and stores the results on the core results file.
        */        
        void results_power_module();
        
        
        /**
        * @fn results_gain_between_barriers(uint32_t results_type). 
        * @brief Calculates the performance metrics (instructions number,cycles number and energy) in the whole system
        * for the current barrier and prints the results in the core results file.
        * @param results_type - Flag which indicates NORMAL_RESULTS ou END_RESULTS.           
        */        
        void results_gain_between_barriers(uint32_t results_type);
        
        
        /**
        * @fn results_reset_counters(). 
        * @brief Resets the counters which keep the metrics used for calculation of performance
        * between two synchronization points (barriers).        
        */         
        void results_reset_counters();
        
        
        /**
        * @fn results_cache_dim_cycles(void) 
        * @brief Calculates the number of cycles spent due to memory accesses done
        * by DIM system.
        * @return The number of cycles spent due to memory accesses done by DIM system.
        */ 
        CYCLE_COUNTER_T results_cache_dim_cycles(void);
        
        /**
        * @fn results_cache_standalone_cycles(void) 
        * @brief Calculates the number of cycles spent due to memory accesses done
        * by standalone system.
        * @return The number of cycles spent due to memory accesses done by standalone system.
        */         
        CYCLE_COUNTER_T results_cache_standalone_cycles(void);
        
        /**
        * @fn results_cache_dim_energy(void) 
        * @brief Calculates the energy spent due to memory accesses done
        * by DIM system.
        * @return The energy (nJ) spent due to memory accesses done by DIM system.
        */         
        ENERGY_T results_cache_dim_energy(void);
        
        /**
        * @fn results_cache_standalone_energy(void) 
        * @brief Calculates the energy spent due to memory accesses done
        * by standalone system.
        * @return The energy (nJ) spent due to memory accesses done by standalone system.
        */                    
        ENERGY_T results_cache_standalone_energy(void);
};
#endif
