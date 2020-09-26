//---------------------------------------------------------------------------
// NEOPOP : Emulator as in Dreamland
//
// Copyright (c) 2001-2002 by neopop_uk
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version. See also the license.txt file for
//	additional informations.
//---------------------------------------------------------------------------

/*
//---------------------------------------------------------------------------
//=========================================================================

	TLCS900h.h

//=========================================================================
//---------------------------------------------------------------------------

  History of changes:
  ===================

26 SEP 2020 - hunterbridges
=======================================
- Refactored into C++ OOP interface

//---------------------------------------------------------------------------
*/

#ifndef __TLCS900H_STATE__
#define __TLCS900H_STATE__
//=============================================================================

#include <stdint.h>
#include "../../mednafen-types.h"

#ifndef __cplusplus
#if defined(_MSC_VER) && _MSC_VER < 1800 && !defined(SN_TARGET_PS3)
	/* Hack applied for MSVC when compiling in C89 mode
	 * as it isn't C99-compliant. */
#define bool unsigned char
#define true 1
#define false 0
#else
#include <stdbool.h>
#endif
#endif

#define FETCH8		loadB(pc++)
#define CALL_MEMBER_FN(object,ptrToMember)  ((object)->*(ptrToMember))

//The value read by bad rCodes, leave 0, improves "Gals Fighters"
#define RERR_VALUE		0

//GPR Access
#define regB(x)	(*(gprMapB[statusRFP][(x)]))
#define regW(x)	(*(gprMapW[statusRFP][(x)]))
#define regL(x)	(*(gprMapL[statusRFP][(x)]))

//Reg.Code Access
#define rCodeB(r)	(*(regCodeMapB[statusRFP][(r)]))
#define rCodeW(r)	(*(regCodeMapW[statusRFP][(r) >> 1]))
#define rCodeL(r)	(*(regCodeMapL[statusRFP][(r) >> 2]))

//Common Registers
#define REGA		(regB(1))
#define REGWA		(regW(0))
#define REGBC		(regW(1))
#define REGXSP		(gpr[3])

#define FLAG_S ((sr & 0x0080) >> 7)
#define FLAG_Z ((sr & 0x0040) >> 6)
#define FLAG_H ((sr & 0x0010) >> 4)
#define FLAG_V ((sr & 0x0004) >> 2)
#define FLAG_N ((sr & 0x0002) >> 1)
#define FLAG_C (sr & 1)

#define SETFLAG_S(s) { uint16 sr1 = sr & 0xFF7F; if (s) sr1 |= 0x0080; sr = sr1; }
#define SETFLAG_Z(z) { uint16 sr1 = sr & 0xFFBF; if (z) sr1 |= 0x0040; sr = sr1; }
#define SETFLAG_H(h) { uint16 sr1 = sr & 0xFFEF; if (h) sr1 |= 0x0010; sr = sr1; }
#define SETFLAG_V(v) { uint16 sr1 = sr & 0xFFFB; if (v) sr1 |= 0x0004; sr = sr1; }
#define SETFLAG_N(n) { uint16 sr1 = sr & 0xFFFD; if (n) sr1 |= 0x0002; sr = sr1; }
#define SETFLAG_C(c) { uint16 sr1 = sr & 0xFFFE; if (c) sr1 |= 0x0001; sr = sr1; }

#define SETFLAG_S0		{ sr &= 0xFF7F;	}
#define SETFLAG_Z0		{ sr &= 0xFFBF;	}
#define SETFLAG_H0		{ sr &= 0xFFEF;	}
#define SETFLAG_V0		{ sr &= 0xFFFB;	}
#define SETFLAG_N0		{ sr &= 0xFFFD;	}
#define SETFLAG_C0		{ sr &= 0xFFFE;	}

#define SETFLAG_S1		{ sr |= 0x0080; }
#define SETFLAG_Z1		{ sr |= 0x0040; }
#define SETFLAG_H1		{ sr |= 0x0010; }
#define SETFLAG_V1		{ sr |= 0x0004; }
#define SETFLAG_N1		{ sr |= 0x0002; }
#define SETFLAG_C1		{ sr |= 0x0001; }

struct TLCS900h
{
	typedef void (TLCS900h::*RegMemberFn)(void);

	uint32_t mem; //Result of addressing mode
	int size; //operand size, 0 = Byte, 1 = Word, 2 = Long
	uint8_t first; //The first byte
	uint8_t second; //The second opcode
	uint8_t R; //big R
	uint8_t rCode; //The code
	int32_t cycles; //How many state changes?
	int32_t cycles_extra; //How many extra state changes?
	bool brCode; //Register code used?

	uint32_t pc;
	uint16_t sr;
	uint8_t f_dash;

	uint32_t gprBank[4][4];
	uint32_t gpr[4];

	uint32_t rErr;

	uint8_t statusRFP;

	// GPR Access
	uint8_t gpr_init_flag;
	uint8_t* gprMapB[4][8];
	uint16_t* gprMapW[4][8];
	uint32_t* gprMapL[4][8];

	// Reg. Code Access
	uint8_t reg_init_flag;
	uint8_t* regCodeMapB[4][256];
	uint16_t* regCodeMapW[4][128];
	uint32_t* regCodeMapL[4][64];

	void (*instruction_error)(const char* vaMessage,...);

	// -------
	// biosHLE
	// -------

	void iBIOSHLE();

	// ------------------
	// TLCS900h_interpret
	// ------------------

	//Interprets a single instruction from 'pc', 
	//pc is incremented to the start of the next instruction.
	//Returns the number of cycles taken for this instruction
	int32 TLCS900h_interpret(void);

	//=============================================================================

	uint16 fetch16(void);
	uint32 fetch24(void);
	uint32 fetch32(void);

	//=============================================================================

	void parityB(uint8 value);
	void parityW(uint16 value);

	//=============================================================================

	void push8(uint8 data);
	void push16(uint16 data);
	void push32(uint32 data);

	uint8 pop8(void);
	uint16 pop16(void);
	uint32 pop32(void);

	//=============================================================================

	//DIV ===============
	uint16 generic_DIV_B(uint16 val, uint8 div);
	uint32 generic_DIV_W(uint32 val, uint16 div);

	//DIVS ===============
	uint16 generic_DIVS_B(int16 val, int8 div);
	uint32 generic_DIVS_W(int32 val, int16 div);

	//ADD ===============
	uint8	generic_ADD_B(uint8 dst, uint8 src);
	uint16 generic_ADD_W(uint16 dst, uint16 src);
	uint32 generic_ADD_L(uint32 dst, uint32 src);

	//ADC ===============
	uint8	generic_ADC_B(uint8 dst, uint8 src);
	uint16 generic_ADC_W(uint16 dst, uint16 src);
	uint32 generic_ADC_L(uint32 dst, uint32 src);

	//SUB ===============
	uint8	generic_SUB_B(uint8 dst, uint8 src);
	uint16 generic_SUB_W(uint16 dst, uint16 src);
	uint32 generic_SUB_L(uint32 dst, uint32 src);

	//SBC ===============
	uint8	generic_SBC_B(uint8 dst, uint8 src);
	uint16 generic_SBC_W(uint16 dst, uint16 src);
	uint32 generic_SBC_L(uint32 dst, uint32 src);

	//=============================================================================

	//Confirms a condition code check
	bool conditionCode(int cc);

	//=============================================================================

	//Translate an rr or RR value for MUL/MULS/DIV/DIVS
	uint8 get_rr_Target(void);
	uint8 get_RR_Target(void);

private:
	static TLCS900h::RegMemberFn decodeExtra[256];
	static TLCS900h::RegMemberFn srcDecode[256];
	static TLCS900h::RegMemberFn dstDecode[256];
	static TLCS900h::RegMemberFn regDecode[256];
	static TLCS900h::RegMemberFn decode[256];

	void ExXWA();
	void ExXBC();
	void ExXDE();
	void ExXHL();
	void ExXIX();
	void ExXIY();
	void ExXIZ();
	void ExXSP();

	void ExXWAd();
	void ExXBCd();
	void ExXDEd();
	void ExXHLd();
	void ExXIXd();
	void ExXIYd();
	void ExXIZd();
	void ExXSPd();

	void Ex8(void);
	void Ex16(void);
	void Ex24(void);
	void ExR32(void);
	void ExDec();
	void ExInc();
	void ExRC();

	void e(void);
	void es(void);
	void ed(void);
	void er(void);

	void src_B();
	void src_W();
	void src_L();
	void dst();

	void reg_B(void);
	void reg_W(void);
	void reg_L(void);

public:

	// ----------------------
	// TLCS900h_interpret_dst
	// ----------------------

	//===== LD (mem),#
	void DST_dstLDBi(void);

	//===== LD (mem),#
	void DST_dstLDWi(void);

	//===== POP (mem)
	void DST_dstPOPB(void);

	//===== POP (mem)
	void DST_dstPOPW(void);

	//===== LD (mem),(nn)
	void DST_dstLDBm16(void);

	//===== LD (mem),(nn)
	void DST_dstLDWm16(void);

	//===== LDA R,mem
	void DST_dstLDAW(void);

	//===== LDA R,mem
	void DST_dstLDAL(void);

	//===== ANDCF A,(mem)
	void DST_dstANDCFA(void);

	//===== ORCF A,(mem)
	void DST_dstORCFA(void);

	//===== XORCF A,(mem)
	void DST_dstXORCFA(void);

	//===== LDCF A,(mem)
	void DST_dstLDCFA(void);

	//===== STCF A,(mem)
	void DST_dstSTCFA(void);

	//===== LD (mem),R
	void DST_dstLDBR(void);

	//===== LD (mem),R
	void DST_dstLDWR(void);

	//===== LD (mem),R
	void DST_dstLDLR(void);

	//===== ANDCF #3,(mem)
	void DST_dstANDCF(void);

	//===== ORCF #3,(mem)
	void DST_dstORCF(void);

	//===== XORCF #3,(mem)
	void DST_dstXORCF(void);

	//===== LDCF #3,(mem)
	void DST_dstLDCF(void);

	//===== STCF #3,(mem)
	void DST_dstSTCF(void);

	//===== TSET #3,(mem)
	void DST_dstTSET(void);

	//===== RES #3,(mem)
	void DST_dstRES(void);

	//===== SET #3,(mem)
	void DST_dstSET(void);

	//===== CHG #3,(mem)
	void DST_dstCHG(void);

	//===== BIT #3,(mem)
	void DST_dstBIT(void);

	//===== JP cc,mem
	void DST_dstJP(void);

	//===== CALL cc,mem
	void DST_dstCALL(void);

	//===== RET cc
	void DST_dstRET(void);

	//=========================================================================

	// ----------------------
	// TLCS900h_interpret_reg
	// ----------------------

	//=========================================================================

	//===== LD r,#
	void regLDi(void);

	//===== PUSH r
	void regPUSH(void);

	//===== POP r
	void regPOP(void);

	//===== CPL r
	void regCPL(void);

	//===== NEG r
	void regNEG(void);

	//===== MUL rr,#
	void regMULi(void);

	//===== MULS rr,#
	void regMULSi(void);

	//===== DIV rr,#
	void regDIVi(void);

	//===== DIVS rr,#
	void regDIVSi(void);

	//===== LINK r,dd
	void regLINK(void);

	//===== UNLK r
	void regUNLK(void);

	//===== BS1F A,r
	void regBS1F(void);

	//===== BS1B A,r
	void regBS1B(void);

	//===== DAA r
	void regDAA(void);

	//===== EXTZ r
	void regEXTZ(void);

	//===== EXTS r
	void regEXTS(void);

	//===== PAA r
	void regPAA(void);

	//===== MIRR r
	void regMIRR(void);

	//===== MULA r
	void regMULA(void);

	//===== DJNZ r,d
	void regDJNZ(void);

	//===== ANDCF #,r
	void regANDCFi(void);

	//===== ORCF #,r
	void regORCFi(void);

	//===== XORCF #,r
	void regXORCFi(void);

	//===== LDCF #,r
	void regLDCFi(void);

	//===== STCF #,r
	void regSTCFi(void);

	//===== ANDCF A,r
	void regANDCFA(void);

	//===== ORCF A,r
	void regORCFA(void);

	//===== XORCF A,r
	void regXORCFA(void);

	//===== LDCF A,r
	void regLDCFA(void);

	//===== STCF A,r
	void regSTCFA(void);

	//===== LDC cr,r
	void regLDCcrr(void);

	//===== LDC r,cr
	void regLDCrcr(void);

	//===== RES #,r
	void regRES(void);

	//===== SET #,r
	void regSET(void);

	//===== CHG #,r
	void regCHG(void);

	//===== BIT #,r
	void regBIT(void);

	//===== TSET #,r
	void regTSET(void);

	//===== MINC1 #,r
	void regMINC1(void);

	//===== MINC2 #,r
	void regMINC2(void);

	//===== MINC4 #,r
	void regMINC4(void);

	//===== MDEC1 #,r
	void regMDEC1(void);

	//===== MDEC2 #,r
	void regMDEC2(void);

	//===== MDEC4 #,r
	void regMDEC4(void);

	//===== MUL RR,r
	void regMUL(void);

	//===== MULS RR,r
	void regMULS(void);

	//===== DIV RR,r
	void regDIV(void);

	//===== DIVS RR,r
	void regDIVS(void);

	//===== INC #3,r
	void regINC(void);

	//===== DEC #3,r
	void regDEC(void);

	//===== SCC cc,r
	void regSCC(void);

	//===== LD R,r
	void regLDRr(void);

	//===== LD r,R
	void regLDrR(void);

	//===== ADD R,r
	void regADD(void);

	//===== ADC R,r
	void regADC(void);

	//===== SUB R,r
	void regSUB(void);

	//===== SBC R,r
	void regSBC(void);

	//===== LD r,#3
	void regLDr3(void);

	//===== EX R,r
	void regEX(void);

	//===== ADD r,#
	void regADDi(void);

	//===== ADC r,#
	void regADCi(void);

	//===== SUB r,#
	void regSUBi(void);

	//===== SBC r,#
	void regSBCi(void);

	//===== CP r,#
	void regCPi(void);

	//===== AND r,#
	void regANDi(void);

	//===== OR r,#
	void regORi(void);

	//===== XOR r,#
	void regXORi(void);

	//===== AND R,r
	void regAND(void);

	//===== OR R,r
	void regOR(void);

	//===== XOR R,r
	void regXOR(void);

	//===== CP r,#3
	void regCPr3(void);

	//===== CP R,r
	void regCP(void);

	//===== RLC #,r
	void regRLCi(void);

	//===== RRC #,r
	void regRRCi(void);

	//===== RL #,r
	void regRLi(void);

	//===== RR #,r
	void regRRi(void);

	//===== SLA #,r
	void regSLAi(void);

	//===== SRA #,r
	void regSRAi(void);

	//===== SLL #,r
	void regSLLi(void);

	//===== SRL #,r
	void regSRLi(void);

	//===== RLC A,r
	void regRLCA(void);

	//===== RRC A,r
	void regRRCA(void);

	//===== RL A,r
	void regRLA(void);

	//===== RR A,r
	void regRRA(void);

	//===== SLA A,r
	void regSLAA(void);

	//===== SRA A,r
	void regSRAA(void);

	//===== SLL A,r
	void regSLLA(void);

	//===== SRL A,r
	void regSRLA(void);

	//=========================================================================

	// -------------------------
	// TLCS900h_interpret_single
	// -------------------------

	//=========================================================================

	//===== NOP
	void sngNOP(void);

	//===== NORMAL
	void sngNORMAL(void);

	//===== PUSH SR
	void sngPUSHSR(void);

	//===== POP SR
	void sngPOPSR(void);

	//===== MAX
	void sngMAX(void);

	//===== HALT
	void sngHALT(void);

	//===== EI #3
	void sngEI(void);

	//===== RETI
	void sngRETI(void);

	//===== LD (n), n
	void sngLD8_8(void);

	//===== PUSH n
	void sngPUSH8(void);

	//===== LD (n), nn
	void sngLD8_16(void);

	//===== PUSH nn
	void sngPUSH16(void);

	//===== INCF
	void sngINCF(void);

	//===== DECF
	void sngDECF(void);

	//===== RET condition
	void sngRET(void);

	//===== RETD dd
	void sngRETD(void);

	//===== RCF
	void sngRCF(void);

	//===== SCF
	void sngSCF(void);

	//===== CCF
	void sngCCF(void);

	//===== ZCF
	void sngZCF(void);

	//===== PUSH A
	void sngPUSHA(void);

	//===== POP A
	void sngPOPA(void);

	//===== EX F,F'
	void sngEX(void);

	//===== LDF #3
	void sngLDF(void);

	//===== PUSH F
	void sngPUSHF(void);

	//===== POP F
	void sngPOPF(void);

	//===== JP nn
	void sngJP16(void);

	//===== JP nnn
	void sngJP24(void);

	//===== CALL #16
	void sngCALL16(void);

	//===== CALL #24
	void sngCALL24(void);

	//===== CALR $+3+d16
	void sngCALR(void);

	//===== LD R, n
	void sngLDB(void);

	//===== PUSH RR
	void sngPUSHW(void);

	//===== LD RR, nn
	void sngLDW(void);

	//===== PUSH XRR
	void sngPUSHL(void);

	//===== LD XRR, nnnn
	void sngLDL(void);

	//===== POP RR
	void sngPOPW(void);

	//===== POP XRR
	void sngPOPL(void);

	//===== JR cc,PC + d
	void sngJR(void);

	//===== JR cc,PC + dd
	void sngJRL(void);

	//===== LDX dst,src
	void sngLDX(void);

	//===== SWI num
	void sngSWI(void);

	//=============================================================================

	// ----------------------
	// TLCS900h_interpret_src
	// ----------------------

	//=============================================================================

	//===== PUSH (mem)
	void srcPUSH(void);

	//===== RLD A,(mem)
	void srcRLD(void);

	//===== RRD A,(mem)
	void srcRRD(void);

	//===== LDI
	void srcLDI(void);

	//===== LDIR
	void srcLDIR(void);

	//===== LDD
	void srcLDD(void);

	//===== LDDR
	void srcLDDR(void);

	//===== CPI
	void srcCPI(void);

	//===== CPIR
	void srcCPIR(void);

	//===== CPD
	void srcCPD(void);

	//===== CPDR
	void srcCPDR(void);

	//===== LD (nn),(mem)
	void srcLD16m(void);

	//===== LD R,(mem)
	void srcLD(void);

	//===== EX (mem),R
	void srcEX(void);

	//===== ADD (mem),#
	void srcADDi(void);

	//===== ADC (mem),#
	void srcADCi(void);

	//===== SUB (mem),#
	void srcSUBi(void);

	//===== SBC (mem),#
	void srcSBCi(void);

	//===== AND (mem),#
	void srcANDi(void);

	//===== OR (mem),#
	void srcORi(void);

	//===== XOR (mem),#
	void srcXORi(void);

	//===== CP (mem),#
	void srcCPi(void);

	//===== MUL RR,(mem)
	void srcMUL(void);

	//===== MULS RR,(mem)
	void srcMULS(void);

	//===== DIV RR,(mem)
	void srcDIV(void);

	//===== DIVS RR,(mem)
	void srcDIVS(void);

	//===== INC #3,(mem)
	void srcINC(void);

	//===== DEC #3,(mem)
	void srcDEC(void);

	//===== RLC (mem)
	void srcRLC(void);

	//===== RRC (mem)
	void srcRRC(void);

	//===== RL (mem)
	void srcRL(void);

	//===== RR (mem)
	void srcRR(void);

	//===== SLA (mem)
	void srcSLA(void);

	//===== SRA (mem)
	void srcSRA(void);

	//===== SLL (mem)
	void srcSLL(void);

	//===== SRL (mem)
	void srcSRL(void);

	//===== ADD R,(mem)
	void srcADDRm(void);

	//===== ADD (mem),R
	void srcADDmR(void);

	//===== ADC R,(mem)
	void srcADCRm(void);

	//===== ADC (mem),R
	void srcADCmR(void);

	//===== SUB R,(mem)
	void srcSUBRm(void);

	//===== SUB (mem),R
	void srcSUBmR(void);

	//===== SBC R,(mem)
	void srcSBCRm(void);

	//===== SBC (mem),R
	void srcSBCmR(void);

	//===== AND R,(mem)
	void srcANDRm(void);

	//===== AND (mem),R
	void srcANDmR(void);

	//===== XOR R,(mem)
	void srcXORRm(void);

	//===== XOR (mem),R
	void srcXORmR(void);

	//===== OR R,(mem)
	void srcORRm(void);

	//===== OR (mem),R
	void srcORmR(void);

	//===== CP R,(mem)
	void srcCPRm(void);

	//===== CP (mem),R
	void srcCPmR(void);

	//=============================================================================

	// ------------------
	// tlcs900h_registers
	// ------------------

	void initGPRTables();
	void initRegCodeTables();

	//=============================================================================

	void reset_registers(void);
	void dump_registers_TLCS900h(void);

	//=============================================================================

	uint8 statusIFF(void);
	void setStatusIFF(uint8 iff);

	void setStatusRFP(uint8 rfp);
	void changedSP(void);

	//=============================================================================
};

extern struct TLCS900h *cur_tlcs900h;

#endif
