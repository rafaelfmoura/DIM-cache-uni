#include "DIM.h"

DIM::DIM() 
{
    this->memories = NULL;
    this->dim_results = NULL;
    this->dim_array = NULL;
    this->current_config = NULL;
}

DIM::DIM(memory_interface* memories_argv,array *array_argv,results *results_argv)
{
    //atualiza as referencias para as memorias cache
    this->memories = memories_argv;
    
    //atualiza a referencia para o array
    this->dim_array = array_argv;
    
    //atualiza a referencia para o results
    this->dim_results = results_argv;   
    
    this->state = SIMPLE_INSTRUCTION_STATE;
    this->spec_deepness = 0;    
    this->detecting = false;
    this->executing = false;    
    this->current_config = NULL;
}

void DIM::DIM_decode_instruction(INSTRUCTION_T instr,ADDRESS_T address,micro_instruction_t *uins)
{
    // Grupos:
    // 1- ALU 2- LOAD 3- STORE 4-MUL
    uint32_t rs1,rs2,rd,opcode=0,is_it_immed,format,format2,field_fp;	
    bool barrier;
    bool its_branch;	
    uint32_t field_trap;
    uint32_t op_w, op_r1, op_r2;
    uint32_t group;    
    barrier=false;
    format = ((instr & 0xFFFFFFFF) >> 30) & 0x3;
    uint32_t instr_nao_compativel = 0;
    
    op_w = op_r1 = op_r2 = 0;
    
    //set the defaults values for uins
    uins->supported = NOT_SUPPORTED;
    uins->instruction_type = NOT_INSTRUCTION;    
    	
    switch (format) 
    {
        /////////////////////////////////////////////////////////////
        // Formato: 01        
        case 1: //Call
        {            
            group = ALU_TYPE;
            its_branch= true;
            op_w =15;
            op_r1 = 0;
            op_r2 = 0;  
            
            uins->instr = instr;
            uins->address = address;
            uins->supported = SUPPORTED;
            uins->op_w = op_w;
            uins->op_r1 = op_r1;
            uins->op_r2 = op_r2;
            uins->group = group;
            uins->its_branch = its_branch;
            uins->instruction_type = NORMAL_INSTRUCTION;
            
            break;
        }			
        /////////////////////////////////////////////////////////////
        // Formato: 00
        case 0:
        {
            format2 = (instr >> 22) & 0x7;
            opcode = (instr >> 25)& 0xf;                     
            switch (format2) 
            {
                //////////////
                case 1:
                case 2:
                case 3:
                case 6:
                {
                    switch(opcode) 
                    {
                        case SPARC_BE:  
                        case SPARC_BLE: 
                        case SPARC_BL:  
                        case SPARC_BLEU:
                        case SPARC_BCS: 
                        case SPARC_BNEG:
                        case SPARC_BVS: 
                        case SPARC_BA:  
                        case SPARC_BNE: 
                        case SPARC_BG:  
                        case SPARC_BGE: 
                        case SPARC_BGU: 
                        case SPARC_BCC: 
                        case SPARC_BPOS:
                        case SPARC_BVC: 
                        {
                            op_r1 = op_w; // op_w da instrucao anterior
                            op_w  = 0;
                            op_r2 = 0;
                            group = ALU_TYPE;
                            its_branch= true;   
                            
                            uins->instr = instr;
                            uins->address = address;
                            uins->supported = SUPPORTED;
                            uins->op_w = op_w;
                            uins->op_r1 = op_r1;
                            uins->op_r2 = op_r2;
                            uins->group = group;
                            uins->its_branch = its_branch;
                            uins->instruction_type = NORMAL_INSTRUCTION;                                                                              
                            
                            break;
                        }
                        case SPARC_BN:
                        {
                            op_w  = 0;
                            op_r1 = 0;
                            op_r2 = 0;
                            group = ALU_TYPE;
                            its_branch= false; 
                            
                            uins->instr = instr;
                            uins->address = address;
                            uins->supported = SUPPORTED;
                            uins->op_w = op_w;
                            uins->op_r1 = op_r1;
                            uins->op_r2 = op_r2;
                            uins->group = group;
                            uins->its_branch = its_branch;
                            uins->instruction_type = NORMAL_INSTRUCTION;                                                                                                              
                            
                            break;
                        }
                        default:
                        {
                            instr_nao_compativel = 1;
                        }
                    }
                    break;
                }
                //////////////
                case 4:
                {
                    switch(instr)
                    {
                        case 0x01000000:
                        {  // NOP
                            group = ALU_TYPE;                            
                            op_w =0;
                            op_r1 =0;
                            op_r2 =0;
                            its_branch= false;  
                            
                            uins->instr = instr;
                            uins->address = address;
                            uins->supported = SUPPORTED;
                            uins->op_w = op_w;
                            uins->op_r1 = op_r1;
                            uins->op_r2 = op_r2;
                            uins->group = group;
                            uins->its_branch = its_branch;
                            uins->instruction_type = NORMAL_INSTRUCTION;                                                                                
                            
                            break;
                        /*BARREIRA DE SINCRONIZACAO TEMPORAL*/
                        }
                        case BARRIER_SIMPLE:                        
                        {                     
                            instr_nao_compativel = 1;
                            barrier=true;                            

                            uins->instruction_type = BARRIER_INSTRUCTION;                            
                            
                            break;                        
                        }
                        case BARRIER_CONTROL:
                        {
                            instr_nao_compativel = 1;
                            barrier=true;                            
                            break;                        
                        }
                        case B_ATOMIC_REGION:
                        case B_CRITICAL_REGION:
                        case E_CRITICAL_REGION:
                        case E_ATOMIC_REGION:
                        {
                            instr_nao_compativel = 1;                            
                            break;
                        }
                        default:
                        {  // SETHI //rd
                            op_w  = 0; 					
                            op_r1 = opcode;  
                            op_r2 = 0;
                            group = ALU_TYPE;
                            its_branch= false;  

                            uins->instr = instr;
                            uins->address = address;
                            uins->supported = SUPPORTED;
                            uins->op_w = op_w;
                            uins->op_r1 = op_r1;
                            uins->op_r2 = op_r2;
                            uins->group = group;
                            uins->its_branch = its_branch;
                            uins->instruction_type = NORMAL_INSTRUCTION;                                                                                  
                            
                            break;
                        }                        
                    }
                    break;
                }
                default:
                {
                    instr_nao_compativel = 1;
                }
            }
            break;
        }
        /////////////////////////////////////////////////////////////
        // Formato: 11
        case 3:
        {
            opcode = (instr >> 19)& 0x0000003f;
            rs1 = (instr >> 14) & 0x0000001f;
            rs2 = instr & 0x0000001f;
            rd = (instr >> 25) & 0x0000001f;
            is_it_immed = (instr >> 13) & 0x00001;                      
            switch(opcode)
            {
                case SPARC_LDX:
                case SPARC_LDXA:
                case SPARC_LDDA:
                case SPARC_LDF:
                case SPARC_LDDF:	
                case SPARC_LDFSR:
                {
                    op_w= rd;
                    if (!is_it_immed) 
                    {
                        op_r1= rs1;
                        op_r2= rs2;
                    }
                    else
                    {
                        op_r1 = rs1;
                        op_r2 = 0;
                    }
                    group = MEM_TYPE; 
                    its_branch= false;    
                    
                    uins->instr = instr;
                    uins->address = address;
                    uins->supported = SUPPORTED;
                    uins->op_w = op_w;
                    uins->op_r1 = op_r1;
                    uins->op_r2 = op_r2;
                    uins->group = group;
                    uins->its_branch = its_branch;
                    uins->instruction_type = NORMAL_INSTRUCTION;                                                                         
                    
                    break;
                }
                case SPARC_LD:  						
                case SPARC_LDA:
                case SPARC_LDSB:
                case SPARC_LDSBA:
                case SPARC_LDSH:
                case SPARC_LDSHA:
                case SPARC_LDSTUB:
                case SPARC_LDSTUBA:
                case SPARC_LDUB:      
                case SPARC_LDUBA:     
                case SPARC_LDUH:      
                case SPARC_LDUHA:
                case SPARC_LDD:
                {
                    op_w= rd;
                    if(!is_it_immed) 
                    {
                        op_r1= rs1;
                        op_r2= rs2;
                    }
                    else
                    {
                        op_r1 = rs1;
                        op_r2 = 0;
                    }
                    group = MEM_TYPE;
                    its_branch= false; 
                    
                    uins->instr = instr;
                    uins->address = address;
                    uins->supported = SUPPORTED;
                    uins->op_w = op_w;
                    uins->op_r1 = op_r1;
                    uins->op_r2 = op_r2;
                    uins->group = group;
                    uins->its_branch = its_branch;
                    uins->instruction_type = NORMAL_INSTRUCTION;                                                                             
                    
                    break;
                }
                case SPARC_STDA:
                case SPARC_STX:
                case SPARC_STF:
                case SPARC_STDF:
                case SPARC_STFSR:
                case SPARC_STFSRQ:
                {
                    // problema aqui, o op_w tambem eh de entrada
                    op_w= rd;
                    if(!is_it_immed)
                    {
                        op_r1= rs1;
                        op_r2= rs2;
                    }
                    else
                    {
                        op_r1 = rs1;
                        op_r2 = rd;
                    }
                    group = MEM_TYPE; 
                    its_branch= false; 
                    
                    uins->instr = instr;
                    uins->address = address;
                    uins->supported = SUPPORTED;
                    uins->op_w = op_w;
                    uins->op_r1 = op_r1;
                    uins->op_r2 = op_r2;
                    uins->group = group;
                    uins->its_branch = its_branch;
                    uins->instruction_type = NORMAL_INSTRUCTION;                                   
                    
                    break;	
                }
                case SPARC_ST:
                case SPARC_STA:
                case SPARC_STB:
                case SPARC_STBA:
                case SPARC_STH:
                case SPARC_STHA:
                case SPARC_SWAP:              
                case SPARC_SWAPA: 
                case SPARC_STD:
                {
                    // problema aqui, o op_w tambem eh de entrada
                    op_w= rd;
                    if(!is_it_immed)
                    {
                        op_r1= rs1;
                        op_r2= rs2;
                    }
                    else
                    {
                        op_r1 = rs1;
                        op_r2 = rd;
                    }
                    group = MEM_TYPE;
                    its_branch= false;                                       
                    
                    uins->instr = instr;
                    uins->address = address;
                    uins->supported = SUPPORTED;
                    uins->op_w = op_w;
                    uins->op_r1 = op_r1;
                    uins->op_r2 = op_r2;
                    uins->group = group;
                    uins->its_branch = its_branch;
                    uins->instruction_type = NORMAL_INSTRUCTION;                    
                    
                    break;
                }
                default:
                {
                    instr_nao_compativel = 1;
                    break;
                }
            }
            break;
        }
        /////////////////////////////////////////////////////////////
        // Formato: 10
        case 2:
        {
            opcode = (instr >> 19)& 0x003f;
            rs1 = (instr >> 14) & 0x0000001f;
            rs2 = instr & 0x0000001f;
            rd = (instr >> 25) & 0x0000001f;
            is_it_immed = (instr >> 13) & 0x0000001;                    
            switch(opcode)
            {
                case SPARC_ADD:  
                case SPARC_ADDCC:  
                case SPARC_ADDX:
                case SPARC_ADDXCC:
                case SPARC_AND:
                case SPARC_ANDCC:
                case SPARC_ANDN:
                case SPARC_ANDNCC:
                case SPARC_OR:
                case SPARC_ORCC:
                case SPARC_ORN:
                case SPARC_ORNCC:
                case SPARC_RESTORE:
                case SPARC_SAVE:
                case SPARC_SLL:
                case SPARC_SRA:
                case SPARC_SRL:
                case SPARC_SUB:
                case SPARC_SUBCC:
                case SPARC_SUBX:
                case SPARC_SUBXCC:
                case SPARC_TADDCC:
                case SPARC_TADDCCTV:
                case SPARC_TSUBCC:
                case SPARC_TSUBCCTV:
                case SPARC_XNOR:
                case SPARC_XNORCC:
                case SPARC_XOR:
                case SPARC_XORCC:
                case SPARC_IFLUSH:
                case SPARC_MOVE:
                {
                    // Nos shifts ha diferenca entre immed e shcnt, mas nao influencia em nada.
                    if (!is_it_immed) 
                    {
                        op_w  = rd;
                        op_r1= rs1;
                        op_r2= rs2;
                    }
                    else
                    {
                        op_w  = rd;
                        op_r1 = rs1;
                        op_r2 =-1;
                    }
                    if(opcode==SPARC_IFLUSH)
                    {
                        op_w =-1;
                        group = MEM_TYPE;
                    }
                    else
                    {
                        op_w= rd;
                        group = ALU_TYPE;
                    }
                    its_branch= false;                                      
                    
                    uins->instr = instr;
                    uins->address = address;
                    uins->supported = SUPPORTED;
                    uins->op_w = op_w;
                    uins->op_r1 = op_r1;
                    uins->op_r2 = op_r2;
                    uins->group = group;
                    uins->its_branch = its_branch;
                    uins->instruction_type = NORMAL_INSTRUCTION;                    
                    
                    break;
                }
                case SPARC_FPS:
                {
                    field_fp= 0x00001FF & (instr >> 5);
                    switch(field_fp)
                    {
                        case SPARC_FMOV: 
                        case SPARC_FNEG:    
                        case SPARC_ABS:	  
                        case SPARC_FADD:	  
                        case SPARC_FADDd:	  
                        case SPARC_FADDq:	  
                        case SPARC_FSUB:	  
                        case SPARC_FSUBd:	  
                        case SPARC_FSUBq:	  
                        {
                            op_w= rd;
                            op_r1= rs1;
                            op_r2= rs2;
                            group = FP_TYPE;
                            its_branch= false;                                                      
                            
                            uins->instr = instr;
                            uins->address = address;
                            uins->supported = SUPPORTED;
                            uins->op_w = op_w;
                            uins->op_r1 = op_r1;
                            uins->op_r2 = op_r2;
                            uins->group = group;
                            uins->its_branch = its_branch;
                            uins->instruction_type = NORMAL_INSTRUCTION;
                            
                            break;		
                        }
                        case SPARC_FMUL:
                        case SPARC_FMULd:
                        case SPARC_FMULq: 
                        {
                            op_w= rd;
                            op_r1= rs1;
                            op_r2= rs2;
                            group = FP_TYPE;
                            its_branch= false;                                                      
                            
                            uins->instr = instr;
                            uins->address = address;
                            uins->supported = SUPPORTED;
                            uins->op_w = op_w;
                            uins->op_r1 = op_r1;
                            uins->op_r2 = op_r2;
                            uins->group = group;
                            uins->its_branch = its_branch;
                            uins->instruction_type = NORMAL_INSTRUCTION;                            
                            
                            break;		
                        }
                    }
                    break;	
                }
                case SPARC_FPS2:
                {
                    field_fp= 0x00001FF & (instr >> 5);
                    switch(field_fp)
                    {
                        case SPARC_CMP:	  
                        case SPARC_CMPD:	  
                        case SPARC_CMPQ:	  
                        case SPARC_FPS:	    
                        {
                            op_w= 0;
                            op_r1= rs1;
                            op_r2= rs2;
                            group = FP_TYPE;
                            its_branch= false;                                                       
                            
                            uins->instr = instr;
                            uins->address = address;
                            uins->supported = SUPPORTED;
                            uins->op_w = op_w;
                            uins->op_r1 = op_r1;
                            uins->op_r2 = op_r2;
                            uins->group = group;
                            uins->its_branch = its_branch;
                            uins->instruction_type = NORMAL_INSTRUCTION;                            
                            
                            break;
                        }
                    }
                    break;	
                }
                case SPARC_RETURN:				
                case SPARC_JMPL:
                {
                    op_w= rd;
                    if (!is_it_immed) 
                    {
                        op_r1= rs1;
                        op_r2= rs2;
                    }
                    else
                    {
                        op_r1 = rs1;
                        op_r2 = 0;
                    }
                    group = ALU_TYPE;
                    its_branch= true;                                      
                    
                    uins->instr = instr;
                    uins->address = address;
                    uins->supported = SUPPORTED;
                    uins->op_w = op_w;
                    uins->op_r1 = op_r1;
                    uins->op_r2 = op_r2;
                    uins->group = group;
                    uins->its_branch = its_branch;
                    uins->instruction_type = NORMAL_INSTRUCTION;                    
                    
                    break;
                }
                case SPARC_MULSCC:  
                case SPARC_UMUL:
                case SPARC_SMUL:
                case SPARC_UMULCC:
                case SPARC_SMULCC:
                {
                    op_w= rd;
                    if(!is_it_immed)
                    {
                        op_r1= rs1;
                        op_r2= rs2;
                    }
                    else
                    {
                        op_r1 = rs1;
                        op_r2 = 0;
                    }
                    group = MUL_TYPE;
                    its_branch= false;                                     
                    
                    uins->instr = instr;
                    uins->address = address;
                    uins->supported = SUPPORTED;
                    uins->op_w = op_w;
                    uins->op_r1 = op_r1;
                    uins->op_r2 = op_r2;
                    uins->group = group;
                    uins->its_branch = its_branch;
                    uins->instruction_type = NORMAL_INSTRUCTION;                    
                    
                    break;
                }
                case SPARC_RDPSR:  
                case SPARC_RDTBR:
                case SPARC_RDWIM:
                case SPARC_RDY:
                {
                    op_w= rd;
                    op_r1 = 0;
                    op_r2 = 0;
                    group = ALU_TYPE;
                    its_branch= false;                                       
                    
                    uins->instr = instr;
                    uins->address = address;
                    uins->supported = SUPPORTED;
                    uins->op_w = op_w;
                    uins->op_r1 = op_r1;
                    uins->op_r2 = op_r2;
                    uins->group = group;
                    uins->its_branch = its_branch;
                    uins->instruction_type = NORMAL_INSTRUCTION;                    
                    
                    break;
                }
                case SPARC_TRAPS:
                {
                    field_trap = rd & 0x0F;
                    switch(field_trap)
                    {
                        case SPARC_TE:  
                        case SPARC_TLE: 
                        case SPARC_TL:  
                        case SPARC_TLEU:
                        case SPARC_TCS: 
                        case SPARC_TNEG:
                        case SPARC_TVS: 
                        case SPARC_TA:  
                        case SPARC_TNE: 
                        case SPARC_TG:  
                        case SPARC_TGE: 
                        case SPARC_TGU: 
                        case SPARC_TCC: 
                        case SPARC_TPOS:
                        case SPARC_TVC: 
                        case SPARC_TN:
                        {
                            op_w= 0;
                            if (!is_it_immed) 
                            {
                                op_r1= rs1;
                                op_r2= rs2;
                            }
                            else 
                            {
                                op_r1 = rs1;
                                op_r2 = 0;
                            }
                            group = ALU_TYPE;
                            its_branch= true;                                                       
                            
                            uins->instr = instr;
                            uins->address = address;
                            uins->supported = SUPPORTED;
                            uins->op_w = op_w;
                            uins->op_r1 = op_r1;
                            uins->op_r2 = op_r2;
                            uins->group = group;
                            uins->its_branch = its_branch;
                            uins->instruction_type = NORMAL_INSTRUCTION;                            
                            
                            break;
                        }
                        default:
                        {
                            instr_nao_compativel = 1;
                            break;
                        }
                    } 
                    break;
                }
                default:
                {
                    instr_nao_compativel = 1;
                    break;
                }
            }   
            break;  
        }
        /////////////////////////////////////////////////////////////
        default:
        {
            instr_nao_compativel = 1;                
            break;
        }
    }           
	            
                
    //Tratamento das instrucoes nao compativeis
    if (instr_nao_compativel) 
    {
        //AQUI PEGA AS INSTRUcoES NAO SUPORTADAS        
        
        op_w  = 0; 
        op_r1 = 0; 
        op_r2 = 0; 
        group =  ALU_TYPE; 
        its_branch= false;
        if(!barrier)
        {   
        
            uins->instr = instr;
            uins->address = address;
            uins->supported = NOT_SUPPORTED;
            uins->op_w = op_w;
            uins->op_r1 = op_r1;
            uins->op_r2 = op_r2;
            uins->group = group;
            uins->its_branch = its_branch;
            uins->instruction_type = NORMAL_INSTRUCTION;                 
        }     
    }  
}

void DIM::DIM_fsm_memory_access(micro_instruction_t *uins)
{
    this->dim_results->results_inc_total_inst(1);
    
    if(this->executing)
    {
        this->DIM_fsm_execution(uins);    
    }
    else if(this->detecting)
    {
        this->memories->memory_interface_unified_request(uins->address,CONFIGURATION_BLOCK,INTERFACE_READ_REQUEST,NULL);
        
        /*if(this->current_config)
        {            
            this->DIM_effectivate_configuration();
            this->spec_deepness = 0;
            this->state = SIMPLE_INSTRUCTION_STATE;
            this->detecting = false;
            this->executing = true;
            	                        
            this->dim_results->results_inc_total_cycles_on_array(this->current_config->configuration_get_cycles_by_spec_deepness(this->spec_deepness));  
            this->dim_results->results_inc_total_inst_on_array(this->current_config->configuration_get_instructions_by_spec_deepness(this->spec_deepness));                        
            this->dim_results->results_inc_total_power_on_array(this->current_config->configuration_get_power());                                     
            this->DIM_fsm_execution(uins);
        }
        else
        { */   
            this->detecting = true;
            this->executing = false;
            this->DIM_fsm_detection(uins);
        //}
    }
    else if(!this->executing && !this->detecting)
    {
        this->current_config = this->memories->memory_interface_unified_request(uins->address,CONFIGURATION_BLOCK,INTERFACE_READ_REQUEST,NULL);
        this->spec_deepness = 0;
        this->state = SIMPLE_INSTRUCTION_STATE;
        
        if(this->current_config)
        {            
            this->detecting = false;
            this->executing = true;
            /*contabiliza os ciclos,instrucoes e potencia gastos por este nivel de especulacao*/	                        
            this->dim_results->results_inc_total_cycles_on_array(this->current_config->configuration_get_cycles_by_spec_deepness(this->spec_deepness));  
            this->dim_results->results_inc_total_inst_on_array(this->current_config->configuration_get_instructions_by_spec_deepness(this->spec_deepness));                        
            this->dim_results->results_inc_total_power_on_array(this->current_config->configuration_get_power());                                     
            this->DIM_fsm_execution(uins);
        }
        else if(uins->supported && !uins->its_branch)
        {         
            this->dim_array->array_create_new_config(uins->address);           
            this->detecting = true;
            this->executing = false;
            this->DIM_fsm_detection(uins);
        }            
    }
    else
    {
        printf("Something bad happened\n");
    }       
}

void DIM::DIM_fsm_detection(micro_instruction_t *uins)
{
    switch(this->state)
    {
        case SIMPLE_INSTRUCTION_STATE:
        {          
            if(!uins->supported || !this->dim_array->array_add_inst_into_array(uins,this->spec_deepness))
            {
                this->dim_array->array_count_spec_deepness_time(this->spec_deepness);
                this->DIM_effectivate_configuration();
                this->detecting = false;
                this->executing = false;
            }
            else if(uins->its_branch)
            {
                this->state = PREVIOUS_WAS_BRANCH_INSTRUCTION_STATE;
            }
            
            break;
        }
        case PREVIOUS_WAS_BRANCH_INSTRUCTION_STATE:
        {           
            if(!uins->supported || uins->its_branch || (!this->dim_array->array_add_inst_into_array(uins,this->spec_deepness)) )
            {         
                this->dim_array->array_count_spec_deepness_time(this->spec_deepness);
                this->DIM_effectivate_configuration();
                this->detecting = false;
                this->executing = false;
            }
            else
            {                
                /*It counts the current speculation level*/
                this->dim_array->array_count_spec_deepness_time(this->spec_deepness);                    
                this->spec_deepness++;           
                
                if (this->spec_deepness == (this->dim_array->array_get_max_spec_depth()))
                {                                   
                    this->DIM_effectivate_configuration();
                    this->detecting = false;
                    this->executing = false;
                }
                else
                {                   
                    this->state = PREVIOUS_WAS_BRANCH_DELAY_SLOT_INSTRUCTION_STATE;
                }               
            }      
            
            break;         
        }
        case PREVIOUS_WAS_BRANCH_DELAY_SLOT_INSTRUCTION_STATE:
        {
            this->state = SIMPLE_INSTRUCTION_STATE;
        
            if(!uins->supported || !this->dim_array->array_add_inst_into_array(uins,this->spec_deepness))
            {
                this->dim_array->array_count_spec_deepness_time(this->spec_deepness);
                this->DIM_effectivate_configuration();
                this->detecting = false;
                this->executing = false;
            }
            else
            {
                this->dim_array->conf_detecting.configuration_update_speculative_pcs(uins->address,this->spec_deepness);            
            }
   
            break;         
        }  
        default:
        {
            printf("DIM_fsm_detection, it lost the states\n");
            break;
        }               
    }     
}

void DIM::DIM_fsm_execution(micro_instruction_t *uins)
{
    switch(this->state)
    {
        case SIMPLE_INSTRUCTION_STATE:
        {                     
            if(uins->address == this->current_config->last_pc)
            {                
                this->detecting = false;
                this->executing = false;
            }      
            else if(uins->its_branch)
            {
                this->state = PREVIOUS_WAS_BRANCH_INSTRUCTION_STATE;
            }
            
            break;
        }
        case PREVIOUS_WAS_BRANCH_INSTRUCTION_STATE:
        {
            if(uins->address != this->current_config->last_pc)
            {
                this->state = PREVIOUS_WAS_BRANCH_DELAY_SLOT_INSTRUCTION_STATE;              
                this->spec_deepness++;
            }   
            else
            { 
                this->detecting = false;
                this->executing = false;               
            }                           
            break;         
        }
        case PREVIOUS_WAS_BRANCH_DELAY_SLOT_INSTRUCTION_STATE:
        {

            this->state = SIMPLE_INSTRUCTION_STATE;
        
            if((uins->address == this->current_config->last_pc) || (!this->current_config->configuration_check_speculative_pc(uins->address,this->spec_deepness)))
            {
                this->detecting = false;
                this->executing = false;
            }   
            
            if(uins->address == this->current_config->last_pc || (this->current_config->configuration_check_speculative_pc(uins->address,this->spec_deepness)))
            {    
                this->dim_results->results_inc_total_cycles_on_array(this->current_config->configuration_get_cycles_by_spec_deepness(this->spec_deepness));  
                this->dim_results->results_inc_total_inst_on_array(this->current_config->configuration_get_instructions_by_spec_deepness(this->spec_deepness));                    
            }            
            
            break;         
        }  
        default:
        {
            printf("DIM_fsm_detection, it lost the states\n");
            break;
        }               

    }     
}

void DIM::DIM_effectivate_configuration(void)
{
    configuration *conf = NULL;
    
    /*If there are at least 'ARRAY_MIN_SIZE' instructions on this configuration,
    it'll store this one in the memory*/
    if(this->dim_array->conf_detecting.configuration_get_total_instructions() >  ARRAY_MIN_SIZE)
    {    
        this->dim_array->array_count_configuration_time();                   
        conf = this->dim_array->array_store_current_config();        
        this->memories->memory_interface_unified_request(conf->first_pc,CONFIGURATION_BLOCK,INTERFACE_WRITE_REQUEST,conf);                            
    }                     
    
    return;
}
