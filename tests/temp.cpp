#include <arpa/inet.h>
#include <iostream>
#include <fstream>

using namespace std;

void outputIntBytes(char* data)
{
  //cout << (unsigned int) data[0] << "," << (unsigned int) data[1] << "," << (unsigned int) data[2] << "," << (unsigned int) data[3] << endl;
  cout << ntohl(*(unsigned int*) &data[0]) << endl;
}

void outputIntHexBigendian(char* data)
{
  char temp[4] = { data[3], data[2], data[1], data[0]};
  cout << hex << *(int*) temp << dec << endl;
}

void outputRMF(char* data)
{
  cout << "FourCC: " << data[0] << data[1] << data[2] << data[3] << endl;

  cout << "chunk size: ";
  outputIntBytes(&data[4]);
  cout << "chunk size (hex): ";
  outputIntHexBigendian(&data[4]);

  // 8, 9 are zero
  cout << "version (zero): " << (int) data[8] << "," << (int) data[9] << endl;

  cout << "file version: ";
  outputIntBytes(&data[10]);
  cout << "no. headers: ";
  outputIntBytes(&data[14]);
}

void outputPROP(char* data)
{
  cout << "FourCC: " << data[0] << data[1] << data[2] << data[3] << endl;

  cout << "chunk size: ";
  outputIntBytes(&data[4]);
  cout << "chunk size (hex): ";
  outputIntHexBigendian(&data[4]);

  // 8, 9 are zero
  cout << "version (zero): " << (int) data[8] << "," << (int) data[9] << endl;
  cout << "Maximum bit rate: ";
  outputIntBytes(&data[10]);
  cout << "Average bit rate: ";
  outputIntBytes(&data[14]);
  cout << "Size of largest data packet: ";
  outputIntBytes(&data[18]);
  cout << "Average size of data packet: ";
  outputIntBytes(&data[22]);
  cout << "Number of data packets in the file: ";
  outputIntBytes(&data[26]);
  cout << "File duration in ms: ";
  outputIntBytes(&data[30]);
  cout << "Suggested number of ms to buffer before starting playback: ";
  outputIntBytes(&data[34]);
  cout << "Offset of the first INDX chunk form the start of the file: ";
  outputIntBytes(&data[38]);
  cout << "Offset of the first DATA chunk form the start of the file: ";
  outputIntBytes(&data[42]);
  cout << "Number of streams in the file: " << (int) data[43] << "," << (int) data[44] << endl;
  cout << "Flags: ";

  for (int i = 7; i >=0; i--) {
    cout << ((((int) data[45]) & 1 << i) >> i);
  }
  cout << ",";
  for (int i = 7; i >=0; i--) {
    cout << ((((int) data[46]) & 1 << i) >> i);
  }
  cout << endl;
  cout << "Int of flags: " << ntohs(*(unsigned short int*) &data[45]) << endl;
  cout << "Hex of flags: " << hex << (int) data[45] << dec << endl;
}

void outputMDPR(char* data)
{
  cout << "FourCC: " << data[0] << data[1] << data[2] << data[3] << endl;
  cout << "chunk size: ";
  outputIntBytes(&data[4]);
  cout << "chunk size (hex): ";
  outputIntHexBigendian(&data[4]);

  cout << "version: " << ntohs(*(unsigned short int*) &data[8]) << endl;
  cout << "stream number: " << ntohs(*(unsigned short int*) &data[10]) << endl;
  cout << "Maximum bit rate: ";
  outputIntBytes(&data[12]);
  cout << "Average bit rate: ";
  outputIntBytes(&data[16]);
  cout << "Size of largest data packet: ";
  outputIntBytes(&data[20]);
  cout << "Average size of data packet: ";
  outputIntBytes(&data[24]);
  cout << "Stream start offset in ms: ";
  outputIntBytes(&data[28]);
  cout << "Preroll in ms: ";
  outputIntBytes(&data[32]);
  cout << "Stream duration in ms: ";
  outputIntBytes(&data[36]);
  
  int size = data[40];
  cout << "Size of stream description string: " << size << endl;
  if (size < 100) {
    char* desc = new char[size+1];
    memcpy(desc, &data[41], size);
    desc[size] = '\0';
    cout << "description: " << desc << endl;
    delete [] desc;
  }else{
    cerr << "Stream description may be a bug? Or is it just really long? not taking the risk, exiting..." << endl;
    exit(1);
  }

  // hard code for now
  size = data[86];
  cout << "Size of mime type string: " << size << endl;

  if (size < 100) {
    char* desc = new char[size+1];
    memcpy(desc, &data[87], size);
    desc[size] = '\0';
    cout << "mime: " << desc << endl;
    delete [] desc;
  }else{
    cerr << "Stream mime may be a bug? Or is it just really long? not taking the risk, exiting..." << endl;
    exit(1);
  }

  size = ntohl(*(unsigned int*) &data[107]);
  cout << "Size of type specific part of the header: " << size << endl;

  if (size < 100) {
    char* desc = new char[size+1];
    memcpy(desc, &data[108], size);
    desc[size] = '\0';
    cout << "type specific: " << desc << endl;
    delete [] desc;
  }else{
    cerr << "Type specific may be a bug? Or is it just really long? not taking the risk, exiting..." << endl;
    exit(1);
  }

}

void outputCont(char* data)
{
  cout << "FourCC: " << data[0] << data[1] << data[2] << data[3] << endl;
  cout << "chunk size: ";
  int size = ntohl(*(unsigned int*) &data[4]);
  outputIntBytes(&data[4]);
  cout << "chunk size (hex): ";
  outputIntHexBigendian(&data[4]);

  for (int i = 0; i < size; i++) {
    if ((int) data[i] > 32 && (int) data[i] < 127) cout << data[i];
  }
  cout << endl;
}

int outputDataPacket(char* data, int packNum)
{
  int offset = 0;
  int version = ntohs(*(unsigned short int*) &data[0]);
  int size = ntohs(*(unsigned short int*) &data[2]);
  cout << "packet version: " << version << endl;
  if (size != 756) cout << "SIZE: " << size << endl; // THIS IS INCLUDING THE HEADER, i.e. size of whole packet!
  cout << "stream number: " << ntohs(*(unsigned short int*) &data[4]) << endl;
  cout << "Timestamp in ms: ";
  outputIntBytes(&data[6]);
  cout << "Unknown: ";
  if (data[10] > 32 && data[10] < 127) cout << data[10];
  cout << endl;

  cout << "Flags (pack no. " << packNum << "): ";
  for (int i = 7; i >=0; i--) {
    cout << ((((int) data[11]) & 1 << i) >> i);
  }
  cout << endl;
  
  offset = 12;

  if (version == 1) offset++;

  for (int i = offset; i < size; i++) {
    if ((int) data[i] > 32 && (int) data[i] < 127) cout << data[i];
  }

  cout << endl;

  offset = size;

  return offset;
}

int outputData(char* data)
{
  cout << "FourCC: " << data[0] << data[1] << data[2] << data[3] << endl;

  cout << "chunk size: ";
  outputIntBytes(&data[4]);
  cout << "chunk size (hex): ";
  outputIntHexBigendian(&data[4]);

  cout << "version: " << ntohs(*(unsigned short int*) &data[8]) << endl;

  cout << "number of packets: ";
  int numPacks = ntohl(*(unsigned int*) &data[10]);
  //outputIntBytes(&data[10]);
  cout << numPacks << endl;

  cout << "offset of next data chunk: ";
  outputIntBytes(&data[14]);

  int offset = 0;
  cout << endl;
  for (int i = 0; i < 201; i++) {
    cout << "Packet: " << i << ", offset: " << offset << " (not generated)" << endl;
    offset += outputDataPacket(&data[18+offset], i);
    cout << endl;
  }
  //checked cout << "offset before: " << offset << endl;

  offset = 18 + numPacks * 756; // 18 is size of DATA and 756 is packet size
  int size = ntohl(*(unsigned int*) &data[4]);
  cout << "Size: " << size << ", offset: " << offset << endl;

  return offset;
}

int checkHeader(char* data)
{
  char fourcc[5];
  memcpy(fourcc, data, 4);
  fourcc[4] = '\0';

  cout << endl << "checkHeader: " << fourcc << endl;
  cout << "int: " << (int) fourcc[0] << "," << (int) fourcc[1] << "," << (int) fourcc[2] << "," << (int) fourcc[3] << endl;
  for (int i = 0; i < 4; i++) {
    switch (fourcc[i]) {
      case 1:
        cout << "start of heading";
        break;
      case 0:
        cout << "NULL";
        break;
      case 17:
        cout << "device control 1";
        break;
      case 18:
        cout << "device control 2";
        break;
    }
    if (fourcc[i] > 32 && fourcc[i] < 127) cout << fourcc[i];
    cout << ",";
  }
  cout << endl;

  int offset = 0;

  if (!strcmp(fourcc, ".RMF")) outputRMF(data);
  if (!strcmp(fourcc, "PROP")) outputPROP(data);
  if (!strcmp(fourcc, "MDPR")) outputMDPR(data);
  if (!strcmp(fourcc, "CONT")) outputCont(data);
  if (!strcmp(fourcc, "DATA")) offset = outputData(data);

  return offset;
}

int main(int argc, char** argv)
{
  ifstream in("pappysfunclub.ra", ios::in | ios::binary | ios::ate);

  int size = in.tellg();

  in.seekg(0, ios::beg);

  char* data = new char[size];

  in.read(data, size);

  in.close();

  int offset = 0;

  checkHeader(data);
  checkHeader(&data[18]);
  checkHeader(&data[65]);
  checkHeader(&data[68]);
  checkHeader(&data[149]);
  checkHeader(&data[343]); // not sure what this is for
  offset = checkHeader(&data[346]); // from PROP offset of first DATA chunk

  offset += 346;
  cout << "offset: " << offset << endl;
  cout << "size: " << size << endl;

  checkHeader(&data[offset]);

  cout << endl;
  for (int i = offset - 400; i < size; i++) {
    //if ((int) data[i] > 32 && (int) data[i] < 127) cout << i << ": " << data[i] << endl;
    if ((int) data[i] > 32 && (int) data[i] < 127) cout << data[i];
    //if ((int) data[i] == 0 && data[i+1] == 0) cout << "i: " << i << " is double zero" << endl;
    //if (data[i] == '7') cout << "that last one was i: " << i << endl;
  }
  cout << endl;

  ofstream out("/tmp/temp.ra", ios::out | ios::binary);

  //out.write(data, offset);

  char indexHead[20];
  indexHead[0] = 'I', indexHead[1] = 'N', indexHead[2] = 'D', indexHead[3] = 'X';
  int temp = htonl(34);
  memcpy(&indexHead[4], &temp, 4);
  indexHead[8] = 0; // version
  indexHead[9] = 0;
  temp = htonl(1); // entries
  cout << "1 converted: " << temp << endl;
  memcpy(&indexHead[10], &temp, 4);
  indexHead[14] = 0; // stream num
  indexHead[15] = 0;
  temp = 0;
  memcpy(&indexHead[16], &temp, 4);

  /*dword   Chunk type ('INDX')
dword   Chunk size
word    Chunk version (always 0, for every known file)
dword   Number of entries in this chunk
word    Stream number
dword   Offset of the next INDX chunk (form the start of the file)
byte[]  Index entries*/

  char indexEntry[14];
  indexEntry[0] = 0;
  indexEntry[1] = 0;
  temp = htonl(18559); // time
  memcpy(&indexEntry[2], &temp, 4);
  temp = htonl(151200); // pack offset
  memcpy(&indexEntry[6], &temp, 4);
  temp = htonl(200); // pack num
  memcpy(&indexEntry[10], &temp, 4);

  /*word   Entry version (always 0, for every known file)
 dword  Timestamp (in ms)
 dword  Packet offset in file (form the start of the file)
 dword  Packet number*/

  out.write(data, size);
  out.write(indexHead, 20);
  out.write(indexEntry, 14);

  //out.write(data, size-offset);

  out.close();

  delete [] data;
  return 0;
}

