#ifndef INPUT
#define INPUT

#define KEYS_UP       0x0001
#define KEYS_LEFT     0x0002
#define KEYS_DOWN     0x0004
#define KEYS_RIGHT    0x0008
#define KEYS_FIRE     0x0010
#define KEYS_QUIT     0x0020
#define KEYS_TALK     0x0040
#define KEYS_NEXT     0x0080
#define KEYS_BACK     0x0100
#define KEYS_ZOOM_IN  0x0200
#define KEYS_ZOOM_OUT 0x0400

class Input {
  protected:
    bool grabbed;

  public:
    Input();

    bool getGrabbed();

    //virtual int check(int) = 0;
};

#endif
