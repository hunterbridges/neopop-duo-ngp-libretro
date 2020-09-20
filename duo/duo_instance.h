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

#include "../mednafen/hw_cpu/z80-fuse/z80_types.h"

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

struct DuoTLCS900hState
{
	uint32_t mem;
	int size;
	uint8_t first;
	uint8_t second;
	uint8_t R;
	uint8_t rCode;
	int32_t cycles;
	bool brCode;

	uint32_t pc;
	uint16_t sr;
	uint8_t f_dash;

	uint32_t gprBank[4][4];
	uint32_t gpr[4];

	uint32_t rErr;

	uint8_t statusRFP;

	// TODO Are these tables needed?
	// uint8_t* gprMapB[4][8];
	// uint16_t* gprMapW[4][8];
	// uint32_t* gprMapL[4][8];
	// uint8_t* regCodeMapB[4][256];
	// uint16_t* regCodeMapW[4][128];
	// uint32_t* regCodeMapL[4][64];

	/*! Captures the state from the global TLCS900h interpreter and
		stores it in this struct */
	void Capture();

	/*! Restores the state stored in this struct into the global
		TLCS900h interpreter */
	void Restore();
};

struct DuoZ80State
{
	struct processor z80;
	uint64_t last_z80_tstates;
	uint64_t z80_tstates;

	// TODO Are these tables needed?
	// uint8_t sz53_table[0x100];
	// uint8_t parity_table[0x100];
	// uint8_t sz53p_table[0x100];

	/*! Captures the state from the global z80 interpreter and
		stores it in this struct */
	void Capture();

	/*! Restores the state stored in this struct into the global
		z80 interpreter */
	void Restore();
};

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
	int32_t z80_runtime;

	// Frame output
	int32_t SoundBufSize;
	unsigned width, height;
	int16_t sound_buf[0x10000];
	MDFN_Rect rects[FB_MAX_HEIGHT];

	// Input
	uint8_t input_buf;
	uint8_t NGPJoyLatch;

	// Processor state
	DuoTLCS900hState tlcs900hState;
	DuoZ80State z80State;

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

private:

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
