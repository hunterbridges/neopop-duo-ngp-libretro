// ----------
// NeoPop Duo
// ----------

#include "duo_instance.h"

#include "../mednafen/ngp/bios.h"
#include "../mednafen/ngp/dma.h"
#include "../mednafen/ngp/gfx.h"
#include "../mednafen/ngp/interrupt.h"
#include "../mednafen/ngp/mem.h"
#include "../mednafen/ngp/rom.h"
#include "../mednafen/ngp/sound.h"
#include "../mednafen/ngp/Z80_interface.h"
#include "../mednafen/ngp/TLCS-900h/TLCS900h_interpret.h"
#include "../mednafen/ngp/TLCS-900h/TLCS900h_registers.h"

DuoInstance *DuoInstance::currentInstance = nullptr;
DuoInstance *DuoInstance::currentInstance = nullptr;
DuoInstance DuoInstance::instances[MAX_INSTANCES];

// ----------------
// DuoTLCS900hState
// ----------------

void DuoTLCS900hState::Capture()
{
	// TODO
}

void DuoTLCS900hState::Restore()
{
	// TODO
}

// -----------
// DuoZ80State
// -----------

void DuoZ80State::Capture()
{
	// TODO
}

void DuoZ80State::Restore()
{
	// TODO
}

// -----------
// DuoInstance
// -----------

DuoInstance::DuoInstance()
{
	this->game = nullptr;
	this->surface = nullptr;
}

DuoInstance::DuoInstance(MDFNGI *game, MDFN_Surface *surface)
{
	this->game = game;
	this->surface = surface;
}

void DuoInstance::Reset()
{
   ngpgfx_power(&gfx->NGPGfx);
   z80i->Z80_reset();
   interrupt->reset_int();
   interrupt->reset_timers();

   mem->reset_memory();
   BIOSHLE_Reset(bios);
   reset_registers();	/* TLCS900H registers */
   dma->reset_dma();
}

int DuoInstance::LoadGame(const char *name, MDFNFILE *fp, const uint8_t *data, size_t size)
{
   if ((data != NULL) && (size != 0)) {
      if (!(ngpc_rom.data = (uint8 *)malloc(size)))
         return(0);
      ngpc_rom.length = size;
      memcpy(ngpc_rom.data, data, size);
   }
   else
   {
      if(!(ngpc_rom.data = (uint8 *)malloc(fp->size)))
         return(0);

      ngpc_rom.length = fp->size;
      memcpy(ngpc_rom.data, fp->data, fp->size);
   }

   rom_loaded();

   NGPGfx = (ngpgfx_t*)calloc(1, sizeof(*NGPGfx));
   NGPGfx->layer_enable = 1 | 2 | 4;

   MDFNGameInfo->fps = (uint32)((uint64)6144000 * 65536 * 256 / 515 / 198); // 3072000 * 2 * 10000 / 515 / 198

   MDFNNGPCSOUND_Init();

   SetFRM(); // Set up fast read memory mapping

   bios_install();

   z80_runtime = 0;

   Reset();

   return(1);
}

void DuoInstance::ProcessFrame(EmulateSpecStruct *espec)
{
   bool drewFrame = false;

   espec->DisplayRect.x = 0;
   espec->DisplayRect.y = 0;
   espec->DisplayRect.w = 160;
   espec->DisplayRect.h = 152;

   if(espec->VideoFormatChanged)
      ngpgfx_set_pixel_format(&gfx->NGPGfx, espec->surface->depth);

   if (espec->SoundFormatChanged)
	  sound->SetSoundRate(espec->SoundRate);

   NGPJoyLatch = input_buf;

   storeB(0x6F82, input_buf);

   sound->ngpc_soundTS = 0;
   interrupt->NGPFrameSkip = espec->skip;

   do
   {
      int32 timetime = (uint8)TLCS900h_interpret();
      drewFrame |= interrupt->updateTimers(espec->surface, timetime);
      z80_runtime += timetime;

      while(z80_runtime > 0)
      {
         int z80rantime = z80i->Z80_RunOP();

         if (z80rantime < 0) // Z80 inactive, so take up all run time!
         {
            z80_runtime = 0;
            break;
         }

         z80_runtime -= z80rantime << 1;

      }
   } while(!drewFrame);

   espec->MasterCycles = sound->ngpc_soundTS;
   espec->SoundBufSize = sound->Flush(espec->SoundBuf, espec->SoundBufMaxSize);
}

// ------------------
// DuoInstance static
// ------------------

void DuoInstance::StageInstance(DuoInstance *instance)
{
	UnstageCurrentInstance();
	
	if (instance == nullptr)
		return;

	currentInstance = instance;
	currentInstance->tlcs900hState.Restore();
	currentInstance->z80State.Restore();
}

void DuoInstance::UnstageCurrentInstance()
{
	if (currentInstance == nullptr)
		return;

	currentInstance->tlcs900hState.Capture();
	currentInstance->z80State.Capture();
	currentInstance = nullptr;
}

