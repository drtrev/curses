#ifndef RINGBUF
#define RINGBUF

class Ringbuf {
  private:
    int BUFFER_SIZE;
    int length; // amount of data on buffer
    char *buffer;
    char *writePtr, *readPtr, *endPtr;
    bool allocated;

    int update(); // wrap around, and error check
    int read(char*, int, bool); // actually do the read

  public:
    Ringbuf();          // constructor
    Ringbuf(const int); // constructor with size allocation

    ~Ringbuf(); // destructor

    void allocate(const int); // allocate buffer memory

    int peek(char*, int); // read but don't move read cursor or change length
    int read(char*, int);
    int write(const char*, int);

    int getLength() const;

    int grabline(char*, int);

    void output();
};

#endif
