#include "clientcontrol.h"
#include <getopt.h>
#include <iostream>
#include "servercontrol.h"

using std::cout;
using std::cerr;
using std::endl;
using std::string;

Curses *curses;

void catchInterrupt(int sig)
{
  curses->gameover();
  if (sig == SIGABRT) cout << "Program aborted." << endl;
  exit(0);
}

void processCLAs(int argc, char** argv, string &logfile, string &picpath, char* ip, verboseEnum &verbosity, bool &dontGrab, bool &fullscreen, bool &SERV)
{
  struct option options[] = {
    {"dontGrab", no_argument, 0, 'd'},
    {"fullscreen", no_argument, 0, 'f'},
    {"help", no_argument, 0, 'h'},
    {"ipaddress", required_argument, 0, 'i'},
    {"log", required_argument, 0, 'l'},
    {"path", required_argument, 0, 'p'}, // picture path
    {"quiet", no_argument, 0, 'q'},
    {"server", no_argument, 0, 's'},
    {"verbose", no_argument, 0, 'v'},
    {0, 0, 0, 0}
  };

  int optionIndex = 0;
  int c = 0;

  // returns EOF (-1) when reaches end of CLAs
  while ((c = getopt_long(argc, argv, "dfhi:l:p:qsv", options, &optionIndex)) != EOF) {
    switch (c) {
      case 'd': // don't grab keyboard
        dontGrab = true;
        break;
      case 'f':
        fullscreen = true;
        break;
      case 'h': // help
        cout << "Usage: ./net [options]" << endl;
        cout << "Options:" << endl;
        cout << "  -d  --dontGrab" << endl;
        cout << "    don't grab keyboard" << endl;
        cout << "  -f  --fullscreen" << endl;
        cout << "    open in fullscreen mode" << endl;
        cout << "  -h  --help" << endl;
        cout << "    display this help" << endl;
        cout << "  -l  --log" << endl;
        cout << "    logfile" << endl;
        exit(0);
        break;
      case 'i':
        if (strlen(optarg) + 1 > 50) {
          cerr << "IP address too long" << endl;
        }else{
          memcpy(ip, optarg, strlen(optarg) + 1);
        }
        break;
      case 'l':
        logfile = optarg;
        break;
      case 'p':
        picpath = optarg;
        break;
      case 'q':
        if (verbosity == VERBOSE_LOUD) verbosity = VERBOSE_NORMAL; // got v as well
        else verbosity = VERBOSE_QUIET;
        break;
      case 's':
        SERV = true; // can't use SERVER cos that's defined in server.h
        break;
      case 'v':
        if (verbosity == VERBOSE_QUIET) verbosity = VERBOSE_NORMAL; // got q as well
        else verbosity = VERBOSE_LOUD;
        break;
      case '?': // error
        cerr << "For usage instructions use './net -h' or './net --help'" << endl;
        break;
    }
  }

}

int main(int argc, char** argv)
{
  signal(SIGINT, &catchInterrupt);
  signal(SIGABRT, &catchInterrupt);

  string logfile = "";
  string picpath = "";
  char localhost[50], ip[50];
  string localstr = "127.0.0.1";
  strncpy(localhost, localstr.c_str(), localstr.length() + 1);
  ip[0] = '\0';
  verboseEnum verbosity = VERBOSE_NORMAL;
  bool dontGrab = false; // grab keyboard
  bool fullscreen = false;
  bool SERV = false;

  processCLAs(argc, argv, logfile, picpath, ip, verbosity, dontGrab, fullscreen, SERV);
  if (strlen(ip) == 0) memcpy(ip, localhost, strlen(localhost) + 1);

  if (!SERV) curses = new Clientcontrol;
  else curses = new Servercontrol;
  
  int port = 3496;

  curses->init(port, logfile, picpath, verbosity, ip, dontGrab, fullscreen);
  curses->go();
  curses->gameover();

  return 0;
}
