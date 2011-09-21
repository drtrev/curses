#include "sound/dev.h"
#include "outverbose.h"

SoundDev::SoundDev()
{
  playDevice = NULL, captureDevice = NULL, context = NULL;
}

SoundDev::~SoundDev()
{
  if (context || playDevice) release();
}

bool SoundDev::grab()
  // keep things simple for application: just grab and release
{
  // if play device doesn't open, don't create context
  // if context doesn't open, close down play device (all or nothing)

  if (openPlayDevice()) {
    if (createContext()) return true;
    else closePlayDevice(); // all or nothing
  }
  return false;
}

bool SoundDev::release()
  // closes what it can, the functions called here will output error messages
{
  bool ok = destroyContext();
  bool ok2 = closePlayDevice(); // attempt this even if context destroy fails

  return (ok && ok2);
}

bool SoundDev::openPlayDevice()
{
  if (!playDevice) {
    playDevice = alcOpenDevice(NULL); // open default device

    if (playDevice) {
      checkAlc("Open device");
      return true;
    }else {
      *out << VERBOSE_LOUD << "Error opening device.\n";
      return false;
    }
  }else{
    *out << VERBOSE_LOUD << "Error opening device: I've already opened it. Close first.\n";
    return false;
  }

}

bool SoundDev::createContext()
{
  if (playDevice) {

    context = alcCreateContext(playDevice, NULL);
    checkAlc("Make context");

    if (context)
    {
      ALCboolean ok = alcMakeContextCurrent(context);
      checkAlc("Make context current");

      if (ok) {
        return true;
      }else{
        *out << VERBOSE_LOUD << "Error making context current.\n";
        context = NULL;
        return false;
      }
    }else{
      *out << VERBOSE_LOUD << "Error making context.\n";
      return false;
    }

  }else{
    *out << VERBOSE_LOUD << "Cannot create context: Play device not opened.\n";
    return false;
  }
}

bool SoundDev::destroyContext()
{
  if (context) {
    ALCboolean ok = alcMakeContextCurrent(NULL);
    checkAlc("Make current context NULL");
    alcDestroyContext(context); // attempt to destroy even if not ok
    checkAlc("Destroy context");
    context = NULL;
    return ok;
  }else{
    *out << VERBOSE_LOUD << "Cannot destroy context: no context found.\n";
    return false;
  }
}

bool SoundDev::closePlayDevice()
{
  if (!context) {
    if (playDevice) { // if failed to open then don't attempt to close cos it'll segfault
      ALCboolean ok = alcCloseDevice(playDevice);
      checkAlc("Close device");
      if (ok) playDevice = NULL;
      return ok;
    }else{
      *out << VERBOSE_LOUD << "Cannot close play device: not open.\n";
      return false;
    }
  }else{
    *out << VERBOSE_LOUD << "Cannot close play device - spec says contexts must be destroyed first.\n";
    return false;
  }
}

bool SoundDev::checkPlayContext()
  // is the play device open and context created?
{
  return (playDevice && context);
}

ALCdevice* SoundDev::getPlayDevice()
{
  return playDevice;
}

bool SoundDev::openCaptureDevice(ALCuint freq, ALCenum format, ALCsizei internalSamples)
{
  // NULL for default device
  captureDevice = alcCaptureOpenDevice( NULL, freq, format, internalSamples);
  checkAlc("Open capture device");

  if (captureDevice) {
    return true;
  }else{
    *out << VERBOSE_LOUD << "Error opening capture device\n";
    return false;
  }
}

bool SoundDev::closeCaptureDevice()
{
  if (captureDevice) {
    ALCboolean ok = alcCaptureCloseDevice(captureDevice);
    checkAlc("Close capture device");
    if (ok) {
      captureDevice = NULL;
    }else{
      *out << VERBOSE_LOUD << "Error with closing capture device\n";
    }
    return ok;
  }else{
    *out << VERBOSE_LOUD << "Cannot close capture device: not open.\n";
    return false;
  }
}

bool SoundDev::checkCaptureDevice()
  // just checks it's not null - here to be consistent with play device
{
  if (captureDevice) return true;
  return false;
}

ALCdevice* SoundDev::getCaptureDevice()
{
  return captureDevice;
}

