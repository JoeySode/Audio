
#include "ca_media.h"

#include <stdio.h>


// A struct representing a WAV file header and the first subchunk
typedef struct CA_WavHeader
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
CA_WavHeader;

// A struct representing a chunk header
typedef struct CA_ChunkHeader
{
  uint32_t head;  // The chunk's header
  uint32_t size;  // The chunk's size (excluding the header)
}
CA_ChunkHeader;


// File chunk headers
#define CA_RIFF_HEAD  0x46464952  // "RIFF"
#define CA_WAVE_HEAD  0x45564157  // "WAVE"
#define CA_FMT_HEAD   0x20746D66  // "fmt "
#define CA_DATA_HEAD  0x61746164  // "data"


CA_Result caMediaGetInfoWAV(CA_WavInfo* wav_info, const char* path)
{
  // Open the file
  FILE* f = fopen(path, "rb");

  if (!f)
    return CA_ERR_FILE;

  // Read the file's header
  CA_WavHeader wav_head;

  if (fread(&wav_head, sizeof(CA_WavHeader), 1, f) != 1)
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
  {
    wav_info->fmt = CA_FMT_I16;
  }
  else if (wav_head.format == 3 && wav_head.sample_size_b == 32)
  {
    wav_info->fmt = CA_FMT_F32;
  }
  else
    return CA_ERR_FTYPE;

  // Fill the remaining values
  wav_info->sample_rate = wav_head.sample_rate;
  wav_info->num_channels = wav_head.num_channels;

  // Done
  return CA_SUCCESS;
}

CA_Result caMediaLoadWAV(CA_Sound* sound, const char* path)
{
  // Open the file
  FILE* f = fopen(path, "rb");

  if (!f)
    return CA_ERR_FILE;

  // Read the file's header
  CA_WavHeader wav_head;

  if (fread(&wav_head, sizeof(CA_WavHeader), 1, f) != 1)
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
  CA_ChunkHeader chunk_head;

  while (fread(&chunk_head, sizeof(CA_ChunkHeader), 1, f) == 1 && chunk_head.head != CA_DATA_HEAD)
    fseek(f, chunk_head.size, SEEK_CUR);

  if (chunk_head.head != CA_DATA_HEAD)
  {
    fclose(f);
    return CA_ERR_IN;
  }

  // Get the audio format
  CA_AudioFormat fmt;

  if (wav_head.format == 1 && wav_head.sample_size_b == 16)
  {
    fmt = CA_FMT_I16;
  }
  else if (wav_head.format == 3 && wav_head.sample_size_b == 32)
  {
    fmt = CA_FMT_F32;
  }
  else
  {
    fclose(f);
    return CA_ERR_FTYPE;
  }

  // Initialize the sound
  CA_Sound new_sound;

  CA_Result r = caSoundCreate(&new_sound, chunk_head.size / fmt, fmt);

  if (r != CA_SUCCESS)
  {
    fclose(f);
    return r;
  }

  // Read the audio data
  if (fread(new_sound._data, 1, chunk_head.size, f) != chunk_head.size)
  {
    fclose(f);
    return CA_ERR_IN;
  }

  // Done
  fclose(f);

  *sound = new_sound;

  return CA_SUCCESS;
}

CA_Result caMediaSaveWAV(const CA_Sound* sound, const char* path, uint32_t sample_rate, uint16_t num_channels)
{
  // Open the file
  FILE* f = fopen(path, "wb");

  if (!f)
    return CA_ERR_FILE;

  // Fill in the header
  size_t data_size = sound->_num_samples * sound->_fmt;

  CA_WavHeader wav_head = (CA_WavHeader)
  {
    // Header
    .head = CA_RIFF_HEAD,
    .file_size = sizeof(CA_WavHeader) + sizeof(CA_ChunkHeader) + data_size - 8,
    .wav_head = CA_WAVE_HEAD,

    // Subchunk 1
    .fmt_head = CA_FMT_HEAD,
    .chunk_size = 16,
    .format = sound->_fmt == CA_FMT_F32 ? 3 : 1,
    .num_channels = num_channels,
    .sample_rate = sample_rate,
    .byte_rate = sample_rate * num_channels * sound->_fmt,
    .alignment = num_channels * sound->_fmt,
    .sample_size_b = sound->_fmt * 8,
  };

  // Fill the data chunk header
  CA_ChunkHeader chunk_head = (CA_ChunkHeader)
  {
    .head = CA_DATA_HEAD,
    .size = data_size,
  };

  // Write the headers
  if (fwrite(&wav_head, sizeof(CA_WavHeader), 1, f) != 1)
  {
    fclose(f);
    return CA_ERR_OUT;
  }

  if (fwrite(&chunk_head, sizeof(CA_ChunkHeader), 1, f) != 1)
  {
    fclose(f);
    return CA_ERR_OUT;
  }

  // Write the audio data
  if (fwrite(sound->_data, sound->_fmt, sound->_num_samples, f) != sound->_num_samples)
  {
    fclose(f);
    return CA_ERR_OUT;
  }

  // Done
  fclose(f);

  return CA_SUCCESS;
}
