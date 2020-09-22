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

	TLCS900h_interpret_dst.c

//=========================================================================
//---------------------------------------------------------------------------

  History of changes:
  ===================

20 JUL 2002 - neopop_uk
=======================================
- Cleaned and tidied up for the source release

22 JUL 2002 - neopop_uk
=======================================
- Added ANDCF,ORCF and XORCF in # and A modes. These were being used
	by one of the obscure pachinko "games".

23 JUL 2002 - neopop_uk
=======================================
- Added cycle count for TSET.

16 AUG 2002 - neopop_uk
=======================================
- Replaced 'second & 7' with 'R', clearer, faster - and for some reason
	more accurate... oh well!

21 AUG 2002 - neopop_uk
=======================================
- Added TSET.

//---------------------------------------------------------------------------
*/

#include "TLCS900h_interpret.h"
#include "TLCS900h_registers.h"
#include "../mem.h"

//=========================================================================

//===== LD (mem),#
void DST_dstLDBi(void)
{
	storeB(cur_tlcs900h->mem, FETCH8);
	cur_tlcs900h->cycles = 5;
}

//===== LD (cur_tlcs900h->mem),#
void DST_dstLDWi(void)
{
	storeW(cur_tlcs900h->mem, fetch16());
	cur_tlcs900h->cycles = 6;
}

//===== POP (cur_tlcs900h->mem)
void DST_dstPOPB(void)
{
	storeB(cur_tlcs900h->mem, pop8());
	cur_tlcs900h->cycles = 6;
}

//===== POP (cur_tlcs900h->mem)
void DST_dstPOPW(void)
{
	storeW(cur_tlcs900h->mem, pop16());
	cur_tlcs900h->cycles = 6;
}

//===== LD (cur_tlcs900h->mem),(nn)
void DST_dstLDBm16(void)
{
	storeB(cur_tlcs900h->mem, loadB(fetch16()));
	cur_tlcs900h->cycles = 8;
}

//===== LD (cur_tlcs900h->mem),(nn)
void DST_dstLDWm16(void)
{
	storeW(cur_tlcs900h->mem, loadW(fetch16()));
	cur_tlcs900h->cycles = 8;
}

//===== LDA R,cur_tlcs900h->mem
void DST_dstLDAW(void)
{
	regW(cur_tlcs900h->R) = (uint16)cur_tlcs900h->mem;
	cur_tlcs900h->cycles = 4;
}

//===== LDA R,cur_tlcs900h->mem
void DST_dstLDAL(void)
{
	regL(cur_tlcs900h->R) = (uint32)cur_tlcs900h->mem;
	cur_tlcs900h->cycles = 4;
}

//===== ANDCF A,(cur_tlcs900h->mem)
void DST_dstANDCFA(void)
{
	uint8 bit = REGA & 0xF;
	uint8 mbit = (loadB(cur_tlcs900h->mem) >> bit) & 1;
	if (bit < 8) SETFLAG_C(mbit & FLAG_C);
	cur_tlcs900h->cycles = 8;
}

//===== ORCF A,(cur_tlcs900h->mem)
void DST_dstORCFA(void)
{
	uint8 bit = REGA & 0xF;
	uint8 mbit = (loadB(cur_tlcs900h->mem) >> bit) & 1;
	if (bit < 8) SETFLAG_C(mbit | FLAG_C);
	cur_tlcs900h->cycles = 8;
}

//===== XORCF A,(cur_tlcs900h->mem)
void DST_dstXORCFA(void)
{
	uint8 bit = REGA & 0xF;
	uint8 mbit = (loadB(cur_tlcs900h->mem) >> bit) & 1;
	if (bit < 8) SETFLAG_C(mbit ^ FLAG_C);
	cur_tlcs900h->cycles = 8;
}

//===== LDCF A,(cur_tlcs900h->mem)
void DST_dstLDCFA(void)
{
	uint8 bit = REGA & 0xF;
	uint8 mask = (1 << bit);
	if (bit < 8) SETFLAG_C(loadB(cur_tlcs900h->mem) & mask);
	cur_tlcs900h->cycles = 8;
}

//===== STCF A,(cur_tlcs900h->mem)
void DST_dstSTCFA(void)
{
	uint8 bit = REGA & 0xF;
	uint8 cmask = ~(1 << bit);
	uint8 set = FLAG_C << bit;
	if (bit < 8) storeB(cur_tlcs900h->mem, (loadB(cur_tlcs900h->mem) & cmask) | set); 
	cur_tlcs900h->cycles = 8;
}

//===== LD (cur_tlcs900h->mem),R
void DST_dstLDBR(void)
{
	storeB(cur_tlcs900h->mem, regB(cur_tlcs900h->R));
	cur_tlcs900h->cycles = 4;
}

//===== LD (cur_tlcs900h->mem),R
void DST_dstLDWR(void)
{
	storeW(cur_tlcs900h->mem, regW(cur_tlcs900h->R));
	cur_tlcs900h->cycles = 4;
}

//===== LD (cur_tlcs900h->mem),R
void DST_dstLDLR(void)
{
	storeL(cur_tlcs900h->mem, regL(cur_tlcs900h->R));
	cur_tlcs900h->cycles = 6;
}

//===== ANDCF #3,(cur_tlcs900h->mem)
void DST_dstANDCF(void)
{
	uint8 bit = cur_tlcs900h->R;
	uint8 mbit = (loadB(cur_tlcs900h->mem) >> bit) & 1;
	SETFLAG_C(mbit & FLAG_C);
	cur_tlcs900h->cycles = 8;
}

//===== ORCF #3,(cur_tlcs900h->mem)
void DST_dstORCF(void)
{
	uint8 bit = cur_tlcs900h->R;
	uint8 mbit = (loadB(cur_tlcs900h->mem) >> bit) & 1;
	SETFLAG_C(mbit | FLAG_C);
	cur_tlcs900h->cycles = 8;
}

//===== XORCF #3,(cur_tlcs900h->mem)
void DST_dstXORCF(void)
{
	uint8 bit = cur_tlcs900h->R;
	uint8 mbit = (loadB(cur_tlcs900h->mem) >> bit) & 1;
	SETFLAG_C(mbit ^ FLAG_C);
	cur_tlcs900h->cycles = 8;
}

//===== LDCF #3,(cur_tlcs900h->mem)
void DST_dstLDCF(void)
{
	uint8 bit = cur_tlcs900h->R;
	uint32 mask = (1 << bit);
	SETFLAG_C(loadB(cur_tlcs900h->mem) & mask);
	cur_tlcs900h->cycles = 8;
}

//===== STCF #3,(cur_tlcs900h->mem)
void DST_dstSTCF(void)
{
	uint8 bit = cur_tlcs900h->R;
	uint8 cmask = ~(1 << bit);
	uint8 set = FLAG_C << bit;
	storeB(cur_tlcs900h->mem, (loadB(cur_tlcs900h->mem) & cmask) | set); 
	cur_tlcs900h->cycles = 8;
}

//===== TSET #3,(cur_tlcs900h->mem)
void DST_dstTSET(void)
{
	SETFLAG_Z(! (loadB(cur_tlcs900h->mem) & (1 << cur_tlcs900h->R)) );
	storeB(cur_tlcs900h->mem, loadB(cur_tlcs900h->mem) | (1 << cur_tlcs900h->R));

	SETFLAG_H1
	SETFLAG_N0
	cur_tlcs900h->cycles = 10;
}

//===== RES #3,(cur_tlcs900h->mem)
void DST_dstRES(void)
{
	storeB(cur_tlcs900h->mem, loadB(cur_tlcs900h->mem) & (~(1 << cur_tlcs900h->R)));
	cur_tlcs900h->cycles = 8;
}

//===== SET #3,(cur_tlcs900h->mem)
void DST_dstSET(void)
{
	storeB(cur_tlcs900h->mem, loadB(cur_tlcs900h->mem) | (1 << cur_tlcs900h->R));
	cur_tlcs900h->cycles = 8;
}

//===== CHG #3,(cur_tlcs900h->mem)
void DST_dstCHG(void)
{
	storeB(cur_tlcs900h->mem, loadB(cur_tlcs900h->mem) ^ (1 << cur_tlcs900h->R));
	cur_tlcs900h->cycles = 8;
}

//===== BIT #3,(cur_tlcs900h->mem)
void DST_dstBIT(void)
{
   SETFLAG_Z(! (loadB(cur_tlcs900h->mem) & (1 << cur_tlcs900h->R)) );
   SETFLAG_H1;
   SETFLAG_N0;
   cur_tlcs900h->cycles = 8;
}

//===== JP cc,cur_tlcs900h->mem
void DST_dstJP(void)
{
   cur_tlcs900h->cycles = 6;
   if (conditionCode(cur_tlcs900h->second & 0xF))
   {
      cur_tlcs900h->pc = cur_tlcs900h->mem;
      cur_tlcs900h->cycles += 3;
   }
}

//===== CALL cc,cur_tlcs900h->mem
void DST_dstCALL(void)
{
   cur_tlcs900h->cycles = 6;
   if (conditionCode(cur_tlcs900h->second & 0xF))
   {
      push32(cur_tlcs900h->pc);
      cur_tlcs900h->pc = cur_tlcs900h->mem;
      cur_tlcs900h->cycles += 6;
   }
}

//===== RET cc
void DST_dstRET(void)
{
   cur_tlcs900h->cycles = 6;
   if (conditionCode(cur_tlcs900h->second & 0xF))
   {
      cur_tlcs900h->pc = pop32();
      cur_tlcs900h->cycles += 6;
   }
}

//=============================================================================
