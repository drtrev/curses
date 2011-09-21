#ifndef SOUND
#define SOUND

#include <AL/alut.h>

class Outverbose;

class Sound {
  private:

  protected:
    Outverbose *out;
    ALCdevice *playDevice;

    ALenum check(const char*);      // error check
    ALCenum checkAlc(const char*);  // context error check

  public:
    Sound();
    ~Sound();

    void initOutput(Outverbose&);

    void setPlayDevice(ALCdevice*);

};

#endif
