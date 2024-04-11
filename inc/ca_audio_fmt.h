

#ifndef CA_AUDIO_H_

#define CA_AUDIO_H_


// An enum representing different audio formats CAudio is able to represent (each value is its size in bytes)
typedef enum
{
  CA_FMT_NONE = 0,  // Undefined/unknown format

  CA_FMT_I16  = 2,  // 16 bit signed integer
  CA_FMT_F32  = 4,  // 32 bit float
}
audio_fmt_t;


#endif // CA_AUDIO_H_
