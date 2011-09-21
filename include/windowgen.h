#ifndef WINDOWGEN
#define WINDOWGEN

#include <iostream>

struct WindowInfo {
  int x;
  int y;
  int width;
  int height;
  bool doubleBuffer;
  bool depthBuffer;
  int refreshRate;
  int colorDepth;
  bool fullscreen;
  std::string title;
};

class Outverbose;

class WindowGen {
  protected:
    int wid;
    Outverbose* out;
    WindowInfo winfo;

  public:
    WindowGen();
    virtual ~WindowGen();

    WindowInfo makeWindowInfo(int, int, int, int, bool, bool, int, int, bool, std::string);

    int getWid();
    int getWidth();
    int getHeight();
    void resize(int, int);

    void initShared(Outverbose&, WindowInfo);
    virtual bool init(Outverbose&, WindowInfo) = 0;
    virtual void refresh() = 0;
    virtual void destroy() = 0;
};
#endif
