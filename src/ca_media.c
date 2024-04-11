

#include "ca_media.h"

#include <stdio.h>


// A struct representing a WAV file header and the first subchunk
typedef struct
{
  // Header
  uint32_t head;          // Must be "RIFF"
  uint32_t file_size;     // (File size in bytes) - 8
  uint32_t wav_head;      // Must be "WAVE"

  // Subchunk 1
  uint32_t fmt_head;      // Must be "fmt "
  uint32_t chunk_size;    // Subchunk 1 size (should be 16)
  uint16_t format;        // Audio format (1 for integer, 3 for float)
  uint16_t num_channels;  // The number of channels
  uint32_t sample_rate;   // The audio data's sample rate
  uint32_t byte_rate;     // The rate at which bytes are processed (sample_rate * num_channels * sample_size)
  uint16_t alignment;     // The sample's alignment (num_channels * sample_size)
  uint16_t sample_size_b; // The sample size in bits (sample_size * 8)
}
wav_head_t;

// A struct representing a chunk header
typedef struct
{
  uint32_t head;  // The chunk's header
  uint32_t size;  // The chunk's size (excluding the header)
}
chunk_head_t;


// File chunk headers
#define CA_RIFF_HEAD  0x46464952  // "RIFF"
#define CA_WAVE_HEAD  0x45564157  // "WAVE"
#define CA_FMT_HEAD   0x20746D66  // "fmt "
#define CA_DATA_HEAD  0x61746164  // "data"


ca_result_t caMediaGetInfoWAV(wav_info_t* p_wav_info, const char* path)
{
  // Open the file
  FILE* f = fopen(path, "rb");

  if (!f)
    return CA_ERR_FILE;

  // Read the file's header
  wav_head_t wav_head;

  if (fread(&wav_head, sizeof(wav_head_t), 1, f) != 1)
  {
    fclose(f);
    return CA_ERR_IN;
  }

  fclose(f);

  // Ensure the header is correct
  if (!(wav_head.head == CA_RIFF_HEAD && wav_head.wav_head == CA_WAVE_HEAD && wav_head.fmt_head == CA_FMT_HEAD))
    return CA_ERR_FTYPE;

  // Get the audio format
  if (wav_head.format == 1 && wav_head.sample_size_b == 16)
    p_wav_info->fmt = CA_FMT_I16;
  else if (wav_head.format == 3 && wav_head.sample_size_b == 32)
    p_wav_info->fmt = CA_FMT_F32;
  else
    return CA_ERR_FTYPE;

  // Fill the remaining values
  p_wav_info->sample_rate = wav_head.sample_rate;
  p_wav_info->num_channels = wav_head.num_channels;

  // Done
  return CA_SUCCESS;
}

ca_result_t caMediaLoadWAV(sound_t* p_sound, const char* path, wav_info_t* p_wav_info)
{
  // Open the file
  FILE* f = fopen(path, "rb");

  if (!f)
    return CA_ERR_FILE;

  // Read the file's header
  wav_head_t wav_head;

  if (fread(&wav_head, sizeof(wav_head_t), 1, f) != 1)
  {
    fclose(f);
    return CA_ERR_IN;
  }

  // Ensure the header is correct
  if (!(wav_head.head == CA_RIFF_HEAD && wav_head.wav_head == CA_WAVE_HEAD && wav_head.fmt_head == CA_FMT_HEAD))
  {
    fclose(f);
    return CA_ERR_FTYPE;
  }

  // Read to the data chunk
  chunk_head_t chunk_head;

  while (fread(&chunk_head, sizeof(chunk_head_t), 1, f) == 1 && chunk_head.head != CA_DATA_HEAD)
    fseek(f, chunk_head.size, SEEK_CUR);

  if (chunk_head.head != CA_DATA_HEAD)
  {
    fclose(f);
    return CA_ERR_IN;
  }

  // Get the audio format
  audio_fmt_t fmt;

  if (wav_head.format == 1 && wav_head.sample_size_b == 16)
    fmt = CA_FMT_I16;
  else if (wav_head.format == 3 && wav_head.sample_size_b == 32)
    fmt = CA_FMT_F32;
  else
  {
    fclose(f);
    return CA_ERR_FTYPE;
  }

  // Initialize the sound
  ca_result_t r = caInitSound(p_sound, chunk_head.size / fmt, fmt);

  if (r != CA_SUCCESS)
  {
    fclose(f);
    return r;
  }

  // Read the audio data
  if (fread(p_sound->data.v, 1, chunk_head.size, f) != chunk_head.size)
  {
    fclose(f);
    return CA_ERR_IN;
  }

  // Copy WAV info if requested
  if (p_wav_info)
  {
    p_wav_info->sample_rate = wav_head.sample_rate;
    p_wav_info->num_channels = wav_head.sample_rate;
    p_wav_info->fmt = fmt;
  }

  // Done
  fclose(f);

  return CA_SUCCESS;
}

ca_result_t caMediaSaveWAV(sound_t* p_sound, const char* path, uint32_t sample_rate, uint16_t num_channels)
{
  // Open the file
  FILE* f = fopen(path, "wb");

  if (!f)
    return CA_ERR_FILE;

  // Fill in the header
  size_t data_size = p_sound->num_samples * p_sound->fmt;

  wav_head_t wav_head = (wav_head_t)
  {
    // Header
    .head = CA_RIFF_HEAD,
    .file_size = sizeof(wav_head_t) + sizeof(chunk_head_t) + data_size - 8,
    .wav_head = CA_WAVE_HEAD,

    // Subchunk 1
    .fmt_head = CA_FMT_HEAD,
    .chunk_size = 16,
    .format = p_sound->fmt == CA_FMT_F32 ? 3 : 1,
    .num_channels = num_channels,
    .sample_rate = sample_rate,
    .byte_rate = sample_rate *num_channels * p_sound->fmt,
    .alignment = num_channels * p_sound->fmt,
    .sample_size_b = p_sound->fmt * 8,
  };

  // Fill the data chunk header
  chunk_head_t chunk_head = (chunk_head_t)
  {
    .head = CA_DATA_HEAD,
    .size = data_size,
  };

  // Write the headers
  if (fwrite(&wav_head, sizeof(wav_head_t), 1, f) != 1)
  {
    fclose(f);
    return CA_ERR_OUT;
  }

  if (fwrite(&chunk_head, sizeof(chunk_head_t), 1, f) != 1)
  {
    fclose(f);
    return CA_ERR_OUT;
  }

  // Write the audio data
  if (fwrite(p_sound->data.v, p_sound->fmt, p_sound->num_samples, f) != p_sound->num_samples)
  {
    fclose(f);
    return CA_ERR_OUT;
  }

  // Done
  fclose(f);

  return CA_SUCCESS;
}
