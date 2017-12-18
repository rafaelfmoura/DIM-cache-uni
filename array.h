/**
 * @file  array.h
 * @brief Prototypes for the array class.
 * @author Rafael Fao de Moura.
 * @copyright GMICRO - UFSM - 2016.
*/

#ifndef  ARRAY_H
#define  ARRAY_H
#include<stdio.h>
#include<iostream>
#include<stdlib.h>
#include<math.h>
#include<stdbool.h>
#include"types.h"
#include"results.h"
#include"configuration.h"
#include"micro_instruction.h"

#ifndef MAX_SPEC_DEPTH
#define MAX_SPEC_DEPTH 4
#endif

#define N_ARRAY_WB       4      /*Register file's ports*/
#define N_TIME_RECONF    4      /*Bynary translation pipeline stages*/
#define UF_EMPTY         0x0    /*Constant which indicates that a UF is empty in the array*/
#define UF_ALOCATED      0x1    /*Constant which indicates that a UF is being used in the array*/
#define UF_VOID          0x2    /*Constant which indicates that a UF is not enabled in the array*/

/*Instruction type groups*/
#define ALU_TYPE    1
#define MEM_TYPE    2
#define MUL_TYPE    3
#define FP_TYPE     4

//DEBUG PRINTS
//#define PRINT_DEBUG_ARRAY

#define GENERATE_SHAPE          1
#define LOAD_REGULAR_SHAPE      2
#define LOAD_IRREGULAR_SHAPE    3


class array
{
    public:
        /*Array configuration variables*/
        uint32_t ARRAY_ROWS_NUMBER;           /**<Array's rows number*/
        uint32_t ARRAY_COLUMNS_NUMBER;        /**<Array's columns number*/
        uint32_t ARRAY_MUL_COLUMNS_NUMBER;    /**<Array's MUL columns number*/
        uint32_t ARRAY_MEM_COLUMNS_NUMBER;    /**<Array's MEM columns number*/
        uint32_t ARRAY_FP_COLUMNS_NUMBER;     /**<Array's FP columns number*/
        uint32_t INPUT_CONTEXT;               /**<Input context size*/
        uint32_t SPEC_DEPTH;                  /**<Speculation depth*/
		
        /*Array components*/
        INSTRUCTION_T **array_bitmap;    /**<Array's bitmap,it must be array_bitmap[ARRAY_ROWS_NUMBER][ARRAY_COLUMNS_NUMBER]*/   
        uint32_t *regs_w;                /**<Table which keeps the registers that are written in each line.It must have the size of ARRAY_ROWS_NUMBER*/
        uint32_t op_read_list;           /**<Table which keeps the registers that are loaded as input for the configuration execution*/
        uint32_t context;                /**<Current context*/
        uint32_t context_used;           /**<Number of registers used by the current context*/
        configuration conf_detecting;    /**<Current configuration which is being detected*/
        
        /*Shape components*/    
        uint32_t shape_function;              /**<Shape function, to generate shape, to load regular/irregular shape*/
        uint32_t shape_number;                /**<Shape number*/ 
        uint32_t **shape_array_used;          /**<Table which keeps the biggest number of each UF type by array row*/
        uint32_t **shape_conf_used;           /**<Table which keeps the biggest number of each UF type by array row in the current configuration*/
        uint32_t shape_array_biggest_line;    /**<The biggest row number used*/
        uint32_t shape_conf_biggest_line;     /**<The biggest row number used in the current configuration*/
        	
        /**
        * @fn array(). 
        * @brief Default constructor. 
        * It initializes all pointers as NULL. 
        */	
        array(void);        
        
        /**
        * @fn ~array(). 
        * @brief Default desctructor. 
        * Releases the memory used by this class' members. 
        */	
        ~array(void);          
           
        /**
        * @fn array(uint32_t INPUT_CONTEXT_argv,uint32_t SPEC_DEPTH_argv,uint32_t shape_function_argv,char *shape_filename_argv,uint32_t shape_number_argv)
        * @brief Second constructor. 
        * It allocates the necessary memory for the structures according to values written in the shape file
        * passed as parameter.
        * @param INPUT_CONTEXT_argv - Input context's size.
        * @param SPEC_DEPTH_argv - Speculation deepness.
        * @param shape_function_argv - Shape function.
        * @param shape_filename_argv - Filename wich contains the settings of the shape that will be loaded.
        * @param shape_number_argv - The shape(core) number
        * @see array_reset_array().              
        */        
        array(uint32_t INPUT_CONTEXT_argv,uint32_t SPEC_DEPTH_argv,uint32_t shape_function_argv,char *shape_filename_argv,uint32_t shape_number_argv);
                
        /**
        * @fn array_calculate_bytes_per_configuration(void).
        * @brief It calculates the number of necessary bytes to store a configuration.
        * @return The number of necessary bytes to store a configuration.
        */        
        uint32_t array_calculate_bytes_per_configuration(void);        
        
        /**
        * @fn array_print_array_status(void).
        * @brief It prints the array status.
        * @see array_calculate_bytes_per_configuration(void).
        */                
        void array_print_array_status(void);        
        
        /**
        * @fn array_log2(uint32_t number).
        * @brief It calculates the base two logarithmic of a 32 bits number passed as parameter.
        * @param number - The 32 bits number.
        * @return The base two logarithmic.
        */         
        uint32_t array_log2(uint32_t number);
        
        /**
        * @fn array_log2(uint64_t number).
        * @brief It calculates the base two logarithmic of a 64 bits number passed as parameter.
        * @param number - The 64 bits number.
        * @return The base two logarithmic.
        */                
        uint64_t array_log2(uint64_t number);        
        
        /**
        * @fn array_reset_array(void).
        * @brief It clears the current configuration's fields which is being detected.
        */                 
        void array_reset_array(void);        
        
        /**
        * @fn array_create_new_config(ADDRESS_T pc).
        * @brief It prepares the array to restart making a new configuration.
        * @param pc - New configuration's pc index.
        * @see array_reset_array(void).
        */          
        void array_create_new_config(ADDRESS_T pc);        
        
        /**
        * @fn array_add_inst_into_array(micro_instruction_t *uins,uint32_t current_spec_depth).
        * @brief It add a new instruction into the current configuration that's being detected.
        * @param uins - A representation of the current instruction to be added into array
        * @param current_spec_depth - Current speculation depth.
        * @return True if the instruction was successfully and False otherwise.
        * @see array_there_is_dependencies(uint32_t op,uint32_t row).
        */         
        bool array_add_inst_into_array(micro_instruction_t *uins,uint32_t current_spec_depth);        
        
        /**
        * @fn array_store_current_config(void).
        * @brief It does a copy of the current configuration being detected.
        * @return A pointer for a copy of its current configuration.
        */         
        configuration* array_store_current_config(void);               
        
        /**
        * @fn array_there_is_dependencies(uint32_t op,uint32_t row).
        * @brief It verifies if there is real dependencie (Read after Write) in the array.
        * @param op - Register that will have dependencies verified.
        * @param row - Array row where will be verified if there is a real dependencie.        
        * @return True if there is a dependencie related to the register 'op' and False otherwise.
        */         
        bool array_there_is_dependencies(uint32_t op,uint32_t row);        
        
        /**
        * @fn array_count_exec_time(void).
        * @brief It counts current configuration's execution time.
        * @return Time (in cycles) to execute this configuration.
        */          
        CYCLE_COUNTER_T array_count_exec_time(void);        
        
        /**
        * @fn array_reconf_exec_time(void).
        * @brief It counts current configuration's reconfiguration time.
        * @return Time (in cycles) to load the operands of this configuration.
        */          
        CYCLE_COUNTER_T array_count_reconf_time(void);        
        
        /**
        * @fn array_count_writeback_time(void).
        * @brief It counts current configuration's write back time.
        * @return Time (in cycles) to perfom write back in this configuration.
        */          
        CYCLE_COUNTER_T array_count_writeback_time(void);        
        
        /**
        * @fn array_count_spec_deepness_time(uint32_t spec_deepness).
        * @brief It counts the total time spent to execute one speculation level on the 
        * current configuration.
        * @param spec_deepness - Current speculation depth.
        */          
        void array_count_spec_deepness_time(uint32_t spec_depth);        
        
        /**
        * @fn array_count_configuration_time(void).
        * @brief It counts the total time spent to execute the whole current configuration.
        */          
        void array_count_configuration_time(void);        
        
        /**
        * @fn  array_get_max_spec_depth(void).
        * @brief It returns the max value for speculation deepness
        * @return The current speculation depth.
        */        
        uint32_t array_get_max_spec_depth(void);     
       
        /**
        * @fn array_merge_conf_shape(void).
        * @brief If we are generating a new shape it'll perfom
        * a merge operation with the current shape generated by the current configuration
        * which was recently detected.
        */        
        void array_merge_conf_shape(void);            
            
        /**
        * @fn array_generate_final_shapes(void).
        * @brief It'll generates the final shapes (both the regular and the irregular)
        */                  
        void array_generate_final_shapes(void);            
            
        /**
        * @fn array_init_shape(void).
        * @brief It reset the shape counters.           
        */        
        void array_init_shape(void);  
};
#endif
