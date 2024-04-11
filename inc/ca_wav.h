

#ifndef CA_WAV_H_

#define CA_WAV_H_


#include <stdint.h>

#include "ca_audio_fmt.h"


// A structure containing WAV file info not stored in a sound_t
typedef struct
{
  uint32_t sample_rate;   // The audio's sample rate
  uint16_t num_channels;  // The number of channels in the audio's data
  audio_fmt_t fmt;        // The wav file's audio format
}
wav_info_t;


#endif // CA_WAV_H_
