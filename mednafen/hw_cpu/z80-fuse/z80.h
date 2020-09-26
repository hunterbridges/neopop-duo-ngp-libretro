/* z80.h: z80 emulation core
   Copyright (c) 1999-2003 Philip Kendall

   $Id: z80.h 4640 2012-01-21 13:26:35Z pak21 $

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

#ifndef FUSE_Z80_H
#define FUSE_Z80_H

#include <stdint.h>

/* Union allowing a register pair to be accessed as bytes or as a word */
typedef union {
#ifdef MSB_FIRST
  struct { uint8_t h,l; } b;
#else
  struct { uint8_t l,h; } b;
#endif
  uint16_t w;
} regpair;

/* What's stored in the main processor */
typedef struct processor {
  regpair af,bc,de,hl;
  regpair af_,bc_,de_,hl_;
  regpair ix,iy;
  uint8_t i;
  uint16_t r;	/* The low seven bits of the R register. 16 bits long
			   so it can also act as an RZX instruction counter */
  uint8_t r7;	/* The high bit of the R register */
  regpair sp,pc;
  uint8_t iff1, iff2, im;
  int halted;

  /* Interrupts were enabled at this time; do not accept any interrupts
     until z80_tstates > this value */
  uint64_t interrupts_enabled_at;

} processor;

extern const uint8_t overflow_add_table[];
extern const uint8_t overflow_sub_table[];
extern const uint8_t halfcarry_add_table[];
extern const uint8_t halfcarry_sub_table[];
extern void (*z80_writebyte)(uint16_t a, uint8_t b);
extern uint8_t (*z80_readbyte)(uint16_t a);
extern void (*z80_writeport)(uint16_t a, uint8_t b);
extern uint8_t (*z80_readport)(uint16_t a);

struct Z80
{
	/* This is what everything acts on! */
	struct processor z80;

	uint64_t last_z80_tstates;
	uint64_t z80_tstates;
	int iline;

	uint8_t sz53_table[0x100]; /* The S, Z, 5 and 3 bits of the index */
	uint8_t parity_table[0x100]; /* The parity of the lookup value */
	uint8_t sz53p_table[0x100]; /* OR the above two tables together */

	void z80_init( void );
	void z80_reset( void );

	void z80_nmi( void );

	void z80_set_interrupt(int set);

	int z80_interrupt( void );

	int z80_do_opcode(void);

	void z80_enable_interrupts( void );

	inline uint16_t z80_getpc(void) { return z80.pc.w; }

	// Ok, I lied, not a macro!

	//Write mem
	inline void Z80_WB_MACRO(uint16_t A, uint8_t V)
	{ 
	 z80_tstates += 3; 
	 z80_writebyte(A, V); 
	}

	// Write port
	inline void Z80_WP_MACRO(uint16_t A, uint8_t V)
	{ 
	 z80_tstates += 4; 
	 z80_writeport(A, V); 
	}

	// Read mem
	inline uint8_t Z80_RB_MACRO(uint16_t A)
	{ 
	 z80_tstates += 3; 
	 return(z80_readbyte(A));
	}

	// Read port
	inline uint8_t Z80_RP_MACRO(uint16_t A)
	{ 
	 z80_tstates += 4; 
	 return(z80_readport(A));
	}

};

#endif			/* #ifndef FUSE_Z80_H */
