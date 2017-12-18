/**
 * @file  results.cpp
 * @brief Implementation of the class results and its methods.
 * @author Rafael Fao de Moura.
 * @copyright GMICRO - UFSM - 2016.
*/

#include "results.h"

results::results()
{
    /*Set the memory interface pointer as NULL*/
    this->memories = NULL;
    
    /*Set the core results file pointer as NULL*/    
    this->results_trace_file = NULL;
}

results::results(uint32_t core_number_argv,memory_interface* memories_argv)
{
    char results_trace_str[256];
    char core_number_str[32];
    
    /*Updates the core number*/
    this->core_number = core_number_argv;
    
    /*Updates the reference to the memory interface*/
    this->memories = memories_argv;
        
    /*Creates the core results file*/
    strcpy(results_trace_str,"results_trace_");
    sprintf(core_number_str,"%u",core_number);
    strcat(results_trace_str,core_number_str);
    strcat(results_trace_str,".txt");    
    results_trace_file = fopen(results_trace_str,"w");
    
    barriers_counter = 1;
    
    /*Reset the performance metrics*/
    results_reset_counters();
}

results::~results()
{    
    /*Closes the core results file if it exists*/
    if(results_trace_file)
    {
        fclose(results_trace_file);
    }      
}

void results::results_inc_total_inst_on_array(INSTRUCTION_COUNTER_T instructions)
{
    /*Increases the number of instructions executed on array*/
    total_instructions_on_array+=instructions;
    
    #ifdef PRINT_DEBUG_RESULTS
        #ifdef x_86
            printf("RESULTS inc inst on array: %llu %llu\n",instructions,total_instructions_on_array);
        #elif defined x_64
            printf("RESULTS inc inst on array: %lu %lu\n",instructions,total_instructions_on_array);
        #endif
    #endif     
}

void results::results_inc_total_inst(INSTRUCTION_COUNTER_T instructions)
{
    /*Increases the total number of instructions executed on simulation*/
    total_instructions+=instructions;
}

void results::results_inc_total_cycles_on_array(CYCLE_COUNTER_T cycles)
{
    /*Increases the number of cycles executed on array*/
    total_cycles_on_array+=cycles;
    
    #ifdef PRINT_DEBUG_RESULTS
        #ifdef x_86
            printf("RESULTS inc cycles on array: %llu %llu\n",cycles,total_cycles_on_array);
        #elif defined x_64
            printf("RESULTS inc cycles on array: %lu %lu\n",cycles,total_cycles_on_array);
        #endif
    #endif     
}

void results::results_inc_total_power_on_array(POWER_T power)
{
    /*Increases the dissipated power on array*/
    total_power_on_array+=power;
    
    #ifdef PRINT_DEBUG_RESULTS
        printf("RESULTS inc power on array: %LF %LF\n",power,total_power_on_array);        
    #endif    
}        

void results::results_power_module() 
{
    /*POTENCIAS DIM*/    
    /*potencia dissipada no DIM*/
    POWER_T mips_power_with_array = 0;
    /*potencia dissipada no PC_TABLE*/
    POWER_T table_pc_power = 0;
        
    /*POTENCIAS STANDALONE*/        
    /*potencia dissipada no standalone*/
    POWER_T mips_power_without_array = 0;
    
    /*ENERGIAS DIM*/
    /*energia total gasta no DIM*/
    ENERGY_T total_energy_with_array = 0;    
    /*energia gasta no pelo array + MIPS*/
    ENERGY_T mips_energy_with_array = 0;
    /*energia gasta no array*/
    ENERGY_T dim_array_energy = 0;
    /*energia gasta no PC_TABLE*/
    ENERGY_T table_pc_energy = 0;
    /*energia gasta pelas memorias no DIM*/
    ENERGY_T mips_memory_energy_with_array = 0;	     
    
    /*ENERGIAS STANDALONE*/
    /*energia total gasta no standalone*/
    ENERGY_T total_energy_without_array = 0;    
    /*energia total gasta no processador standalone*/
    ENERGY_T mips_energy_without_array = 0;
    /*energia gasta pelas memorias no standalone*/
    ENERGY_T mips_memory_energy_without_array = 0;
    
    #ifdef PRINT_DEBUG_RESULTS
        printf("-----------------------------------------\n");
        printf("results.cpp::results_power_module\n");
    #endif
    
    /*CALCULO DA POTENCIA DISSIPADA NO DIM*/
    /*Table PC power*/
    table_pc_power = TABLE_PC*(((total_instructions - total_instructions_on_array)*SPARC_CPI));    
    /*Mips power*/
    mips_power_with_array = (((total_instructions - total_instructions_on_array)*SPARC_CPI)*(MIPS_POWER)) + total_power_on_array;
    
    #ifdef PRINT_DEBUG_RESULTS
        printf("table_pc_power %LF\n",table_pc_power);
        printf("power-on-array %LF\n",total_power_on_array);
        printf("mips_power_with_array %LF\n",mips_power_with_array);
    #endif    
    
    /*CALCULO DA POTENCIA DISSIPADA NO STANDALONE*/
    /*Mips power standalone*/
    mips_power_without_array = (total_instructions*SPARC_CPI) * (MIPS_POWER);  
    
    #ifdef PRINT_DEBUG_RESULTS  
        printf("mips_power_without_array %LF\n",mips_power_without_array);
    #endif

    /*CALCULO DA ENERGIA GASTA NO STANDALONE*/
    /*energia do processador*/
    mips_energy_without_array = (mips_power_without_array * 0.001)* (0.00000000166);
    /*energia das memorias*/
    mips_memory_energy_without_array = results_cache_standalone_energy();
    /*energia total*/
    total_energy_without_array = mips_energy_without_array + mips_memory_energy_without_array;
    
    #ifdef PRINT_DEBUG_RESULTS
        printf("mips_energy_without_array %LF\n",mips_energy_without_array);
        printf("mips_memory_energy_without_array %LF\n",mips_memory_energy_without_array);
        printf("total_energy_without_array %LF\n",total_energy_without_array);
    #endif
   
    /*CALCULO DA ENERGIA GASTA NO DIM*/
    /*energia do processador MIPS*/
    mips_energy_with_array = (mips_power_with_array * 0.001)* (0.00000000166);
    /*energia do array*/
    dim_array_energy = (total_power_on_array * 0.001)* (0.00000000166);
    /*energia do PC_TABLE*/
    table_pc_energy = (table_pc_power * 0.001) * (0.00000000166);
    /*energia das memorias*/
    mips_memory_energy_with_array = results_cache_dim_energy();
    /*energia total*/
    total_energy_with_array = mips_energy_with_array + dim_array_energy + table_pc_energy+ mips_memory_energy_with_array;
    
    #ifdef PRINT_DEBUG_RESULTS
        printf("mips_energy_with_array %LF\n",mips_energy_with_array);
        printf("dim_array_energy %LF\n",dim_array_energy);
        printf("table_pc_energy %LF\n",table_pc_energy);
        printf("mips_memory_energy_with_array %LF\n",mips_memory_energy_with_array);
        printf("total_energy_with_array %LF\n",total_energy_with_array);
    #endif    
    
    /*imprime os resultados no arquivo de resultados do core*/
    fprintf(results_trace_file, "PowerMIPS: %5.10F\n",0.0);
    fprintf(results_trace_file, "PowerArray+MIPS: %5.10F\n",0.0);
    fprintf(results_trace_file, "EnergyMIPSstandalone: %5.10LF\n",total_energy_without_array);
    fprintf(results_trace_file, "EnergyMIPS+Array: %5.10LF\n",total_energy_with_array);	
}

void results::results_gain_between_barriers(uint32_t results_type) 
{
    /*contadores dos ciclos do DIM*/
    CYCLE_COUNTER_T cycles_dim_cpu = 0;
    CYCLE_COUNTER_T cycles_dim_cache = 0;
    CYCLE_COUNTER_T total_cycles_dim = 0;  
    
    /*contadores dos ciclos do standalone*/
    CYCLE_COUNTER_T cycles_standalone_cpu = 0;    
    CYCLE_COUNTER_T cycles_standalone_cache = 0;
    CYCLE_COUNTER_T total_cycles_standalone = 0;      
    
    /*printa o cabecalho dos resultados no arquivo de saida  
    *de acordo com o tipo (resultados finais ou de uma barreira comum)*/
    if(results_type == NORMAL_RESULTS)
    {
        fprintf(results_trace_file,"BarrierS %u\n", barriers_counter);
    }
    else
    {
        fprintf(results_trace_file,"End 0\n");
    }
    
    /*incrementa o contador de barreiras*/
    barriers_counter++;
    
    /*se uma barreira foi inserida durante a execucao de uma configuracao,
    * ela quebrara a configuracao e todas as suas instrucoes e ciclos acabam
    * sendo contabilizados como parte da barreira anterior, o que pode gerar
    * inconsistencia quanto ao numero total de instrucoes executadas na simulacao*/
    if(total_instructions_on_array > total_instructions)
    {
        total_instructions_on_array = total_instructions;
    }
    
    /*atualiza os contadores de ciclos do standalone*/
    cycles_standalone_cpu = (CYCLE_COUNTER_T)(total_instructions*SPARC_CPI);
    cycles_standalone_cache = results_cache_standalone_cycles();		
    total_cycles_standalone = cycles_standalone_cpu + cycles_standalone_cache;	
			
    /*atualiza os contadores de ciclos do DIM*/
    cycles_dim_cpu = (CYCLE_COUNTER_T)(((total_instructions-total_instructions_on_array)*SPARC_CPI) + total_cycles_on_array);
    cycles_dim_cache = results_cache_dim_cycles();
    total_cycles_dim = cycles_dim_cpu + cycles_dim_cache;
			
    #if defined (PRINT_DEBUG_RESULTS)
        printf("--------------------------------------------------\n");
        printf("results.cpp::results_gain_between_barriers\n");
        #if defined (x_86)
            printf("ciclos cpu standalone %llu\n",cycles_standalone_cpu);
            printf("ciclos cache standalone %llu\n",cycles_standalone_cache);
            printf("total ciclos standalone %llu\n",total_cycles_standalone);
            printf("ciclos cpu dim %llu\n",cycles_dim_cpu);
            printf("ciclos cache dim %llu\n",cycles_dim_cache);
            printf("total ciclos dim %llu\n",total_cycles_dim);					
        #elif defined (x_64)
            printf("ciclos cpu standalone %lu\n",cycles_standalone_cpu);
            printf("ciclos cache standalone %lu\n",cycles_standalone_cache);
            printf("total ciclos standalone %lu\n",total_cycles_standalone);
            printf("ciclos cpu dim %lu\n",cycles_dim_cpu);
            printf("ciclos cache dim %lu\n",cycles_dim_cache);
            printf("total ciclos dim %lu\n",total_cycles_dim);					
        #endif
    #endif
	
    /*escreve no arquivo os valores calculados*/
    #if defined (x_86)		
        fprintf(results_trace_file,"Numerototaldeinstrucoesexecutadas: %llu\n",total_instructions);
        fprintf(results_trace_file,"Ciclostotalsemarray: %llu\n", total_cycles_standalone );
        fprintf(results_trace_file,"Numerodeinstrucoesnoarray: %llu\n",total_instructions_on_array);
        fprintf(results_trace_file,"Ciclosnoarray: %llu\n",total_cycles_on_array);
        fprintf(results_trace_file,"Ciclostotalcomarray: %llu\n",total_cycles_dim);//        
    #elif defined (x_64)
        fprintf(results_trace_file,"Numerototaldeinstrucoesexecutadas: %lu\n",total_instructions);
        fprintf(results_trace_file,"Ciclostotalsemarray: %lu\n",total_cycles_standalone);
        fprintf(results_trace_file,"Numerodeinstrucoesnoarray: %lu\n",total_instructions_on_array);
        fprintf(results_trace_file,"Ciclosnoarray: %lu\n",total_cycles_on_array);
        fprintf(results_trace_file,"Ciclostotalcomarray: %lu\n",total_cycles_dim);//		
    #endif
    
    /*calcula as metricas referentes a potencia e a energia*/
    results_power_module();		
} 
 
void results::results_reset_counters() 
{
    /*reseta os contadores de instrucoes, ciclos e potencia dissipada no array*/
    total_power_on_array = 0.0;
    total_cycles_on_array = 0;
    total_instructions_on_array = 0;
    
    /*reseta o contador total de instrucoes*/
    total_instructions = 0;   

    /*reseta os contadores da cache de configuracoes*/
    if(this->memories)	
    {
        this->memories->memory_interface_reset_counters();
    }
}

CYCLE_COUNTER_T results::results_cache_dim_cycles(void)
{
    if(this->memories)
    {
        return this->memories->memory_interface_get_cycles();
    }
    else
    {
        return 0;
    }
}
 
CYCLE_COUNTER_T results::results_cache_standalone_cycles(void)
{
    return 0;
}

ENERGY_T results::results_cache_dim_energy(void)
{
    if(this->memories)
    {
        return this->memories->memory_interface_get_energy()/1000000000;
    }
    else
    {
        return 0;
    }    
}

ENERGY_T results::results_cache_standalone_energy(void)
{
    return 0;
}
