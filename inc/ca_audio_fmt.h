
#ifndef CA_AUDIO_FORMAT_H_
#define CA_AUDIO_FORMAT_H_


// An enum representing different audio formats CAudio is able to represent (each value is its size in bytes)
typedef enum CA_AudioFormat
{
  CA_FMT_NONE = 0x00, // Undefined/unknown format

  CA_FMT_I16  = 0x02, // 16 bit signed integer
  CA_FMT_F32  = 0x04, // 32 bit float
}
CA_AudioFormat;


#endif // CA_AUDIO_FORMAT_H_
