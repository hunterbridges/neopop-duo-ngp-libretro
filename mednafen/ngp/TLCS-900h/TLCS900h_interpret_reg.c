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

	TLCS900h_interpret_reg.c

//=========================================================================
//---------------------------------------------------------------------------

  History of changes:
  ===================

20 JUL 2002 - neopop_uk
=======================================
- Cleaned and tidied up for the source release

22 JUL 2002 - neopop_uk
=======================================
- Added ANDCF,ORCF and XORCF in A mode and improved the remaining ones.

22 JUL 2002 - neopop_uk
=======================================
- Added LINK and UNLK to fix "Rockman Battle and Fighters"

23 JUL 2002 - neopop_uk
=======================================
- Added MIRR to fix "Card Fighters 2"
- Added cycle counts for TSET.

28 JUL 2002 - neopop_uk
=======================================
- Converted DIV/DIVS to use the generic function

16 AUG 2002 - neopop_uk
=======================================
- Removed all of the 'second & 7' with R as it's pre-calculated anyway.
- Fixed V flag emulation of INC/DEC, fixes "Cotton" menus
- Fixed MINC4

21 AUG 2002 - neopop_uk
=======================================
- Added TSET and MULA, both untested.

04 SEP 2002 - neopop_uk
=======================================
- Fixed GCC compatibility.

//---------------------------------------------------------------------------
*/

#include "TLCS900h_interpret.h"
#include "TLCS900h_registers.h"
#include "../mem.h"
#include "../dma.h"

//=========================================================================

//===== LD r,#
void regLDi()
{
	switch(cur_tlcs900h->size)
	{
	case 0:	rCodeB(cur_tlcs900h->rCode) = FETCH8;		cur_tlcs900h->cycles = 4; break;
	case 1:	rCodeW(cur_tlcs900h->rCode) = fetch16();	cur_tlcs900h->cycles = 4;	break;
	case 2: rCodeL(cur_tlcs900h->rCode) = fetch32();	cur_tlcs900h->cycles = 6;	break;
	}
}

//===== PUSH r
void regPUSH()
{
	switch(cur_tlcs900h->size)
	{
	case 0:	push8(rCodeB(cur_tlcs900h->rCode));  cur_tlcs900h->cycles = 5;break;
	case 1:	push16(rCodeW(cur_tlcs900h->rCode)); cur_tlcs900h->cycles = 5;break;
	case 2: push32(rCodeL(cur_tlcs900h->rCode)); cur_tlcs900h->cycles = 7;break;
	}
}

//===== POP r
void regPOP()
{
	switch(cur_tlcs900h->size)
	{
	case 0:	rCodeB(cur_tlcs900h->rCode) = pop8();		cur_tlcs900h->cycles = 6;break;
	case 1:	rCodeW(cur_tlcs900h->rCode) = pop16();	cur_tlcs900h->cycles = 6;break;
	case 2: rCodeL(cur_tlcs900h->rCode) = pop32(); 	cur_tlcs900h->cycles = 8;break;
	}
}

//===== CPL r
void regCPL()
{
	switch(cur_tlcs900h->size)
	{
	case 0: rCodeB(cur_tlcs900h->rCode) = ~ rCodeB(cur_tlcs900h->rCode);	break;
	case 1: rCodeW(cur_tlcs900h->rCode) = ~ rCodeW(cur_tlcs900h->rCode);	break;
	}

	SETFLAG_H1;
	SETFLAG_N1;
	cur_tlcs900h->cycles = 4;
}

//===== NEG r
void regNEG()
{
	switch(cur_tlcs900h->size)
	{
	case 0: rCodeB(cur_tlcs900h->rCode) = generic_SUB_B(0, rCodeB(cur_tlcs900h->rCode)); break;
	case 1: rCodeW(cur_tlcs900h->rCode) = generic_SUB_W(0, rCodeW(cur_tlcs900h->rCode)); break;
	}
	cur_tlcs900h->cycles = 5;
}

//===== MUL rr,#
void regMULi()
{
	uint8 target = get_rr_Target();
	if (target == 0x80)
	{
#ifdef NEOPOP_DEBUG
		instruction_error("reg: MULi bad \'rr\' dst code");
#endif
		return;
	}

	switch(cur_tlcs900h->size)
	{
	case 0: rCodeW(target) = (rCodeW(target) & 0xFF) * FETCH8;
		cur_tlcs900h->cycles = 18; break;
	case 1: rCodeL(target) = (rCodeL(target) & 0xFFFF) * fetch16();
		cur_tlcs900h->cycles = 26; break;
	}
}

//===== MULS rr,#
void regMULSi()
{
	uint8 target = get_rr_Target();
	if (target == 0x80)
	{
		instruction_error("reg: MULSi bad \'rr\' dst code");
		return;
	}

	switch(cur_tlcs900h->size)
	{
	case 0: rCodeW(target) = (int8)(rCodeW(target) & 0xFF) * (int8)FETCH8;
		cur_tlcs900h->cycles = 18; break;
	case 1: rCodeL(target) = (int16)(rCodeL(target) & 0xFFFF) * (int16)fetch16();
		cur_tlcs900h->cycles = 26; break;
	}
}

//===== DIV rr,#
void regDIVi()
{
	uint8 target = get_rr_Target();
	if (target == 0x80)
	{
		instruction_error("reg: DIVi bad \'rr\' dst code");
		return;
	}

	switch(cur_tlcs900h->size)
	{
	case 0: {	rCodeW(target) =  generic_DIV_B(rCodeW(target), FETCH8);
				cur_tlcs900h->cycles = 22;
				break;	}

	case 1: {	rCodeL(target) =  generic_DIV_W(rCodeL(target), fetch16());
				cur_tlcs900h->cycles = 30;
				break;	}
	}
}

//===== DIVS rr,#
void regDIVSi()
{
	uint8 target = get_rr_Target();
	if (target == 0x80)
	{
		instruction_error("reg: DIVSi bad \'rr\' dst code");
		return;
	}

	switch(cur_tlcs900h->size)
	{
	case 0: {	rCodeW(target) =  generic_DIVS_B(rCodeW(target), FETCH8);
				cur_tlcs900h->cycles = 24;
				break;	}

	case 1: {	rCodeL(target) =  generic_DIVS_W(rCodeL(target), fetch16());
				cur_tlcs900h->cycles = 32;
				break;	}
	}
}

//===== LINK r,dd
void regLINK()
{
	int16 d = (int16)fetch16();
	push32(rCodeL(cur_tlcs900h->rCode));
	rCodeL(cur_tlcs900h->rCode) = REGXSP;
	REGXSP += d;
	cur_tlcs900h->cycles = 10;
}

//===== UNLK r
void regUNLK()
{
	REGXSP = rCodeL(cur_tlcs900h->rCode);
	rCodeL(cur_tlcs900h->rCode) = pop32();
	cur_tlcs900h->cycles = 8;
}

//===== BS1F A,r
void regBS1F()
{
	uint16 data = rCodeW(cur_tlcs900h->rCode), mask = 0x0001;
	uint8 i;
	
	SETFLAG_V0;
	for (i = 0; i < 15; i++)
	{
		if (data & mask)
		{
			REGA = i;
			return;
		}

		mask <<= 1;
	}

	SETFLAG_V1;
	cur_tlcs900h->cycles = 4;
}

//===== BS1B A,r
void regBS1B()
{
	uint16 data = rCodeW(cur_tlcs900h->rCode), mask = 0x8000;
	uint8 i;
	
	SETFLAG_V0;
	for (i = 0; i < 15; i++)
	{
		if (data & mask)
		{
			REGA = 15 - i;
			return;
		}

		mask >>= 1;
	}

	SETFLAG_V1;
	cur_tlcs900h->cycles = 4;
}

//===== DAA r
void regDAA()
{
	uint16 resultC;
	uint8 src = rCodeB(cur_tlcs900h->rCode), result, added = 0, half;
	bool setC = false;

	uint8 upper4 = (src & 0xF0);
	uint8 lower4 = (src & 0x0F);

	if (FLAG_C)	// {C = 1}
	{
		if (FLAG_H)	// {H = 1}
		{
			setC = true;
			added = 0x66;
		}
		else		// {H = 0}
		{
			if      (lower4 < 0x0a)		{ added = 0x60; }
			else						{ added = 0x66; }
			setC = true;
		}
	}
	else	// {C = 0}
	{
		if (FLAG_H)	// {H = 1}
		{
			if		(src < 0x9A)		{ added = 0x06; }
			else						{ added = 0x66; }
		}
		else		// {H = 0}
		{
			if		((upper4 < 0x90) && (lower4 > 0x9))	{ added = 0x06; }
			else if ((upper4 > 0x80) && (lower4 > 0x9))	{ added = 0x66; }
			else if ((upper4 > 0x90) && (lower4 < 0xa))	{ added = 0x60; }
		}
	}

	if (FLAG_N)
	{
		resultC = (uint16)src - (uint16)added;
		half = (src & 0xF) - (added & 0xF);
	}
	else
	{
		resultC = (uint16)src + (uint16)added;
		half = (src & 0xF) + (added & 0xF);
	}

	result = (uint8)(resultC & 0xFF);	

	SETFLAG_S(result & 0x80);
	SETFLAG_Z(result == 0);
	SETFLAG_H(half > 0xF);

	if (FLAG_N)		SETFLAG_C(result > src || setC)
	else			SETFLAG_C(result < src || setC)
	
	parityB(result);
	rCodeB(cur_tlcs900h->rCode) = result;
	cur_tlcs900h->cycles = 6;
}

//===== EXTZ r
void regEXTZ()
{
	switch(cur_tlcs900h->size)
	{
	case 1:	rCodeW(cur_tlcs900h->rCode) &= 0xFF;	break;
	case 2: rCodeL(cur_tlcs900h->rCode) &= 0xFFFF;	break;
	}

	cur_tlcs900h->cycles = 4;
}

//===== EXTS r
void regEXTS()
{
	switch(cur_tlcs900h->size)
	{
	case 1:	if (rCodeW(cur_tlcs900h->rCode) & 0x0080) 
				{ rCodeW(cur_tlcs900h->rCode) |= 0xFF00; } else 
				{ rCodeW(cur_tlcs900h->rCode) &= 0x00FF; }
		break;
		
	case 2:	if (rCodeL(cur_tlcs900h->rCode) & 0x00008000) 
				{ rCodeL(cur_tlcs900h->rCode) |= 0xFFFF0000; } else 
				{ rCodeL(cur_tlcs900h->rCode) &= 0x0000FFFF; }
		break;
	}

	cur_tlcs900h->cycles = 5;
}

//===== PAA r
void regPAA()
{
	switch(cur_tlcs900h->size)
	{
	case 1:	if (rCodeW(cur_tlcs900h->rCode) & 0x1) rCodeW(cur_tlcs900h->rCode)++; break;
	case 2:	if (rCodeL(cur_tlcs900h->rCode) & 0x1) rCodeL(cur_tlcs900h->rCode)++; break;
	}
	cur_tlcs900h->cycles = 4;
}

//===== MIRR r
void regMIRR()
{
	uint16 src = rCodeW(cur_tlcs900h->rCode), dst = 0, bit;

	//Undocumented - see p165 of CPU .PDF
	//Seems to mirror bits completely, ie. 1234 -> 4321

	for (bit = 0; bit < 16; bit++)
		if (src & (1 << bit))
			dst |= (1 << (15 - bit));

	rCodeW(cur_tlcs900h->rCode) = dst;
	cur_tlcs900h->cycles = 4;
}

//===== MULA rr
void regMULA()
{
	uint32 src = (int16)loadW(regL(2/*XDE*/)) * (int16)loadW(regL(3/*XHL*/));
	uint32 dst = rCodeL(cur_tlcs900h->rCode);
	uint32 result = dst + src;

	SETFLAG_S(result & 0x80000000);
	SETFLAG_Z(result == 0);

	if ((((int32)dst >= 0) && ((int32)src >= 0) && ((int32)result < 0)) || 
		(((int32)dst < 0)  && ((int32)src < 0) && ((int32)result >= 0)))
	{SETFLAG_V1} else {SETFLAG_V0}
	
	cur_tlcs900h->cycles = 31;
}

//===== DJNZ r,d
void regDJNZ()
{
	int8 offset = FETCH8;

	cur_tlcs900h->cycles = 7;

	switch(cur_tlcs900h->size)
	{
	case 0: 
		rCodeB(cur_tlcs900h->rCode) --;
		if (rCodeB(cur_tlcs900h->rCode) != 0)
		{
			cur_tlcs900h->cycles = 11;
			cur_tlcs900h->pc = cur_tlcs900h->pc + offset;
		}
		break;

	case 1: 
		rCodeW(cur_tlcs900h->rCode) --;
		if (rCodeW(cur_tlcs900h->rCode) != 0)
		{
			cur_tlcs900h->cycles = 11;
			cur_tlcs900h->pc = cur_tlcs900h->pc + offset;
		}
		break;
	}
}

//===== ANDCF #,r
void regANDCFi()
{
	uint8 data, bit = FETCH8 & 0xF;
	switch(cur_tlcs900h->size)
	{
	case 0: {	data = (rCodeB(cur_tlcs900h->rCode) >> bit) & 1;
				if (bit < 8) SETFLAG_C(FLAG_C & data); 
				break; }

	case 1: {	data = (rCodeW(cur_tlcs900h->rCode) >> bit) & 1;
				SETFLAG_C(FLAG_C & data); 
				break; }
	}
	cur_tlcs900h->cycles = 4;
}

//===== ORCF #,r
void regORCFi()
{
	uint8 data, bit = FETCH8 & 0xF;
	switch(cur_tlcs900h->size)
	{
	case 0: {	data = (rCodeB(cur_tlcs900h->rCode) >> bit) & 1;
				if (bit < 8) SETFLAG_C(FLAG_C | data); 
				break; }

	case 1: {	data = (rCodeW(cur_tlcs900h->rCode) >> bit) & 1;
				SETFLAG_C(FLAG_C | data); 
				break; }
	}
	cur_tlcs900h->cycles = 4;
}

//===== XORCF #,r
void regXORCFi()
{
	uint8 data, bit = FETCH8 & 0xF;
	switch(cur_tlcs900h->size)
	{
	case 0: {	data = (rCodeB(cur_tlcs900h->rCode) >> bit) & 1;
				if (bit < 8) SETFLAG_C(FLAG_C ^ data); 
				break; }

	case 1: {	data = (rCodeW(cur_tlcs900h->rCode) >> bit) & 1;
				SETFLAG_C(FLAG_C ^ data); 
				break; }
	}
	cur_tlcs900h->cycles = 4;
}

//===== LDCF #,r
void regLDCFi()
{
	uint8 bit = FETCH8 & 0xF;
	switch(cur_tlcs900h->size)
	{
	case 0: {	uint8 mask = (1 << bit);
				if (bit < 8)
					SETFLAG_C(rCodeB(cur_tlcs900h->rCode) & mask);
				break; }


	case 1: {	uint16 mask = (1 << bit);
				SETFLAG_C(rCodeW(cur_tlcs900h->rCode) & mask);
				break; }
	}

	cur_tlcs900h->cycles = 4;
}

//===== STCF #,r
void regSTCFi()
{
	uint8 bit = FETCH8 & 0xF;
	switch(cur_tlcs900h->size)
	{
	case 0: {	uint8 cmask = ~(1 << bit);
				uint8 set = FLAG_C << bit;
				if (bit < 8) rCodeB(cur_tlcs900h->rCode) = (rCodeB(cur_tlcs900h->rCode) & cmask) | set;
				break;	}

	case 1: {	uint16 cmask = ~(1 << bit);
				uint16 set = FLAG_C << bit;
				rCodeW(cur_tlcs900h->rCode) = (rCodeW(cur_tlcs900h->rCode) & cmask) | set;
				break;	}
	}

	cur_tlcs900h->cycles = 4;
}

//===== ANDCF A,r
void regANDCFA()
{
	uint8 data, bit = REGA & 0xF;
	switch(cur_tlcs900h->size)
	{
	case 0: {	data = (rCodeB(cur_tlcs900h->rCode) >> bit) & 1;
				if (bit < 8) SETFLAG_C(FLAG_C & data); 
				break; }

	case 1: {	data = (rCodeW(cur_tlcs900h->rCode) >> bit) & 1;
				SETFLAG_C(FLAG_C & data); 
				break; }
	}
	cur_tlcs900h->cycles = 4;
}

//===== ORCF A,r
void regORCFA()
{
	uint8 data, bit = REGA & 0xF;
	switch(cur_tlcs900h->size)
	{
	case 0: {	data = (rCodeB(cur_tlcs900h->rCode) >> bit) & 1;
				if (bit < 8) SETFLAG_C(FLAG_C | data); 
				break; }

	case 1: {	data = (rCodeW(cur_tlcs900h->rCode) >> bit) & 1;
				SETFLAG_C(FLAG_C | data); 
				break; }
	}
	cur_tlcs900h->cycles = 4;
}

//===== XORCF A,r
void regXORCFA()
{
	uint8 data, bit = REGA & 0xF;
	switch(cur_tlcs900h->size)
	{
	case 0: {	data = (rCodeB(cur_tlcs900h->rCode) >> bit) & 1;
				if (bit < 8) SETFLAG_C(FLAG_C ^ data); 
				break; }

	case 1: {	data = (rCodeW(cur_tlcs900h->rCode) >> bit) & 1;
				SETFLAG_C(FLAG_C ^ data); 
				break; }
	}
	cur_tlcs900h->cycles = 4;
}

//===== LDCF A,r
void regLDCFA()
{
	uint8 bit = REGA & 0xF;
	uint32 mask = (1 << bit);

	switch(cur_tlcs900h->size)
	{
	case 0: if (bit < 8) SETFLAG_C(rCodeB(cur_tlcs900h->rCode) & mask); break;
	case 1: SETFLAG_C(rCodeW(cur_tlcs900h->rCode) & mask); break;
	}

	cur_tlcs900h->cycles = 4;
}

//===== STCF A,r
void regSTCFA()
{
	switch(cur_tlcs900h->size)
	{
	case 0: {	uint8 bit = REGA & 0xF;
				uint8 cmask = ~(1 << bit);
				uint8 set = FLAG_C << bit;
				if (bit < 8) rCodeB(cur_tlcs900h->rCode) = (rCodeB(cur_tlcs900h->rCode) & cmask) | set;
				break;	}

	case 1: {	uint8 bit = REGA & 0xF;
				uint16 cmask = ~(1 << bit);
				uint16 set = FLAG_C << bit;
				rCodeW(cur_tlcs900h->rCode) = (rCodeW(cur_tlcs900h->rCode) & cmask) | set;
				break;	}
	}

	cur_tlcs900h->cycles = 4;
}

//===== LDC cr,r
void regLDCcrr()
{
	uint8 cr = FETCH8;

	switch(cur_tlcs900h->size)
	{
	case 0: dmaStoreB(cr, rCodeB(cur_tlcs900h->rCode)); break;
	case 1: dmaStoreW(cr, rCodeW(cur_tlcs900h->rCode)); break;
	case 2: dmaStoreL(cr, rCodeL(cur_tlcs900h->rCode)); break;
	}

	cur_tlcs900h->cycles = 8;
}

//===== LDC r,cr
void regLDCrcr()
{
	uint8 cr = FETCH8;

	switch(cur_tlcs900h->size)
	{
	case 0: rCodeB(cur_tlcs900h->rCode) = dmaLoadB(cr); break;
	case 1: rCodeW(cur_tlcs900h->rCode) = dmaLoadW(cr); break;
	case 2: rCodeL(cur_tlcs900h->rCode) = dmaLoadL(cr); break;
	}

	cur_tlcs900h->cycles = 8;
}

//===== RES #,r
void regRES()
{
	uint8 b = FETCH8 & 0xF;

	switch(cur_tlcs900h->size)
	{
	case 0: rCodeB(cur_tlcs900h->rCode) &= ~(uint8)(1 << b); break;
	case 1: rCodeW(cur_tlcs900h->rCode) &= ~(uint16)(1 << b); break;
	}

	cur_tlcs900h->cycles = 4;
}

//===== SET #,r
void regSET()
{
	uint8 b = FETCH8 & 0xF;

	switch(cur_tlcs900h->size)
	{
	case 0: rCodeB(cur_tlcs900h->rCode) |= (1 << b); break;
	case 1: rCodeW(cur_tlcs900h->rCode) |= (1 << b); break;
	}

	cur_tlcs900h->cycles = 4;
}

//===== CHG #,r
void regCHG()
{
	uint8 b = FETCH8 & 0xF;

	switch(cur_tlcs900h->size)
	{
	case 0: rCodeB(cur_tlcs900h->rCode) ^= (1 << b); break;
	case 1: rCodeW(cur_tlcs900h->rCode) ^= (1 << b); break;
	}

	cur_tlcs900h->cycles = 4;
}

//===== BIT #,r
void regBIT()
{
	uint8 b = FETCH8 & 0xF;
	
	switch(cur_tlcs900h->size)
	{
	case 0:	SETFLAG_Z(! (rCodeB(cur_tlcs900h->rCode) & (1 << b))	);	break;
	case 1:	SETFLAG_Z(! (rCodeW(cur_tlcs900h->rCode) & (1 << b))	);	break;
	}

	SETFLAG_H1;
	SETFLAG_N0;
	cur_tlcs900h->cycles = 4;
}

//===== TSET #,r
void regTSET()
{
	uint8 b = FETCH8 & 0xF;
	
	switch(cur_tlcs900h->size)
	{
	case 0:	SETFLAG_Z(! (rCodeB(cur_tlcs900h->rCode) & (1 << b))	);	
			rCodeB(cur_tlcs900h->rCode) |= (1 << b);
			break;

	case 1:	SETFLAG_Z(! (rCodeW(cur_tlcs900h->rCode) & (1 << b))	);
			rCodeW(cur_tlcs900h->rCode) |= (1 << b);
			break;
	}

	SETFLAG_H1
	SETFLAG_N0
	cur_tlcs900h->cycles = 6;
}

//===== MINC1 #,r
void regMINC1()
{
	uint16 num = fetch16() + 1;

	if (cur_tlcs900h->size == 1 && num)
	{
		if ((rCodeW(cur_tlcs900h->rCode) % num) == (num - 1))
			rCodeW(cur_tlcs900h->rCode) -= (num - 1);
		else
			rCodeW(cur_tlcs900h->rCode) += 1;
	}

	cur_tlcs900h->cycles = 8;
}

//===== MINC2 #,r
void regMINC2()
{
	uint16 num = fetch16() + 2;

	if (cur_tlcs900h->size == 1 && num)
	{
		if ((rCodeW(cur_tlcs900h->rCode) % num) == (num - 2))
			rCodeW(cur_tlcs900h->rCode) -= (num - 2);
		else
			rCodeW(cur_tlcs900h->rCode) += 2;
	}

	cur_tlcs900h->cycles = 8;
}

//===== MINC4 #,r
void regMINC4()
{
	uint16 num = fetch16() + 4;

	if (cur_tlcs900h->size == 1 && num)
	{
		if ((rCodeW(cur_tlcs900h->rCode) % num) == (num - 4))
			rCodeW(cur_tlcs900h->rCode) -= (num - 4);
		else
			rCodeW(cur_tlcs900h->rCode) += 4;
	}

	cur_tlcs900h->cycles = 8;
}

//===== MDEC1 #,r
void regMDEC1()
{
	uint16 num = fetch16() + 1;

	if (cur_tlcs900h->size == 1 && num)
	{
		if ((rCodeW(cur_tlcs900h->rCode) % num) == 0)
			rCodeW(cur_tlcs900h->rCode) += (num - 1);
		else
			rCodeW(cur_tlcs900h->rCode) -= 1;
	}

	cur_tlcs900h->cycles = 7;
}

//===== MDEC2 #,r
void regMDEC2()
{
	uint16 num = fetch16() + 2;

	if (cur_tlcs900h->size == 1 && num)
	{
		if ((rCodeW(cur_tlcs900h->rCode) % num) == 0)
			rCodeW(cur_tlcs900h->rCode) += (num - 2);
		else
			rCodeW(cur_tlcs900h->rCode) -= 2;
	}

	cur_tlcs900h->cycles = 7;
}

//===== MDEC4 #,r
void regMDEC4()
{
	uint16 num = fetch16() + 4;

	if (cur_tlcs900h->size == 1 && num)
	{
		if ((rCodeW(cur_tlcs900h->rCode) % num) == 0)
			rCodeW(cur_tlcs900h->rCode) += (num - 4);
		else
			rCodeW(cur_tlcs900h->rCode) -= 4;
	}

	cur_tlcs900h->cycles = 7;
}

//===== MUL RR,r
void regMUL()
{
	uint8 target = get_RR_Target();
	if (target == 0x80)
	{
		instruction_error("reg: MUL bad \'RR\' dst code");
		return;
	}

	switch(cur_tlcs900h->size)
	{
	case 0: rCodeW(target) = (rCodeW(target) & 0xFF) * rCodeB(cur_tlcs900h->rCode);
		cur_tlcs900h->cycles = 18; break;
	case 1: rCodeL(target) = (rCodeL(target) & 0xFFFF) * rCodeW(cur_tlcs900h->rCode);	
		cur_tlcs900h->cycles = 26; break;
	}
}

//===== MULS RR,r
void regMULS()
{
	uint8 target = get_RR_Target();
	if (target == 0x80)
	{
		instruction_error("reg: MUL bad \'RR\' dst code");
		return;
	}

	switch(cur_tlcs900h->size)
	{
	case 0: rCodeW(target) = (int8)(rCodeW(target) & 0xFF) * (int8)rCodeB(cur_tlcs900h->rCode);	
		cur_tlcs900h->cycles = 18; break;
	case 1: rCodeL(target) = (int16)(rCodeL(target) & 0xFFFF) * (int16)rCodeW(cur_tlcs900h->rCode);	
		cur_tlcs900h->cycles = 26; break;
	}
}

//===== DIV RR,r
void regDIV()
{
	uint8 target = get_RR_Target();
	if (target == 0x80)
	{
		instruction_error("reg: DIV bad \'RR\' dst code");
		return;
	}

	switch(cur_tlcs900h->size)
	{
	case 0: {	rCodeW(target) =  generic_DIV_B(rCodeW(target), rCodeB(cur_tlcs900h->rCode));
				cur_tlcs900h->cycles = 22;
				break;	}

	case 1: {	rCodeL(target) =  generic_DIV_W(rCodeL(target), rCodeW(cur_tlcs900h->rCode));
				cur_tlcs900h->cycles = 30;
				break;	}
	}
}

//===== DIVS RR,r
void regDIVS()
{
	uint8 target = get_RR_Target();
	if (target == 0x80)
	{
		instruction_error("reg: DIVS bad \'RR\' dst code");
		return;
	}

	switch(cur_tlcs900h->size)
	{
	case 0: {	rCodeW(target) = generic_DIVS_B(rCodeW(target), rCodeB(cur_tlcs900h->rCode));
				cur_tlcs900h->cycles = 24;
				break;	}

	case 1: {	rCodeL(target) = generic_DIVS_W(rCodeL(target), rCodeW(cur_tlcs900h->rCode));
				cur_tlcs900h->cycles = 32;
				break;	}
	}
}

//===== INC #3,r
void regINC()
{
	uint8 val = cur_tlcs900h->R;
	if (val == 0)
		val = 8;

	switch(cur_tlcs900h->size)
	{
	case 0: {	uint8 dst = rCodeB(cur_tlcs900h->rCode);
				uint8 half = (dst & 0xF) + val;
				uint32 resultC = dst + val;
				uint8 result = (uint8)(resultC & 0xFF);
				SETFLAG_S(result & 0x80);

				if (((int8)dst >= 0) && ((int8)result < 0))
				{SETFLAG_V1} else {SETFLAG_V0}

				SETFLAG_H(half > 0xF);
				SETFLAG_Z(result == 0);
				SETFLAG_N0;
				rCodeB(cur_tlcs900h->rCode) = result;
				break;	}

	case 1: {	rCodeW(cur_tlcs900h->rCode) += val; break; }

	case 2: {	rCodeL(cur_tlcs900h->rCode) += val; break; }
	}

	cur_tlcs900h->cycles = 4;
}

//===== DEC #3,r
void regDEC()
{
	uint8 val = cur_tlcs900h->R;
	if (val == 0)
		val = 8;

	switch(cur_tlcs900h->size)
	{
	case 0: {	uint8 dst = rCodeB(cur_tlcs900h->rCode);
				uint8 half = (dst & 0xF) - val;
				uint32 resultC = dst - val;
				uint8 result = (uint8)(resultC & 0xFF);
				SETFLAG_S(result & 0x80);

				if (((int8)dst < 0) && ((int8)result >= 0))
				{SETFLAG_V1} else {SETFLAG_V0}

				SETFLAG_H(half > 0xF);
				SETFLAG_Z(result == 0);
				SETFLAG_N1;
				rCodeB(cur_tlcs900h->rCode) = result;
				cur_tlcs900h->cycles = 4;
				break;	}

	case 1: {	rCodeW(cur_tlcs900h->rCode) -= val; cur_tlcs900h->cycles = 4; break; }

	case 2: {	rCodeL(cur_tlcs900h->rCode) -= val; cur_tlcs900h->cycles = 5; break; }
	}
}

//===== SCC cc,r
void regSCC()
{
	uint32 result;

	if (conditionCode(cur_tlcs900h->second & 0xF))
		result = 1;
	else
		result = 0;

	switch(cur_tlcs900h->size)
	{
	case 0: rCodeB(cur_tlcs900h->rCode) = (uint8)result; break;
	case 1: rCodeW(cur_tlcs900h->rCode) = (uint16)result; break;
	}

	cur_tlcs900h->cycles = 6;
}

//===== LD R,r
void regLDRr()
{
	switch(cur_tlcs900h->size)
	{
	case 0:	regB(cur_tlcs900h->R) = rCodeB(cur_tlcs900h->rCode);	break;
	case 1:	regW(cur_tlcs900h->R) = rCodeW(cur_tlcs900h->rCode);	break;
	case 2: regL(cur_tlcs900h->R) = rCodeL(cur_tlcs900h->rCode);	break;
	}

	cur_tlcs900h->cycles = 4;
}

//===== LD r,R
void regLDrR()
{
	switch(cur_tlcs900h->size)
	{
	case 0:	rCodeB(cur_tlcs900h->rCode) = regB(cur_tlcs900h->R);	break;
	case 1:	rCodeW(cur_tlcs900h->rCode) = regW(cur_tlcs900h->R);	break;
	case 2: rCodeL(cur_tlcs900h->rCode) = regL(cur_tlcs900h->R);	break;
	}

	cur_tlcs900h->cycles = 4;
}

//===== ADD R,r
void regADD()
{
	switch(cur_tlcs900h->size)
	{
	case 0: regB(cur_tlcs900h->R) = generic_ADD_B(regB(cur_tlcs900h->R), rCodeB(cur_tlcs900h->rCode)); cur_tlcs900h->cycles = 4; break;
	case 1: regW(cur_tlcs900h->R) = generic_ADD_W(regW(cur_tlcs900h->R), rCodeW(cur_tlcs900h->rCode)); cur_tlcs900h->cycles = 4; break;
	case 2: regL(cur_tlcs900h->R) = generic_ADD_L(regL(cur_tlcs900h->R), rCodeL(cur_tlcs900h->rCode)); cur_tlcs900h->cycles = 7; break;
	}
}

//===== ADC R,r
void regADC()
{
	switch(cur_tlcs900h->size)
	{
	case 0: regB(cur_tlcs900h->R) = generic_ADC_B(regB(cur_tlcs900h->R), rCodeB(cur_tlcs900h->rCode)); cur_tlcs900h->cycles = 4; break;
	case 1: regW(cur_tlcs900h->R) = generic_ADC_W(regW(cur_tlcs900h->R), rCodeW(cur_tlcs900h->rCode)); cur_tlcs900h->cycles = 4; break;
	case 2: regL(cur_tlcs900h->R) = generic_ADC_L(regL(cur_tlcs900h->R), rCodeL(cur_tlcs900h->rCode)); cur_tlcs900h->cycles = 7; break;
	}
}

//===== SUB R,r
void regSUB()
{
	switch(cur_tlcs900h->size)
	{
	case 0: regB(cur_tlcs900h->R) = generic_SUB_B(regB(cur_tlcs900h->R), rCodeB(cur_tlcs900h->rCode)); cur_tlcs900h->cycles = 4; break;
	case 1: regW(cur_tlcs900h->R) = generic_SUB_W(regW(cur_tlcs900h->R), rCodeW(cur_tlcs900h->rCode)); cur_tlcs900h->cycles = 4; break;
	case 2: regL(cur_tlcs900h->R) = generic_SUB_L(regL(cur_tlcs900h->R), rCodeL(cur_tlcs900h->rCode)); cur_tlcs900h->cycles = 7; break;
	}
}

//===== SBC R,r
void regSBC()
{
	switch(cur_tlcs900h->size)
	{
	case 0: regB(cur_tlcs900h->R) = generic_SBC_B(regB(cur_tlcs900h->R), rCodeB(cur_tlcs900h->rCode)); cur_tlcs900h->cycles = 4; break;
	case 1: regW(cur_tlcs900h->R) = generic_SBC_W(regW(cur_tlcs900h->R), rCodeW(cur_tlcs900h->rCode)); cur_tlcs900h->cycles = 4; break;
	case 2: regL(cur_tlcs900h->R) = generic_SBC_L(regL(cur_tlcs900h->R), rCodeL(cur_tlcs900h->rCode)); cur_tlcs900h->cycles = 7; break;
	}
}

//===== LD r,#3
void regLDr3()
{
	switch(cur_tlcs900h->size)
	{
	case 0:	rCodeB(cur_tlcs900h->rCode) = cur_tlcs900h->R;	break;
	case 1:	rCodeW(cur_tlcs900h->rCode) = cur_tlcs900h->R;	break;
	case 2:	rCodeL(cur_tlcs900h->rCode) = cur_tlcs900h->R;	break;
	}

	cur_tlcs900h->cycles = 4;
}

//===== EX R,r
void regEX()
{
	switch(cur_tlcs900h->size)
	{
	case 0:	{ uint8  temp = regB(cur_tlcs900h->R); regB(cur_tlcs900h->R) = rCodeB(cur_tlcs900h->rCode); rCodeB(cur_tlcs900h->rCode) = temp; break;}
	case 1:	{ uint16 temp = regW(cur_tlcs900h->R); regW(cur_tlcs900h->R) = rCodeW(cur_tlcs900h->rCode); rCodeW(cur_tlcs900h->rCode) = temp; break;}
	case 2:	{ uint32 temp = regL(cur_tlcs900h->R); regL(cur_tlcs900h->R) = rCodeL(cur_tlcs900h->rCode); rCodeL(cur_tlcs900h->rCode) = temp; break;}
	}

	cur_tlcs900h->cycles = 5;
}

//===== ADD r,#
void regADDi()
{
	switch(cur_tlcs900h->size)
	{
	case 0: rCodeB(cur_tlcs900h->rCode) = generic_ADD_B(rCodeB(cur_tlcs900h->rCode), FETCH8); cur_tlcs900h->cycles = 4;break;
	case 1: rCodeW(cur_tlcs900h->rCode) = generic_ADD_W(rCodeW(cur_tlcs900h->rCode), fetch16()); cur_tlcs900h->cycles = 4;break;
	case 2: rCodeL(cur_tlcs900h->rCode) = generic_ADD_L(rCodeL(cur_tlcs900h->rCode), fetch32()); cur_tlcs900h->cycles = 7;break;
	}
}

//===== ADC r,#
void regADCi()
{
	switch(cur_tlcs900h->size)
	{
	case 0: rCodeB(cur_tlcs900h->rCode) = generic_ADC_B(rCodeB(cur_tlcs900h->rCode), FETCH8); cur_tlcs900h->cycles = 4;break;
	case 1: rCodeW(cur_tlcs900h->rCode) = generic_ADC_W(rCodeW(cur_tlcs900h->rCode), fetch16()); cur_tlcs900h->cycles = 4;break;
	case 2: rCodeL(cur_tlcs900h->rCode) = generic_ADC_L(rCodeL(cur_tlcs900h->rCode), fetch32()); cur_tlcs900h->cycles = 7;break;
	}
}

//===== SUB r,#
void regSUBi()
{
	switch(cur_tlcs900h->size)
	{
	case 0: rCodeB(cur_tlcs900h->rCode) = generic_SUB_B(rCodeB(cur_tlcs900h->rCode), FETCH8); cur_tlcs900h->cycles = 4;break;
	case 1: rCodeW(cur_tlcs900h->rCode) = generic_SUB_W(rCodeW(cur_tlcs900h->rCode), fetch16()); cur_tlcs900h->cycles = 4;break;
	case 2: rCodeL(cur_tlcs900h->rCode) = generic_SUB_L(rCodeL(cur_tlcs900h->rCode), fetch32()); cur_tlcs900h->cycles = 7;break;
	}
}

//===== SBC r,#
void regSBCi()
{
	switch(cur_tlcs900h->size)
	{
	case 0: rCodeB(cur_tlcs900h->rCode) = generic_SBC_B(rCodeB(cur_tlcs900h->rCode), FETCH8); cur_tlcs900h->cycles = 4;break;
	case 1: rCodeW(cur_tlcs900h->rCode) = generic_SBC_W(rCodeW(cur_tlcs900h->rCode), fetch16()); cur_tlcs900h->cycles = 4;break;
	case 2: rCodeL(cur_tlcs900h->rCode) = generic_SBC_L(rCodeL(cur_tlcs900h->rCode), fetch32()); cur_tlcs900h->cycles = 7;break;
	}
}

//===== CP r,#
void regCPi()
{
	switch(cur_tlcs900h->size)
	{
	case 0:	generic_SUB_B(rCodeB(cur_tlcs900h->rCode), FETCH8);	cur_tlcs900h->cycles = 4;break;
	case 1:	generic_SUB_W(rCodeW(cur_tlcs900h->rCode), fetch16());cur_tlcs900h->cycles = 4;	break;
	case 2:	generic_SUB_L(rCodeL(cur_tlcs900h->rCode), fetch32());cur_tlcs900h->cycles = 7;	break;
	}
}

//===== AND r,#
void regANDi()
{
	switch(cur_tlcs900h->size)
	{
	case 0:	{	uint8 result = rCodeB(cur_tlcs900h->rCode) & FETCH8;
				rCodeB(cur_tlcs900h->rCode) = result;
				SETFLAG_Z(result == 0);
				SETFLAG_S(result & 0x80);
				parityB(result);
				cur_tlcs900h->cycles = 4;
				break; }
	
	case 1: {	uint16 result = rCodeW(cur_tlcs900h->rCode) & fetch16();
				rCodeW(cur_tlcs900h->rCode) = result;
				SETFLAG_Z(result == 0);
				SETFLAG_S(result & 0x8000);
				parityW(result);
				cur_tlcs900h->cycles = 4;
				break; }

	case 2:	{	uint32 result = rCodeL(cur_tlcs900h->rCode) & fetch32();
				rCodeL(cur_tlcs900h->rCode) = result;
				SETFLAG_Z(result == 0);
				SETFLAG_S(result & 0x80000000);
				cur_tlcs900h->cycles = 7;
				break; }
	}

	SETFLAG_H1;
	SETFLAG_N0;
	SETFLAG_C0;
}

//===== OR r,#
void regORi()
{
	switch(cur_tlcs900h->size)
	{
	case 0: {	uint8 result = rCodeB(cur_tlcs900h->rCode) | FETCH8;
				SETFLAG_S(result & 0x80);
				SETFLAG_Z(result == 0);
				rCodeB(cur_tlcs900h->rCode) = result;
				parityB(result);
				cur_tlcs900h->cycles = 4;
				break; }

	case 1: {	uint16 result = rCodeW(cur_tlcs900h->rCode) | fetch16();
				SETFLAG_S(result & 0x8000);
				SETFLAG_Z(result == 0);
				rCodeW(cur_tlcs900h->rCode) = result;
				parityW(result);
				cur_tlcs900h->cycles = 4;
				break; }

	case 2: {	uint32 result = rCodeL(cur_tlcs900h->rCode) | fetch32();
				SETFLAG_S(result & 0x80000000);
				SETFLAG_Z(result == 0);
				rCodeL(cur_tlcs900h->rCode) = result;
				cur_tlcs900h->cycles = 7;
				break; }
	}

	SETFLAG_H0;
	SETFLAG_N0;
	SETFLAG_C0;
}

//===== XOR r,#
void regXORi()
{
	switch(cur_tlcs900h->size)
	{
	case 0: {	uint8 result = rCodeB(cur_tlcs900h->rCode) ^ FETCH8;
				SETFLAG_S(result & 0x80);
				SETFLAG_Z(result == 0);
				rCodeB(cur_tlcs900h->rCode) = result;
				parityB(result);
				cur_tlcs900h->cycles = 4;
				break; }

	case 1: {	uint16 result = rCodeW(cur_tlcs900h->rCode) ^ fetch16();
				SETFLAG_S(result & 0x8000);
				SETFLAG_Z(result == 0);
				rCodeW(cur_tlcs900h->rCode) = result;
				parityW(result);
				cur_tlcs900h->cycles = 4;
				break; }

	case 2: {	uint32 result = rCodeL(cur_tlcs900h->rCode) ^ fetch32();
				SETFLAG_S(result & 0x80000000);
				SETFLAG_Z(result == 0);
				rCodeL(cur_tlcs900h->rCode) = result;
				cur_tlcs900h->cycles = 7;
				break; }
	}

	SETFLAG_H0;
	SETFLAG_N0;
	SETFLAG_C0;
}

//===== AND R,r
void regAND()
{
	switch(cur_tlcs900h->size)
	{
	case 0: {	uint8 result = regB(cur_tlcs900h->R) & rCodeB(cur_tlcs900h->rCode);
				SETFLAG_S(result & 0x80);
				SETFLAG_Z(result == 0);
				regB(cur_tlcs900h->R) = result;
				parityB(result);
				cur_tlcs900h->cycles = 4;
				break; }

	case 1: {	uint16 result = regW(cur_tlcs900h->R) & rCodeW(cur_tlcs900h->rCode);
				SETFLAG_S(result & 0x8000);
				SETFLAG_Z(result == 0);
				regW(cur_tlcs900h->R) = result;
				parityW(result);
				cur_tlcs900h->cycles = 4;
				break; }

	case 2: {	uint32 result = regL(cur_tlcs900h->R) & rCodeL(cur_tlcs900h->rCode);
				SETFLAG_S(result & 0x80000000);
				SETFLAG_Z(result == 0);
				regL(cur_tlcs900h->R) = result;
				cur_tlcs900h->cycles = 7;
				break; }
	}

	SETFLAG_H1;
	SETFLAG_N0;
	SETFLAG_C0;
}

//===== OR R,r
void regOR()
{
	switch(cur_tlcs900h->size)
	{
	case 0: {	uint8 result = regB(cur_tlcs900h->R) | rCodeB(cur_tlcs900h->rCode);
				SETFLAG_S(result & 0x80);
				SETFLAG_Z(result == 0);
				regB(cur_tlcs900h->R) = result;
				parityB(result);
				cur_tlcs900h->cycles = 4;
				break; }

	case 1: {	uint16 result = regW(cur_tlcs900h->R) | rCodeW(cur_tlcs900h->rCode);
				SETFLAG_S(result & 0x8000);
				SETFLAG_Z(result == 0);
				regW(cur_tlcs900h->R) = result;
				parityW(result);
				cur_tlcs900h->cycles = 4;
				break; }

	case 2: {	uint32 result = regL(cur_tlcs900h->R) | rCodeL(cur_tlcs900h->rCode);
				SETFLAG_S(result & 0x80000000);
				SETFLAG_Z(result == 0);
				regL(cur_tlcs900h->R) = result;
				cur_tlcs900h->cycles = 7;
				break; }
	}

	SETFLAG_H0;
	SETFLAG_N0;
	SETFLAG_C0;
}

//===== XOR R,r
void regXOR()
{
	switch(cur_tlcs900h->size)
	{
	case 0: {	uint8 result = regB(cur_tlcs900h->R) ^ rCodeB(cur_tlcs900h->rCode);
				SETFLAG_S(result & 0x80);
				SETFLAG_Z(result == 0);
				regB(cur_tlcs900h->R) = result;
				parityB(result);
				cur_tlcs900h->cycles = 4;
				break; }

	case 1: {	uint16 result = regW(cur_tlcs900h->R) ^ rCodeW(cur_tlcs900h->rCode);
				SETFLAG_S(result & 0x8000);
				SETFLAG_Z(result == 0);
				regW(cur_tlcs900h->R) = result;
				parityW(result);
				cur_tlcs900h->cycles = 4;
				break; }

	case 2: {	uint32 result = regL(cur_tlcs900h->R) ^ rCodeL(cur_tlcs900h->rCode);
				SETFLAG_S(result & 0x80000000);
				SETFLAG_Z(result == 0);
				regL(cur_tlcs900h->R) = result;
				cur_tlcs900h->cycles = 7;
				break; }
	}

	SETFLAG_H0;
	SETFLAG_N0;
	SETFLAG_C0;
}

//===== CP r,#3
void regCPr3()
{	
	switch(cur_tlcs900h->size)
	{
	case 0:	generic_SUB_B(rCodeB(cur_tlcs900h->rCode), cur_tlcs900h->R);	break;
	case 1:	generic_SUB_W(rCodeW(cur_tlcs900h->rCode), cur_tlcs900h->R);	break;
	}

	cur_tlcs900h->cycles = 4;
}

//===== CP R,r
void regCP()
{
	switch(cur_tlcs900h->size)
	{
	case 0:	generic_SUB_B(regB(cur_tlcs900h->R), rCodeB(cur_tlcs900h->rCode));cur_tlcs900h->cycles = 4;	break;
	case 1:	generic_SUB_W(regW(cur_tlcs900h->R), rCodeW(cur_tlcs900h->rCode));cur_tlcs900h->cycles = 4;	break;
	case 2:	generic_SUB_L(regL(cur_tlcs900h->R), rCodeL(cur_tlcs900h->rCode));cur_tlcs900h->cycles = 7;	break;
	}
}

//===== RLC #,r
void regRLCi()
{
	int i;
	uint8 sa = FETCH8 & 0xF;
	if (sa == 0) sa = 16;

	switch(cur_tlcs900h->size)
	{
	case 0:		for (i = 0; i < sa; i++) 
				{
					SETFLAG_C(rCodeB(cur_tlcs900h->rCode) & 0x80);
					rCodeB(cur_tlcs900h->rCode) <<= 1;
					if (FLAG_C) rCodeB(cur_tlcs900h->rCode) |= 1;
				}
				SETFLAG_S(rCodeB(cur_tlcs900h->rCode) & 0x80);
				SETFLAG_Z(rCodeB(cur_tlcs900h->rCode) == 0);
				parityB(rCodeB(cur_tlcs900h->rCode));
				cur_tlcs900h->cycles = 6 + (2*sa);
				break;
		
	case 1:		for (i = 0; i < sa; i++) 
				{	
					SETFLAG_C(rCodeW(cur_tlcs900h->rCode) & 0x8000); 
					rCodeW(cur_tlcs900h->rCode) <<= 1;
					if (FLAG_C) rCodeW(cur_tlcs900h->rCode) |= 1;
				}
				SETFLAG_S(rCodeW(cur_tlcs900h->rCode) & 0x8000);
				SETFLAG_Z(rCodeW(cur_tlcs900h->rCode) == 0);
				parityW(rCodeW(cur_tlcs900h->rCode));
				cur_tlcs900h->cycles = 6 + (2*sa);
				break;

	case 2:		for (i = 0; i < sa; i++) 
				{
					SETFLAG_C(rCodeL(cur_tlcs900h->rCode) & 0x80000000);
					rCodeL(cur_tlcs900h->rCode) <<= 1;
					if (FLAG_C) rCodeL(cur_tlcs900h->rCode) |= 1;
				}
				SETFLAG_S(rCodeL(cur_tlcs900h->rCode) & 0x80000000);
				SETFLAG_Z(rCodeL(cur_tlcs900h->rCode) == 0);
				cur_tlcs900h->cycles = 8 + (2*sa);
				break;
	}

	SETFLAG_H0;
	SETFLAG_N0;
}

//===== RRC #,r
void regRRCi()
{
	int i;
	uint8 sa = FETCH8 & 0xF;
	if (sa == 0) sa = 16;

	switch(cur_tlcs900h->size)
	{
	case 0:		for (i = 0; i < sa; i++) 
				{
					SETFLAG_C(rCodeB(cur_tlcs900h->rCode) & 1);
					rCodeB(cur_tlcs900h->rCode) >>= 1;
					if (FLAG_C) rCodeB(cur_tlcs900h->rCode) |= 0x80;
				}
				SETFLAG_S(rCodeB(cur_tlcs900h->rCode) & 0x80);
				SETFLAG_Z(rCodeB(cur_tlcs900h->rCode) == 0);
				parityB(rCodeB(cur_tlcs900h->rCode));
				cur_tlcs900h->cycles = 6 + (2*sa);
				break;
		
	case 1:		for (i = 0; i < sa; i++) 
				{
					SETFLAG_C(rCodeW(cur_tlcs900h->rCode) & 1);
					rCodeW(cur_tlcs900h->rCode) >>= 1;
					if (FLAG_C) rCodeW(cur_tlcs900h->rCode) |= 0x8000;
				}
				SETFLAG_S(rCodeW(cur_tlcs900h->rCode) & 0x8000);
				SETFLAG_Z(rCodeW(cur_tlcs900h->rCode) == 0);
				parityW(rCodeW(cur_tlcs900h->rCode));
				cur_tlcs900h->cycles = 6 + (2*sa);
				break;

	case 2:		for (i = 0; i < sa; i++) 
				{
					SETFLAG_C(rCodeL(cur_tlcs900h->rCode) & 1);
					rCodeL(cur_tlcs900h->rCode) >>= 1;
					if (FLAG_C) rCodeL(cur_tlcs900h->rCode) |= 0x80000000;
				}
				SETFLAG_S(rCodeL(cur_tlcs900h->rCode) & 0x80000000);
				SETFLAG_Z(rCodeL(cur_tlcs900h->rCode) == 0);
				cur_tlcs900h->cycles = 8 + (2*sa);
				break;
	}

	SETFLAG_H0;
	SETFLAG_N0;
}

//===== RL #,r
void regRLi(void)
{
	int i;
	bool tempC;
	uint8 sa = FETCH8 & 0xF;
	if (sa == 0)
      sa = 16;

	switch(cur_tlcs900h->size)
   {
      case 0:
         {
            uint8 result = 0;
            for (i = 0; i < sa; i++) 
            {
               result = rCodeB(cur_tlcs900h->rCode);
               tempC = FLAG_C;
               SETFLAG_C(result & 0x80);
               result <<= 1;
               if (tempC) result |= 1;
               rCodeB(cur_tlcs900h->rCode) = result;
            }
            SETFLAG_S(result & 0x80);
            SETFLAG_Z(result == 0);
            parityB(result);
            cur_tlcs900h->cycles = 6 + (2*sa);
         }
         break;

      case 1:
         {
            uint16 result = 0;
            for (i = 0; i < sa; i++) 
            {
               result = rCodeW(cur_tlcs900h->rCode);
               tempC = FLAG_C;
               SETFLAG_C(result & 0x8000);
               result <<= 1;
               if (tempC) result |= 1;
               rCodeW(cur_tlcs900h->rCode) = result;
            }
            SETFLAG_S(result & 0x8000);
            SETFLAG_Z(result == 0);
            parityW(result);
            cur_tlcs900h->cycles = 6 + (2*sa);
         }
         break;

      case 2:
         {
            uint32 result = 0;
            for (i = 0; i < sa; i++) 
            {
               result = rCodeL(cur_tlcs900h->rCode);
               tempC = FLAG_C;
               SETFLAG_C(result & 0x80000000);
               result <<= 1;
               if (tempC) result |= 1;
               rCodeL(cur_tlcs900h->rCode) = result;
            }
            SETFLAG_S(result & 0x80000000);
            SETFLAG_Z(result == 0);
            cur_tlcs900h->cycles = 8 + (2*sa);
         }
         break;
   }

	SETFLAG_H0;
	SETFLAG_N0;
}

//===== RR #,r
void regRRi(void)
{
	int i;
	bool tempC;
	uint8 sa = FETCH8 & 0xF;
	if (sa == 0)
      sa = 16;

	switch(cur_tlcs900h->size)
   {
      case 0:
         {
            uint8 result = 0;
            for (i = 0; i < sa; i++) 
            {
               result = rCodeB(cur_tlcs900h->rCode);
               tempC = FLAG_C;
               SETFLAG_C(result & 0x01);
               result >>= 1;
               if (tempC) result |= 0x80;
               rCodeB(cur_tlcs900h->rCode) = result;
            }
            SETFLAG_S(result & 0x80);
            SETFLAG_Z(result == 0);
            cur_tlcs900h->cycles = 6 + (2*sa);
            parityB(result);
         }
         break;
      case 1:
         {
            uint16 result = 0;
            for (i = 0; i < sa; i++) 
            {
               result = rCodeW(cur_tlcs900h->rCode);
               tempC = FLAG_C;
               SETFLAG_C(result & 0x01);
               result >>= 1;
               if (tempC) result |= 0x8000;
               rCodeW(cur_tlcs900h->rCode) = result;
            }
            SETFLAG_S(result & 0x8000);
            SETFLAG_Z(result == 0);
            cur_tlcs900h->cycles = 6 + (2*sa);
            parityW(result);
         }
         break;

      case 2:
         {
            uint32 result = 0;
            for (i = 0; i < sa; i++) 
            {
               result = rCodeL(cur_tlcs900h->rCode);
               tempC = FLAG_C;
               SETFLAG_C(result & 0x01);
               result >>= 1;
               if (tempC) result |= 0x80000000;
               rCodeL(cur_tlcs900h->rCode) = result;
            }
            SETFLAG_S(result & 0x80000000);
            SETFLAG_Z(result == 0);
            cur_tlcs900h->cycles = 8 + (2*sa);
         }
         break;
   }

	SETFLAG_H0;
	SETFLAG_N0;
}

//===== SLA #,r
void regSLAi()
{
	int8 sa = FETCH8 & 0xF;
	if (sa == 0) sa = 16;
	sa--;

	switch(cur_tlcs900h->size)
	{
	case 0:	{	int8 result, data = (int8)rCodeB(cur_tlcs900h->rCode);
				result = (data << sa);
				SETFLAG_C(result & 0x80);
				result <<= 1;
				SETFLAG_S(result & 0x80);
				rCodeB(cur_tlcs900h->rCode) = result;
				SETFLAG_Z(result == 0);
				parityB(result);
				cur_tlcs900h->cycles = 6 + 2 + (2*sa);
				break;	}
	
	case 1:	{	int16 result, data = (int16)rCodeW(cur_tlcs900h->rCode);
				result = (data << sa);
				SETFLAG_C(result & 0x8000);
				result <<= 1;
				SETFLAG_S(result & 0x8000);
				rCodeW(cur_tlcs900h->rCode) = result;
				SETFLAG_Z(result == 0);
				parityW(result);
				cur_tlcs900h->cycles = 6 + 2 + (2*sa);
				break;	}

	case 2:	{	int32 result, data = (int32)rCodeL(cur_tlcs900h->rCode);
				result = (data << sa);
				SETFLAG_C(result & 0x80000000);
				result <<= 1;
				SETFLAG_S(result & 0x80000000);
				rCodeL(cur_tlcs900h->rCode) = result;
				SETFLAG_Z(result == 0);
				cur_tlcs900h->cycles = 8 + 2 + (2*sa);
				break;	}
	}

	SETFLAG_H0;
	SETFLAG_N0;
}

//===== SRA #,r
void regSRAi()
{
	int8 sa = FETCH8 & 0xF;
	if (sa == 0) sa = 16;
	sa--;

	switch(cur_tlcs900h->size)
	{
	case 0:	{	int8 data = (int8)rCodeB(cur_tlcs900h->rCode), result;
				result = (data >> sa);
				SETFLAG_C(result & 1);
				result >>= 1;
				SETFLAG_S(result & 0x80);
				rCodeB(cur_tlcs900h->rCode) = result;
				SETFLAG_Z(result == 0);
				parityB(result);
				cur_tlcs900h->cycles = 6 + 2 + (2*sa);
				break; }

	case 1:	{	int16 data = (int16)rCodeW(cur_tlcs900h->rCode), result;
				result = (data >> sa);
				SETFLAG_C(result & 1);
				result >>= 1;
				SETFLAG_S(result & 0x8000);
				rCodeW(cur_tlcs900h->rCode) = result;
				SETFLAG_Z(result == 0);
				parityW(result);
				cur_tlcs900h->cycles = 6 + 2 + (2*sa);
				break; }

	case 2:	{	int32 data = (int32)rCodeL(cur_tlcs900h->rCode), result;
				result = (data >> sa);
				SETFLAG_C(result & 1);
				result >>= 1;
				SETFLAG_S(result & 0x80000000);
				rCodeL(cur_tlcs900h->rCode) = result;
				SETFLAG_Z(result == 0);
				cur_tlcs900h->cycles = 8 + 2 + (2*sa);
				break; }
	}

	SETFLAG_H0;
	SETFLAG_N0;
}

//===== SLL #,r
void regSLLi()
{
	uint8 sa = FETCH8 & 0xF;
	if (sa == 0) sa = 16;
	sa--;

	switch(cur_tlcs900h->size)
	{
	case 0:	{	uint8 result, data = rCodeB(cur_tlcs900h->rCode);
				result = (data << sa);
				SETFLAG_C(result & 0x80);
				result <<= 1;
				SETFLAG_S(result & 0x80);
				rCodeB(cur_tlcs900h->rCode) = result;
				SETFLAG_Z(result == 0);
				parityB(result);
				cur_tlcs900h->cycles = 6 + 2 + (2*sa);
				break;	}
	
	case 1:	{	uint16 result, data = rCodeW(cur_tlcs900h->rCode);
				result = (data << sa);
				SETFLAG_C(result & 0x8000);
				result <<= 1;
				SETFLAG_S(result & 0x8000);
				rCodeW(cur_tlcs900h->rCode) = result;
				SETFLAG_Z(result == 0);
				parityW(result);
				cur_tlcs900h->cycles = 6 + 2 + (2*sa);
				break;	}

	case 2:	{	uint32 result, data = rCodeL(cur_tlcs900h->rCode);
				result = (data << sa);
				SETFLAG_C(result & 0x80000000);
				result <<= 1;
				SETFLAG_S(result & 0x80000000);
				rCodeL(cur_tlcs900h->rCode) = result;
				SETFLAG_Z(result == 0);
				cur_tlcs900h->cycles = 8 + 2 + (2*sa);
				break;	}
	}

	SETFLAG_H0;
	SETFLAG_N0;
}

//===== SRL #,r
void regSRLi()
{
	uint8 sa = FETCH8 & 0xF;
	if (sa == 0) sa = 16;
	sa--;

	switch(cur_tlcs900h->size)
	{
	case 0:	{	uint8 data = rCodeB(cur_tlcs900h->rCode), result;
				result = (data >> sa);
				SETFLAG_C(result & 1);
				result >>= 1;
				SETFLAG_S(result & 0x80);
				rCodeB(cur_tlcs900h->rCode) = result;
				SETFLAG_Z(result == 0);
				parityB(result);
				cur_tlcs900h->cycles = 6 + 2 + (2*sa);
				break; }

	case 1:	{	uint16 data = rCodeW(cur_tlcs900h->rCode), result;
				result = (data >> sa);
				SETFLAG_C(result & 1);
				result >>= 1;
				SETFLAG_S(result & 0x8000);
				rCodeW(cur_tlcs900h->rCode) = result;
				SETFLAG_Z(result == 0);
				parityW(result);
				cur_tlcs900h->cycles = 6 + 2 + (2*sa);
				break; }

	case 2:	{	uint32 data = rCodeL(cur_tlcs900h->rCode), result;
				result = (data >> sa);
				SETFLAG_C(result & 1);
				result >>= 1;
				SETFLAG_S(result & 0x80000000);
				rCodeL(cur_tlcs900h->rCode) = result;
				SETFLAG_Z(result == 0);
				cur_tlcs900h->cycles = 8 + 2 + (2*sa);
				break; }
	}

	SETFLAG_H0;
	SETFLAG_N0;
}

//===== RLC A,r
void regRLCA()
{
	int i;
	uint8 sa = REGA & 0xF;
	if (sa == 0) sa = 16;

	switch(cur_tlcs900h->size)
	{
	case 0:	for (i = 0; i < sa; i++) 
			{
				SETFLAG_C(rCodeB(cur_tlcs900h->rCode) & 0x80); 
				rCodeB(cur_tlcs900h->rCode) <<= 1;
				if (FLAG_C) rCodeB(cur_tlcs900h->rCode) |= 1;	
			}
			SETFLAG_S(rCodeB(cur_tlcs900h->rCode) & 0x80);
			SETFLAG_Z(rCodeB(cur_tlcs900h->rCode) == 0);
			cur_tlcs900h->cycles = 6 + (2*sa);
			parityB(rCodeB(cur_tlcs900h->rCode));
			break;
		
	case 1:	for (i = 0; i < sa; i++) 
			{
				SETFLAG_C(rCodeW(cur_tlcs900h->rCode) & 0x8000);
				rCodeW(cur_tlcs900h->rCode) <<= 1;
				if (FLAG_C) rCodeW(cur_tlcs900h->rCode) |= 1;
			}
			SETFLAG_S(rCodeW(cur_tlcs900h->rCode) & 0x8000);
			SETFLAG_Z(rCodeW(cur_tlcs900h->rCode) == 0);
			cur_tlcs900h->cycles = 6 + (2*sa);
			parityW(rCodeW(cur_tlcs900h->rCode));
			break;

	case 2:	for (i = 0; i < sa; i++) 
			{
				SETFLAG_C(rCodeL(cur_tlcs900h->rCode) & 0x80000000);
				rCodeL(cur_tlcs900h->rCode) <<= 1;
				if (FLAG_C) rCodeL(cur_tlcs900h->rCode) |= 1;
			}
			SETFLAG_S(rCodeL(cur_tlcs900h->rCode) & 0x80000000);
			SETFLAG_Z(rCodeL(cur_tlcs900h->rCode) == 0);
			cur_tlcs900h->cycles = 8 + (2*sa);
			break;
	}

	SETFLAG_H0;
	SETFLAG_N0;
}

//===== RRC A,r
void regRRCA()
{
	int i;
	uint8 sa = REGA & 0xF;
	if (sa == 0) sa = 16;

	switch(cur_tlcs900h->size)
	{
	case 0:	for (i = 0; i < sa; i++) 
			{
				SETFLAG_C(rCodeB(cur_tlcs900h->rCode) & 1);
				rCodeB(cur_tlcs900h->rCode) >>= 1;
				if (FLAG_C) rCodeB(cur_tlcs900h->rCode) |= 0x80;	
			}
			SETFLAG_S(rCodeB(cur_tlcs900h->rCode) & 0x80);
			SETFLAG_Z(rCodeB(cur_tlcs900h->rCode) == 0);
			parityB(rCodeB(cur_tlcs900h->rCode));
			cur_tlcs900h->cycles = 6 + (2*sa);
			break;
		
	case 1:	for (i = 0; i < sa; i++) 
			{
				SETFLAG_C(rCodeW(cur_tlcs900h->rCode) & 1); 
				rCodeW(cur_tlcs900h->rCode) >>= 1;
				if (FLAG_C) rCodeW(cur_tlcs900h->rCode) |= 0x8000;
			}
			SETFLAG_S(rCodeW(cur_tlcs900h->rCode) & 0x8000);
			SETFLAG_Z(rCodeW(cur_tlcs900h->rCode) == 0);
			parityW(rCodeW(cur_tlcs900h->rCode));
			cur_tlcs900h->cycles = 6 + (2*sa);
			break;

	case 2:	for (i = 0; i < sa; i++) 
			{
				SETFLAG_C(rCodeL(cur_tlcs900h->rCode) & 1);
				rCodeL(cur_tlcs900h->rCode) >>= 1;
				if (FLAG_C) rCodeL(cur_tlcs900h->rCode) |= 0x80000000;
			}
			SETFLAG_S(rCodeL(cur_tlcs900h->rCode) & 0x80000000);
			SETFLAG_Z(rCodeL(cur_tlcs900h->rCode) == 0);
			cur_tlcs900h->cycles = 8 + (2*sa);
			break;
	}

	SETFLAG_H0;
	SETFLAG_N0;
}

//===== RL A,r
void regRLA(void)
{
   int i;
   bool tempC;
   uint8 sa = REGA & 0xF;
   if (sa == 0)
      sa = 16;

   switch(cur_tlcs900h->size)
   {
      case 0:
         {
            uint8 result = 0;
            for (i = 0; i < sa; i++) 
            {
               result = rCodeB(cur_tlcs900h->rCode);
               tempC = FLAG_C;
               SETFLAG_C(result & 0x80);
               result <<= 1;
               if (tempC) result |= 1;
               rCodeB(cur_tlcs900h->rCode) = result;
            }
            SETFLAG_S(result & 0x80);
            SETFLAG_Z(result == 0);
            cur_tlcs900h->cycles = 6 + (2*sa);
            parityB(result);
         }
         break;

      case 1:
         {
            uint16 result = 0;
            for (i = 0; i < sa; i++) 
            {
               result = rCodeW(cur_tlcs900h->rCode);
               tempC = FLAG_C;
               SETFLAG_C(result & 0x8000);
               result <<= 1;
               if (tempC) result |= 1;
               rCodeW(cur_tlcs900h->rCode) = result;
            }
            SETFLAG_S(result & 0x8000);
            SETFLAG_Z(result == 0);
            cur_tlcs900h->cycles = 6 + (2*sa);
            parityW(result);
         }
         break;

      case 2:
         {
            uint32 result = 0;
            for (i = 0; i < sa; i++) 
            {
               result = rCodeL(cur_tlcs900h->rCode);
               tempC = FLAG_C;
               SETFLAG_C(result & 0x80000000);
               result <<= 1;
               if (tempC) result |= 1;
               rCodeL(cur_tlcs900h->rCode) = result;
            }
            SETFLAG_S(result & 0x80000000);
            SETFLAG_Z(result == 0);
            cur_tlcs900h->cycles = 8 + (2*sa);
         }
         break;
   }

   SETFLAG_H0;
   SETFLAG_N0;
}

//===== RR A,r
void regRRA(void)
{
   int i;
   bool tempC;
   uint8 sa = REGA & 0xF;
   if (sa == 0)
      sa = 16;

   switch(cur_tlcs900h->size)
   {
      case 0:
         {	uint8 result = 0;
            for (i = 0; i < sa; i++) 
            {
               result = rCodeB(cur_tlcs900h->rCode);
               tempC = FLAG_C;
               SETFLAG_C(result & 0x01);
               result >>= 1;
               if (tempC) result |= 0x80;
               rCodeB(cur_tlcs900h->rCode) = result;
            }
            SETFLAG_S(result & 0x80);
            SETFLAG_Z(result == 0);
            cur_tlcs900h->cycles = 6 + (2*sa);
            parityB(result);
         }
         break;
      case 1:
         {
            uint16 result = 0;
            for (i = 0; i < sa; i++) 
            {
               result = rCodeW(cur_tlcs900h->rCode);
               tempC = FLAG_C;
               SETFLAG_C(result & 0x01);
               result >>= 1;
               if (tempC) result |= 0x8000;
               rCodeW(cur_tlcs900h->rCode) = result;
            }
            SETFLAG_S(result & 0x8000);
            SETFLAG_Z(result == 0);
            cur_tlcs900h->cycles = 6 + (2*sa);
            parityW(result);
         }
         break;
      case 2:
         {	uint32 result = 0;
            for (i = 0; i < sa; i++) 
            {
               result = rCodeL(cur_tlcs900h->rCode);
               tempC = FLAG_C;
               SETFLAG_C(result & 0x01);
               result >>= 1;
               if (tempC) result |= 0x80000000;
               rCodeL(cur_tlcs900h->rCode) = result;
            }
            SETFLAG_S(result & 0x80000000);
            SETFLAG_Z(result == 0);
            cur_tlcs900h->cycles = 8 + (2*sa);
         }
         break;
   }

   SETFLAG_H0;
   SETFLAG_N0;
}

//===== SLA A,r
void regSLAA()
{
	int8 sa = REGA & 0xF;
	if (sa == 0) sa = 16;
	sa--;

	switch(cur_tlcs900h->size)
	{
	case 0:	{	int8 result, data = (int8)rCodeB(cur_tlcs900h->rCode);
				result = (data << sa);
				SETFLAG_C(result & 0x80);
				result <<= 1;
				SETFLAG_S(result & 0x80);
				rCodeB(cur_tlcs900h->rCode) = result;
				SETFLAG_Z(result == 0);
				parityB(result);
				cur_tlcs900h->cycles = 6 + 2 + (2*sa);
				break;	}
	
	case 1:	{	int16 result, data = (int16)rCodeW(cur_tlcs900h->rCode);
				result = (data << sa);
				SETFLAG_C(result & 0x8000);
				result <<= 1;
				SETFLAG_S(result & 0x8000);
				rCodeW(cur_tlcs900h->rCode) = result;
				SETFLAG_Z(result == 0);
				parityW(result);
				cur_tlcs900h->cycles = 6 + 2 + (2*sa);
				break;	}

	case 2:	{	int32 result, data = (int32)rCodeL(cur_tlcs900h->rCode);
				result = (data << sa);
				SETFLAG_C(result & 0x80000000);
				result <<= 1;
				SETFLAG_S(result & 0x80000000);
				rCodeL(cur_tlcs900h->rCode) = result;
				SETFLAG_Z(result == 0);
				cur_tlcs900h->cycles = 8 + 2 + (2*sa);
				break;	}
	}

	SETFLAG_H0;
	SETFLAG_N0;
}

//===== SRA A,r
void regSRAA()
{
	int8 sa = REGA & 0xF;
	if (sa == 0) sa = 16;
	sa--;

	switch(cur_tlcs900h->size)
	{
	case 0:	{	int8 data = (int8)rCodeB(cur_tlcs900h->rCode), result;
				result = (data >> sa);
				SETFLAG_C(result & 1);
				result >>= 1;
				SETFLAG_S(result & 0x80);
				rCodeB(cur_tlcs900h->rCode) = result;
				SETFLAG_Z(result == 0);
				parityB(result);
				cur_tlcs900h->cycles = 6 + 2 + (2*sa);
				break; }

	case 1:	{	int16 data = (int16)rCodeW(cur_tlcs900h->rCode), result;
				result = (data >> sa);
				SETFLAG_C(result & 1);
				result >>= 1;
				SETFLAG_S(result & 0x8000);
				rCodeW(cur_tlcs900h->rCode) = result;
				SETFLAG_Z(result == 0);
				parityW(result);
				cur_tlcs900h->cycles = 6 + 2 + (2*sa);
				break; }

	case 2:	{	int32 data = (int32)rCodeL(cur_tlcs900h->rCode), result;
				result = (data >> sa);
				SETFLAG_C(result & 1);
				result >>= 1;
				SETFLAG_S(result & 0x80000000);
				rCodeL(cur_tlcs900h->rCode) = result;
				SETFLAG_Z(result == 0);
				cur_tlcs900h->cycles = 8 + 2 + (2*sa);
				break; }
	}

	SETFLAG_H0;
	SETFLAG_N0;
}

//===== SLL A,r
void regSLLA()
{
	uint8 sa = REGA & 0xF;
	if (sa == 0) sa = 16;
	sa--;

	switch(cur_tlcs900h->size)
	{
	case 0:	{	uint8 result, data = rCodeB(cur_tlcs900h->rCode);
				result = (data << sa);
				SETFLAG_C(result & 0x80);
				result <<= 1;
				SETFLAG_S(result & 0x80);
				rCodeB(cur_tlcs900h->rCode) = result;
				SETFLAG_Z(result == 0);
				parityB(result);
				cur_tlcs900h->cycles = 6 + 2 + (2*sa);
				break;	}
	
	case 1:	{	uint16 result, data = rCodeW(cur_tlcs900h->rCode);
				result = (data << sa);
				SETFLAG_C(result & 0x8000);
				result <<= 1;
				SETFLAG_S(result & 0x8000);
				rCodeW(cur_tlcs900h->rCode) = result;
				SETFLAG_Z(result == 0);
				parityW(result);
				cur_tlcs900h->cycles = 6 + 2 + (2*sa);
				break;	}

	case 2:	{	uint32 result, data = rCodeL(cur_tlcs900h->rCode);
				result = (data << sa);
				SETFLAG_C(result & 0x80000000);
				result <<= 1;
				SETFLAG_S(result & 0x80000000);
				rCodeL(cur_tlcs900h->rCode) = result;
				SETFLAG_Z(result == 0);
				cur_tlcs900h->cycles = 8 + 2 + (2*sa);
				break;	}
	}

	SETFLAG_H0;
	SETFLAG_N0;
}

//===== SRL A,r
void regSRLA()
{
	uint8 sa = REGA & 0xF;
	if (sa == 0) sa = 16;
	sa--;

	switch(cur_tlcs900h->size)
	{
	case 0:	{	uint8 data = rCodeB(cur_tlcs900h->rCode), result;
				result = (data >> sa);
				SETFLAG_C(result & 1);
				result >>= 1;
				SETFLAG_S(result & 0x80);
				rCodeB(cur_tlcs900h->rCode) = result;
				SETFLAG_Z(result == 0);
				parityB(result);
				cur_tlcs900h->cycles = 6 + 2 + (2*sa);
				break; }

	case 1:	{	uint16 data = rCodeW(cur_tlcs900h->rCode), result;
				result = (data >> sa);
				SETFLAG_C(result & 1);
				result >>= 1;
				SETFLAG_S(result & 0x8000);
				rCodeW(cur_tlcs900h->rCode) = result;
				SETFLAG_Z(result == 0);
				parityW(result);
				cur_tlcs900h->cycles = 6 + 2 + (2*sa);
				break; }

	case 2:	{	uint32 data = rCodeL(cur_tlcs900h->rCode), result;
				result = (data >> sa);
				SETFLAG_C(result & 1);
				result >>= 1;
				SETFLAG_S(result & 0x80000000);
				rCodeL(cur_tlcs900h->rCode) = result;
				SETFLAG_Z(result == 0);
				cur_tlcs900h->cycles = 8 + 2 + (2*sa);
				break; }
	}

	SETFLAG_H0;
	SETFLAG_N0;
}

//=============================================================================
