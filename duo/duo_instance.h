// ----------
// NeoPop Duo
// ----------

#ifndef DUO_INSTANCE_H
#define DUO_INSTANCE_H

#include <stdint.h>
#include "../mednafen/ngp/bios.h"
#include "../mednafen/ngp/rtc.h"
#include "../mednafen/mednafen-types.h"
#include "../mednafen/git.h"
#include "../mednafen/video.h"

#include "../mednafen/hw_cpu/z80-fuse/z80.h"
#include "../mednafen/ngp/TLCS-900h/TLCS900h.h"

#include "duo_settings.h"

struct neopop_comms_t;
struct neopop_dma_t;
struct neopop_flash_t;
struct neopop_gfx_t;
struct neopop_interrupt_t;
struct neopop_io_t;
struct neopop_mem_t;
struct neopop_rom_t;
struct neopop_sound_t;
struct neopop_z80i_t;

// #define TLCS_PC_BREADCRUMB

class DuoInstance
{
public:
	enum Maximums
	{
		MAX_INSTANCES = 2
	};

	// Mednafen metadata
	EmulateSpecStruct spec;
	MDFNGI *game;
	MDFN_Surface *surface;

	uint8_t system_colour;
	uint8_t last_raster_line;
	int32_t z80_runtime;

	// Frame output
	int32_t SoundBufSize;
	unsigned width, height;
	int16_t sound_buf[0x10000];
	MDFN_Rect rects[FB_MAX_HEIGHT];
	unsigned hintCounter;

	// Input
	uint8_t input_buf;
	uint8_t NGPJoyLatch;

	// Processor state
	uint32_t absTime;
	TLCS900h tlcs900h_state;
	Z80 z80_state;

	// NeoPop core
	neopop_bios_t *bios;
	neopop_comms_t *comms;
	neopop_dma_t *dma;
	neopop_flash_t *flash;
	neopop_gfx_t *gfx;
	neopop_interrupt_t *interrupt;
	neopop_io_t *io;
	neopop_mem_t *mem;
	neopop_rom_t *rom;
	neopop_rtc_t *rtc;
	neopop_sound_t *sound;
	neopop_z80i_t *z80i;

#ifdef TLCS_PC_BREADCRUMB
	static const uint32_t PC_HISTORY_LEN = 4096;
	uint32_t pc_history[PC_HISTORY_LEN];
#endif

	DuoInstance();

	bool Initialize();
	void Deinitialize();

	/*! WARNING: Make sure the instance is active before running this! */
	void Reset();

	/*! WARNING: Make sure the instance is active before running this! */
	bool LoadGame(MDFNGI *game_info, uint32 rom_size, const void *rom_data);

	/*! WARNING: Make sure the instance is active before running this! */
	void UnloadGame();

	void ConfigureSpec();

	/*! WARNING: Make sure the instance is active before running this! */
	void ProcessFrame();

	static void ProcessFrame_Interleaved(DuoInstance *duoA, DuoInstance *duoB);

private:
	void StartFrame();
	void FinishFrame();

	// ------
	// Static
	// ------
public:
	static DuoInstance *currentInstance;
	static DuoInstance instances[MAX_INSTANCES];

	/*! Prepares a given DuoInstance for use and stores it in
		the currentInstance pointer */
	static void StageInstance(DuoInstance *instance);

	/*! Unstages the current instance and backs up any changes
		that may have happened in the processor interpreters */
	static void UnstageCurrentInstance();

};

inline DuoInstance *GetDuoWithModuleAtOffset(void *ptr, size_t offset)
{
	for (int i = 0; i < DuoInstance::MAX_INSTANCES; i++)
	{
		uint8 *duoPtr = (uint8*)&DuoInstance::instances[i];
		uint8 *modulePtr = duoPtr + offset;
		if (*(void**)modulePtr == ptr)
			return &DuoInstance::instances[i];
	}

	return NULL;
}

#define GetDuoFromModule(MODULE_PTR, MODULE_NAME) \
	GetDuoWithModuleAtOffset(MODULE_PTR, offsetof(DuoInstance, MODULE_NAME))

#endif
