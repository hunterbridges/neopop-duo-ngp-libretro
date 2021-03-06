#include "neopop.h"
#include "sound.h"

#include "../state.h"
#include "../../duo/duo_instance.h"

// ----------------
// Instance methods
// ----------------

void neopop_sound_t::SetEnable(bool set)
{
   schipenable = set;
   if(!set)
      apu.reset();
}

int32_t neopop_sound_t::Flush(int16_t *SoundBuf, const int32_t MaxSoundFrames)
{
   int32_t FrameCount = 0;

   apu.end_frame(ngpc_soundTS >> 1);

   buf.end_frame(ngpc_soundTS >> 1);

   if(SoundBuf)
      FrameCount = buf.read_samples(SoundBuf, MaxSoundFrames * 2) / 2;
   else
      buf.clear();

   return(FrameCount);
}

void neopop_sound_t::RedoVolume(void)
{
   apu.output(buf.center(), buf.left(), buf.right());
   apu.volume(0.30);
   synth.volume(0.40);
}

void neopop_sound_t::Init(void)
{
   SetSoundRate(0);
   buf.clock_rate((long)(3072000));

   RedoVolume();
   buf.bass_freq(20);
}

bool neopop_sound_t::SetSoundRate(uint32_t rate)
{
   buf.set_sample_rate(rate?rate:44100, 60);
   return(true);
}

void neopop_sound_t::Write_SoundChipLeft(uint8_t data)
{
   if(schipenable)
      apu.write_data_left(ngpc_soundTS >> 1, data);
}

void neopop_sound_t::Write_SoundChipRight(uint8_t data)
{
   if(schipenable)
      apu.write_data_right(ngpc_soundTS >> 1, data);
}

void neopop_sound_t::dac_write_left(uint8_t data)
{
   CurrentDACLeft = data;

   synth.offset_inline(ngpc_soundTS >> 1, CurrentDACLeft - LastDACLeft, buf.left());

   LastDACLeft = data;
}

void neopop_sound_t::dac_write_right(uint8_t data)
{
   CurrentDACRight = data;

   synth.offset_inline(ngpc_soundTS >> 1, CurrentDACRight - LastDACRight, buf.right());

   LastDACRight = data;
}

// --------------
// Mednafen Hooks
// --------------

void MDFNNGPCSOUND_SetEnable(bool set)
{
   neopop_sound_t *sound = DuoInstance::currentInstance->sound;
   sound->SetEnable(set);
}

int neopop_sound_t::StateAction(void *data, int load, int data_only)
{
   T6W28_ApuState *sn_state;

   if(!load)
      sn_state = apu.save_state();
   else
      sn_state = (T6W28_ApuState *)malloc(sizeof(T6W28_ApuState));

   SFORMAT StateRegs[] =
   {
      SFVAR(CurrentDACLeft),
      SFVAR(CurrentDACRight),

      SFVAR(schipenable),

      SFARRAY32N(sn_state->delay, 4, "Delay"),
      SFARRAY32N(sn_state->volume_left, 4, "VolumeLeft"),
      SFARRAY32N(sn_state->volume_right, 4, "VolumeRight"),
      SFARRAY32N(sn_state->sq_period, 3, "SQPeriod"),
      SFARRAY32N(sn_state->sq_phase, 3, "SQPhase"),
      SFVARN(sn_state->noise_period, "NPeriod"),
      SFVARN(sn_state->noise_shifter, "NShifter"),
      SFVARN(sn_state->noise_tap, "NTap"),
      SFVARN(sn_state->noise_period_extra, "NPeriodExtra"),
      SFVARN(sn_state->latch_left, "LatchLeft"),
      SFVARN(sn_state->latch_right, "LatchRight"),
      SFEND
   };

   if(!MDFNSS_StateAction(data, load, data_only, StateRegs, "SND", false))
   {
      free(sn_state);
      return 0;
   }

   if(load)
   {
      buf.clear();
      apu.load_state(sn_state);

      LastDACLeft = CurrentDACLeft;
      LastDACRight = CurrentDACRight;
   }

   free(sn_state);
   return 1;
}
