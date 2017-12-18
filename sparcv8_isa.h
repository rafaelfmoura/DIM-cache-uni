#ifndef SPARCV8_ISA_H
#define SPARCV8_ISA_H

#define BARRIER_SIMPLE    0x01000004
#define BARRIER_CONTROL   0x0100000A
#define B_CRITICAL_REGION 0x0100003D
#define E_CRITICAL_REGION 0x01000047
#define B_ATOMIC_REGION   0x01000051
#define E_ATOMIC_REGION   0x0100005B

// decodificando as instrucoes
/////////////////////////////////////////////////////////////
// Formato: 11

#define SPARC_LD        0x00 //ok
#define SPARC_LDA       0x10 //ok
#define SPARC_LDSB      0x09 //ok
#define SPARC_LDSBA     0x19 //ok
#define SPARC_LDSH      0x0a //ok
#define SPARC_LDSHA     0x1a //ok
#define SPARC_LDSTUB    0x0d //ok
#define SPARC_LDSTUBA   0x1d //ok
#define SPARC_LDUB      0x01 //ok
#define SPARC_LDUBA     0x11 //ok
#define SPARC_LDUH      0x02 //ok
#define SPARC_LDUHA     0x12 //ok

#define SPARC_ST        0x04 //ok
#define SPARC_STA       0x14 //ok
#define SPARC_STB       0x05 //ok
#define SPARC_STBA      0x15 //ok
#define SPARC_STH       0x06 //ok
#define SPARC_STHA      0x16 //ok        

#define SPARC_SWAP      0x0f //ok
#define SPARC_SWAPA     0x1f //ok



/////////////////////////////////////////////////////////////
// Formato: 10
// Logicas e Aritmeticas

#define SPARC_ADD       0x00 //ok
#define SPARC_ADDCC     0x10 //ok
#define SPARC_ADDX      0x08 //ok
#define SPARC_ADDXCC    0x18 //ok
#define SPARC_AND       0x01 //ok
#define SPARC_ANDCC     0x11 //ok
#define SPARC_ANDN      0x05 //ok
#define SPARC_ANDNCC    0x15 //ok
#define SPARC_OR        0x02 //ok
#define SPARC_ORCC      0x12 //ok
#define SPARC_ORN       0x06 //ok
#define SPARC_ORNCC     0x16 //ok
//mudança

#define SPARC_MOVE      0x2C //ok
#define SPARC_RETURN    0x39 //ok

#define SPARC_SUB       0x04 //ok
#define SPARC_SUBCC     0x14 //ok
#define SPARC_SUBX      0x0C //ok
#define SPARC_SUBXCC    0x1C //ok

#define SPARC_TADDCC    0x20 //ok
#define SPARC_TADDCCTV  0x22 //ok

#define SPARC_SLL       0x25 //ok
#define SPARC_SRA       0x27 //ok
#define SPARC_SRL       0X26 //ok

#define SPARC_RESTORE   0x3d //ok
#define SPARC_SAVE      0x3c //ok

#define SPARC_RDPSR     0x29 //ok
#define SPARC_RDTBR     0x2b //ok
#define SPARC_RDWIM     0x2a //ok
#define SPARC_RDY       0x28 //ok



#define SPARC_JMPL      0x38 //ok

#define SPARC_MULSCC    0x24 //ok

#define SPARC_UMUL      0x0A //ok 
#define SPARC_UMUL      0x0A //ok 
#define SPARC_SMUL      0x0B //ok
#define SPARC_UMULCC    0x1A //ok
#define SPARC_SMULCC    0x1B //ok

#define SPARC_TRAPS     0x3A //ok

#define SPARC_TN        0x00 //ok
#define SPARC_TE        0x01 //ok
#define SPARC_TLE       0x02 //ok
#define SPARC_TL        0x03 //ok
#define SPARC_TLEU      0x04 //ok
#define SPARC_TCS       0x05 //ok
#define SPARC_TNEG      0x06 //ok
#define SPARC_TVS       0x07 //ok

#define SPARC_TA        0x08 //ok
#define SPARC_TNE       0x09 //ok
#define SPARC_TG        0x0a //ok
#define SPARC_TGE       0x0b //ok
#define SPARC_TGU       0x0c //ok
#define SPARC_TCC       0x0d //ok
#define SPARC_TPOS      0x0e //ok
#define SPARC_TVC       0x0f //ok

#define SPARC_TSUBCC    0x21 //ok
#define SPARC_TSUBCCTV  0x23 //ok

#define SPARC_XNOR      0x07 //ok
#define SPARC_XNORCC    0x17 //ok
#define SPARC_XOR       0x03 //ok
#define SPARC_XORCC     0x13 //ok
#define SPARC_IFLUSH    0x3B //ok

/////////////////////////////////////////////////////////////
// Formato: 00
// Branchs

// Formato "secundario"
// 001 - Está errado no manual do SPARC. Diz que é 010
// 

#define SPARC_BN        0x0 //ok
#define SPARC_BE        0x1 //ok
#define SPARC_BLE       0x2 //ok
#define SPARC_BL        0x3 //ok
#define SPARC_BLEU      0x4 //ok
#define SPARC_BCS       0x5 //ok
#define SPARC_BNEG      0x6 //ok
#define SPARC_BVS       0x7 //ok

#define SPARC_BA        0x8 //ok
#define SPARC_BNE       0x9 //ok
#define SPARC_BG        0xa //ok
#define SPARC_BGE       0xb //ok
#define SPARC_BGU       0xc //ok
#define SPARC_BCC       0xd //ok
#define SPARC_BPOS      0xe //ok
#define SPARC_BVC       0xf //ok

// 110

#define SPARC_SETHI     0xF1 //ok 
#define SPARC_NOP       0xF2 //ok



//FLOATING POINT INSTRUCTIONS 
// LDC   - Load Coprocessor register
// LDCSR - Load Coprocessor State Register
// LDD   - Load Doubleword
// LDDA  - Load Doubleword from Alternate space
// LDDC  - Load Doubleword Coprocessor
// LDDF  - Load Doubleword Floating-Point
// LDF   - Load Floating-Point register
// LDFSR - Load Floating-Point State Register

#define SPARC_STX	  0x0E
#define SPARC_STDA	  0x37

#define SPARC_STD     0x07 
#define SPARC_LDD     0x03 
 
#define SPARC_LDDA    0x33
#define SPARC_LDX     0x1B
#define SPARC_LDXA    0x0B
#define SPARC_LDF     0x20
#define SPARC_LDDF    0x23
#define SPARC_LDFSR   0x21

#define SPARC_STF     0x24
#define SPARC_STDF    0x27
#define SPARC_STFSR   0x25
#define SPARC_STFSRQ  0x26
#define SPARC_FPS	  0x34
#define SPARC_FPS2	  0x35

#define SPARC_FMOV	  0x01
#define SPARC_FNEG    0x05
#define SPARC_ABS	  0x09
#define SPARC_FADD	  0x41
#define SPARC_FADDd	  0x42
#define SPARC_FADDq	  0x43
#define SPARC_FSUB	  0x45
#define SPARC_FSUBd	  0x46
#define SPARC_FSUBq	  0x47


#define SPARC_FMUL	  0x49
#define SPARC_FMULd	  0x4a
#define SPARC_FMULq	  0x4b

#define SPARC_CMP	  0x51
#define SPARC_CMPD	  0x52
#define SPARC_CMPQ	  0x53
#define SPARC_FPS	  0x34

#endif
