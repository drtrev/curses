#ifndef SERVERCONTROL
#define SERVERCONTROL

#include "curses.h"
#include "inputSDL.h"
#include "network/server.h"

class Servercontrol : public Curses {
  private:
    Server server;

    InputSDL input;

    timeval lastSent;

    int keyset[MAX_CLIENTS]; // keys received from clients
    int keys; // keys pressed by server user

    bool gameoverState;

    struct {
      float x;
      float y;
      float zoom;
    } oldmapposition;

  public:
    Servercontrol();
    ~Servercontrol();

    void init(int, std::string, std::string, verboseEnum, char*, bool, bool);
    void gameover();

    void go();

    void doloop(int, timeval&, void (Servercontrol::*)());

    void inputloop();
    void sendStatus(int);
    void process(Unit);
    void networkloop();
    void physicsloop();
    void graphicsloop();

};

#endif
