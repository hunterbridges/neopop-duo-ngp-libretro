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

	TLCS900h_interpret_single.c

//=========================================================================
//---------------------------------------------------------------------------

  History of changes:
  ===================

20 JUL 2002 - neopop_uk
=======================================
- Cleaned and tidied up for the source release

22 JUL 2002 - neopop_uk
=======================================
- Removed setting the register file pointer to 3 in the SWI instruction
	This has fixed "Metal Slug 2" and flash saving in many games.

26 JUL 2002 - neopop_uk
=======================================
- Prefixed all instruction functions with "sng" to avoid a repeat of the
	the 'EX' fiasco.

30 JUL 2002 - neopop_uk
=======================================
- Um... SWI doesn't cause a problem if IFF is set to 3... why did 
"Metal Slug 2" start working???

//---------------------------------------------------------------------------
*/

#include <stdio.h>
#include "TLCS900h_interpret.h"
#include "TLCS900h_registers.h"
#include "../mem.h"
#include "../interrupt.h"

//=========================================================================

//===== NOP
void sngNOP()
{
	cur_tlcs900h->cycles = 2;
}

//===== NORMAL
void sngNORMAL()
{
	//Not supported
	cur_tlcs900h->cycles = 4;
}

//===== PUSH SR
void sngPUSHSR()
{
	push16(cur_tlcs900h->sr);
	cur_tlcs900h->cycles = 4;
}

//===== POP SR
void sngPOPSR()
{
	cur_tlcs900h->sr = pop16();	changedSP();
	cur_tlcs900h->cycles = 6;
}

//===== MAX
void sngMAX()
{
	//Not supported
	cur_tlcs900h->cycles = 4;
}

//===== HALT
void sngHALT()
{
	//MDFN_printf("CPU halt requested and ignored.\nPlease send me a saved state.");
	cur_tlcs900h->cycles = 8;
}

//===== EI #3
void sngEI()
{
	setStatusIFF(FETCH8);
	int_check_pending();
	cur_tlcs900h->cycles = 5;
}

//===== RETI
void sngRETI()
{
	uint16 temp = pop16();
	cur_tlcs900h->pc = pop32();
	cur_tlcs900h->sr = temp; changedSP();
	cur_tlcs900h->cycles = 12;
}

//===== LD (n), n
void sngLD8_8()
{
	uint8 dst = FETCH8;
	uint8 src = FETCH8;
	storeB(dst, src);
	cur_tlcs900h->cycles = 5;
}

//===== PUSH n
void sngPUSH8()
{
	uint8 data = FETCH8;
	push8(data);
	cur_tlcs900h->cycles = 4;
}

//===== LD (n), nn
void sngLD8_16()
{
	uint8 dst = FETCH8;
	uint16 src = fetch16();
	storeW(dst, src);
	cur_tlcs900h->cycles = 6;
}

//===== PUSH nn
void sngPUSH16()
{
	push16(fetch16());
	cur_tlcs900h->cycles = 5;
}

//===== INCF
void sngINCF()
{
	setStatusRFP(((cur_tlcs900h->sr & 0x300) >> 8) + 1);
	cur_tlcs900h->cycles = 2;
}

//===== DECF
void sngDECF()
{
	setStatusRFP(((cur_tlcs900h->sr & 0x300) >> 8) - 1);
	cur_tlcs900h->cycles = 2;
}

//===== RET condition
void sngRET()
{
	cur_tlcs900h->pc = pop32();
	cur_tlcs900h->cycles = 9;
}

//===== RETD dd
void sngRETD()
{
	int16 d = (int16)fetch16();
	cur_tlcs900h->pc = pop32();
	REGXSP += d;
	cur_tlcs900h->cycles = 9;
}

//===== RCF
void sngRCF()
{
	SETFLAG_N0;
	SETFLAG_V0;
	SETFLAG_C0;
	cur_tlcs900h->cycles = 2;
}

//===== SCF
void sngSCF()
{
	SETFLAG_H0;
	SETFLAG_N0;
	SETFLAG_C1;
	cur_tlcs900h->cycles = 2;
}

//===== CCF
void sngCCF()
{
	SETFLAG_N0;
	SETFLAG_C(!FLAG_C);
	cur_tlcs900h->cycles = 2;
}

//===== ZCF
void sngZCF()
{
	SETFLAG_N0;
	SETFLAG_C(!FLAG_Z);
	cur_tlcs900h->cycles = 2;
}

//===== PUSH A
void sngPUSHA()
{
	push8(REGA);
	cur_tlcs900h->cycles = 3;
}

//===== POP A
void sngPOPA()
{
	REGA = pop8();
	cur_tlcs900h->cycles = 4;
}

//===== EX F,F'
void sngEX()
{
	uint8 f = cur_tlcs900h->sr & 0xFF;
	cur_tlcs900h->sr = (cur_tlcs900h->sr & 0xFF00) | cur_tlcs900h->f_dash;
	cur_tlcs900h->f_dash = f;
	cur_tlcs900h->cycles = 2;
}

//===== LDF #3
void sngLDF()
{
	setStatusRFP(FETCH8);
	cur_tlcs900h->cycles = 2;
}

//===== PUSH F
void sngPUSHF()
{
	push8(cur_tlcs900h->sr & 0xFF);
	cur_tlcs900h->cycles = 3;
}

//===== POP F
void sngPOPF()
{
	cur_tlcs900h->sr = (cur_tlcs900h->sr & 0xFF00) | pop8();
	cur_tlcs900h->cycles = 4;
}

//===== JP nn
void sngJP16()
{
	cur_tlcs900h->pc = fetch16();
	cur_tlcs900h->cycles = 7;
}

//===== JP nnn
void sngJP24()
{
	cur_tlcs900h->pc = fetch24();
	cur_tlcs900h->cycles = 7;
}

//===== CALL #16
void sngCALL16()
{
	uint32 target = fetch16();
	push32(cur_tlcs900h->pc);
	cur_tlcs900h->pc = target;
	cur_tlcs900h->cycles = 12;
}

//===== CALL #24
void sngCALL24()
{
	uint32 target = fetch24();
	push32(cur_tlcs900h->pc);
	cur_tlcs900h->pc = target;
	cur_tlcs900h->cycles = 12;
}

//===== CALR $+3+d16
void sngCALR()
{
	int16 displacement = (int16)fetch16();
	uint32 target = cur_tlcs900h->pc + displacement;
	push32(cur_tlcs900h->pc);
	cur_tlcs900h->pc = target;
	cur_tlcs900h->cycles = 12;
}

//===== LD R, n
void sngLDB()
{
	regB(cur_tlcs900h->first & 7) = FETCH8;
	cur_tlcs900h->cycles = 2;
}

//===== PUSH RR
void sngPUSHW()
{
	push16(regW(cur_tlcs900h->first & 7));
	cur_tlcs900h->cycles = 3;
}

//===== LD RR, nn
void sngLDW()
{
	regW(cur_tlcs900h->first & 7) = fetch16();
	cur_tlcs900h->cycles = 3;
}

//===== PUSH XRR
void sngPUSHL()
{
	push32(regL(cur_tlcs900h->first & 7));
	cur_tlcs900h->cycles = 5;
}

//===== LD XRR, nnnn
void sngLDL()
{
	regL(cur_tlcs900h->first & 7) = fetch32();
	cur_tlcs900h->cycles = 5;
}

//===== POP RR
void sngPOPW()
{
	regW(cur_tlcs900h->first & 7) = pop16();
	cur_tlcs900h->cycles = 4;
}

//===== POP XRR
void sngPOPL()
{
	regL(cur_tlcs900h->first & 7) = pop32();
	cur_tlcs900h->cycles = 6;
}

//===== JR cc,PC + d
void sngJR()
{
	if (conditionCode(cur_tlcs900h->first & 0xF))
	{
		int8 displacement = (int8)FETCH8;

		cur_tlcs900h->cycles = 8;
		cur_tlcs900h->pc += displacement;
	}
	else
	{
		cur_tlcs900h->cycles = 4;
		FETCH8;
	}
}

//===== JR cc,PC + dd
void sngJRL()
{
	if (conditionCode(cur_tlcs900h->first & 0xF))
	{
		int16 displacement = (int16)fetch16();
		cur_tlcs900h->cycles = 8;
		cur_tlcs900h->pc += displacement;
	}
	else
	{
		cur_tlcs900h->cycles = 4;
		fetch16();
	}
}

//===== LDX dst,src
void sngLDX()
{
	uint8 dst, src;

	FETCH8;			//00
	dst = FETCH8;	//#8
	FETCH8;			//00
	src = FETCH8;	//#
	FETCH8;			//00

	storeB(dst, src);
	cur_tlcs900h->cycles = 9;
}

//===== SWI num
void sngSWI()
{
	cur_tlcs900h->cycles = 16;

	//printf("SWI: %02x\n", cur_tlcs900h->first & 0x7);
	switch(cur_tlcs900h->first & 7)
	{
	//System Call
	case 1: push32(cur_tlcs900h->pc);
		cur_tlcs900h->pc = loadL(0xFFFE00 + ((rCodeB(0x31) & 0x1F) << 2));
		break;

	case 3: set_interrupt(0, true); break;  //SWI 3
	case 4: set_interrupt(1, true); break;  //SWI 4
	case 5: set_interrupt(2, true); break;  //SWI 5
	case 6: set_interrupt(3, true); break;  //SWI 6

	default: instruction_error("SWI %d is not valid.", cur_tlcs900h->first & 7); break;
	}
}
//=============================================================================
