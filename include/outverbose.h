#ifndef OUTVERBOSE
#define OUTVERBOSE

#include "out.h"

enum verboseEnum { VERBOSE_QUIET, VERBOSE_NORMAL, VERBOSE_LOUD };

// always use error stream for operator overloads
//enum streamEnum { STREAM_STDOUT, STREAM_STDERR };

class Outverbose : public Out {
  private:
    verboseEnum verbose, outVerbosity;
    //streamEnum err;

    bool checkVerbosity(verboseEnum);

  public:
    Outverbose(); // constructor
    ~Outverbose(); // destructor

    void setVerbosity(verboseEnum);

    void outputVerbosity(verboseEnum);

    void add(const char*, verboseEnum);
    void add(std::string, verboseEnum);
    void add(char, verboseEnum);
    void add(int, verboseEnum);
    void addln(int, verboseEnum);
    void addln(const char*, verboseEnum);
    void addln(std::string, verboseEnum);

    void adderr(const char*, verboseEnum); // add also to stderr
    void adderr(std::string, verboseEnum);
    void adderr(char, verboseEnum);
    void adderr(int, verboseEnum);
    void adderrln(int, verboseEnum);
    void adderrln(const char*, verboseEnum);
    void adderrln(std::string, verboseEnum);

    Outverbose& operator<<(const verboseEnum);
    Outverbose& operator<<(const char*);
    Outverbose& operator<<(const std::string&);
    Outverbose& operator<<(const char);
    Outverbose& operator<<(const int);

    using Out::addln; // make parent functions available here, since they've been overloaded
    using Out::add;
};

#endif
