/**
 * @file  configuration.cpp
 * @brief Implementation for the configuration class.
 * @author Rafael Fao de Moura.
 * @copyright GMICRO - UFSM - 2017.
*/

#include "configuration.h"

configuration::configuration(void)
{
    /*It clears all those configuration's fields*/
    this->configuration_reset_configuration();
        
    return;
}

configuration::~configuration(void)
{
    return;
}

configuration* configuration::configuration_copy_configuration(void)
{
    configuration* new_configuration = NULL;
    uint32_t counter;
    
    new_configuration = new configuration();
    
    /*It copies the general fields*/
    new_configuration->first_pc = this->first_pc;
    new_configuration->last_pc = this->last_pc;
    new_configuration->power = this->power;
    new_configuration->total_instructions = this->total_instructions;
    
    /*It copies the fields which depend on the speculation level*/
    for(counter = 0;counter < MAX_SPEC_DEPTH;counter++)
    {
        new_configuration->speculative_pcs[counter] = this->speculative_pcs[counter];
        new_configuration->instructions[counter] = this->instructions[counter];
        new_configuration->total_cycles[counter] = this->total_cycles[counter];
        new_configuration->exec_cycles[counter] = this->exec_cycles[counter];
        new_configuration->wb_cycles[counter] = this->wb_cycles[counter];        
    }  
        
    return new_configuration;    
}

void configuration::configuration_reset_configuration(void)
{
    uint32_t counter;
    
    /*It resets the general fields*/
    this->first_pc = 0;
    this->last_pc = 0;
    this->power = 0;
    this->total_instructions = 0;
    
    /*It resets the fields which depend on the speculation level*/
    for(counter = 0;counter < MAX_SPEC_DEPTH;counter++)
    {
        this->speculative_pcs[counter]=0;
        this->instructions[counter]=0;
        this->total_cycles[counter]=0;
        this->exec_cycles[counter]=0;
        this->wb_cycles[counter]=0;        
    }
}

POWER_T configuration::configuration_get_power(void)
{
    /*It returns the total power due to the configuration's execution*/
    return this->power;
}


INSTRUCTION_COUNTER_T configuration::configuration_get_total_instructions(void)
{
    /*It returns the total number of instrcutions contained in a configuration*/
    return this->total_instructions; 
}
    
INSTRUCTION_COUNTER_T configuration::configuration_get_instructions_by_spec_deepness(uint32_t spec_deepness)
{
    /*It returns the instructions number contained in a specific
    speculation level*/
    return this->instructions[spec_deepness];
}
    
CYCLE_COUNTER_T configuration::configuration_get_cycles_by_spec_deepness(uint32_t spec_deepness)
{
    /*It returns the number of cycles for each speculation level*/
    return this->total_cycles[spec_deepness];
}
    
bool configuration::configuration_check_speculative_pc(ADDRESS_T pc, uint32_t spec_deepness)
{
    /*It checks if we took the correct branch*/
    return (this->speculative_pcs[spec_deepness] == pc);
}   

void configuration::configuration_update_speculative_pcs(ADDRESS_T new_pc, uint32_t spec_deepness)
{
    /*It just updates the speculative pc in its current speculation level*/
    this->speculative_pcs[spec_deepness] = new_pc;    
}     

void configuration::configuration_print_configuration(void)
{    
    uint32_t counter;   
    
    printf("===============CONFIGURATION===============\n");
    
    /*It copies the general fields*/
    printf("First pc %lx\n",this->first_pc);
    printf("Last pc %lx\n",this->last_pc);
    printf("Power %Lf\n",this->power);
    printf("Instructions %lu\n",this->total_instructions);
    
    /*It copies the fields which depend on the speculation level*/
    for(counter = 0;counter < MAX_SPEC_DEPTH;counter++)
    {
        printf("SPEC LEVEL %u\n",counter);
        printf("Speculatibe pc %lx\n",this->speculative_pcs[counter]);
        printf("Cycles %lu\n",this->total_cycles[counter]);    
    }     

}
