#include "sound/talk.h"
#include <cstdlib> // for NULL
#include "network/net.h"
#include "outverbose.h"
#include "sound/dev.h"

Talk::Talk()
{
  soundDev = NULL;
  captureDevice = NULL;
  capturing = false;
}

Talk::~Talk()
{
  if (captureDevice) closeCaptureDevice();
}

void Talk::setSoundDev(SoundDev& sd)
{
  soundDev = &sd;
}

void Talk::setCaptureDevice(ALCdevice* dev)
{
  captureDevice = dev;
}

bool Talk::openCaptureDevice()
{
  if (soundDev) return soundDev->openCaptureDevice(freq, format, internalSamples);
  else{
    *out << VERBOSE_LOUD << "Error: attempt to open capture device without initialising soundDev first\n";
    return false;
  }
}

bool Talk::closeCaptureDevice()
{
  if (soundDev) return soundDev->closeCaptureDevice();
  else return false;
}

void Talk::captureStart()
{
  alcCaptureStart(captureDevice);
  checkAlc("Capture start");
  capturing = true;
}

void Talk::captureStop()
{
  alcCaptureStop(captureDevice);
  checkAlc("Capture stop");
  capturing = false;
}

bool Talk::getCapturing()
{
  return capturing;
}

void Talk::capture(int id, Net &net, Client &client)
{
  //static int tempId = 0;
  ALCint samps;

  // how many samples are on the capture buffer?
  alcGetIntegerv(captureDevice, ALC_CAPTURE_SAMPLES, 1, &samps);
  checkAlc("Get number of samples");

  if (samps > chunkSamples - 1) {
    // got enough, read off chunkSamples

    // note: a way to test chunks are arriving in sequence is to use something like this:
    // unit.audio.id = tempId++;
    // where tempId is a static int, starting at 0

    Unit unit;
    unit.audio.flag = UNIT_AUDIO;
    unit.audio.id = id;
    alcCaptureSamples(captureDevice, unit.audio.data, chunkSamples);
    checkAlc("Capture samples");

    net.addUnit(unit, client);
  }
}
