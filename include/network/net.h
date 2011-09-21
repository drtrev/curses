#ifndef NET
#define NET

// Tools for processing network data

#include "network.h"
#include <stdint.h> // for uint32_t etc.

// preprocessor expansion did not work
#define ID_PLAYER_MIN   0
#define ID_PLAYER_MAX   10
#define ID_BAD_MIN      10
#define ID_BAD_MAX      60
//#define ID_PIC_MIN      60
//#define ID_PIC_MAX      100
//#define ID_BULLET_MIN   60
//#define ID_BULLET_MAX   110

// forward declaration
// the ones that are modified (e.g. client.h and server.h) are included in net.cpp
class Badcontrol; class Bulletcontrol; class Client; class Outverbose; class Player; class Picturecontrol; class Powerupcontrol; class Ringbuf; class Server; class Talk; class Timer; class Transfercontrol;

class Net {
  private:
    Outverbose* out;

    int flagsize, unitsize[UNITS], maxSize, maxClients;

    int audioDataSize;
    char* audioData;

    int readInt(char*&);
    float readFloat1(char*&); // 4 bytes float
    float readFloat2(char*&); // 4 bytes integral part followed by 4 bytes fraction (* 10000)
    int writeShortInt(Ringbuf&, uint16_t);
    int writeInt(Ringbuf&, uint32_t);
    int writeFloat1(Ringbuf&, float);
    int writeFloat2(Ringbuf&, float);

  public:
    Net();
    ~Net();

    void init(Outverbose&, int, int[], int);
    void setAudioDataSize(int);

    Unit bytesToUnit(int, char*);

    int unitToBytes(Unit, char*);

    void addUnit(Unit, Client&);
    void addUnit(int, Unit, Server&);
    void addUnitAll(Unit, Server&, int); // add unit to all client buffers except source

    //void process(Unit, int&, Player[], int&, Badcontrol&, Bulletcontrol&, Picturecontrol&, Powerupcontrol&, Transfercontrol&, int[], Timer&, bool, Talk&, Client&, Server&);

    void sendStatus(int, Badcontrol&, Bulletcontrol&, Picturecontrol&, Server&);
};

#endif
