

#ifndef CA_MIXER_H_

#define CA_MIXER_H_


#include "ca_audio_fmt.h"
#include "ca_result.h"
#include "ca_sound.h"
#include "ca_wav.h"

#include <stddef.h>
#include <stdint.h>


// A structure that holds and plays audio data
typedef struct mixer_t__* mixer_t;


// Creates a mixer with room for the given number of sounds
ca_result_t caMixerCreate(mixer_t* p_mixer, audio_fmt_t fmt, uint32_t sample_rate, uint16_t num_channels, uint32_t frames_per_buffer);

// Destroys the mixer and its sounds
void caMixerDestroy(mixer_t mixer);


// Plays the sound from the beginning. 'signal' is an optional parameter that will be set to true initially then false once the sound stops playing
ca_result_t caMixerPlaySound(mixer_t mixer, sound_t sound, bool* signal);

// Begins the mixer's stream
void caMixerBegin(mixer_t mixer);

// Copies the sound's wav info to the given one
void caMixerGetWavInfo(mixer_t mixer, wav_info_t* wav_info);


#endif
