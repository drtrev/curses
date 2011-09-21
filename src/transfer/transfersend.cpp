#include "transfer/transfersend.h"
#include <cstring>
#include "network/net.h"
#include <iostream>

Transfersend::Transfersend(int s, int d, int i, const char* p, const char* f)
{
  source = s;
  dest = d;
  id = i;
  if (std::strlen(f) < MAX_FILENAME_SIZE - 1) {
    strncpy(filename, f, MAX_FILENAME_SIZE);
  }else{
    filename[0] = '\0';
    std::cerr << "Error: filename too long, size: " << std::strlen(f) << std::endl;
  }

  if (std::strlen(p) < MAX_FILENAME_SIZE - 2) {
    strncpy(path, p, MAX_FILENAME_SIZE);
    // add on trailing slash if necessary
    if (strlen(path) > 0) {
      char* endchar = &path[strlen(path)-1];
      if (*endchar != '/') {
        *endchar = '/';
        *(endchar+1) = '\0';
      }
    }
  }else{
    path[0] = '\0';
    std::cerr << "Error: path too long, size: " << std::strlen(p) << std::endl;
  }

  std::cerr << "Set transaction for filename as: " << filename << '\n';

  offset = 0;
  active = true;
  started = false;

  file = NULL;
}

int Transfersend::getSource()
{
  return source;
}

int Transfersend::getDest()
{
  return dest;
}

int Transfersend::getId()
{
  return id;
}

char* Transfersend::getFilename()
{
  return filename;
}

bool Transfersend::getActive()
{
  return active;
}

void Transfersend::send(Net &net, Client &client)
{
  if (!started) {
    char temp[MAX_FILENAME_SIZE*2+1];
    strncpy(temp, path, MAX_FILENAME_SIZE);
    strncat(temp, filename, MAX_FILENAME_SIZE*2);
    file = fopen(temp, "rb");
    started = true;
  }

  if (active) {
    // read next bit
    Unit unit;

    unit.transfer.flag = UNIT_TRANSFER;
    unit.transfer.to = dest;
    unit.transfer.from = source;
    unit.transfer.id = id;
    strncpy(unit.transfer.filename, filename, MAX_FILENAME_SIZE);

    // read in elements of size one, so the amount returned is correct (instead of just '0' or '1' elements read)
    unit.transfer.amount = fread(unit.transfer.data, 1, TRANSFER_DATA_SIZE, file);
    //std::cerr << "transfer amount: " << unit.transfer.amount << std::endl;
    if (unit.transfer.amount < TRANSFER_DATA_SIZE) {
      active = false;
      fclose(file);
    }

    net.addUnit(unit, client);
    //std::cerr << "Sending chunk" << std::endl;
  }
}

