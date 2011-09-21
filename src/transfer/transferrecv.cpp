#include "transfer/transferrecv.h"
#include <cstdlib>
#include <cstring>
#include <errno.h>
#include <iostream>
#include "network/net.h"
#include <sys/stat.h>
#include <sys/types.h>

Transferrecv::Transferrecv(int s, int d, int i, const char* p, const char* f)
{
  source = s;
  dest = d;
  id = i;
  if (std::strlen(f) < MAX_FILENAME_SIZE) {
    strncpy(filename, f, MAX_FILENAME_SIZE);
  }else{
    filename[0] = '\0';
    std::cerr << "Error, filename too long: " << f << std::endl;
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

  started = false;
  active = true;
  opened = false;
  ready = true; // ready to receive

  // check path exists, or attempt to make it
  std::cerr << "Checking if directory exists: " << path << std::endl;
  std::cerr << "strlen(path): " << strlen(path) << std::endl;

  if (!checkFileExists(path, true)) {
    std::cerr << "Directory does not exist, attempting to create dir: " << path << std::endl;
    std::cerr << "This will probably fail. Make the dir yourself or fix this." << std::endl;
    //mkdir(path, 448); // rwx for owner
    mkdir(path, 0700);
  }

  char temp[MAX_FILENAME_SIZE*2+1];
  strncpy(temp, path, MAX_FILENAME_SIZE);
  strncat(temp, filename, MAX_FILENAME_SIZE);

  std::cerr << "Checking if file exists: " << temp << std::endl;

  // check if file already exists
  if (checkFileExists(temp, false)) {
    std::cerr << "file already exists... cancelling transfer\n";
    active = false; // transaction will be removed
  }
}

bool Transferrecv::checkFileExists(char* name, bool isdir)
{
  struct stat buf;

  if (!stat(name, &buf)) {
    if (!isdir) {
      if (S_ISREG(buf.st_mode)) {
        std::cerr << "test file exists!" << std::endl;
        return true;
      }else{
        std::cerr << "test file exists but is not a regular file. Is it a device or directory? Can't work with this. Exiting..." << std::endl;
        std::exit(1);
      }
    }else{
      if (S_ISDIR(buf.st_mode)) {
        std::cerr << "test dir exists!" << std::endl;
        return true;
      }else{
        std::cerr << "test dir exists but is not a directory. Can't work with this. Exiting..." << std::endl;
        std::exit(1);
      }
    }
  }else{  
    int e = errno;
    std::cerr << "Could not 'stat' file/dir:" << name << std::endl;
    if (e == ENOENT) {
      std::cerr << "Detected file/dir as non-existent" << std::endl;
      return false;
    }else{
      std::cerr << "Error 'stat'ing file/dir, not ENOENT" << std::endl;
      errno = e;
      perror("perror stat file/dir");
      std::cerr << "Not sure why stat failed, so exiting..." << std::endl;
      std::exit(1);
    }
  }
}

bool Transferrecv::getStarted()
{
  return started;
}

void Transferrecv::setStarted(bool b)
{
  started = b;
}

bool Transferrecv::getReady()
{
  return ready;
}

void Transferrecv::setReady(bool b)
{
  ready = b;
}

bool Transferrecv::getActive()
{
  return active;
}

int Transferrecv::getId()
{
  return id;
}

int Transferrecv::getSource()
{
  return source;
}

void Transferrecv::request(Net &net, Client &client)
{
  if (strlen(filename) > 0) {
    Unit unit;

    unit.transferreq.flag = UNIT_TRANSFERREQ;
    unit.transferreq.to = source;
    unit.transferreq.from = dest;
    unit.transferreq.id = id;
    strncpy(unit.transferreq.filename, filename, MAX_FILENAME_SIZE);

    net.addUnit(unit, client);

    //std::cerr << "Sending transfer request..." << std::endl;
  }else{
    std::cerr << "Filename not initialised" << std::endl;
  }
}

void Transferrecv::receive(const char* data, int amount)
{
  // store data

  // open file if not already
  if (!opened) {
    if (strlen(filename) < 1) {
      std::cerr << "Error with filename: " << filename << std::endl;
    }else{
      std::cerr << "opening file" << std::endl;
      /*char temp[MAX_FILENAME_SIZE + 10];
      snprintf(temp, MAX_FILENAME_SIZE + 10, "%s%s", "/tmp/trev/", filename);
      temp[MAX_FILENAME_SIZE + 9] = '\0';*/
      char temp[MAX_FILENAME_SIZE*2+1];
      strncpy(temp, path, MAX_FILENAME_SIZE);
      strncat(temp, filename, MAX_FILENAME_SIZE);
      file = fopen(temp, "wb");
      opened = true;
      std::cerr << "opened file: " << temp << std::endl;
    }
  }

  if (amount > 0) {
    //std::cerr << "Writing data: " << amount << std::endl;
    fwrite(data, amount, 1, file);
    //std::cerr << "written data" << std::endl;
  }

  if (amount < TRANSFER_DATA_SIZE) {
    fclose(file);
    std::cerr << "Closed file" << std::endl;
    active = false;
  }
}

