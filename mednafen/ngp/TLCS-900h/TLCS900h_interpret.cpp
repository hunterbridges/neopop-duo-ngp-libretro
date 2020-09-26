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

	TLCS900h_interpret.c

//=========================================================================
//---------------------------------------------------------------------------

  History of changes:
  ===================

20 JUL 2002 - neopop_uk
=======================================
- Cleaned and tidied up for the source release

26 JUL 2002 - neopop_uk
=======================================
- Fixed a nasty bug that only affects [src]"EX (mem), XWA", 
	it was executing "EX F,F'" instead - Very bad! 

28 JUL 2002 - neopop_uk
=======================================
- Added generic DIV and DIVS functions

30 AUG 2002 - neopop_uk
=======================================
- Fixed detection of R32+d16 addressing mode.

02 SEP 2002 - neopop_uk
=======================================
- Added the undocumented type 0x13 R32 address mode.

09 SEP 2002 - neopop_uk
=======================================
- Extra cycles for addressing modes.

//---------------------------------------------------------------------------
*/

#include "TLCS900h.h"
#include "../mem.h"
#include "../bios.h"

static void DUMMY_instruction_error(const char* vaMessage,...) { }

void (*instruction_error)(const char* vaMessage,...) = DUMMY_instruction_error;

//=========================================================================

uint16 TLCS900h::fetch16(void)
{
	uint16 a = loadW(pc);
	pc += 2;
	return a;
}

uint32 TLCS900h::fetch24(void)
{
	uint32 b, a = loadW(pc);
	pc += 2;
	b = loadB(pc++);
	return (b << 16) | a;
}

uint32 TLCS900h::fetch32(void)
{
	uint32 a = loadL(pc);
	pc += 4;
	return a;
}

//=============================================================================

void TLCS900h::parityB(uint8 value)
{
	uint8 count = 0, i;

	for (i = 0; i < 8; i++)
	{
		if (value & 1) count++;
		value >>= 1;
	}

	// if (count & 1) == false, means even, thus SET
	SETFLAG_V((count & 1) == 0);
}

void TLCS900h::parityW(uint16 value)
{
	uint8 count = 0, i;

	for (i = 0; i < 16; i++)
	{
		if (value & 1) count++;
		value >>= 1;
	}

	// if (count & 1) == false, means even, thus SET
	SETFLAG_V((count & 1) == 0);
}

//=========================================================================

void TLCS900h::push8(uint8 data)
{
   REGXSP -= 1;
   storeB(REGXSP, data);
}

void TLCS900h::push16(uint16 data)
{
   REGXSP -= 2;
   storeW(REGXSP, data);
}

void TLCS900h::push32(uint32 data)
{
   REGXSP -= 4;
   storeL(REGXSP, data);
}

uint8 TLCS900h::pop8(void)
{
   uint8 temp = loadB(REGXSP);
   REGXSP += 1;
   return temp;
}

uint16 TLCS900h::pop16(void)
{
   uint16 temp = loadW(REGXSP);
   REGXSP += 2;
   return temp;
}

uint32 TLCS900h::pop32(void)
{
   uint32 temp = loadL(REGXSP);
   REGXSP += 4;
   return temp;
}

//=============================================================================

uint16 TLCS900h::generic_DIV_B(uint16 val, uint8 div)
{
	if (div == 0)
	{ 
		SETFLAG_V1
		return (val << 8) | ((val >> 8) ^ 0xFF);
	}
	else
	{
		uint16 quo = val / (uint16)div;
		uint16 rem = val % (uint16)div;
		if (quo > 0xFF) SETFLAG_V1 else SETFLAG_V0
		return (quo & 0xFF) | ((rem & 0xFF) << 8);
	}
}

uint32 TLCS900h::generic_DIV_W(uint32 val, uint16 div)
{
	if (div == 0)
	{ 
		SETFLAG_V1
		return (val << 16) | ((val >> 16) ^ 0xFFFF);
	}
	else
	{
		uint32 quo = val / (uint32)div;
		uint32 rem = val % (uint32)div;
		if (quo > 0xFFFF) SETFLAG_V1 else SETFLAG_V0
		return (quo & 0xFFFF) | ((rem & 0xFFFF) << 16);
	}
}

//=============================================================================

uint16 TLCS900h::generic_DIVS_B(int16 val, int8 div)
{
	if (div == 0)
	{
		SETFLAG_V1
		return (val << 8) | ((val >> 8) ^ 0xFF);
	}
	else
	{
		int16 quo = val / (int16)div;
		int16 rem = val % (int16)div;
		if (quo > 0xFF) SETFLAG_V1 else SETFLAG_V0
		return (quo & 0xFF) | ((rem & 0xFF) << 8);
	}
}

uint32 TLCS900h::generic_DIVS_W(int32 val, int16 div)
{
	if (div == 0)
	{
		SETFLAG_V1
		return (val << 16) | ((val >> 16) ^ 0xFFFF);
	}
	else
	{
		int32 quo = val / (int32)div;
		int32 rem = val % (int32)div;
		if (quo > 0xFFFF) SETFLAG_V1 else SETFLAG_V0
		return (quo & 0xFFFF) | ((rem & 0xFFFF) << 16);
	}
}

//=============================================================================

uint8 TLCS900h::generic_ADD_B(uint8 dst, uint8 src)
{
	uint8 half = (dst & 0xF) + (src & 0xF);
	uint32 resultC = (uint32)dst + (uint32)src;
	uint8 result = (uint8)(resultC & 0xFF);

	SETFLAG_S(result & 0x80);
	SETFLAG_Z(result == 0);
	SETFLAG_H(half > 0xF);

	if ((((int8)dst >= 0) && ((int8)src >= 0) && ((int8)result < 0)) ||
		(((int8)dst < 0)  && ((int8)src < 0) && ((int8)result >= 0)))
	{SETFLAG_V1} else {SETFLAG_V0}

	SETFLAG_N0;
	SETFLAG_C(resultC > 0xFF);

	return result;
}

uint16 TLCS900h::generic_ADD_W(uint16 dst, uint16 src)
{
	uint16 half = (dst & 0xF) + (src & 0xF);
	uint32 resultC = (uint32)dst + (uint32)src;
	uint16 result = (uint16)(resultC & 0xFFFF);

	SETFLAG_S(result & 0x8000);
	SETFLAG_Z(result == 0);
	SETFLAG_H(half > 0xF);

	if ((((int16)dst >= 0) && ((int16)src >= 0) && ((int16)result < 0)) ||
		(((int16)dst < 0)  && ((int16)src < 0) && ((int16)result >= 0)))
	{SETFLAG_V1} else {SETFLAG_V0}

	SETFLAG_N0;
	SETFLAG_C(resultC > 0xFFFF);

	return result;
}

uint32 TLCS900h::generic_ADD_L(uint32 dst, uint32 src)
{
	uint64 resultC = (uint64)dst + (uint64)src;
	uint32 result = (uint32)(resultC & 0xFFFFFFFF);

	SETFLAG_S(result & 0x80000000);
	SETFLAG_Z(result == 0);

	if ((((int32)dst >= 0) && ((int32)src >= 0) && ((int32)result < 0)) || 
		(((int32)dst < 0)  && ((int32)src < 0) && ((int32)result >= 0)))
	{SETFLAG_V1} else {SETFLAG_V0}
	
	SETFLAG_N0;
	SETFLAG_C(resultC > 0xFFFFFFFF);

	return result;
}

//=============================================================================

uint8 TLCS900h::generic_ADC_B(uint8 dst, uint8 src)
{
	uint8 half = (dst & 0xF) + (src & 0xF) + FLAG_C;
	uint32 resultC = (uint32)dst + (uint32)src + (uint32)FLAG_C;
	uint8 result = (uint8)(resultC & 0xFF);

	SETFLAG_S(result & 0x80);
	SETFLAG_Z(result == 0);
	SETFLAG_H(half > 0xF);

	if ((((int8)dst >= 0) && ((int8)src >= 0) && ((int8)result < 0)) || 
		(((int8)dst < 0)  && ((int8)src < 0) && ((int8)result >= 0)))
	{SETFLAG_V1} else {SETFLAG_V0}

	SETFLAG_N0;
	SETFLAG_C(resultC > 0xFF);

	return result;
}

uint16 TLCS900h::generic_ADC_W(uint16 dst, uint16 src)
{
	uint16 half = (dst & 0xF) + (src & 0xF) + FLAG_C;
	uint32 resultC = (uint32)dst + (uint32)src + (uint32)FLAG_C;
	uint16 result = (uint16)(resultC & 0xFFFF);

	SETFLAG_S(result & 0x8000);
	SETFLAG_Z(result == 0);
	SETFLAG_H(half > 0xF);

	if ((((int16)dst >= 0) && ((int16)src >= 0) && ((int16)result < 0)) || 
		(((int16)dst < 0)  && ((int16)src < 0) && ((int16)result >= 0)))
	{SETFLAG_V1} else {SETFLAG_V0}

	SETFLAG_N0;
	SETFLAG_C(resultC > 0xFFFF);

	return result;
}

uint32 TLCS900h::generic_ADC_L(uint32 dst, uint32 src)
{
	uint64 resultC = (uint64)dst + (uint64)src + (uint64)FLAG_C;
	uint32 result = (uint32)(resultC & 0xFFFFFFFF);

	SETFLAG_S(result & 0x80000000);
	SETFLAG_Z(result == 0);

	if ((((int32)dst >= 0) && ((int32)src >= 0) && ((int32)result < 0)) || 
		(((int32)dst < 0)  && ((int32)src < 0) && ((int32)result >= 0)))
	{SETFLAG_V1} else {SETFLAG_V0}
	
	SETFLAG_N0;
	SETFLAG_C(resultC > 0xFFFFFFFF);

	return result;
}

//=============================================================================

uint8 TLCS900h::generic_SUB_B(uint8 dst, uint8 src)
{
	uint8 half = (dst & 0xF) - (src & 0xF);
	uint32 resultC = (uint32)dst - (uint32)src;
	uint8 result = (uint8)(resultC & 0xFF);

	SETFLAG_S(result & 0x80);
	SETFLAG_Z(result == 0);
	SETFLAG_H(half > 0xF);

	if ((((int8)dst >= 0) && ((int8)src < 0) && ((int8)result < 0)) ||
		(((int8)dst < 0) && ((int8)src >= 0) && ((int8)result >= 0)))
	{
      SETFLAG_V1
   }
   else
   {
      SETFLAG_V0
   }

	SETFLAG_N1;
	SETFLAG_C(resultC > 0xFF);

	return result;
}

uint16 TLCS900h::generic_SUB_W(uint16 dst, uint16 src)
{
	uint16 half = (dst & 0xF) - (src & 0xF);
	uint32 resultC = (uint32)dst - (uint32)src;
	uint16 result = (uint16)(resultC & 0xFFFF);

	SETFLAG_S(result & 0x8000);
	SETFLAG_Z(result == 0);
	SETFLAG_H(half > 0xF);

	if ((((int16)dst >= 0) && ((int16)src < 0) && ((int16)result < 0)) ||
		(((int16)dst < 0) && ((int16)src >= 0) && ((int16)result >= 0)))
	{SETFLAG_V1} else {SETFLAG_V0}

	SETFLAG_N1;
	SETFLAG_C(resultC > 0xFFFF);

	return result;
}

uint32 TLCS900h::generic_SUB_L(uint32 dst, uint32 src)
{
	uint64 resultC = (uint64)dst - (uint64)src;
	uint32 result = (uint32)(resultC & 0xFFFFFFFF);

	SETFLAG_S(result & 0x80000000);
	SETFLAG_Z(result == 0);

	if ((((int32)dst >= 0) && ((int32)src < 0) && ((int32)result < 0)) ||
		(((int32)dst < 0) && ((int32)src >= 0) && ((int32)result >= 0)))
	{SETFLAG_V1} else {SETFLAG_V0}
	
	SETFLAG_N1;
	SETFLAG_C(resultC > 0xFFFFFFFF);

	return result;
}

//=============================================================================

uint8 TLCS900h::generic_SBC_B(uint8 dst, uint8 src)
{
	uint8 half = (dst & 0xF) - (src & 0xF) - FLAG_C;
	uint32 resultC = (uint32)dst - (uint32)src - (uint32)FLAG_C;
	uint8 result = (uint8)(resultC & 0xFF);

	SETFLAG_S(result & 0x80);
	SETFLAG_Z(result == 0);
	SETFLAG_H(half > 0xF);

	if ((((int8)dst >= 0) && ((int8)src < 0) && ((int8)result < 0)) ||
		(((int8)dst < 0) && ((int8)src >= 0) && ((int8)result >= 0)))
	{SETFLAG_V1} else {SETFLAG_V0}

	SETFLAG_N1;
	SETFLAG_C(resultC > 0xFF);

	return result;
}

uint16 TLCS900h::generic_SBC_W(uint16 dst, uint16 src)
{
	uint16 half = (dst & 0xF) - (src & 0xF) - FLAG_C;
	uint32 resultC = (uint32)dst - (uint32)src - (uint32)FLAG_C;
	uint16 result = (uint16)(resultC & 0xFFFF);

	SETFLAG_S(result & 0x8000);
	SETFLAG_Z(result == 0);
	SETFLAG_H(half > 0xF);

	if ((((int16)dst >= 0) && ((int16)src < 0) && ((int16)result < 0)) ||
		(((int16)dst < 0) && ((int16)src >= 0) && ((int16)result >= 0)))
	{SETFLAG_V1} else {SETFLAG_V0}

	SETFLAG_N1;
	SETFLAG_C(resultC > 0xFFFF);

	return result;
}

uint32 TLCS900h::generic_SBC_L(uint32 dst, uint32 src)
{
	uint64 resultC = (uint64)dst - (uint64)src - (uint64)FLAG_C;
	uint32 result = (uint32)(resultC & 0xFFFFFFFF);

	SETFLAG_S(result & 0x80000000);
	SETFLAG_Z(result == 0);

	if ((((int32)dst >= 0) && ((int32)src < 0) && ((int32)result < 0)) ||
		(((int32)dst < 0) && ((int32)src >= 0) && ((int32)result >= 0)))
	{SETFLAG_V1} else {SETFLAG_V0}
	
	SETFLAG_N1;
	SETFLAG_C(resultC > 0xFFFFFFFF);

	return result;
}

//=============================================================================

bool TLCS900h::conditionCode(int cc)
{
   switch(cc)
   {
      case 0:
         return 0;	//(F)
      case 1:
         if (FLAG_S ^ FLAG_V)
            return 1;
         return 0;	//(LT)
      case 2:
         if (FLAG_Z | (FLAG_S ^ FLAG_V))
            return 1;
         return 0;	//(LE)
      case 3:
         if (FLAG_C | FLAG_Z)
            return 1;
         return 0;	//(ULE)
      case 4:
         if (FLAG_V)
            return 1;
         return 0;	//(OV)
      case 5:
         if (FLAG_S)
            return 1;
         return 0;	//(MI)
      case 6:
         if (FLAG_Z)
            return 1;
         return 0;	//(Z)
      case 7:
         if (FLAG_C)
            return 1;
         return 0;	//(C)
      case 8:
         return 1;	//always True														
      case 9:
         if (FLAG_S ^ FLAG_V)
            return 0;
         return 1;	//(GE)
      case 10:
         if (FLAG_Z | (FLAG_S ^ FLAG_V))
            return 0;
         return 1;	//(GT)
      case 11:
         if (FLAG_C | FLAG_Z)
            return 0;
         return 1;	//(UGT)
      case 12:
         if (FLAG_V)
            return 0;
         return 1;	//(NOV)
      case 13:
         if (FLAG_S)
            return 0;
         return 1;	//(PL)
      case 14:
         if (FLAG_Z)
            return 0;
         return 1;	//(NZ)
      case 15:
         if (FLAG_C)
            return 0;
         return 1;	//(NC)
   }

#ifdef NEOPOP_DEBUG
   system_debug_message("Unknown Condition Code %d", cc);
#endif
   return false;
}

//=============================================================================

uint8 TLCS900h::get_rr_Target(void)
{
	uint8 target = 0x80;

	if (size == 0 && first == 0xC7)
		return rCode;

	//Create a regCode
	switch(first & 7)
	{
	case 0: if (size == 1)	target = 0xE0;	break;
	case 1:	
		if (size == 0)	target = 0xE0;
		if (size == 1)	target = 0xE4;
		break;
	case 2: if (size == 1)	target = 0xE8;	break;
	case 3:
		if (size == 0)	target = 0xE4;
		if (size == 1)	target = 0xEC;
		break;
	case 4: if (size == 1)	target = 0xF0;	break;
	case 5:	
		if (size == 0)	target = 0xE8;
		if (size == 1)	target = 0xF4;
		break;
	case 6: if (size == 1)	target = 0xF8;	break;
	case 7:
		if (size == 0)	target = 0xEC;
		if (size == 1)	target = 0xFC;
		break;
	}

	return target;
}

uint8 TLCS900h::get_RR_Target(void)
{
   uint8 target = 0x80;

   //Create a regCode
   switch(second & 7)
   {
      case 0:
         if (size == 1)
            target = 0xE0;
         break;
      case 1:	
         if (size == 0)
            target = 0xE0;
         if (size == 1)
            target = 0xE4;
         break;
      case 2:
         if (size == 1)
            target = 0xE8;
         break;
      case 3:
         if (size == 0)
            target = 0xE4;
         if (size == 1)
            target = 0xEC;
         break;
      case 4:
         if (size == 1)
            target = 0xF0;
         break;
      case 5:	
         if (size == 0)
            target = 0xE8;
         if (size == 1)
            target = 0xF4;
         break;
      case 6:
         if (size == 1)
            target = 0xF8;
         break;
      case 7:
         if (size == 0)
            target = 0xEC;
         if (size == 1)
            target = 0xFC;
         break;
   }

   return target;
}

//=========================================================================

void TLCS900h::ExXWA()		{mem = regL(0);}
void TLCS900h::ExXBC()		{mem = regL(1);}
void TLCS900h::ExXDE()		{mem = regL(2);}
void TLCS900h::ExXHL()		{mem = regL(3);}
void TLCS900h::ExXIX()		{mem = regL(4);}
void TLCS900h::ExXIY()		{mem = regL(5);}
void TLCS900h::ExXIZ()		{mem = regL(6);}
void TLCS900h::ExXSP()		{mem = regL(7);}

void TLCS900h::ExXWAd()	{mem = regL(0) + (int8)FETCH8; cycles_extra = 2;}
void TLCS900h::ExXBCd()	{mem = regL(1) + (int8)FETCH8; cycles_extra = 2;}
void TLCS900h::ExXDEd()	{mem = regL(2) + (int8)FETCH8; cycles_extra = 2;}
void TLCS900h::ExXHLd()	{mem = regL(3) + (int8)FETCH8; cycles_extra = 2;}
void TLCS900h::ExXIXd()	{mem = regL(4) + (int8)FETCH8; cycles_extra = 2;}
void TLCS900h::ExXIYd()	{mem = regL(5) + (int8)FETCH8; cycles_extra = 2;}
void TLCS900h::ExXIZd()	{mem = regL(6) + (int8)FETCH8; cycles_extra = 2;}
void TLCS900h::ExXSPd()	{mem = regL(7) + (int8)FETCH8; cycles_extra = 2;}

void TLCS900h::Ex8(void)
{
   mem = FETCH8;
   cycles_extra = 2;
}

void TLCS900h::Ex16(void)
{
   mem = fetch16();
   cycles_extra = 2;
}

void TLCS900h::Ex24(void)
{
   mem = fetch24();
   cycles_extra = 3;
}

void TLCS900h::ExR32(void)
{
	uint8 data = FETCH8;

	if (data == 0x03)
	{
		uint8 rIndex, r32;
		r32 = FETCH8;		//r32
		rIndex = FETCH8;	//r8
		mem = rCodeL(r32) + (int8)rCodeB(rIndex);
		cycles_extra = 8;
		return;
	}

	if (data == 0x07)
	{
		uint8 rIndex, r32;
		r32 = FETCH8;		//r32
		rIndex = FETCH8;	//r16
		mem = rCodeL(r32) + (int16)rCodeW(rIndex);
		cycles_extra = 8;
		return;
	}

	//Undocumented mode!
	if (data == 0x13)
	{
		int16 disp = fetch16();
		mem = pc + disp;
		cycles_extra = 8;	//Unconfirmed... doesn't make much difference
		return;
	}

	cycles_extra = 5;

   mem = rCodeL(data);
	if ((data & 3) == 1)
		mem += (int16)fetch16();
}

void TLCS900h::ExDec()
{
	uint8 data = FETCH8;
	uint8 r32 = data & 0xFC;

	cycles_extra = 3;

	switch(data & 3)
	{
	case 0:	rCodeL(r32) -= 1;	mem = rCodeL(r32);	break;
	case 1:	rCodeL(r32) -= 2;	mem = rCodeL(r32);	break;
	case 2:	rCodeL(r32) -= 4;	mem = rCodeL(r32);	break;
	}
}

void TLCS900h::ExInc()
{
   uint8 data = FETCH8;
   uint8 r32 = data & 0xFC;

   cycles_extra = 3;

   switch(data & 3)
   {
      case 0:
         mem = rCodeL(r32);
         rCodeL(r32) += 1;
         break;
      case 1:
         mem = rCodeL(r32);
         rCodeL(r32) += 2;
         break;
      case 2:
         mem = rCodeL(r32);
         rCodeL(r32) += 4;
         break;
   }
}

void TLCS900h::ExRC()
{
	brCode = true;
	rCode = FETCH8;
	cycles_extra = 1;
}

//=========================================================================

//Address Mode & Register Code
TLCS900h::RegMemberFn TLCS900h::decodeExtra[256] = 
{
/*0*/	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
/*1*/	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
/*2*/	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
/*3*/	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
/*4*/	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
/*5*/	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
/*6*/	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
/*7*/	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
/*8*/	&TLCS900h::ExXWA,	&TLCS900h::ExXBC,	&TLCS900h::ExXDE,	&TLCS900h::ExXHL,	&TLCS900h::ExXIX,	&TLCS900h::ExXIY,	&TLCS900h::ExXIZ,	&TLCS900h::ExXSP,
		&TLCS900h::ExXWAd,	&TLCS900h::ExXBCd,	&TLCS900h::ExXDEd,	&TLCS900h::ExXHLd,	&TLCS900h::ExXIXd,	&TLCS900h::ExXIYd,	&TLCS900h::ExXIZd,	&TLCS900h::ExXSPd,
/*9*/	&TLCS900h::ExXWA,	&TLCS900h::ExXBC,	&TLCS900h::ExXDE,	&TLCS900h::ExXHL,	&TLCS900h::ExXIX,	&TLCS900h::ExXIY,	&TLCS900h::ExXIZ,	&TLCS900h::ExXSP,
		&TLCS900h::ExXWAd,	&TLCS900h::ExXBCd,	&TLCS900h::ExXDEd,	&TLCS900h::ExXHLd,	&TLCS900h::ExXIXd,	&TLCS900h::ExXIYd,	&TLCS900h::ExXIZd,	&TLCS900h::ExXSPd,
/*A*/	&TLCS900h::ExXWA,	&TLCS900h::ExXBC,	&TLCS900h::ExXDE,	&TLCS900h::ExXHL,	&TLCS900h::ExXIX,	&TLCS900h::ExXIY,	&TLCS900h::ExXIZ,	&TLCS900h::ExXSP,
		&TLCS900h::ExXWAd,	&TLCS900h::ExXBCd,	&TLCS900h::ExXDEd,	&TLCS900h::ExXHLd,	&TLCS900h::ExXIXd,	&TLCS900h::ExXIYd,	&TLCS900h::ExXIZd,	&TLCS900h::ExXSPd,
/*B*/	&TLCS900h::ExXWA,	&TLCS900h::ExXBC,	&TLCS900h::ExXDE,	&TLCS900h::ExXHL,	&TLCS900h::ExXIX,	&TLCS900h::ExXIY,	&TLCS900h::ExXIZ,	&TLCS900h::ExXSP,
		&TLCS900h::ExXWAd,	&TLCS900h::ExXBCd,	&TLCS900h::ExXDEd,	&TLCS900h::ExXHLd,	&TLCS900h::ExXIXd,	&TLCS900h::ExXIYd,	&TLCS900h::ExXIZd,	&TLCS900h::ExXSPd,
/*C*/	&TLCS900h::Ex8,	&TLCS900h::Ex16,	&TLCS900h::Ex24,	&TLCS900h::ExR32,	&TLCS900h::ExDec,	&TLCS900h::ExInc,	0,		&TLCS900h::ExRC,
		0,		0,		0,		0,		0,		0,		0,		0,
/*D*/	&TLCS900h::Ex8,	&TLCS900h::Ex16,	&TLCS900h::Ex24,	&TLCS900h::ExR32,	&TLCS900h::ExDec,	&TLCS900h::ExInc,	0,		&TLCS900h::ExRC,
		0,		0,		0,		0,		0,		0,		0,		0,
/*E*/	&TLCS900h::Ex8,	&TLCS900h::Ex16,	&TLCS900h::Ex24,	&TLCS900h::ExR32,	&TLCS900h::ExDec,	&TLCS900h::ExInc,	0,		&TLCS900h::ExRC,
		0,		0,		0,		0,		0,		0,		0,		0,
/*F*/	&TLCS900h::Ex8,	&TLCS900h::Ex16,	&TLCS900h::Ex24,	&TLCS900h::ExR32,	&TLCS900h::ExDec,	&TLCS900h::ExInc,	0,		0,
		0,		0,		0,		0,		0,		0,		0,		0
};

//=========================================================================

void TLCS900h::e(void)
{
	instruction_error("Unknown instruction %02X", first);
}

void TLCS900h::es(void)
{
	//instruction_error("Unknown [src] instruction %02X", second);
}

void TLCS900h::ed(void)
{
	//instruction_error("Unknown [dst] instruction %02X", second);
}

void TLCS900h::er(void)
{
	//instruction_error("Unknown [reg] instruction %02X", second);
}

//=========================================================================

//Secondary (SRC) Instruction decode
TLCS900h::RegMemberFn TLCS900h::srcDecode[256] = 
{
/*0*/	&TLCS900h::es,			&TLCS900h::es,			&TLCS900h::es,			&TLCS900h::es,			&TLCS900h::srcPUSH,	&TLCS900h::es,			&TLCS900h::srcRLD,		&TLCS900h::srcRRD,
		&TLCS900h::es,			&TLCS900h::es,			&TLCS900h::es,			&TLCS900h::es,			&TLCS900h::es,			&TLCS900h::es,			&TLCS900h::es,			&TLCS900h::es,
/*1*/	&TLCS900h::srcLDI,		&TLCS900h::srcLDIR,	&TLCS900h::srcLDD,		&TLCS900h::srcLDDR,	&TLCS900h::srcCPI,		&TLCS900h::srcCPIR,	&TLCS900h::srcCPD,		&TLCS900h::srcCPDR,
		&TLCS900h::es,			&TLCS900h::srcLD16m,	&TLCS900h::es,			&TLCS900h::es,			&TLCS900h::es,			&TLCS900h::es,			&TLCS900h::es,			&TLCS900h::es,
/*2*/	&TLCS900h::srcLD,		&TLCS900h::srcLD,		&TLCS900h::srcLD,		&TLCS900h::srcLD,		&TLCS900h::srcLD,		&TLCS900h::srcLD,		&TLCS900h::srcLD,		&TLCS900h::srcLD,
		&TLCS900h::es,			&TLCS900h::es,			&TLCS900h::es,			&TLCS900h::es,			&TLCS900h::es,			&TLCS900h::es,			&TLCS900h::es,			&TLCS900h::es,
/*3*/	&TLCS900h::srcEX,		&TLCS900h::srcEX,		&TLCS900h::srcEX,		&TLCS900h::srcEX,		&TLCS900h::srcEX,		&TLCS900h::srcEX,		&TLCS900h::srcEX,		&TLCS900h::srcEX,
		&TLCS900h::srcADDi,	&TLCS900h::srcADCi,	&TLCS900h::srcSUBi,	&TLCS900h::srcSBCi,	&TLCS900h::srcANDi,	&TLCS900h::srcXORi,	&TLCS900h::srcORi,		&TLCS900h::srcCPi,
/*4*/	&TLCS900h::srcMUL,		&TLCS900h::srcMUL,		&TLCS900h::srcMUL,		&TLCS900h::srcMUL,		&TLCS900h::srcMUL,		&TLCS900h::srcMUL,		&TLCS900h::srcMUL,		&TLCS900h::srcMUL,
		&TLCS900h::srcMULS,	&TLCS900h::srcMULS,	&TLCS900h::srcMULS,	&TLCS900h::srcMULS,	&TLCS900h::srcMULS,	&TLCS900h::srcMULS,	&TLCS900h::srcMULS,	&TLCS900h::srcMULS,
/*5*/	&TLCS900h::srcDIV,		&TLCS900h::srcDIV,		&TLCS900h::srcDIV,		&TLCS900h::srcDIV,		&TLCS900h::srcDIV,		&TLCS900h::srcDIV,		&TLCS900h::srcDIV,		&TLCS900h::srcDIV,
		&TLCS900h::srcDIVS,	&TLCS900h::srcDIVS,	&TLCS900h::srcDIVS,	&TLCS900h::srcDIVS,	&TLCS900h::srcDIVS,	&TLCS900h::srcDIVS,	&TLCS900h::srcDIVS,	&TLCS900h::srcDIVS,
/*6*/	&TLCS900h::srcINC,		&TLCS900h::srcINC,		&TLCS900h::srcINC,		&TLCS900h::srcINC,		&TLCS900h::srcINC,		&TLCS900h::srcINC,		&TLCS900h::srcINC,		&TLCS900h::srcINC,
		&TLCS900h::srcDEC,		&TLCS900h::srcDEC,		&TLCS900h::srcDEC,		&TLCS900h::srcDEC,		&TLCS900h::srcDEC,		&TLCS900h::srcDEC,		&TLCS900h::srcDEC,		&TLCS900h::srcDEC,
/*7*/	&TLCS900h::es,			&TLCS900h::es,			&TLCS900h::es,			&TLCS900h::es,			&TLCS900h::es,			&TLCS900h::es,			&TLCS900h::es,			&TLCS900h::es,
		&TLCS900h::srcRLC,		&TLCS900h::srcRRC,		&TLCS900h::srcRL,		&TLCS900h::srcRR,		&TLCS900h::srcSLA,		&TLCS900h::srcSRA,		&TLCS900h::srcSLL,		&TLCS900h::srcSRL,
/*8*/	&TLCS900h::srcADDRm,	&TLCS900h::srcADDRm,	&TLCS900h::srcADDRm,	&TLCS900h::srcADDRm,	&TLCS900h::srcADDRm,	&TLCS900h::srcADDRm,	&TLCS900h::srcADDRm,	&TLCS900h::srcADDRm,
		&TLCS900h::srcADDmR,	&TLCS900h::srcADDmR,	&TLCS900h::srcADDmR,	&TLCS900h::srcADDmR,	&TLCS900h::srcADDmR,	&TLCS900h::srcADDmR,	&TLCS900h::srcADDmR,	&TLCS900h::srcADDmR,
/*9*/	&TLCS900h::srcADCRm,	&TLCS900h::srcADCRm,	&TLCS900h::srcADCRm,	&TLCS900h::srcADCRm,	&TLCS900h::srcADCRm,	&TLCS900h::srcADCRm,	&TLCS900h::srcADCRm,	&TLCS900h::srcADCRm,
		&TLCS900h::srcADCmR,	&TLCS900h::srcADCmR,	&TLCS900h::srcADCmR,	&TLCS900h::srcADCmR,	&TLCS900h::srcADCmR,	&TLCS900h::srcADCmR,	&TLCS900h::srcADCmR,	&TLCS900h::srcADCmR,
/*A*/	&TLCS900h::srcSUBRm,	&TLCS900h::srcSUBRm,	&TLCS900h::srcSUBRm,	&TLCS900h::srcSUBRm,	&TLCS900h::srcSUBRm,	&TLCS900h::srcSUBRm,	&TLCS900h::srcSUBRm,	&TLCS900h::srcSUBRm,
		&TLCS900h::srcSUBmR,	&TLCS900h::srcSUBmR,	&TLCS900h::srcSUBmR,	&TLCS900h::srcSUBmR,	&TLCS900h::srcSUBmR,	&TLCS900h::srcSUBmR,	&TLCS900h::srcSUBmR,	&TLCS900h::srcSUBmR,
/*B*/	&TLCS900h::srcSBCRm,	&TLCS900h::srcSBCRm,	&TLCS900h::srcSBCRm,	&TLCS900h::srcSBCRm,	&TLCS900h::srcSBCRm,	&TLCS900h::srcSBCRm,	&TLCS900h::srcSBCRm,	&TLCS900h::srcSBCRm,
		&TLCS900h::srcSBCmR,	&TLCS900h::srcSBCmR,	&TLCS900h::srcSBCmR,	&TLCS900h::srcSBCmR,	&TLCS900h::srcSBCmR,	&TLCS900h::srcSBCmR,	&TLCS900h::srcSBCmR,	&TLCS900h::srcSBCmR,
/*C*/	&TLCS900h::srcANDRm,	&TLCS900h::srcANDRm,	&TLCS900h::srcANDRm,	&TLCS900h::srcANDRm,	&TLCS900h::srcANDRm,	&TLCS900h::srcANDRm,	&TLCS900h::srcANDRm,	&TLCS900h::srcANDRm,
		&TLCS900h::srcANDmR,	&TLCS900h::srcANDmR,	&TLCS900h::srcANDmR,	&TLCS900h::srcANDmR,	&TLCS900h::srcANDmR,	&TLCS900h::srcANDmR,	&TLCS900h::srcANDmR,	&TLCS900h::srcANDmR,
/*D*/	&TLCS900h::srcXORRm,	&TLCS900h::srcXORRm,	&TLCS900h::srcXORRm,	&TLCS900h::srcXORRm,	&TLCS900h::srcXORRm,	&TLCS900h::srcXORRm,	&TLCS900h::srcXORRm,	&TLCS900h::srcXORRm,
		&TLCS900h::srcXORmR,	&TLCS900h::srcXORmR,	&TLCS900h::srcXORmR,	&TLCS900h::srcXORmR,	&TLCS900h::srcXORmR,	&TLCS900h::srcXORmR,	&TLCS900h::srcXORmR,	&TLCS900h::srcXORmR,
/*E*/	&TLCS900h::srcORRm,	&TLCS900h::srcORRm,	&TLCS900h::srcORRm,	&TLCS900h::srcORRm,	&TLCS900h::srcORRm,	&TLCS900h::srcORRm,	&TLCS900h::srcORRm,	&TLCS900h::srcORRm,
		&TLCS900h::srcORmR,	&TLCS900h::srcORmR,	&TLCS900h::srcORmR,	&TLCS900h::srcORmR,	&TLCS900h::srcORmR,	&TLCS900h::srcORmR,	&TLCS900h::srcORmR,	&TLCS900h::srcORmR,
/*F*/	&TLCS900h::srcCPRm,	&TLCS900h::srcCPRm,	&TLCS900h::srcCPRm,	&TLCS900h::srcCPRm,	&TLCS900h::srcCPRm,	&TLCS900h::srcCPRm,	&TLCS900h::srcCPRm,	&TLCS900h::srcCPRm,
		&TLCS900h::srcCPmR,	&TLCS900h::srcCPmR,	&TLCS900h::srcCPmR,	&TLCS900h::srcCPmR,	&TLCS900h::srcCPmR,	&TLCS900h::srcCPmR,	&TLCS900h::srcCPmR,	&TLCS900h::srcCPmR
};

//Secondary (DST) Instruction decode
TLCS900h::RegMemberFn TLCS900h::dstDecode[256] = 
{
/*0*/	&TLCS900h::DST_dstLDBi,	&TLCS900h::ed,			&TLCS900h::DST_dstLDWi,	&TLCS900h::ed,			&TLCS900h::DST_dstPOPB,	&TLCS900h::ed,			&TLCS900h::DST_dstPOPW,	&TLCS900h::ed,
		&TLCS900h::ed,			&TLCS900h::ed,			&TLCS900h::ed,			&TLCS900h::ed,			&TLCS900h::ed,			&TLCS900h::ed,			&TLCS900h::ed,			&TLCS900h::ed,
/*1*/	&TLCS900h::ed,			&TLCS900h::ed,			&TLCS900h::ed,			&TLCS900h::ed,			&TLCS900h::DST_dstLDBm16,	&TLCS900h::ed,			&TLCS900h::DST_dstLDWm16,	&TLCS900h::ed,
		&TLCS900h::ed,			&TLCS900h::ed,			&TLCS900h::ed,			&TLCS900h::ed,			&TLCS900h::ed,			&TLCS900h::ed,			&TLCS900h::ed,			&TLCS900h::ed,
/*2*/	&TLCS900h::DST_dstLDAW,	&TLCS900h::DST_dstLDAW,	&TLCS900h::DST_dstLDAW,	&TLCS900h::DST_dstLDAW,	&TLCS900h::DST_dstLDAW,	&TLCS900h::DST_dstLDAW,	&TLCS900h::DST_dstLDAW,	&TLCS900h::DST_dstLDAW,
		&TLCS900h::DST_dstANDCFA,	&TLCS900h::DST_dstORCFA,	&TLCS900h::DST_dstXORCFA,	&TLCS900h::DST_dstLDCFA,	&TLCS900h::DST_dstSTCFA,	&TLCS900h::ed,			&TLCS900h::ed,			&TLCS900h::ed,
/*3*/	&TLCS900h::DST_dstLDAL,	&TLCS900h::DST_dstLDAL,	&TLCS900h::DST_dstLDAL,	&TLCS900h::DST_dstLDAL,	&TLCS900h::DST_dstLDAL,	&TLCS900h::DST_dstLDAL,	&TLCS900h::DST_dstLDAL,	&TLCS900h::DST_dstLDAL,
		&TLCS900h::ed,			&TLCS900h::ed,			&TLCS900h::ed,			&TLCS900h::ed,			&TLCS900h::ed,			&TLCS900h::ed,			&TLCS900h::ed,			&TLCS900h::ed,
/*4*/	&TLCS900h::DST_dstLDBR,	&TLCS900h::DST_dstLDBR,	&TLCS900h::DST_dstLDBR,	&TLCS900h::DST_dstLDBR,	&TLCS900h::DST_dstLDBR,	&TLCS900h::DST_dstLDBR,	&TLCS900h::DST_dstLDBR,	&TLCS900h::DST_dstLDBR,
		&TLCS900h::ed,			&TLCS900h::ed,			&TLCS900h::ed,			&TLCS900h::ed,			&TLCS900h::ed,			&TLCS900h::ed,			&TLCS900h::ed,			&TLCS900h::ed,
/*5*/	&TLCS900h::DST_dstLDWR,	&TLCS900h::DST_dstLDWR,	&TLCS900h::DST_dstLDWR,	&TLCS900h::DST_dstLDWR,	&TLCS900h::DST_dstLDWR,	&TLCS900h::DST_dstLDWR,	&TLCS900h::DST_dstLDWR,	&TLCS900h::DST_dstLDWR,
		&TLCS900h::ed,			&TLCS900h::ed,			&TLCS900h::ed,			&TLCS900h::ed,			&TLCS900h::ed,			&TLCS900h::ed,			&TLCS900h::ed,			&TLCS900h::ed,
/*6*/	&TLCS900h::DST_dstLDLR,	&TLCS900h::DST_dstLDLR,	&TLCS900h::DST_dstLDLR,	&TLCS900h::DST_dstLDLR,	&TLCS900h::DST_dstLDLR,	&TLCS900h::DST_dstLDLR,	&TLCS900h::DST_dstLDLR,	&TLCS900h::DST_dstLDLR,
		&TLCS900h::ed,			&TLCS900h::ed,			&TLCS900h::ed,			&TLCS900h::ed,			&TLCS900h::ed,			&TLCS900h::ed,			&TLCS900h::ed,			&TLCS900h::ed,
/*7*/	&TLCS900h::ed,			&TLCS900h::ed,			&TLCS900h::ed,			&TLCS900h::ed,			&TLCS900h::ed,			&TLCS900h::ed,			&TLCS900h::ed,			&TLCS900h::ed,
		&TLCS900h::ed,			&TLCS900h::ed,			&TLCS900h::ed,			&TLCS900h::ed,			&TLCS900h::ed,			&TLCS900h::ed,			&TLCS900h::ed,			&TLCS900h::ed,
/*8*/	&TLCS900h::DST_dstANDCF,	&TLCS900h::DST_dstANDCF,	&TLCS900h::DST_dstANDCF,	&TLCS900h::DST_dstANDCF,	&TLCS900h::DST_dstANDCF,	&TLCS900h::DST_dstANDCF,	&TLCS900h::DST_dstANDCF,	&TLCS900h::DST_dstANDCF,
		&TLCS900h::DST_dstORCF,	&TLCS900h::DST_dstORCF,	&TLCS900h::DST_dstORCF,	&TLCS900h::DST_dstORCF,	&TLCS900h::DST_dstORCF,	&TLCS900h::DST_dstORCF,	&TLCS900h::DST_dstORCF,	&TLCS900h::DST_dstORCF,
/*9*/	&TLCS900h::DST_dstXORCF,	&TLCS900h::DST_dstXORCF,	&TLCS900h::DST_dstXORCF,	&TLCS900h::DST_dstXORCF,	&TLCS900h::DST_dstXORCF,	&TLCS900h::DST_dstXORCF,	&TLCS900h::DST_dstXORCF,	&TLCS900h::DST_dstXORCF,
		&TLCS900h::DST_dstLDCF,	&TLCS900h::DST_dstLDCF,	&TLCS900h::DST_dstLDCF,	&TLCS900h::DST_dstLDCF,	&TLCS900h::DST_dstLDCF,	&TLCS900h::DST_dstLDCF,	&TLCS900h::DST_dstLDCF,	&TLCS900h::DST_dstLDCF,
/*A*/	&TLCS900h::DST_dstSTCF,	&TLCS900h::DST_dstSTCF,	&TLCS900h::DST_dstSTCF,	&TLCS900h::DST_dstSTCF,	&TLCS900h::DST_dstSTCF,	&TLCS900h::DST_dstSTCF,	&TLCS900h::DST_dstSTCF,	&TLCS900h::DST_dstSTCF,	
		&TLCS900h::DST_dstTSET,	&TLCS900h::DST_dstTSET,	&TLCS900h::DST_dstTSET,	&TLCS900h::DST_dstTSET,	&TLCS900h::DST_dstTSET,	&TLCS900h::DST_dstTSET,	&TLCS900h::DST_dstTSET,	&TLCS900h::DST_dstTSET,
/*B*/	&TLCS900h::DST_dstRES,		&TLCS900h::DST_dstRES,		&TLCS900h::DST_dstRES,		&TLCS900h::DST_dstRES,		&TLCS900h::DST_dstRES,		&TLCS900h::DST_dstRES,		&TLCS900h::DST_dstRES,		&TLCS900h::DST_dstRES,
		&TLCS900h::DST_dstSET,		&TLCS900h::DST_dstSET,		&TLCS900h::DST_dstSET,		&TLCS900h::DST_dstSET,		&TLCS900h::DST_dstSET,		&TLCS900h::DST_dstSET,		&TLCS900h::DST_dstSET,		&TLCS900h::DST_dstSET,
/*C*/	&TLCS900h::DST_dstCHG,		&TLCS900h::DST_dstCHG,		&TLCS900h::DST_dstCHG,		&TLCS900h::DST_dstCHG,		&TLCS900h::DST_dstCHG,		&TLCS900h::DST_dstCHG,		&TLCS900h::DST_dstCHG,		&TLCS900h::DST_dstCHG,
		&TLCS900h::DST_dstBIT,		&TLCS900h::DST_dstBIT,		&TLCS900h::DST_dstBIT,		&TLCS900h::DST_dstBIT,		&TLCS900h::DST_dstBIT,		&TLCS900h::DST_dstBIT,		&TLCS900h::DST_dstBIT,		&TLCS900h::DST_dstBIT,
/*D*/	&TLCS900h::DST_dstJP,		&TLCS900h::DST_dstJP,		&TLCS900h::DST_dstJP,		&TLCS900h::DST_dstJP,		&TLCS900h::DST_dstJP,		&TLCS900h::DST_dstJP,		&TLCS900h::DST_dstJP,		&TLCS900h::DST_dstJP,
		&TLCS900h::DST_dstJP,		&TLCS900h::DST_dstJP,		&TLCS900h::DST_dstJP,		&TLCS900h::DST_dstJP,		&TLCS900h::DST_dstJP,		&TLCS900h::DST_dstJP,		&TLCS900h::DST_dstJP,		&TLCS900h::DST_dstJP,
/*E*/	&TLCS900h::DST_dstCALL,	&TLCS900h::DST_dstCALL,	&TLCS900h::DST_dstCALL,	&TLCS900h::DST_dstCALL,	&TLCS900h::DST_dstCALL,	&TLCS900h::DST_dstCALL,	&TLCS900h::DST_dstCALL,	&TLCS900h::DST_dstCALL,
		&TLCS900h::DST_dstCALL,	&TLCS900h::DST_dstCALL,	&TLCS900h::DST_dstCALL,	&TLCS900h::DST_dstCALL,	&TLCS900h::DST_dstCALL,	&TLCS900h::DST_dstCALL,	&TLCS900h::DST_dstCALL,	&TLCS900h::DST_dstCALL,
/*F*/	&TLCS900h::DST_dstRET,		&TLCS900h::DST_dstRET,		&TLCS900h::DST_dstRET,		&TLCS900h::DST_dstRET,		&TLCS900h::DST_dstRET,		&TLCS900h::DST_dstRET,		&TLCS900h::DST_dstRET,		&TLCS900h::DST_dstRET,
		&TLCS900h::DST_dstRET,		&TLCS900h::DST_dstRET,		&TLCS900h::DST_dstRET,		&TLCS900h::DST_dstRET,		&TLCS900h::DST_dstRET,		&TLCS900h::DST_dstRET,		&TLCS900h::DST_dstRET,		&TLCS900h::DST_dstRET
};

//Secondary (REG) Instruction decode
TLCS900h::RegMemberFn TLCS900h::regDecode[256] =
{
/*0*/	&TLCS900h::er,			&TLCS900h::er,			&TLCS900h::er,			&TLCS900h::regLDi,		&TLCS900h::regPUSH,	&TLCS900h::regPOP,		&TLCS900h::regCPL,		&TLCS900h::regNEG,
		&TLCS900h::regMULi,	&TLCS900h::regMULSi,	&TLCS900h::regDIVi,	&TLCS900h::regDIVSi,	&TLCS900h::regLINK,	&TLCS900h::regUNLK,	&TLCS900h::regBS1F,	&TLCS900h::regBS1B,
/*1*/	&TLCS900h::regDAA,		&TLCS900h::er,			&TLCS900h::regEXTZ,	&TLCS900h::regEXTS,	&TLCS900h::regPAA,		&TLCS900h::er,			&TLCS900h::regMIRR,	&TLCS900h::er,
		&TLCS900h::er,			&TLCS900h::regMULA,	&TLCS900h::er,			&TLCS900h::er,			&TLCS900h::regDJNZ,	&TLCS900h::er,			&TLCS900h::er,			&TLCS900h::er,
/*2*/	&TLCS900h::regANDCFi,	&TLCS900h::regORCFi,	&TLCS900h::regXORCFi,	&TLCS900h::regLDCFi,	&TLCS900h::regSTCFi,	&TLCS900h::er,			&TLCS900h::er,			&TLCS900h::er,
		&TLCS900h::regANDCFA,	&TLCS900h::regORCFA,	&TLCS900h::regXORCFA,	&TLCS900h::regLDCFA,	&TLCS900h::regSTCFA,	&TLCS900h::er,			&TLCS900h::regLDCcrr,	&TLCS900h::regLDCrcr,
/*3*/	&TLCS900h::regRES,		&TLCS900h::regSET,		&TLCS900h::regCHG,		&TLCS900h::regBIT,		&TLCS900h::regTSET,	&TLCS900h::er,			&TLCS900h::er,			&TLCS900h::er,
		&TLCS900h::regMINC1,	&TLCS900h::regMINC2,	&TLCS900h::regMINC4,	&TLCS900h::er,			&TLCS900h::regMDEC1,	&TLCS900h::regMDEC2,	&TLCS900h::regMDEC4,	&TLCS900h::er,
/*4*/	&TLCS900h::regMUL,		&TLCS900h::regMUL,		&TLCS900h::regMUL,		&TLCS900h::regMUL,		&TLCS900h::regMUL,		&TLCS900h::regMUL,		&TLCS900h::regMUL,		&TLCS900h::regMUL,
		&TLCS900h::regMULS,	&TLCS900h::regMULS,	&TLCS900h::regMULS,	&TLCS900h::regMULS,	&TLCS900h::regMULS,	&TLCS900h::regMULS,	&TLCS900h::regMULS,	&TLCS900h::regMULS,
/*5*/	&TLCS900h::regDIV,		&TLCS900h::regDIV,		&TLCS900h::regDIV,		&TLCS900h::regDIV,		&TLCS900h::regDIV,		&TLCS900h::regDIV,		&TLCS900h::regDIV,		&TLCS900h::regDIV,
		&TLCS900h::regDIVS,	&TLCS900h::regDIVS,	&TLCS900h::regDIVS,	&TLCS900h::regDIVS,	&TLCS900h::regDIVS,	&TLCS900h::regDIVS,	&TLCS900h::regDIVS,	&TLCS900h::regDIVS,
/*6*/	&TLCS900h::regINC,		&TLCS900h::regINC,		&TLCS900h::regINC,		&TLCS900h::regINC,		&TLCS900h::regINC,		&TLCS900h::regINC,		&TLCS900h::regINC,		&TLCS900h::regINC,
		&TLCS900h::regDEC,		&TLCS900h::regDEC,		&TLCS900h::regDEC,		&TLCS900h::regDEC,		&TLCS900h::regDEC,		&TLCS900h::regDEC,		&TLCS900h::regDEC,		&TLCS900h::regDEC,
/*7*/	&TLCS900h::regSCC,		&TLCS900h::regSCC,		&TLCS900h::regSCC,		&TLCS900h::regSCC,		&TLCS900h::regSCC,		&TLCS900h::regSCC,		&TLCS900h::regSCC,		&TLCS900h::regSCC,
		&TLCS900h::regSCC,		&TLCS900h::regSCC,		&TLCS900h::regSCC,		&TLCS900h::regSCC,		&TLCS900h::regSCC,		&TLCS900h::regSCC,		&TLCS900h::regSCC,		&TLCS900h::regSCC,
/*8*/	&TLCS900h::regADD,		&TLCS900h::regADD,		&TLCS900h::regADD,		&TLCS900h::regADD,		&TLCS900h::regADD,		&TLCS900h::regADD,		&TLCS900h::regADD,		&TLCS900h::regADD,
		&TLCS900h::regLDRr,	&TLCS900h::regLDRr,	&TLCS900h::regLDRr,	&TLCS900h::regLDRr,	&TLCS900h::regLDRr,	&TLCS900h::regLDRr,	&TLCS900h::regLDRr,	&TLCS900h::regLDRr,
/*9*/	&TLCS900h::regADC,		&TLCS900h::regADC,		&TLCS900h::regADC,		&TLCS900h::regADC,		&TLCS900h::regADC,		&TLCS900h::regADC,		&TLCS900h::regADC,		&TLCS900h::regADC,
		&TLCS900h::regLDrR,	&TLCS900h::regLDrR,	&TLCS900h::regLDrR,	&TLCS900h::regLDrR,	&TLCS900h::regLDrR,	&TLCS900h::regLDrR,	&TLCS900h::regLDrR,	&TLCS900h::regLDrR,
/*A*/	&TLCS900h::regSUB,		&TLCS900h::regSUB,		&TLCS900h::regSUB,		&TLCS900h::regSUB,		&TLCS900h::regSUB,		&TLCS900h::regSUB,		&TLCS900h::regSUB,		&TLCS900h::regSUB,
		&TLCS900h::regLDr3,	&TLCS900h::regLDr3,	&TLCS900h::regLDr3,	&TLCS900h::regLDr3,	&TLCS900h::regLDr3,	&TLCS900h::regLDr3,	&TLCS900h::regLDr3,	&TLCS900h::regLDr3,
/*B*/	&TLCS900h::regSBC,		&TLCS900h::regSBC,		&TLCS900h::regSBC,		&TLCS900h::regSBC,		&TLCS900h::regSBC,		&TLCS900h::regSBC,		&TLCS900h::regSBC,		&TLCS900h::regSBC,
		&TLCS900h::regEX,		&TLCS900h::regEX,		&TLCS900h::regEX,		&TLCS900h::regEX,		&TLCS900h::regEX,		&TLCS900h::regEX,		&TLCS900h::regEX,		&TLCS900h::regEX,
/*C*/	&TLCS900h::regAND,		&TLCS900h::regAND,		&TLCS900h::regAND,		&TLCS900h::regAND,		&TLCS900h::regAND,		&TLCS900h::regAND,		&TLCS900h::regAND,		&TLCS900h::regAND,
		&TLCS900h::regADDi,	&TLCS900h::regADCi,	&TLCS900h::regSUBi,	&TLCS900h::regSBCi,	&TLCS900h::regANDi,	&TLCS900h::regXORi,	&TLCS900h::regORi,		&TLCS900h::regCPi,
/*D*/	&TLCS900h::regXOR,		&TLCS900h::regXOR,		&TLCS900h::regXOR,		&TLCS900h::regXOR,		&TLCS900h::regXOR,		&TLCS900h::regXOR,		&TLCS900h::regXOR,		&TLCS900h::regXOR,
		&TLCS900h::regCPr3,	&TLCS900h::regCPr3,	&TLCS900h::regCPr3,	&TLCS900h::regCPr3,	&TLCS900h::regCPr3,	&TLCS900h::regCPr3,	&TLCS900h::regCPr3,	&TLCS900h::regCPr3,
/*E*/	&TLCS900h::regOR,		&TLCS900h::regOR,		&TLCS900h::regOR,		&TLCS900h::regOR,		&TLCS900h::regOR,		&TLCS900h::regOR,		&TLCS900h::regOR,		&TLCS900h::regOR,
		&TLCS900h::regRLCi,	&TLCS900h::regRRCi,	&TLCS900h::regRLi,		&TLCS900h::regRRi,		&TLCS900h::regSLAi,	&TLCS900h::regSRAi,	&TLCS900h::regSLLi,	&TLCS900h::regSRLi,
/*F*/	&TLCS900h::regCP,		&TLCS900h::regCP,		&TLCS900h::regCP,		&TLCS900h::regCP,		&TLCS900h::regCP,		&TLCS900h::regCP,		&TLCS900h::regCP,		&TLCS900h::regCP,
		&TLCS900h::regRLCA,	&TLCS900h::regRRCA,	&TLCS900h::regRLA,		&TLCS900h::regRRA,		&TLCS900h::regSLAA,	&TLCS900h::regSRAA,	&TLCS900h::regSLLA,	&TLCS900h::regSRLA
};

//=========================================================================

void TLCS900h::src_B()
{
	second = FETCH8;			//Get the second opcode
	R = second & 7;
	size = 0;					//Byte Size

	CALL_MEMBER_FN(this, srcDecode[second])();
}

void TLCS900h::src_W()
{
	second = FETCH8;			//Get the second opcode
	R = second & 7;
	size = 1;					//Word Size

	CALL_MEMBER_FN(this, srcDecode[second])();
}

void TLCS900h::src_L()
{
	second = FETCH8;			//Get the second opcode
	R = second & 7;
	size = 2;					//Long Size

	CALL_MEMBER_FN(this, srcDecode[second])();
}

void TLCS900h::dst()
{
	second = FETCH8;			//Get the second opcode
	R = second & 7;

	CALL_MEMBER_FN(this, dstDecode[second])();
}

static uint8 rCodeConversionB[8] = { 0xE1, 0xE0, 0xE5, 0xE4, 0xE9, 0xE8, 0xED, 0xEC };
static uint8 rCodeConversionW[8] = { 0xE0, 0xE4, 0xE8, 0xEC, 0xF0, 0xF4, 0xF8, 0xFC };
static uint8 rCodeConversionL[8] = { 0xE0, 0xE4, 0xE8, 0xEC, 0xF0, 0xF4, 0xF8, 0xFC };

void TLCS900h::reg_B(void)
{
	second = FETCH8;			//Get the second opcode
	R = second & 7;
	size = 0;					//Byte Size

	if (brCode == false)
	{
		brCode = true;
		rCode = rCodeConversionB[first & 7];
	}

	CALL_MEMBER_FN(this, regDecode[second])();
}

void TLCS900h::reg_W(void)
{
	second = FETCH8;			//Get the second opcode
	R = second & 7;
	size = 1;					//Word Size

	if (brCode == false)
	{
		brCode = true;
		rCode = rCodeConversionW[first & 7];
	}

	CALL_MEMBER_FN(this, regDecode[second])();
}

void TLCS900h::reg_L(void)
{
	second = FETCH8;			//Get the second opcode
	R = second & 7;
	size = 2;					//Long Size

	if (brCode == false)
	{
		brCode = true;
		rCode = rCodeConversionL[first & 7];
	}

	CALL_MEMBER_FN(this, regDecode[second])();
}

//=============================================================================

//Primary Instruction decode
TLCS900h::RegMemberFn TLCS900h::decode[256] = 
{
/*0*/	&TLCS900h::sngNOP,		&TLCS900h::sngNORMAL,	&TLCS900h::sngPUSHSR,	&TLCS900h::sngPOPSR,	&TLCS900h::sngMAX,		&TLCS900h::sngHALT,	&TLCS900h::sngEI,		&TLCS900h::sngRETI,
		&TLCS900h::sngLD8_8,	&TLCS900h::sngPUSH8,	&TLCS900h::sngLD8_16,	&TLCS900h::sngPUSH16,	&TLCS900h::sngINCF,	&TLCS900h::sngDECF,	&TLCS900h::sngRET,		&TLCS900h::sngRETD,
/*1*/	&TLCS900h::sngRCF,		&TLCS900h::sngSCF,		&TLCS900h::sngCCF,		&TLCS900h::sngZCF,		&TLCS900h::sngPUSHA,	&TLCS900h::sngPOPA,	&TLCS900h::sngEX,		&TLCS900h::sngLDF,
		&TLCS900h::sngPUSHF,	&TLCS900h::sngPOPF,	&TLCS900h::sngJP16,	&TLCS900h::sngJP24,	&TLCS900h::sngCALL16,	&TLCS900h::sngCALL24,	&TLCS900h::sngCALR,	&TLCS900h::iBIOSHLE,
/*2*/	&TLCS900h::sngLDB,		&TLCS900h::sngLDB,		&TLCS900h::sngLDB,		&TLCS900h::sngLDB,		&TLCS900h::sngLDB,		&TLCS900h::sngLDB,		&TLCS900h::sngLDB,		&TLCS900h::sngLDB,
		&TLCS900h::sngPUSHW,	&TLCS900h::sngPUSHW,	&TLCS900h::sngPUSHW,	&TLCS900h::sngPUSHW,	&TLCS900h::sngPUSHW,	&TLCS900h::sngPUSHW,	&TLCS900h::sngPUSHW,	&TLCS900h::sngPUSHW,
/*3*/	&TLCS900h::sngLDW,		&TLCS900h::sngLDW,		&TLCS900h::sngLDW,		&TLCS900h::sngLDW,		&TLCS900h::sngLDW,		&TLCS900h::sngLDW,		&TLCS900h::sngLDW,		&TLCS900h::sngLDW,
		&TLCS900h::sngPUSHL,	&TLCS900h::sngPUSHL,	&TLCS900h::sngPUSHL,	&TLCS900h::sngPUSHL,	&TLCS900h::sngPUSHL,	&TLCS900h::sngPUSHL,	&TLCS900h::sngPUSHL,	&TLCS900h::sngPUSHL,
/*4*/	&TLCS900h::sngLDL,		&TLCS900h::sngLDL,		&TLCS900h::sngLDL,		&TLCS900h::sngLDL,		&TLCS900h::sngLDL,		&TLCS900h::sngLDL,		&TLCS900h::sngLDL,		&TLCS900h::sngLDL,
		&TLCS900h::sngPOPW,	&TLCS900h::sngPOPW,	&TLCS900h::sngPOPW,	&TLCS900h::sngPOPW,	&TLCS900h::sngPOPW,	&TLCS900h::sngPOPW,	&TLCS900h::sngPOPW,	&TLCS900h::sngPOPW,
/*5*/	&TLCS900h::e,			&TLCS900h::e,			&TLCS900h::e,			&TLCS900h::e,			&TLCS900h::e,			&TLCS900h::e,			&TLCS900h::e,			&TLCS900h::e,
		&TLCS900h::sngPOPL,	&TLCS900h::sngPOPL,	&TLCS900h::sngPOPL,	&TLCS900h::sngPOPL,	&TLCS900h::sngPOPL,	&TLCS900h::sngPOPL,	&TLCS900h::sngPOPL,	&TLCS900h::sngPOPL,
/*6*/	&TLCS900h::sngJR,		&TLCS900h::sngJR,		&TLCS900h::sngJR,		&TLCS900h::sngJR,		&TLCS900h::sngJR,		&TLCS900h::sngJR,		&TLCS900h::sngJR,		&TLCS900h::sngJR,
		&TLCS900h::sngJR,		&TLCS900h::sngJR,		&TLCS900h::sngJR,		&TLCS900h::sngJR,		&TLCS900h::sngJR,		&TLCS900h::sngJR,		&TLCS900h::sngJR,		&TLCS900h::sngJR,
/*7*/	&TLCS900h::sngJRL,		&TLCS900h::sngJRL,		&TLCS900h::sngJRL,		&TLCS900h::sngJRL,		&TLCS900h::sngJRL,		&TLCS900h::sngJRL,		&TLCS900h::sngJRL,		&TLCS900h::sngJRL,
		&TLCS900h::sngJRL,		&TLCS900h::sngJRL,		&TLCS900h::sngJRL,		&TLCS900h::sngJRL,		&TLCS900h::sngJRL,		&TLCS900h::sngJRL,		&TLCS900h::sngJRL,		&TLCS900h::sngJRL,
/*8*/	&TLCS900h::src_B,		&TLCS900h::src_B,		&TLCS900h::src_B,		&TLCS900h::src_B,		&TLCS900h::src_B,		&TLCS900h::src_B,		&TLCS900h::src_B,		&TLCS900h::src_B,
		&TLCS900h::src_B,		&TLCS900h::src_B,		&TLCS900h::src_B,		&TLCS900h::src_B,		&TLCS900h::src_B,		&TLCS900h::src_B,		&TLCS900h::src_B,		&TLCS900h::src_B,
/*9*/	&TLCS900h::src_W,		&TLCS900h::src_W,		&TLCS900h::src_W,		&TLCS900h::src_W,		&TLCS900h::src_W,		&TLCS900h::src_W,		&TLCS900h::src_W,		&TLCS900h::src_W,
		&TLCS900h::src_W,		&TLCS900h::src_W,		&TLCS900h::src_W,		&TLCS900h::src_W,		&TLCS900h::src_W,		&TLCS900h::src_W,		&TLCS900h::src_W,		&TLCS900h::src_W,
/*A*/	&TLCS900h::src_L,		&TLCS900h::src_L,		&TLCS900h::src_L,		&TLCS900h::src_L,		&TLCS900h::src_L,		&TLCS900h::src_L,		&TLCS900h::src_L,		&TLCS900h::src_L,
		&TLCS900h::src_L,		&TLCS900h::src_L,		&TLCS900h::src_L,		&TLCS900h::src_L,		&TLCS900h::src_L,		&TLCS900h::src_L,		&TLCS900h::src_L,		&TLCS900h::src_L,
/*B*/	&TLCS900h::dst,		&TLCS900h::dst,		&TLCS900h::dst,		&TLCS900h::dst,		&TLCS900h::dst,		&TLCS900h::dst,		&TLCS900h::dst,		&TLCS900h::dst,
		&TLCS900h::dst,		&TLCS900h::dst,		&TLCS900h::dst,		&TLCS900h::dst,		&TLCS900h::dst,		&TLCS900h::dst,		&TLCS900h::dst,		&TLCS900h::dst,
/*C*/	&TLCS900h::src_B,		&TLCS900h::src_B,		&TLCS900h::src_B,		&TLCS900h::src_B,		&TLCS900h::src_B,		&TLCS900h::src_B,		&TLCS900h::e,			&TLCS900h::reg_B,
		&TLCS900h::reg_B,		&TLCS900h::reg_B,		&TLCS900h::reg_B,		&TLCS900h::reg_B,		&TLCS900h::reg_B,		&TLCS900h::reg_B,		&TLCS900h::reg_B,		&TLCS900h::reg_B,
/*D*/	&TLCS900h::src_W,		&TLCS900h::src_W,		&TLCS900h::src_W,		&TLCS900h::src_W,		&TLCS900h::src_W,		&TLCS900h::src_W,		&TLCS900h::e,			&TLCS900h::reg_W,
		&TLCS900h::reg_W,		&TLCS900h::reg_W,		&TLCS900h::reg_W,		&TLCS900h::reg_W,		&TLCS900h::reg_W,		&TLCS900h::reg_W,		&TLCS900h::reg_W,		&TLCS900h::reg_W,
/*E*/	&TLCS900h::src_L,		&TLCS900h::src_L,		&TLCS900h::src_L,		&TLCS900h::src_L,		&TLCS900h::src_L,		&TLCS900h::src_L,		&TLCS900h::e,			&TLCS900h::reg_L,
		&TLCS900h::reg_L,		&TLCS900h::reg_L,		&TLCS900h::reg_L,		&TLCS900h::reg_L,		&TLCS900h::reg_L,		&TLCS900h::reg_L,		&TLCS900h::reg_L,		&TLCS900h::reg_L,
/*F*/	&TLCS900h::dst,		&TLCS900h::dst,		&TLCS900h::dst,		&TLCS900h::dst,		&TLCS900h::dst,		&TLCS900h::dst,		&TLCS900h::e,			&TLCS900h::sngLDX,
		&TLCS900h::sngSWI,		&TLCS900h::sngSWI,		&TLCS900h::sngSWI,		&TLCS900h::sngSWI,		&TLCS900h::sngSWI,		&TLCS900h::sngSWI,		&TLCS900h::sngSWI,		&TLCS900h::sngSWI
};

//=============================================================================

int32 TLCS900h::TLCS900h_interpret(void)
{
	brCode = false;

	first = FETCH8;	//Get the first byte

	//Is any extra data used by this instruction?
	cycles_extra = 0;
	if (decodeExtra[first])
		CALL_MEMBER_FN(this, decodeExtra[first])();

	CALL_MEMBER_FN(this, decode[first])();

	return cycles + cycles_extra;
}

//=============================================================================
