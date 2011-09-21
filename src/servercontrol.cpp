#include "servercontrol.h"
#include "bad.h" // it works without this but not sure how!
#include "network/client.h"
#include "graphicscurses.h"
#include "sound/talk.h" // TODO remove when divide up net.process

Servercontrol::~Servercontrol()
{
  oldmapposition.x = 0, oldmapposition.y = 0;
}

Servercontrol::Servercontrol()
{
  graphics = new GraphicsCurses; // if using GraphicsOpenGL then load textures with picture.load in init

  SERV = true;
  //input = new InputSDL;
  keys = 0;
}

void Servercontrol::init(int port, std::string logfile, std::string picpath, verboseEnum verbosity, char* ip, bool dontGrab, bool fullscreen)
  // initialisation. ip and dontGrab are for client, here because init is virtual
{
  initShared(logfile, verbosity, fullscreen);

  // note number of pending connection requests allowed
  server.init(out, port, 10, flagsize, unitsize);
  server.startListening();
}

void Servercontrol::gameover()
{
  server.closeAll(); // closes any that are still active

  //delete input;

  gameoverShared();
}

void Servercontrol::go()
{
  for (int i = 0; i < MAX_CLIENTS; i++) keyset[i] = 0;

  gameoverState = false;
  lastSent.tv_sec = 0;
  lastSent.tv_usec = 0;

  // loop frequencies in Hz (calls per second)
  int inputfreq = 20;
  int networkfreq = 30;
  int physicsfreq = 100;
  int graphicsfreq = 60; // 60

  int inputdelay = (int) round(1000000.0/inputfreq);
  int networkdelay = (int) round(1000000.0/networkfreq);
  int physicsdelay = (int) round(1000000.0/physicsfreq);
  int graphicsdelay = (int) round(1000000.0/graphicsfreq);

  timeval inputtime, networktime, physicstime, graphicstime;
  timer.update();

  inputtime = timer.getCurrent();
  networktime = timer.getCurrent();
  physicstime = timer.getCurrent();
  graphicstime = timer.getCurrent();

  while (!gameoverState) { // TODO change to keys & KEYS_QUIT
    timer.update();

    doloop(inputdelay, inputtime, &Servercontrol::inputloop);
    doloop(networkdelay, networktime, &Servercontrol::networkloop);
    doloop(physicsdelay, physicstime, &Servercontrol::physicsloop);
    doloop(graphicsdelay, graphicstime, &Servercontrol::graphicsloop);

    usleep(100);
  }
}

void Servercontrol::doloop(int delay, timeval &lasttime, void (Servercontrol::*loopPtr) ())
{
  timeval elapsed;

  elapsed = timer.elapsed(lasttime);

  if (elapsed.tv_sec > 0 || elapsed.tv_usec > delay) {
    lasttime = timer.getCurrent();
    sync = elapsed.tv_sec + elapsed.tv_usec / 1000000.0;
    (*this.*loopPtr)();
  }
}

void Servercontrol::inputloop()
{
  keys = input.check(keys); // TODO does this do anything? Surely client deals with input and sends it into keyset?

  if (keys & KEYS_QUIT) {
    gameoverState = true; // TODO make this so it actually works!
    server.closeAll(); // TODO move to end of game loop so it's done once not in physics also
  }
}

void Servercontrol::sendStatus(int cid)
{
  Bad* bad = badcontrol.getBaddies();

  // transmit positions of baddies and players first?? or just think it'll be there in 0.05 seconds anyway!

  Unit unit;
  unit.bad.flag = UNIT_BAD;
  unit.bad.type = -1;

  for (int i = 0; i < badlen; i++) {
    unit.bad.id = i;

    if (!bad[i].getActive()) unit.bad.status = 0;
    else{
      unit.bad.type = bad[i].getType();
      unit.bad.status = 1;
    }

    net.addUnit(cid, unit, server);
  }

  unit.bullet.flag = UNIT_BULLET;
  unit.bullet.x = 0;
  unit.bullet.y = 0;
  unit.bullet.z = 0;
  unit.bullet.dirY = 0;
  unit.bullet.speedY = 0;
  unit.bullet.owner = 0;

  Bullet* bullet = bulletcontrol.getBullets();

  for (int i = 0; i < bulletlen; i++) {
    unit.bullet.id = i;

    if (bullet[i].getActive()) {
      unit.bullet.active = 1;
      unit.bullet.x = bullet[i].getX();
      unit.bullet.y = bullet[i].getY();
      unit.bullet.z = 0;
      unit.bullet.dirY = bullet[i].getDirY();
      unit.bullet.speedY = bullet[i].getSpeedY();
      unit.bullet.owner = bullet[i].getOwner();
    }else unit.bullet.active = 0;

    net.addUnit(cid, unit, server);
  }

  unit.flag = UNIT_PICALLOC;
  for (int i = 0; i < MAX_CLIENTS; i++) {
    unit.picalloc.id = i;
    unit.picalloc.total = picturecontrol.getPiclen(i);
    if (unit.picalloc.total > 0) net.addUnit(cid, unit, server);
  }

  unit.flag = UNIT_PICSELECT;
  unit.picselect.clientnum = picturecontrol.getClientnum();
  unit.picselect.picnum = picturecontrol.getPicnum();
  unit.picselect.direction = 1;
  net.addUnit(cid, unit, server);
}

void Servercontrol::process(Unit unit)
//void Curses::process(Unit unit, Client &client, Server &server, Talk &talk, int keyset[])
  // process data unit
{
  switch (unit.flag) {
    /*case UNIT_ASSIGN:
      myId = unit.assign.id;
      out << VERBOSE_QUIET << "Received my client ID: " << myId << '\n';
      players++;
      if (myId < 0) {
        out << VERBOSE_LOUD << "Error with assigned ID: " << myId << '\n';
      }
      break;*/
    /*case UNIT_ATTACK:
      badcontrol.makeAttack(unit.attack.wave, timer.getCurrent());
      break;*/
    case UNIT_AUDIO:
      //if (SERV) {
        net.addUnitAll(unit, server, unit.audio.id);
        // out << VERBOSE_LOUD << "Got talk chunk\n";
        //net.addUnitAll(unit, server, -1); // send back to yourself (for testing purposes)
      /*}else{
        // out << VERBOSE_LOUD << "Received chunk for ID: " << unit.audio.id << '\n';
        talk.receive(unit.audio.data);
        // see if buffer is filling up...
        if (talk.getChunksRecvd() > 2) out << VERBOSE_LOUD << "Buffered " << talk.getChunksRecvd() << " chunks\n"; // TODO remove this
      }*/
      break;
    /*case UNIT_BAD:
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
      break;*/
    case UNIT_KEYS:
      if (unit.keys.id > -1 && unit.keys.id < players) {
        keyset[unit.keys.id] = unit.keys.bits;
      }
      break;
    /*case UNIT_LOGOFF:
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
      break;*/
    case UNIT_PICALLOC:
      picturecontrol.allocate(unit.picalloc.id, unit.picalloc.total);
      net.addUnitAll(unit, server, unit.picalloc.id); // send out to others
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
            net.addUnit(unit.picfetch.to, unit, server);
            out << VERBOSE_LOUD << "passing on picfetch\n";
          /*}else{
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
          }*/

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
        picturecontrol.setPicnum(unit.picselect.clientnum, unit.picselect.picnum, unit.picselect.direction); // so it can send out to new clients
        net.addUnitAll(unit, server, -1);
      /*}else{
        out << VERBOSE_QUIET << "Calling picselect, clientnum: " << unit.picselect.clientnum << ", picnum: " << unit.picselect.picnum << '\n';
        picturecontrol.setPicnum(unit.picselect.clientnum, unit.picselect.picnum, unit.picselect.direction);
        out << VERBOSE_QUIET << "Called picselect\n";
      }*/
      break;
    /*case UNIT_POSITION:
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
      break;*/
    case UNIT_TRANSFER:
      //if (SERV) {
        net.addUnit(unit.transfer.to, unit, server);
      /*}else{
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
      }*/
      break;
    case UNIT_TRANSFERFIN:
      //if (SERV) {
        net.addUnitAll(unit, server, unit.transferfin.from); // send out to all so they all addLoaded
      /*}else{
        if (unit.transferfin.to > -1 && unit.transferfin.to < MAX_CLIENTS) {
          if (unit.transferfin.id > -1 && unit.transferfin.id < picturecontrol.getPiclen(unit.transferfin.to)) {
            picturecontrol.getPics()[unit.transferfin.to][unit.transferfin.id].addLoaded();
          }else{
            out << VERBOSE_LOUD << "TransferFIN: error with id: " << unit.transferfin.id << '\n';
          }
        }else{
          out << VERBOSE_LOUD << "TransferFIN: error with to: " << unit.transferfin.to << '\n';
        }
      }*/
      break;
    case UNIT_TRANSFERREQ:
      //if (SERV) {
        net.addUnit(unit.transferreq.to, unit, server);
        // out << "Passing on transfer request...\n";
      /*}else{
        // received transfer request, send next chunk
        // out << "Received transfer request...\n";
        transfercontrol.send(unit.transferreq.to, unit.transferreq.from, unit.transferreq.id, unit.transferreq.filename, net, client);
      }*/
      break;
    default:
      out << VERBOSE_LOUD << "Error, flag not found: " << unit.flag << '\n';
  }
}

void Servercontrol::networkloop()
{
  //Talk talk; // used for net process, but not used by server TODO think about dividing up net.process into cli/serv

  int logoff = server.checkClosedConnections();

  if (logoff > -1) {
    // get next logoff
    for (int i = logoff; i < players - 1; i++) player[i] = player[i+1];
    players--;
    unit.logoff.flag = UNIT_LOGOFF;
    unit.logoff.id = logoff;
    net.addUnitAll(unit, server, -1);
    // note logons will only occur during checkNewConn's
  }

  server.doSelect();

  if (server.checkNewConnections(net)) {
    if (players < MAX_CLIENTS) {
      player[players].reset(badcontrol); // new player logged on, so reset them
      oldposition[players].x = player[players].getX() + 10; // fake an old position so it sends position straight away
      oldposition[players].y = player[players].getY() + 10;
      oldposition[players].z = 0;
      players++;
      sendStatus(players - 1); // id of new client
      //net.sendStatus(players - 1, badcontrol, bulletcontrol, picturecontrol, server);
    }else out << VERBOSE_LOUD << "Error, logon received but too many clients connected!\n";
  }

  server.recvAll();

  unit = server.getDataUnit(net);

  Client client; // not actually used by server (when SERV is true), need to think about this: split net.process?

  while (unit.flag > -1) {
    process(unit);
    //process(unit, client, server, talk, keyset);
    //net.process(unit, myId, player, players, badcontrol, bulletcontrol, picturecontrol, powerupcontrol, transfercontrol, keyset, timer, SERV, talk, client, server);
    unit = server.getDataUnit(net);
  }


  // sending

  timeval elapsed;

  elapsed = timer.elapsed(lastSent);
  if (elapsed.tv_sec > 0 || elapsed.tv_usec > 50000) {
    //cerr << "playerX: " << player.getX() << endl;
    lastSent = timer.getCurrent();

    // send map position
    if (fabs(map.getX() - oldmapposition.x) > 0.001 || fabs(map.getY() - oldmapposition.y) > 0.001
      || fabs(map.getZoom() - oldmapposition.zoom) > 0.001) {
      oldmapposition.x = map.getX();
      oldmapposition.y = map.getY();
      oldmapposition.zoom = map.getZoom();

      unit.map.flag = UNIT_MAP;
      unit.map.x = map.getX();
      unit.map.y = map.getY();
      unit.map.zoom = map.getZoom();

      net.addUnitAll(unit, server, -1);
    }

    // send player positions
    for (int i = 0; i < players; i++) {
      if (fabs(player[i].getX() - oldposition[i].x) > 0.001 || fabs(player[i].getY() - oldposition[i].y) > 0.001) {
        // if player has moved further than tolerance, store position and send

        oldposition[i].x = player[i].getX();
        oldposition[i].y = player[i].getY();
        oldposition[i].z = 0;

        unit.position.flag = UNIT_POSITION;
        unit.position.id = i + ID_PLAYER_MIN;
        unit.position.x = player[i].getX();
        unit.position.y = player[i].getY();
        unit.position.z = 0;

        if (player[i].getX() == 0) out << VERBOSE_LOUD << "WHAHAHAHATT?????\n"; // TODO remove this

        net.addUnitAll(unit, server, -1);
      }
    }

    Bad* bad = badcontrol.getBaddies();

    // send baddie positions
    for (int i = 0; i < badlen; i++) {
      if (bad[i].getActive()) {
        unit.position.flag = UNIT_POSITION;
        unit.position.id = i + ID_BAD_MIN;
        unit.position.x = bad[i].getX();
        unit.position.y = bad[i].getY();
        unit.position.z = 0;

        net.addUnitAll(unit, server, -1);
      }
    }

    /*Picture* pic = picturecontrol.getPics();

    for (int i = 0; i < picturecontrol.getPiclen(); i++) {
      if (pic[i].getActive()) {
        unit.position.flag = UNIT_POSITION;
        unit.position.id = i + ID_PIC_MIN;
        unit.position.x = pic[i].getX();
        unit.position.y = pic[i].getY();
        unit.position.z = pic[i].getZ();

        net.addUnitAll(unit, server, -1);
      }
    }*/

    //Bullet* bullet = bulletcontrol.getBullets();

    /*for (int i = 0; i < bulletlen; i++) {
      if (bullet[i].getActive()) {
      unit.position.flag = UNIT_POSITION;
      unit.position.id = i + ID_BULLET_MIN;
      unit.position.x = bullet[i].getX();
      unit.position.y = bullet[i].getY();
      unit.position.z = 0;

      net.addUnitAll(unit, &server);
      }
      }*/

  }

  server.sendAll();
}

void Servercontrol::physicsloop()
{
  badcontrol.go(player[0].getX(), player[0].getY(), sync, bulletcontrol, powerupcontrol, timer, gameoverState, net, server);

  // note badcontrol.go() might set gameoverState
  if (gameoverState) { // TODO move out to end of main loop cos key press might set this also
    server.closeAll();
  }else{
    bulletcontrol.go(sync, net, server);
    powerupcontrol.go(sync, net, server);

    for (int i = 0; i < players; i++) {
      // TODO make player.go() ?
      player[i].input(i, keyset[i], bulletcontrol, timer, sync, net, server);
      player[i].move(sync);
      //player[i].draw();
      if (!player[i].getDying()) player[i].checkCollision(i, bulletcontrol, badcontrol, powerupcontrol, timer.getCurrent(), net, server);
      else if (player[i].getLives() > -1 && timer.elapsed(player[i].getDeadTime()).tv_sec > 2) player[i].respawn(i, badcontrol, net, server);

      map.input(keyset[i], sync);
      map.move(sync);

      picturecontrol.input(keyset[i], net, server); // pass input for each player
      // remove next and back (single effect) keys from keyset because they've been dealt with now
      keyset[i] &= ~(KEYS_NEXT | KEYS_BACK);
    }

    picturecontrol.go(sync); // should do transmission in own loop, like sound loop in client

  } // !gameover
}

void Servercontrol::graphicsloop()
{
  graphics->drawStart();
  badcontrol.draw();
  bulletcontrol.draw();
  map.draw();
  powerupcontrol.draw();

  for (int i = 0; i < players; i++) player[i].draw();
  graphics->drawStop();

  out.refreshScreen();
  graphics->refresh();

  badcontrol.clear();
  bulletcontrol.clear();
  powerupcontrol.clear();
  for (int i = 0; i < players; i++) player[i].clear();
}

