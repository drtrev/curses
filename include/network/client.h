#ifndef CLIENT
#define CLIENT

#define SENDBUF_SIZE 10000
#define MAX_SEND_DATA SENDBUF_SIZE

#include "network/network.h"
#include "ringbuf.h"

#ifdef _WIN32

#include <winsock.h>

#else

#include <netinet/in.h>

#endif

// forward declaration
class Net; class Outverbose;

class Client {
  private:
    char ipAddress[16];                 // store in case need to reconnect
    struct hostent *host;               // host
    struct sockaddr_in serverAddress;   // server address information 
    int sockfd;                         // socket file descriptor
    fd_set readSocks;                   // sockets to read from
    fd_set writeSocks;                  // sockets to write to
    fd_set exceptionSocks;              // sockets to watch for exceptions on
    int numSocksReadable;

    int PORT, flagsize, unitsize[UNITS];
    int statSent, statRecvd; // for statistics on data transfer rate

#ifdef _WIN32
    bool winSockInit;
#endif

    Ringbuf sendBuf;
    bool connected;

    int recvSize;

    int findUnit(char*, int);

  protected:
    Outverbose* out;

  public:
    Client();                    // constructor

    void init(Outverbose&, int, int, int[]);

#ifdef _WIN32
    bool initWinSock();
#endif

    int getBufferSpace(); // find space remaining on buffer

    bool openConnection(const char*);    // set up and connect to socket
    int closeConnection();
    bool getConnected() const;

    int addData(uint16_t); // add int to buffer for sending
    int addData(uint32_t); // add int to buffer for sending
    int addData(const char*, int); // add data to buffer for sending

    void doSelect();

    void sendData();            // send data from buffer

    Unit recvDataUnit(Net&);  // receive data from socket

    int getStatSent();
    int getStatRecvd();
};
#endif
