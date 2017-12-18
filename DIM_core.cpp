#include "DIM_core.h"
DIM_core::DIM_core()
{
    dim_array = NULL;
    dim_results = NULL;
    dim = NULL;
    /*===========MEMORIAS CACHE============*/
    cache_uni_dim = NULL;
    memories = NULL;
    /*=====================================*/
    
    count_instr = 1;
    core_number = 0;
    thread_number = 0;
}

DIM_core::DIM_core(char *argv,cache* ul2,pthread_mutex_t* bus_l1_to_l2,cache* ul3,pthread_mutex_t* bus_l2_to_l3)
{
    
    /*==VARIAVEIS PARA CONFIGURACAO DO ARRAY==*/
    uint32_t SPEC_DEPTH=0;   
    uint32_t INPUT_CONTEXT=0;
    /*========================================*/
    
    /*VARIAVEIS DE CONFIGURACAO DAS MEMORIAS CACHE*/	
    uint32_t bytes_block_uni_dim=0;	
    uint32_t cache_uni_size_dim=0;
    uint32_t cache_uni_associativity_dim=0;	
    uint32_t replace_method_uni_dim=0;    
    /*============================================*/   
    
    dim_array = NULL;
    dim_results = NULL;
    dim = NULL;
    /*===========MEMORIAS CACHE============*/
    cache_uni_dim = NULL;
    memories = NULL;
    
    /*=====================================*/

    /*variaveis parametros para o uso do shape*/    
   
    uint32_t shape_function = GENERATE_SHAPE;
    char shape_file_name[250] = "";    
   
        
    count_instr = 1;
   
    char *param_field = NULL;    
    
    param_field = strtok(argv," ");
    /*leitura dos campos da linha com os parametros para o array*/
    while(param_field)
    {
        
        if(param_field[0] == '-') 
        {
            switch(param_field[1])
            {
                case 'r':
                case 'R':
                {                    
                    param_field = strtok(NULL," ");
                    sscanf(param_field, "%u",&INPUT_CONTEXT);
                    break;
                }      
                case 'u':
                case 'U':
                {
                    /*Cache unificada L1
                    -UL1 [SIZE,ASSOCIATIVITY,BYTES_BLOCK,REPLACE_METHOD]*/
                    if(!strcmp(param_field,"-UL1"))
                    {                        
                        param_field = strtok(NULL," ");
                        sscanf(param_field, "[%u,%u,%u,%u]",&cache_uni_size_dim,&cache_uni_associativity_dim,&bytes_block_uni_dim,&replace_method_uni_dim);
                    }
                    break;
                }
                case 'c':
                case 'C':                   
                {
                    param_field = strtok(NULL," ");
                    sscanf(param_field, "%u",&core_number);
                    break;
                }                                                       							
                case 's':
                case 'S':
                {  
                    /*Shape Function*/                 
                    if(!strcmp(param_field,"-SFU"))
                    {
                        param_field = strtok(NULL," ");
                        sscanf(param_field, "%u",&shape_function);                    
                    }
                    /*Shape File*/                 
                    else if(!strcmp(param_field,"-SFI"))
                    {
                        param_field = strtok(NULL," ");
                        sscanf(param_field,"%s",shape_file_name);                          
                    }              
                                        
                    break;		
                }												
                case 'b':
                case 'B':
                {
                    /*Number of basic blocks*/
                    if(!strcmp(param_field,"-BB"))		
                    {
                        param_field = strtok(NULL," ");
                        sscanf(param_field, "%u",&SPEC_DEPTH);                        
                    }
                    break;
                }
            }
        }
        param_field = strtok(NULL," ");        
    }
    
    thread_number = core_number;
    
    printf("======Starting Core %u======\n",core_number);
    
   

    dim_array = new array(INPUT_CONTEXT,SPEC_DEPTH,shape_function,shape_file_name,core_number);   
    
    dim_array->array_print_array_status();

    bytes_block_uni_dim=dim_array->array_calculate_bytes_per_configuration();    
    cache_uni_dim = new cache(cache_uni_size_dim,cache_uni_associativity_dim,bytes_block_uni_dim,replace_method_uni_dim,UNIFIED_CACHE);    
    if(cache_uni_dim != NULL)
    {
        cache_uni_dim->cache_print_status();
        memories = new memory_interface(cache_uni_dim,ul2,bus_l1_to_l2,ul3,bus_l2_to_l3);
    }    
    dim_results = new results(core_number,memories);            
    dim = new DIM(memories,dim_array,dim_results);           
  
    return;    
    
}

DIM_core::~DIM_core()
{
    printf("==============SIMULATION END==============\n");
    printf("Core %u\n",core_number);
    #if defined(x_86)    
        printf("count_instr: %llu\n", count_instr);
    #elif defined(x_64)        
        printf("count_instr: %lu\n", count_instr);
    #endif  	

    /*======LIBERA A MEMORIA DOS OBJETOS INSTANCIADOS======*/
    if(dim_results != NULL)
    {
        delete dim_results;
    }    
    if(dim_array != NULL)
    {
        delete dim_array;
    }
    if(dim != NULL)
    {
        delete dim;
    }
    if(memories != NULL)
    {
        delete memories;
    }    
    return;
}

uint32_t DIM_core::DIM_core_process_trace_line(char *line)
{
    sscanf(line, "%s %s %s %s %s %s %s %s %s %s", field1, field2,field3,field4,field5,field6,field7,field8,field9,field10);
    trace_select=strcmp(field2,"[");
    if(trace_select==0)
    {    
        //poucas  
        strcpy(type,field1);
        strcpy(address_p,field7);
        strcpy(instruction,field8);       
    }
    else
    {
        //muitas        
        strcpy(type,field1);
        strcpy(address_p,field6);
        strcpy(instruction,field7);
    } 
        
    for(uint32_t i=5;i<21;i++)
    {
        address_temp[i-5]=address_p[i];
    }     
    #if defined (x_86)
        sscanf(address_temp, "%llx",&address_hex);
    #elif defined (x_64)
        sscanf(address_temp, "%lx",&address_hex);
    #endif           
        

    if(!strcmp(type, "inst:"))
    {        
        if(!(count_instr % 1000000))
        {
            printf("Core %u ",core_number);
            #if defined(x_86)
                printf("%llu instrucoes\n", count_instr);                
            #elif defined(x_64)
                printf("%lu instrucoes\n", count_instr);                                
            #endif
        }
        count_instr++;
         //--Coverting instruction to hexadecimal number--//
        sscanf(instruction, "%x",&instruction_hex);
        //-----------------------------------------------// 	        
            
        dim->DIM_decode_instruction(instruction_hex,address_hex,&uins);
        
        if(uins.instruction_type == NORMAL_INSTRUCTION)              
        {
            dim->DIM_fsm_memory_access(&uins);            
        }
        
        return uins.instruction_type;   
    }      
    return NORMAL_INSTRUCTION;
} 

void DIM_core::DIM_core_set_trace_name(char *trace_name_argv)
{
    strcpy(trace_name,trace_name_argv);
}
