#ifndef TRANSFERRECV
#define TRANSFERRECV

#include "network/network.h"
#include <cstdio>

class Net; class Client;

class Transferrecv {
  private:
    int id;
    char filename[MAX_FILENAME_SIZE];
    char path[MAX_FILENAME_SIZE];
    int source;
    int dest;
    bool started;
    bool active;
    bool opened;
    bool ready;

    std::FILE *file;

  public:
    Transferrecv(int source, int dest, int id, const char* path, const char* filename);

    bool checkFileExists(char*, bool);

    bool getStarted();
    void setStarted(bool);
    bool getReady();
    void setReady(bool);
    bool getActive();
    int getId();
    int getSource();

    void request(Net&, Client&);
    void receive(const char*, int);

};

#endif
