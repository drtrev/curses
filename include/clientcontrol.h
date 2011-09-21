#ifndef CLIENTCONTROL
#define CLIENTCONTROL

#include "network/client.h"
#include "curses.h"
//#include "inputX11.h"
#include "inputSDL.h"
#include "sound/dev.h"
#include "sound/talk.h"

class Clientcontrol : public Curses {
  private:
    Client client;
    //InputX11 input; // doesn't seem to work in fullscreen
    InputSDL input;
    SoundDev soundDev;
    Talk talk;

    int keys, keysOld; // which keys are being held down, each bit represents a key, see input.h

  public:
    Clientcontrol();
    ~Clientcontrol();

    void init(int, std::string, std::string, verboseEnum, char*, bool, bool);
    void gameover();

    void go();

    void doloop(int, timeval&, void (Clientcontrol::*)());

    void inputloop();
    void process(Unit);
    void networkloop(); // do the network sending and receiving/processing
    void physicsloop();
    void graphicsloop();
    void soundloop();
    void transferloop();

};

#endif
