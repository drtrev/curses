#ifndef SOUND_DEV
#define SOUND_DEV

#include "sound.h"

class SoundDev : public Sound {
  private:
    // note playDevice is in parent since it's needed to checkAlc
    ALCdevice *captureDevice;
    ALCcontext *context;

    bool openPlayDevice();
    bool createContext();
    bool destroyContext();
    bool closePlayDevice();

  public:
    SoundDev();
    ~SoundDev();

    bool grab(); // open play device and create context
    bool release(); // destroy context and close play device

    bool checkPlayContext(); // check we've got play device and context ready to go
    ALCdevice* getPlayDevice(); // for passing to other instances

    bool openCaptureDevice(ALCuint, ALCenum, ALCsizei);
    bool closeCaptureDevice();
    bool checkCaptureDevice(); // check not null (here to be consistent with play device)
    ALCdevice* getCaptureDevice();

};

#endif
