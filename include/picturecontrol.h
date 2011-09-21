#ifndef PICTURECONTROL
#define PICTURECONTROL

#include "directory.h"
#include "network/network.h"
#include <string>
#include "textures.h"
#include <vector>

class Client; class Graphics; class Net; class Outverbose; class Picture; class Server;

class Picturecontrol {
  private:
    Picture *pic[MAX_CLIENTS];

    int piclen[MAX_CLIENTS], picnum, clientnum;

    //bool allocated;

    Graphics* graphics;
    Outverbose* out;
    std::string readPath;
    std::string writePath;
    std::vector <std::string> files;
    std::vector < std::vector<int> > transmit;
    std::vector <FILE*> transmitFile;
    Textures textures;

    Directory dir;

  public:
    Picturecontrol();
    ~Picturecontrol();

    void init(Outverbose&, Graphics&);
    void setReadPath(std::string);
    void setWritePath(std::string);
    void load(int, Net&, Client&);
    void loadRemote(int, int, char*);
    
    int getTotal();
    int getPiclen(int);
    Picture** getPics();

    //int getWidth(int);
    //int getHeight(int);
    //int imageLoad(char*, Image2*);
    void draw(int);

    int getClientnum();
    int getPicnum();

    void setPicnum(int, int, int);
    void allocate(int, int);
    void fetch(int, Net&, Client&);

    void setPicholder(int);

    void sendFilename(int, int, int, Net&, Client&);
    // server receives in net.cpp and passes on

    void incrementLoaded(int, int);

    void input(int, Net&, Server&);
    void local(double);
    void go(double);


};

#endif
