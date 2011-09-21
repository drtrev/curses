#ifndef NETWORK
#define NETWORK

enum Flag { UNIT_ASSIGN, UNIT_ATTACK, UNIT_AUDIO, UNIT_BAD, UNIT_BULLET, UNIT_KEYS, UNIT_LOGOFF, UNIT_MAP, UNIT_NEWCLIENT, UNIT_PLAYER, UNIT_PICALLOC, UNIT_PICFETCH, UNIT_PICSELECT, UNIT_POSITION, UNIT_POWERUP, UNIT_TRANSFER, UNIT_TRANSFERFIN, UNIT_TRANSFERREQ }; // change line below when editing this
#define UNITS 18 // don't forget to change this when adding or removing units!

#define TRANSFER_DATA_SIZE 4000
#define MAX_FILENAME_SIZE 100

// Following the principle of least privelege, flagsize and unitsize are
// initialised in the main program and passed only to objects that need them

union Unit {

  int flag; // always have a flag, -1 is no unit received

  struct {
    int flag;
    int id; // for assigning id
  } assign;

  struct {
    int flag;
    int wave;
  } attack;

  struct {
    int flag;
    int id;
    char data[4000];
  } audio;

  struct {
    int flag;
    int id;
    int type;
    int status; // 0 inactive, 1 active, 2 kill
  } bad;

  struct {
    int flag;
    int id;
    int active;
    float x;
    float y;
    float z;
    int dirY;
    float speedY;
    int owner;
  } bullet;

  struct {
    int flag;
    int id;
    int bits;
  } keys;

  struct {
    int flag;
    int id;
  } logoff;

  struct {
    int flag;
    float x;
    float y;
    float zoom;
  } map;

  struct {
    int flag;
    int id;
  } newclient;

  struct {
    int flag;
    int id;
    int status; // 0 = kill, 1 = respawn
  } player;

  struct {
    int flag;
    int id;
    int total;
  } picalloc; // allocate pictures to id

  struct {
    int flag;
    int from; // ID of sender
    int to; // destination
    int pid; // picture ID
    char filename[MAX_FILENAME_SIZE]; // filename of picture required for transfer
  } picfetch;

  struct {
    int flag;
    int clientnum;
    int picnum;
    int direction; // 1 = clockwise, 0 = anticlockwise
  } picselect; // specify which picture is selected, using client id and picture id

  struct {
    int flag;
    int id;
    float x;
    float y;
    float z;
  } position;

  struct {
    int flag;
    int id;
    float x;
    float y;
    float z;
    float speedY;
    int type;
    int collected; // id of who collected it or -1 if activated, -2 if destroyed (e.g. out of screen)
  } powerup;

  struct {
    int flag;
    int to;
    int from;
    int id;
    char filename[MAX_FILENAME_SIZE];
    char data[TRANSFER_DATA_SIZE];
    int amount;
  } transfer;

  struct {
    int flag;
    int to;
    int from;
    int id;
  } transferfin; // signal to sender that we've received last bit of transfer

  struct {
    int flag;
    int to;
    int from;
    int id;
    char filename[MAX_FILENAME_SIZE];
  } transferreq;

};

#define MAX_CLIENTS 10

#endif
