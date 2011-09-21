#ifndef STREAM
#define STREAM

#include "sound.h"

//#include "network.h" // TODO why do I need network here??
#include "ringbuf.h"

class Client; class Net;

#define TALK_SWAP_MAX 5 // TODO rename to STREAM_SWAP_MAX??

class Stream : public Sound {
  private:
    Ringbuf streamBuf;
    ALuint streamSource, streamSwapBuf[TALK_SWAP_MAX]; // swap buffer - play one, queue other then swap
    int swap, swaps, minQueueSize; // minQueueSize removes popping effect, but adds delay

    bool initialisedStream;

    bool queue();
    int unqueuePlayedBuffers();
    bool stream(ALuint);

  protected:
    ALCenum format;                 // format of captured samples
    ALCuint freq;                   // total number of samples per second
    float chunkSeconds;             // length of capture chunks in seconds
    ALCint chunkSamples;           // number of samples per chunk
    ALCint chunkBytes;             // number of bytes in chunk
    ALCsizei internalSamples;       // size of internal OpenAL capture buffer

  public:
    Stream();
    ~Stream();

    void initStream();

    int getChunkBytes();
    int getChunksRecvd();

    void receive(const char*);

    bool play();
    bool update();

    bool playing();
};

#endif
