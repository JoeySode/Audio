

#ifndef CA_MEDIA_H_

#define CA_MEDIA_H_


#include "ca_result.h"
#include "ca_sound.h"
#include "ca_wav.h"


// Fills the WAV info with the given WAV file's info
ca_result_t caMediaGetInfoWAV(wav_info_t* p_wav_info, const char* path);

// Initializes the sound from the given WAV file (wav_info is optional and will be filled if not null)
ca_result_t caMediaLoadWAV(sound_t* p_sound, const char* path, wav_info_t* p_wav_info);

// Saves the sound as a WAV file in the given path
ca_result_t caMediaSaveWAV(sound_t* p_sound, const char* path, uint32_t sample_rate, uint16_t num_channels);


#endif // CA_MEDIA_H_
