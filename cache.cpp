/**
 * @file  cache.cpp
 * @brief Implementation for the cache class.
 * @author Rafael Fao de Moura.
 * @copyright GMICRO - UFSM - 2017.
*/

#include "cache.h"

cache::cache() 
{
    this->cache_blocks = NULL;
}

cache::cache(uint32_t cache_size,uint32_t associativity,uint32_t block_bytes,uint32_t replacement_method,uint32_t cache_type)
{
    uint32_t i, j;
    uint64_t aux_bits_mask = 1;
    cache_block* temp_cache_block = NULL; 
    
    this->instructions_hits_counter = 0;
    this->instructions_accesses_counter = 0;
    this->configurations_hits_counter = 0;
    this->configurations_accesses_counter = 0;     
    	
    /*Cache size in KB*/
    this->cache_size = cache_size;
    
    /*Associativity*/
    this->associativity = associativity;
    
    /*Bytes per block*/
    this->block_bytes = block_bytes;
    
    /*Replacement method*/
    this->replacement_method = replacement_method;
    
    /*Cache type*/
    this->cache_type= cache_type;

    
    /*
    Gets the number of rows:
    cache_rows = cache_size/(block_bytes*associativity)
    To get the cache_size in Bytes we must multiply it by 1024
    */	
    this->cache_rows=(this->cache_size*1024)/(this->block_bytes*this->associativity);   

    
    /*
    Gets the number of bits necessary for the index calculation.
    index_bits_number=log2(cache_rows)
    */
    this->index_bits_number =(uint32_t) log2(this->cache_rows);	
    
    /*Gets the bits number for addressing the bytes inside the cache blocks:
    offset_bits_number = log2(block_bytes)*/	
    this->offset_bits_number =(uint32_t) log2(this->block_bytes);
    
    /*Generates the index bits mask*/	    
    this->index_bits_mask = 0;
    for(i=(index_bits_number+offset_bits_number) -1;i>=offset_bits_number;i--)
    {
        this->index_bits_mask = this->index_bits_mask | (uint64_t)(aux_bits_mask<<i);
    }

    /*Generates the tag bits mask*/	    
    this->tag_bits_mask = 0;    
    if(this->cache_type != CONFIGURATION_CACHE)
    {        
        for(i=63;i>=(index_bits_number+offset_bits_number);i--)
        {
            this->tag_bits_mask = this->tag_bits_mask | (aux_bits_mask<<i);         
        }
    }

    /*Rows creation*/
    this->cache_blocks = new cache_block*[this->cache_rows];

    /*Columns creation*/
    for(i = 0; i < (this->cache_rows); i++)		    							//alocação das colunas da cache
    {
        this->cache_blocks[i] = new cache_block[this->associativity];            
    }        

    /*Initializes the cache blocks*/        
    for (i = 0; i < (this->cache_rows); i++) 
    {
        for (j = 0; j < (this->associativity); j++) 
        {
            switch(this->cache_type)
            {
                case INSTRUCTION_CACHE:
                {
                    temp_cache_block = new cache_block(INSTRUCTION_BLOCK,NULL);
                    this->cache_blocks[i][j] = *(temp_cache_block);
                    break;
                }
                case DATA_CACHE:
                {
                    temp_cache_block = new cache_block(DATA_BLOCK,NULL);
                    this->cache_blocks[i][j] = *(temp_cache_block);
                    break;
                }                
                case CONFIGURATION_CACHE:
                {
                    temp_cache_block = new cache_block(CONFIGURATION_BLOCK,NULL);
                    this->cache_blocks[i][j] = *(temp_cache_block);
                    break;
                }   
                case UNIFIED_CACHE:
                {
                    if(j < (this->associativity/2))
                    {
                        temp_cache_block = new cache_block(INSTRUCTION_BLOCK,NULL);
                    }
                    else
                    {
                        temp_cache_block = new cache_block(CONFIGURATION_BLOCK,NULL);
                    }
                    this->cache_blocks[i][j] = *(temp_cache_block);
                    break;                    
                }             
                default:
                {
                    temp_cache_block = NULL;
                    this->cache_blocks[i][j] = *(temp_cache_block);
                    break;
                }                
            }
            
        }
    }   
    
    /*Gets both time and energy access from CACTI*/
    uca_org_t result;  
    result = cacti_interface((int)(this->cache_size*1024),(int)(this->block_bytes),(int)(this->associativity),1,1,(int)(this->block_bytes*8),(int)(64 - (this->offset_bits_number + this->index_bits_number)));        
    double cacti_access_time_ns = result.access_time*1e9;
    double cacti_energy_access_nj = result.power.readOp.dynamic*1e9;
    this->cycles_per_access = (CYCLE_COUNTER_T)(ceil((cacti_access_time_ns/(1.0/0.6))));
    if(this->cycles_per_access == 0)
    {
        this->cycles_per_access = 1;
    }
    this->energy_per_access = (ENERGY_T)(cacti_energy_access_nj);
    
    /*THE VARIABLES BELOW ARE ONLY USED FOR UNIFIED CACHE*/        
    this->threshold=5;		
    this->threshold_counter=8;		
    this->threshold_max=16;        
}

cache::~cache()
{
    uint32_t i;                  
           
    /*Releases the cache rows*/
    for(i = 0;i<(this->cache_rows);i++)    
    {
        delete[] this->cache_blocks[i];
    }
    /*Releases the entire cache*/
    delete[] this->cache_blocks;   
    
    return;
}

cache_block* cache::cache_read_access(ADDRESS_T address,uint8_t data_type)
{    
    ADDRESS_T tag;
    ADDRESS_T index; 
    uint32_t columns_counter;
       
    /*Gets the tag through the tag bits mask*/
    if(data_type != CONFIGURATION_BLOCK)
    {
        tag = (address & this->tag_bits_mask) >> (this->index_bits_number + this->offset_bits_number);
    }
    else
    {
        tag = address;
        this->configurations_accesses_counter++;
    }             
    
    if(data_type == INSTRUCTION_BLOCK)
    {
        this->instructions_accesses_counter++;    
    }     

    /*Gets the index through the index bits mask*/
    index = (address & this->index_bits_mask) >> (this->offset_bits_number);  
    
    /*Searches for the requested block in the cache*/
    for(columns_counter = 0;columns_counter < (this->associativity); columns_counter++)
    {
        /*If it was found,returns a pointer to this block*/
        if( (this->cache_blocks[index][columns_counter].tag==tag) && (this->cache_blocks[index][columns_counter].valid_bit) && (this->cache_blocks[index][columns_counter].data_type==data_type))    
        {         
            if(data_type == CONFIGURATION_BLOCK)
            {                
                this->configurations_hits_counter++;                
            }
            else if(data_type == INSTRUCTION_BLOCK)
            {               
                this->instructions_hits_counter++;                
            }            
            
            return &(this->cache_blocks[index][columns_counter]);
        }
    }
    return NULL;
}

void cache::cache_write_access(ADDRESS_T data_address,cache_block* block_write,uint8_t dirty_flag,configuration* data)
{   
    ADDRESS_T tag;
    ADDRESS_T tag_and_index;
    
    /*Gets the tag through the tag bits mask*/
    if(block_write->data_type != CONFIGURATION_BLOCK)
    {
        tag = (data_address & this->tag_bits_mask) >> (this->index_bits_number + this->offset_bits_number);
    }
    else
    {
        tag = data_address;
    }    
    
    /*Gets the bits from index and tag for write back mechanism*/
    tag_and_index = data_address & (this->tag_bits_mask | this->index_bits_mask);

    /*Stores the data in the block*/
    if(block_write->data)
    {
        delete(block_write->data);
        block_write->data = NULL;
    }
    if(data)
    {
        block_write->data = data->configuration_copy_configuration();
    }
    else
    {
        block_write->data = NULL;
    }
    block_write->tag = tag;
    block_write->valid_bit = 1;
    block_write->dirty_bit = dirty_flag;    
    block_write->tag_and_index = tag_and_index;
}


void cache::cache_update_lru(ADDRESS_T data_address,cache_block *block)
{
    uint32_t j;
    ADDRESS_T index;
    
    index = (data_address & this->index_bits_mask) >> (this->offset_bits_number);

    /*Sets the latest used block counter to 1*/
    block->lru_counter = 1;
    
    /*For the others blocks, increases their counters*/
    for (j = 0; j < this->associativity; j++) 
    {
        if( ( &(this->cache_blocks[index][j]) != block ) && (this->cache_blocks[index][j].valid_bit) )
        {
            this->cache_blocks[index][j].lru_counter++;
        }
    }
}

cache_block* cache::cache_choose_way(ADDRESS_T address,uint8_t data_type)
{
    /*Gets the index through the index bits mask*/
    ADDRESS_T index = (address & this->index_bits_mask) >> (this->offset_bits_number);  

    /*Selects a block to be replaced in the given index according to the replacement method*/
    if(this->replacement_method == LRU_REPLACEMENT)
    {
        return cache_lru(index,data_type);
    }
    else
    {
        return cache_random(index,data_type);    
    }    
}

cache_block* cache::cache_random(ADDRESS_T index,uint8_t data_type)
{
    uint32_t j;
    uint32_t chosen;      
    
    /*Searches among the blocks for an invalid block*/
    for (j = 0; j < this->associativity; j++) 
    {
        /*If a INVALID block was found returns it*/
        if(!this->cache_blocks[index][j].valid_bit && this->cache_blocks[index][j].data_type == data_type)
        {
            return &(this->cache_blocks[index][j]);
        }           
    }
    
    /*Selecrs randomly a block to be replaced and returns it*/
    while(true)
    {
        srand(time(NULL));
        chosen = rand()%this->associativity;
        if(this->cache_blocks[index][chosen].data_type == data_type)
        {
            return &(this->cache_blocks[index][chosen]);   
        }
    }
}

cache_block* cache::cache_lru(ADDRESS_T index,uint8_t data_type)
{
    uint32_t chosen = 0, counter = 0;
    uint32_t j;       
    
    /*Searches among the blocks for the Least Recently Used or an invalid block*/
    for (j = 0; j < this->associativity; j++) 
    {
        /*If a INVALID block was found returns it*/
        if(!this->cache_blocks[index][j].valid_bit && (this->cache_blocks[index][j].data_type == data_type) )
        {
            return &(this->cache_blocks[index][j]);
        }
        /*Else, continues looking for the Least Recently Used block*/
        else if ( (this->cache_blocks[index][j].data_type == data_type) && (this->cache_blocks[index][j].lru_counter >= counter) ) 
        {
            chosen = j;
            counter = this->cache_blocks[index][j].lru_counter;
        }
    }
    return &(this->cache_blocks[index][chosen]);
}

void cache::cache_flush(void)
{
    /*Reset the cache blocks*/
    uint32_t i,j;
    
    for (i = 0; i < this->cache_rows; i++) 
    {
        for (j = 0; j < this->associativity; j++) 
        {
            this->cache_blocks[i][j].valid_bit = 0;		
        }
    }	
}

cache_block* cache::cache_update_environment(ADDRESS_T address,uint8_t data_type)
{
    cache_block* chosen = NULL;

    /*If it had happened a instruction miss*/
    if(data_type == INSTRUCTION_BLOCK)
    {
        /*If the threshold counter value is less than the max value, increases it*/
        if(this->threshold_counter < this->threshold_max)
        {
            this->threshold_counter++;
        }
        /*If the theshould counter's value is bigger than the threshold
        and the instruction blocks number is less than the associativuty,
        releases a configuration block and set it as an instruction block.*/
        if( (this->threshold_counter > this->threshold) && (this->cache_count_blocks_type(address,INSTRUCTION_BLOCK) < this->associativity) )
        {
            chosen = this->cache_choose_way(address,CONFIGURATION_BLOCK); 
            chosen->data_type = INSTRUCTION_BLOCK;
        }
        /*Else, releases a instruction block*/
        else
        {
            chosen = this->cache_choose_way(address,INSTRUCTION_BLOCK); 
        }
    }
    /*If it had happened a configuration miss*/
    else
    {        
        /*If the threshold counter value is bigger than zero,
        decreases it.*/
        if(this->threshold_counter > 0) 
        {
            this->threshold_counter--;
        }
        /*If the threshold counter's value is less than the threshold
        and the configuration blocks number is less than the associativity,
        releases a instruction block and set it as a configuration block.*/
        if( (this->threshold_counter < this->threshold) && ( this->cache_count_blocks_type(address,CONFIGURATION_BLOCK) < this->associativity)  )
        {
            chosen =  this->cache_choose_way(address,INSTRUCTION_BLOCK); 
            chosen->data_type = CONFIGURATION_BLOCK;
        }
        /*Else, releases a configuration block*/
        else
        {
            chosen =  this->cache_choose_way(address,CONFIGURATION_BLOCK); 
        }        
    }
    
    return chosen;	
}

uint32_t cache::cache_count_blocks_type(ADDRESS_T address,uint8_t data_type)
{
    uint32_t i;
    uint32_t counter = 0;
    ADDRESS_T index = (address & this->index_bits_mask) >> (this->offset_bits_number);  
    
    for(i=0;i<this->associativity;i++)
    {
        if(this->cache_blocks[index][i].data_type == data_type)
        {
            counter++;
        }
    }
    
    return counter;
}

void cache::cache_print_status(void)
{
    printf("====CACHE STATUS====\n");
    printf("Cache size: %u\n",this->cache_size);
    printf("Associativity: %u\n",this->associativity);
    printf("Bytes per block: %u\n",this->block_bytes);    
    printf("Cache rows: %u\n",this->cache_rows);    
    printf("Number of bits used for block offset: %u\n",this->offset_bits_number);    
    printf("Number of bits used for index calculation: %u\n",this->index_bits_number);    
    printf("Index bits mask: %lu\n",this->index_bits_mask);          
    printf("Tag bits mask: %lu\n",this->tag_bits_mask);      
    printf("Cycles per access: %lu\n",this->cycles_per_access);
    printf("Energy per access(nJ): %LF\n",this->energy_per_access);    
    
}
