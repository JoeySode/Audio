

#include "ca_sound.h"

#include <stdlib.h>
#include <string.h>


typedef struct sound_t__
{
  void* data;
  size_t num_samples;

  wav_info_t wav_info;
}
sound_t__;


ca_result_t caSoundCreate(sound_t* p_sound, size_t num_samples, audio_fmt_t fmt)
{
  wav_info_t wav_info = (wav_info_t){ .sample_rate = 0, .num_channels = 0, .fmt = fmt };

  return caSoundCreateEx(p_sound, num_samples, &wav_info);
}

ca_result_t caSoundCreateEx(sound_t* p_sound, size_t num_samples, wav_info_t* wav_info)
{
  // Allocate the sound
  sound_t__* sound = (sound_t__*)malloc(sizeof(sound_t__));

  if (!sound)
    return CA_ERR_ALLOC;

  // Allocate the sound's data
  sound->data = (void*)malloc(num_samples * wav_info->fmt);

  if (!sound->data)
  {
    caSoundDestroy(sound);
    return CA_ERR_ALLOC;
  }

  // Initialize remaining data
  sound->num_samples = num_samples;

  sound->wav_info = *wav_info;

  // Done
  *p_sound = sound;

  return CA_SUCCESS;
}

void caSoundDestroy(sound_t sound)
{
  if (!sound)
    return;

  if (sound->data)
    free(sound->data);

  free(sound);
}


ca_result_t caSoundCopy(sound_t src, sound_t* p_dst)
{
  // Create the new sound
  sound_t sound;
  ca_result_t r = caSoundCreateEx(&sound, src->num_samples, &src->wav_info);

  if (r != CA_SUCCESS)
    return r;

  // Copy the data
  memcpy(sound->data, src->data, caSoundGetSize(src));

  // Done
  *p_dst = sound;

  return CA_SUCCESS;
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

void* caSoundGetData(sound_t sound)
{
  return sound->data;
}

size_t caSoundGetNumSamples(sound_t sound)
{
  return sound->num_samples;
}

size_t caSoundGetSize(sound_t sound)
{
  return sound->num_samples * sound->wav_info.fmt;
}

void caSoundGetWavInfo(sound_t sound, wav_info_t* wav_info)
{
  *wav_info = sound->wav_info;
}

void caSoundSetWavInfo(sound_t sound, wav_info_t* wav_info)
{
  sound->wav_info = *wav_info;
}
