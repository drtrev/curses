#include "clientcontrol.h"
#include "graphicsopengl.h"
#include "network/server.h"

Clientcontrol::~Clientcontrol()
{
}

Clientcontrol::Clientcontrol()
{
  graphics = new GraphicsOpenGL;

  keys = 0, keysOld = 0;
  SERV = false;
  //input = new InputX11; if use this then delete in gameover
}

void Clientcontrol::init(int port, std::string logfile, std::string readPath, verboseEnum verbosity, char* ip, bool dontGrab, bool fullscreen)
{
  initShared(logfile, verbosity, fullscreen); // init everything shared between client and server

  client.init(out, port, flagsize, unitsize);

  out << VERBOSE_QUIET << "Using ip address: " << ip << '\n';

  if (!client.openConnection(ip)) {
    gameover();
    std::cerr << "Could not connect to server at address: " << ip << std::endl;
    exit(1);
  }

  if (!dontGrab) input.grab();

  std::string writePath = "/tmp/trev/pics";

  picturecontrol.setReadPath(readPath);
  picturecontrol.setWritePath(writePath);
  //picturecontrol.load(picpath, net, client);
  transfercontrol.setReadPath(readPath);
  transfercontrol.setWritePath(writePath);

  soundDev.initOutput(out);
  talk.initOutput(out);
  talk.setSoundDev(soundDev);
  if (soundDev.grab() && talk.openCaptureDevice()) {
    talk.setPlayDevice(soundDev.getPlayDevice());
    talk.setCaptureDevice(soundDev.getCaptureDevice());
    talk.initStream();
    talk.captureStart();
  }

  out << VERBOSE_QUIET << "finished init\n";
}

void Clientcontrol::gameover()
{
  if (talk.getCapturing()) talk.captureStop();
  if (client.getConnected()) client.closeConnection();
  if (input.getGrabbed()) input.release(); // release keyboard

  //delete input;

  gameoverShared();
}

void Clientcontrol::go()
  // main loop
{
  // client stats
  timer.update(); // to set current time for stats
  timeval statProgramStart = timer.getCurrent();

  // TODO should these always be the same as server freq's?
  // if so put in curses.h!
  // loop frequencies in Hz (calls per second)
  int inputfreq = 20; // if too low then press and release will be processed in same loop and key will be ignored
  int networkfreq = 30; // if too low then graphics appear jerky without dead reckoning
  int physicsfreq = 100;
  int graphicsfreq = 60; // was 60
  int soundfreq = 40;
  int transferfreq = 10;

  int inputdelay = (int) round(1000000.0/inputfreq); // delay in microseconds
  int networkdelay = (int) round(1000000.0/networkfreq);
  int physicsdelay = (int) round(1000000.0/physicsfreq);
  int graphicsdelay = (int) round(1000000.0/graphicsfreq);
  int sounddelay = (int) round(1000000.0/soundfreq);
  int transferdelay = (int) round(1000000.0/transferfreq);

  timeval inputtime, networktime, physicstime, graphicstime, soundtime, transfertime;

  // timer updated above
  inputtime = timer.getCurrent();
  networktime = timer.getCurrent();
  physicstime = timer.getCurrent();
  graphicstime = timer.getCurrent();
  soundtime = timer.getCurrent();
  transfertime = timer.getCurrent();

  while (!(keys & KEYS_QUIT) && client.getConnected()) {
    timer.update();

    doloop(inputdelay, inputtime, &Clientcontrol::inputloop);
    doloop(networkdelay, networktime, &Clientcontrol::networkloop);
    doloop(physicsdelay, physicstime, &Clientcontrol::physicsloop);
    doloop(graphicsdelay, graphicstime, &Clientcontrol::graphicsloop);
    doloop(sounddelay, soundtime, &Clientcontrol::soundloop);
    doloop(transferdelay, transfertime, &Clientcontrol::transferloop);

    usleep(100);
  }

  timeval statProgramDuration = timer.elapsed(statProgramStart);
  double statProgramDurationDbl = statProgramDuration.tv_sec + statProgramDuration.tv_usec / 1000000.0;
  int bytesRecv = client.getStatRecvd();
  int bytesSend = client.getStatSent();
  int total = bytesRecv + bytesSend;
  double rate = total / statProgramDurationDbl;
  std::cerr << "Stats.\nRecv: " << bytesRecv << "\nSent: " << bytesSend << "\nTotal: " << total
    << "\nDuration: " << statProgramDurationDbl << "\nRate: " << rate << "\n";
}

void Clientcontrol::doloop(int delay, timeval &lasttime, void (Clientcontrol::*loopPtr) ())
{
  timeval elapsed;

  elapsed = timer.elapsed(lasttime);

  if (elapsed.tv_sec > 0 || elapsed.tv_usec > delay) {
    lasttime = timer.getCurrent();
    sync = elapsed.tv_sec + elapsed.tv_usec / 1000000.0;
    (*this.*loopPtr)();
  }
}

void Clientcontrol::inputloop()
{
  if (input.getGrabbed()) keys = input.check(keys);

  if (keys != keysOld) {
    // if keys have changed, send them to server

    keysOld = keys;

    // make unit
    unit.keys.flag = UNIT_KEYS;
    unit.keys.id = myId;
    unit.keys.bits = keys;

    net.addUnit(unit, client);

  }
}

void Clientcontrol::process(Unit unit)
//void Curses::process(Unit unit, Client &client, Server &server, Talk &talk, int keyset[])
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
      //if (SERV) {
        //net.addUnitAll(unit, server, unit.audio.id);
        // out << VERBOSE_LOUD << "Got talk chunk\n";
        //net.addUnitAll(unit, server, -1); // send back to yourself (for testing purposes)
      //}else{
        // out << VERBOSE_LOUD << "Received chunk for ID: " << unit.audio.id << '\n';
        talk.receive(unit.audio.data);
        // see if buffer is filling up...
        if (talk.getChunksRecvd() > 2) out << VERBOSE_LOUD << "Buffered " << talk.getChunksRecvd() << " chunks\n"; // TODO remove this
      //}
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
    /*case UNIT_KEYS:
      if (unit.keys.id > -1 && unit.keys.id < players) {
        keyset[unit.keys.id] = unit.keys.bits;
      }
      break;*/
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
    case UNIT_MAP:
      map.setX(unit.map.x);
      map.setY(unit.map.y);
      map.setZoom(unit.map.zoom);
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
      //if (SERV) net.addUnitAll(unit, server, unit.picalloc.id); // send out to others
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

          //if (SERV) {
            // pass onto other client
            //net.addUnit(unit.picfetch.to, unit, server);
            //out << VERBOSE_LOUD << "passing on picfetch\n";
          //}else{
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
          //}

        }else{
          out << VERBOSE_LOUD << "Invalid picture id in picfetch: " << unit.picfetch.pid << '\n';
        }
      }else{
        out << VERBOSE_LOUD << "Invalid ID for from or to in picfetch, from: " << unit.picfetch.from
        << " to: " << unit.picfetch.to << '\n';
      }
      break;
    case UNIT_PICSELECT:
      //if (SERV) {
       // picturecontrol.setPicnum(unit.picselect.clientnum, unit.picselect.picnum, unit.picselect.direction); // so it can send out to new clients
       // net.addUnitAll(unit, server, -1);
      //}else{
        out << VERBOSE_QUIET << "Calling picselect, clientnum: " << unit.picselect.clientnum << ", picnum: " << unit.picselect.picnum << '\n';
        picturecontrol.setPicnum(unit.picselect.clientnum, unit.picselect.picnum, unit.picselect.direction);
        out << VERBOSE_QUIET << "Called picselect\n";
      //}
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
      //if (SERV) {
       // net.addUnit(unit.transfer.to, unit, server);
      //}else{
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
      //}
      break;
    case UNIT_TRANSFERFIN:
      //if (SERV) {
        //net.addUnitAll(unit, server, unit.transferfin.from); // send out to all so they all incrementLoaded
      //}else{
        if (unit.transferfin.to > -1 && unit.transferfin.to < MAX_CLIENTS) {
          if (unit.transferfin.id > -1 && unit.transferfin.id < picturecontrol.getPiclen(unit.transferfin.to)) {
            //picturecontrol.getPics()[unit.transferfin.to][unit.transferfin.id].addLoaded();
            picturecontrol.incrementLoaded(unit.transferfin.to, unit.transferfin.id);
          }else{
            out << VERBOSE_LOUD << "TransferFIN: error with id: " << unit.transferfin.id << '\n';
          }
        }else{
          out << VERBOSE_LOUD << "TransferFIN: error with to: " << unit.transferfin.to << '\n';
        }
      //}
      break;
    case UNIT_TRANSFERREQ:
      //if (SERV) {
        //net.addUnit(unit.transferreq.to, unit, server);
        //// out << "Passing on transfer request...\n";
      //}else{
        // received transfer request, send next chunk
        // out << "Received transfer request...\n";
        transfercontrol.send(unit.transferreq.to, unit.transferreq.from, unit.transferreq.id, unit.transferreq.filename, net, client);
      //}
      break;
    default:
      out << VERBOSE_LOUD << "Error, flag not found: " << unit.flag << '\n';
  }
}

void Clientcontrol::networkloop()
{
  //out << VERBOSE_QUIET << "started network loop\n";

  //int keyset[MAX_CLIENTS]; // used by server only

  client.doSelect();
  unit = client.recvDataUnit(net);

  //Server server; // not actually used by client (when SERV is false), need to think about this: split net.process?

  while (unit.flag > -1) {
    process(unit);
    //process(unit, client, server, talk, keyset);
    //net.process(unit, myId, player, players, badcontrol, bulletcontrol, picturecontrol, powerupcontrol, transfercontrol, keyset, timer, SERV, talk, client, server);
    client.doSelect();
    unit = client.recvDataUnit(net);
  }

  client.sendData();

  //out << VERBOSE_QUIET << "finished network loop\n";
}

void Clientcontrol::physicsloop()
{
  //out << VERBOSE_QUIET << "started physics loop\n";

  // do any local calculations (e.g. explosions; moving objects like bullets that have no randomness)
  badcontrol.local(sync);
  bulletcontrol.local(sync);
  powerupcontrol.local(sync);
  for (int i = 0; i < players; i++) player[i].local(sync);
  picturecontrol.local(sync);

  //out << VERBOSE_QUIET << "finished physics loop\n";
}

void Clientcontrol::graphicsloop()
{
  // important the way this is done:
  // graphics: draw->refresh->clear, then status changes in other loops
  // this means output is cleared in the correct location, and avoids problems of
  // clear->draw->refresh, then status change: deactivate which would mean deactivated
  // objects are not cleared

  graphics->drawStart();
  badcontrol.draw();
  bulletcontrol.draw();
  map.draw();
  powerupcontrol.draw();

  for (int i = 0; i < players; i++) player[i].draw();

  picturecontrol.draw(players);

  graphics->drawStop();

  out.refreshScreen();
  graphics->refresh();

  badcontrol.clear();
  bulletcontrol.clear();
  powerupcontrol.clear();
  for (int i = 0; i < players; i++) player[i].clear();
}

void Clientcontrol::soundloop()
{
  // temporarily stop this. if (talk.getCapturing()) talk.capture(myId, net, client);
  // temp stop. if (soundDev.checkPlayContext()) talk.update();
}

void Clientcontrol::transferloop()
// deal with file transfer
{
  static bool loadedpics = false;

  if (myId > -1 && !loadedpics) {
    out << VERBOSE_LOUD << "Calling load...\n";
    picturecontrol.load(myId, net, client); // loads if path was set
    loadedpics = true;
  }

  //out << VERBOSE_LOUD << "Calling fetch\n";
  picturecontrol.fetch(myId, net, client); // deal with fetching

  // net files transmission requests using startTransmission()
  //picturecontrol.transmitClientSend(myId, net, client); // deal with sending data, processing transmission requests
  //out << VERBOSE_LOUD << "Calling transfer.go\n";
  transfercontrol.go(net, client); // process transactions

  // server passes on data straight to client, which calls transfercontrol.receive() from net.cpp
  //out << VERBOSE_LOUD << "Finished transferloop\n";
}

