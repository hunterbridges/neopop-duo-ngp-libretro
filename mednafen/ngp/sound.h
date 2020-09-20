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

#ifndef __SOUND__
#define __SOUND__

#include <stdint.h>

#include "../include/blip/Blip_Buffer.h"
#include "../include/blip/Stereo_Buffer.h"
#include "T6W28_Apu.h"

/* Speed of DAC playback (in Hz) */
#define DAC_FREQUENCY		8000

typedef Blip_Synth<blip_good_quality, 0xFF> Synth;

struct neopop_sound_t
{
   int32_t ngpc_soundTS;

   T6W28_Apu apu;

   Stereo_Buffer buf;

   uint8_t LastDACLeft = 0;
   uint8_t LastDACRight = 0;
   uint8_t CurrentDACLeft = 0;
   uint8_t CurrentDACRight = 0;

   Synth synth;
   bool schipenable = 0;

   void SetEnable(bool set);
   int32_t Flush(int16_t *SoundBuf, const int32_t MaxSoundFrames);
   void Init(void);
   bool SetSoundRate(uint32_t rate);

   void Write_SoundChipLeft(uint8_t data);
   void Write_SoundChipRight(uint8_t data);

   void dac_write_left(uint8_t);
   void dac_write_right(uint8_t);

private:
   void RedoVolume();
};

int MDFNNGPCSOUND_StateAction(void *data, int load, int data_only);
void MDFNNGPCSOUND_SetEnable(bool set);

#endif
