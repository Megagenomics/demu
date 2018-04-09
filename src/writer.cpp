#include "writer.h"

Writer::Writer(string *fullPathName, char *text, int size, bool isCompressed)
{
  mFullPathName=fullPathName;
  mText=text;
  mIsCompressed=isCompressed;
  mSize=size;
}

Writer::~Writer(){}

void Writer::write(){  
  if(mIsCompressed){
    gzFile mZipFile= gzopen(mFullPathName->c_str(), "w+");
    gzwrite(mZipFile, mText, mSize);
    gzflush(mZipFile,Z_FINISH);
    gzclose(mZipFile);
  }else{
    ofstream mFile;
    mFile.open(mFullPathName->c_str(),  ios::out | ios::app);
    mFile<<mText;
    //mFile.write(mText, mSize);
    mFile.flush();
    mFile.close();
  }
}
