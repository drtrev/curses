#include "network/net.h"
//#include "bad.h"
//#include "badcontrol.h"
//#include "bullet.h"
//#include "bulletcontrol.h"
#include <cmath>
#include <cstring>
#include "network/client.h"
#include "network/server.h"
#include "outverbose.h"
//#include "player.h"
//#include "picture.h"
//#include "picturecontrol.h"
//#include "powerupcontrol.h"
//#include "ringbuf.h"
//#include "sound/talk.h"
//#include "timer.h"
//#include "transfer/transfercontrol.h"

using std::memcpy;

Net::Net()
{
  audioDataSize = 0;
  maxSize = -1; // defined on init
}

Net::~Net()
{

  //if (audioDataSize > 0) delete [] audioData;
}

void Net::init(Outverbose &o, int f, int u[], int m)
{
  out = &o;
  flagsize = f;
  memcpy(unitsize, u, UNITS*sizeof(int));
  maxClients = m;

  int largestUnit = 0;

  for (int i = 0; i < UNITS; i++) {
    if (unitsize[i] > largestUnit) largestUnit = unitsize[i];
  }

  maxSize = largestUnit + flagsize;
}

void Net::setAudioDataSize(int s)
{
  //audioData = new char[s];
  audioDataSize = s;
}

int Net::readInt(char* &data)
  // reads an integer and advances the pointer
{
  int i = ntohl( *reinterpret_cast<uint32_t*>( data ) );
  data += 4; // advance 4 bytes
  return i;
}

float Net::readFloat1(char* &data)
  // takes four bytes that have been reinterpreted as int and
  // gets them back to a float
{
  //uint32_t temp = ntohl( *reinterpret_cast<uint32_t*>( &data[0] ) );

  float f = *reinterpret_cast<float*>( data );

  data += 4;

  return f;
}

float Net::readFloat2(char* &data)
  // takes eight bytes, two integers, the first representing the integral part of the float
  // and the second representing the fractional part * 10000
{
  // implicit cast to signed int
  int32_t i1 = ntohl( *reinterpret_cast<uint32_t*>( &data[0] ) );

  int32_t i2 = ntohl( *reinterpret_cast<uint32_t*>( &data[4] ) );

  float f = i1 + i2 / 10000.0;

  data += 8; // 8 bytes

  return f;
}

int Net::writeShortInt(Ringbuf &buf, uint16_t data)
{
  data = htons(data);

  return buf.write(reinterpret_cast<char*>( &data ), 2);
}

int Net::writeInt(Ringbuf &buf, uint32_t data)
{
  data = htonl(data);

  return buf.write(reinterpret_cast<char*>( &data ), 4);
}

int Net::writeFloat1(Ringbuf &buf, float data)
{
  return buf.write(reinterpret_cast<char*>( &data ), 4);
}

int Net::writeFloat2(Ringbuf &buf, float data)
{
  float integral = 0;
  float fraction = roundf( modff(data, &integral) * 10000 ); // just keep 4 decimal places

  return writeInt(buf, static_cast<uint32_t>( integral )) + writeInt(buf, static_cast<uint32_t>( fraction ));
}

Unit Net::bytesToUnit(int unitId, char* data)
  // converts from char* to Unit
  // trusts that unit has already been correctly identified with error checks
{
  Unit unit;

  unit.flag = -1;

  if (unitId > -1 && unitId < UNITS) {
    unit.flag = unitId;

    char* ptr = &data[2]; // after flag

    switch (unitId) {
      case UNIT_ASSIGN:
        unit.assign.id = readInt(ptr);
        break;
      case UNIT_ATTACK:
        unit.attack.wave = readInt(ptr);
        break;
      case UNIT_AUDIO:
        unit.audio.id = readInt(ptr);
        memcpy(unit.audio.data, ptr, audioDataSize); // need to advance the ptr if reading anything after this
        break;
      case UNIT_BAD:
        unit.bad.id = readInt(ptr);
        unit.bad.type = readInt(ptr);
        unit.bad.status = readInt(ptr);
        break;
      case UNIT_BULLET:
        unit.bullet.id = readInt(ptr);
        unit.bullet.active = readInt(ptr);
        unit.bullet.x = readFloat2(ptr);
        unit.bullet.y = readFloat2(ptr);
        unit.bullet.z = readFloat2(ptr);
        unit.bullet.dirY = readInt(ptr);
        unit.bullet.speedY = readFloat2(ptr);
        unit.bullet.owner = readInt(ptr);
        break;
      case UNIT_KEYS:
        unit.keys.id = readInt(ptr);
        unit.keys.bits = readInt(ptr);
        break;
      case UNIT_LOGOFF:
        unit.logoff.id = readInt(ptr);
        break;
      case UNIT_MAP:
        unit.map.x = readFloat2(ptr);
        unit.map.y = readFloat2(ptr);
        unit.map.zoom = readFloat2(ptr);
        break;
      case UNIT_NEWCLIENT:
        unit.newclient.id = readInt(ptr);
        break;
      case UNIT_PLAYER:
        unit.player.id = readInt(ptr);
        unit.player.status = readInt(ptr);
        break;
      case UNIT_PICALLOC:
        unit.picalloc.id = readInt(ptr);
        unit.picalloc.total = readInt(ptr);
        break;
      case UNIT_PICFETCH:
        unit.picfetch.from = readInt(ptr);
        unit.picfetch.to = readInt(ptr);
        unit.picfetch.pid = readInt(ptr);
        memcpy(unit.picfetch.filename, ptr, MAX_FILENAME_SIZE);
        break;
      /*case UNIT_PICSEND:
        unit.picsend.cid = readInt(ptr);
        unit.picsend.pid = readInt(ptr);
        memcpy(unit.picsend.bytes, ptr, audioDataSize);
        break;*/
      case UNIT_PICSELECT:
        unit.picselect.clientnum = readInt(ptr);
        unit.picselect.picnum = readInt(ptr);
        unit.picselect.direction = readInt(ptr);
        break;
      case UNIT_POSITION:
        unit.position.id = readInt(ptr);
        unit.position.x = readFloat2(ptr);
        unit.position.y = readFloat2(ptr);
        unit.position.z = readFloat2(ptr);
        break;
      case UNIT_POWERUP:
        unit.powerup.id = readInt(ptr);
        unit.powerup.x = readFloat2(ptr);
        unit.powerup.y = readFloat2(ptr);
        unit.powerup.z = readFloat2(ptr);
        unit.powerup.speedY = readFloat2(ptr);
        unit.powerup.type = readInt(ptr);
        unit.powerup.collected = readInt(ptr);
        break;
      case UNIT_TRANSFER:
        unit.transfer.to = readInt(ptr);
        unit.transfer.from = readInt(ptr);
        unit.transfer.id = readInt(ptr);
        //*out << VERBOSE_LOUD << "bytesToUnit, to: " << unit.transfer.to << ", from: " << unit.transfer.from << ", id: " << unit.transfer.id << '\n';
        memcpy(unit.transfer.filename, ptr, MAX_FILENAME_SIZE); // need to advance the ptr
        ptr += MAX_FILENAME_SIZE;
        //*out << "filename: " << unit.transfer.filename << '\n';
        memcpy(unit.transfer.data, ptr, TRANSFER_DATA_SIZE);
        ptr += TRANSFER_DATA_SIZE;
        unit.transfer.amount = readInt(ptr);
        //*out << VERBOSE_LOUD << "bytesToUnit, amount: " << unit.transfer.amount << '\n';
        break;
      case UNIT_TRANSFERFIN:
        unit.transferfin.to = readInt(ptr);
        unit.transferfin.from = readInt(ptr);
        unit.transferfin.id = readInt(ptr);
        break;
      case UNIT_TRANSFERREQ:
        unit.transferreq.to = readInt(ptr);
        unit.transferreq.from = readInt(ptr);
        unit.transferreq.id = readInt(ptr);
        memcpy(unit.transferreq.filename, ptr, MAX_FILENAME_SIZE);
        break;
    }

  }

  return unit;
}

int Net::unitToBytes(Unit unit, char* data)
  // convert to network safe bytes
{
  int size = -1;

  if (unit.flag > -1 && unit.flag < UNITS) {
    Ringbuf buf(maxSize);

    size = writeShortInt(buf, (uint16_t) unit.flag);

    switch (unit.flag) {
      case UNIT_ASSIGN:
        size += writeInt(buf, unit.assign.id);
        break;
      case UNIT_ATTACK:
        size += writeInt(buf, unit.attack.wave);
        break;
      case UNIT_AUDIO:
        size += writeInt(buf, unit.audio.id);
        size += buf.write(unit.audio.data, audioDataSize);
        break;
      case UNIT_BAD:
        size += writeInt(buf, unit.bad.id);
        size += writeInt(buf, unit.bad.type);
        size += writeInt(buf, unit.bad.status);
        break;
      case UNIT_BULLET:
        size += writeInt(buf, unit.bullet.id);
        size += writeInt(buf, unit.bullet.active);
        size += writeFloat2(buf, unit.bullet.x);
        size += writeFloat2(buf, unit.bullet.y);
        size += writeFloat2(buf, unit.bullet.z);
        size += writeInt(buf, unit.bullet.dirY);
        size += writeFloat2(buf, unit.bullet.speedY);
        size += writeInt(buf, unit.bullet.owner);
        break;
      case UNIT_KEYS:
        size += writeInt(buf, unit.keys.id);
        size += writeInt(buf, unit.keys.bits);
        break;
      case UNIT_LOGOFF:
        size += writeInt(buf, unit.logoff.id);
        break;
      case UNIT_MAP:
        size += writeFloat2(buf, unit.map.x);
        size += writeFloat2(buf, unit.map.y);
        size += writeFloat2(buf, unit.map.zoom);
        break;
      case UNIT_NEWCLIENT:
        size += writeInt(buf, unit.newclient.id);
        break;
      case UNIT_PLAYER:
        size += writeInt(buf, unit.player.id);
        size += writeInt(buf, unit.player.status);
        break;
      case UNIT_PICALLOC:
        size += writeInt(buf, unit.picalloc.id);
        size += writeInt(buf, unit.picalloc.total);
        break;
      case UNIT_PICFETCH:
        size += writeInt(buf, unit.picfetch.from);
        size += writeInt(buf, unit.picfetch.to);
        size += writeInt(buf, unit.picfetch.pid);
        size += buf.write(unit.picfetch.filename, MAX_FILENAME_SIZE);
        break;
      /*case UNIT_PICSEND:
        size += writeInt(buf, unit.picsend.cid);
        size += writeInt(buf, unit.picsend.pid);
        size += buf.write(unit.picsend.bytes, pictureDataSize);
        break;*/
      case UNIT_PICSELECT:
        size += writeInt(buf, unit.picselect.clientnum);
        size += writeInt(buf, unit.picselect.picnum);
        size += writeInt(buf, unit.picselect.direction);
        break;
      case UNIT_POSITION:
        size += writeInt(buf, unit.position.id);
        size += writeFloat2(buf, unit.position.x);
        size += writeFloat2(buf, unit.position.y);
        size += writeFloat2(buf, unit.position.z);
        break;
      case UNIT_POWERUP:
        size += writeInt(buf, unit.powerup.id);
        size += writeFloat2(buf, unit.powerup.x);
        size += writeFloat2(buf, unit.powerup.y);
        size += writeFloat2(buf, unit.powerup.z);
        size += writeFloat2(buf, unit.powerup.speedY);
        size += writeInt(buf, unit.powerup.type);
        size += writeInt(buf, unit.powerup.collected);
        break;
      case UNIT_TRANSFER:
        size += writeInt(buf, unit.transfer.to);
        size += writeInt(buf, unit.transfer.from);
        size += writeInt(buf, unit.transfer.id);
        size += buf.write(unit.transfer.filename, MAX_FILENAME_SIZE);
        size += buf.write(unit.transfer.data, TRANSFER_DATA_SIZE);
        size += writeInt(buf, unit.transfer.amount);
        //*out << VERBOSE_LOUD << "unitToBytes, amount: " << unit.transfer.amount << '\n';
        //*out << VERBOSE_LOUD << "size: " << size << '\n';
        break;
      case UNIT_TRANSFERFIN:
        size += writeInt(buf, unit.transferfin.to);
        size += writeInt(buf, unit.transferfin.from);
        size += writeInt(buf, unit.transferfin.id);
        break;
      case UNIT_TRANSFERREQ:
        size += writeInt(buf, unit.transferreq.to);
        size += writeInt(buf, unit.transferreq.from);
        size += writeInt(buf, unit.transferreq.id);
        size += buf.write(unit.transferreq.filename, MAX_FILENAME_SIZE);
        break;
    }

    if (size != buf.getLength()) {
      *out << VERBOSE_LOUD << "Error writing to buffer, dropping data unit. Size: "
           << size << '\n';
      size = -1;
    }else{
      if (size > 0) buf.read(data, size);
      else {
        *out << VERBOSE_LOUD << "Error. Size is zero\n";
      }
    }

  }else{
    *out << VERBOSE_LOUD << "Invalid flag in unitToBytes\n";
  }

  return size;
}

void Net::addUnit(Unit unit, Client &client)
{
  char *data = new char[maxSize];
  int size = unitToBytes(unit, data);

  if (size != flagsize + unitsize[unit.flag]) {
    *out << VERBOSE_LOUD << "Error converting unit to bytes. Size: " << size << '\n';
  }else{
    int added = client.addData(data, size); // checks if it will fit - writes all or nothing

    if (added != flagsize + unitsize[unit.flag]) {
      *out << VERBOSE_LOUD << "Error with addData - client buffer full\n";
    }
  }

  delete [] data;

}

void Net::addUnit(int cid, Unit unit, Server &server)
{
  char *data = new char[maxSize];

  int size = unitToBytes(unit, data);

  if (size != flagsize + unitsize[unit.flag]) {
    *out << VERBOSE_LOUD << "Error converting unit to bytes. Size: " << size << '\n';
  }else{
    int added = server.addData(cid, data, size); // checks if it will fit - writes all or nothing

    if (added != flagsize + unitsize[unit.flag]) {
      *out << VERBOSE_LOUD << "Error with addData - client buffer full for client: "
           << cid << '\n';
    }
  }

  delete [] data;

}

void Net::addUnitAll(Unit unit, Server &server, int source)
  // add to all client buffers except source (where the data originates from)
  // source can be set to -1 to send out to them all
{
  char *data = new char[maxSize];

  int size = unitToBytes(unit, data);

  if (size != flagsize + unitsize[unit.flag]) {
    *out << VERBOSE_LOUD << "Error converting unit to bytes. Size: " << size << '\n';
  }else{
    server.addDataAll(data, size, source); // error checks within
  }

  delete [] data;
}

/*void Net::process(Unit unit, int &myId, Player player[], int &players, Badcontrol &badc, Bulletcontrol &bullc, Picturecontrol &picc, Powerupcontrol &pc, Transfercontrol &transferc, int keyset[], Timer &timer, bool SERV, Talk &talk, Client &client, Server &server)
  // process data unit
{
  switch (unit.flag) {
    case UNIT_ASSIGN:
      myId = unit.assign.id;
      *out << VERBOSE_QUIET << "Received my client ID: " << myId << '\n';
      players++;
      if (myId < 0) {
        *out << VERBOSE_LOUD << "Error with assigned ID: " << myId << '\n';
      }
      break;
    case UNIT_ATTACK:
      badc.makeAttack(unit.attack.wave, timer.getCurrent());
      break;
    case UNIT_AUDIO:
      if (SERV) {
        addUnitAll(unit, server, unit.audio.id);
        // *out << VERBOSE_LOUD << "Got talk chunk\n";
        //addUnitAll(unit, server, -1); // send back to yourself (for testing purposes)
      }else{
        // *out << VERBOSE_LOUD << "Received chunk for ID: " << unit.audio.id << '\n';
        talk.receive(unit.audio.data);
        // see if buffer is filling up...
        if (talk.getChunksRecvd() > 2) *out << VERBOSE_LOUD << "Buffered " << talk.getChunksRecvd() << " chunks\n"; // TODO remove this
      }
      break;
    case UNIT_BAD:
      if (unit.bad.id > -1 && unit.bad.id < badlen) {
        if (unit.bad.type > -1) {
          badc.getBaddies()[unit.bad.id].setType((BadType) unit.bad.type);
        }
        if (unit.bad.status > 0) {
          if (unit.bad.status == 1) {
            badc.getBaddies()[unit.bad.id].setActive(true);
            badc.getBaddies()[unit.bad.id].respawn();
          }else if (unit.bad.status == 2) {
            badc.getBaddies()[unit.bad.id].kill();
          }
        }else{
          badc.getBaddies()[unit.bad.id].setActive(false);
        }
      }else{
        *out << VERBOSE_LOUD << "Invalid baddie id: " << unit.bad.id << '\n';
      }
      break;
    case UNIT_BULLET:
      if (unit.bullet.id > -1 && unit.bullet.id < bulletlen) {
        if (unit.bullet.active) {
          bullc.getBullets()[unit.bullet.id].setActive(true);
          bullc.getBullets()[unit.bullet.id].set(unit.bullet.x, unit.bullet.y, unit.bullet.dirY, unit.bullet.speedY, unit.bullet.owner);
        }else{
          bullc.getBullets()[unit.bullet.id].setActive(false);
        }
      }else{
        *out << VERBOSE_LOUD << "Invalid bullet id: " << unit.bullet.id << '\n';
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
          *out << VERBOSE_LOUD << "Error, server says I logged off!\n";
        }else{
          if (myId > unit.logoff.id) myId--;
          for (int i = unit.logoff.id; i < players - 1; i++) {
            player[i] = player[i+1]; // when new players logon, they are reset in curses.cpp, see checkNewConnections
          }
        }
        players--;
      }
      else *out << VERBOSE_LOUD << "Error, logoff received but no one left!\n";
      break;
    case UNIT_NEWCLIENT:
      if (unit.newclient.id > -1 && unit.newclient.id < maxClients) {
        players++;
        player[unit.newclient.id].reset(badc); // not entirely necessary for client, cos just get info from server
      }else{
        *out << VERBOSE_LOUD << "Invalid ID for new client\n";
      }
      break;
    case UNIT_PLAYER:
      if (unit.player.id > -1 && unit.player.id < players) {
        if (unit.player.status == 0) {
          player[unit.player.id].kill();
        }else if (unit.player.status == 1) {
          player[unit.player.id].respawnLocal();
        }else *out << VERBOSE_LOUD << "Invalid status for player: " << unit.player.status << '\n';
      }else *out << VERBOSE_LOUD << "Invalid ID for player: " << unit.player.id << '\n';
      break;
    case UNIT_PICALLOC:
      picc.allocate(unit.picalloc.id, unit.picalloc.total);
      if (SERV) addUnitAll(unit, server, unit.picalloc.id); // send out to others
      break;
    case UNIT_PICFETCH:
      // deal with fetch request (start transmission)
      if (unit.picfetch.from > -1 && unit.picfetch.from < maxClients
        && unit.picfetch.to > -1 && unit.picfetch.to < maxClients) {
        // if going from requester to source then filename is blank, and source is 'to'
        // otherwise filename is filled in, and source is 'from'
        if (unit.picfetch.pid > -1 && ((strlen(unit.picfetch.filename) == 0
          && unit.picfetch.pid < picc.getPiclen(unit.picfetch.to))
          || (strlen(unit.picfetch.filename) > 0 && unit.picfetch.pid < picc.getPiclen(unit.picfetch.from)))) {

          if (SERV) {
            // pass onto other client
            addUnit(unit.picfetch.to, unit, server);
            *out << VERBOSE_LOUD << "passing on picfetch\n";
          }else{
            // only destination client will receive this
            *out << VERBOSE_LOUD << "Got picfetch\n";
            if (strlen(unit.picfetch.filename) == 0) picc.sendFilename(unit.picfetch.from, unit.picfetch.to, unit.picfetch.pid, *this, client);
            else if (!transferc.start(unit.picfetch.from, unit.picfetch.to, unit.picfetch.pid, unit.picfetch.filename, *this, client)) {
              // already got file
              picc.loadRemote(unit.picfetch.from, unit.picfetch.pid, unit.picfetch.filename);

              // send this so they count it as loaded
              Unit unit2;
              unit2.flag = UNIT_TRANSFERFIN;
              unit2.transferfin.to = unit.picfetch.from;
              unit2.transferfin.from = unit.picfetch.to;
              unit2.transferfin.id = unit.picfetch.pid;
              addUnit(unit2, client);
            }
          }

        }else{
          *out << VERBOSE_LOUD << "Invalid picture id in picfetch: " << unit.picfetch.pid << '\n';
        }
      }else{
        *out << VERBOSE_LOUD << "Invalid ID for from or to in picfetch, from: " << unit.picfetch.from
        << " to: " << unit.picfetch.to << '\n';
      }
      break;
    case UNIT_PICSELECT:
      if (SERV) {
        picc.setPicnum(unit.picselect.clientnum, unit.picselect.picnum, unit.picselect.direction); // so it can send out to new clients
        addUnitAll(unit, server, -1);
      }else{
        *out << VERBOSE_QUIET << "Calling picselect, clientnum: " << unit.picselect.clientnum << ", picnum: " << unit.picselect.picnum << '\n';
        picc.setPicnum(unit.picselect.clientnum, unit.picselect.picnum, unit.picselect.direction);
        *out << VERBOSE_QUIET << "Called picselect\n";
      }
      break;
    case UNIT_POSITION:
      if (unit.position.id > ID_PLAYER_MIN -1 && unit.position.id < players) {
        player[unit.position.id].setX(unit.position.x);
        player[unit.position.id].setY(unit.position.y);
        if (unit.position.x == 0) *out << VERBOSE_LOUD << "ZERRERRERREROOOO!!!!!" << '\n'; // TODO remove this
      }else if (unit.position.id > ID_BAD_MIN - 1 && unit.position.id < ID_BAD_MAX) {
        int badid = unit.position.id - ID_BAD_MIN;
        badc.getBaddies()[badid].setX(unit.position.x);
        badc.getBaddies()[badid].setY(unit.position.y);
      //}else if (unit.position.id > ID_PIC_MIN - 1 && unit.position.id < ID_PIC_MAX) {
        //int picid = unit.position.id - ID_PIC_MIN;
        //picc.getPics()[picid].setX(unit.position.x);
        //picc.getPics()[picid].setY(unit.position.y);
        //picc.getPics()[picid].setZ(unit.position.z);
      }else *out << VERBOSE_LOUD << "Invalid position ID: " << unit.position.id << '\n';
      //else if (unit.position.id > ID_BULLET_MIN - 1 && unit.position.id < ID_BULLET_MAX) {
        //int bullid = unit.position.id - ID_BULLET_MIN;
        //bullc.getBullets()[bullid].setX(unit.position.x);
        //bullc.getBullets()[bullid].setY(unit.position.y);
      //}
      break;
    case UNIT_POWERUP:
      // need to use same ID as server for collection purposes
      if (unit.powerup.id > -1 && unit.powerup.id < poweruplen) {
        if (unit.powerup.collected == -2) {
          // deactivated (out of screen)
          pc.getPowerups()[unit.powerup.id].setActive(false);
        }else if (unit.powerup.collected == -1) {
          // activated
          pc.getPowerups()[unit.powerup.id].setActive(true);
          pc.getPowerups()[unit.powerup.id].set(unit.powerup.x, unit.powerup.y, unit.powerup.speedY, (PowerupType) unit.powerup.type);
        }else if (unit.powerup.collected > -1 && unit.powerup.collected < players) {
          // TODO this could be like bullet (i.e. change collected to active, 0 or 1)
          // The only reason I can think of for storing the player that collected it is for a ghost (local copy) player
          // so I'm leaving this in.
          // (Remember at the minute the server stores everything about the player and the client just does what it's told)
          //player[unit.powerup.collected].collect(pc.getPowerups()[unit.powerup.id].getType());
          pc.getPowerups()[unit.powerup.id].setActive(false);
          // as long as powerups are called in this order: clear -> net.process -> draw; then it's ok to set inactive here
          // (server calls draw to show collisions, then checks collisions, so it has to make it dead first so it is cleared)
        }else *out << VERBOSE_LOUD << "Invalid value for powerup collected: " << unit.powerup.collected << '\n';
      }else *out << VERBOSE_LOUD << "Error with powerup data unit. Invalid ID: " << unit.powerup.id << '\n';
      break;
    case UNIT_TRANSFER:
      if (SERV) {
        addUnit(unit.transfer.to, unit, server);
      }else{
        // store chunk
        // *out << VERBOSE_LOUD << "Receiving chunk, amount: " << unit.transfer.amount << "...\n";
        if (!transferc.receive(unit)) { //, *this, client)) cb
          // send finished transfer, load picture
          Unit unit2;
          unit2.flag = UNIT_TRANSFERFIN;
          unit2.transferfin.to = unit.transfer.from;
          unit2.transferfin.from = unit.transfer.to;
          unit2.transferfin.id = unit.transfer.id;
          addUnit(unit2, client);

          picc.loadRemote(unit.transfer.from, unit.transfer.id, unit.transfer.filename);
        }
      }
      break;
    case UNIT_TRANSFERFIN:
      if (SERV) {
        addUnitAll(unit, server, unit.transferfin.from); // send out to all so they all addLoaded
      }else{
        if (unit.transferfin.to > -1 && unit.transferfin.to < MAX_CLIENTS) {
          if (unit.transferfin.id > -1 && unit.transferfin.id < picc.getPiclen(unit.transferfin.to)) {
            picc.getPics()[unit.transferfin.to][unit.transferfin.id].addLoaded();
          }else{
            *out << VERBOSE_LOUD << "TransferFIN: error with id: " << unit.transferfin.id << '\n';
          }
        }else{
          *out << VERBOSE_LOUD << "TransferFIN: error with to: " << unit.transferfin.to << '\n';
        }
      }
      break;
    case UNIT_TRANSFERREQ:
      if (SERV) {
        addUnit(unit.transferreq.to, unit, server);
        // *out << "Passing on transfer request...\n";
      }else{
        // received transfer request, send next chunk
        // *out << "Received transfer request...\n";
        transferc.send(unit.transferreq.to, unit.transferreq.from, unit.transferreq.id, unit.transferreq.filename, *this, client);
      }
      break;
    default:
      *out << VERBOSE_LOUD << "Error, flag not found: " << unit.flag << '\n';
  }
}*/

// only used by server, so put in server control
/*void Net::sendStatus(int cid, Badcontrol &badc, Bulletcontrol &bullc, Picturecontrol &picc, Server &server)
{
  Bad* bad = badc.getBaddies();

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

    addUnit(cid, unit, server);
  }

  unit.bullet.flag = UNIT_BULLET;
  unit.bullet.x = 0;
  unit.bullet.y = 0;
  unit.bullet.z = 0;
  unit.bullet.dirY = 0;
  unit.bullet.speedY = 0;
  unit.bullet.owner = 0;

  Bullet* bullet = bullc.getBullets();

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

    addUnit(cid, unit, server);
  }

  unit.flag = UNIT_PICALLOC;
  for (int i = 0; i < MAX_CLIENTS; i++) {
    unit.picalloc.id = i;
    unit.picalloc.total = picc.getPiclen(i);
    if (unit.picalloc.total > 0) addUnit(cid, unit, server);
  }

  unit.flag = UNIT_PICSELECT;
  unit.picselect.clientnum = picc.getClientnum();
  unit.picselect.picnum = picc.getPicnum();
  unit.picselect.direction = 1;
  addUnit(cid, unit, server);
}*/

