
#include "ca_mixer.h"

#include "portaudio.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>


// A node in the mixer's sound queue
typedef struct CA_SoundNode
{
  void* data;
  size_t num_samples;
  size_t cur_sample;

  bool* signal;

  struct CA_SoundNode* next;
}
CA_SoundNode;

// A function that adds n samples of a given type
typedef void(*CA_AdderFn)(void* dst, CA_SoundNode* node, size_t num_frames);


// Static declarations

// Returns the audio format as the PortAudio enum variant
static PaSampleFormat caAudioFmtToPA(CA_AudioFormat fmt);

// Adds the sound to the audio queue
static CA_Result caMixerAddSound(CA_Mixer* mixer, CA_Sound* sound, bool* signal, size_t start);

// Stream callback function
static int caMixerAudioCallback(const void* src, void* dst, unsigned long num_frames, const struct PaStreamCallbackTimeInfo* time_info, PaStreamCallbackFlags flags, void* data);

// Removes any sounds that are finished playing from the mixer's queue
static inline void caMixerCleanQueue(CA_Mixer* mixer);

// 32 bit float sample adder
static void caMixerSampleAdderF32(void* v_dst, CA_SoundNode* node, size_t num_samples);

// 16 bit integer sample adder
static void caMixerSampleAdderI16(void* v_dst, CA_SoundNode* node, size_t num_samples);

// Header function definitions

CA_Result caMixerCreate(CA_Mixer* mixer, CA_AudioFormat fmt, uint32_t sample_rate, uint16_t num_channels, uint32_t frames_per_buffer)
{
  // Allocate the sound queue
  mixer->_head = (CA_SoundNode*)malloc(sizeof(CA_SoundNode));

  if (!mixer->_head)
  {
    caMixerDestroy(mixer);
    return CA_ERR_ALLOC;
  }

  mixer->_head->next = NULL;

  // Initialize PortAudio
  PaError pa_err = Pa_Initialize();

  if (pa_err != paNoError)
  {
    caMixerDestroy(mixer);
    return CA_ERR_PA;
  }

  mixer->_pa_initialized = true;

  // Create audio stream
  pa_err = Pa_OpenDefaultStream(&mixer->_stream, 0, (int)num_channels, caAudioFmtToPA(fmt), (double)sample_rate, frames_per_buffer, caMixerAudioCallback, mixer);

  if (pa_err != paNoError)
  {
    caMixerDestroy(mixer);
    return CA_ERR_PA;
  }

  // Finish initialization
  mixer->_adder_fn = fmt == CA_FMT_F32 ? caMixerSampleAdderF32 : caMixerSampleAdderI16;

  mixer->_wav_info = (CA_WavInfo){ .sample_rate = sample_rate, .num_channels = num_channels, .fmt = fmt };

  // Done
  return CA_SUCCESS;
}

void caMixerDestroy(CA_Mixer* mixer)
{
  // Free the sound queue
  CA_SoundNode* node = mixer->_head;

  while (node != NULL)
  {
    CA_SoundNode* temp = node->next;

    free(node);
    node = temp;
  }

  // End the mixer's stream
  if (mixer->_stream)
  {
    Pa_StopStream(mixer->_stream);
    Pa_CloseStream(mixer->_stream);
  }

  // Terminate PortAudio (only actually terminates if every other initialization is met with a termination)
  if (mixer->_pa_initialized)
    Pa_Terminate();
}


CA_Result caMixerPlaySound(CA_Mixer* mixer, CA_Sound* sound, bool* signal)
{
  return caMixerAddSound(mixer, sound, signal, 0);
}

CA_Result caMixerPlaySoundFrom(CA_Mixer* mixer, CA_Sound* sound, bool* signal, double seconds)
{
  const size_t start = (size_t)((double)mixer->_wav_info.sample_rate * (double)mixer->_wav_info.num_channels * seconds);

  return caMixerAddSound(mixer, sound, signal, start);
}

void caMixerBegin(CA_Mixer* mixer)
{
  Pa_StartStream(mixer->_stream);
}

void caMixerGetWavInfo(const CA_Mixer* mixer, CA_WavInfo* wav_info)
{
  *wav_info = mixer->_wav_info;
}


// Static definitions
static PaSampleFormat caAudioFmtToPA(CA_AudioFormat fmt)
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

static CA_Result caMixerAddSound(CA_Mixer* mixer, CA_Sound* sound, bool* signal, size_t start)
{
  // Create a new node
  CA_SoundNode* node = (CA_SoundNode*)malloc(sizeof(CA_SoundNode));

  if (!node)
    return CA_ERR_ALLOC;

  // Set up the node
  node->data = sound->_data;
  node->num_samples = sound->_num_samples;
  node->cur_sample = start;

  if (signal != NULL)
  {
    node->signal = signal;
    *signal = true;
  }
  else
    node->signal = mixer->_dummy_signal;

  // Place into the queue
  node->next = mixer->_head->next;
  mixer->_head->next = node;

  // Done
  return CA_SUCCESS;
}

static int caMixerAudioCallback(const void* src, void* dst, unsigned long num_frames, const struct PaStreamCallbackTimeInfo* time_info, PaStreamCallbackFlags flags, void* data)
{
  CA_Mixer* mixer = (CA_Mixer*)data;
  size_t num_samples = num_frames * mixer->_wav_info.num_channels;

  // Fill the output with silence
  memset(dst, 0, num_samples * mixer->_wav_info.fmt);

  // Add all sounds in queue to the buffer
  CA_SoundNode* cur = mixer->_head->next;

  while (cur != NULL)
  {
    mixer->_adder_fn(dst, cur, num_samples);

    cur = cur->next;
  }

  // Clean up the queue
  caMixerCleanQueue(mixer);

  // Done
  return paContinue;
}

static inline void caMixerCleanQueue(CA_Mixer* mixer)
{
  // Check all queue nodes from the beginning
  CA_SoundNode* cur = mixer->_head;

  while (cur->next != NULL)
  {
    // Continue if the sound is not finished
    if (cur->next->cur_sample < cur->next->num_samples)
    {
      cur = cur->next;
      continue;
    }

    // Remove the sound
    CA_SoundNode* node = cur->next;

    *node->signal = false;

    cur->next = node->next;
    free(node);

  }
}

static void caMixerSampleAdderF32(void* v_dst, CA_SoundNode* node, size_t num_samples)
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

static void caMixerSampleAdderI16(void* v_dst, CA_SoundNode* node, size_t num_samples)
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
