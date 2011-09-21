#include "curses.h"
#include "graphics.h"
//#include "graphicscurses.h"
//#include "graphicsopengl.h"
#include "sound/talk.h"
/*#include <cmath>
#include <getopt.h>
#include <iostream>
#include <signal.h>
#include "timer.h"
#include "outverbose.h"
#include "badcontrol.h"
#include "bulletcontrol.h"
#include "client.h"
#include "input.h"
#include "network/net.h"
#include "network/network.h"
#include "player.h"
#include "powerupcontrol.h"
#include "server.h"
#include "sound/dev.h"
#include "sound/talk.h"*/

using std::cout;
using std::cerr;
using std::endl;
using std::string;

Curses::Curses()
{
  //graphics = new GraphicsOpenGL();

  SERV = false;

  flagsize = 2; // number of bytes for flag
  // note if change this then need to change uint16_t

  myId = -1;

  players = 0;
  
  sync = 0; // all operations should multiply, and allow for this being zero
}

Curses::~Curses()
{
}

void Curses::gameoverShared()
{
  out.endWin();

  if (out.getOpenedLog()) out.closeLog(); // close log file

  graphics->kill();
  delete graphics;

  cout << "Bye bye!" << endl;
}

void Curses::initUnitSize()
{
  // size of each data unit in bytes (get this right!)
  // this is just the unit, not the flag
  // floats are sent as two ints, so 8 bytes
  //int unitsize[] = { 4, 4, 28, 8, 8, 8, 4 };

  unitsize[UNIT_ASSIGN] = 4;
  unitsize[UNIT_ATTACK] = 4;
  unitsize[UNIT_AUDIO] = 4004;
  unitsize[UNIT_BAD] = 12;
  unitsize[UNIT_BULLET] = 48;
  unitsize[UNIT_KEYS] = 8;
  unitsize[UNIT_LOGOFF] = 4;
  unitsize[UNIT_MAP] = 24;
  unitsize[UNIT_NEWCLIENT] = 4;
  unitsize[UNIT_PLAYER] = 8;
  unitsize[UNIT_PICALLOC] = 8;
  unitsize[UNIT_PICFETCH] = MAX_FILENAME_SIZE + 12;
  //unitsize[UNIT_PICSEND] = PIC_DATA_SIZE + 8;
  unitsize[UNIT_PICSELECT] = 12;
  unitsize[UNIT_POSITION] = 28;
  unitsize[UNIT_POWERUP] = 44;
  unitsize[UNIT_TRANSFER] = 16 + MAX_FILENAME_SIZE + TRANSFER_DATA_SIZE;
  unitsize[UNIT_TRANSFERFIN] = 12;
  unitsize[UNIT_TRANSFERREQ] = 12 + MAX_FILENAME_SIZE;
}

void Curses::initShared(string logfile, verboseEnum verbosity, bool fullscreen)
  // initialisation that's the same for client and server
{
  initUnitSize();

  out.setVerbosity(verbosity);
  out.init(); // init curses

  out.setCursor(0);
  
  out << VERBOSE_LOUD << "Initialising graphics...\n";
  if (!graphics->init(out, graphics->makeWindowInfo(0, 0, 100, 100, true, true, 60, 24, fullscreen, "Title"),
    //"/usr/share/fonts/bitstream-vera/Vera.ttf",
    "/usr/share/fonts/truetype/ubuntu-font-family/Ubuntu-R.ttf",
    42)) {
    // TODO don't hard code font - may not have it!
    // TODO gameover here
  }


  badcontrol.init(out, *graphics);
  bulletcontrol.init(out, *graphics);
  map.init(out, *graphics);
  picturecontrol.init(out, *graphics);
  for (int i = 0; i < MAX_CLIENTS; i++) player[i].init(out, *graphics);
  powerupcontrol.init(out, *graphics);
  transfercontrol.init(out);


  // network
  //int PORT = 3496;
  net.init(out, flagsize, unitsize, MAX_CLIENTS);
  Talk talk; // just for getting chunk bytes
  net.setAudioDataSize(talk.getChunkBytes()); // TODO when use UDP remove this

  if (logfile != "") { // TODO this is from an older program, tidy up
    out.openLog(logfile.c_str());
    out.add("Using logfile: ", VERBOSE_NORMAL);
    out.addln(logfile, VERBOSE_NORMAL);
    out.add("Note: Error messages may be sent on standard error stream, so need to redirect this.\n", VERBOSE_NORMAL);
    out.add("E.g. ./net -l log 2> errlog\n\n", VERBOSE_NORMAL);
  }
}

/*void Curses::process(Unit unit, Client &client, Server &server, Talk &talk, int keyset[])
  // process data unit
{
  switch (unit.flag) {
    case UNIT_ASSIGN:
      myId = unit.assign.id;
      out << VERBOSE_QUIET << "Received my client ID: " << myId << '\n';
      players++;
      if (myId < 0) {
        out << VERBOSE_LOUD << "Error with assigned ID: " << myId << '\n';
      }
      break;
    case UNIT_ATTACK:
      badcontrol.makeAttack(unit.attack.wave, timer.getCurrent());
      break;
    case UNIT_AUDIO:
      if (SERV) {
        net.addUnitAll(unit, server, unit.audio.id);
        // out << VERBOSE_LOUD << "Got talk chunk\n";
        //net.addUnitAll(unit, server, -1); // send back to yourself (for testing purposes)
      }else{
        // out << VERBOSE_LOUD << "Received chunk for ID: " << unit.audio.id << '\n';
        talk.receive(unit.audio.data);
        // see if buffer is filling up...
        if (talk.getChunksRecvd() > 2) out << VERBOSE_LOUD << "Buffered " << talk.getChunksRecvd() << " chunks\n"; // TODO remove this
      }
      break;
    case UNIT_BAD:
      if (unit.bad.id > -1 && unit.bad.id < badlen) {
        if (unit.bad.type > -1) {
          badcontrol.getBaddies()[unit.bad.id].setType((BadType) unit.bad.type);
        }
        if (unit.bad.status > 0) {
          if (unit.bad.status == 1) {
            badcontrol.getBaddies()[unit.bad.id].setActive(true);
            badcontrol.getBaddies()[unit.bad.id].respawn();
          }else if (unit.bad.status == 2) {
            badcontrol.getBaddies()[unit.bad.id].kill();
          }
        }else{
          badcontrol.getBaddies()[unit.bad.id].setActive(false);
        }
      }else{
        out << VERBOSE_LOUD << "Invalid baddie id: " << unit.bad.id << '\n';
      }
      break;
    case UNIT_BULLET:
      if (unit.bullet.id > -1 && unit.bullet.id < bulletlen) {
        if (unit.bullet.active) {
          bulletcontrol.getBullets()[unit.bullet.id].setActive(true);
          bulletcontrol.getBullets()[unit.bullet.id].set(unit.bullet.x, unit.bullet.y, unit.bullet.dirY, unit.bullet.speedY, unit.bullet.owner);
        }else{
          bulletcontrol.getBullets()[unit.bullet.id].setActive(false);
        }
      }else{
        out << VERBOSE_LOUD << "Invalid bullet id: " << unit.bullet.id << '\n';
      }
      break;
    case UNIT_KEYS:
      if (unit.keys.id > -1 && unit.keys.id < players) {
        keyset[unit.keys.id] = unit.keys.bits;
      }
      break;
    case UNIT_LOGOFF:
      if (players > 0) {
        if (myId == unit.logoff.id) {
          out << VERBOSE_LOUD << "Error, server says I logged off!\n";
        }else{
          if (myId > unit.logoff.id) myId--;
          for (int i = unit.logoff.id; i < players - 1; i++) {
            player[i] = player[i+1]; // when new players logon, they are reset in curses.cpp, see checkNewConnections
          }
        }
        players--;
      }
      else out << VERBOSE_LOUD << "Error, logoff received but no one left!\n";
      break;
    case UNIT_NEWCLIENT:
      if (unit.newclient.id > -1 && unit.newclient.id < MAX_CLIENTS) {
        players++;
        player[unit.newclient.id].reset(badcontrol); // not entirely necessary for client, cos just get info from server
      }else{
        out << VERBOSE_LOUD << "Invalid ID for new client\n";
      }
      break;
    case UNIT_PLAYER:
      if (unit.player.id > -1 && unit.player.id < players) {
        if (unit.player.status == 0) {
          player[unit.player.id].kill();
        }else if (unit.player.status == 1) {
          player[unit.player.id].respawnLocal();
        }else out << VERBOSE_LOUD << "Invalid status for player: " << unit.player.status << '\n';
      }else out << VERBOSE_LOUD << "Invalid ID for player: " << unit.player.id << '\n';
      break;
    case UNIT_PICALLOC:
      picturecontrol.allocate(unit.picalloc.id, unit.picalloc.total);
      if (SERV) net.addUnitAll(unit, server, unit.picalloc.id); // send out to others
      break;
    case UNIT_PICFETCH:
      // deal with fetch request (start transmission)
      if (unit.picfetch.from > -1 && unit.picfetch.from < MAX_CLIENTS
        && unit.picfetch.to > -1 && unit.picfetch.to < MAX_CLIENTS) {
        // if going from requester to source then filename is blank, and source is 'to'
        // otherwise filename is filled in, and source is 'from'
        if (unit.picfetch.pid > -1 && ((strlen(unit.picfetch.filename) == 0
          && unit.picfetch.pid < picturecontrol.getPiclen(unit.picfetch.to))
          || (strlen(unit.picfetch.filename) > 0 && unit.picfetch.pid < picturecontrol.getPiclen(unit.picfetch.from)))) {

          if (SERV) {
            // pass onto other client
            net.addUnit(unit.picfetch.to, unit, server);
            out << VERBOSE_LOUD << "passing on picfetch\n";
          }else{
            // only destination client will receive this
            out << VERBOSE_LOUD << "Got picfetch\n";
            if (strlen(unit.picfetch.filename) == 0) picturecontrol.sendFilename(unit.picfetch.from, unit.picfetch.to, unit.picfetch.pid, net, client);
            else if (!transfercontrol.start(unit.picfetch.from, unit.picfetch.to, unit.picfetch.pid, unit.picfetch.filename, net, client)) {
              // already got file
              picturecontrol.loadRemote(unit.picfetch.from, unit.picfetch.pid, unit.picfetch.filename);

              // send this so they count it as loaded
              Unit unit2;
              unit2.flag = UNIT_TRANSFERFIN;
              unit2.transferfin.to = unit.picfetch.from;
              unit2.transferfin.from = unit.picfetch.to;
              unit2.transferfin.id = unit.picfetch.pid;
              net.addUnit(unit2, client);
            }
          }

        }else{
          out << VERBOSE_LOUD << "Invalid picture id in picfetch: " << unit.picfetch.pid << '\n';
        }
      }else{
        out << VERBOSE_LOUD << "Invalid ID for from or to in picfetch, from: " << unit.picfetch.from
        << " to: " << unit.picfetch.to << '\n';
      }
      break;
    case UNIT_PICSELECT:
      if (SERV) {
        picturecontrol.setPicnum(unit.picselect.clientnum, unit.picselect.picnum, unit.picselect.direction); // so it can send out to new clients
        net.addUnitAll(unit, server, -1);
      }else{
        out << VERBOSE_QUIET << "Calling picselect, clientnum: " << unit.picselect.clientnum << ", picnum: " << unit.picselect.picnum << '\n';
        picturecontrol.setPicnum(unit.picselect.clientnum, unit.picselect.picnum, unit.picselect.direction);
        out << VERBOSE_QUIET << "Called picselect\n";
      }
      break;
    case UNIT_POSITION:
      if (unit.position.id > ID_PLAYER_MIN -1 && unit.position.id < players) {
        player[unit.position.id].setX(unit.position.x);
        player[unit.position.id].setY(unit.position.y);
        if (unit.position.x == 0) out << VERBOSE_LOUD << "ZERRERRERREROOOO!!!!!" << '\n'; // TODO remove this
      }else if (unit.position.id > ID_BAD_MIN - 1 && unit.position.id < ID_BAD_MAX) {
        int badid = unit.position.id - ID_BAD_MIN;
        badcontrol.getBaddies()[badid].setX(unit.position.x);
        badcontrol.getBaddies()[badid].setY(unit.position.y);
      }else out << VERBOSE_LOUD << "Invalid position ID: " << unit.position.id << '\n';
      break;
    case UNIT_POWERUP:
      // need to use same ID as server for collection purposes
      if (unit.powerup.id > -1 && unit.powerup.id < poweruplen) {
        if (unit.powerup.collected == -2) {
          // deactivated (out of screen)
          powerupcontrol.getPowerups()[unit.powerup.id].setActive(false);
        }else if (unit.powerup.collected == -1) {
          // activated
          powerupcontrol.getPowerups()[unit.powerup.id].setActive(true);
          powerupcontrol.getPowerups()[unit.powerup.id].set(unit.powerup.x, unit.powerup.y, unit.powerup.speedY, (PowerupType) unit.powerup.type);
        }else if (unit.powerup.collected > -1 && unit.powerup.collected < players) {
          // TODO this could be like bullet (i.e. change collected to active, 0 or 1)
          // The only reason I can think of for storing the player that collected it is for a ghost (local copy) player
          // so I'm leaving this in.
          // (Remember at the minute the server stores everything about the player and the client just does what it's told)
          //player[unit.powerup.collected].collect(pc.getPowerups()[unit.powerup.id].getType());
          powerupcontrol.getPowerups()[unit.powerup.id].setActive(false);
          // as long as powerups are called in this order: clear -> net.process -> draw; then it's ok to set inactive here
          // (server calls draw to show collisions, then checks collisions, so it has to make it dead first so it is cleared)
        }else out << VERBOSE_LOUD << "Invalid value for powerup collected: " << unit.powerup.collected << '\n';
      }else out << VERBOSE_LOUD << "Error with powerup data unit. Invalid ID: " << unit.powerup.id << '\n';
      break;
    case UNIT_TRANSFER:
      if (SERV) {
        net.addUnit(unit.transfer.to, unit, server);
      }else{
        // store chunk
        // out << VERBOSE_LOUD << "Receiving chunk, amount: " << unit.transfer.amount << "...\n";
        if (!transfercontrol.receive(unit)) { //, *this, client)) cb
          // send finished transfer, load picture
          Unit unit2;
          unit2.flag = UNIT_TRANSFERFIN;
          unit2.transferfin.to = unit.transfer.from;
          unit2.transferfin.from = unit.transfer.to;
          unit2.transferfin.id = unit.transfer.id;
          net.addUnit(unit2, client);

          picturecontrol.loadRemote(unit.transfer.from, unit.transfer.id, unit.transfer.filename);
        }
      }
      break;
    case UNIT_TRANSFERFIN:
      if (SERV) {
        net.addUnitAll(unit, server, unit.transferfin.from); // send out to all so they all addLoaded
      }else{
        if (unit.transferfin.to > -1 && unit.transferfin.to < MAX_CLIENTS) {
          if (unit.transferfin.id > -1 && unit.transferfin.id < picturecontrol.getPiclen(unit.transferfin.to)) {
            picturecontrol.getPics()[unit.transferfin.to][unit.transferfin.id].addLoaded();
          }else{
            out << VERBOSE_LOUD << "TransferFIN: error with id: " << unit.transferfin.id << '\n';
          }
        }else{
          out << VERBOSE_LOUD << "TransferFIN: error with to: " << unit.transferfin.to << '\n';
        }
      }
      break;
    case UNIT_TRANSFERREQ:
      if (SERV) {
        net.addUnit(unit.transferreq.to, unit, server);
        // out << "Passing on transfer request...\n";
      }else{
        // received transfer request, send next chunk
        // out << "Received transfer request...\n";
        transfercontrol.send(unit.transferreq.to, unit.transferreq.from, unit.transferreq.id, unit.transferreq.filename, net, client);
      }
      break;
    default:
      out << VERBOSE_LOUD << "Error, flag not found: " << unit.flag << '\n';
  }
}*/

