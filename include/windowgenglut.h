#ifndef WINDOWGEN_GLUT
#define WINDOWGEN_GLUT

#include "windowgen.h"

class WindowGlut : public WindowGen {
  public:
    bool init(Outverbose&, WindowInfo);
    void refresh();
    void destroy();

    static void reshape(int, int);
};

#endif
