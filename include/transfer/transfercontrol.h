#ifndef TRANSFERCONTROL
#define TRANSFERCONTROL

#include <deque>
#include <string>
#include "network/network.h"
#include "transfer/transfersend.h"
#include "transfer/transferrecv.h"

class Client; class Net; class Outverbose;

class Transfercontrol {
  private:
    Outverbose* out;

    std::string readPath;
    std::string writePath;

    std::deque <Transfersend> transfersend;
    std::deque <Transferrecv> transferrecv;

  public:
    void init(Outverbose&);

    void setReadPath(std::string);
    void setWritePath(std::string);

    bool checkFilename(std::string);

    // start calls request and stores transaction, returns false if file already exists
    bool start(int source, int dest, int id, char* filename, Net &net, Client &client);

    void send(int source, int dest, int id, char* filename, Net &net, Client &client);
    //bool request(int source, int dest, char* filename, Net &net, Client &client);

    bool receive(Unit);//, Net&, Client&);

    void go(Net&, Client&);

};

#endif
