#include "network/client.h"
//#include <errno.h>
#include <cstring>
#include "network/net.h"
#include "outverbose.h"
#include <netdb.h> // for gethostbyname
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

Client::Client()
{
  connected = false;

  sendBuf.allocate(SENDBUF_SIZE);

  numSocksReadable = 0;

  PORT = 0;

  statSent = 0; // produce some stats
  statRecvd = 0;

  // UNITS defined in network.h

#ifdef _WIN32
  winSockInit = false;
#endif

  recvSize = -1;
}

void Client::init(Outverbose &o, int p, int f, int u[])
{
  out = &o;
  PORT = p;
  flagsize = f;
  std::memcpy(unitsize, u, UNITS*sizeof(int));

  int largestUnit = 0;

  for (int i = 0; i < UNITS; i++) {
    if (unitsize[i] > largestUnit) largestUnit = unitsize[i];
  }

  recvSize = largestUnit + flagsize;
}

#ifdef _WIN32
bool Client::initWinSock()
{
  if (WSAStartup(MAKEWORD(1, 1), &wsaData) != 0) {
    *out << VERBOSE_LOUD << "WSAStartup failed.\n";
    return false;
  }
  else
  {
    *out << VERBOSE_NORMAL << "WSAStartup successful\n";
    winSockInit = true;
  }

  return true;
}
#endif

int Client::getBufferSpace()
{
  return SENDBUF_SIZE - sendBuf.getLength();
}

bool Client::openConnection(const char* ip)
{
  if (connected) {
    int closeval = closeConnection();
    *out << VERBOSE_QUIET << "Close connection returned: " << closeval << '\n';
  }

#ifdef _WIN32
  if (!winSockInit) {
    if (!initWinSock()) return false;
  }
#endif

  if (strlen(ip) > 15) {
    // if length not including terminating null char is too long
    *out << VERBOSE_LOUD << "IP address too long\n";
    return false;
  }

  strncpy(ipAddress, ip, 16); // store in case need to reconnect

  if ((host = gethostbyname(ip)) == NULL) {
    perror("perror gethostbyname()");
    return false;
  }

  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    perror("perror socket()");
    return false;
  }

  serverAddress.sin_family = AF_INET;                          // host byte order 
  serverAddress.sin_port = htons(PORT);                          // short, network byte order 
  serverAddress.sin_addr = *((struct in_addr *)host->h_addr);
  memset(&(serverAddress.sin_zero), '\0', 8);                  // zero the rest of the struct 

  *out << VERBOSE_QUIET << "Connecting...\n";

  if (connect(sockfd, (struct sockaddr *)&serverAddress, sizeof(struct sockaddr)) == -1) {
    perror("perror connect()");
    return false;
  }else connected = true;

  *out << VERBOSE_QUIET << "Connected\n";

  return true; // connected
}

int Client::closeConnection()
{
  connected = false;
#ifndef _WIN32
  return close(sockfd);
#else
  return closesocket(sockfd);
#endif
}

bool Client::getConnected() const
{
  return connected;
}

int Client::addData(uint16_t data)
  // if calling this then check all your data unit fits on buffer first with getBufferSpace()
  // returns bytes written or -1 on error
{
  data = htons(data);

  return addData((char*) &data, 2);
}

int Client::addData(uint32_t data)
  // if calling this then check all your data unit fits on buffer first with getBufferSpace()
  // returns bytes written or -1 on error
{
  data = htonl(data);

  return addData((char*) &data, 4);
}

int Client::addData(const char* data, int amount)
  // checks it fits on buffer - simpler than ending up with half a data unit being
  // written, with application having to store the rest (i.e. app can just drop whole
  // data unit instead)
{
  int written = -1; // error

  if (amount < getBufferSpace() + 1)
    written = sendBuf.write(data, amount);

  return written;
}

void Client::doSelect()
{
  struct timeval timeout;             // timeout for using select()

  timeout.tv_sec = 0; // don't halt
  timeout.tv_usec = 0;
  FD_ZERO(&readSocks);
  FD_ZERO(&writeSocks);
  FD_ZERO(&exceptionSocks);
  FD_SET(sockfd, &readSocks); // check for sockfd being readable
  FD_SET(sockfd, &writeSocks); // check for sockfd being writable
  FD_SET(sockfd, &exceptionSocks);

  // check if socket is readable/writable
  if ((numSocksReadable = select(sockfd + 1, &readSocks, &writeSocks, &exceptionSocks, &timeout)) == -1) {
    perror("perror select()");
    *out << VERBOSE_LOUD << "Error with select()\n";
  }

  if (FD_ISSET(sockfd, &exceptionSocks)) {
    *out << VERBOSE_LOUD << "SELECT EXCEPTION DETECTED\n";
  }
}

void Client::sendData()
  // requires doSelect to be called by application (doesn't call it here so
  // that app can call once for receive and send)
{
  int numSent = 0;

  if (sendBuf.getLength() > 0 && FD_ISSET(sockfd, &writeSocks)) {
    char data[MAX_SEND_DATA];
    int size = sendBuf.peek(data, MAX_SEND_DATA);

    if (size > 0) {
      numSent = send(sockfd, data, size, MSG_NOSIGNAL); // don't send SIGPIPE
      if (numSent == -1) {
        perror("perror send()");
        *out << VERBOSE_LOUD << "Client::sendData(): error with send.\n";
        if (closeConnection() == -1) {
          perror("closeConnection()");
          *out << VERBOSE_LOUD << "Error closing connection\n";
        }
      }else if (numSent > 0) {
        size = sendBuf.read(data, numSent);
        if (size != numSent) *out << VERBOSE_LOUD << "Error reading chars from sendBuf\n";
        statSent += numSent;
      }
    }
  }

}

int Client::findUnit(char* data, int datasize)
  // return the data unit found
{
  int unitFound = -1;

  if (datasize > flagsize - 1) {
    unitFound = (int) ntohs(*(uint16_t*) &data[0]);
  }

  return unitFound;
}

Unit Client::recvDataUnit(Net &net)
  // requires app to call doSelect
  // recvSize should be size of largest unit + flagsize
  // returns unit found (with flag of -1 if not found)
{
  char *data = new char[recvSize];
  int numRecv = 0, unitFound = -1;

  if (numSocksReadable > 0 && FD_ISSET(sockfd, &readSocks)) {

    numRecv = recv(sockfd, data, recvSize, MSG_PEEK);

    // error check
    if (numRecv == -1) {
      perror("perror recv peek");
      *out << VERBOSE_LOUD << "Client::recvDataUnit(): error with recv peek.\n";
      if (closeConnection() == -1) {
        perror("closeConnection()");
        *out << VERBOSE_LOUD << "Error closing connection\n";
      }
    }else if (numRecv > flagsize - 1) {
      unitFound = findUnit(data, numRecv);

      if (unitFound > -1 && unitFound < UNITS) {
        if (numRecv > unitsize[unitFound] + flagsize - 1) {
          // read off data unit
          numRecv = recv(sockfd, data, unitsize[unitFound] + flagsize, 0);

          // error check
          if (numRecv == -1) {
            perror("perror recv no peek");
            *out << VERBOSE_LOUD << "Client::recvDataUnit(): error with recv no peek.\n";
            if (closeConnection() == -1) {
              perror("closeConnection()");
              *out << VERBOSE_LOUD << "Error closing connection\n";
            }
            unitFound = -1;
          }else if (numRecv != unitsize[unitFound] + flagsize) {
            *out << VERBOSE_LOUD << "Could not manage to get full data unit so dropping it\n";
            unitFound = -1;
          }else{
            statRecvd += numRecv; // assuming no errors for stats
          }
        }else{
          //*out << VERBOSE_LOUD << "Waiting for more data... do I care?\n"; // TODO remove this
          unitFound = -1;
        }
      }else{
        *out << VERBOSE_LOUD << "Received false unit! TODO\n";
        unitFound = -1; // nothing
        // drop data unit and move on...
        // but how do I know when next data unit starts?
        // reconnect!
        closeConnection();
        openConnection(ipAddress);
        // TODO when have worked out logon logoff, can I reconnect as same player?
      }
    }else{
      if (numRecv == 0) {
        // server closed
        if (closeConnection() == -1) {
          perror("closeConnection()");
          *out << VERBOSE_LOUD << "Error closing connection\n";
        }
      }else{
        *out << VERBOSE_LOUD << "WAITING for more data2... do I care?\n"; // TODO do I care?
      }
    }

  }

  Unit unit;
  unit.flag = -1;

  if (unitFound > -1) unit = net.bytesToUnit(unitFound, data);

  delete [] data;

  return unit;
}

int Client::getStatSent()
{
  return statSent;
}

int Client::getStatRecvd()
{
  return statRecvd;
}

