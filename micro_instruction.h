#ifndef MICRO_INSTRUCTION_H
#define MICRO_INSTRUCTION_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "types.h"

//microinstruction struct
typedef struct micro_instruction_t micro_instruction_t;
struct micro_instruction_t
{
    INSTRUCTION_T instr;
    ADDRESS_T address;
    bool supported;
    uint32_t op_w;
    uint32_t op_r1;
    uint32_t op_r2;
    uint32_t group;
    bool its_branch;
    uint32_t instruction_type;
};

#endif
