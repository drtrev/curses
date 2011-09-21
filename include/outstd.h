#ifndef OUTSTD
#define OUTSTD

#include <fstream>

#define MAX_COLS 500 // There's 157 on my monitor, but allow more in case want to type more in!!
#define MAX_LINES 100 // 55 for me

class Out {
  private:
    bool initialised;
    int outputX, outputY;
    int inputX, inputY;
    int menuPos;
    int cursor;
    char outputLines[MAX_LINES][MAX_COLS];
    char inputLines[MAX_LINES][MAX_COLS];
    int outputLine, outputChar, inputLine, inputChar;

    int cols, lines;

    std::ofstream logfile;
    bool openedLog;

  public:
    Out(); // constructor
    ~Out(); // destructor

    void init();
    void endWin();

    int getWidth();
    int getHeight();

    void addCh(int, int, char);
    void add(const char*); // add message
    void add(std::string);
    void add(char);
    void add(int);
    void addln(int);
    void addln(const char*);
    void addln(std::string);

    //Out& operator<<(const char*);
    //Out& operator<<(const std::string&); // need string as well as char*

    void refreshScreen(); // note this is not needed if using getCh
    int getCh();
    char get();
    char getIn(int);
    void putIn(const char*); // put some stuff into the input stream

    void scrollInput();
    void scrollOutput();

    void setMenu(const char*);

    void setCursor(bool); // turn cursor on/off

    void openLog(const char*);
    void closeLog();
    bool getOpenedLog();
};

#endif
