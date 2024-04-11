

#ifndef CA_MIXER_H_

#define CA_MIXER_H_


#include "ca_audio_fmt.h"
#include "ca_result.h"
#include "ca_sound.h"

#include <stddef.h>
#include <stdint.h>


// A structure that holds and plays audio data
typedef struct mixer_t__* mixer_t;


// Creates a mixer with room for the given number of sounds
ca_result_t caCreateMixer(mixer_t* p_mixer, size_t num_sounds, audio_fmt_t fmt, double sample_rate, int num_channels, unsigned int frames_per_buffer);

// Destroys the mixer and its sounds
void caDestroyMixer(mixer_t mixer);


// Begins the mixer's stream
void caMixerBegin(mixer_t mixer);

// Plays the sound from its current point
void caMixerPlaySound(mixer_t mixer, sound_t sound);

// Plays the sound from the beginning
void caMixerStartSound(mixer_t mixer, sound_t sound);


#endif
