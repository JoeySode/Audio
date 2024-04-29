
#ifndef CA_MIXER_H_
#define CA_MIXER_H_


#include "ca_audio_fmt.h"
#include "ca_result.h"
#include "ca_sound.h"
#include "ca_wav_info.h"

#include <stddef.h>
#include <stdint.h>


// A structure representing an audio mixer/player
typedef struct CA_Mixer
{
  struct CA_SoundNode* _head;                             // The head ot the sound queue

  void* _stream;                                          // The mixer's audio stream

  void(*_adder_fn)(void*, struct CA_SoundNode*, size_t);  // Function used to add samples to the audio stream

  CA_WavInfo _wav_info;                                   // The mixer's wav data

  bool* _dummy_signal;                                    // Used to prevent branching when using playing signals
  bool _pa_initialized;                                   // True if the mixer has initialized PortAudio
}
CA_Mixer;


// Creates a mixer with room for the given number of sounds
CA_Result caMixerCreate(CA_Mixer* mixer, CA_AudioFormat fmt, uint32_t sample_rate, uint16_t num_channels, uint32_t frames_per_buffer);

// Destroys the mixer and its sounds
void caMixerDestroy(CA_Mixer* mixer);


// Plays the sound from the beginning. 'signal' is an optional parameter that will be set to true initially then false once the sound stops playing
CA_Result caMixerPlaySound(CA_Mixer* mixer, CA_Sound* sound, bool* signal);

// Plays the sound from the given seconds in. 'signal' is an optional parameter that will be set to true initially then false once the sound stops playing
CA_Result caMixerPlaySoundFrom(CA_Mixer* mixer, CA_Sound* sound, bool* signal, double seconds);

// Begins the mixer's stream
void caMixerBegin(CA_Mixer* mixer);

// Copies the sound's wav info to the given one
void caMixerGetWavInfo(const CA_Mixer* mixer, CA_WavInfo* wav_info);


#endif
