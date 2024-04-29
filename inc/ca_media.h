
#ifndef CA_MEDIA_H_
#define CA_MEDIA_H_


#include "ca_result.h"
#include "ca_sound.h"
#include "ca_wav_info.h"


// Fills the WAV info with the given WAV file's info
CA_Result caMediaGetInfoWAV(CA_WavInfo* wav_info, const char* path);

// Initializes the sound from the given WAV file, initializing its wav info as well (wav_info is optional and will be filled if not null)
CA_Result caMediaLoadWAV(CA_Sound* sound, const char* path);

// Saves the sound as a WAV file in the given path
CA_Result caMediaSaveWAV(const CA_Sound* sound, const char* path, uint32_t sample_rate, uint16_t num_channels);


#endif // CA_MEDIA_H_
