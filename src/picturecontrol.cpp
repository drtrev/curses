#include "picturecontrol.h"
#include <cstring>
#include "graphics.h"
#include "input.h"
#include <Magick++.h>
#include "network/net.h"
#include "network/network.h"
#include "outverbose.h"
#include "picture.h"
//#include <string>

Picturecontrol::Picturecontrol()
{
  picnum = 0;
  clientnum = 0;
  for (int i = 0; i < MAX_CLIENTS; i++) piclen[i] = 0;

  readPath = "";
  writePath = "";

  //pic = new Picture[piclen];
  //allocated = false;
}

Picturecontrol::~Picturecontrol()
{
  for (int i = 0; i < MAX_CLIENTS; i++) {
    if (piclen[i] > 0) delete [] pic[i];
  }
}

void Picturecontrol::init(Outverbose &o, Graphics &g)
{
  out = &o;
  graphics = &g;
  textures.init(o); // here server does this too, but it just initialises out, not doing any harm

}

void Picturecontrol::setReadPath(std::string s)
{
  if (s.size() > 0 && s[s.size()-1] != '/') s += '/';
  readPath = s;
}

void Picturecontrol::setWritePath(std::string s)
{
  if (s.size() > 0 && s[s.size()-1] != '/') s += '/';
  writePath = s;
}

void Picturecontrol::load(int myId, Net &net, Client &client)
// if we are given a path, then load and transmit which pics are active
// if path is an empty string then we expect either pictures aren't being used or we're receving them
{
  if (readPath != "") {
    *out << VERBOSE_LOUD << "Loading pics...\n";
    files = dir.getPics(readPath);

    // error checks done in allocate, but left to segfault below!
    // (error messages from allocate() should help track down problem!)
    allocate(myId, files.size()); // this also calls incrementLoaded

    for (int i = 0; i < piclen[myId]; i++) {
      pic[myId][i].load(readPath+files[i]);
      pic[myId][i].setActive(true);
    }

    *out << VERBOSE_LOUD << "Loaded\n";

    // tell server number of pics
    Unit unit;

    unit.picalloc.flag = UNIT_PICALLOC;
    unit.picalloc.id = myId;
    unit.picalloc.total = piclen[myId];
    net.addUnit(unit, client);

    // set to plausible picnum and transmit picselect,
    // note that client who has pictures may not be client 0
    if (picnum > piclen[clientnum] - 1) {

      bool found = false;

      for (int c = clientnum + 1; c < MAX_CLIENTS; c++) {
        if (piclen[c] > 0) {
          clientnum = c;
          found = true;
          break;
        }
      }

      if (!found) {
        for (int c = 0; c < clientnum; c++) {
          if (piclen[c] > 0) {
            clientnum = c;
            found = true;
            break;
          }
        }
      }

      if (!found) *out << VERBOSE_LOUD << "Error: not found any plausible clientnum in Picturecontrol::load()\n";

      picnum = 0;
    }

    // transmit picnum
    unit.picselect.flag = UNIT_PICSELECT;
    unit.picselect.clientnum = clientnum;
    unit.picselect.picnum = picnum;
    unit.picselect.direction = 1;

    net.addUnit(unit, client);
    // this transmission will bounce back and setPicnum will be called to set up positions
    //setPicnum(clientnum, picnum); // sets up positions

    *out << VERBOSE_LOUD << "Picselect clientnum: " << unit.picselect.clientnum << ", picnum: " << unit.picselect.picnum
    << ", direction: " << unit.picselect.direction << '\n';
  }
}

void Picturecontrol::loadRemote(int cid, int pid, char* filename)
// received a file from a remote user. Will have already been allocated (before I called picfetch for it)
{
  if (cid > -1 && cid < MAX_CLIENTS && pid > -1 && pid < piclen[cid]) {
    *out << VERBOSE_LOUD << "Loading remote...\n";
    if (writePath.size() > 0) {
      pic[cid][pid].load(writePath+filename);
      pic[cid][pid].setActive(true);
      pic[cid][pid].incrementLoaded();
    }else{
      *out << VERBOSE_LOUD << "Error, no write path specified\n";
    }
  }else{
    *out << VERBOSE_LOUD << "Error with loadRemote, pictures not allocated for client, cid: " << cid << ", pid: " << pid << '\n';
  }
}

int Picturecontrol::getTotal()
{
  int total = 0;
  for (int i = 0; i < MAX_CLIENTS; i++) {
    total += piclen[i];
  }
  return total;
}

int Picturecontrol::getPiclen(int n)
{
  if (n > -1 && n < MAX_CLIENTS) return piclen[n];
  else return -1;
}

Picture** Picturecontrol::getPics()
{
  return pic;
}

/*int Picturecontrol::getWidth(int n)
{
  if (n > -1 && n < piclen) return pic[n].getWidth();
  return -1;
}

int Picturecontrol::getHeight(int n)
{
  if (n > -1 && n < piclen) return pic[n].getHeight();
  return -1;
}*/

void Picturecontrol::draw(int players)
{
  for (int c = 0; c < MAX_CLIENTS; c++) {
    for (int i = 0; i < piclen[c]; i++) {
      // TODO deal with logoff and possibly removing pictures from list (deallocate)
      //if (pic[c][i].getActive())
      pic[c][i].draw(players);
    }
  }
}

int Picturecontrol::getClientnum()
{
  return clientnum;
}

int Picturecontrol::getPicnum()
{
  return picnum;
}

void Picturecontrol::setPicnum(int cn, int pn, int direction)
{
  if (cn > -1 && cn < MAX_CLIENTS) {
    if (pn > -1 && pn < piclen[cn]) {

      //int oldpicnum = picnum;
      //int oldclientnum = clientnum;

      picnum = pn;
      clientnum = cn;

      // set target positions
      int nextp = picnum + 1;
      int nextc = clientnum;
      if (nextp > piclen[clientnum] - 1) {
        nextp = 0;
        for (int c = 0; c < MAX_CLIENTS; c++) {
          if (piclen[(c+clientnum+1)%MAX_CLIENTS] > 0) {
            nextc = (c+clientnum+1) % MAX_CLIENTS;
            break;
          }
        }
      }

      int prevp = picnum - 1;
      int prevc = clientnum;
      if (prevp < 0) {
        prevp = piclen[clientnum] - 1; // if there's only one client this will do

        // check for previous clients
        for (int c = clientnum - 1; c >= 0; c--) {
          if (piclen[c] > 0) {
            prevc = c;
            prevp = piclen[c] - 1;
            break;
          }
        }
        if (prevc == clientnum) {
          // not found a previous client, check from end
          for (int c = MAX_CLIENTS - 1; c > clientnum; c--) {
            if (piclen[c] > 0) {
              prevc = c;
              prevp = piclen[c] - 1;
              break;
            }
          }
        }
      }

      //*out << VERBOSE_LOUD << "graphics->getWidth/2.0: " << (int) (graphics->getWidth()) << '\n';
      *out << VERBOSE_LOUD << "clientnum: " << clientnum << ", picnum: " << picnum << ", nextc: " << nextc;
      *out << ", nextp: " << nextp << ", prevc: " << prevc << ", prevp: " << prevp << '\n';

      for (int c = 0; c < MAX_CLIENTS; c++) {
        for (int i = 0; i < piclen[c]; i++) {
          pic[c][i].setDirection(direction);
          //pic[c][i].loopAngles();
          pic[c][i].setTargetZ(-300);
          //pic[c][i].setSpeedRot(70);
        }
      }

      //pic[oldclientnum][oldpicnum].setSpeedRot(120); // get out of the way fast

      //pic[clientnum][picnum].setTargetX(0);
      pic[clientnum][picnum].setTargetZ(0);
      // if facing away from camera then speed things up
      if ((pic[clientnum][picnum].getAngleY() < -90 && pic[clientnum][picnum].getAngleY() > -270)
        || (pic[clientnum][picnum].getAngleY() > 90 && pic[clientnum][picnum].getAngleY() < 270)) {
        if (direction == 1) pic[clientnum][picnum].setAngleY(90);
        else pic[clientnum][picnum].setAngleY(270);
      }
      pic[clientnum][picnum].setTargetAngleY(0);

      //pic[nextc][nextp].setTargetX(0); //graphics->getWidth() / 2.0);
      pic[nextc][nextp].setTargetZ(-300);
      if ((pic[nextc][nextp].getAngleY() < -89 && pic[nextc][nextp].getAngleY() > -270)
        || (pic[nextc][nextp].getAngleY() > 90 && pic[nextc][nextp].getAngleY() < 270)) {
        pic[nextc][nextp].setAngleY(90);
      }
      pic[nextc][nextp].setTargetAngleY(90);

      //pic[prevc][prevp].setTargetX(-graphics->getWidth() / 2.0);
      pic[prevc][prevp].setTargetZ(-300);
      if ((pic[prevc][prevp].getAngleY() > 89 && pic[prevc][prevp].getAngleY() < 270)
        || (pic[prevc][prevp].getAngleY() < -90 && pic[prevc][prevp].getAngleY() > -270)) {
        pic[prevc][prevp].setAngleY(-90);
      }
      pic[prevc][prevp].setTargetAngleY(-90);

    }else{
      *out << VERBOSE_LOUD << "setPicnum error: pn out of range: " << pn << '\n';
    }
  }else{
    *out << VERBOSE_LOUD << "setPicnum error: cn out of range: " << cn << '\n';
  }
}

void Picturecontrol::allocate(int cid, int n)
{
  if (cid > -1 && cid < MAX_CLIENTS) {
    if (n > 0) {

      if (piclen[cid] > 0) {
        *out << VERBOSE_LOUD << "Huh? Already allocated... deleting\n";
        delete [] pic[cid];
      }

      piclen[cid] = n;

      pic[cid] = new Picture[piclen[cid]];

      for (int i = 0; i < piclen[cid]; i++) {
        pic[cid][i].init(*out, *graphics, textures);
        pic[cid][i].incrementLoaded(); // it's loaded by the source client
      }

      *out << VERBOSE_LOUD << "Allocated, ID: " << cid << ", piclen: " << piclen[cid] << '\n';

    }else{
      *out << VERBOSE_LOUD << "n out of range in allocate: " << n << '\n';
    }
  }else{
    *out << VERBOSE_LOUD << "cid out of range in allocate: " << cid << '\n';
  }

  //allocated = true;
}

void Picturecontrol::fetch(int myId, Net &net, Client &client)
{
  // deal with fetching
  Unit unit;

  for (int c = 0; c < MAX_CLIENTS; c++) {
    for (int i = 0; i < piclen[c]; i++) {
      // if not active then fetch them
      if (!pic[c][i].getActive() && !pic[c][i].getRequested()) {
        pic[c][i].setRequested(true);
        unit.picfetch.flag = UNIT_PICFETCH;
        unit.picfetch.from = myId;
        unit.picfetch.to = c;
        unit.picfetch.pid = i;
        unit.picfetch.filename[0] = '\0';
        net.addUnit(unit, client);
        *out << VERBOSE_QUIET << "Requesting picture: " << unit.picfetch.pid << '\n';
      }
    }
  }
}

void Picturecontrol::sendFilename(int cid, int myId, int pid, Net &net, Client &client)
{
  // boundary checks for from and to already done in net.cpp
  // pid is checked against piclen but not against files

  if (pid > -1 && pid < (int) files.size()) {
    *out << VERBOSE_QUIET << "sending filename...\n";

    Unit unit;

    unit.picfetch.flag = UNIT_PICFETCH;
    unit.picfetch.from = myId;
    unit.picfetch.to = cid;
    unit.picfetch.pid = pid;

    // don't get rid of path cos when client requests file I need to know where it is!
    /*std::string name;
    size_t found = files[pid].find_last_of('/');
    if (found != std::string::npos) name = files[pid].substr(found+1);
    else name = files[pid];*/

    if (files[pid].size() > MAX_FILENAME_SIZE - 1) {
      *out << VERBOSE_LOUD << "Filename too long: " << files[pid] << '\n';
    }else{

      std::strncpy(unit.picfetch.filename, files[pid].c_str(), MAX_FILENAME_SIZE);
      net.addUnit(unit, client);
    }
  }else{
    *out << VERBOSE_LOUD << "Error, pid out of range for files.size: " << (int) files.size() << '\n';
  }
}

void Picturecontrol::incrementLoaded(int cid, int pid)
{
  pic[cid][pid].incrementLoaded();
}

/*void Picturecontrol::startTransmission(int cid, int pid)
{
  // check not already transmitting
  for (int i = 0; i < (int) transmit.size(); i++) {
    if (transmit[i][0] == cid && transmit[i][1] == pid) {
      *out << VERBOSE_LOUD << "Error with transmission request - already transmitting!\n";
      return;
    }
  }

  if (pid > -1 && pid < piclen) {
    std::vector <int> temp(2);
    temp.push_back(cid);
    temp.push_back(pid);
    //temp.push_back(0); // amount sent
    temp.push_back(transmitFile.size()); // file ID

    transmit.push_back(temp); // add to transmission list

    transmitFile.push_back(fopen(files[pid], "rb")); // open file and store ptr
  }else *out << VERBOSE_LOUD << "Error with start transmission, pid out of bounds: " << pid << '\n';
}

void Picturecontrol::transmitClientSend(int myId, Net &net, Client &client)
{
  // deal with transmission
  Unit unit;

  for (int i = 0; i < (int) transmit.size(); i++) {
    // transmit has cid, pid, file ID
    if (transmit[i][1] > -1 && transmit[i][1] < piclen) {

      if (transmit[i][2] > -1 && transmit[i][2] < transmitFile.size()) {

        unit.picsend.flag = UNIT_PICSEND;
        unit.picsend.pid = transmit[i][1];

        unit.picsend.bytes = fread(unit.picsend.data, PIC_DATA_SIZE, 1, transmitFile[transmit[i][2]]);

        net.addUnit(unit, client);

        if (unit.picsend.bytes < PIC_DATA_SIZE) { // eof assumed
          if (!feof(transmitFile[transmit[i][2]])) {
            *out << VERBOSE_LOUD << "Error, assumed feof but it's not\n";
          }
          // close file
          fclose(transmitFile[transmit[i][2]]);

          // delete
          for (int j = i; j < (int) transmitFile.size() - 1; j++) {
            transmitFile[j] = transmitFile[j+1];
          }
          transmitFile.pop_back();

          for (int j = 0; j < (int) transmit.size(); j++) { // shift references back
            if (transmit[j][2] >= i) transmit[j][2]--;
          }

          // delete this transmit entry
          for (int j = i; j < (int) transmit.size() - 1; j++) {
            transmit[j] = transmit[j+1];
          }
          transmit.pop_back();

          i--; // do next entry next loop
        }

      }else{
        *out << VERBOSE_LOUD << "Error in transmission array, transmit ID out of bounds: " << transmit[i][2] << '\n';
      }

    }else{
      *out << VERBOSE_LOUD << "Error in transmission array, pid out of bounds: " << transmit[i][1] << '\n';
    }

  } // end for

}

void Picturecontrol::transmitClientRecv(int from, int pid, char* bytes)
{
}*/

void Picturecontrol::input(int keys, Net& net, Server &server)
// called by server, transmit picnum to clients
{
  if (clientnum > -1 && clientnum < MAX_CLIENTS && picnum > -1 && picnum < piclen[clientnum]) {
    //&& pic[clientnum][picnum].getAngleY() > -20 && pic[clientnum][picnum].getAngleY() < 20) {
    int oldpicnum = picnum;

    if (keys & KEYS_NEXT) picnum++;
    if (keys & KEYS_BACK) picnum--;

    if (picnum < 0) {
      picnum = 0; // default (no pics found)

      bool found = false;

      // go to end of last client that has something
      for (int c = clientnum - 1; c >= 0; c--) {
        if (piclen[c] > 0) {
          picnum = piclen[c] - 1;
          clientnum = c;
          found = true;
          break;
        }
      }

      if (!found) {
        // count back from end, up until and including current client
        for (int c = MAX_CLIENTS - 1; c >= clientnum; c--) {
          if (piclen[c] > 0) {
            picnum = piclen[c] - 1;
            clientnum = c;
            break;
          }
        }
      }
    }

    if (picnum > piclen[clientnum] - 1) {
      bool found = false;

      for (int c = clientnum + 1; c < MAX_CLIENTS; c++) {
        if (piclen[c] > 0) {
          clientnum = c;
          found = true;
          break;
        }
      }

      if (!found) {
        // go through from start
        for (int c = 0; c < clientnum + 1; c++) {
          if (piclen[c] > 0) {
            clientnum = c;
            break;
          }
        }
      }

      picnum = 0;
    }

    if (oldpicnum != picnum) {
      int direction = (keys & KEYS_NEXT) ? 1 : 0;
      setPicnum(clientnum, picnum, direction); // set up positions

      // transmit picnum
      Unit unit;
      unit.picselect.flag = UNIT_PICSELECT;
      unit.picselect.clientnum = clientnum;
      unit.picselect.picnum = picnum;
      unit.picselect.direction = direction;

      net.addUnitAll(unit, server, -1);

      *out << VERBOSE_LOUD << "Picselect clientnum: " << unit.picselect.clientnum << ", picnum: " << unit.picselect.picnum
      << ", direction: " << unit.picselect.direction << '\n';
    }
  }
}

void Picturecontrol::local(double sync)
{
  for (int c = 0; c < MAX_CLIENTS; c++) {
    for (int i = 0; i < piclen[c]; i++) {
      pic[c][i].move(sync);
    }
  }
}

/*int Picturecontrol::getData(int id, int amount, char* data)
  // transmit whole file - means can view locally afterwards
{
  fread(data, amount, 1, transmitFile[id]);
}*/

void Picturecontrol::go(double sync)
{
  /*int next = (picnum + 1 > piclen - 1) ? 0 : picnum + 1;
  int prev = (picnum - 1 < 0) ? piclen - 1 : picnum - 1;

  pic[picnum].setTargetX(0);
  pic[next].setTargetX(400); // if server is using curses then just make up some value like this
  pic[prev].setTargetX(-400);
  */

  for (int c = 0; c < MAX_CLIENTS; c++) {
    for (int i = 0; i < piclen[c]; i++) {
      pic[c][i].move(sync);
    }
  }

}

