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

#include "neopop.h"
#include "../../duo/duo_instance.h"
#include "mem.h"
#include "sound.h"
#include "Z80_interface.h"
#include "TLCS-900h/TLCS900h_registers.h"
#include "interrupt.h"
#include "dma.h"
#include "../hw_cpu/z80-fuse/z80.h"
#include "../hw_cpu/z80-fuse/z80_macros.h"

#include "../state.h"

uint8_t neopop_z80i_t::Z80_ReadComm(void)
{
   return CommByte;
}

void neopop_z80i_t::Z80_WriteComm(uint8_t data)
{
   CommByte = data;
}

static uint8_t NGP_z80_readbyte(uint16_t address)
{
   DuoInstance *duo = DuoInstance::currentInstance;

   if (address <= 0xFFF)
      return duo->mem->loadB(0x7000 + address);

   switch (address)
   {
      case 0x8000:
         return duo->z80i->CommByte;
      default:
         break;
   }

   return 0;
}

static void NGP_z80_writebyte(uint16_t address, uint8_t value)
{
   DuoInstance *duo = DuoInstance::currentInstance;

   if (address <= 0x0FFF)
   {
      duo->mem->storeB(0x7000 + address, value);
      return;
   }

   switch (address)
   {
      case 0x8000:
          duo->z80i->CommByte = value;
         break;
      case 0x4001:
         duo->sound->Write_SoundChipLeft(value);
         break;
      case 0x4000:
         duo->sound->Write_SoundChipRight(value);
         break;
      case 0xC000:
          duo->interrupt->TestIntHDMA(6, 0x0C);
         break;
   }

}

static void NGP_z80_writeport(uint16_t port, uint8_t value)
{
	//printf("Portout: %04x %02x\n", port, value);
	z80_set_interrupt(0);
}

static uint8_t NGP_z80_readport(uint16_t port)
{
	//printf("Portin: %04x\n", port);
	return 0;
}

void neopop_z80i_t::Z80_nmi(void)
{
	z80_nmi();
}

void neopop_z80i_t::Z80_irq(void)
{
	z80_set_interrupt(1);
}

void neopop_z80i_t::Z80_reset(void)
{
	Z80Enabled = 0;

	z80_writebyte = NGP_z80_writebyte;
	z80_readbyte = NGP_z80_readbyte;
	z80_writeport = NGP_z80_writeport;
	z80_readport = NGP_z80_readport;

	z80_init();
	z80_reset();
}

void neopop_z80i_t::Z80_SetEnable(bool set)
{
   Z80Enabled = set;
   if(!set)
      z80_reset();
}

bool neopop_z80i_t::Z80_IsEnabled(void)
{
   return(Z80Enabled);
}

int neopop_z80i_t::Z80_RunOP(void)
{
   if(!Z80Enabled)
      return -1;

   return(z80_do_opcode());
}

// ------------
// State Action
// ------------

int z80_state_action(void *data, int load, int data_only, const char *section_name)
{
   uint8_t r_register;

   SFORMAT StateRegs[] =
   {
      { &(cur_z80->z80.af.w), sizeof(cur_z80->z80.af.w), 0x80000000, "AF" },
      { &(cur_z80->z80.bc.w), sizeof(cur_z80->z80.bc.w), 0x80000000, "BC" },
      { &(cur_z80->z80.de.w), sizeof(cur_z80->z80.de.w), 0x80000000, "DE" },
      { &(cur_z80->z80.hl.w), sizeof(cur_z80->z80.hl.w), 0x80000000, "HL" },
      { &(cur_z80->z80.af_.w), sizeof(cur_z80->z80.af_.w), 0x80000000, "AF_" },
      { &(cur_z80->z80.bc_.w), sizeof(cur_z80->z80.bc_.w), 0x80000000, "BC_" },
      { &(cur_z80->z80.de_.w), sizeof(cur_z80->z80.de_.w), 0x80000000, "DE_" },
      { &(cur_z80->z80.hl_.w), sizeof(cur_z80->z80.hl_.w), 0x80000000, "HL_" },
      { &(cur_z80->z80.ix.w), sizeof(cur_z80->z80.ix.w), 0x80000000, "IX" },
      { &(cur_z80->z80.iy.w), sizeof(cur_z80->z80.iy.w), 0x80000000, "IY" },
      { &(cur_z80->z80.i), sizeof(cur_z80->z80.i), 0x80000000, "I" },
      { &(cur_z80->z80.sp.w), sizeof(cur_z80->z80.sp.w), 0x80000000, "SP" },
      { &(cur_z80->z80.pc.w), sizeof(cur_z80->z80.pc.w), 0x80000000, "PC" },
      { &(cur_z80->z80.iff1), sizeof(cur_z80->z80.iff1), 0x80000000, "IFF1" },
      { &(cur_z80->z80.iff2), sizeof(cur_z80->z80.iff2), 0x80000000, "IFF2" },
      { &(cur_z80->z80.im), sizeof(cur_z80->z80.im), 0x80000000, "IM" },
      { &(r_register), sizeof(r_register), 0x80000000, "R" },

      { &(cur_z80->z80.interrupts_enabled_at), sizeof(cur_z80->z80.interrupts_enabled_at), 0x80000000, "interrupts_enabled_at" },
      { &(cur_z80->z80.halted), sizeof(cur_z80->z80.halted), 0x80000000, "halted" },

      { &((cur_z80->z80_tstates)), sizeof((cur_z80->z80_tstates)), 0x80000000, "z80_tstates" },
      { &((cur_z80->last_z80_tstates)), sizeof((cur_z80->last_z80_tstates)), 0x80000000, "last_z80_tstates" },

      { 0, 0, 0, 0 }
   };

   if(!load)
      r_register = (cur_z80->z80.r7 & 0x80) | (cur_z80->z80.r & 0x7f);

   if(!MDFNSS_StateAction(data, load, data_only, StateRegs, section_name, false))
      return(0);

   if(load)
   {
       cur_z80->z80.r7 = r_register & 0x80;
       cur_z80->z80.r = r_register & 0x7F;
   }

   return(1);
}

int MDFNNGPCZ80_StateAction(void *data, int load, int data_only)
{
   // TODO Where does z80i come from in this scope?
   neopop_z80i_t *z80i = NULL;

   SFORMAT StateRegs[] =
   {
      SFVAR(z80i->CommByte),
      SFVAR(z80i->Z80Enabled),
      SFEND
   };

   if(!MDFNSS_StateAction(data, load, data_only, StateRegs, "Z80X", false))
      return 0;

   if(!z80_state_action(data, load, data_only, "Z80"))
      return 0;

   return 1;
}
