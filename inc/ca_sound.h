
#ifndef CA_SOUND_H_
#define CA_SOUND_H_


#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "ca_audio_fmt.h"
#include "ca_result.h"


// A structure representing a sound
typedef struct CA_Sound
{
  void* _data;
  size_t _num_samples;

  CA_AudioFormat _fmt;
}
CA_Sound;


// Creates the sound
CA_Result caSoundCreate(CA_Sound* sound, size_t num_samples, CA_AudioFormat fmt);

// Destroys the sound
void caSoundDestroy(CA_Sound* sound);


// Copies the first sound's data to the second (the second should not have been created)
CA_Result caSoundCopy(CA_Sound* dst, const CA_Sound* src);

// Converts the sound's audio data from int16 to float (the sound's audio data must be in int16 format)
CA_Result caSoundToF(CA_Sound* sound);

// Converts the sound's audio date from float to int16 (the sound's audio data must be in float format)
CA_Result caSoundToI(CA_Sound* sound);

// The sound's audio format
CA_AudioFormat caSoundGetFormat(const CA_Sound* sound);

// The sound's audio data
void* caSoundGetData(const CA_Sound* sound);

// The number of samples the sound holds
size_t caSoundGetNumSamples(const CA_Sound* sound);

// The size of the sound's audio data in bytes
size_t caSoundGetSize(const CA_Sound* sound);


#endif // CA_SOUND_H_
