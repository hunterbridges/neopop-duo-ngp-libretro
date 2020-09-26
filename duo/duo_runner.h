// ----------
// NeoPop Duo
// ----------

#ifndef DUO_RUNNER_H
#define DUO_RUNNER_H

#include <stdint.h>
#include <libretro.h>
#include <string>
#include "duo_instance.h"

class DuoRunner
{
public:
	enum AVMode
	{
		AV_P1_ONLY,
		AV_P2_ONLY,
		AV_BOTH_PLAYERS
	};

	enum AVLayout
	{
		AV_LAYOUT_H,
		AV_LAYOUT_V,
		AV_LAYOUT_SWITCH_H,
		AV_LAYOUT_SWITCH_V,

		AV_LAYOUT_DIR_MASK = 0x1,
		AV_LAYOUT_SWITCH_MASK = 0x2
	};

	int RETRO_SAMPLE_RATE = 44100;
	int RETRO_PIX_BYTES = 2;
	int RETRO_PIX_DEPTH = 15;

	std::string retro_base_name;
	char retro_base_directory[1024];
	char retro_save_directory[1024];

	MDFNGI game_info;

	int instance_count = 1;

	uint32_t setting_ngp_language = 0;

	bool update_video;
	bool update_audio;
	uint64_t video_frames;
	uint64_t audio_frames;

	AVMode videoMixMode = AV_P1_ONLY;
	AVLayout videoLayout = AV_LAYOUT_H;
	AVMode audioMixMode = AV_P1_ONLY;

	struct retro_perf_callback perf_cb;
	retro_get_cpu_features_t perf_get_cpu_features_cb;
	retro_log_printf_t log_cb;
	retro_video_refresh_t video_cb;
	retro_audio_sample_batch_t audio_batch_cb;
	retro_environment_t environ_cb;
	retro_input_poll_t input_poll_cb;
	retro_input_state_t input_state_cb;

	bool failed_init;
	bool libretro_supports_bitmasks;
	bool overscan;

	void Initialize(void);
	void Deinitialize(void);

	void Reset(void);

	bool LoadGame(const struct retro_game_info *info);
	void UnloadGame();
	void Run(void);

	void GetSystemInfo(struct retro_system_info *info);
	void GetAvInfo(struct retro_system_av_info *info);

	int SaveStateAction(StateMem *sm, int load, int data_only);

private:
	void MixFrameAV(void);

	void UpdateInput(void);

	void SetBasename(const char *path);

	void CheckSystemSpecs(void);
	void CheckColorDepth(void);
	void CheckVariables(void);

	void InitGameInfo();
	bool LoadGameFromFile(const char *name);
	bool LoadGameFromData(const uint8_t *data, size_t size);

public:
	static DuoRunner shared;
};

extern "C"
{
	const char *DuoRunner_GetBaseDir();
	const char *DuoRunner_GetSaveDir();
	uint32_t DuoRunner_GetLangSetting();
};

#endif
