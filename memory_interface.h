/**
 * @file  memory_interface.h
 * @brief Prototypes for the class which implements a basic memory interface.
 * @author Rafael Fao de Moura.
 * @copyright GMICRO - UFSM - 2017.
*/

#ifndef MEMORY_INTERFACE_H
#define MEMORY_INTERFACE_H

#include "types.h"
#include "cache.h"
#include <stdlib.h>
#include <stdio.h>

#define INTERFACE_READ_REQUEST 0
#define INTERFACE_WRITE_REQUEST 1

class memory_interface
{
    public:
        /*VARIABLES*/             
        cache* UL1;                             /**<Unified cache level 1*/  
        cache* UL2;                             /**<Unified cache level 2*/
        cache* UL3;                             /**<Unified cache level 3*/          
        pthread_mutex_t* bus_l2_to_l3;          /**<Mutex which controls the access to l3 cache*/         
        pthread_mutex_t* bus_l1_to_l2;          /**<Mutex which controls the access to l2 cache*/
        CYCLE_COUNTER_T l1_unified_accesses;    /**<L1 unified cache accesses counter*/
        CYCLE_COUNTER_T l2_unified_accesses;    /**<L2 unified cache accesses counter*/  
        CYCLE_COUNTER_T l3_unified_accesses;    /**<L3 unified cache accesses counter*/              
        
        /*METHODS' PROTOTYPES*/
        /**
        * @fn memory_interface(). 
        * @brief Standard constructor. 
        * Initializes the values of the pointers as NULL. 
        */         
        memory_interface();
        
        /**
        * @fn memory_interface(cache* ul1,cache* ul2,pthread_mutex_t* bus_l1_to_l2,cache* ul3,pthread_mutex_t* bus_l2_to_l3). 
        * @brief Second constructor. 
        * Initializes the values of the pointers as those passed by this fhunction.                             
        * @param ul1 - Pointer for the unified cache level 1
        * @param ul2 - Pointer for the unified cache level 2
        * @param ul3 - Pointer for the unified cache level 3
        * @param bus_l1_to_l2 - Pointer for the mutex which controls the access to l2 cache
        * @param bus_l2_to_l3 - Pointer for the mutex which controls the access to l3 cache
        */         
        memory_interface(cache* ul1,cache* ul2,pthread_mutex_t* bus_l1_to_l2,cache* ul3,pthread_mutex_t* bus_l2_to_l3);
        
        /**
        * @fn memory_interface(). 
        * @brief Standard destructor. 
        * Releases the memory used by the memories. 
        */         
        ~memory_interface();        
        
        /**
        * @fn memory_interface_unified_request(ADDRESS_T data_address,uint8_t data_type,uint8_t access_type,configuration* data)
        * @brief Performs a unified memory access searching by a specific data
        * Performs a memory access according the parameters passed by parameters
        * @param data_address - The address
        * @param data_type  - Type of data being searched (data,instruction or configuration)
        * @param access_type - Flag which indicates the access_type (read of write access)
        * @param data - Pointer to a data to be stored in the memory
        * @return A pointer to a block which contains the data requested
        */
        configuration* memory_interface_unified_request(ADDRESS_T data_address,uint8_t data_type,uint8_t access_type,configuration* data);
        
        /**
        * @fn memory_interface_reset_counters(void). 
        * @brief Reset the value of counters which keep the metrics. 
        */        
        void memory_interface_reset_counters(void);
        
        /**
        * @fn memory_interface_get_energy(void). 
        * @brief Calculates the energy wasted by the memories. 
        * @return The energy spent by the memories in nJ
        */         
        ENERGY_T memory_interface_get_energy(void);
        
        /**
        * @fn memory_interface_get_cycles(void). 
        * @brief Calculates the number of cycles wasted by the memories. 
        * @return The number of cycles spent by the memories.
        */          
        CYCLE_COUNTER_T memory_interface_get_cycles(void);
        
        /**
        * @fn memory_interface_unified_write_back(ADDRESS_T data_address,configuration* data,uint8_t cache_level). 
        * @brief Performs a write back operation. 
        * @param data_address - Address that will be stored in the next cache level
        * @param data - Pointer to the data that will be stored in the next cache level
        * @param cache_level - Next cache level
        */        
        void memory_interface_unified_write_back(ADDRESS_T data_address,configuration* data,uint8_t cache_level);
};

#endif
