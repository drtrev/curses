#include "sound/sound.h"
#include "outverbose.h"

Sound::Sound()
{
  alGetError(); // clear error code

  playDevice = NULL; // needed for SoundDev and checkAlc
}

Sound::~Sound()
{
}

void Sound::initOutput(Outverbose &o)
{
  out = &o;
}

void Sound::setPlayDevice(ALCdevice *dev)
{
  playDevice = dev;
}

ALenum Sound::check(const char* errorMaker)
  // check for an error and pass a string to say where we are in the program
  // errorMaker - what made the error?
{
  ALenum error = alGetError();

  if(error != AL_NO_ERROR) {
    *out << VERBOSE_LOUD << "OpenAL error: ";
    switch (error) {
      case AL_INVALID_NAME:
        *out << "invalid name.\n";
        break;
      case AL_INVALID_ENUM:
        *out << "invalid enum.\n";
        break;
      case AL_INVALID_VALUE:
        *out << "invalid value.\n";
        break;
      case AL_INVALID_OPERATION:
        *out << "invalid operation.\n";
        break;
      case AL_OUT_OF_MEMORY:
        *out << "out of memory.\n";
        break;
      default:
        *out << error << '\n';
        break;
    }
    *out << "Occurred at: " << errorMaker << '\n';
  }
  return error;
}

ALCenum Sound::checkAlc(const char* errorMaker)
  // check ALC error
{
  ALCenum error = alcGetError(playDevice);

  if (error != ALC_NO_ERROR) {
    *out << VERBOSE_LOUD << "OpenAL context error: ";
    switch (error) {
      case ALC_INVALID_DEVICE:
        *out << "invalid device.\n";
        break;
      case ALC_INVALID_CONTEXT:
        *out << "invalid context.\n";
        break;
      case ALC_INVALID_ENUM:
        *out << "invalid enum.\n";
        break;
      case ALC_INVALID_VALUE:
        *out << "invalid value.\n";
        break;
      case ALC_OUT_OF_MEMORY:
        *out << "Out of memory.\n";
        break;
    }
    *out << "Occurred at: " << errorMaker << '\n';
  }

  return error;
}

