# CAudio

A lightweight library that allows reading modifying and writing to WAV files, as well as playing multiple sounds using PortAudio

## Installation

CAudio uses PortAudio as a dependency. Before compiling, PortAudio's lib binary should be in 'dep/lib' and the header (portaudio.h) shoud be in 'dep/inc'

Once that is done, run 'make' in the terminal which should compile the library.

Run 'make clean' to remove the build directory.

## Example

```c
mixer_t mixer;
caMixerCreate(&mixer, CA_FMT_F32, 48000.0, 2, 2048);
caMixerBegin(mixer);

sound_t sound;
caMediaLoadWAV(&sound, "example_song.wav");

bool playing;
caMixerPlaySound(mixer, sound, &playing);

while (playing) {}

caSoundDestroy(&sound);
caMixerDestroy(mixer);
```

## License

[MIT](https://choosealicense.com/licenses/mit/)
