// ----------
// NeoPop Duo
// ----------

#include "duo_instance.h"
#include "duo_runner.h"

#include <malloc.h>

#include "../mednafen/ngp/bios.h"
#include "../mednafen/ngp/dma.h"
#include "../mednafen/ngp/flash.h"
#include "../mednafen/ngp/gfx.h"
#include "../mednafen/ngp/interrupt.h"
#include "../mednafen/ngp/mem.h"
#include "../mednafen/ngp/rom.h"
#include "../mednafen/ngp/sound.h"
#include "../mednafen/ngp/system.h"
#include "../mednafen/ngp/Z80_interface.h"
#include "../mednafen/ngp/TLCS-900h/TLCS900h.h"

DuoInstance *DuoInstance::currentInstance = nullptr;
DuoInstance DuoInstance::instances[MAX_INSTANCES];

// -----------
// DuoInstance
// -----------

DuoInstance::DuoInstance()
{
	game = NULL;
	surface = NULL;
	NGPJoyLatch = 0;
	SoundBufSize = 0;
	bios = NULL;
	comms = NULL;
	dma = NULL;
	flash = NULL;
	gfx = NULL;
	interrupt = NULL;
	io = NULL;
	mem = NULL;
	rom = NULL;
	rtc = NULL;
	sound = NULL;
	z80i = NULL;
	absTime = 0;
}

bool DuoInstance::Initialize()
{
	bios = (neopop_bios_t*)calloc(1, sizeof(neopop_bios_t));
	if (bios == NULL) goto error;
	// C-style, no constructor

	comms = (neopop_comms_t*)calloc(1, sizeof(neopop_comms_t));
	if (comms == NULL) goto error;
	new (comms) neopop_comms_t();

	dma = (neopop_dma_t*)calloc(1, sizeof(neopop_dma_t));
	if (dma == NULL) goto error;
	new (dma) neopop_dma_t();

	flash = (neopop_flash_t*)calloc(1, sizeof(neopop_flash_t));
	if (flash == NULL) goto error;
	new (flash) neopop_flash_t();

	gfx = (neopop_gfx_t*)calloc(1, sizeof(neopop_gfx_t));
	if (gfx == NULL) goto error;
	new (gfx) neopop_gfx_t();

	interrupt = (neopop_interrupt_t*)calloc(1, sizeof(neopop_interrupt_t));
	if (interrupt == NULL) goto error;
	new (interrupt) neopop_interrupt_t();

	io = (neopop_io_t*)calloc(1, sizeof(neopop_io_t));
	if (io == NULL) goto error;
	new (io) neopop_io_t();

	mem = (neopop_mem_t*)calloc(1, sizeof(neopop_mem_t));
	if (mem == NULL) goto error;
	new (mem) neopop_mem_t();

	rom = (neopop_rom_t*)calloc(1, sizeof(neopop_rom_t));
	if (rom == NULL) goto error;
	new (rom) neopop_rom_t();

	rtc = (neopop_rtc_t*)calloc(1, sizeof(neopop_rtc_t));
	if (rtc == NULL) goto error;
	// C-style, no constructor

	sound = (neopop_sound_t*)calloc(1, sizeof(neopop_sound_t));
	if (sound == NULL) goto error;
	new (sound) neopop_sound_t();

	z80i = (neopop_z80i_t*)calloc(1, sizeof(neopop_z80i_t));
	if (z80i == NULL) goto error;
	new (z80i) neopop_z80i_t();

	// Zero out the CPU states
	memset(&z80_state, 0, sizeof(z80_state));

	// Initialize the TLCS900h tables
	tlcs900h_state.initGPRTables();
	tlcs900h_state.initRegCodeTables();

	return true;
error:
	Deinitialize();
	return false;
}

void DuoInstance::Deinitialize()
{
	if (bios)
	{
		free(bios);
		bios = NULL;
	}

	if (comms)
	{
		comms->~neopop_comms_t();
		free(comms);
		comms = NULL;
	}

	if (dma)
	{
		dma->~neopop_dma_t();
		free(dma);
		dma = NULL;
	}

	if (flash)
	{
		flash->~neopop_flash_t();
		free(flash);
		flash = NULL;
	}

	if (gfx)
	{
		gfx->~neopop_gfx_t();
		free(gfx);
		gfx = NULL;
	}

	if (interrupt)
	{
		interrupt->~neopop_interrupt_t();
		free(interrupt);
		interrupt = NULL;
	}

	if (io)
	{
		io->~neopop_io_t();
		free(io);
		io = NULL;
	}

	if (mem)
	{
		mem->~neopop_mem_t();
		free(mem);
		mem = NULL;
	}

	if (rom)
	{
		rom->~neopop_rom_t();
		free(rom);
		rom = NULL;
	}

	if (rtc)
	{
		free(rtc);
		rtc = NULL;
	}

	if (sound)
	{
		sound->~neopop_sound_t();
		free(sound);
		sound = NULL;
	}

	if (z80i)
	{
		z80i->~neopop_z80i_t();
		free(z80i);
		z80i = NULL;
	}

	if (surface)
	{
		if (surface->pixels)
			free(surface->pixels);

		free(surface);
		surface = NULL;
	}
}

void DuoInstance::Reset()
{
	ngpgfx_power(&gfx->NGPGfx);
	z80i->Z80_reset();
	interrupt->reset_int();
	interrupt->reset_timers();

	mem->reset_memory();
	BIOSHLE_Reset(bios);
	tlcs900h_state.reset_registers(); /* TLCS900H registers */
	dma->reset_dma();
}

bool DuoInstance::LoadGame(MDFNGI *game_info, uint32 rom_size, const void *rom_data)
{
	// Copy the rom data into the instance
	rom->ngpc_rom.data = (uint8*)malloc(rom_size);
	if (rom->ngpc_rom.data == NULL)
		goto error;

	rom->ngpc_rom.length = rom_size;
	memcpy(rom->ngpc_rom.data, rom_data, rom_size);

	rom->rom_loaded();

	// Initialize runtime aspects of subsystems
	memset(&gfx->NGPGfx, 0, sizeof(ngpgfx_t));
	gfx->NGPGfx.layer_enable = 1 | 2 | 4;

	sound->Init();

	// Set up fast read memory mapping
	mem->SetFRM();

	bios_install(bios);

	z80_runtime = 0;

	Reset();

	// Init output surface (instance)
	surface = (MDFN_Surface*)calloc(1, sizeof(MDFN_Surface));

	if (surface == NULL)
		goto error;

	surface->width = FB_WIDTH;
	surface->height = FB_HEIGHT;
	surface->pitch = FB_WIDTH;
	surface->depth = DuoRunner::shared.RETRO_PIX_DEPTH;

	surface->pixels = (uint16_t*)calloc(1, FB_WIDTH * FB_HEIGHT * sizeof(uint32_t));

	if (surface->pixels == NULL)
		goto error;

	// Set core gfx settings (instance)
	ngpgfx_set_pixel_format(&gfx->NGPGfx, surface->depth);

	// Set core sound settings (instance)
	sound->SetSoundRate(DuoRunner::shared.RETRO_SAMPLE_RATE);

	game = game_info;

	return true;

error:
	if (surface)
	{
		if (surface->pixels)
		{
			free(surface->pixels);
			surface->pixels = NULL;
		}

		free(surface);
		surface = NULL;
	}

	if (rom->ngpc_rom.data)
	{
		free(rom->ngpc_rom.data);
		rom->ngpc_rom.data = NULL;
		rom->rom_header = NULL;
	}

	game = NULL;

	return false;
}

void DuoInstance::UnloadGame()
{
	if (game != NULL)
	{
		rom->rom_unload();
		memset(&gfx->NGPGfx, 0, sizeof(ngpgfx_t));
		game = NULL;
	}

	if (surface)
	{
		if (surface->pixels)
		{
			free(surface->pixels);
			surface->pixels = NULL;
		}

		free(surface);
		surface = NULL;
	}

	if (rom->ngpc_rom.data)
	{
		free(rom->ngpc_rom.data);
		rom->ngpc_rom.data = NULL;
		rom->rom_header = NULL;
	}
}

void DuoInstance::ConfigureSpec()
{
	spec.surface = surface;
	spec.SoundRate = DuoRunner::shared.RETRO_SAMPLE_RATE;
	spec.SoundBuf = sound_buf;
	spec.LineWidths = rects;
	spec.SoundBufMaxSize = sizeof(sound_buf) / 2;
	spec.SoundVolume = 1.0;
	spec.soundmultiplier = 1.0;
	spec.SoundBufSize = 0;
	spec.VideoFormatChanged = DuoRunner::shared.update_video;
	spec.SoundFormatChanged = DuoRunner::shared.update_audio;
}

void DuoInstance::StartFrame()
{
	rects[0].w = ~0;

	spec.DisplayRect.x = 0;
	spec.DisplayRect.y = 0;
	spec.DisplayRect.w = 160;
	spec.DisplayRect.h = 152;

	if (spec.VideoFormatChanged)
		ngpgfx_set_pixel_format(&gfx->NGPGfx, spec.surface->depth);

	if (spec.SoundFormatChanged)
		sound->SetSoundRate(spec.SoundRate);

	NGPJoyLatch = input_buf;

	storeB(0x6F82, input_buf);

	sound->ngpc_soundTS = 0;
	interrupt->NGPFrameSkip = spec.skip;
}

void DuoInstance::FinishFrame()
{
	spec.MasterCycles = sound->ngpc_soundTS;
	spec.SoundBufSize = sound->Flush(spec.SoundBuf, spec.SoundBufMaxSize);

	// Set frame output vars
	SoundBufSize = spec.SoundBufSize - spec.SoundBufSizeALMS;

	spec.SoundBufSize = spec.SoundBufSizeALMS + SoundBufSize;

	width = spec.DisplayRect.w;
	height = spec.DisplayRect.h;
}

void DuoInstance::ProcessFrame()
{
	bool drewFrame = false;

	StartFrame();

	do
	{
		int32 timetime = tlcs900h_state.TLCS900h_interpret();
		drewFrame |= interrupt->updateTimers(spec.surface, timetime);
		z80_runtime += timetime;

		while (z80_runtime > 0)
		{
			int z80rantime = z80i->Z80_RunOP();

			if (z80rantime < 0) // Z80 inactive, so take up all run time!
			{
				z80_runtime = 0;
				break;
			}

			z80_runtime -= z80rantime << 1;

		}
	} while (!drewFrame);

	FinishFrame();
}

void DuoInstance::ProcessFrame_Interleaved(DuoInstance *duoA, DuoInstance *duoB)
{
	DuoInstance *instances[2] = { duoA, duoB };
	bool drewFrame[2] = { false, false };

	for (int i = 0; i < 2; i++)
	{
		DuoInstance::StageInstance(instances[i]);
		instances[i]->StartFrame();
		DuoInstance::UnstageCurrentInstance();
	}

	do
	{
		for (int i = 0; i < 2; i++)
		{
			if (drewFrame[i] == true)
				return;

			DuoInstance::StageInstance(instances[i]);

			int32 tlcsCycles = instances[i]->tlcs900h_state.TLCS900h_interpret();
			drewFrame[i] |= instances[i]->interrupt->updateTimers(instances[i]->spec.surface, tlcsCycles);
			instances[i]->z80_runtime += tlcsCycles;

			while (instances[i]->z80_runtime > 0)
			{
				int z80rantime = instances[i]->z80i->Z80_RunOP();

				if (z80rantime < 0) // Z80 inactive, so take up all run time!
				{
					instances[i]->z80_runtime = 0;
					break;
				}

				instances[i]->z80_runtime -= z80rantime << 1;

			}

			instances[i]->absTime += tlcsCycles;
			DuoInstance::UnstageCurrentInstance();
		}
	} while (!drewFrame[0] || !drewFrame[1]);

	for (int i = 0; i < 2; i++)
	{
		DuoInstance::StageInstance(instances[i]);
		instances[i]->FinishFrame();
		DuoInstance::UnstageCurrentInstance();
	}
}

// ------------------
// DuoInstance static
// ------------------

void DuoInstance::StageInstance(DuoInstance *instance)
{
	UnstageCurrentInstance();

	if (instance == NULL)
		return;

	currentInstance = instance;
}

void DuoInstance::UnstageCurrentInstance()
{
	if (currentInstance == NULL)
		return;

	currentInstance = NULL;
}

