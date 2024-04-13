

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

  wav_info_t wav_info;
}
sound_t__;


// A node in the mixer's sound queue
typedef struct sound_node_t
{
  void* data;
  size_t num_samples;
  size_t cur_sample;

  bool* signal;

  struct sound_node_t* next;
}
sound_node_t;

// A function that adds n samples of a given type
typedef void(*sample_add_fn_t)(void* dst, sound_node_t* node, size_t num_frames);

typedef struct mixer_t__
{
  sound_node_t* head;       // The head ot the sound queue

  PaStream* stream;         // The mixer's audio stream

  sample_add_fn_t adder_fn; // Function used to add samples to the audio stream

  wav_info_t wav_info;      // The mixer's wav info

  bool* dummy_signal;        // Used to prevent branching when using playing signals
  bool pa_initialized;      // True if the mixer has initialized PortAudio
}
mixer_t__;


// Static declarations

// Returns the audio format as the PortAudio enum variant
static PaSampleFormat caAudioFmtToPA(audio_fmt_t fmt);

// Stream callback function
static int caMixerAudioCallback(const void* src, void* dst, unsigned long num_frames, const struct PaStreamCallbackTimeInfo* time_info, PaStreamCallbackFlags flags, void* data);

// Removes any sounds that are finished playing from the mixer's queue
static inline void caMixerCleanQueue(mixer_t mixer);

// 32 bit float sample adder
static void caMixerSampleAdderF32(void* v_dst, sound_node_t* node, size_t num_samples);

// 16 bit integer sample adder
static void caMixerSampleAdderI16(void* v_dst, sound_node_t* node, size_t num_samples);

// Header function definitions

ca_result_t caMixerCreate(mixer_t* p_mixer, audio_fmt_t fmt, uint32_t sample_rate, uint16_t num_channels, uint32_t frames_per_buffer)
{
  // Allocate the mixer
  mixer_t__* mixer = (mixer_t__*)malloc(sizeof(mixer_t__));

  if (!mixer)
    return CA_ERR_ALLOC;

  // Allocate the sound queue
  mixer->head = (sound_node_t*)malloc(sizeof(sound_node_t));

  if (!mixer->head)
  {
    caMixerDestroy(mixer);
    return CA_ERR_ALLOC;
  }

  mixer->head->next = NULL;

  // Initialize PortAudio
  PaError pa_err = Pa_Initialize();

  if (pa_err != paNoError)
  {
    caMixerDestroy(mixer);
    return CA_ERR_PA;
  }

  mixer->pa_initialized = true;

  // Create audio stream
  pa_err = Pa_OpenDefaultStream(&mixer->stream, 0, (int)num_channels, caAudioFmtToPA(fmt), (double)sample_rate, frames_per_buffer, caMixerAudioCallback, mixer);

  if (pa_err != paNoError)
  {
    caMixerDestroy(mixer);
    return CA_ERR_PA;
  }

  // Finish initialization
  mixer->adder_fn = (fmt == CA_FMT_F32) ? caMixerSampleAdderF32 : caMixerSampleAdderI16;

  mixer->wav_info = (wav_info_t){ .sample_rate = sample_rate, .num_channels = num_channels, .fmt = fmt };

  // Done
  *p_mixer = mixer;

  return CA_SUCCESS;
}

void caMixerDestroy(mixer_t mixer)
{
  if (!mixer)
    return;

  // Free the sound queue
  sound_node_t* node = mixer->head;

  while (node != NULL)
  {
    sound_node_t* temp = node->next;

    free(node);
    node = temp;
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


ca_result_t caMixerPlaySound(mixer_t mixer, sound_t sound, bool* signal)
{
  // Create a new node
  sound_node_t* node = (sound_node_t*)malloc(sizeof(sound_node_t));

  if (!node)
    return CA_ERR_ALLOC;

  // Set up the node
  node->data = sound->data;
  node->num_samples = sound->num_samples;
  node->cur_sample = 0;

  if (signal != NULL)
  {
    node->signal = signal;
    *signal = true;
  }
  else
    node->signal = mixer->dummy_signal;

  // Place into the queue
  node->next = mixer->head->next;
  mixer->head->next = node;

  // Done
  return CA_SUCCESS;
}

void caMixerBegin(mixer_t mixer)
{
  Pa_StartStream(mixer->stream);
}


// Static definitions
static PaSampleFormat caAudioFmtToPA(audio_fmt_t fmt)
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

static int caMixerAudioCallback(const void* src, void* dst, unsigned long num_frames, const struct PaStreamCallbackTimeInfo* time_info, PaStreamCallbackFlags flags, void* data)
{
  mixer_t mixer = (mixer_t)data;
  size_t num_samples = num_frames * mixer->wav_info.num_channels;

  // Fill the output with silence
  memset(dst, 0, num_samples * mixer->wav_info.fmt);

  // Add all sounds in queue to the buffer
  sound_node_t* cur = mixer->head->next;

  while (cur != NULL)
  {
    mixer->adder_fn(dst, cur, num_samples);

    cur = cur->next;
  }

  // Clean up the queue
  caMixerCleanQueue(mixer);

  // Done
  return paContinue;
}

static inline void caMixerCleanQueue(mixer_t mixer)
{
  // Check all queue nodes from the beginning
  sound_node_t* cur = mixer->head;

  while (cur->next != NULL)
  {
    // Continue if the sound is not finished
    if (cur->next->cur_sample < cur->next->num_samples)
    {
      cur = cur->next;
      continue;
    }

    // Remove the sound
    sound_node_t* node = cur->next;

    *node->signal = false;

    cur->next = node->next;
    free(node);

  }
}

static void caMixerSampleAdderF32(void* v_dst, sound_node_t* node, size_t num_samples)
{
  float* dst = (float*)v_dst;
  float* src = (float*)node->data;

  // Find the number of samples to add
  size_t n = num_samples < (node->num_samples - node->cur_sample) ? num_samples : (node->num_samples - node->cur_sample);

  // Add the samples
  for (size_t i = 0; i < n; i++)
    dst[i] += src[node->cur_sample + i];

  // Increment the sample counter
  node->cur_sample += num_samples;
}

static void caMixerSampleAdderI16(void* v_dst, sound_node_t* node, size_t num_samples)
{
  int16_t* dst = (int16_t*)v_dst;
  int16_t* src = (int16_t*)node->data;

  // Find the number of samples to add
  size_t n = num_samples < (node->num_samples - node->cur_sample) ? num_samples : (node->num_samples - node->cur_sample);

  // Add the samples
  for (size_t i = 0; i < n; i++)
    dst[i] += src[node->cur_sample + i];

  // Increment the sample counter
  node->cur_sample += num_samples;
}
