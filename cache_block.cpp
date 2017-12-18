/**
 * @file  cache_block.cpp
 * @brief Implementation of the prototypes for the class which implements a basic cache memory block.
 * @author Rafael Fao de Moura.
 * @copyright GMICRO - UFSM - 2017.
*/

#include "cache_block.h"

cache_block::cache_block()
{
    //this->current_MESI_state = INVALID;
    this->valid_bit = 0;
    this->tag = 0;
    this->lru_counter = 0;
    this->dirty_bit = 0;  
    this->tag_and_index = 0;  
    this->data = NULL;
}

cache_block::cache_block(uint8_t data_type,configuration* data)
{
    //this->current_MESI_state = INVALID;
    this->valid_bit = 0;
    this->tag = 0;
    this->lru_counter = 0;
    this->dirty_bit = 0;    
    this->data_type = data_type;    
    this->data = data;
    this->tag_and_index = 0;    
}

cache_block::~cache_block()
{
    if(this->data)
    {    
        delete(this->data);     
        this->data = NULL;
    }
    return;
}
