#include "transfer/transfercontrol.h"
#include <cstring>
#include "outverbose.h"

void Transfercontrol::init(Outverbose &o)
{
  out = &o;
}

void Transfercontrol::setReadPath(std::string s)
{
  if (s.size() > 0 && s[s.size()-1] != '/') s += '/';
  readPath = s;
}

void Transfercontrol::setWritePath(std::string s)
{
  if (s.size() > 0 && s[s.size()-1] != '/') s += '/';
  writePath = s;
}

bool Transfercontrol::checkFilename(std::string s)
{
  if (s.find("/") != std::string::npos) {
    *out << VERBOSE_LOUD << "Error, filename contains directory: " << s << '\n';
    return false;
  }
  return true;
}

bool Transfercontrol::start(int source, int dest, int id, char* filename, Net &net, Client &client)
  // initiate file transfer from source to dest
  // the caller should give this transaction an id that is unique to this source
{
  if (std::strlen(filename) < MAX_FILENAME_SIZE) {
    *out << VERBOSE_LOUD << "Starting transfer...\n";

    bool found = false;

    for (int i = 0; i < (int) transferrecv.size(); i++) {
      if (transferrecv[i].getSource() == source && transferrecv[i].getId() == id) {
        // needs a source, id pair to be unique
        found = true;
        break;
      }
    }

    if (!found) {
      if (checkFilename(filename)) {
        transferrecv.push_back(Transferrecv(source, dest, id, writePath.c_str(), filename));
        if (!transferrecv.back().getActive()) return false; // already got file
      }else *out << VERBOSE_LOUD << "Error with filename, failed the check, transaction ignored\n";
    }else{
      *out << VERBOSE_LOUD << "Error, transfer already being processed\n";
    }
    *out << VERBOSE_LOUD << "Started transfer\n";

  }else{
    *out << VERBOSE_LOUD << "Error with transfer start, filename too long\n";
  }

  return true; // started - only return false if file is already there, indicating need to loadRemote
}

void Transfercontrol::send(int source, int dest, int id, char* filename, Net &net, Client &client)
{
  bool found = false;

  for (int i = 0; i < (int) transfersend.size(); i++) {
    // see if we're already sending this file to this client
    if (transfersend[i].getDest() == dest && !strcmp(transfersend[i].getFilename(), filename) && transfersend[i].getId() == id) {
      //*out << VERBOSE_LOUD << "Found transaction, calling send...\n";
      transfersend[i].send(net, client);
      found = true;
      break;
    }else{
      *out << VERBOSE_LOUD << "not found: dest, filename, id compared: " << dest << ", " << filename << ", " << id <<
      " -- " << transfersend[i].getDest() << ", " << transfersend[i].getFilename() << ", " << transfersend[i].getId() << '\n';
    }
  }

  if (!found) {
    // add transaction here. the check below should be fine now.
    transfersend.push_back(Transfersend(source, dest, id, readPath.c_str(), filename));
    *out << VERBOSE_LOUD << "Created transaction: source, dest, id, path, filename: "
    << source << ", " << dest << ", " << id << ", " << readPath << ", " << filename << ". calling send...\n";
    transfersend.back().send(net, client);
  }

  // this check should be fine, because if a transaction was not found (e.g. no
  // elements in transfersend) then it would have been added above
  if (!transfersend[0].getActive()) {
    *out << VERBOSE_LOUD << "Removing transfer send transaction\n";
    transfersend.pop_front();
    if ((int) transfersend.size() == 0) *out << "Transfer send empty!\n";
  }
}

bool Transfercontrol::receive(Unit unit) //, Net &net, Client &client)
{
  for (int i = 0; i < (int) transferrecv.size(); i++) {
    if (transferrecv[i].getId() == unit.transfer.id) {
      //*out << VERBOSE_LOUD << "found transaction, calling transfer receive...\n";
      transferrecv[i].receive(unit.transfer.data, unit.transfer.amount);
      if (transferrecv[i].getActive()) {
        // could set something here, e.g. to say that it's "ready" for next request,
        // but this could also slow things down (e.g. waiting for it to be ready, requesting others in the meantime,
        // ending up downloading everything at once)
        transferrecv[i].setReady(true);
        return true; // continue
      }else{
        return false; // finished
      }
    }
  }

  *out << VERBOSE_LOUD << "receive error, can't find transaction with id " << unit.transfer.id << '\n';
  return false;
}

void Transfercontrol::go(Net &net, Client &client)
{
  bool transferring = false;

  // it will only be active if it's still got stuff to transfer,
  // and only started if nothing else was transferring.

  for (int i = 0; i < (int) transferrecv.size(); i++) {
    if (transferrecv[i].getActive() && transferrecv[i].getStarted()) {
      transferring = true; // got something to transfer, just check it's ready
      if (transferrecv[i].getReady()) {
        transferrecv[i].request(net, client); // request next piece of data
        transferrecv[i].setReady(false); // set to ready again when receive something, see above
      }
      break;
    }
  }

  // remove dead ones if poss
  if ((int) transferrecv.size() > 0 && !transferrecv[0].getActive()) {
    *out << VERBOSE_LOUD << "Removing transfer receive transaction\n";
    transferrecv.pop_front();
    if ((int) transferrecv.size() == 0) *out << "Transfer recv empty!\n";
  }

  // note element zero may have changed here, so
  // could be unactive, because only first unactive is popped above
  if ((int) transferrecv.size() > 0 && !transferring && transferrecv[0].getActive()) {
    transferrecv[0].request(net, client); // start it off
    transferrecv[0].setStarted(true);
  }
}

