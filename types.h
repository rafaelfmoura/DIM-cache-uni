#ifndef TYPES_H
#define TYPES_H

#include <stdint.h>

/*NEW TYPES*/
typedef uint64_t ADDRESS_T;
typedef uint64_t CYCLE_COUNTER_T;
typedef uint64_t INSTRUCTION_COUNTER_T;
typedef uint32_t INSTRUCTION_T;
typedef long double POWER_T;
typedef long double ENERGY_T;

/*ARCHITECTURE*/
#if defined(__x86_64__) || defined(_M_X64)
/* x86 64-bit ----------------------------------------------- */
    #define x_64	
#elif defined(__i386) || defined(_M_IX86)
/* x86 32-bit ----------------------------------------------- */
    #define x_86
#endif      

#endif
