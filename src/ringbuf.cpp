#include "ringbuf.h"
#include <cstring>
#include <iostream>

using std::cout;
using std::cerr;
using std::endl;
using std::memcpy;

Ringbuf::Ringbuf()
{
  buffer = 0;
  length = 0;
  writePtr = 0;
  readPtr = 0;
  endPtr = 0;
  BUFFER_SIZE = 0;
  // user needs to call allocate to set things up
  allocated = false;
}

Ringbuf::Ringbuf(const int size)
{
  length = 0;
  BUFFER_SIZE = size;

  buffer = new char[BUFFER_SIZE];
  allocated = true;

  writePtr = buffer;
  readPtr = buffer;
  endPtr = buffer+BUFFER_SIZE;

  // for testing purposes
  /*for (int i = 0; i < BUFFER_SIZE; i++) {
    buffer[i] = 'a';
  }*/
}

Ringbuf::~Ringbuf()
{
  if (allocated) delete [] buffer;
}

void Ringbuf::allocate(const int size)
{
  BUFFER_SIZE = size;

  buffer = new char[BUFFER_SIZE];
  allocated = true;

  writePtr = buffer;
  readPtr = buffer;
  endPtr = buffer+BUFFER_SIZE;
}

int Ringbuf::update()
{
  // in case in future programs are allowed to mince around
  // with these pointers, then do the boundary check at the start
  if (writePtr < buffer) {
    cerr << "Error in Ringbuf::write(): writePtr < buffer" << endl;
    return -1;
  }
  
  if (readPtr < buffer) {
    cerr << "Error in Ringbuf, readPtr < buffer" << endl;
    return -1;
  }

  // wrap around
  if (writePtr > endPtr - 1) writePtr = buffer;
  if (readPtr > endPtr - 1) readPtr = buffer;

  // check length
  int l = 0;

  if (writePtr > readPtr) l = writePtr - readPtr;
  else if (writePtr < readPtr) {
    l = endPtr - readPtr;
    l += writePtr - buffer;
  }
  else if (writePtr == readPtr) {
    // length is zero or max
    if (length == BUFFER_SIZE) l = BUFFER_SIZE;
  }

  if (length != l) {
    cerr << "Error in Ringbuf, length incorrect. Correcting..." << endl;
    length = l;
    return -1;
  }

  return 0; // all good
}

int Ringbuf::peek(char* data, int amount)
{
  return read(data, amount, true);
}

int Ringbuf::read(char* data, int amount)
{
  return read(data, amount, false);
}

int Ringbuf::read(char* data, int amount, bool peek)
  // Reads amount characters from buffer into data
  // If peeking, don't change readPtr or length
  // Returns number of characters read
{
  if (update() == -1) return -1;

  // can't take more than total buffer size
  if (amount > BUFFER_SIZE) amount = BUFFER_SIZE;

  // can't take more than available
  if (amount > length) amount = length;

  if (amount > 0) {
    if (readPtr < writePtr || amount < endPtr - readPtr + 1) {
      // all in one line, no wrap around, just simply read it!
      memcpy(data, readPtr, amount);
      if (!peek) readPtr += amount;
    }else{
      // read till end
      int firstAmount = endPtr - readPtr;
      memcpy(data, readPtr, firstAmount);
      // then read the rest
      memcpy(&data[firstAmount], buffer, amount-firstAmount);
      if (!peek) readPtr = buffer + amount-firstAmount;
    }

    if (!peek) length -= amount;
  }else amount = 0; // could be negative

  return amount;
}

int Ringbuf::write(const char* data, int amount)
  // Write amount of data to buffer
  // Returns amount written or -1 on error
{
  if (update() == -1) return -1;

  // limit amount
  if (amount > BUFFER_SIZE) amount = BUFFER_SIZE;

  // make sure we can fit it on
  if (amount > BUFFER_SIZE - length) amount = BUFFER_SIZE - length;

  if (amount > 0) {

    if (writePtr + amount < endPtr + 1) {
      // if it fits, copy the lot
      memcpy(writePtr, data, amount);
      writePtr += amount;
    }else{
      // otherwise copy first bit till the end
      int firstAmount = endPtr - writePtr;
      memcpy(writePtr, data, firstAmount);
      // then copy the rest
      memcpy(buffer, &data[firstAmount], amount-firstAmount);
      writePtr = buffer + amount-firstAmount;
    }

    length += amount;
  }else amount = 0; // could be negative

  return amount; // amount written
}

int Ringbuf::getLength() const
{
  return length;
}

int Ringbuf::grabline(char* data, int max)
  // if max is not big enough then you may get no data
  // or only part of a line
{
  if (update() == -1) return -1;

  // can't take more than total buffer size
  if (max > BUFFER_SIZE) max = BUFFER_SIZE;
  
  int grabbed = 0;
  
  if (length > 0) {
    char* endLine;

    if (writePtr > readPtr) {
      // if writePtr doesn't wrap around, read up till writePtr
      endLine = (char*) memchr(readPtr, '\n', writePtr-readPtr);
    }else{
      // otherwise read up till the end of the buffer
      endLine = (char*) memchr(readPtr, '\n', endPtr-readPtr);
      if (endLine == NULL && writePtr > buffer) {
        // then if it wasn't found there, check from the start
        endLine = (char*) memchr(buffer, '\n', writePtr-buffer);
      }
    }

    if (endLine != NULL) {
      // if found, grab line

      if (endLine - readPtr >= 0) {
        // working without wrap around
        if (endLine - readPtr < max) {
          // if can fit it all on
          memcpy(data, readPtr, endLine-readPtr+1); // include newline char
          grabbed = endLine-readPtr+1;
          length -= grabbed;
          readPtr += grabbed;
        }
      }else{
        // read up till end
        if (endPtr - readPtr < max + 1) {
          // if can fit
          int firstAmount = endPtr - readPtr;
          memcpy(data, readPtr, firstAmount);
          grabbed = firstAmount;
          max -= firstAmount; // note this change of max
          length -= firstAmount;
          readPtr += firstAmount;

          if (endLine - buffer < max) {
            // copy from start
            memcpy(&data[firstAmount], buffer, endLine-buffer+1); // include newline char
            grabbed += endLine-buffer+1;
            length -= endLine-buffer+1;
            readPtr = endLine + 1;
          }
        }
      }
    } // end if found
  } // end if length > 0

  return grabbed;
}

void Ringbuf::output()
{
  for (int i = 0; i < readPtr - buffer; i++) cout << " ";
  cout << "^" << endl;

  for (int i = 0; i < BUFFER_SIZE; i++) {
    cout << buffer[i];
  }
  cout << endl;
  for (int i = 0; i < writePtr - buffer; i++) cout << " ";
  cout << "^" << endl;
}

