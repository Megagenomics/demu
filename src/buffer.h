#ifndef _BUFFER_H
#define _BUFFER_H

#include <mutex>
#include <cstring>
#include "common.h"
#include "writer.h"

using namespace std;

class Buffer
{
public:
  mutex mtx;
  int mPaired=0;
  char *block1=nullptr;
  char *block2=nullptr;
  char *pos1=nullptr;
  char *pos2=nullptr;
  int freeSpace1=OUT_BUFFER_SIZE;
  int freeSpace2=OUT_BUFFER_SIZE;
  string mFullPathName1;
  string mFullPathName2;
  bool mIsCompressed=false;

public:
  Buffer();
  ~Buffer();
  Buffer(const Buffer &B);
  Buffer(int paired,const string &fullPathName1,const string &fullPathName2,bool isCompressed);
  Buffer& operator=(const Buffer &B);
  void storeStr(string str1,string str2);
  void flush();
};
#endif