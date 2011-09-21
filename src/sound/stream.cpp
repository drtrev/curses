#include "sound/talk.h"
#include <cstdlib>
#include "network/net.h"
#include "outverbose.h"

//#define NDEBUG
#include <cassert>

Stream::Stream()
{
  format = AL_FORMAT_MONO16;                      // format of captured samples
  int formatBytes = 2;                            // size of sample in bytes (e.g. mono16 is 2 bytes)
  freq = 20000;                                   // total number of samples per second
  chunkSeconds = 0.1;                             // length of capture chunks in seconds
  chunkSamples = (ALCint) (freq * chunkSeconds);  // number of samples per chunk
  chunkBytes = chunkSamples * formatBytes;        // number of bytes in chunk
  internalSamples = chunkSamples * 2;             // size of internal OpenAL capture buffer
                                                  // (must be big enough to keep capturing during chunk processing)

  swap = 0; // the next swap buffer to be filled and queued

  swaps = TALK_SWAP_MAX; // number of swap buffers
  minQueueSize = 2; // this removes popping effect but adds delay

  assert(swaps < TALK_SWAP_MAX + 1);
  assert(minQueueSize < swaps);
  assert(chunkBytes == 4000); // this is fixed in network.h atm
  // NOTE if make it so user can change these values at runtime then these assertions must still stand

  streamBuf.allocate(freq*formatBytes*2); // buffer 2 secs of audio

  //unit.audio.flag = UNIT_AUDIO; // TODO remove this (make local to capture)
  //unit.audio.id = -1;
  ////unit.audio.data = new char[chunkBytes];

  initialisedStream = false;

  //*out << VERBOSE_LOUD << "Chunk samples: " << ((int) chunkSamples) << '\n';
  //out->adderrln((int) chunkSamples, VERBOSE_LOUD);
  //std::cerr << "Chunk samps: " << chunkSamples << std::endl;
}

Stream::~Stream()
{
  //delete [] unit.audio.data; TODO

  if (initialisedStream) {
    // according to the 1.1 spec, 'A playing source can be deleted – the source
    // will be stopped automatically and then deleted.'
    // However, this does not work. Even though there is a context, the error
    // returned is AL_INVALID_OPERATION. Stopping the source first solves this problem.
    // Possibly because this isn't a normal source (AL_STATIC), it's streaming.
    // (see ogg.cpp)
    alSourceStop(streamSource); // this will stop the source or be a legal NOP
    alDeleteSources(1, &streamSource);
    alDeleteBuffers(swaps, streamSwapBuf);
    // openAL spec says a buffer which is attached to a source cannot be deleted,
    // so hopefully deleting the source will be enough to 'detach' the buffers
  }
}

void Stream::initStream()
{
  // this didn't work in constructor, came up with INVALID_NAME all the time (presume need to Sound::grab first)
  alGenSources(1, &streamSource);
  check("Gen source");
  alGenBuffers(swaps, streamSwapBuf);
  check("Gen buffers");
  initialisedStream = true;
}

int Stream::getChunkBytes()
{
  return chunkBytes;
}

int Stream::getChunksRecvd()
{
  return (streamBuf.getLength() / chunkBytes);
}

void Stream::receive(const char* chunk)
{
  if (streamBuf.write(chunk, chunkBytes) != chunkBytes) {
    *out << VERBOSE_LOUD << "Dropping some audio stream data, ran out of buffer space.\n";
  }
}

bool Stream::playing()
{
  ALint state = 0;
  alGetSourcei(streamSource, AL_SOURCE_STATE, &state);
  check("Get state");

  return (state == AL_PLAYING);
}

bool Stream::queue()
  // attempt to queue next part of stream
  // returns true on success,
  // or false if there is no more stream to queue (i.e. if stream returns false)
{
  if (swap < 0 || swap > swaps - 1) { // boundary check
    *out << VERBOSE_LOUD << "Error in update: swap out of range: " << swap << '\n';
    swap = 0;
  }

  bool gotChunk = stream(streamSwapBuf[swap]);

  if (gotChunk) {
    alSourceQueueBuffers(streamSource, 1, &streamSwapBuf[swap]);
    check("Queue buffer for update");

    swap++;
    if (swap > swaps - 1) swap = 0;

    //somethingGotBuffered = true;
    return true;
  }else
    return false; // nothing more to stream yet

}

bool Stream::update()
  // called every frame, keeps things going
{
  // I used to have two different functions, update() to refill played buffers,
  // and play() to establish a queue to start things off. Trouble arose when
  // there was either not enough stream for play (coming over network not from
  // a file) or when there was not enough stream for a refill in update (so
  // queue dwindled). Now update deals with filling up until above
  // minQueueSize, starting playback, refilling played buffers, and attempting
  // to keep queue topped up if it dwindles

  // Part 1: if number of queued buffers is not more than minQueueSize then refill some
  // if it can. (i.e. if minQueueSize is two, then it will not drop below two,
  // so mostly it will be three, until one is played, then the unqueued will be refilled.)
  // Part 2: decide if it's active or finished. If something was queued and it's not playing and
  // enough is on the queue then start playing. If there's some processed, or it's still playing,
  // or there's some just been queued then it's active. If not then there's either nothing received
  // yet, or there hasn't been for a minQueueSize length of time.

  // In short: Just unqueue what's played, and try to keep queue topped up.
  // play only when it's above minQueueSize.

  bool queued = false; // set to true when we've queued something new

  unqueuePlayedBuffers();

  // 1) How many more does it want to queue if it can?
  int numQueued = 0, want = 0;

  alGetSourcei(streamSource, AL_BUFFERS_QUEUED, &numQueued);
  check("Get queued");

  want = minQueueSize - numQueued + 1; // want one more than minQueueSize

  // minQueueSize must be less than swaps
  if (want < swaps + 1) {
    for (int i = 0; i < want; i++) {
      if (queue()) queued = true;
      else break; // nothing left for now
    }
  }else *out << VERBOSE_LOUD << "Error, want too big!\n";

  // 2) check it's still playing
  // if it's not, and nothing's been queued, and there's
  // no buffers just been processed (see below),
  // then this is no longer active
  bool stillPlaying = playing();

  // I suppose it's possible that after checking for unqueued buffers and before checking that
  // playback is still happening the buffers could be played and playback would stop.
  // If this is the case (i.e. there's some processed buffers left) then return true:
  // Next time these will be unqueued, and playback will be started again if more is added.
  int processed = 0;
  alGetSourcei(streamSource, AL_BUFFERS_PROCESSED, &processed);
  check("Get processed");

  if (queued || stillPlaying || processed) {
    if (!stillPlaying) {
      unqueuePlayedBuffers();
      int queued = 0;
      alGetSourcei(streamSource, AL_BUFFERS_QUEUED, &queued);
      check("Get queued");

      if (queued > minQueueSize) {
        alSourcePlay(streamSource);
        check("Update Play");
      }
    }
    return true; // still active
  }else{
    return false;
  }
}

bool Stream::stream(ALuint buffer)
  // returns true if next bit of stream has been added to buffer,
  // or false if there's not enough for next chunk
{
  bool added = false;

  if (streamBuf.getLength() > chunkBytes - 1) {
    // got chunk

    char *data = new char[chunkBytes];
    int read = streamBuf.read(data, chunkBytes);
    
    if (read == chunkBytes) {

      alBufferData(buffer, format, data, chunkBytes, freq);
      check("Copy data to an AL buffer");

      added = true;

    }else{

      *out << VERBOSE_LOUD << "Error reading bytes off stream!\n";

    }

    delete [] data;
  }

  return added;
}

int Stream::unqueuePlayedBuffers()
{
  int processed = 0;

  alGetSourcei(streamSource, AL_BUFFERS_PROCESSED, &processed);
  check("Get buffers processed");

  ALuint buffer[TALK_SWAP_MAX];

  if (processed > TALK_SWAP_MAX - 1) {
    *out << VERBOSE_LOUD << "Error, processed buffers is more than what should have been queued!\n";
    processed = TALK_SWAP_MAX - 1;
  }

  alSourceUnqueueBuffers(streamSource, processed, buffer);
  check("Unqueue played buffers");

  return processed;
}

