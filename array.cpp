/**
 * @file  array.cpp
 * @brief Implementation for the array class.
 * @author Rafael Fao de Moura.
 * @copyright GMICRO - UFSM - 2016.
*/

#include "array.h"

array::array(void)
{
    /*Array configuration variables*/		
    ARRAY_ROWS_NUMBER = 0;										
    ARRAY_COLUMNS_NUMBER = 0;										
    ARRAY_MUL_COLUMNS_NUMBER = 0;									
    ARRAY_MEM_COLUMNS_NUMBER = 0;									
    ARRAY_FP_COLUMNS_NUMBER = 0;									
    INPUT_CONTEXT = 0;										
    SPEC_DEPTH = 0;	
	
    /*Array components*/
    array_bitmap = NULL;    
    regs_w = NULL;
    op_read_list = 0;    
    context = 0;
    context_used = 0;
    
    /*Shape components*/
    shape_array_used = NULL;
    shape_conf_used = NULL;    
}
	
array::~array(void)
{
    /*It releases the memory used by the write-table*/
    free(regs_w);
    
    /*It releases the array bitmap*/
    for(uint32_t i = 0;i<ARRAY_ROWS_NUMBER;i++)    
    {
        free(array_bitmap[i]);
    }    
    free(array_bitmap);    
    
    /*If we were generating a new shape, it'll release
    the memory used by shape's components*/
    if(shape_function == GENERATE_SHAPE)
    {    
        /*It generates the shape results*/
        array_generate_final_shapes();
            
        /*It releases the shape matrices*/
        for(uint32_t i = 0;i<ARRAY_ROWS_NUMBER;i++)    
        {
            free(shape_array_used[i]);
            free(shape_conf_used[i]);
        }    
        free(shape_array_used);
        free(shape_conf_used);          
    }    
}

array::array(uint32_t INPUT_CONTEXT_argv,uint32_t SPEC_DEPTH_argv,uint32_t shape_function_argv,char *shape_filename_argv,uint32_t shape_number_argv)
{    
    this->conf_detecting = configuration();

    /*Array configuration variables*/								
    INPUT_CONTEXT = INPUT_CONTEXT_argv;										
    SPEC_DEPTH = SPEC_DEPTH_argv;

    /*Shape components*/    
    FILE *shape_file = NULL;
    shape_function = shape_function_argv;
    shape_number = shape_number_argv;
    char shape_file_name[250] = "";
    strcpy(shape_file_name,shape_filename_argv);
    
    /*Variables used to read the shape file*/            
    char field1[250],field2[250],field3[250],field4[250],field5[250];
    char line[1024];
            
    /*It tries to open the shape file*/        
    if(!(shape_file = fopen(shape_file_name,"r")))
    {
        printf("It has failed to open the file %s\n",shape_file_name);
        exit(0);
    }
        
    /*It reads the number of rows*/
    fgets(line,sizeof(line),shape_file);         
    sscanf(line, "%u",&ARRAY_ROWS_NUMBER);        
        
    /*It reads the number of MEM, MUL, and FP columns*/
    fgets(line,sizeof(line),shape_file);        
    sscanf(line, "%s %s %s %s %s",field1,field2,field3,field4,field5);
    sscanf(field1, "%u",&ARRAY_COLUMNS_NUMBER);        
    sscanf(field3, "%u",&ARRAY_MEM_COLUMNS_NUMBER);
    sscanf(field4, "%u",&ARRAY_MUL_COLUMNS_NUMBER);
    sscanf(field5, "%u",&ARRAY_FP_COLUMNS_NUMBER);        
    
    /*Array's components*/    
    regs_w = (unsigned int*)malloc(ARRAY_ROWS_NUMBER * sizeof(unsigned int));                                   
    op_read_list = 0;  
    context = 0;
    context_used = 0;    
    array_bitmap = (unsigned int**)malloc(ARRAY_ROWS_NUMBER * sizeof(unsigned int*));  
    for(uint32_t row = 0; row < ARRAY_ROWS_NUMBER; row++)
    {
        array_bitmap[row]=  (unsigned int*)malloc(ARRAY_COLUMNS_NUMBER * sizeof(unsigned int));
    }
       
    /*It initializes the array matrix according to the shape function*/
    if(shape_function != LOAD_IRREGULAR_SHAPE)
    {
        for(uint32_t row  = 0; row < ARRAY_ROWS_NUMBER; row++)
        {
            for(uint32_t column = 0;column < ARRAY_COLUMNS_NUMBER; column++)
            {
                array_bitmap[row][column] = UF_EMPTY;
            }
        }           
    }
    else
    {
        for(uint32_t row = 0; row < ARRAY_ROWS_NUMBER; row++)
        {
            uint32_t alu,mem,mul,fp;
            alu = mem = mul = fp = 0;
            /*Reading ALU,MEM,MUL and FP columns number*/
            fgets(line,sizeof(line),shape_file);
            sscanf(line, "%s %s %s %s %s",field1,field2,field3,field4,field5);
            sscanf(field2, "%u",&alu);        
            sscanf(field3, "%u",&mem);
            sscanf(field4, "%u",&mul);
            sscanf(field5, "%u",&fp);           
                
            /*ALU allocation*/                
            for(uint32_t column = 0;column < ARRAY_COLUMNS_NUMBER - ARRAY_MUL_COLUMNS_NUMBER - ARRAY_MEM_COLUMNS_NUMBER - ARRAY_FP_COLUMNS_NUMBER; column++)
            {
                if(column < alu)
                {
                    array_bitmap[row][column] = UF_EMPTY;
                }
                else
                {
                    array_bitmap[row][column] = UF_VOID;
                }
            }                
                       
            /*MEM allocation*/ 
            mem = mem + ARRAY_COLUMNS_NUMBER - ARRAY_MUL_COLUMNS_NUMBER - ARRAY_MEM_COLUMNS_NUMBER - ARRAY_FP_COLUMNS_NUMBER; 
            for(uint32_t column = ARRAY_COLUMNS_NUMBER - ARRAY_MUL_COLUMNS_NUMBER - ARRAY_MEM_COLUMNS_NUMBER - ARRAY_FP_COLUMNS_NUMBER;column < ARRAY_COLUMNS_NUMBER - ARRAY_MUL_COLUMNS_NUMBER - ARRAY_FP_COLUMNS_NUMBER; column++)
            {
                if(column < mem)
                {
                    array_bitmap[row][column] = UF_EMPTY;
                }
                else
                {
                    array_bitmap[row][column] = UF_VOID;
                }
            }                   
            
            /*MUL allocation*/
            mul = mul + ARRAY_COLUMNS_NUMBER - ARRAY_MUL_COLUMNS_NUMBER - ARRAY_FP_COLUMNS_NUMBER;
            for(uint32_t column = ARRAY_COLUMNS_NUMBER - ARRAY_MUL_COLUMNS_NUMBER - ARRAY_FP_COLUMNS_NUMBER;column < ARRAY_COLUMNS_NUMBER - ARRAY_FP_COLUMNS_NUMBER; column++)
            {
                if(column < mul)
                {
                    array_bitmap[row][column] = UF_EMPTY;
                }
                else
                {
                    array_bitmap[row][column] = UF_VOID;
                }
            }                         
                
            /*FP allocation*/
            fp = fp + ARRAY_COLUMNS_NUMBER - ARRAY_FP_COLUMNS_NUMBER;
            for(uint32_t column = ARRAY_COLUMNS_NUMBER - ARRAY_FP_COLUMNS_NUMBER;column < ARRAY_COLUMNS_NUMBER; column++)
            {
                if(column < fp)
                {
                    array_bitmap[row][column] = UF_EMPTY;
                }
                else
                {
                    array_bitmap[row][column] = UF_VOID;
                }
            }                                                
        }        
    }   
     
    /*It closes the shape file*/    
    fclose(shape_file);
     
    /*If we will generate new shapes*/    
    if(shape_function == GENERATE_SHAPE)
    {
        /*Shape's components*/
        shape_array_used = (unsigned int**)malloc(ARRAY_ROWS_NUMBER * sizeof(unsigned int*));
        shape_conf_used = (unsigned int**)malloc(ARRAY_ROWS_NUMBER * sizeof(unsigned int*));
        for(uint32_t row = 0; row < ARRAY_ROWS_NUMBER; row++)		    							//alocacao das colunas do array
        {
            shape_array_used[row]=  (unsigned int*)malloc(5 * sizeof(unsigned int));
            shape_conf_used[row]=  (unsigned int*)malloc(5 * sizeof(unsigned int));
        }
        
        /*It initializes the shape detection mechanisms*/
        array_init_shape();    
    }    
    else
    {    
        /*Shape's components*/
        shape_array_used = NULL;
        shape_conf_used = NULL;                            
    }
	
    /*It resets the configuration that's being detected*/   
    array_reset_array();  
}

uint32_t  array::array_calculate_bytes_per_configuration(void)
{
    uint32_t TOTAL_BYTES=0;
    uint32_t ARRAY_N_ALUS=ARRAY_COLUMNS_NUMBER - (ARRAY_MUL_COLUMNS_NUMBER+ARRAY_MEM_COLUMNS_NUMBER+ARRAY_FP_COLUMNS_NUMBER);

    uint32_t UF1,UF2,UF3,MUX;
    uint32_t resource_table,reads_table,writes_table,context_start,context_current,imediate_table;
	
    UF1=ARRAY_ROWS_NUMBER*ARRAY_N_ALUS;
    UF2=(ARRAY_ROWS_NUMBER/3)*ARRAY_MEM_COLUMNS_NUMBER;
    UF3=(ARRAY_ROWS_NUMBER/3)*ARRAY_MUL_COLUMNS_NUMBER;
    MUX=2*(UF1+UF2+UF3);
	
    resource_table = 3*UF1 + UF2 + UF3;
    reads_table = MUX*4;
    writes_table = (INPUT_CONTEXT*ARRAY_FP_COLUMNS_NUMBER)*ARRAY_ROWS_NUMBER;
    context_start = INPUT_CONTEXT*5;
    context_current = context_start;
    imediate_table=128;
	
    TOTAL_BYTES=(resource_table+reads_table+writes_table+context_start+context_current+imediate_table)/8;	
	
    printf("Number of bytes required to store a configuration: %u",TOTAL_BYTES);
    if(TOTAL_BYTES > pow(2,array_log2(TOTAL_BYTES)))
    {       
        TOTAL_BYTES=pow(2,array_log2(TOTAL_BYTES)+1);		
        printf(" -> %u",TOTAL_BYTES);
    }	
    printf("\n");	
    return TOTAL_BYTES;	
}

void array::array_print_array_status(void)
{
    printf("-------------------------------------\n");
    printf("Array configuration\n");
    printf("Rows: %u\n",ARRAY_ROWS_NUMBER);
    printf("Columns: %u\n",ARRAY_COLUMNS_NUMBER);
    printf("ALU: %u\n",ARRAY_COLUMNS_NUMBER - (ARRAY_MUL_COLUMNS_NUMBER+ARRAY_MEM_COLUMNS_NUMBER+ARRAY_FP_COLUMNS_NUMBER));
    printf("MUL: %u\n",ARRAY_MUL_COLUMNS_NUMBER);
    printf("MEM: %u\n",ARRAY_MEM_COLUMNS_NUMBER);
    printf("FP: %u\n",ARRAY_FP_COLUMNS_NUMBER);   
}

uint32_t array::array_log2(uint32_t number)
{
    uint32_t bit;
    for(bit = 31; bit > 0; bit --)
    {
        if((number >> bit))
        {
            return bit;
        }
    }
    return 0;	
}

uint64_t array::array_log2(uint64_t number)
{
    uint64_t bit;
    for(bit = 63; bit > 0; bit --)
    {
        if((number >> bit))
        {
            return bit;
        }
    }
    return 0;	
}

void array::array_reset_array(void)
{    	
    /*It resets the componets which are indexed by speculation level*/
    this->conf_detecting.configuration_reset_configuration();    
    context = 0;
    context_used = 0;
    op_read_list = 0;
    
    /*It resets the componets which depend on the number of rows and columns*/
    for(uint32_t row = 0; row < ARRAY_ROWS_NUMBER; row++)
    {        
        regs_w[row] = 0;               
        for(uint32_t column = 0; column < ARRAY_COLUMNS_NUMBER; column++)
        {
            if(array_bitmap[row][column] == UF_ALOCATED)
            {
                array_bitmap[row][column] = UF_EMPTY;
            }            
        }    
        if(shape_function == GENERATE_SHAPE)
        {
            for(uint32_t column = 0;column < 5; column++)
            {
                shape_conf_used[row][column] = 0;
            }
            shape_conf_biggest_line = 0;
        }        
    }    
}

void array::array_create_new_config(ADDRESS_T pc)
{
    /*It resets the current configuration that was being detected*/
    array_reset_array();
    
    /*It attributes a new pc index value for the current configuration*/
    conf_detecting.first_pc = pc;	    
}

bool array::array_add_inst_into_array(micro_instruction_t *uins,uint32_t current_spec_depth)
{    

    /*Variables used to perform the resources allocation in the array*/
    uint32_t beg_column,end_column,line,dest_line,delay;
    beg_column = end_column = line = dest_line = delay = 0;  
    uint32_t temp_context = context;  
    uint32_t temp_op_read_list = op_read_list;
	
    /*If the instruction is a NOP, it just add it as a normal instrcution and returns*/
    if(uins->instr == 0x01000000) 
    {
        conf_detecting.total_instructions++;
        conf_detecting.instructions[current_spec_depth]++;
        return true;			
    }
	
    /*It add the operands used by the instruction into the configuration's context*/
    if(uins->op_r1)
    {      
        /*If this register wasn't added into the context yet, it'll add it*/
        if(!(temp_context &  (1<< uins->op_r1) ))
        {
            temp_context |=  (1<< uins->op_r1); 
            temp_op_read_list |=  (1<< uins->op_r1);           
            context_used++;
        }        
    }
    
    /*It add the operands used by the instruction into the configuration's context*/
    if (uins->op_r2)
    {    
        /*If this register wasn't added into the context yet, it'll add it*/
        if(!(temp_context &  (1<< uins->op_r2) ))
        {
            temp_context |=  (1<< uins->op_r2);  
            temp_op_read_list |=  (1<< uins->op_r2);          
            context_used++;
        }        
    }
    
    /*It add the operands used by the instruction into the configuration's context*/
    if(uins->op_w)
    {
        /*If this register wasn't added into the context yet, it'll add it*/    
        if(!(temp_context &  (1<< uins->op_w) ))
        {
            temp_context |=  (1<< uins->op_w);
            context_used++;
        }
    }

    /*It checks if we have exceeded the context*/
    if(context_used > INPUT_CONTEXT) 
    {
        return false;        
    }
    	
    /*It searches for the biggest row that doesn't have dependencies with the r1 register*/
    if(uins->op_r1) 
    {        
        for(line = 0; line < ARRAY_ROWS_NUMBER; line++) 
        {
            if(!(array_there_is_dependencies(uins->op_r1,line))) 
            {
                dest_line = line + 1;
            }
        }
    }

    /*It searches for the biggest row that doesn't have dependencies with the r2 register*/
    if(uins->op_r2) 
    {
        for(line = 0; line < ARRAY_ROWS_NUMBER; line++) 
        {
            if (!(array_there_is_dependencies(uins->op_r2,line))) 
            {
                if ((line + 1) > dest_line) 
                {
                    dest_line = line + 1;
                }
            }
        }               
    } 

    /*It checks if we have exceeded the rows*/
    if (dest_line >= (ARRAY_ROWS_NUMBER)) 
    {                            
        return false;
    } 
	
    /*It calculates the column range that this instruction can be allocated depending on its
    UF type*/
    switch(uins->group) 
    {
        case ALU_TYPE:
        {
            beg_column = 0;
            end_column = ARRAY_COLUMNS_NUMBER - ARRAY_MUL_COLUMNS_NUMBER - ARRAY_MEM_COLUMNS_NUMBER - ARRAY_FP_COLUMNS_NUMBER;           
            break;
        }    
        case MEM_TYPE:
        {
            beg_column = ARRAY_COLUMNS_NUMBER - ARRAY_MUL_COLUMNS_NUMBER - ARRAY_MEM_COLUMNS_NUMBER - ARRAY_FP_COLUMNS_NUMBER;
            end_column = ARRAY_COLUMNS_NUMBER - ARRAY_MUL_COLUMNS_NUMBER - ARRAY_FP_COLUMNS_NUMBER;            
            break;
        }        
        case MUL_TYPE:
        {
            beg_column = ARRAY_COLUMNS_NUMBER - ARRAY_MUL_COLUMNS_NUMBER - ARRAY_FP_COLUMNS_NUMBER;
            end_column = ARRAY_COLUMNS_NUMBER - ARRAY_FP_COLUMNS_NUMBER;            
            break;
        }        
        case FP_TYPE:
        {
            beg_column = ARRAY_COLUMNS_NUMBER - ARRAY_FP_COLUMNS_NUMBER;
            end_column = ARRAY_COLUMNS_NUMBER;            
            break;
        }
        default:
        {
            printf("Not supported UF\n");
            exit(1);
            break;
        }	
    }       
    
    uint32_t include = 1;   
    uint32_t i,j; 
    
    /*It determines the column where the instruction can be (or not) allocated*/
    do 
    {           
        for(j = beg_column; j < end_column; j++) 
        {             
            /*Depending on the UF type, each instruction has a different delay*/  
            if(uins->group == ALU_TYPE) 
            {
                delay = 0;
            }
            else
            {
                while(dest_line % 3)
                {
                    dest_line++;
                }
                delay = 2;            
            }           

            if( (dest_line < ARRAY_ROWS_NUMBER) && (array_bitmap[dest_line][j] == UF_EMPTY)) 
            {
                /*Depending on the UF type, each instruction has a different delay*/
                for (i = 0; i <= delay; i++)
                {
                    array_bitmap[dest_line + i][j] = UF_ALOCATED;                    
                    
                    if(shape_function == GENERATE_SHAPE)
                    {
                        shape_conf_used[dest_line +i][uins->group]++;
                        shape_conf_used[dest_line +i][0]++;
                    }                    
                }        
                dest_line += delay;         
                if(uins->op_w)
                {
                    regs_w[dest_line] |= 1 << uins->op_w;       
                }
               
                
                if(shape_function == GENERATE_SHAPE)
                {
                    if(  (dest_line + 1) > shape_conf_biggest_line )
                    {
                        shape_conf_biggest_line = dest_line + 1;
                    }                
                }
                                                     		
                dest_line--;                
                include = 0;
                break;
            }
        }
        dest_line++;
        if ( dest_line >= ARRAY_ROWS_NUMBER) 
        {                              
            return false;
        }
    }while(include);
    
    /*Depending on its instruction type, it updates the total power spent
    due to this configuration execution*/
    switch(uins->group) 
    {
        case ALU_TYPE:
        {       
            conf_detecting.power+= ALU_POWER;           
            break;
        }  
        case MEM_TYPE:
        {         
            conf_detecting.power+= MEM_POWER;         
            break;
        }    
        case MUL_TYPE:
        {           
            conf_detecting.power+= MUL_POWER;          
            break;
        }              
        case FP_TYPE:
        {   
            conf_detecting.power+= FP_ALU_POWER;           
            break;
        }
        default:
        {
            printf("Not supported UF\n");
            exit(1);
            break;
        }	
    }   
    
    /*It updates the context*/
    context = temp_context;
    
    /*It updates the input context*/ 
    op_read_list = temp_op_read_list;
    
    /*It increases the configuration instructions number*/
    conf_detecting.total_instructions++;
    
    /*It increases the current configuration speculation's level instructions number*/
    conf_detecting.instructions[current_spec_depth]++;          
    
    this->conf_detecting.last_pc = uins->address;
    
    return true;        
}

configuration* array::array_store_current_config(void)
{
    /*It copies the current configuration*/
    configuration* new_conf = NULL;    
    new_conf =  this->conf_detecting.configuration_copy_configuration();
    
    /*If we are generating a new shape, performs a merge
    operation with the biggest values of UF used until now and this 
    current configuration's values*/
    if(shape_function == GENERATE_SHAPE)
    {
        array_merge_conf_shape();
    }
        
    /*It returns a pointer to this copy*/    
    return new_conf;
}

bool array::array_there_is_dependencies(uint32_t op,uint32_t row)
{
    /*If the register op is not written in the indicated line,
    it means that there insn't a real dependencie on this array row*/
    if (((regs_w[row]) & (1 << op)) != 0)
    {
        return false;
    } 
    else 
    {
        return true;
    }    
}

CYCLE_COUNTER_T array::array_count_exec_time(void) 
{
    uint32_t slot = 0;
    CYCLE_COUNTER_T cycle=0;
    CYCLE_COUNTER_T temp_exec_time = 0;
    
    /*For each three rows used in the array, it'll count 
    one cycle in the total execution time cycles*/    
    for (uint32_t i = 0; i < ARRAY_ROWS_NUMBER; i++) 
    {
        if ((i % 3) == 0) 
        {
            cycle=0;
        }
        for (uint32_t j = 0; j < ARRAY_COLUMNS_NUMBER; j++) 
        {            
            if (array_bitmap[i][j] == UF_ALOCATED)
            {
                slot = 1;
                break;
            }
        }
        cycle++;
        if (cycle == 3) 
        {
            if (slot == 1)
            {
                temp_exec_time++;
            }
            else
            {
                break;
            }
            slot = 0;
        }
    }   
  
    return temp_exec_time;
}

CYCLE_COUNTER_T array::array_count_reconf_time(void) 
{
    CYCLE_COUNTER_T temp_final_rec = 0, temp_rec = 0;   
    
    /*It counts the number of registers that will be read*/
    for (uint32_t j = 0; j < 32; j++) 
    {
        if (((op_read_list >> j) & 0x1) != 0) 
        {
            temp_rec++;
        }
    }
    
    /*The number of cycles required to load the input registers
    depends on the number of ports in the resgister file*/
    temp_rec = (temp_rec / N_ARRAY_WB);
    
    /*If it wasn't a perfect division operation, it'll sum more one cycle*/
    if(temp_rec % N_ARRAY_WB)
    {
        temp_rec++;
    }
    
    /*It discounts the time due to Bynary Translation's pipeline*/
    if ( (int64_t)(temp_rec - N_TIME_RECONF) <= 0)   
    {
        temp_final_rec = 0;
    }
    else
    {
        temp_final_rec = temp_rec - N_TIME_RECONF;
    }
       
    return temp_final_rec;
}

CYCLE_COUNTER_T array::array_count_writeback_time(void)
{
    CYCLE_COUNTER_T temp_final_wb = 0;
    uint32_t regs_to_wb = 0;
    uint32_t regs_to_wb_count = 0;
    
    /*It makes a list with contains all register that have
    to perform a write back operation*/
    for(uint32_t row = 0; row < ARRAY_ROWS_NUMBER;row++)
    {
        regs_to_wb |= regs_w[row];        
    }
    
    /*It counts how many registers have to be
    written back*/
    for(uint32_t bit = 0;bit < 32 ; bit++)    
    {
        if(regs_to_wb & (1 << bit))
        {
            regs_to_wb_count++;
        }
    }
    
    /*The number of cycles required to write back registers
    depends on the number of ports in the resgister file*/    
    temp_final_wb = regs_to_wb_count / N_ARRAY_WB;
    
    /*If it wasn't a perfect division operation, it'll sum one cycle*/    
    if(regs_to_wb_count % N_ARRAY_WB)
    {
        temp_final_wb++;
    }    

    return temp_final_wb;  
}

void array::array_count_spec_deepness_time(uint32_t spec_depth)
{
    CYCLE_COUNTER_T wb_time = 0;    
    
    /*It calculates the writeback time until the current speculation level*/
    wb_time = array_count_writeback_time();
   
    /*It updates the current speculation level's writeback time*/   
    conf_detecting.wb_cycles[spec_depth] = wb_time;
        
    /*It discounts the writeback time from the previuos speculation levels*/
    for(uint32_t i = 0; i<spec_depth; i++)
    {
        wb_time -= conf_detecting.wb_cycles[i];
    }
    
    /*It updates the current speculation level's total time with the writeback time*/
    conf_detecting.total_cycles[spec_depth] = wb_time;
}

void array::array_count_configuration_time(void)
{    
    CYCLE_COUNTER_T reconf_time = 0;
    CYCLE_COUNTER_T exec_time = 0;
    
    /*It calculates the execution time of the whole configuration*/
    exec_time = array_count_exec_time();
    
    /*It calculates the reconfiguration time*/    
    reconf_time = array_count_reconf_time(); 
    
    /*It sums at the first speculation time both the reconfiguration time
    and execution time*/   
    conf_detecting.total_cycles[0]+=reconf_time + exec_time;
}

uint32_t array::array_get_max_spec_depth(void)
{
    /*It returns the max spec depth allowed on this current simulation*/
    return SPEC_DEPTH;
}

void array::array_merge_conf_shape(void)
{
    /*soma do numero de UF's utilizadas em cada row do array*/
    uint32_t sum_columns;
        
    /*atualiza o numero da maior row utilizada*/
    if(shape_conf_biggest_line > shape_array_biggest_line)
    {
        shape_array_biggest_line = shape_conf_biggest_line;
    }
        
    /*Verifica todas as rows do shape comparando os valores do uso
    * de UF's no shape da configuracao em relacao ao shape global do array*/
    for(uint32_t row = 0;row < shape_conf_biggest_line; row++)
    {
        /*resseta o contador de UF's utilizadas nesta row*/
        sum_columns = 0;
            
        /*Percorre os grupos de unidades funcionais [ALU][MEM][MUL][FP]
        * comparando os valores do shape global com o gerado pela
        * configuracao atual.*/
        for(uint32_t column = 1; column <5; column++)
        {
            if(shape_array_used[row][column] < shape_conf_used[row][column])
            {
                shape_array_used[row][column] = shape_conf_used[row][column];
            }
            sum_columns += shape_array_used[row][column];
        }
            
        /*atualiza o contador de UF's utilizadas nesta row*/
        shape_array_used[row][0] = sum_columns;
    }
}
           
void array::array_generate_final_shapes(void)
{
    /*Results files*/
    FILE *irregular_file = NULL;
    FILE *regular_file = NULL;
        
    /*Strings with the results files' names*/
    char irregular_file_name[250] = "";
    char regular_file_name[250] = "";
        
    /*Vector used in the regular shape generation*/
    uint32_t biggest_uf[4] = {0,0,0,0};
        
    /*Offset to make the rows number as a multiple of three*/
    uint32_t offset_lines = 0;
    if(shape_array_biggest_line % 3)
    {
        offset_lines = 3 - (shape_array_biggest_line % 3);
    }
        
    /*It open the results files*/
    sprintf(irregular_file_name,"irregular_shape_%u.txt",shape_number);
    sprintf(regular_file_name,"regular_shape_%u.txt",shape_number);        
      
    irregular_file = fopen(irregular_file_name,"w");
    regular_file = fopen(regular_file_name,"w");
        
    /*It stores the rows number of the regular and irregular shape*/
    fprintf(irregular_file,"%u\n",shape_array_biggest_line + offset_lines);
    fprintf(regular_file,"%u\n",shape_array_biggest_line + offset_lines);
        
    /*It updates the biggest_uf values*/
    for(uint32_t row = 0; row < shape_array_biggest_line; row++)
    {           
        for(uint32_t column = 1;column < 5; column++)
        {
             if(biggest_uf[column -1] < shape_array_used[row][column])
             {
                 biggest_uf[column -1] = shape_array_used[row][column];
             }
        }
    }        
        
    /*It prints the irregular shape's results file*/
    fprintf(irregular_file,"%u %u %u %u %u\n",biggest_uf[0] + biggest_uf[1] + biggest_uf[2] + biggest_uf[3],biggest_uf[0],biggest_uf[1],biggest_uf[2],biggest_uf[3]);
    for(uint32_t row = 0; row < shape_array_biggest_line; row++)
    {        
        /*sintaxe: TOTAL_UFS ALU MEM MUM FP*/
        fprintf(irregular_file,"%u %u %u %u %u\n",shape_array_used[row][0],shape_array_used[row][1],shape_array_used[row][2],shape_array_used[row][3],shape_array_used[row][4]);                      
    }         
     
    /*It prints the regular shape's results file*/        
    /*sintaxe: TOTAL_UFS ALU MEM MUM FP*/
    fprintf(regular_file,"%u %u %u %u %u\n",biggest_uf[0] + biggest_uf[1] + biggest_uf[2] + biggest_uf[3],biggest_uf[0],biggest_uf[1],biggest_uf[2],biggest_uf[3]);        
              
    /*It completes the rows offset*/
    for(uint32_t line = 0; line < offset_lines; line++)
    {
        /*Irregular -> sintaxe: TOTAL_UFS ALU MEM MUM FP*/
        fprintf(irregular_file,"%u %u %u %u %u\n",shape_array_used[shape_array_biggest_line -1][0],shape_array_used[shape_array_biggest_line -1][1],shape_array_used[shape_array_biggest_line -1][2],shape_array_used[shape_array_biggest_line -1][3],shape_array_used[shape_array_biggest_line -1][4]);        
    }
    
    /*It closes the files*/         
    fclose(irregular_file);
    fclose(regular_file);
}
            
void array::array_init_shape(void)          
{
    shape_array_biggest_line = 0;
    shape_conf_biggest_line = 0;
 
    /*It resets all the shape's counters*/       
    for(uint32_t row = 0; row < ARRAY_ROWS_NUMBER; row++)
    {
        for(uint32_t column = 0; column < 5; column++)
        {
            shape_array_used[row][column] = 0;
            shape_conf_used[row][column] = 0;
        }
    }
}                      

