

#include "ca_sound.h"

#include <stdlib.h>
#include <string.h>


typedef struct sound_t__
{
  void* data;
  size_t num_samples;
  size_t cur_sample;

  wav_info_t wav_info;

  bool is_playing;
}
sound_t__;


ca_result_t caCreateSound(sound_t* p_sound, size_t num_samples, audio_fmt_t fmt)
{
  wav_info_t wav_info = (wav_info_t){ .sample_rate = 0, .num_channels = 0, .fmt = fmt };

  return caCreateSoundEx(p_sound, num_samples, &wav_info);
}

ca_result_t caCreateSoundEx(sound_t* p_sound, size_t num_samples, wav_info_t* wav_info)
{
  // Allocate the sound
  sound_t__* sound = (sound_t__*)malloc(sizeof(sound_t__));

  if (!sound)
    return CA_ERR_ALLOC;

  // Allocate the sound's data
  sound->data = (void*)calloc(num_samples, wav_info->fmt);

  if (!sound->data)
  {
    caDestroySound(sound);
    return CA_ERR_ALLOC;
  }

  // Initialize remaining data
  sound->num_samples = num_samples;
  sound->cur_sample = 0;

  sound->wav_info = *wav_info;

  sound->is_playing = false;

  // Done
  *p_sound = sound;

  return CA_SUCCESS;
}

void caDestroySound(sound_t sound)
{
  if (!sound)
    return;

  if (sound->data)
    free(sound->data);

  free(sound);
}


ca_result_t caSoundCopy(sound_t src, sound_t* p_dst)
{
  return caCreateSoundEx(p_dst, src->num_samples, &src->wav_info);
}

ca_result_t caSoundToF(sound_t sound)
{
  // Allocate the new data
  float* new_data = (float*)malloc(sound->num_samples * sizeof(float));

  if (!new_data)
    return CA_ERR_ALLOC;

  // Convert all samples
  int16_t* old_data = (int16_t*)sound->data;

  for (size_t i = 0; i < sound->num_samples; i++)
    new_data[i] = (float)old_data[i] / (float)INT16_MAX;

  // Free old data and swap
  free(sound->data);

  sound->data = (void*)new_data;

  // Done
  return CA_SUCCESS;
}

ca_result_t caSoundToI(sound_t sound)
{
  // Allocate the new data
  int16_t* new_data = (int16_t*)malloc(sound->num_samples * sizeof(int16_t));

  if (!new_data)
    return CA_ERR_ALLOC;

  // Convert all samples
  float* old_data = (float*)sound->data;

  for (size_t i = 0; i < sound->num_samples; i++)
    new_data[i] = (int16_t)(old_data[i] * INT16_MAX);

  // Free old data and swap
  free(sound->data);

  sound->data = (void*)new_data;

  // Done
  return CA_SUCCESS;
}

audio_fmt_t caSoundGetFormat(sound_t sound)
{
  return sound->wav_info.fmt;
}

size_t caSoundGetSize(sound_t sound)
{
  return sound->num_samples * sound->wav_info.fmt;
}

bool caSoundIsPlaying(sound_t sound)
{
  return sound->is_playing;
}

void caSoundGetWavInfo(sound_t sound, wav_info_t* wav_info)
{
  *wav_info = sound->wav_info;
}

void caSoundSetSeconds(sound_t sound, float seconds)
{
  sound->cur_sample = (size_t)(seconds * (float)(sound->wav_info.sample_rate * sound->wav_info.num_channels));
}

void caSoundSetWavInfo(sound_t sound, wav_info_t* wav_info)
{
  sound->wav_info = *wav_info;
}
