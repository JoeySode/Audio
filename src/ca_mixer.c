

#include "ca_mixer.h"

#include "portaudio.h"
#include "ca_wav.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>


typedef struct sound_t__
{
  void* data;
  size_t num_samples;
  size_t cur_sample;

  wav_info_t wav_info;

  bool is_playing;
}
sound_t__;


// A function that adds n samples of a given type
typedef void(*sample_add_fn_t)(sound_t sound, void* dst, size_t num_frames);

typedef struct mixer_t__
{
  sound_t* p_sounds;        // The sound queue
  size_t num_sounds;        // The number of sounds in the queue
  size_t cap_sounds;        // The maximum capacity of the sound queue

  PaStream* stream;         // The mixer's audio stream

  sample_add_fn_t adder_fn; // Function used to add samples to the audio stream

  size_t num_channels;      // The number of channels in the stream's data
  size_t sample_size;       // The size of one of the mixer's samples in bytes (includes channels)

  bool pa_initialized;      // True if the mixer has initialized PortAudio
}
mixer_t__;


// Returns the audio format as the PortAudio enum variant
static PaSampleFormat caAudioFmtToPa(audio_fmt_t fmt)
{
  switch (fmt)
  {
    case CA_FMT_I16:
      return paInt16;

    case CA_FMT_F32:
      return paFloat32;

    default:
      return 0;
  }
}

// Stream callback function
static int caMixerAudioCallback(const void* src, void* dst, unsigned long num_frames, const struct PaStreamCallbackTimeInfo* time_info, PaStreamCallbackFlags flags, void* data)
{
  mixer_t mixer = (mixer_t)data;
  size_t num_vals= num_frames * mixer->num_channels;

  // Fill the output with silence
  memset(dst, 0, num_vals * mixer->sample_size);

  // Iterate through all sounds adding their data if they are playing
  for (size_t i = 0; i < mixer->num_sounds; i++)
  {
    sound_t sound = mixer->p_sounds[i];

    // Skip if not playing
    if (!sound->is_playing)
      continue;

    // Call the adder function (see caMixerAddSamples... functions below)
    mixer->adder_fn(sound, dst, num_vals);
  }

  // Remove any finished sound from the queue
  size_t i = 0;

  while (i < mixer->num_sounds)
  {
    // Skip if the sound is playing
    if (mixer->p_sounds[i]->is_playing)
    {
      i += 1;
      continue;
    }

    // Move all the succeeding sounds back to remove it from the queue
    memmove(&mixer->p_sounds[i], &mixer->p_sounds[i + 1], (mixer->num_sounds - i) * sizeof(sound_t*));
    mixer->num_sounds -= 1;
  }

  // Done
  return paContinue;
}

// Sample adding function for float
static void caMixerAddSamplesF32(sound_t sound, void* _out, size_t num_frames)
{
  float* out = (float*)_out;
  float* in = (float*)sound->data;

  // Calculate the number of samples to copy, ending the sound if we reach past its data
  size_t num_samples;
  size_t rem_samples = sound->num_samples - sound->cur_sample;

  if (rem_samples <= num_frames)
  {
    num_samples = rem_samples;
    sound->is_playing = false;
  }
  else
    num_samples = num_frames;

  // Add the data
  for (size_t i = 0; i < num_samples; i++)
    out[i] += in[sound->cur_sample + i];

  // Progress the pointer
  sound->cur_sample += num_frames;
}

// Sample adding function for int16_t
static void caMixerAddSamplesI16(sound_t sound, void* _out, size_t num_frames)
{
  int16_t* out = (int16_t*)_out;
  int16_t* in = (int16_t*)sound->data;

  // Calculate the number of samples to copy, ending the sound if we reach past its data
  size_t num_samples;
  size_t rem_samples = sound->num_samples - sound->cur_sample;

  if (rem_samples <= num_frames)
  {
    num_samples = rem_samples;
    sound->is_playing = false;
  }
  else
    num_samples = num_frames;

  // Add the data
  for (size_t i = 0; i < num_samples; i++)
    out[i] += in[sound->cur_sample + i];

  // Progress the pointer
  sound->cur_sample += num_frames;
}

// Returns a sample adder function for the given audio format
static sample_add_fn_t caAudioFmtGetSampleAdder(audio_fmt_t fmt)
{
  switch (fmt)
  {
    case CA_FMT_I16:
      return caMixerAddSamplesI16;

    case CA_FMT_F32:
      return caMixerAddSamplesF32;

    default:
      return NULL;
  }
}


ca_result_t caCreateMixer(mixer_t* p_mixer, size_t num_sounds, audio_fmt_t fmt, double sample_rate, int num_channels, unsigned int frames_per_buffer)
{
  // Allocate the mixer
  mixer_t__* mixer = (mixer_t__*)malloc(sizeof(mixer_t__));

  if (!mixer)
    return CA_ERR_ALLOC;

  // Allocate the sound queue
  mixer->p_sounds = (sound_t*)calloc(num_sounds, sizeof(sound_t*));

  if (!mixer->p_sounds)
  {
    caDestroyMixer(mixer);
    return CA_ERR_ALLOC;
  }

  // Initialize PortAudio
  PaError pa_err = Pa_Initialize();

  if (pa_err != paNoError)
  {
    caDestroyMixer(mixer);
    return CA_ERR_PA;
  }

  mixer->pa_initialized = true;

  // Create audio stream
  pa_err = Pa_OpenDefaultStream(&mixer->stream, 0, num_channels, caAudioFmtToPa(fmt), sample_rate, frames_per_buffer, caMixerAudioCallback, mixer);

  if (pa_err != paNoError)
  {
    caDestroyMixer(mixer);
    return CA_ERR_PA;
  }

  // Finish initialization
  mixer->num_sounds = 0;
  mixer->cap_sounds = num_sounds;

  mixer->adder_fn = caAudioFmtGetSampleAdder(fmt);

  mixer->num_channels = num_channels;
  mixer->sample_size = fmt;

  // Done
  *p_mixer = mixer;

  return CA_SUCCESS;
}

void caDestroyMixer(mixer_t mixer)
{
  if (!mixer)
    return;

  // Free the sounds
  if (mixer->p_sounds)
  {
    // Tell all sounds in queue that they are no longer being played
    for (size_t i = 0; i < mixer->num_sounds; i++)
    {
      if (mixer->p_sounds[i])
        mixer->p_sounds[i]->is_playing = false;
    }

    free(mixer->p_sounds);
  }

  // End the mixer's stream
  if (mixer->stream)
  {
    Pa_StopStream(mixer->stream);
    Pa_CloseStream(mixer->stream);
  }

  // Terminate PortAudio (only actually terminates if every other initialization is met with a termination)
  if (mixer->pa_initialized)
    Pa_Terminate();

  // Free the mixer
  free(mixer);
}


void caMixerBegin(mixer_t mixer)
{
  Pa_StartStream(mixer->stream);
}

void caMixerPlaySound(mixer_t mixer, sound_t sound)
{
  // Add the sound to the queue if it is not already
  if (!sound->is_playing)
  {
    mixer->p_sounds[mixer->num_sounds] = sound;
    mixer->num_sounds += 1;
  }

  // Play the sound
  sound->is_playing = true;
}

void caMixerStartSound(mixer_t mixer, sound_t sound)
{
  sound->cur_sample = 0;

  caMixerPlaySound(mixer, sound);
}
