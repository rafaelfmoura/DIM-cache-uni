/**
 * @file  configuration.h
 * @brief Prototypes for the configuration class.
 * @author Rafael Fao de Moura.
 * @copyright GMICRO - UFSM - 2017.
*/

#ifndef CONFIGURATION_H
#define CONFIGURATION_H
#include <iostream>
#include<stdio.h>
#include "types.h"

#ifndef MAX_SPEC_DEPTH
#define MAX_SPEC_DEPTH 4
#endif

class configuration
{    
    public:
        ADDRESS_T first_pc;                                    /**<Configuration's index PC*/
        ADDRESS_T last_pc;                                     /**<Configuration's last PC*/
        ADDRESS_T speculative_pcs[MAX_SPEC_DEPTH];             /**<Vector which contains the speculated PCs*/
        POWER_T power;                                         /**<Power dissipated on array*/
        INSTRUCTION_COUNTER_T instructions[MAX_SPEC_DEPTH];    /**<Number of instructions on the array by speculation level*/
        CYCLE_COUNTER_T total_cycles[MAX_SPEC_DEPTH];          /**<Number of total cycles to execute the configuration on the array by speculation level*/
        CYCLE_COUNTER_T exec_cycles[MAX_SPEC_DEPTH];           /**<Number of cycles to execute the configuration on the array by speculation level*/
        CYCLE_COUNTER_T wb_cycles[MAX_SPEC_DEPTH];             /**<Number of cycles to do write back on the array by speculation level*/
        INSTRUCTION_COUNTER_T total_instructions;              /**<Number of total instructions*/         
        
    /**
    * @fn configuration(void). 
    * @brief Default constructor. 
    */        
    configuration(void);
        
    /**
    * @fn configuration(void). 
    * @brief Default destructor. 
    */    
    ~configuration(void);
    
    /**
    * @fn configuration_copy_configuration(void). 
    * @brief Is makes a copy of this current configuration. 
    * @return A pointer to its copy.
    */    
    configuration* configuration_copy_configuration(void);
    
    /**
    * @fn configuration_reset_configuration(void). 
    * @brief It clears all configuration's fields'.    
    */     
    void configuration_reset_configuration(void);
    
    /**
    * @fn configuration_get_power(void).
    * @brief It returns the total power to execute a whole configuration
    * @return Power (mW).
    */        
    POWER_T configuration_get_power(void); 
    
    /**
    * @fn configuration_get_total_instructions(void).
    * @brief It retuns the number of total instructions contained into a specific configuration.
    * @return The number of instructions into the configuration.
    */         
    INSTRUCTION_COUNTER_T configuration_get_total_instructions(void);  
    
    /**
    * @fn configuration_get_instructions_by_spec_deepness(uint32_t spec_deepness).
    * @brief It returns the number of instruction into a specific speculation level
    * @param spec_deepness - Current speculation level.
    * @return The number of instructions contained into the speculation level.
    */         
    INSTRUCTION_COUNTER_T configuration_get_instructions_by_spec_deepness(uint32_t spec_deepness); 
    
    /**
    * @fn configuration_get_cycles_by_spec_deepness(uint32_t spec_deepness).
    * @brief It retuns the total cycles in order to execute a whole speculation level in the array
    * @param spec_deepness - Current speculation level.
    * @return Time (in cycles).
    */               
    CYCLE_COUNTER_T configuration_get_cycles_by_spec_deepness(uint32_t spec_deepness); 
    
    /**
    * @fn configuration_check_speculative_pc(ADDRESS_T pc, uint32_t spec_deepness).
    * @brief It checks if we took the correct branch.
    * @param pc -  Speculative PC.
    * @param spec_deepness - Current speculation deepness.
    * @return True if we took the correct branch and False otherwise.
    */                          
    bool configuration_check_speculative_pc(ADDRESS_T pc, uint32_t spec_deepness);
    
    /**
    * @fn configuration_update_speculative_pcs(ADDRESS_T new_pc, uint32_t spec_deepness).
    * @brief It updates the speculatives PC's that will be checked in the future.
    * @param new_pc - Speculative PC to be added.
    * @param spec_deepness - Current speculation depth.
    */                  
    void configuration_update_speculative_pcs(ADDRESS_T new_pc, uint32_t spec_deepness);  
    
    void configuration_print_configuration(void);       
};    
#endif
