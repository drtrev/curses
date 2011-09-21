#include "network/server.h"
//#include <arpa/inet.h> // for inet_ntoa
//#include <errno.h>
#include <cmath>
#include <cstring>
#include <iostream>
#include "network/net.h"
#include <netdb.h> // for gethostbyname
#include "outverbose.h"
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

using std::cerr;
using std::endl;

Server::Server()
{
  for (int i = 0; i < MAX_CLIENTS; i++) {
    clientfd[i] = 0;
    clientActive[i] = false;
    sendBuf[i].allocate(SENDBUF_SIZE); // TODO make CLIENT_SENDBUF_SIZE and SERVER_SENDBUF_SIZE (and same for recv buf?)
  }

  recvBuf.allocate(RECVBUF_SIZE);

  clients = 0; // none connected to start with

  PORT = 0;
  BACKLOG = 0;
  flagsize = 0;

  // UNITS defined in network.h

#ifdef _WIN32
  winSockInit = false;
#endif

  recvSize = -1;
}

void Server::init(Outverbose &o, int p, int b, int f, int u[])
{
  out = &o;
  PORT = p;
  BACKLOG = b;
  flagsize = f;
  std::memcpy(unitsize, u, UNITS*sizeof(int));

  int largestUnit = 0;

  for (int i = 0; i < UNITS; i++) {
    if (unitsize[i] > largestUnit) largestUnit = unitsize[i];
  }

  recvSize = largestUnit + flagsize;
}

#ifdef _WIN32
bool Server::initWinSock()
{
  if (WSAStartup(MAKEWORD(1, 1), &wsaData) != 0) {
    *out << VERBOSE_LOUD << "WSAStartup failed.\n";
    return false;
  }
  else
  {
    *out << VERBOSE_LOUD << "WSAStartup successful\n";
    winSockInit = true;
  }

  return true;
}
#endif

bool Server::startListening()
{
#ifdef _WIN32
  if (!winSockInit) {
    if (!initWinSock()) return false;
  }
#endif

  if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    perror("perror socket()");
    *out << VERBOSE_LOUD << "Server::startListening(): error setting up listenfd. socket() returned an error\n";
    return false;
  }

  int sockoptyes = 1;

  // Allow the reusing of this address.
  // By default, when the socket is closed, it will enter a TIME_WAIT state for 2 x maximum
  // segment lifetime (MLS) - this is 240 secs (4 mins). This is to allow duplicate packets
  // to die out. However it's probably too long to hold onto the address (and using INADDR_ANY
  // takes all network interfaces, using the IP address of the first interface) so just forget it
  // and hope you don't get ghost packets!
  // See http://www.port80software.com/200ok/archive/2004/12/07/205.aspx
  // and http://orca.st.usm.edu/~seyfarth/network_pgm/net-6-9-6.html
#ifdef _WIN32
  if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (char*) &sockoptyes, sizeof(int)) == -1)
#else
  if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &sockoptyes, sizeof(int)) == -1)
#endif
  {
    perror("setsockopt");
    *out << VERBOSE_LOUD << "Server::init(): error setting up listenfd with setsockopt()\n";
    return false;
  }

  myaddr.sin_family = AF_INET;           // Address Family internet, same as Protocol Family internet (PF_INET)
  myaddr.sin_port = htons(PORT);         // short, network byte order 
  myaddr.sin_addr.s_addr = INADDR_ANY;   // this takes all network interfaces
  memset(&(myaddr.sin_zero), '\0', 8);   // zero the rest of the struct 

  if (bind(listenfd, (struct sockaddr *)&myaddr, sizeof(struct sockaddr)) == -1) {
    perror("bind");
    *out << VERBOSE_LOUD << "Server::init(): error setting up listenfd with bind()\n";
    return false;
  }

  if (listen(listenfd, BACKLOG) == -1) {
    perror("listen");
    *out << VERBOSE_LOUD << "Server::init(): error setting up listenfd with listen()\n";
    return false;
  }

  return true; // ok
}

bool Server::acceptConnection(Net &net)
{
  struct sockaddr_in their_addr; // connector's address information
#ifndef _WIN32
  socklen_t addrlen = sizeof(struct sockaddr_in);
#else
  int addrlen = sizeof(struct sockaddr_in);
#endif

  int check;

#ifndef _WIN32
  if ((check = accept(listenfd, (struct sockaddr *)&their_addr, (socklen_t*) &addrlen)) == -1)
#else
  if ((check = accept(listenfd, (struct sockaddr *)&their_addr, (int*) &addrlen)) == -1)
#endif
  {
    *out << VERBOSE_LOUD << "Server::acceptConnection(): accept error\n";
  }else{
    //cerr << "Server::acceptConnection(): OK. Got connection from " << inet_ntoa(their_addr.sin_addr) << endl;
    if (clients < MAX_CLIENTS) {
      clientfd[clients] = check;
      clientActive[clients] = true;
      clients++;

      // assign ID
      Unit unit;
      unit.assign.flag = UNIT_ASSIGN;
      unit.assign.id = clients - 1;

      net.addUnit(clients - 1, unit, *this);
      /*char data[recvSize];
      int size = net.unitToBytes(unit, data);

      if (size != flagsize + unitsize[unit.flag]) {
        *out << VERBOSE_LOUD << "Error converting unit to bytes for assigning ID! Size: "
             << size << '\n';
      }else{
        if (addData(clients - 1, data, size) != size) {
          *out << VERBOSE_LOUD << "Error adding data for sending ID to client: "
               << (clients - 1) << '\n';
        }
      }*/

      // tell others about new client
      unit.newclient.flag = UNIT_NEWCLIENT;
      unit.newclient.id = clients - 1;

      char *data = new char[recvSize];
      int size = net.unitToBytes(unit, data);

      if (size != flagsize + unitsize[unit.flag]) {
        *out << VERBOSE_LOUD << "Error converting unit to bytes for assigning ID! Size: "
             << size << '\n';
      }else{
        for (int i = 0; i < clients - 1; i++) {
          if (addData(i, data, size) != size) {
            *out << VERBOSE_LOUD << "Error adding data for sending ID to client: "
                 << i << '\n';
          }
        }
      }

      // tell new client about others
      unit.newclient.flag = UNIT_NEWCLIENT;

      for (int i = 0; i < clients - 1; i++) {
        unit.newclient.id = i;
        size = net.unitToBytes(unit, data);

        if (size != flagsize + unitsize[unit.flag]) {
          *out << VERBOSE_LOUD << "Error converting unit to bytes for telling new client about others. Size: "
               << size << '\n';
        }else{
          if (addData(clients - 1, data, size) != size) {
            *out << VERBOSE_LOUD << "Error adding data for sending to new client: "
                 << (clients - 1) << '\n';
          }
        }
      }

      delete [] data;

      return true; // new client

    }else{
      *out << VERBOSE_LOUD << "Error. Reached maximum number of clients. Closing socket.\n";
#ifndef _WIN32
      if (close(check) == -1)
#else
      if (closesocket(check) == -1)
#endif
      {
        perror("perror close()");
        *out << VERBOSE_LOUD << "Server::acceptConnection(): close error\n";
      }
    }
  }

  return false;

}

void Server::closeConnection(int fd)
{
  *out << VERBOSE_QUIET << "Closing connection to fd: " << fd << '\n';

#ifndef _WIN32
  if (close(fd) == -1)
#else
  if (closesocket(fd) == -1)
#endif
  {
    perror("perror close()");
    *out << VERBOSE_LOUD << "closeConnection error\n";
  }

  // mark as inactive
  for (int i = 0; i < clients; i++) {
    if (clientfd[i] == fd) {
      // found
      clientActive[i] = false;
      break;
    }
  }
}

int Server::getClients()
{
  return clients;
}

int Server::addData(int cid, uint16_t data)
  // input client id and data, return bytes written or -1 on error
{
  data = htons(data);

  return addData(cid, (char*) &data, 2);
}

int Server::addData(int cid, uint32_t data)
  // input client id and data, return bytes written or -1 on error
{
  data = htonl(data);

  return addData(cid, (char*) &data, 4);
}

int Server::addData(int cid, float data)
  // input client id and data, returns total of two addData's which may not be much use.
  // Basically don't use this function. Convert a whole data unit to bytes then attempt to add with
  // addData(int, const char*, int) instead. (i.e. use net.addUnit())
{
  // there are two ways to send a float
  //
  // 1) Knowing I have control over both ends, can do
  //    (char*) &data
  //    or reinterpret_cast <char*> (&data)
  //    but this may not work with different endianness
  // 2) Split into integral and fractional parts using modf
  //    and send as two ints
  //
  // Note: "(uint32_t) data" rounds to an int (static cast).
  // Also, remember to change unitsize, and how it is interpreted
  // on the other end!

  // 1)
  //return addData(cid, reinterpret_cast<char*>( &data ), 4);

  // 2)
  float integral = 0;
  float fraction = roundf( modff(data, &integral) * 10000 ); // just keep 4 decimal places

  return addData(cid, static_cast<uint32_t>( integral )) + addData(cid, static_cast<uint32_t>( fraction ));
}

int Server::addData(int cid, const char* data, int amount)
  // checks it fits on buffer - simpler than ending up with half a data unit being
  // written, with application having to store the rest (i.e. app can just drop whole
  // data unit instead)
  // returns 0 if doesn't fit (nothing written)
  // or -1 on error
{
  int written = -1; // error

  if (cid > -1 && cid < MAX_CLIENTS) {
    if (amount < SENDBUF_SIZE - sendBuf[cid].getLength() + 1) {
      written = sendBuf[cid].write(data, amount);

      if (written != amount) {
        *out << VERBOSE_LOUD << "Error writing to send buffer!!\n";
        written = -1;
      }
    }else written = 0;
  }else *out << VERBOSE_LOUD << "Error with cid in Server::addData\n";

  return written;
}

void Server::addDataAll(const char* data, int size, int source)
  // add data to all client buffers except 'source' and error check
  // source is the client the data originated from, and can be -1
  // (or any other invalid client number) to transmit to all
{
  if (size > 0) {

    int added = 0;

    for (int i = 0; i < clients; i++) {

      if (i != source) {
        added = addData(i, data, size);

        if (added == 0) {
          *out << VERBOSE_LOUD << "Error adding data, buffer full for client: " << i << '\n';
        }else if (added == -1) {
          *out << VERBOSE_LOUD << "Error detected from add data\n";
        }
      }

    }

  }else *out << VERBOSE_LOUD << "Error adding data to all, size is invalid: " << size << '\n';
}

void Server::doSelect()
{
  struct timeval timeout;             // timeout for using select()

  timeout.tv_sec = 0; // don't halt
  timeout.tv_usec = 0;
  FD_ZERO(&readSocks);
  FD_ZERO(&writeSocks);
  FD_ZERO(&exceptionSocks);
  FD_SET(listenfd, &readSocks); // check for sockfd being readable
  FD_SET(listenfd, &exceptionSocks);

  int maxfd = listenfd + 1;

  for (int i = 0; i < clients; i++) {
    FD_SET(clientfd[i], &readSocks);
    FD_SET(clientfd[i], &writeSocks);
    FD_SET(clientfd[i], &exceptionSocks);

    if (clientfd[i] > maxfd) maxfd = clientfd[i];
  }

  // check if socket is readable/writable
  if ((numSocksReadable = select(maxfd + 1, &readSocks, &writeSocks, &exceptionSocks, &timeout)) == -1) {
    perror("perror select()");
    *out << VERBOSE_LOUD << "Error with select()\n";
  }

  if (numSocksReadable > 0) {
    for (int i = 0; i < clients; i++) {
      if (FD_ISSET(clientfd[i], &exceptionSocks)) {
        *out << VERBOSE_LOUD << "SELECT EXCEPTION DETECTED for client " << i << '\n';
      }
    }

    if (FD_ISSET(listenfd, &exceptionSocks)) {
      *out << VERBOSE_LOUD << "SELECT EXCEPTION DETECTED for listen sock\n";
    }
  }

}

void Server::recvAll()
  // call doSelect first
{
  if (numSocksReadable > 0) {

    // receive
    char *dataRecv = new char[recvSize];
    int unitFound = -1, bytesWritten = 0;

    for (int i = 0; i < clients; i++) {
      if (FD_ISSET(clientfd[i], &readSocks)) {
        unitFound = recvData(clientfd[i], dataRecv);

        if (unitFound > -1) {
          if (unitsize[unitFound] + flagsize < RECVBUF_SIZE - recvBuf.getLength() + 1) {
            bytesWritten = recvBuf.write(dataRecv, unitsize[unitFound] + flagsize);

            if (bytesWritten != unitsize[unitFound] + flagsize) {
              *out << VERBOSE_LOUD << "Error writing to receive buffer!!\n";
            }
          }else{
            *out << VERBOSE_LOUD << "Receive buffer full, dropping data unit\n";
          }
        }
      }
    }

    delete [] dataRecv;

  }
}

void Server::sendAll()
  // call doSelect first
{
  if (numSocksReadable > 0) {
    // send
    for (int i = 0; i < clients; i++) {
      if (sendBuf[i].getLength() > 0 && FD_ISSET(clientfd[i], &writeSocks)) {
        sendData(i);
      }
    }
  }
}

bool Server::checkNewConnections(Net &net)
  // call doSelect first
{
  // accept
  if (FD_ISSET(listenfd, &readSocks)) return acceptConnection(net);
  return false;
}

int Server::checkClosedConnections()
  // returns id of next inactive client
{
  for (int i = 0; i < clients; i++) {
    if (!clientActive[i]) {
      for (int j = i; j < clients - 1; j++) {
        clientfd[j] = clientfd[j+1];
        clientActive[j] = clientActive[j+1];
        sendBuf[j] = sendBuf[j+1];
      }
      // clear buffer
      char *temp = new char[sendBuf[clients - 1].getLength()];
      sendBuf[clients - 1].read(temp, sendBuf[clients - 1].getLength());
      clients--;
      delete [] temp;
      return i;
    }
  }
  return -1;
}

void Server::closeAll()
  // close all connections
{
  for (int i = 0; i < clients; i++) {
    if (clientActive[i]) closeConnection(clientfd[i]);
  }
}

void Server::sendData(int client)
{
  char data[MAX_SEND_DATA];
  // peek, only read off buffer on successful send
  int size = sendBuf[client].peek(data, MAX_SEND_DATA);
  int numSent = 0;

  if (size > 0) {
    numSent = send(clientfd[client], data, size, MSG_NOSIGNAL); // don't send SIGPIPE
    if (numSent == -1) {
      perror("perror send()");
      *out << VERBOSE_LOUD << "Server::sendData(): error with send.\n";
      closeConnection(clientfd[client]);
    }else if (numSent > 0) {
      size = sendBuf[client].read(data, numSent);
      if (size != numSent) *out << VERBOSE_LOUD << "Error reading chars from sendBuf\n";
    }
  }
}

int Server::findUnit(char* data, int datasize)
  // return the data unit found
{
  int unitFound = -1;

  if (datasize > flagsize - 1) {
    unitFound = (int) ntohs(*(uint16_t*) &data[0]);
  }

  return unitFound;
}

int Server::recvData(int fd, char* data)
  // returns int of unit found or -1 if not found
  // this needs to identify the unit so it knows if it's got enough data
  // so might as well return id of unit so other functions don't have to identify it
{
  int unitFound = -1;

  int numRecv = recv(fd, data, recvSize, MSG_PEEK);

  if (numRecv == -1) {
    perror("perror recv peek");
    *out << VERBOSE_LOUD << "Server::recvData(): error with recv peek.\n";
    closeConnection(fd);
  }else if (numRecv > flagsize - 1) {
    unitFound = findUnit(data, numRecv);

    if (unitFound > -1 && unitFound < UNITS) {
      if (numRecv > unitsize[unitFound] + flagsize - 1) {
        // read off data unit
        numRecv = recv(fd, data, unitsize[unitFound] + flagsize, 0);

        // error check
        if (numRecv == -1) {
          perror("perror recv no peek");
          *out << VERBOSE_LOUD << "Server::recvData(): error with recv no peek.\n";
          closeConnection(fd);
          unitFound = -1;
        }else if (numRecv != unitsize[unitFound] + flagsize) {
          *out << VERBOSE_LOUD << "Could not manage to get full data unit so dropping it\n";
          unitFound = -1;
        }
      }else{
        unitFound = -1; // waiting for more data
        //*out << VERBOSE_LOUD << "Waiting for more data... do I care?\n"; // TODO have seen this output. Remove now
      }
    }else{
      *out << VERBOSE_LOUD << "Received false unit! TODO\n";
      unitFound = -1; // nothing
      // drop data unit and move on...
      // but how do you know when a flag is the start of a unit and not just garbage?
      // disconnect client and they can reconnect
      // TODO when have worked out logon logoff, can client reconnect as same player?
      // also need to make client reconnect
      closeConnection(fd);
    }
  }else{
    if (numRecv == 0) {
      // orderly shutdown
      closeConnection(fd);
    }else{
      // useful for debugging - could some data remain on socket after crash??
      *out << VERBOSE_LOUD << "WAITING for more data2... 1 bytes remaining\n";
    }
  }

  return unitFound;
}

Unit Server::getDataUnit(Net &net)
  // recvSize should be set to size of largest data unit + flagsize
{
  char *data = new char[recvSize];
  char *flag = new char[flagsize];
  int bytes = 0;
  int unitFound = -1;

  if (recvBuf.getLength() > flagsize - 1) {
    bytes = recvBuf.peek(flag, flagsize);

    if (bytes != flagsize) *out << VERBOSE_LOUD << "Error reading from recvBuf\n";
    else unitFound = findUnit(flag, flagsize);

    if (unitFound > -1 && unitFound < UNITS) {
      if (recvBuf.getLength() > unitsize[unitFound] + flagsize - 1) {
        bytes = recvBuf.read(data, unitsize[unitFound] + flagsize);
        if (bytes != unitsize[unitFound] + flagsize) {
          *out << VERBOSE_LOUD << "Error reading from recvBuf\n";
          unitFound = -1;
        }
      }else{
        *out << VERBOSE_LOUD << "IMPORTANT error - receive buf not big enough for data unit when it should be\n";
        unitFound = -1;
      }
    }else{
      unitFound = -1;
    }
  }

  Unit unit;
  unit.flag = -1;

  if (unitFound > -1) unit = net.bytesToUnit(unitFound, data);

  delete [] data;
  delete [] flag;

  return unit;
}
