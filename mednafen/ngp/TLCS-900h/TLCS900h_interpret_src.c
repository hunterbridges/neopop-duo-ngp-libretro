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

	TLCS900h_interpret_src.c

//=========================================================================
//---------------------------------------------------------------------------

  History of changes:
  ===================

20 JUL 2002 - neopop_uk
=======================================
- Cleaned and tidied up for the source release

24 JUL 2002 - neopop_uk
=======================================
- Fixed S flag in "RRC (mem)"

25 JUL 2002 - neopop_uk
=======================================
- Removed unneeded Long mode from EX.

28 JUL 2002 - neopop_uk
=======================================
- Improved the LDIR/LDDR/CPIR/CPDR instructions so that they
	finish with the correct register settings, even if there is
	a memory error.
- Converted DIV/DIVS to use the generic function

16 AUG 2002 - neopop_uk
=======================================
- Replaced 'second & 7' with 'R', clearer, faster - and for some reason
	more accurate... oh well!
- Fixed V flag emulation of INC/DEC, fixes "Cotton" menus

21 AUG 2002 - neopop_uk
=======================================
- Fixed "RR (mem)" - It was actually the [REG] version that hadn't been
	changed to use memory accesses!

30 AUG 2002 - neopop_uk
=======================================
- Fixed "DIV RR,(mem)" in long mode, wrong operand size.

04 SEP 2002 - neopop_uk
=======================================
- Fixed GCC compatibility.

//---------------------------------------------------------------------------
*/

#include "TLCS900h_interpret.h"
#include "TLCS900h_registers.h"
#include "../mem.h"

//=========================================================================

//===== PUSH (cur_tlcs900h->mem)
void srcPUSH(void)
{
   switch(cur_tlcs900h->size)
   {
      case 0:
         push8(loadB(cur_tlcs900h->mem));
         break;
      case 1:
         push16(loadW(cur_tlcs900h->mem));
         break;
   }
   cur_tlcs900h->cycles = 7;
}

//===== RLD A,(cur_tlcs900h->mem)
void srcRLD(void)
{
	uint8 al = REGA & 0xF, m, mh, ml;

	m = loadB(cur_tlcs900h->mem);
	mh = (m & 0xF0) >> 4;
	ml = (m & 0x0F) << 4;
	
	REGA = (REGA & 0xF0) | mh;
	storeB(cur_tlcs900h->mem, ml | al);

	SETFLAG_S(REGA & 0x80);
	SETFLAG_Z(REGA == 0);
	SETFLAG_H0
	SETFLAG_N0
	parityB(REGA);

	cur_tlcs900h->cycles = 12;
}

//===== RRD A,(cur_tlcs900h->mem)
void srcRRD()
{
	uint8 al = (REGA & 0xF) << 4, m, mh, ml;

	m = loadB(cur_tlcs900h->mem);
	mh = (m & 0xF0) >> 4;
	ml = m & 0x0F;
	
	REGA = (REGA & 0xF0) | ml;
	storeB(cur_tlcs900h->mem, al | mh);

	SETFLAG_S(REGA & 0x80);
	SETFLAG_Z(REGA == 0);
	SETFLAG_H0
	SETFLAG_N0
	parityB(REGA);

	cur_tlcs900h->cycles = 12;
}

//===== LDI
void srcLDI()
{
   uint8 dst = 2/*XDE*/, src = 3/*XHL*/;
   if ((cur_tlcs900h->first & 0xF) == 5)
   {
      dst = 4/*XIX*/;
      src = 5/*XIY*/;
   }

   switch(cur_tlcs900h->size)
   {
      case 0:
         storeB(regL(dst), loadB(regL(src)));
         regL(dst) += 1;
         regL(src) += 1;
         break;

      case 1:
         storeW(regL(dst), loadW(regL(src)));
         regL(dst) += 2;
         regL(src) += 2;
         break;
   }

   REGBC --;
   SETFLAG_V(REGBC);

   SETFLAG_H0;
   SETFLAG_N0;
   cur_tlcs900h->cycles = 10;
}

//===== LDIR
void srcLDIR()
{
   uint8 dst = 2/*XDE*/, src = 3/*XHL*/;
   if ((cur_tlcs900h->first & 0xF) == 5)
   {
      dst = 4/*XIX*/;
      src = 5/*XIY*/;
   }

   cur_tlcs900h->cycles = 10;

   do
   {
      switch(cur_tlcs900h->size)
      {
         case 0:
            if (debug_abort_memory == false)
               storeB(regL(dst), loadB(regL(src)));
            regL(dst) += 1;
            regL(src) += 1;
            break;
         case 1:
            if (debug_abort_memory == false)
               storeW(regL(dst), loadW(regL(src)));
            regL(dst) += 2;
            regL(src) += 2;
            break;
      }

      REGBC --;
      SETFLAG_V(REGBC);

      cur_tlcs900h->cycles += 14;
   }
   while (FLAG_V);

   SETFLAG_H0;
   SETFLAG_N0;
}

//===== LDD
void srcLDD()
{
	uint8 dst = 2/*XDE*/, src = 3/*XHL*/;
	if ((cur_tlcs900h->first & 0xF) == 5) { dst = 4/*XIX*/; src = 5/*XIY*/; }

	switch(cur_tlcs900h->size)
	{
	case 0:
		storeB(regL(dst), loadB(regL(src)));
		regL(dst) -= 1;
		regL(src) -= 1;
		break;

	case 1:
		storeW(regL(dst), loadW(regL(src)));
		regL(dst) -= 2;
		regL(src) -= 2;
		break;
	}

	REGBC --;
	SETFLAG_V(REGBC);

	SETFLAG_H0;
	SETFLAG_N0;
	cur_tlcs900h->cycles = 10;
}

//===== LDDR
void srcLDDR()
{
	uint8 dst = 2/*XDE*/, src = 3/*XHL*/;
	if ((cur_tlcs900h->first & 0xF) == 5)	{ dst = 4/*XIX*/; src = 5/*XIY*/; }

	cur_tlcs900h->cycles = 10;

	do
	{
		switch(cur_tlcs900h->size)
		{
		case 0:
			if (debug_abort_memory == false)
				storeB(regL(dst), loadB(regL(src)));
			regL(dst) -= 1;
			regL(src) -= 1;
			break;

		case 1:
			if (debug_abort_memory == false)
				storeW(regL(dst), loadW(regL(src)));
			regL(dst) -= 2;
			regL(src) -= 2;
			break;
		}

		REGBC --;
		SETFLAG_V(REGBC);

		cur_tlcs900h->cycles += 14;
	}
	while (FLAG_V);

	SETFLAG_H0;
	SETFLAG_N0;
}

//===== CPI
void srcCPI()
{
	uint8 R = cur_tlcs900h->first & 7;

	switch(cur_tlcs900h->size)
	{
	case 0: generic_SUB_B(REGA, loadB(regL(cur_tlcs900h->R)));
			regL(cur_tlcs900h->R) ++; break;

	case 1:	generic_SUB_W(REGWA, loadW(regL(cur_tlcs900h->R)));
			regL(cur_tlcs900h->R) += 2; break;
	}

	REGBC --;
	SETFLAG_V(REGBC);

	cur_tlcs900h->cycles = 8;
}

//===== CPIR
void srcCPIR()
{
	uint8 R = cur_tlcs900h->first & 7;

	cur_tlcs900h->cycles = 10;

	do
	{
		switch(cur_tlcs900h->size)
		{
		case 0:	if (debug_abort_memory == false)
					generic_SUB_B(REGA, loadB(regL(cur_tlcs900h->R)));
				regL(cur_tlcs900h->R) ++; break;

		case 1:	if (debug_abort_memory == false)
					generic_SUB_W(REGWA, loadW(regL(cur_tlcs900h->R)));
				regL(cur_tlcs900h->R) += 2; break;
		}

		REGBC --;
		SETFLAG_V(REGBC);

		cur_tlcs900h->cycles += 14;
	}
	while (FLAG_V && (FLAG_Z == false));
}

//===== CPD
void srcCPD()
{
	uint8 R = cur_tlcs900h->first & 7;

	switch(cur_tlcs900h->size)
	{
	case 0:	generic_SUB_B(REGA, loadB(regL(cur_tlcs900h->R)));
			regL(cur_tlcs900h->R) --;	break;

	case 1:	generic_SUB_W(REGWA, loadW(regL(cur_tlcs900h->R)));
			regL(cur_tlcs900h->R) -= 2; break;
	}

	REGBC --;
	SETFLAG_V(REGBC);

	cur_tlcs900h->cycles = 8;
}

//===== CPDR
void srcCPDR()
{
	uint8 R = cur_tlcs900h->first & 7;

	cur_tlcs900h->cycles = 10;

	do
	{
		switch(cur_tlcs900h->size)
		{
		case 0:	if (debug_abort_memory == false)
					generic_SUB_B(REGA, loadB(regL(cur_tlcs900h->R)));
				regL(cur_tlcs900h->R) -= 1; break;

		case 1: if (debug_abort_memory == false)
					generic_SUB_W(REGWA, loadW(regL(cur_tlcs900h->R)));
				regL(cur_tlcs900h->R) -= 2; break;
		}

		REGBC --;
		SETFLAG_V(REGBC);

		cur_tlcs900h->cycles += 14;
	}
	while (FLAG_V && (FLAG_Z == false));
}

//===== LD (nn),(cur_tlcs900h->mem)
void srcLD16m()
{
	switch(cur_tlcs900h->size)
	{
	case 0:	storeB(fetch16(), loadB(cur_tlcs900h->mem)); break;
	case 1: storeW(fetch16(), loadW(cur_tlcs900h->mem)); break;
	}

	cur_tlcs900h->cycles = 8;
}

//===== LD R,(cur_tlcs900h->mem)
void srcLD()
{
	switch(cur_tlcs900h->size)
	{
	case 0: regB(cur_tlcs900h->R) = loadB(cur_tlcs900h->mem); cur_tlcs900h->cycles = 4; break;
	case 1: regW(cur_tlcs900h->R) = loadW(cur_tlcs900h->mem); cur_tlcs900h->cycles = 4; break;
	case 2: regL(cur_tlcs900h->R) = loadL(cur_tlcs900h->mem); cur_tlcs900h->cycles = 6; break;
	}
}

//===== EX (cur_tlcs900h->mem),R
void srcEX()
{
	switch(cur_tlcs900h->size)
	{
	case 0:	{	uint8 temp = regB(cur_tlcs900h->R); 
				regB(cur_tlcs900h->R) = loadB(cur_tlcs900h->mem); 
				storeB(cur_tlcs900h->mem, temp); break;		}

	case 1:	{	uint16 temp = regW(cur_tlcs900h->R); 
				regW(cur_tlcs900h->R) = loadW(cur_tlcs900h->mem); 
				storeW(cur_tlcs900h->mem, temp); break;		}
	}

	cur_tlcs900h->cycles = 6;
}

//===== ADD (cur_tlcs900h->mem),#
void srcADDi()
{
	switch(cur_tlcs900h->size)
	{
	case 0:	storeB(cur_tlcs900h->mem, generic_ADD_B(loadB(cur_tlcs900h->mem), FETCH8)); cur_tlcs900h->cycles = 7;break;
	case 1:	storeW(cur_tlcs900h->mem, generic_ADD_W(loadW(cur_tlcs900h->mem), fetch16())); cur_tlcs900h->cycles = 8;break;
	}
}

//===== ADC (cur_tlcs900h->mem),#
void srcADCi()
{
	switch(cur_tlcs900h->size)
	{
	case 0:	storeB(cur_tlcs900h->mem, generic_ADC_B(loadB(cur_tlcs900h->mem), FETCH8)); cur_tlcs900h->cycles = 7;break;
	case 1:	storeW(cur_tlcs900h->mem, generic_ADC_W(loadW(cur_tlcs900h->mem), fetch16())); cur_tlcs900h->cycles = 8;break;
	}
}

//===== SUB (cur_tlcs900h->mem),#
void srcSUBi()
{
	switch(cur_tlcs900h->size)
	{
	case 0:	storeB(cur_tlcs900h->mem, generic_SUB_B(loadB(cur_tlcs900h->mem), FETCH8)); cur_tlcs900h->cycles = 7;break;
	case 1:	storeW(cur_tlcs900h->mem, generic_SUB_W(loadW(cur_tlcs900h->mem), fetch16())); cur_tlcs900h->cycles = 8;break;
	}
}

//===== SBC (cur_tlcs900h->mem),#
void srcSBCi()
{
	switch(cur_tlcs900h->size)
	{
	case 0:	storeB(cur_tlcs900h->mem, generic_SBC_B(loadB(cur_tlcs900h->mem), FETCH8)); cur_tlcs900h->cycles = 7;break;
	case 1:	storeW(cur_tlcs900h->mem, generic_SBC_W(loadW(cur_tlcs900h->mem), fetch16())); cur_tlcs900h->cycles = 8;break;
	}
}

//===== AND (cur_tlcs900h->mem),#
void srcANDi()
{
	switch(cur_tlcs900h->size)
	{
	case 0: {	uint8 result = loadB(cur_tlcs900h->mem) & FETCH8;
				storeB(cur_tlcs900h->mem, result);
				SETFLAG_S(result & 0x80);
				SETFLAG_Z(result == 0);
				parityB(result);
				cur_tlcs900h->cycles = 7;
				break; }

	case 1: {	uint16 result = loadW(cur_tlcs900h->mem) & fetch16();
				storeW(cur_tlcs900h->mem, result);
				SETFLAG_S(result & 0x8000);
				SETFLAG_Z(result == 0);
				parityW(result);
				cur_tlcs900h->cycles = 8;
				break; }
	}

	SETFLAG_H1;
	SETFLAG_N0;
	SETFLAG_C0;
}

//===== OR (cur_tlcs900h->mem),#
void srcORi()
{
	switch(cur_tlcs900h->size)
	{
	case 0: {	uint8 result = loadB(cur_tlcs900h->mem) | FETCH8;
				storeB(cur_tlcs900h->mem, result);
				SETFLAG_S(result & 0x80);
				SETFLAG_Z(result == 0);
				parityB(result);
				cur_tlcs900h->cycles = 7;
				break; }

	case 1: {	uint16 result = loadW(cur_tlcs900h->mem) | fetch16();
				storeW(cur_tlcs900h->mem, result);
				SETFLAG_S(result & 0x8000);
				SETFLAG_Z(result == 0);
				parityW(result);
				cur_tlcs900h->cycles = 8;
				break; }
	}

	SETFLAG_H0;
	SETFLAG_N0;
	SETFLAG_C0;
}

//===== XOR (cur_tlcs900h->mem),#
void srcXORi()
{
	switch(cur_tlcs900h->size)
	{
	case 0: {	uint8 result = loadB(cur_tlcs900h->mem) ^ FETCH8;
				storeB(cur_tlcs900h->mem, result);
				SETFLAG_S(result & 0x80);
				SETFLAG_Z(result == 0);
				parityB(result);
				cur_tlcs900h->cycles = 7;
				break; }

	case 1: {	uint16 result = loadW(cur_tlcs900h->mem) ^ fetch16();
				storeW(cur_tlcs900h->mem, result);
				SETFLAG_S(result & 0x8000);
				SETFLAG_Z(result == 0);
				parityW(result);
				cur_tlcs900h->cycles = 8;
				break; }
	}

	SETFLAG_H0;
	SETFLAG_N0;
	SETFLAG_C0;
}

//===== CP (cur_tlcs900h->mem),#
void srcCPi()
{
	switch(cur_tlcs900h->size)
	{
	case 0:	generic_SUB_B(loadB(cur_tlcs900h->mem), FETCH8);	break;
	case 1:	generic_SUB_W(loadW(cur_tlcs900h->mem), fetch16());	break;
	}
	
	cur_tlcs900h->cycles = 6;
}

//===== MUL RR,(cur_tlcs900h->mem)
void srcMUL()
{
	uint8 target = get_RR_Target();
	if (target == 0x80)
	{
		instruction_error("src: MUL bad \'RR\' dst code");
		return;
	}

	switch(cur_tlcs900h->size)
	{
	case 0: rCodeW(target) = (rCodeW(target) & 0xFF) * loadB(cur_tlcs900h->mem);
		cur_tlcs900h->cycles = 18; break;
	case 1: rCodeL(target) = (rCodeL(target) & 0xFFFF) * loadW(cur_tlcs900h->mem);
		cur_tlcs900h->cycles = 26; break;
	}
}

//===== MULS RR,(cur_tlcs900h->mem)
void srcMULS()
{
	uint8 target = get_RR_Target();
	if (target == 0x80)
	{
		instruction_error("src: MUL bad \'RR\' dst code");
		return;
	}

	switch(cur_tlcs900h->size)
	{
	case 0: rCodeW(target) = (int8)(rCodeW(target) & 0xFF) * (int8)loadB(cur_tlcs900h->mem);
		cur_tlcs900h->cycles = 18; break;
	case 1: rCodeL(target) = (int16)(rCodeL(target) & 0xFFFF) * (int16)loadW(cur_tlcs900h->mem);
		cur_tlcs900h->cycles = 26; break;
	}
}

//===== DIV RR,(cur_tlcs900h->mem)
void srcDIV()
{
	uint8 target = get_RR_Target();
	if (target == 0x80)
	{
		instruction_error("src: DIV bad \'RR\' dst code");
		return;
	}

	switch(cur_tlcs900h->size)
	{
	case 0: {	rCodeW(target) = generic_DIV_B(rCodeW(target), loadB(cur_tlcs900h->mem));
				cur_tlcs900h->cycles = 22;
				break;	}
				
	case 1: {	rCodeL(target) = generic_DIV_W(rCodeL(target), loadW(cur_tlcs900h->mem));
				cur_tlcs900h->cycles = 30;
				break;	}
	}
}

//===== DIVS RR,(cur_tlcs900h->mem)
void srcDIVS()
{
	uint8 target = get_RR_Target();
	if (target == 0x80)
	{
		instruction_error("src: DIVS bad \'RR\' dst code");
		return;
	}

	switch(cur_tlcs900h->size)
	{
	case 0: {	rCodeW(target) = generic_DIVS_B(rCodeW(target), loadB(cur_tlcs900h->mem));
				cur_tlcs900h->cycles = 24;
				break;	}

	case 1: {	rCodeL(target) = generic_DIVS_W(rCodeL(target), loadW(cur_tlcs900h->mem));
				cur_tlcs900h->cycles = 32;
				break;	}
	}
}

//===== INC #3,(cur_tlcs900h->mem)
void srcINC()
{
	uint8 val = cur_tlcs900h->R;
	if (val == 0)
		val = 8;

	switch(cur_tlcs900h->size)
	{
	case 0: {	uint8 dst = loadB(cur_tlcs900h->mem);
				uint32 resultC = dst + val;
				uint8 half = (dst & 0xF) + val;
				uint8 result = (uint8)(resultC & 0xFF);
				SETFLAG_Z(result == 0);
				SETFLAG_H(half > 0xF);
				SETFLAG_S(result & 0x80);
				SETFLAG_N0;

				if (((int8)dst >= 0) && ((int8)result < 0))
				{SETFLAG_V1} else {SETFLAG_V0}

				storeB(cur_tlcs900h->mem, result);
				break; }

	case 1: {	uint16 dst = loadW(cur_tlcs900h->mem);
				uint32 resultC = dst + val;
				uint8 half = (dst & 0xF) + val;
				uint16 result = (uint16)(resultC & 0xFFFF);
				SETFLAG_Z(result == 0);
				SETFLAG_H(half > 0xF);
				SETFLAG_S(result & 0x8000);
				SETFLAG_N0;

				if (((int16)dst >= 0) && ((int16)result < 0))
				{SETFLAG_V1} else {SETFLAG_V0}

				storeW(cur_tlcs900h->mem, result);
				break; }
	}

	cur_tlcs900h->cycles = 6;
}

//===== DEC #3,(cur_tlcs900h->mem)
void srcDEC()
{
	uint8 val = cur_tlcs900h->R;
	if (val == 0)
		val = 8;

	switch(cur_tlcs900h->size)
	{
	case 0: {	uint8 dst = loadB(cur_tlcs900h->mem);
				uint32 resultC = dst - val;
				uint8 half = (dst & 0xF) - val;
				uint8 result = (uint8)(resultC & 0xFF);
				SETFLAG_Z(result == 0);
				SETFLAG_H(half > 0xF);
				SETFLAG_S(result & 0x80);
				SETFLAG_N1;

				if (((int8)dst < 0) && ((int8)result >= 0))
				{SETFLAG_V1} else {SETFLAG_V0}

				storeB(cur_tlcs900h->mem, result);
				break; }

	case 1: {	uint16 dst = loadW(cur_tlcs900h->mem);
				uint32 resultC = dst - val;
				uint8 half = (dst & 0xF) - val;
				uint16 result = (uint16)(resultC & 0xFFFF);
				SETFLAG_Z(result == 0);
				SETFLAG_H(half > 0xF);
				SETFLAG_S(result & 0x8000);
				SETFLAG_N1;

				if (((int16)dst < 0) && ((int16)result >= 0))
				{SETFLAG_V1} else {SETFLAG_V0}

				storeW(cur_tlcs900h->mem, result);
				break; }
	}

	cur_tlcs900h->cycles = 6;
}

//===== RLC (cur_tlcs900h->mem)
void srcRLC()
{
	switch(cur_tlcs900h->size)
	{
	case 0:	{	uint8 result = loadB(cur_tlcs900h->mem);
				SETFLAG_C(result & 0x80);
				result <<= 1;
				if (FLAG_C) result |= 1;
				storeB(cur_tlcs900h->mem, result);
				SETFLAG_S(result & 0x80);
				SETFLAG_Z(result == 0);
				parityB(result);
				break; }
		
	case 1:	{	uint16 result = loadW(cur_tlcs900h->mem);
				SETFLAG_C(result & 0x8000);
				result <<= 1;
				if (FLAG_C) result |= 1;
				storeW(cur_tlcs900h->mem, result);
				SETFLAG_S(result & 0x8000);
				SETFLAG_Z(result == 0);
				parityW(result);
				break; }
	}

	SETFLAG_H0;
	SETFLAG_N0;

	cur_tlcs900h->cycles = 8;
}

//===== RRC (cur_tlcs900h->mem)
void srcRRC()
{
	switch(cur_tlcs900h->size)
	{
	case 0:	{	uint8 data = loadB(cur_tlcs900h->mem), result;
				SETFLAG_C(data & 1);
				result = data >> 1;
				if (FLAG_C) result |= 0x80;
				storeB(cur_tlcs900h->mem, result);
				SETFLAG_S(result & 0x80);
				SETFLAG_Z(result == 0);
				parityB(result);
				break; }
		
	case 1:	{	uint16 data = loadW(cur_tlcs900h->mem), result;
				SETFLAG_C(data & 1);
				result = data >> 1;
				if (FLAG_C) result |= 0x8000;
				storeW(cur_tlcs900h->mem, result);
				SETFLAG_S(result & 0x8000);
				SETFLAG_Z(result == 0);
				parityW(result);
				break; }
	}

	SETFLAG_H0;
	SETFLAG_N0;

	cur_tlcs900h->cycles = 8;
}

//===== RL (cur_tlcs900h->mem)
void srcRL()
{
	bool tempC;

	switch(cur_tlcs900h->size)
	{
	case 0:	{	uint8 result = loadB(cur_tlcs900h->mem);
				tempC = FLAG_C;
				SETFLAG_C(result & 0x80);
				result <<= 1;
				if (tempC) result |= 1;
				storeB(cur_tlcs900h->mem, result);
				SETFLAG_S(result & 0x80);
				SETFLAG_Z(result == 0);
				parityB(result);
				break; }
		
	case 1:	{	uint16 result = loadW(cur_tlcs900h->mem);
				tempC = FLAG_C;
				SETFLAG_C(result & 0x8000);
				result <<= 1;
				if (tempC) result |= 1;
				storeW(cur_tlcs900h->mem, result);
				SETFLAG_S(result & 0x8000);
				SETFLAG_Z(result == 0);
				parityW(result);
				break; }
	}

	cur_tlcs900h->cycles = 8;
}

//===== RR (cur_tlcs900h->mem)
void srcRR()
{
	bool tempC;

	switch(cur_tlcs900h->size)
	{
	case 0:	{	uint8 result = loadB(cur_tlcs900h->mem);
				tempC = FLAG_C;
				SETFLAG_C(result & 1);
				result >>= 1;
				if (tempC) result |= 0x80;
				storeB(cur_tlcs900h->mem, result);
				SETFLAG_S(result & 0x80);
				SETFLAG_Z(result == 0);
				parityB(result);
				break; }
		
	case 1:	{	uint16 result = loadW(cur_tlcs900h->mem);
				tempC = FLAG_C;
				SETFLAG_C(result & 1);
				result >>= 1;
				if (tempC) result |= 0x8000;
				storeW(cur_tlcs900h->mem, result);
				SETFLAG_S(result & 0x8000);
				SETFLAG_Z(result == 0);
				parityW(result);
				break; }
	}

	cur_tlcs900h->cycles = 8;
}

//===== SLA (cur_tlcs900h->mem)
void srcSLA()
{
	switch(cur_tlcs900h->size)
	{
	case 0:	{	uint8 result, data = loadB(cur_tlcs900h->mem);
				SETFLAG_C(data & 0x80);
				result = ((int8)data << 1);
				SETFLAG_S(result & 0x80);
				storeB(cur_tlcs900h->mem, result);
				SETFLAG_Z(result == 0);
				parityB(result);
				break;	}

	case 1:	{	uint16 result, data = loadW(cur_tlcs900h->mem);
				SETFLAG_C(data & 0x8000);
				result = ((int16)data << 1);
				SETFLAG_S(result & 0x8000);
				storeW(cur_tlcs900h->mem, result);
				SETFLAG_Z(result == 0);
				parityW(result);
				break;	}
	}

	SETFLAG_H0;
	SETFLAG_N0;

	cur_tlcs900h->cycles = 8;
}

//===== SRA (cur_tlcs900h->mem)
void srcSRA()
{
	switch(cur_tlcs900h->size)
	{
	case 0:	{	uint8 result, data = loadB(cur_tlcs900h->mem);
				SETFLAG_C(data & 0x1);
				result = ((int8)data >> 1);
				SETFLAG_S(result & 0x80);
				storeB(cur_tlcs900h->mem, result);
				SETFLAG_Z(result == 0);
				parityB(result);
				break;	}
	
	case 1:	{	uint16 result, data = loadW(cur_tlcs900h->mem);
				SETFLAG_C(data & 0x1);
				result = ((int16)data >> 1);
				SETFLAG_S(result & 0x8000);
				storeW(cur_tlcs900h->mem, result);
				SETFLAG_Z(result == 0);
				parityW(result);
				break;	}
	}

	SETFLAG_H0;
	SETFLAG_N0;

	cur_tlcs900h->cycles = 8;
}

//===== SLL (cur_tlcs900h->mem)
void srcSLL()
{
	switch(cur_tlcs900h->size)
	{
	case 0:	{	uint8 result, data = loadB(cur_tlcs900h->mem);
				SETFLAG_C(data & 0x80);
				result = (data << 1);
				SETFLAG_S(result & 0x80);
				storeB(cur_tlcs900h->mem, result);
				SETFLAG_Z(result == 0);
				parityB(result);
				break;	}
	
	case 1:	{	uint16 result, data = loadW(cur_tlcs900h->mem);
				SETFLAG_C(data & 0x8000);
				result = (data << 1);
				SETFLAG_S(result & 0x8000);
				storeW(cur_tlcs900h->mem, result);
				SETFLAG_Z(result == 0);
				parityW(result);
				break;	}
	}

	SETFLAG_H0;
	SETFLAG_N0;

	cur_tlcs900h->cycles = 8;
}

//===== SRL (cur_tlcs900h->mem)
void srcSRL()
{
	switch(cur_tlcs900h->size)
	{
	case 0:	{	uint8 result, data = loadB(cur_tlcs900h->mem);
				SETFLAG_C(data & 0x01);
				result = (data >> 1);
				SETFLAG_S(result & 0x80);
				storeB(cur_tlcs900h->mem, result);
				SETFLAG_Z(result == 0);
				parityB(result);
				break;	}

	case 1:	{	uint16 result, data = loadW(cur_tlcs900h->mem);
				SETFLAG_C(data & 0x0001);
				result = (data >> 1);
				SETFLAG_S(result & 0x8000);
				storeW(cur_tlcs900h->mem, result);
				SETFLAG_Z(result == 0);
				parityW(result);
				break;	}
	}

	SETFLAG_H0;
	SETFLAG_N0;

	cur_tlcs900h->cycles = 8;
}

//===== ADD R,(cur_tlcs900h->mem)
void srcADDRm()
{
	switch(cur_tlcs900h->size)
	{
	case 0: regB(cur_tlcs900h->R) = generic_ADD_B(regB(cur_tlcs900h->R), loadB(cur_tlcs900h->mem)); cur_tlcs900h->cycles = 4;break;
	case 1: regW(cur_tlcs900h->R) = generic_ADD_W(regW(cur_tlcs900h->R), loadW(cur_tlcs900h->mem)); cur_tlcs900h->cycles = 4;break;
	case 2: regL(cur_tlcs900h->R) = generic_ADD_L(regL(cur_tlcs900h->R), loadL(cur_tlcs900h->mem)); cur_tlcs900h->cycles = 6;break;
	}
}

//===== ADD (cur_tlcs900h->mem),R
void srcADDmR()
{
	switch(cur_tlcs900h->size)
	{
	case 0:	storeB(cur_tlcs900h->mem, generic_ADD_B(loadB(cur_tlcs900h->mem), regB(cur_tlcs900h->R))); cur_tlcs900h->cycles = 6;break;
	case 1:	storeW(cur_tlcs900h->mem, generic_ADD_W(loadW(cur_tlcs900h->mem), regW(cur_tlcs900h->R))); cur_tlcs900h->cycles = 6;break;
	case 2:	storeL(cur_tlcs900h->mem, generic_ADD_L(loadL(cur_tlcs900h->mem), regL(cur_tlcs900h->R))); cur_tlcs900h->cycles = 10;break;
	}
}

//===== ADC R,(cur_tlcs900h->mem)
void srcADCRm()
{
	switch(cur_tlcs900h->size)
	{
	case 0: regB(cur_tlcs900h->R) = generic_ADC_B(regB(cur_tlcs900h->R), loadB(cur_tlcs900h->mem)); cur_tlcs900h->cycles = 4;break;
	case 1: regW(cur_tlcs900h->R) = generic_ADC_W(regW(cur_tlcs900h->R), loadW(cur_tlcs900h->mem)); cur_tlcs900h->cycles = 4;break;
	case 2: regL(cur_tlcs900h->R) = generic_ADC_L(regL(cur_tlcs900h->R), loadL(cur_tlcs900h->mem)); cur_tlcs900h->cycles = 6;break;
	}
}

//===== ADC (cur_tlcs900h->mem),R
void srcADCmR()
{
	switch(cur_tlcs900h->size)
	{
	case 0:	storeB(cur_tlcs900h->mem, generic_ADC_B(loadB(cur_tlcs900h->mem), regB(cur_tlcs900h->R))); cur_tlcs900h->cycles = 6;break;
	case 1:	storeW(cur_tlcs900h->mem, generic_ADC_W(loadW(cur_tlcs900h->mem), regW(cur_tlcs900h->R))); cur_tlcs900h->cycles = 6;break;
	case 2:	storeL(cur_tlcs900h->mem, generic_ADC_L(loadL(cur_tlcs900h->mem), regL(cur_tlcs900h->R))); cur_tlcs900h->cycles = 10;break;
	}
}

//===== SUB R,(cur_tlcs900h->mem)
void srcSUBRm()
{
	switch(cur_tlcs900h->size)
	{
	case 0: regB(cur_tlcs900h->R) = generic_SUB_B(regB(cur_tlcs900h->R), loadB(cur_tlcs900h->mem)); cur_tlcs900h->cycles = 4;break;
	case 1: regW(cur_tlcs900h->R) = generic_SUB_W(regW(cur_tlcs900h->R), loadW(cur_tlcs900h->mem)); cur_tlcs900h->cycles = 4;break;
	case 2: regL(cur_tlcs900h->R) = generic_SUB_L(regL(cur_tlcs900h->R), loadL(cur_tlcs900h->mem)); cur_tlcs900h->cycles = 6;break;
	}
}

//===== SUB (cur_tlcs900h->mem),R
void srcSUBmR()
{
	switch(cur_tlcs900h->size)
	{
	case 0:	storeB(cur_tlcs900h->mem, generic_SUB_B(loadB(cur_tlcs900h->mem), regB(cur_tlcs900h->R))); cur_tlcs900h->cycles = 6;break;
	case 1:	storeW(cur_tlcs900h->mem, generic_SUB_W(loadW(cur_tlcs900h->mem), regW(cur_tlcs900h->R))); cur_tlcs900h->cycles = 6;break;
	case 2:	storeL(cur_tlcs900h->mem, generic_SUB_L(loadL(cur_tlcs900h->mem), regL(cur_tlcs900h->R))); cur_tlcs900h->cycles = 10;break;
	}
}

//===== SBC R,(cur_tlcs900h->mem)
void srcSBCRm()
{
	switch(cur_tlcs900h->size)
	{
	case 0: regB(cur_tlcs900h->R) = generic_SBC_B(regB(cur_tlcs900h->R), loadB(cur_tlcs900h->mem)); cur_tlcs900h->cycles = 4;break;
	case 1: regW(cur_tlcs900h->R) = generic_SBC_W(regW(cur_tlcs900h->R), loadW(cur_tlcs900h->mem)); cur_tlcs900h->cycles = 4;break;
	case 2: regL(cur_tlcs900h->R) = generic_SBC_L(regL(cur_tlcs900h->R), loadL(cur_tlcs900h->mem)); cur_tlcs900h->cycles = 6;break;
	}
}

//===== SBC (cur_tlcs900h->mem),R
void srcSBCmR()
{
	switch(cur_tlcs900h->size)
	{
	case 0:	storeB(cur_tlcs900h->mem, generic_SBC_B(loadB(cur_tlcs900h->mem), regB(cur_tlcs900h->R))); cur_tlcs900h->cycles = 6;break;
	case 1:	storeW(cur_tlcs900h->mem, generic_SBC_W(loadW(cur_tlcs900h->mem), regW(cur_tlcs900h->R))); cur_tlcs900h->cycles = 6;break;
	case 2:	storeL(cur_tlcs900h->mem, generic_SBC_L(loadL(cur_tlcs900h->mem), regL(cur_tlcs900h->R))); cur_tlcs900h->cycles = 10;break;
	}
}

//===== AND R,(cur_tlcs900h->mem)
void srcANDRm()
{
	switch(cur_tlcs900h->size)
	{
	case 0:	{	uint8 result = regB(cur_tlcs900h->R) & loadB(cur_tlcs900h->mem);
				regB(cur_tlcs900h->R) = result;
				SETFLAG_Z(result == 0);
				SETFLAG_S(result & 0x80);
				parityB(result);
				cur_tlcs900h->cycles = 4;
				break; }
	
	case 1: {	uint16 result = regW(cur_tlcs900h->R) & loadW(cur_tlcs900h->mem);
				regW(cur_tlcs900h->R) = result;
				SETFLAG_Z(result == 0);
				SETFLAG_S(result & 0x8000);
				parityW(result);
				cur_tlcs900h->cycles = 4;
				break; }

	case 2:	{	uint32 result = regL(cur_tlcs900h->R) & loadL(cur_tlcs900h->mem);
				regL(cur_tlcs900h->R) = result;
				SETFLAG_Z(result == 0);
				SETFLAG_S(result & 0x80000000);
				cur_tlcs900h->cycles = 6;
				break; }
	}

	SETFLAG_H1;
	SETFLAG_N0;
	SETFLAG_C0;
}

//===== AND (cur_tlcs900h->mem),R
void srcANDmR()
{
	switch(cur_tlcs900h->size)
	{
	case 0:	{	uint8 result = regB(cur_tlcs900h->R) & loadB(cur_tlcs900h->mem);
				storeB(cur_tlcs900h->mem, result);
				SETFLAG_Z(result == 0);
				SETFLAG_S(result & 0x80);
				parityB(result);
				cur_tlcs900h->cycles = 6;
				break; }
	
	case 1: {	uint16 result = regW(cur_tlcs900h->R) & loadW(cur_tlcs900h->mem);
				storeW(cur_tlcs900h->mem, result);
				SETFLAG_Z(result == 0);
				SETFLAG_S(result & 0x8000);
				parityW(result);
				cur_tlcs900h->cycles = 6;
				break; }

	case 2:	{	uint32 result = regL(cur_tlcs900h->R) & loadL(cur_tlcs900h->mem);
				storeL(cur_tlcs900h->mem, result);
				SETFLAG_Z(result == 0);
				SETFLAG_S(result & 0x80000000);
				cur_tlcs900h->cycles = 10;
				break; }
	}

	SETFLAG_H1;
	SETFLAG_N0;
	SETFLAG_C0;
}

//===== XOR R,(cur_tlcs900h->mem)
void srcXORRm()
{
	switch(cur_tlcs900h->size)
	{
	case 0:	{	uint8 result = regB(cur_tlcs900h->R) ^ loadB(cur_tlcs900h->mem);
				regB(cur_tlcs900h->R) = result;
				SETFLAG_Z(result == 0);
				SETFLAG_S(result & 0x80);
				parityB(result);
				cur_tlcs900h->cycles = 4;
				break; }
	
	case 1: {	uint16 result = regW(cur_tlcs900h->R) ^ loadW(cur_tlcs900h->mem);
				regW(cur_tlcs900h->R) = result;
				SETFLAG_Z(result == 0);
				SETFLAG_S(result & 0x8000);
				parityW(result);
				cur_tlcs900h->cycles = 4;
				break; }

	case 2:	{	uint32 result = regL(cur_tlcs900h->R) ^ loadL(cur_tlcs900h->mem);
				regL(cur_tlcs900h->R) = result;
				SETFLAG_Z(result == 0);
				SETFLAG_S(result & 0x80000000);
				cur_tlcs900h->cycles = 6;
				break; }
	}

	SETFLAG_H0;
	SETFLAG_N0;
	SETFLAG_C0;
}

//===== XOR (cur_tlcs900h->mem),R
void srcXORmR()
{
	switch(cur_tlcs900h->size)
	{
	case 0:	{	uint8 result = regB(cur_tlcs900h->R) ^ loadB(cur_tlcs900h->mem);
				storeB(cur_tlcs900h->mem, result);
				SETFLAG_Z(result == 0);
				SETFLAG_S(result & 0x80);
				parityB(result);
				cur_tlcs900h->cycles = 6;
				break; }
	
	case 1: {	uint16 result = regW(cur_tlcs900h->R) ^ loadW(cur_tlcs900h->mem);
				storeW(cur_tlcs900h->mem, result);
				SETFLAG_Z(result == 0);
				SETFLAG_S(result & 0x8000);
				parityW(result);
				cur_tlcs900h->cycles = 6;
				break; }

	case 2:	{	uint32 result = regL(cur_tlcs900h->R) ^ loadL(cur_tlcs900h->mem);
				storeL(cur_tlcs900h->mem, result);
				SETFLAG_Z(result == 0);
				SETFLAG_S(result & 0x80000000);
				cur_tlcs900h->cycles = 10;
				break; }
	}

	SETFLAG_H0;
	SETFLAG_N0;
	SETFLAG_C0;
}

//===== OR R,(cur_tlcs900h->mem)
void srcORRm()
{
	switch(cur_tlcs900h->size)
	{
	case 0:	{	uint8 result = regB(cur_tlcs900h->R) | loadB(cur_tlcs900h->mem);
				regB(cur_tlcs900h->R) = result;
				SETFLAG_Z(result == 0);
				SETFLAG_S(result & 0x80);
				parityB(result);
				cur_tlcs900h->cycles = 4;
				break; }
	
	case 1: {	uint16 result = regW(cur_tlcs900h->R) | loadW(cur_tlcs900h->mem);
				regW(cur_tlcs900h->R) = result;
				SETFLAG_Z(result == 0);
				SETFLAG_S(result & 0x8000);
				parityW(result);
				cur_tlcs900h->cycles = 4;
				break; }

	case 2:	{	uint32 result = regL(cur_tlcs900h->R) | loadL(cur_tlcs900h->mem);
				regL(cur_tlcs900h->R) = result;
				SETFLAG_Z(result == 0);
				SETFLAG_S(result & 0x80000000);
				cur_tlcs900h->cycles = 6;
				break; }
	}

	SETFLAG_H0;
	SETFLAG_N0;
	SETFLAG_C0;
}

//===== OR (cur_tlcs900h->mem),R
void srcORmR()
{
	switch(cur_tlcs900h->size)
	{
	case 0:	{	uint8 result = regB(cur_tlcs900h->R) | loadB(cur_tlcs900h->mem);
				storeB(cur_tlcs900h->mem, result);
				SETFLAG_Z(result == 0);
				SETFLAG_S(result & 0x80);
				parityB(result);
				cur_tlcs900h->cycles = 6;
				break; }
	
	case 1: {	uint16 result = regW(cur_tlcs900h->R) | loadW(cur_tlcs900h->mem);
				storeW(cur_tlcs900h->mem, result);
				SETFLAG_Z(result == 0);
				SETFLAG_S(result & 0x8000);
				parityW(result);
				cur_tlcs900h->cycles = 6;
				break; }

	case 2:	{	uint32 result = regL(cur_tlcs900h->R) | loadL(cur_tlcs900h->mem);
				storeL(cur_tlcs900h->mem, result);
				SETFLAG_Z(result == 0);
				SETFLAG_S(result & 0x80000000);
				cur_tlcs900h->cycles = 10;
				break; }
	}

	SETFLAG_H0;
	SETFLAG_N0;
	SETFLAG_C0;
}

//===== CP R,(cur_tlcs900h->mem)
void srcCPRm(void)
{
   switch(cur_tlcs900h->size)
   {
      case 0:
         generic_SUB_B(regB(cur_tlcs900h->R), loadB(cur_tlcs900h->mem));
         cur_tlcs900h->cycles = 4;
         break;
      case 1:
         generic_SUB_W(regW(cur_tlcs900h->R), loadW(cur_tlcs900h->mem));
         cur_tlcs900h->cycles = 4;
         break;
      case 2:
         generic_SUB_L(regL(cur_tlcs900h->R), loadL(cur_tlcs900h->mem));
         cur_tlcs900h->cycles = 6;
         break;
   }
}

//===== CP (cur_tlcs900h->mem),R
void srcCPmR(void)
{
   switch(cur_tlcs900h->size)
   {
      case 0:
         generic_SUB_B(loadB(cur_tlcs900h->mem), regB(cur_tlcs900h->R));
         break;
      case 1:
         generic_SUB_W(loadW(cur_tlcs900h->mem), regW(cur_tlcs900h->R));
         break;
      case 2:
         generic_SUB_L(loadL(cur_tlcs900h->mem), regL(cur_tlcs900h->R));
         break;
   }

   cur_tlcs900h->cycles = 6;
}

//=============================================================================
