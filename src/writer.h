#ifndef _WRITER_H
#define _WRITER_H

#include <iostream>
#include <fstream>
#include <zlib.h>

using std::string;
using std::ios;
using std::ofstream;

class Writer{
  
public:
	Writer(string *fullPathName=nullptr, char *text=nullptr, int size=0, bool isCompressed=false);
	~Writer();
  void write();

private:
	string *mFullPathName=nullptr;
  char *mText=nullptr;
  int mSize=0;
  bool mIsCompressed=false;
	
};

#endif