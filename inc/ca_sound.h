


#ifndef CA_SOUND_H_

#define CA_SOUND_H_


#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "ca_audio_fmt.h"
#include "ca_result.h"


// A union that points to either int16_t data or float data (foo.i foo.f respectively) or as a void* (foo.v)
typedef union
{
  int16_t* i;
  float* f;
  void* v;
}
audio_data_t;

// A structure containing audio data
typedef struct
{
  audio_data_t data;  // A pointer to the sound's audio data (pointer value should only be modified by CAudio fundtions, indexed values can be modified)
  size_t num_samples; // The number of samples in the sound's audio data (should only be modifirf by CAudio functions)
  size_t cur_sample;  // The current index of the sound's playing position (must be between 0 and num_samples)

  audio_fmt_t fmt;    // The sound's audio format (should only be modified by CAudio functions)

  bool is_playing;    // True if the sound is being played in a mixer, false if not (should only be modified by CAudio functions)
}
sound_t;


// Initializes the sound
ca_result_t caInitSound(sound_t* p_sound, size_t num_samples, audio_fmt_t fmt);

// Destroys the sound
void caDestroySound(sound_t* p_sound);


// Copies the first sound's data to the second (the second should not contain any data)
ca_result_t caSoundCopy(sound_t* src, sound_t* dst);

// Converts the sound's audio data from int16 to float (the sound's audio data must be in int16 format)
ca_result_t caSoundToF(sound_t* p_sound);

// Converts the sound's audio date from flaot to int16 (the sound's audio data must be in float format)
ca_result_t caSoundToI(sound_t* p_sound);

// The size of the sound's audio data in bytes
size_t caSoundSize(sound_t* p_sound);

// Moves the sound's time to the given seconds
void caSoundSetSeconds(sound_t* p_sound, float seconds, uint32_t sample_rate, uint16_t num_channels);


#endif // CA_SOUND_H_
