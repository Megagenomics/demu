#ifndef _READER_H
#define _READER_H

#include "buffer.h"

#include <sys/prctl.h>
#include <iostream>
#include <fstream>
#include <cstring>
#include <string>
#include <zlib.h>
#include <thread>
#include <mutex>
#include <map> 
#include <condition_variable>

using namespace std;

class Reader{
public:
  int mInputFlag=0;
  int mFileNum=0;
  int mThreads=0;
  int mReadNum4Buffer=0;
  int mLineLength=0;
  int mSampleSize=0;

  string *mInputFileName=nullptr;
  gzFile *mGzFile=nullptr;
  ifstream *mFile=nullptr;
  bool *mIsGz=nullptr;

  mutex *mtx_buf=nullptr;
  condition_variable *cv_buf=nullptr;

  int **mReadNum=nullptr;
  char **mReadRepository=nullptr;
  bool *mReadRepositoryFlag=nullptr;

  thread **mInputFileThread=nullptr;
  thread **mConsumerThread=nullptr;

  mutex mtx_input2input;
  condition_variable cv_input2input;

  int DFlag=0;
  int IFlag=0;
  int TFlag=-1;
  int mIsCompleted=0;

  map<string,Buffer> outer;

public:
  Reader();
  void init(int threads,int inputFlag,int readNum4Buffer,int lineLength,int mSampleSize,string *inputFileName);
  void garbageCollection();
  void startUpInputThread();
  void fillPack(int id);
  void joinInputThread();
  void setConsumer();
  void consumer(int id);
  void destroyConsumer();
  void flush();
};

#endif