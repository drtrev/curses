#ifndef CURSES
#define CURSES

#include <cmath>
#include <iostream>
#include <signal.h>
#include "timer.h"
#include "outverbose.h"
#include "badcontrol.h"
#include "bulletcontrol.h"
//#include "client.h"
//#include "clientcontrol.h"
//#include "graphics.h"
//#include "input.h"
#include "map.h"
#include "network/net.h"
#include "network/network.h"
#include "picturecontrol.h"
#include "player.h"
#include "powerupcontrol.h"
#include "transfer/transfercontrol.h"
//#include "server.h"
//#include "sound/dev.h"
//#include "sound/talk.h"

// not pointers so can't do this
//class Badcontrol; class Bulletcontrol; class Client; class Input; class Net; class Outverbose; class Player; class Powerupcontrol; class Server; class SoundDev; class Talk; class Timer;

class Graphics; class Client; class Server;

class Curses {
  protected:
    Badcontrol badcontrol;
    Bulletcontrol bulletcontrol;
    Graphics* graphics;
    Map map;
    Net net;
    Outverbose out;
    Picturecontrol picturecontrol;
    Player player[MAX_CLIENTS];
    Powerupcontrol powerupcontrol;
    Timer timer;
    Transfercontrol transfercontrol;

    double sync;

    bool SERV;
    int flagsize;
    int myId;
    Unit unit;
    int players;

    // TODO should this just be in servercontrol.h?
    // store the previous position of players
    // this is initialised upon logon
    struct Oldposition {
      float x;
      float y;
      float z;
    } oldposition[MAX_CLIENTS];

    int unitsize[UNITS];

  public:
    Curses();
    virtual ~Curses();

    virtual void go() = 0;

    void gameoverShared(); // relevant to client and server
    virtual void gameover() = 0;

    void initUnitSize();
    void initShared(std::string, verboseEnum, bool); // relevant to client and server
    virtual void init(int, std::string, std::string, verboseEnum, char*, bool, bool) = 0;

    //void init(int argc, char** argv);
    //void init(int, std::string, char*, verboseEnum, bool, bool);
    
    //void process(Unit, Client&, Server&, Talk&, int[]);

};

#endif
