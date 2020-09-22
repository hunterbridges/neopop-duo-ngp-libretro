/* z80_macros.h: Some commonly used z80 things as macros
   Copyright (c) 1999-2011 Philip Kendall

   $Id: z80_macros.h 4624 2012-01-09 20:59:35Z pak21 $

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License along
   with this program; if not, write to the Free Software Foundation, Inc.,
   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

   Author contact information:

   E-mail: philip-fuse@shadowmagic.org.uk

*/

#ifndef FUSE_Z80_MACROS_H
#define FUSE_Z80_MACROS_H

#include <stdint.h>

/* Macros used for accessing the registers */
#define A   cur_z80->z80.af.b.h
#define F   cur_z80->z80.af.b.l
#define AF  cur_z80->z80.af.w

#define B   cur_z80->z80.bc.b.h
#define C   cur_z80->z80.bc.b.l
#define BC  cur_z80->z80.bc.w

#define D   cur_z80->z80.de.b.h
#define E   cur_z80->z80.de.b.l
#define DE  cur_z80->z80.de.w

#define H   cur_z80->z80.hl.b.h
#define L   cur_z80->z80.hl.b.l
#define HL  cur_z80->z80.hl.w

#define A_  cur_z80->z80.af_.b.h
#define F_  cur_z80->z80.af_.b.l
#define AF_ cur_z80->z80.af_.w

#define B_  cur_z80->z80.bc_.b.h
#define C_  cur_z80->z80.bc_.b.l
#define BC_ cur_z80->z80.bc_.w

#define D_  cur_z80->z80.de_.b.h
#define E_  cur_z80->z80.de_.b.l
#define DE_ cur_z80->z80.de_.w

#define H_  cur_z80->z80.hl_.b.h
#define L_  cur_z80->z80.hl_.b.l
#define HL_ cur_z80->z80.hl_.w

#define IXH cur_z80->z80.ix.b.h
#define IXL cur_z80->z80.ix.b.l
#define IX  cur_z80->z80.ix.w

#define IYH cur_z80->z80.iy.b.h
#define IYL cur_z80->z80.iy.b.l
#define IY  cur_z80->z80.iy.w

#define SPH cur_z80->z80.sp.b.h
#define SPL cur_z80->z80.sp.b.l
#define SP  cur_z80->z80.sp.w

#define PCH cur_z80->z80.pc.b.h
#define PCL cur_z80->z80.pc.b.l
#define PC  cur_z80->z80.pc.w

#define I  cur_z80->z80.i
#define R  cur_z80->z80.r
#define R7 cur_z80->z80.r7

#define IFF1 cur_z80->z80.iff1
#define IFF2 cur_z80->z80.iff2
#define IM   cur_z80->z80.im

#define IR ( ( cur_z80->z80.i ) << 8 | ( cur_z80->z80.r7 & 0x80 ) | ( cur_z80->z80.r & 0x7f ) )

/* The flags */

#define Z80_FLAG_C	0x01
#define Z80_FLAG_N	0x02
#define Z80_FLAG_P	0x04
#define Z80_FLAG_V	Z80_FLAG_P
#define Z80_FLAG_3	0x08
#define Z80_FLAG_H	0x10
#define Z80_FLAG_5	0x20
#define Z80_FLAG_Z	0x40
#define Z80_FLAG_S	0x80

/* Get the appropriate contended memory delay. Use a macro for performance
   reasons in the main core, but a function for flexibility when building
   the core tester */

#define contend_read(address,time)          cur_z80->z80_tstates += (time);

#define contend_read_no_mreq(address,time)  cur_z80->z80_tstates += (time);

#define contend_write_no_mreq(address,time) cur_z80->z80_tstates += (time);

/* Some commonly used instructions */
#define AND(value)\
{\
  A &= (value);\
  F = Z80_FLAG_H | sz53p_table[A];\
}

#define ADC(value)\
{\
  uint16_t adctemp = A + (value) + ( F & Z80_FLAG_C ); \
  uint8_t lookup = ( (       A & 0x88 ) >> 3 ) | \
			    ( ( (value) & 0x88 ) >> 2 ) | \
			    ( ( adctemp & 0x88 ) >> 1 );  \
  A=adctemp;\
  F = ( adctemp & 0x100 ? Z80_FLAG_C : 0 ) |\
    halfcarry_add_table[lookup & 0x07] | overflow_add_table[lookup >> 4] |\
    sz53_table[A];\
}

#define ADC16(value)\
{\
  uint32_t add16temp= HL + (value) + ( F & Z80_FLAG_C ); \
  uint8_t lookup = ( (        HL & 0x8800 ) >> 11 ) | \
			    ( (   (value) & 0x8800 ) >> 10 ) | \
			    ( ( add16temp & 0x8800 ) >>  9 );  \
  HL = add16temp;\
  F = ( add16temp & 0x10000 ? Z80_FLAG_C : 0 )|\
    overflow_add_table[lookup >> 4] |\
    ( H & ( Z80_FLAG_3 | Z80_FLAG_5 | Z80_FLAG_S ) ) |\
    halfcarry_add_table[lookup&0x07]|\
    ( HL ? 0 : Z80_FLAG_Z );\
}

#define ADD(value)\
{\
  uint16_t addtemp = A + (value); \
  uint8_t lookup = ( (       A & 0x88 ) >> 3 ) | \
			    ( ( (value) & 0x88 ) >> 2 ) | \
			    ( ( addtemp & 0x88 ) >> 1 );  \
  A=addtemp;\
  F = ( addtemp & 0x100 ? Z80_FLAG_C : 0 ) |\
    halfcarry_add_table[lookup & 0x07] | overflow_add_table[lookup >> 4] |\
    sz53_table[A];\
}

#define ADD16(value1,value2)\
{\
  uint32_t add16temp = (value1) + (value2); \
  uint8_t lookup = ( (  (value1) & 0x0800 ) >> 11 ) | \
			    ( (  (value2) & 0x0800 ) >> 10 ) | \
			    ( ( add16temp & 0x0800 ) >>  9 );  \
  (value1) = add16temp;\
  F = ( F & ( Z80_FLAG_V | Z80_FLAG_Z | Z80_FLAG_S ) ) |\
    ( add16temp & 0x10000 ? Z80_FLAG_C : 0 )|\
    ( ( add16temp >> 8 ) & ( Z80_FLAG_3 | Z80_FLAG_5 ) ) |\
    halfcarry_add_table[lookup];\
}

/* This may look fairly inefficient, but the (gcc) optimiser does the
   right thing assuming it's given a constant for 'bit' */
#define BIT( bit, value ) \
{ \
  F = ( F & Z80_FLAG_C ) | Z80_FLAG_H | ( value & ( Z80_FLAG_3 | Z80_FLAG_5 ) ); \
  if( ! ( (value) & ( 0x01 << (bit) ) ) ) F |= Z80_FLAG_P | Z80_FLAG_Z; \
  if( (bit) == 7 && (value) & 0x80 ) F |= Z80_FLAG_S; \
}

#define BIT_I( bit, value, address ) \
{ \
  F = ( F & Z80_FLAG_C ) | Z80_FLAG_H | ( ( address >> 8 ) & ( Z80_FLAG_3 | Z80_FLAG_5 ) ); \
  if( ! ( (value) & ( 0x01 << (bit) ) ) ) F |= Z80_FLAG_P | Z80_FLAG_Z; \
  if( (bit) == 7 && (value) & 0x80 ) F |= Z80_FLAG_S; \
}  

#define CALL()\
{\
  uint8_t calltempl, calltemph; \
  calltempl=Z80_RB_MACRO(PC++);\
  calltemph=Z80_RB_MACRO( PC ); \
  contend_read_no_mreq( PC, 1 ); PC++;\
  PUSH16(PCL,PCH);\
  PCL=calltempl; PCH=calltemph;\
}

#define CP(value)\
{\
  uint16_t cptemp = A - value; \
  uint8_t lookup = ( (       A & 0x88 ) >> 3 ) | \
			    ( ( (value) & 0x88 ) >> 2 ) | \
			    ( (  cptemp & 0x88 ) >> 1 );  \
  F = ( cptemp & 0x100 ? Z80_FLAG_C : ( cptemp ? 0 : Z80_FLAG_Z ) ) | Z80_FLAG_N |\
    halfcarry_sub_table[lookup & 0x07] |\
    overflow_sub_table[lookup >> 4] |\
    ( value & ( Z80_FLAG_3 | Z80_FLAG_5 ) ) |\
    ( cptemp & Z80_FLAG_S );\
}

/* Macro for the {DD,FD} CB dd xx rotate/shift instructions */
#define DDFDCB_ROTATESHIFT(time, target, instruction)\
z80_tstates+=(time);\
{\
  (target) = Z80_RB_MACRO( tempaddr );\
  instruction( (target) );\
  Z80_WB_MACRO( tempaddr, (target) );\
}\
break

#define DEC(value)\
{\
  F = ( F & Z80_FLAG_C ) | ( (value)&0x0f ? 0 : Z80_FLAG_H ) | Z80_FLAG_N;\
  (value)--;\
  F |= ( (value)==0x7f ? Z80_FLAG_V : 0 ) | sz53_table[value];\
}

#define Z80_IN( reg, port )\
{\
  (reg)=Z80_RP_MACRO((port));\
  F = ( F & Z80_FLAG_C) | sz53p_table[(reg)];\
}

#define INC(value)\
{\
  (value)++;\
  F = ( F & Z80_FLAG_C ) | ( (value)==0x80 ? Z80_FLAG_V : 0 ) |\
  ( (value)&0x0f ? 0 : Z80_FLAG_H ) | sz53_table[(value)];\
}

#define LD16_NNRR(regl,regh)\
{\
  uint16_t ldtemp; \
  ldtemp=Z80_RB_MACRO(PC++);\
  ldtemp|=Z80_RB_MACRO(PC++) << 8;\
  Z80_WB_MACRO(ldtemp++,(regl));\
  Z80_WB_MACRO(ldtemp,(regh));\
  break;\
}

#define LD16_RRNN(regl,regh)\
{\
  uint16_t ldtemp; \
  ldtemp=Z80_RB_MACRO(PC++);\
  ldtemp|=Z80_RB_MACRO(PC++) << 8;\
  (regl)=Z80_RB_MACRO(ldtemp++);\
  (regh)=Z80_RB_MACRO(ldtemp);\
  break;\
}

#define JP()\
{\
  uint16_t jptemp=PC; \
  PCL=Z80_RB_MACRO(jptemp++);\
  PCH=Z80_RB_MACRO(jptemp);\
}

#define JR()\
{\
  int8 jrtemp = Z80_RB_MACRO( PC ); \
  contend_read_no_mreq( PC, 1 ); contend_read_no_mreq( PC, 1 ); \
  contend_read_no_mreq( PC, 1 ); contend_read_no_mreq( PC, 1 ); \
  contend_read_no_mreq( PC, 1 ); \
  PC += jrtemp; \
}

#define OR(value)\
{\
  A |= (value);\
  F = sz53p_table[A];\
}

#define POP16(regl,regh)\
{\
  (regl)=Z80_RB_MACRO(SP++);\
  (regh)=Z80_RB_MACRO(SP++);\
}

#define PUSH16(regl,regh)\
{\
  Z80_WB_MACRO( --SP, (regh) );\
  Z80_WB_MACRO( --SP, (regl) );\
}

#define RET()\
{\
  POP16(PCL,PCH);\
}

#define RL(value)\
{\
  uint8_t rltemp = (value); \
  (value) = ( (value)<<1 ) | ( F & Z80_FLAG_C );\
  F = ( rltemp >> 7 ) | sz53p_table[(value)];\
}

#define RLC(value)\
{\
  (value) = ( (value)<<1 ) | ( (value)>>7 );\
  F = ( (value) & Z80_FLAG_C ) | sz53p_table[(value)];\
}

#define RR(value)\
{\
  uint8_t rrtemp = (value); \
  (value) = ( (value)>>1 ) | ( F << 7 );\
  F = ( rrtemp & Z80_FLAG_C ) | sz53p_table[(value)];\
}

#define RRC(value)\
{\
  F = (value) & Z80_FLAG_C;\
  (value) = ( (value)>>1 ) | ( (value)<<7 );\
  F |= sz53p_table[(value)];\
}

#define RST(value)\
{\
  PUSH16(PCL,PCH);\
  PC=(value);\
}

#define SBC(value)\
{\
  uint16_t sbctemp = A - (value) - ( F & Z80_FLAG_C ); \
  uint8_t lookup = ( (       A & 0x88 ) >> 3 ) | \
			    ( ( (value) & 0x88 ) >> 2 ) | \
			    ( ( sbctemp & 0x88 ) >> 1 );  \
  A=sbctemp;\
  F = ( sbctemp & 0x100 ? Z80_FLAG_C : 0 ) | Z80_FLAG_N |\
    halfcarry_sub_table[lookup & 0x07] | overflow_sub_table[lookup >> 4] |\
    sz53_table[A];\
}

#define SBC16(value)\
{\
  uint32_t sub16temp = HL - (value) - (F & Z80_FLAG_C); \
  uint8_t lookup = ( (        HL & 0x8800 ) >> 11 ) | \
			    ( (   (value) & 0x8800 ) >> 10 ) | \
			    ( ( sub16temp & 0x8800 ) >>  9 );  \
  HL = sub16temp;\
  F = ( sub16temp & 0x10000 ? Z80_FLAG_C : 0 ) |\
    Z80_FLAG_N | overflow_sub_table[lookup >> 4] |\
    ( H & ( Z80_FLAG_3 | Z80_FLAG_5 | Z80_FLAG_S ) ) |\
    halfcarry_sub_table[lookup&0x07] |\
    ( HL ? 0 : Z80_FLAG_Z) ;\
}

#define SLA(value)\
{\
  F = (value) >> 7;\
  (value) <<= 1;\
  F |= sz53p_table[(value)];\
}

#define SLL(value)\
{\
  F = (value) >> 7;\
  (value) = ( (value) << 1 ) | 0x01;\
  F |= sz53p_table[(value)];\
}

#define SRA(value)\
{\
  F = (value) & Z80_FLAG_C;\
  (value) = ( (value) & 0x80 ) | ( (value) >> 1 );\
  F |= sz53p_table[(value)];\
}

#define SRL(value)\
{\
  F = (value) & Z80_FLAG_C;\
  (value) >>= 1;\
  F |= sz53p_table[(value)];\
}

#define SUB(value)\
{\
  uint16_t subtemp = A - (value); \
  uint8_t lookup = ( (       A & 0x88 ) >> 3 ) | \
			    ( ( (value) & 0x88 ) >> 2 ) | \
			    (  (subtemp & 0x88 ) >> 1 );  \
  A=subtemp;\
  F = ( subtemp & 0x100 ? Z80_FLAG_C : 0 ) | Z80_FLAG_N |\
    halfcarry_sub_table[lookup & 0x07] | overflow_sub_table[lookup >> 4] |\
    sz53_table[A];\
}

#define XOR(value)\
{\
  A ^= (value);\
  F = sz53p_table[A];\
}

#endif		/* #ifndef FUSE_Z80_MACROS_H */
