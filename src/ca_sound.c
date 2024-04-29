
#include "ca_sound.h"

#include <stdlib.h>
#include <string.h>


CA_Result caSoundCreate(CA_Sound* sound, size_t num_samples, CA_AudioFormat fmt)
{
  // Allocate the sound's data
  sound->_data = (void*)malloc(num_samples * fmt);

  if (!sound->_data)
  {
    caSoundDestroy(sound);
    return CA_ERR_ALLOC;
  }

  // Initialize remaining data
  sound->_num_samples = num_samples;

  sound->_fmt = fmt;

  // Done
  return CA_SUCCESS;
}

void caSoundDestroy(CA_Sound* sound)
{
  // Free the sound's data if it has any
  if (sound->_data)
  {
    free(sound->_data);
  }

  // Uninitialize the sound
  sound->_data = NULL;

  sound->_num_samples = 0;

  sound->_fmt = CA_FMT_NONE;
}


CA_Result caSoundCopy(CA_Sound* dst, const CA_Sound* src)
{
  // Allocate the dst data
  dst->_data = (void*)malloc(src->_num_samples * src->_fmt);

  if (!dst->_data)
  {
    return CA_ERR_ALLOC;
  }

  // Copy remaining values
  dst->_num_samples = src->_num_samples;

  dst->_fmt = src->_fmt;

  // Done
  return CA_SUCCESS;
}

CA_Result caSoundToF(CA_Sound* sound)
{
  // Allocate the new data
  float* new_data = (float*)malloc(sound->_num_samples * sizeof(float));

  if (!new_data)
    return CA_ERR_ALLOC;

  // Convert all samples
  int16_t* old_data = (int16_t*)sound->_data;

  for (size_t i = 0; i < sound->_num_samples; i++)
    new_data[i] = (float)old_data[i] / (float)INT16_MAX;

  // Free old data and swap
  free(sound->_data);

  sound->_data = (void*)new_data;

  // Done
  return CA_SUCCESS;
}

CA_Result caSoundToI(CA_Sound* sound)
{
  // Allocate the new data
  int16_t* new_data = (int16_t*)malloc(sound->_num_samples * sizeof(int16_t));

  if (!new_data)
    return CA_ERR_ALLOC;

  // Convert all samples
  float* old_data = (float*)sound->_data;

  for (size_t i = 0; i < sound->_num_samples; i++)
    new_data[i] = (int16_t)(old_data[i] * INT16_MAX);

  // Free old data and swap
  free(sound->_data);

  sound->_data = (void*)new_data;

  // Done
  return CA_SUCCESS;
}

CA_AudioFormat caSoundGetFormat(const CA_Sound* sound)
{
  return sound->_fmt;
}

void* caSoundGetData(const CA_Sound* sound)
{
  return sound->_data;
}

size_t caSoundGetNumSamples(const CA_Sound* sound)
{
  return sound->_num_samples;
}

size_t caSoundGetSize(const CA_Sound* sound)
{
  return sound->_num_samples * sound->_fmt;
}
