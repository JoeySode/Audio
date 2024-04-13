


#ifndef CA_SOUND_H_

#define CA_SOUND_H_


#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "ca_audio_fmt.h"
#include "ca_result.h"
#include "ca_wav.h"


// A structure containing audio data
typedef struct sound_t__* sound_t;


// Creates the sound
ca_result_t caSoundCreate(sound_t* p_sound, size_t num_samples, audio_fmt_t fmt);

// Creates the sound with the given wav data
ca_result_t caSoundCreateEx(sound_t* p_sound, size_t num_samples, wav_info_t* wav_info);

// Destroys the sound
void caSoundDestroy(sound_t p_sound);


// Copies the first sound's data to the second (the second should not have been created)
ca_result_t caSoundCopy(sound_t src, sound_t* p_dst);

// Converts the sound's audio data from int16 to float (the sound's audio data must be in int16 format)
ca_result_t caSoundToF(sound_t sound);

// Converts the sound's audio date from float to int16 (the sound's audio data must be in float format)
ca_result_t caSoundToI(sound_t sound);

// The sound's audio format
audio_fmt_t caSoundGetFormat(sound_t sound);

// The sound's audio data
void* caSoundGetData(sound_t sound);

// The number of samples the sound holds
size_t caSoundGetNumSamples(sound_t sound);

// The size of the sound's audio data in bytes
size_t caSoundGetSize(sound_t sound);

// Copies the sound's wav info to the given one
void caSoundGetWavInfo(sound_t sound, wav_info_t* wav_info);

// Sets the sound's wav info to the given one
void caSoundSetWavInfo(sound_t sound, wav_info_t* wav_info);


#endif // CA_SOUND_H_
