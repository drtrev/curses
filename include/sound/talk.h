#ifndef TALK
#define TALK

#include "stream.h"

class SoundDev; // TODO need this?

class Talk : public Stream {
  private:
    ALCdevice *captureDevice;
    SoundDev *soundDev;
    bool capturing;

  public:
    Talk();
    ~Talk();

    void setSoundDev(SoundDev&); // set SoundDev instance for opening capture device
    void setCaptureDevice(ALCdevice*); // set from SoundDev

    bool openCaptureDevice();
    bool closeCaptureDevice();
    
    void captureStart();
    void captureStop();
    bool getCapturing();
    void capture(int, Net&, Client&);

};

#endif
