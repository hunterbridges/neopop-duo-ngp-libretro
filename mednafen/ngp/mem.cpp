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
#include "TLCS-900h/TLCS900h.h"
#include "Z80_interface.h"
#include "bios.h"
#include "gfx.h"
#include "mem.h"
#include "interrupt.h"
#include "sound.h"
#include "flash.h"
#include "rtc.h"

#ifdef MSB_FIRST
#include "../masmem.h"
#endif

#include "../settings.h"
#include "../../duo/duo_instance.h"

bool debug_abort_memory = false;
bool debug_mask_memory_error_messages = false;

/* Call this function after ROM is loaded */
void neopop_mem_t::SetFRM(void) 
{
   DuoInstance *duo = GetDuoFromModule(this, mem);

   unsigned int x;

   for(x = 0; x < 256; x++)
      FastReadMapReal[x] = NULL;

   for(x = 0x20; x <= 0x3f; x++)
   {
      if(duo->rom->ngpc_rom.length > (x * 65536 + 65535 - 0x20000))
         FastReadMapReal[x] = &duo->rom->ngpc_rom.data[x * 65536 - 0x200000] - x * 65536;
   }

   for(x = 0x80; x <= 0x9f; x++)
   {
      if(duo->rom->ngpc_rom.length > (x * 65536 + 65535 - 0x80000))
         FastReadMapReal[x] = &duo->rom->ngpc_rom.data[x * 65536 - 0x800000] - x * 65536;
   }
}

void neopop_mem_t::RecacheFRM(void)
{
   int x;
   for (x = 0; x < 256; x++)
      FastReadMap[x] = FlashStatusEnable ? NULL : FastReadMapReal[x];
}

void* neopop_mem_t::translate_address_read(uint32 address)
{
	DuoInstance *duo = GetDuoFromModule(this, mem);

	address &= 0xFFFFFF;

	if (FlashStatusEnable)
	{
      /*Get Flash status? */
		if (
            (address >= ROM_START && address <= ROM_END) || 
            (address >= HIROM_START && address <= HIROM_END))
      {
         FlashStatusEnable = false;
         RecacheFRM();
         if (address == 0x220000 || address == 0x230000)
         {
            FlashStatus = 0xFFFFFFFF;
            return &FlashStatus;
         }
      }
	}

	if (address >= ROM_START && address <= ROM_END)
	{
      /* ROM (LOW) */
		if (address < ROM_START + duo->rom->ngpc_rom.length)
			return duo->rom->ngpc_rom.data + (address - ROM_START);
      return NULL;
	}

	if (address >= HIROM_START && address <= HIROM_END)
	{
      /* ROM (HIGH) */
		if (address < HIROM_START + (duo->rom->ngpc_rom.length - 0x200000))
			return duo->rom->ngpc_rom.data + 0x200000 + (address - HIROM_START);
      return NULL;
	}

	/*BIOS Access? */
    uint8_t *ngpc_bios = duo->bios->bios;
	if ((address & 0xFF0000) == 0xFF0000)
		return ngpc_bios + (address & 0xFFFF); /* BIOS ROM */
	return NULL;
}

void *neopop_mem_t::translate_address_write(uint32 address)
{	
   DuoInstance *duo = GetDuoFromModule(this, mem);
   address &= 0xFFFFFF;

   if (memory_unlock_flash_write)
   {
      /* ROM (LOW) */
      if (address >= ROM_START && address <= ROM_END)
      {
         if (address < ROM_START + duo->rom->ngpc_rom.length)
            return duo->rom->ngpc_rom.data + (address - ROM_START);
         return NULL;
      }

      /* ROM (HIGH) */
      if (address >= HIROM_START && address <= HIROM_END)
      {
         if (address < HIROM_START + (duo->rom->ngpc_rom.length - 0x200000))
            return duo->rom->ngpc_rom.data + 0x200000 + (address - HIROM_START);
         return NULL;
      }
   }
   else
   {
      /*ROM (LOW) */

      if (address >= ROM_START && address <= ROM_END)
      {
         //Ignore Flash commands
         if (address == 0x202AAA || address == 0x205555)
         {
            //			system_debug_message("%06X: Enable Flash command from %06X", pc, address);
            memory_flash_command = true;
            return NULL;
         }

         //Set Flash status reading?
         if (address == 0x220000 || address == 0x230000)
         {
            //			system_debug_message("%06X: Flash status read from %06X", pc, address);
            FlashStatusEnable = true;
            RecacheFRM();
            return NULL;
         }

         if (memory_flash_command)
         {
            //Write the 256byte block around the flash data
            duo->flash->flash_write(address & 0xFFFF00, 256);

            //Need to issue a new command before writing will work again.
            memory_flash_command = false;

            //			system_debug_message("%06X: Direct Flash write to %06X", pc, address & 0xFFFF00);
            //			system_debug_stop();

            //Write to the rom itself.
            if (address < ROM_START + duo->rom->ngpc_rom.length)
               return duo->rom->ngpc_rom.data + (address - ROM_START);
         }
      }
   }

   return NULL;
}

uint8_t neopop_mem_t::loadB(uint32 address)
{
   DuoInstance *duo = GetDuoFromModule(this, mem);
   uint8_t *ptr;
   address &= 0xFFFFFF;

   if(FastReadMap[address >> 16])
      return(FastReadMap[address >> 16][address]);

   ptr = (uint8_t*)translate_address_read(address);

   if (ptr)
      return *ptr;

   if(address >= 0x8000 && address <= 0xbfff)
      return(ngpgfx_read8(&duo->gfx->NGPGfx, address));

   if(address >= 0x4000 && address <= 0x7fff)
      return(*(uint8_t *)(CPUExRAM + address - 0x4000));

   if(address >= 0x70 && address <= 0x7F)
      return(duo->interrupt->int_read8(address));

   if(address >= 0x90 && address <= 0x97)
      return(rtc_read8(duo->rtc, address));

   if(address >= 0x20 && address <= 0x29)
      return(duo->interrupt->timer_read8(address));

   switch (address)
   {
      case 0x50:
         return(SC0BUF_rx);
      case 0x51:
         return(SC0CR);
      case 0x52:
         return(SC0MOD);
      case 0x53:
         return(BR0CR);
      case 0x54:
         return(SC1BUF_rx);
      case 0x55:
         return(SC1CR);
      case 0x56:
         return(SC1MOD);
      case 0x57:
         return(BR1CR);
      case 0x58:
         return(ODE);
      case 0xBC:
         return duo->z80i->Z80_ReadComm();
   }

   //printf("UNK B R: %08x\n", address);

   return 0;
}

uint16_t neopop_mem_t::loadW(uint32 address)
{
   DuoInstance *duo = GetDuoFromModule(this, mem);
   address &= 0xFFFFFF;

   if(address & 1)
   {
      uint16 ret;

      ret = loadB(address);
      ret |= loadB(address + 1) << 8;

      return(ret);
   }

   if(FastReadMap[address >> 16])
   {
      uint16_t *ptr16 = (uint16_t*)&FastReadMap[address >> 16][address];
#ifdef MSB_FIRST
      return LoadU16_RBO(ptr16);
#else
      return *ptr16;
#endif
   }

   uint16_t* ptr = (uint16_t*)translate_address_read(address);
   if(ptr)
   {
#ifdef MSB_FIRST
      return LoadU16_RBO(ptr);
#else
      return *ptr;
#endif
   }

   if(address >= 0x8000 && address <= 0xbfff)
      return(ngpgfx_read16(&duo->gfx->NGPGfx, address));

   if(address >= 0x4000 && address <= 0x7fff)
   {
      uint16_t *ptr16 = (uint16_t *)(CPUExRAM + address - 0x4000);
#ifdef MSB_FIRST
      return LoadU16_RBO(ptr16);
#else
      return *ptr16;
#endif
   }

   if(address == 0x50)
	   return(SC0BUF_rx);
   else if (address == 0x51)
	   return(SC0CR);
   else if (address == 0x52)
	   return(SC0MOD);
   else if (address == 0x53)
	   return(BR0CR);
   else if (address == 0x54)
	   return(SC1BUF_rx);
   else if (address == 0x55)
	   return(SC1CR);
   else if (address == 0x56)
	   return(SC1MOD);
   else if (address == 0x57)
	   return(BR1CR);
   else if (address == 0x58)
	   return(ODE);

   if(address >= 0x70 && address <= 0x7F)
   {
      uint16 ret;

      ret = duo->interrupt->int_read8(address);
      ret |= duo->interrupt->int_read8(address + 1) << 8;

      return(ret);
   }

   if(address >= 0x90 && address <= 0x97)
   {
      uint16 ret;

      ret = rtc_read8(duo->rtc, address);
      ret |= rtc_read8(duo->rtc, address + 1) << 8;

      return(ret);
   }

   if(address >= 0x20 && address <= 0x29)
   {
      uint16 ret;

      ret = duo->interrupt->timer_read8(address);
      ret |= duo->interrupt->timer_read8(address + 1) << 8;

      return(ret);
   }

   if(address == 0xBC)
      return duo->z80i->Z80_ReadComm();

   //printf("UNK W R: %08x\n", address);

   return(0);
}

uint32 neopop_mem_t::loadL(uint32 address)
{
   uint32 ret;

   ret = loadW(address);
   ret |= loadW(address + 2) << 16;

   return(ret);
}

void neopop_mem_t::storeB(uint32 address, uint8_t data)
{
   DuoInstance *duo = GetDuoFromModule(this, mem);
   address &= 0xFFFFFF;

   if(address < 0x80)
      lastpoof = data;

   if(address >= 0x8000 && address <= 0xbfff)
   {
      ngpgfx_write8(&duo->gfx->NGPGfx, address, data);
      return;
   }

   if(address >= 0x4000 && address <= 0x7fff)
   {
      *(uint8_t *)(CPUExRAM + address - 0x4000) = data;
      return;
   }
   if(address >= 0x70 && address <= 0x7F)
   {
      duo->interrupt->int_write8(address, data);
      return;
   }
   if(address >= 0x20 && address <= 0x29)
   {
       duo->interrupt->timer_write8(address, data);
      return;
   }

   switch (address)
   {
      case 0x50:
         SC0BUF_tx = data;
         return;
      case 0x51:
         SC0CR = data;
         return;
      case 0x52:
         SC0MOD = data;
         return;
      case 0x53:
         BR0CR = data;
         return;
      case 0x54:
         SC1BUF_tx = data;
         return;
      case 0x55:
         SC1CR = data;
         return;
      case 0x56:
         SC1MOD = data;
         return;
      case 0x57:
         BR1CR = data;
         return;
      case 0x58:
         ODE = data;
         return;
      case 0x6f: /* Watchdog timer */
         return;
      case 0xb2: /* Comm */
         COMMStatus = data & 1;
         return;
      case 0xb9:
         if(data == 0x55)
            duo->z80i->Z80_SetEnable(1);
         else if(data == 0xAA)
            duo->z80i->Z80_SetEnable(0);
         return;
      case 0xb8:
         if(data == 0x55)
            MDFNNGPCSOUND_SetEnable(1);
         else if(data == 0xAA)
            MDFNNGPCSOUND_SetEnable(0);
         return;
      case 0xBA:
         duo->z80i->Z80_nmi();
         return;
      case 0xBC:
         duo->z80i->Z80_WriteComm(data);
         return;
   }

   if(address >= 0xa0 && address <= 0xA3)
   {
      if(!duo->z80i->Z80_IsEnabled())
      {
         if (address == 0xA1)
            duo->sound->Write_SoundChipLeft(data);
         else if (address == 0xA0)
            duo->sound->Write_SoundChipRight(data);
      } 
      //DAC Write
      if (address == 0xA2)
         duo->sound->dac_write_left(data);
      else if (address == 0xA3)
         duo->sound->dac_write_right(data);
      return;
   }

   uint8_t* ptr = (uint8_t*)translate_address_write(address);

   /* Write */
   if (ptr)
      *ptr = data;
}

void neopop_mem_t::storeW(uint32 address, uint16_t data)
{
   DuoInstance *duo = GetDuoFromModule(this, mem);
   uint16_t* ptr;
   address &= 0xFFFFFF;

   if(address & 1)
   {
      storeB(address + 0, data & 0xFF);
      storeB(address + 1, data >> 8);
      return;
   }

   if(address < 0x80)
      lastpoof = data >> 8;

   if(address >= 0x8000 && address <= 0xbfff)
   {
      ngpgfx_write16(&duo->gfx->NGPGfx, address, data);
      return;
   }
   if(address >= 0x4000 && address <= 0x7fff)
   {
      uint16_t *ptr16 = (uint16_t *)(CPUExRAM + address - 0x4000);
#ifdef MSB_FIRST
      StoreU16_RBO(ptr16, data);
#else
      *ptr16 = data;
#endif
      return;
   }
   if(address >= 0x70 && address <= 0x7F)
   {
      duo->interrupt->int_write8(address, data & 0xFF);
      duo->interrupt->int_write8(address + 1, data >> 8);
      return;
   }

   if(address >= 0x20 && address <= 0x29)
   {
      duo->interrupt->timer_write8(address, data & 0xFF);
      duo->interrupt->timer_write8(address + 1, data >> 8);
   }

   switch (address)
   {
      case 0x50:
         SC0BUF_tx = data & 0xFF;
         return;
      case 0x51:
         SC0CR = data & 0xFF;
         return;
      case 0x52:
         SC0MOD = data & 0xFF;
         return;
      case 0x53:
         BR0CR = data & 0xFF;
         return;
      case 0x54:
         SC1BUF_tx = data & 0xFF;
         return;
      case 0x55:
         SC1CR = data & 0xFF;
         return;
      case 0x56:
         SC1MOD = data & 0xFF;
         return;
      case 0x57:
         BR1CR = data & 0xFF;
         return;
      case 0x58:
         ODE = data & 0xFF;
         return;
      case 0x6e: /* Watchdog timer(technically 0x6f) */
         return;
      case 0xB2: /* Comm */
         COMMStatus = data & 1;
         return;
      case 0xb8:
         if((data & 0xFF00) == 0x5500)
            duo->z80i->Z80_SetEnable(1);
         else if((data & 0xFF00) == 0xAA00)
            duo->z80i->Z80_SetEnable(0);

         if((data & 0xFF) == 0x55)
            MDFNNGPCSOUND_SetEnable(1);
         else if((data & 0xFF) == 0xAA)
            MDFNNGPCSOUND_SetEnable(0);
         return;
      case 0xBA:
         duo->z80i->Z80_nmi();
         return;
      case 0xBC:
         duo->z80i->Z80_WriteComm(data);
         return;
   }

   if(address >= 0xa0 && address <= 0xA3)
   {
      storeB(address, data & 0xFF);
      storeB(address + 1, data >> 8);
      return;
   }

   ptr = (uint16_t*)translate_address_write(address);

   /* Write */
   if (ptr)
   {
#ifdef MSB_FIRST
      StoreU16_RBO(ptr, data);
#else
      *ptr = data;
#endif
   }
}

void neopop_mem_t::storeL(uint32 address, uint32 data)
{
   storeW(address, data & 0xFFFF);
   storeW(address + 2, data >> 16);
}

static const uint8_t systemMemory[] = 
{
	// 0x00												// 0x08
	0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x08, 0xFF, 0xFF,
	// 0x10												// 0x18
	0x34, 0x3C, 0xFF, 0xFF, 0xFF, 0x3F, 0x00, 0x00,		0x3F, 0xFF, 0x2D, 0x01, 0xFF, 0xFF, 0x03, 0xB2,
	// 0x20												// 0x28
	0x80, 0x00, 0x01, 0x90, 0x03, 0xB0, 0x90, 0x62,		0x05, 0x00, 0x00, 0x00, 0x0C, 0x0C, 0x4C, 0x4C,
	// 0x30												// 0x38
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		0x30, 0x00, 0x00, 0x00, 0x20, 0xFF, 0x80, 0x7F,
	// 0x40												// 0x48
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	// 0x50												// 0x58
	0x00, 0x20, 0x69, 0x15, 0x00, 0x00, 0x00, 0x00,		0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF,
	// 0x60												// 0x68
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		0x17, 0x17, 0x03, 0x03, 0x02, 0x00, 0x00, 0x4E,
	// 0x70												// 0x78
	0x02, 0x32, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	// 0x80												// 0x88
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	// 0x90												// 0x98
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	// 0xA0												// 0xA8
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	// 0xB0												// 0xB8
	0x00, 0x00, 0x00, 0x00, 0x0A, 0x00, 0x00, 0x00,		0xAA, 0xAA, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	// 0xC0												// 0xC8
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	// 0xD0												// 0xD8
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	// 0xE0												// 0xE8
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	// 0xF0												// 0xF8
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

void neopop_mem_t::reset_memory(void)
{
	DuoInstance *duo = GetDuoFromModule(this, mem);
	unsigned int i;

	FlashStatusEnable = false;
	RecacheFRM();

	memory_flash_command = false;

   /* 000000 -> 000100	CPU Internal RAM (Timers/DMA/Z80) */

	for (i = 0; i < sizeof(systemMemory); i++)
		storeB(i, systemMemory[i]);

   /* 006C00 -> 006FFF	BIOS Workspace */

	storeL(0x6C00, duo->rom->rom_header->startPC);		//Start

	storeW(0x6C04, duo->rom->rom_header->catalog);
	storeW(0x6E82, duo->rom->rom_header->catalog);

	storeB(0x6C06, duo->rom->rom_header->subCatalog);
	storeB(0x6E84, duo->rom->rom_header->subCatalog);

	for(i = 0; i < 12; i++)
	 storeB(0x6c08 + i, duo->rom->ngpc_rom.data[0x24 + i]);

	storeB(0x6C58, 0x01);

	/* 32MBit cart? */
	if (duo->rom->ngpc_rom.length > 0x200000)
		storeB(0x6C59, 0x01);
	else
		storeB(0x6C59, 0x00);

	storeB(0x6C55, 1);      /* Commercial game */

	storeB(0x6F80, 0xFF);	/* Lots of battery power! */
	storeB(0x6F81, 0x03);

	storeB(0x6F84, 0x40);	/* "Power On" startup */
	storeB(0x6F85, 0x00);	/* No shutdown request */
	storeB(0x6F86, 0x00);	/* No user answer (?) */

	/* Language: 0 = Japanese, 1 = English */
	storeB(0x6F87, MDFN_GetSettingB("ngp.language"));

	/* Color Mode Selection: 0x00 = B&W, 0x10 = Colour */
	storeB(0x6F91, duo->rom->rom_header->mode);
	storeB(0x6F95, duo->rom->rom_header->mode);

	/* Interrupt table */
	for (i = 0; i < 0x12; i++)
		storeL(0x6FB8 + i * 4, 0x00FF23DF);


   /* 008000 -> 00BFFF	Video RAM */
	storeB(0x8000, 0xC0);	// Both interrupts allowed

	/* Hardware window */
	storeB(0x8002, 0x00);
	storeB(0x8003, 0x00);
	storeB(0x8004, 0xFF);
	storeB(0x8005, 0xFF);

	storeB(0x8006, 0xc6);	// Frame Rate Register

	storeB(0x8012, 0x00);	// NEG / OOWC setting.

	storeB(0x8118, 0x80);	// BGC on!

	storeB(0x83E0, 0xFF);	// Default background colour
	storeB(0x83E1, 0x0F);

	storeB(0x83F0, 0xFF);	// Default window colour
	storeB(0x83F1, 0x0F);

	storeB(0x8400, 0xFF);	// LED on
	storeB(0x8402, 0x80);	// Flash cycle = 1.3s

	storeB(0x87E2, loadB(0x6F95) ? 0x00 : 0x80);

	//
	// Metal Slug - 2nd Mission oddly relies on a specific character RAM pattern.
	//
	{
	 static const uint8 char_data[64] = {
	 	255, 63, 255, 255, 0, 252, 255, 255, 255, 63, 3, 0, 255, 255, 255, 255, 
	 	240, 243, 252, 243, 255, 3, 255, 195, 255, 243, 243, 243, 240, 243, 240, 195, 
		207, 15, 207, 15, 207, 15, 207, 207, 207, 255, 207, 255, 207, 255, 207, 63, 
		255, 192, 252, 195, 240, 207, 192, 255, 192, 255, 240, 207, 252, 195, 255, 192 };

         for(i = 0; i < 64; i++)
	 {
	  storeB(0xA1C0 + i, char_data[i]);
	 }
	}
}

// ---------------
// Global Wrappers
// ---------------

#ifdef __cplusplus
extern "C" {
#endif

uint8_t loadB(uint32_t address)
{
    return DuoInstance::currentInstance->mem->loadB(address);
}

uint16_t loadW(uint32_t address)
{
    return DuoInstance::currentInstance->mem->loadW(address);
}

uint32_t loadL(uint32_t address)
{
    return DuoInstance::currentInstance->mem->loadL(address);
}

void storeB(uint32_t address, uint8_t data)
{
    DuoInstance::currentInstance->mem->storeB(address, data);
}

void storeW(uint32_t address, uint16_t data)
{
    DuoInstance::currentInstance->mem->storeW(address, data);
}

void storeL(uint32_t address, uint32_t data)
{
    DuoInstance::currentInstance->mem->storeL(address, data);
}

#ifdef __cplusplus
}
#endif

