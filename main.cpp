#include <iostream>
#include <zlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "DIM_core.h"
#include <pthread.h>

#define MAX_CORES 256

#define STATIC_SCHEDULER 0

void* static_scheduler_function(void * argv);
void generate_cache_statistics(uint32_t cores,uint32_t l2_number,uint32_t l3_number);

pthread_mutex_t mutex_sync;
uint32_t can_execute[MAX_CORES];
pthread_mutex_t mutex_can_execute[MAX_CORES];
pthread_mutex_t mutex_bus_l1_to_l2[MAX_CORES];
pthread_mutex_t mutex_bus_l2_to_l3[MAX_CORES];
cache* cache_l2_vector = NULL;
cache* cache_l3_vector = NULL;
pthread_mutex_t mutex_finish;
pthread_cond_t var_cond_threads[MAX_CORES];
pthread_cond_t var_cond_main;
pthread_barrier_t barrier;
uint32_t syncronized = 0;
uint32_t finish = 0;
uint32_t total_cores = 0;
gzFile_s *traces[MAX_CORES];

uint64_t *l1_instructions_accesses_counter = NULL;
uint64_t *l1_configurations_accesses_counter = NULL;
uint64_t *l1_instructions_hits_counter = NULL;
uint64_t *l1_configurations_hits_counter = NULL;

int main(int ac, char *av[])
{ 
    DIM_core *cpus_dim[MAX_CORES] = {NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};
    pthread_t threads[MAX_CORES];
    FILE *args_file = NULL;    
    cache* temp_l2 = NULL;
    cache* temp_l3 = NULL;
    
    uint32_t ul2_unified_cache_size; 
    uint32_t ul2_unified_cache_associativity;
    uint32_t ul2_unified_cache_block_size;    
    uint32_t ul2_unified_cache_replacement_method;     
    uint32_t ul2_unified_cores_per_cache;  
    
    uint32_t ul3_unified_cache_size; 
    uint32_t ul3_unified_cache_associativity;
    uint32_t ul3_unified_cache_block_size;    
    uint32_t ul3_unified_cache_replacement_method;     
    uint32_t ul3_unified_cores_per_cache;      
    
    char line_buffer[1024];    
    uint32_t scheduler_id = 0;    
    
    args_file = fopen(av[1],"r");
    if(!args_file)
    {
        printf("arquivo de configuracoes inexistente!!\n");
        return 1;
    }
    
    /*Leitura da configuração das caches L2*/
    fgets(line_buffer,sizeof(line_buffer),args_file);
    sscanf(line_buffer, "-UL2 [%u,%u,%u,%u,%u]",&ul2_unified_cache_size,&ul2_unified_cache_associativity,&ul2_unified_cache_block_size,&ul2_unified_cache_replacement_method,&ul2_unified_cores_per_cache);       

    /*Leitura da configuração das caches L3*/
    fgets(line_buffer,sizeof(line_buffer),args_file);
    sscanf(line_buffer, "-UL3 [%u,%u,%u,%u,%u]",&ul3_unified_cache_size,&ul3_unified_cache_associativity,&ul3_unified_cache_block_size,&ul3_unified_cache_replacement_method,&ul3_unified_cores_per_cache);       
    
    /*leitura do numero de processadores*/
    fgets(line_buffer,sizeof(line_buffer),args_file);
    sscanf(line_buffer,"%u",&total_cores);
    
    /*Criação dos vetores com as taxas de hit nas caches L1*/    
    l1_instructions_accesses_counter = new unsigned long int[total_cores];
    l1_configurations_accesses_counter = new unsigned long int[total_cores];
    l1_instructions_hits_counter = new unsigned long int[total_cores];
    l1_configurations_hits_counter = new unsigned long int[total_cores];    
    
   
    cache_l2_vector = new cache[total_cores/ul2_unified_cores_per_cache];
    /*Criação das UL2*/
    for(uint32_t counter_l2 = 0;counter_l2 < (total_cores/ul2_unified_cores_per_cache); counter_l2 ++)
    {
        if(ul2_unified_cache_size)
        {
            temp_l2 = new cache(ul2_unified_cache_size,ul2_unified_cache_associativity,ul2_unified_cache_block_size,ul2_unified_cache_replacement_method,UNIFIED_CACHE);            
            temp_l2->cache_print_status();
        }
        else
        {
            temp_l2 = NULL;
        }
        cache_l2_vector[counter_l2] = *(temp_l2);
    }
    
     
    cache_l3_vector = new cache[total_cores/ul3_unified_cores_per_cache];
    /*Criação das UL3*/
    for(uint32_t counter_l3 = 0;counter_l3 < (total_cores/ul3_unified_cores_per_cache); counter_l3 ++)
    {
        if(ul3_unified_cache_size)
        {
            temp_l3 = new cache(ul3_unified_cache_size,ul3_unified_cache_associativity,ul3_unified_cache_block_size,ul3_unified_cache_replacement_method,UNIFIED_CACHE);            
            temp_l3->cache_print_status();
        }
        else
        {
            temp_l3 = NULL;
        }
        cache_l3_vector[counter_l3] = *(temp_l3);
    }    
    
    /*leitura dos argumentos para os cores e criacao dos mesmos*/
    for(uint32_t core = 0;core < total_cores; core++)
    {       
        fgets(line_buffer,sizeof(line_buffer),args_file);
        cpus_dim[core] = new DIM_core(line_buffer,&cache_l2_vector[core/(ul2_unified_cores_per_cache)],&mutex_bus_l1_to_l2[core/(ul2_unified_cores_per_cache)],&cache_l3_vector[core/(ul3_unified_cores_per_cache)],&mutex_bus_l2_to_l3[core/(ul3_unified_cores_per_cache)]);
    }
    
    /*leitura do identificador do escalonador*/
    fgets(line_buffer,sizeof(line_buffer),args_file);
    sscanf(line_buffer,"%u",&scheduler_id);    
    
    /*toma a acao de acordo com o escalonador utilizado*/
    switch(scheduler_id)
    {
        /*escalonador estatico*/
        case STATIC_SCHEDULER:
        {
            char trace_name[512];
        
            /*le a localizacao dos traces compactados de cada core e cria
            as threads que executarao cada core*/
            for(uint32_t core = 0;core < total_cores;core++)
            {
                /*obtem a localizacao do arquivo de trace compactado*/
                fgets(line_buffer,sizeof(line_buffer),args_file);
                sscanf(line_buffer,"%s",trace_name);
                cpus_dim[core]->DIM_core_set_trace_name(trace_name);
                
                /*cria a thread*/
                pthread_create(&threads[core], NULL,static_scheduler_function, (void *)cpus_dim[core]);
                //static_scheduler_function((void *)cpus_dim[core]);
            }
            /*fecha o arquivo com os argumentos para o simulador*/           
            fclose(args_file);           
                        
            /*espera ate que todas as threads tenham terminado*/
            for(uint32_t core = 0;core < total_cores;core++)
            {
                pthread_join(threads[core],NULL); 
            }                        
           
            break;
        }             
        default:
        {
            printf("escalonador nao definido\n");
            return 1;
        }
    }   
   
    generate_cache_statistics(total_cores,total_cores/ul2_unified_cores_per_cache,total_cores/ul3_unified_cores_per_cache);
   
    delete[] l1_instructions_accesses_counter;
    delete[] l1_configurations_accesses_counter;
    delete[] l1_instructions_hits_counter;
    delete[] l1_configurations_hits_counter;
    
    pthread_exit(NULL);    
   
    return 0;
}

void* static_scheduler_function(void * argv)
{
    DIM_core *core = (DIM_core*)argv;
    char buffer[2048];          
    
    buffer[0] = 0;
    if(!(traces[core->thread_number] = gzopen(core->trace_name,"r")))
    {
        printf("Nao foi possivel abrir o arquivo %s\n",core->trace_name);
    
        /*deleta o core*/
        delete core;
    
        /*retorna*/
        pthread_exit(NULL);        
        return NULL;
    } 
    
    while(1)
    {
        /*le uma linha do arquivo da thread*/
        gzgets(traces[core->thread_number],buffer,2047);
        
        /*ainda não chegou ao fim do arquivo*/
        if(buffer[0])
        {       
            if(core->DIM_core_process_trace_line(buffer) == BARRIER_INSTRUCTION)
            {
                core->dim_results->results_gain_between_barriers(NORMAL_RESULTS);         
                core->dim_results->results_reset_counters();                
            }            
        }
        /*fim do arquivo*/
        else
        {
            break;    
        }
        
        /*limpa o buffer*/
        buffer[0] = 0;
    }
    
    /*fecha o arquivo do trace*/
    gzclose(traces[core->thread_number]); 
    
    /*contabiliza as metricas de desempenho para as instrucoes executadas
    * apos a ultima barreira de sincronizacao*/
    core->dim_results->results_gain_between_barriers(END_RESULTS);          
    
    l1_instructions_accesses_counter[core->thread_number] = core->cache_uni_dim->instructions_accesses_counter;
    l1_configurations_accesses_counter[core->thread_number] = core->cache_uni_dim->configurations_accesses_counter;    
    l1_instructions_hits_counter[core->thread_number] = core->cache_uni_dim->instructions_hits_counter;
    l1_configurations_hits_counter[core->thread_number] = core->cache_uni_dim->configurations_hits_counter;
        
    /*deleta o core*/
    delete core;
    
    /*retorna*/
    pthread_exit(NULL);
    return NULL;
}

void generate_cache_statistics(uint32_t cores,uint32_t l2_number,uint32_t l3_number)
{
    FILE* results_cache = NULL;

    uint64_t l1_max_instructions_accesses_counter = 0;
    uint64_t l1_max_configurations_accesses_counter = 0;
    uint64_t l1_max_instructions_hits_counter = 0;
    uint64_t l1_max_configurations_hits_counter = 0;

    uint64_t l2_max_instructions_accesses_counter = 0;
    uint64_t l2_max_configurations_accesses_counter = 0;
    uint64_t l2_max_instructions_hits_counter = 0;
    uint64_t l2_max_configurations_hits_counter = 0;
    
    uint64_t l3_max_instructions_accesses_counter = 0;
    uint64_t l3_max_configurations_accesses_counter = 0;
    uint64_t l3_max_instructions_hits_counter = 0;
    uint64_t l3_max_configurations_hits_counter = 0;            
    
    for(uint32_t counter = 0;counter<cores;counter++)    
    {
        if(l1_instructions_accesses_counter[counter] > l1_max_instructions_accesses_counter)
        {
            l1_max_instructions_accesses_counter = l1_instructions_accesses_counter[counter];
            l1_max_instructions_hits_counter = l1_instructions_hits_counter[counter];
        }
        if(l1_configurations_accesses_counter[counter] > l1_max_configurations_accesses_counter)
        {
            l1_max_configurations_accesses_counter = l1_configurations_accesses_counter[counter];
            l1_max_configurations_hits_counter = l1_configurations_hits_counter[counter];
        }        
    } 
    
    for(uint32_t counter = 0;counter<l2_number;counter++)    
    {        
        if(cache_l2_vector[counter].instructions_accesses_counter > l2_max_instructions_accesses_counter )
        {
            l2_max_instructions_accesses_counter = cache_l2_vector[counter].instructions_accesses_counter;
            l2_max_instructions_hits_counter = cache_l2_vector[counter].instructions_hits_counter;
        }
        if(cache_l2_vector[counter].configurations_accesses_counter > l2_max_configurations_accesses_counter)
        {
            l2_max_configurations_accesses_counter = cache_l2_vector[counter].configurations_accesses_counter;
            l2_max_configurations_hits_counter = cache_l2_vector[counter].configurations_hits_counter;
        }
    } 
    
    for(uint32_t counter = 0;counter<l3_number;counter++)    
    {
        if(cache_l3_vector[counter].instructions_accesses_counter > l3_max_instructions_accesses_counter)
        {
            l3_max_instructions_accesses_counter = cache_l3_vector[counter].instructions_accesses_counter;
            l3_max_instructions_hits_counter = cache_l3_vector[counter].instructions_hits_counter;
        }
        if(cache_l3_vector[counter].configurations_accesses_counter > l3_max_configurations_accesses_counter)
        {
            l3_max_configurations_accesses_counter = cache_l3_vector[counter].configurations_accesses_counter;
            l3_max_configurations_hits_counter = cache_l3_vector[counter].configurations_hits_counter;
        }
    }             
   

    results_cache = fopen("results_cache.txt","w");    

    fprintf(results_cache,"Sintaxe: INSTRUCTIONS ACCESSES,INSTRUCTIONS HITS,CONFIGURATIONS ACCESSES,CONFIGURATIONS HITS\n");
    fprintf(results_cache,"%lu,%lu,%lu,%lu\n",l1_max_instructions_accesses_counter,l1_max_instructions_hits_counter,l1_max_configurations_accesses_counter,l1_max_configurations_hits_counter);
    fprintf(results_cache,"%lu,%lu,%lu,%lu\n",l2_max_instructions_accesses_counter,l2_max_instructions_hits_counter,l2_max_configurations_accesses_counter,l2_max_configurations_hits_counter);
    fprintf(results_cache,"%lu,%lu,%lu,%lu\n",l3_max_instructions_accesses_counter,l3_max_instructions_hits_counter,l3_max_configurations_accesses_counter,l3_max_configurations_hits_counter);

    printf("Sintaxe: INSTRUCTIONS ACCESSES,INSTRUCTIONS HITS,CONFIGURATIONS ACCESSES,CONFIGURATIONS HITS\n");
    printf("%lu,%lu,%lu,%lu\n",l1_max_instructions_accesses_counter,l1_max_instructions_hits_counter,l1_max_configurations_accesses_counter,l1_max_configurations_hits_counter);
    printf("%lu,%lu,%lu,%lu\n",l2_max_instructions_accesses_counter,l2_max_instructions_hits_counter,l2_max_configurations_accesses_counter,l2_max_configurations_hits_counter);
    printf("%lu,%lu,%lu,%lu\n",l3_max_instructions_accesses_counter,l3_max_instructions_hits_counter,l3_max_configurations_accesses_counter,l3_max_configurations_hits_counter);    
    
    fclose(results_cache);
}
