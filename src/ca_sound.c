

#include "ca_sound.h"

#include <stdlib.h>
#include <string.h>


ca_result_t caInitSound(sound_t* p_sound, size_t num_samples, audio_fmt_t fmt)
{
  // Allocate the audio data
  p_sound->data.v = (void*)calloc(num_samples, fmt);

  if (!p_sound->data.v)
    return CA_ERR_ALLOC;

  // Initialize remaining values
  p_sound->num_samples = num_samples;
  p_sound->cur_sample = 0;

  p_sound->fmt = fmt;

  p_sound->is_playing = false;

  // Done
  return CA_SUCCESS;
}

void caDestroySound(sound_t* p_sound)
{
  if (!p_sound)
    return;

  // Free the sound's data
  if (p_sound->data.v)
  {
    free(p_sound->data.v);
    p_sound->data.v = NULL;
  }

  // Finish deinitialization
  p_sound->num_samples = 0;
  p_sound->cur_sample = 0;

  p_sound->fmt = CA_FMT_NONE;

  p_sound->is_playing = false;
}


ca_result_t caSoundCopy(sound_t* src, sound_t* dst)
{
  // Allocate the dst's data
  size_t data_size = src->num_samples * src->fmt;

  dst->data.v = (void*)malloc(data_size);

  if (!dst->data.v)
    return CA_ERR_ALLOC;

  // Copy data
  memcpy(dst->data.v, src->data.v, data_size);

  dst->num_samples = src->num_samples;
  dst->cur_sample = src->cur_sample;

  dst->fmt = src->fmt;

  dst->is_playing = false;

  // Done
  return CA_SUCCESS;
}

ca_result_t caSoundToF(sound_t* p_sound)
{
  // Allocate the new data
  float* new_data = (float*)malloc(p_sound->num_samples * sizeof(float));

  if (!new_data)
    return CA_ERR_ALLOC;

  // Copy the old data over converting to float
  for (size_t i = 0; i < p_sound->num_samples; i++)
    new_data[i] = (float)p_sound->data.i[i] / -32768.0f;

  // Swap data and free old
  free(p_sound->data.v);

  p_sound->data.f = new_data;

  // Done
  return CA_SUCCESS;
}

ca_result_t caSoundToI(sound_t* p_sound)
{
  // Allocate the new data
  int16_t* new_data = (int16_t*)malloc(p_sound->num_samples * sizeof(int16_t));

  if (!new_data)
    return CA_ERR_ALLOC;

  // Copy the old data over converting to float
  for (size_t i = 0; i < p_sound->num_samples; i++)
    new_data[i] = (int16_t)(p_sound->data.f[i] * 32767.0f);

  // Swap data and free old
  free(p_sound->data.v);

  p_sound->data.i = new_data;

  // Done
  return CA_SUCCESS;
}

size_t caSoundSize(sound_t* p_sound)
{
  return p_sound->num_samples * p_sound->fmt;
}

void caSoundSetSeconds(sound_t* p_sound, float seconds, uint32_t sample_rate, uint16_t num_channels)
{
  p_sound->cur_sample = (size_t)((float)(sample_rate * num_channels) * seconds);
}
