#ifndef WINDOWGEN_SDL
#define WINDOWGEN_SDL

#include "windowgen.h"

class WindowSDL : public WindowGen {
  public:
    bool init(Outverbose&, WindowInfo);
    void refresh();
    void destroy();
};

#endif
