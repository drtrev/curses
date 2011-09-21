#ifndef SERVER
#define SERVER

#define SENDBUF_SIZE 10000
#define MAX_SEND_DATA SENDBUF_SIZE
#define RECVBUF_SIZE 10000

#include "network/network.h"
#include "ringbuf.h"

#ifdef _WIN32

#include <winsock.h>

#else

#include <netinet/in.h>

#endif

class Net; class Outverbose;

class Server {
  private:
    struct hostent *host;               // host
    struct sockaddr_in myaddr;          // my address information 
    int listenfd;                       // socket file descriptor to listen on
    int clientfd[MAX_CLIENTS];          // file descriptors of clients
    bool clientActive[MAX_CLIENTS];     // store which fd's are active
    int clients;                        // number of clients connected
    fd_set readSocks;                   // sockets to read from
    fd_set writeSocks;                  // sockets to write to
    fd_set exceptionSocks;              // sockets to watch for exceptions on
    int numSocksReadable;               // technically number of bits set in read/write/exceptions by select

    int PORT, BACKLOG, flagsize, unitsize[UNITS];

#ifdef _WIN32
    bool winSockInit;
#endif

    Ringbuf sendBuf[MAX_CLIENTS], recvBuf;
    int recvSize;

    void sendData(int);            // send data from buffer
    int recvData(int, char*);  // receive data from socket
    int findUnit(char*, int);

    bool acceptConnection(Net&);

    void closeConnection(int);

  protected:
    Outverbose* out;

  public:
    Server();                    // constructor

    void init(Outverbose&, int, int, int, int[]);

#ifdef _WIN32
    bool initWinSock();
#endif

    bool startListening();

    int getClients();

    int addData(int, uint16_t);
    int addData(int, uint32_t); // add int to buffer for sending
    int addData(int, float);
    int addData(int, const char*, int); // add data to buffer for sending
    void addDataAll(const char*, int, int);
    void doSelect();
    void recvAll(); // receive from all clients
    void sendAll(); // send out to all clients
    bool checkNewConnections(Net&);
    int checkClosedConnections();
    void closeAll();

    Unit getDataUnit(Net&);
};
#endif
