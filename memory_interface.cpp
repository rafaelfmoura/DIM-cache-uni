/**
 * @file  memory_interface.cpp
 * @brief Implementation of the clas which represents a basic memory interface.
 * @author Rafael Fao de Moura.
 * @copyright GMICRO - UFSM - 2017.
*/

#include "memory_interface.h"

memory_interface::memory_interface()
{     
    this->UL1 = NULL;      
    this->UL2 = NULL;
    this->UL2 = NULL;
    this->bus_l1_to_l2 = NULL;                         
    this->bus_l2_to_l3 = NULL;
}
        
memory_interface::memory_interface(cache* ul1,cache* ul2,pthread_mutex_t* bus_l1_to_l2,cache* ul3,pthread_mutex_t* bus_l2_to_l3)
{
    this->UL1 = ul1;   
    this->UL2 = ul2;
    this->UL3 = ul3;    
    this->bus_l1_to_l2 = bus_l1_to_l2;
    this->bus_l2_to_l3 = bus_l2_to_l3;
    this->memory_interface_reset_counters();  
}

memory_interface::~memory_interface()
{     
    if(this->UL1)
    {
        delete this->UL1; 
        this->UL1 = NULL;                              
    }
}
        
configuration* memory_interface::memory_interface_unified_request(ADDRESS_T data_address,uint8_t data_type,uint8_t access_type,configuration* data)
{    
    cache_block* temp_block_l1 = NULL;
    cache_block* temp_block_l2 = NULL;   
    cache_block* temp_block_l3 = NULL;       
        
    /*L1 access*/
    if(this->UL1)
    {
        temp_block_l1 = this->UL1->cache_read_access(data_address,data_type);
        
        if((data_type == CONFIGURATION_BLOCK) && (temp_block_l1 == NULL) && (access_type == INTERFACE_READ_REQUEST) )
        {
            temp_block_l1 = this->UL1->cache_read_access(data_address,INSTRUCTION_BLOCK);
        }
        
        /*L1 hit*/
        if(temp_block_l1)
        {
            /*Updates the metrics and LRU counter*/
            this->l1_unified_accesses++;
            this->UL1->cache_update_lru(data_address,temp_block_l1);
            
            /*If it's a write operation, performs it*/
            if(access_type == INTERFACE_WRITE_REQUEST)
            {
                this->UL1->cache_write_access(data_address,temp_block_l1,1,data);
                delete(data);
                data = NULL;
            }           
            return temp_block_l1->data;
        }
        /*L1 miss*/
        else
        {
            /*Updates the metrics*/
            this->l1_unified_accesses++;
            
            /*Instruction or configuration read miss*/
            if(access_type == INTERFACE_READ_REQUEST)
            {
                temp_block_l1 = this->UL1->cache_update_environment(data_address,INSTRUCTION_BLOCK);
                
                /*No existence of cache levels below this*/
                if(!this->UL2)
                {
                    /*Releases the memory*/
                    delete(temp_block_l1->data);
                    temp_block_l1->data = NULL;

                    /*L1 write operation*/
                    this->UL1->cache_write_access(data_address,temp_block_l1,0,NULL);                                
                                        
                    /*Updates the LRU counter for L1 cache level*/
                    this->UL1->cache_update_lru(data_address,temp_block_l1);                     
                    
                    return NULL;
                }
                
                /*Write back L2 mechanism*/
                if(temp_block_l1->data)
                {
                    this->memory_interface_unified_write_back(temp_block_l1->tag,temp_block_l1->data,2);                    
                } 
                
                /*Lock the mutex for L2 bus*/                
                if(this->bus_l1_to_l2)
                {
                    pthread_mutex_lock(this->bus_l1_to_l2);
                }                                              
                
                temp_block_l2 = this->UL2->cache_read_access(data_address,CONFIGURATION_BLOCK);
                /*Configuration read hit in L2*/
                if(temp_block_l2)
                {
                    /*L1 write operation*/
                    this->UL1->cache_write_access(data_address,temp_block_l1,1,temp_block_l2->data);
                    
                    /*Updates the L1 metrics*/
                    this->l1_unified_accesses++;
                    
                    /*Updates the LRU counter for both cache levels*/
                    this->UL1->cache_update_lru(data_address,temp_block_l1);
                    this->UL2->cache_update_lru(data_address,temp_block_l2);
                    
                    /*Updates the L2 metrics*/
                    this->l2_unified_accesses++;
                    
                    /*Unlock the mutex for L2 bus*/
                    if(this->bus_l1_to_l2)
                    {
                        pthread_mutex_unlock(this->bus_l1_to_l2);
                    }                      
                    
                    return temp_block_l1->data;
                }
                
                temp_block_l2 = this->UL2->cache_read_access(data_address,INSTRUCTION_BLOCK);
                /*Instruction read hit in L2*/
                if(temp_block_l2)
                {
                    /*L1 write operation*/
                    this->UL1->cache_write_access(data_address,temp_block_l1,0,NULL);                                
                    
                    /*Updates the L1 metrics*/
                    this->l1_unified_accesses++;                    
                    
                    /*Updates the LRU counter for both cache levels*/
                    this->UL1->cache_update_lru(data_address,temp_block_l1);
                    this->UL2->cache_update_lru(data_address,temp_block_l2);
                    
                    /*Updates the L2 metrics*/
                    this->l2_unified_accesses++;                
                    
                    /*Unlock the mutex for L2 bus*/
                    if(this->bus_l1_to_l2)
                    {
                        pthread_mutex_unlock(this->bus_l1_to_l2);
                    }                      
                    
                    return NULL;                
                }
                /*Instruction read miss in L2*/
                else
                {
                    temp_block_l2 = this->UL2->cache_update_environment(data_address,INSTRUCTION_BLOCK);

                    /*Write back L3 mechanism*/
                    if(temp_block_l2->data)
                    {
                        if(this->UL3)
                        {
                            this->memory_interface_unified_write_back(temp_block_l2->tag,temp_block_l2->data,3);
                        }
                        delete(temp_block_l2->data);   
                        temp_block_l2->data = NULL;                    
                    } 
                    
                    if(this->UL3)                   
                    {                
                    
                        /*Lock the mutex for L3 bus*/                        
                        if(this->bus_l2_to_l3)
                        {
                            pthread_mutex_lock(this->bus_l2_to_l3);
                        }                 
                               
                        temp_block_l3 = this->UL3->cache_read_access(data_address,CONFIGURATION_BLOCK);                    
                        /*UL3 hit conf*/
                        if(temp_block_l3)
                        {
                            this->UL2->cache_write_access(data_address,temp_block_l2,1,temp_block_l3->data);
                            /*Updates the L2 metrics*/
                            this->l2_unified_accesses++;                            
                            this->UL2->cache_update_lru(data_address,temp_block_l2);                            
                    
                            this->UL1->cache_write_access(data_address,temp_block_l1,1,temp_block_l3->data);
                            /*Updates the L1 metrics*/
                            this->l1_unified_accesses++;                            
                            this->UL1->cache_update_lru(data_address,temp_block_l1);                      
                            
                            /*Updates the L3 metrics*/                          
                            this->l3_unified_accesses++;                            
                            this->UL3->cache_update_lru(data_address,temp_block_l3); 
                            
                            /*Unlock the mutex for L3 bus*/
                            if(this->bus_l2_to_l3)
                            {
                                pthread_mutex_unlock(this->bus_l2_to_l3);
                            }                            
                            
                            /*Unlock the mutex for L2 bus*/
                            if(this->bus_l1_to_l2)
                            {
                                pthread_mutex_unlock(this->bus_l1_to_l2);
                            }
                            
                            return temp_block_l3->data;                           
                        }
                    
                        temp_block_l3 = this->UL3->cache_read_access(data_address,INSTRUCTION_BLOCK);
                        /*UL3 hit inst*/
                        if(temp_block_l3)
                        {
                            /*Updates the L3 metrics*/                          
                            this->l3_unified_accesses++;                            
                            this->UL3->cache_update_lru(data_address,temp_block_l3);                                              
                        }  
                        
                        /*UL3 miss inst*/
                        else
                        { 
                            temp_block_l3 = this->UL3->cache_update_environment(data_address,INSTRUCTION_BLOCK);

                            /*Write back L4 mechanism*/
                            if(temp_block_l3->data)
                            {
                                delete(temp_block_l3->data);   
                                temp_block_l3->data = NULL;                    
                            }                                                   
                            
                            /*Updates the L3 metrics*/   
                            this->UL1->cache_write_access(data_address,temp_block_l3,0,NULL);                       
                            this->l3_unified_accesses++;                            
                            this->UL3->cache_update_lru(data_address,temp_block_l3);                            
                        }
                        
                        /*Unlock the mutex for L3 bus*/
                        if(this->bus_l2_to_l3)
                        {
                            pthread_mutex_unlock(this->bus_l2_to_l3);
                        }                       
                    }
                    
                    this->UL2->cache_write_access(data_address,temp_block_l2,0,NULL);
                    /*Updates the L2 metrics*/
                    this->l2_unified_accesses++;                            
                    this->UL2->cache_update_lru(data_address,temp_block_l2);                            
                    
                    this->UL1->cache_write_access(data_address,temp_block_l1,0,NULL);
                    /*Updates the L1 metrics*/
                    this->l1_unified_accesses++;                            
                    this->UL1->cache_update_lru(data_address,temp_block_l1);  
                 
                    /*Unlock the mutex for L2 bus*/
                    if(this->bus_l1_to_l2)
                    {
                        pthread_mutex_unlock(this->bus_l1_to_l2);
                    }                         
                    
                    return NULL;                  
                }              
                
            }
            /*Configuration write miss*/
            else if(access_type == INTERFACE_WRITE_REQUEST)
            {
                temp_block_l1 = this->UL1->cache_update_environment(data_address,CONFIGURATION_BLOCK);

                /*Write back L2 mechanism*/
                if(temp_block_l1->data && this->UL2)
                {
                    this->memory_interface_unified_write_back(temp_block_l1->tag,temp_block_l1->data,2);                                       
                }

                this->UL1->cache_write_access(data_address,temp_block_l1,1,data); 
                delete(data);
                data = NULL;
                           
                /*Updates the LRU counter*/
                this->UL1->cache_update_lru(data_address,temp_block_l1);       
                
                return NULL;         
            }
        }
    }
    
    return NULL;
}

void memory_interface::memory_interface_reset_counters(void)
{
    this->l1_unified_accesses = 0;
    this->l2_unified_accesses = 0;
    this->l3_unified_accesses = 0;    
}

ENERGY_T memory_interface::memory_interface_get_energy(void)
{
    ENERGY_T total_energy = 0;
    
    if(this->UL1)
    {
        total_energy += (ENERGY_T)((this->l1_unified_accesses) * (this->UL1->energy_per_access));
    }
    
    if(this->UL2)
    {
        total_energy += (ENERGY_T)((this->l2_unified_accesses) * (this->UL2->energy_per_access));
    }
    
    if(this->UL3)
    {
        total_energy += (ENERGY_T)((this->l3_unified_accesses) * (this->UL3->energy_per_access));
    }    
    
    return total_energy;
}

CYCLE_COUNTER_T memory_interface::memory_interface_get_cycles(void)
{
    CYCLE_COUNTER_T total_cycles = 0;   
    
    if(this->UL1 && (this->UL1->cycles_per_access > 1) )
    {       
        total_cycles += this->l1_unified_accesses * this->UL1->cycles_per_access;
        total_cycles -= this->l1_unified_accesses;
    }
    
    if(this->UL2 && (this->UL2->cycles_per_access > 1) )
    {        
        total_cycles += this->l2_unified_accesses * this->UL2->cycles_per_access;
    }   
    
    if(this->UL3 && (this->UL3->cycles_per_access > 1) )
    {        
        total_cycles += this->l3_unified_accesses * this->UL3->cycles_per_access;
    } 
    
    //printf("memory cycles %lu\n",total_cycles);
    return total_cycles;
}

void memory_interface::memory_interface_unified_write_back(ADDRESS_T data_address,configuration* data,uint8_t cache_level)
{
    cache_block* temp_block = NULL;
    
    if(cache_level == 2)
    {        
        /*Lock the mutex for L2 bus*/
        if(this->bus_l1_to_l2)
        {
            pthread_mutex_lock(this->bus_l1_to_l2);
        }       
    
        temp_block = this->UL2->cache_read_access(data_address,CONFIGURATION_BLOCK);
        
        /*Write miss*/
        if(!temp_block)
        {
            temp_block = this->UL2->cache_update_environment(data_address,CONFIGURATION_BLOCK); 
            
            /*Write back for UL3*/
            if(temp_block->data)
            {
                if(this->UL3)
                {
                    this->memory_interface_unified_write_back(temp_block->tag,temp_block->data,3);
                }
                delete(temp_block->data);
                temp_block->data = NULL;
            }                       
        }        
        

        
        /*Performs the write operation, update the metrics and LRU counter*/
        this->UL2->cache_write_access(data_address,temp_block,1,data); 
        this->UL2->cache_update_lru(data_address,temp_block); 
        this->l2_unified_accesses++;  
        
        /*Unlock the mutex for L2 bus*/
        if(this->bus_l1_to_l2)
        {
            pthread_mutex_unlock(this->bus_l1_to_l2);
        }          
    }
    else if(cache_level == 3)
    {       
        /*Lock the mutex for L3 bus*/
        if(this->bus_l2_to_l3)
        {
            pthread_mutex_lock(this->bus_l2_to_l3);
        }       
    
        temp_block = this->UL3->cache_read_access(data_address,CONFIGURATION_BLOCK);
        
        /*Write miss*/
        if(!temp_block)
        {
            temp_block = this->UL3->cache_update_environment(data_address,CONFIGURATION_BLOCK); 
            
            /*Write back for UL4*/
            if(temp_block->data)
            {                
                delete(temp_block->data);
                temp_block->data = NULL;
            }                       
        }        
        

        
        /*Performs the write operation, update the metrics and LRU counter*/
        this->UL3->cache_write_access(data_address,temp_block,1,data); 
        this->UL3->cache_update_lru(data_address,temp_block); 
        this->l3_unified_accesses++;  
        
        /*Unlock the mutex for L2 bus*/
        if(this->bus_l2_to_l3)
        {
            pthread_mutex_unlock(this->bus_l2_to_l3);
        }            
    }    
    
}

