#ifndef TRANSFERSEND
#define TRANSFERSEND

#include <cstdio>
#include "network/network.h"

class Net; class Client;

class Transfersend {
  private:
    int id;
    char filename[MAX_FILENAME_SIZE];
    char path[MAX_FILENAME_SIZE];
    int source;
    int dest;
    int offset;
    bool started;
    bool active;

    std::FILE *file;

  public:
    Transfersend(int source, int dest, int id, const char* path, const char* filename);

    //bool getStarted();
    //void setStarted(bool);
    int getSource();
    int getDest();
    int getId();
    char* getFilename();
    bool getActive();

    void request(Net&, Client&);
    bool receive(const char*);

    void send(Net&, Client&);

};

#endif
