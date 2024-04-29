
#ifndef CA_WAV_H_
#define CA_WAV_H_


#include <stdint.h>

#include "ca_audio_fmt.h"


// A structure containing WAV file info not stored in a sound_t
typedef struct CA_WavInfo
{
  uint32_t sample_rate;   // The audio's sample rate
  uint16_t num_channels;  // The number of channels in the audio's data
  CA_AudioFormat fmt;        // The wav file's audio format
}
CA_WavInfo;


#endif // CA_WAV_H_
