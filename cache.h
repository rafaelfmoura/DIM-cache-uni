/**
 * @file  cache.h
 * @brief Prototypes for the class which implements a basic cache memory.
 * @author Rafael Fao de Moura.
 * @copyright GMICRO - UFSM - 2017.
*/

#ifndef  CACHE_H
#define  CACHE_H
#include "types.h"
#include "cache_block.h"
#include <time.h>
#include <math.h>
#include <map>
#include <stdlib.h>
#include <stdio.h>
#include "cacti65/io.h"

//Cache types
#define DATA_CACHE 0
#define INSTRUCTION_CACHE 1
#define CONFIGURATION_CACHE 2
#define UNIFIED_CACHE 3

//Replacement methods
#define LRU_REPLACEMENT 0
#define RANDOM_REPLACEMENT 1

//#define PRINT_DEBUG_CACHE

using namespace std;

class cache
{
    public:
    
        /*VARIABLES*/
        uint32_t cache_type;                  /**<Cache type (data,instruction,configuration or unified)*/
        cache_block** cache_blocks;           /**<Blocks which compose the memory*/                
        uint32_t cache_size;                  /**<Cache size(KB)*/  
        uint32_t cache_rows;                  /**<Cache rows*/        
        uint32_t index_bits_number;           /**<Number of bits used for addressing the index*/
        uint32_t offset_bits_number;          /**<Number of bits used for addressing the bytes inside the block*/        
        uint32_t replacement_method;          /**<Replacement method (0 = LRU, 1 = random)*/        
        uint32_t block_bytes;                 /**<Number of bytes peer block*/        
        uint32_t associativity;               /**<Associativity*/
        ADDRESS_T index_bits_mask;            /**<Mask used for the index calculation*/
        ADDRESS_T tag_bits_mask;              /**<Mask used for the tag calculation*/        
        CYCLE_COUNTER_T cycles_per_access;    /**<Time in cycles to do a memory access*/
        ENERGY_T energy_per_access;           /**<Energy in nJ consumed by an access done*/
        
        CYCLE_COUNTER_T instructions_hits_counter;
        CYCLE_COUNTER_T instructions_accesses_counter;
        CYCLE_COUNTER_T configurations_hits_counter;
        CYCLE_COUNTER_T configurations_accesses_counter;
        
        /*THE VARIABLES BELOW ARE USED FOR UNIFIED CACHE*/        
        uint32_t threshold; /**<Base threshold.*/		
        uint32_t threshold_counter; /**<Threshold counter*/		
        uint32_t threshold_max; /**<Threshold max value.*/         

        /*METHODS' PROTOTYPES*/
        
        /**
        * @fn cache(). 
        * @brief Standard constructor. 
        * Initializes the values of the variables as zero and the pointers as NULL. 
        */         
        cache();   
        
        /**
        * @fn cache(uint32_t cache_size,uint32_t associativity,uint32_t block_bytes,uint32_t replacement_method,uint32_t cache_type).
        * @brief Second constructor. 
        * Initializes the values of the variables as the parameters and allocates the cache memory blocks. 
        * @param cache_size - Cache size (KB).
        * @param associativity - Cache associativity.
        * @param block_bytes - Bytes peer block.
        * @param replacement_method - Replacement method (0 = LRU, 1 = random).
        * @param cache_type - Cache type (data_cache,instruction_cache,configuration_cache,unified_cache,perfect_cache).        
        */               
        cache(uint32_t cache_size,uint32_t associativity,uint32_t block_bytes,uint32_t replacement_method,uint32_t cache_type);
        
        /**
        * @fn cache(). 
        * @brief Standard destructor. 
        * Releases the memory used by this cache. 
        */         
        ~cache();   
           
        /**
        * @fn cache_read_access(ADDRESS_T address,uint8_t data_type). 
        * @brief Performs a reading memory access. 
        * Search this cache memory to find the block required by the address. 
        * @param address - Address to be searched in the cache.
        * @param data_type - Type of data that will be searched (instruction,data or configuration).
        * @return A pointer to a block which contains the data requested if it was found else NULL.
        */             
        cache_block* cache_read_access(ADDRESS_T address,uint8_t data_type);
        
        /**
        * @fn cache_write_access(ADDRESS_T data_address,cache_block* block_write,uint8_t dirty_flag,configuration* data). 
        * @brief Performs a writing memory access. 
        * Stores the data in the cache according to the address. 
        * @param data_address - Address        
        * @param block_write - Block to be writen     
        * @param dirty_flag - Value to be stored on the dirty bit 
        * @param data - Pointer to the data that will be stored in the cache  
        */        
        void cache_write_access(ADDRESS_T data_address,cache_block* block_write,uint8_t dirty_flag,configuration* data);

        /**
        * @fn cache_update_lru(ADDRESS_T data_address,cache_block* block). 
        * @brief Updates the LRU counters. 
        * Updates the LRU counters of the blocks in a specific cache row. 
        * @param data_address - Data address which will have the LRU counters updated.
        * @param block - Latest accessed block.
        */
        void cache_update_lru(ADDRESS_T data_address,cache_block* block);
        
        /**
        * @fn cache_choose_way(ADDRESS_T address,uint8_t data_type). 
        * @brief Chooses a block to be replaced. 
        * Chooses a block to be replaced in the cache according to the replacement method. 
        * @param address - Address to be located in the cache.
        * @param data_type - Type of block which can be replaced
        * @return A pointer to a block selected to be replaced in the cache.
        */
        cache_block* cache_choose_way(ADDRESS_T address,uint8_t data_type);
        
        /**
        * @fn cache_random(ADDRESS_T index,uint8_t data_type).
        * @brief Chooses a block to be replaced. 
        * Chooses a block to be replaced in the cache according to the random replacement method. 
        * @param index - Index where a block will be selected to be replaced.
        * @param data_type - Type of block which can be replaced
        * @return A pointer to a block selected to be replaced in the cache.
        */        
        cache_block* cache_random(ADDRESS_T index,uint8_t data_type);
        
        /**
        * @fn cache_lru(ADDRESS_T index,uint8_t data_type).
        * @brief Chooses a block to be replaced. 
        * Chooses a block to be replaced in the cache according to the LRU replacement method. 
        * @param index - Index where a block will be selected to be replaced.
        * @param data_type - Type of block which can be replaced
        * @return A pointer to a block selected to be replaced in the cache.
        */        
        cache_block* cache_lru(ADDRESS_T index,uint8_t data_type);  
        
        /**
        * @fn cache_flush(void). 
        * @brief Performs a cache flush. 
        * Invalidates all the cache blocks. 
        */          
        void cache_flush(void);  
        
        /**
        * @fn cache_update_environment(ADDRESS_T address,uint8_t data_type). 
        * @brief Updates the threshold counter and verifies if a block must be released according to 
        * the new threshould value.  
        * @param address - Data address.
        * @param data_type - Data type.
        * @return A pointer to a block if it was replaced in the cache.
        */        
        cache_block* cache_update_environment(ADDRESS_T address,uint8_t data_type);
        
        /**
        * @fn cache_count_blocks_type(ADDRESS_T index,uint8_t type). 
        * @brief Counts the number of a specific type of block in a 
        * specific index according an address.  
        * @param address - The address.
        * @param data_type - Data type.
        * @return The number of blocks in the index.
        */        
        uint32_t cache_count_blocks_type(ADDRESS_T address,uint8_t data_type);
        
        /**
        * @fn cache_print_status(void). 
        * @brief Print the status of cache.  
        */        
        void cache_print_status(void);        
};
#endif
