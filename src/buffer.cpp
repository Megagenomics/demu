#include "buffer.h"

Buffer::Buffer(){}

Buffer::~Buffer()
{
  if(block1) delete [] block1;
  if(block2) delete [] block2;
}

Buffer::Buffer(const Buffer &B)
{
  mPaired=B.mPaired;
  if(mPaired&0x1){
    block1=new char[OUT_BUFFER_SIZE];
    block1[0]='\0';
    strcpy(block1,B.block1);
    pos1=B.pos1;
    freeSpace1=B.freeSpace1;
    mFullPathName1=B.mFullPathName1;
    if(B.block1)delete [] B.block1;
  }
  if(mPaired&0x2){
    block2=new char[OUT_BUFFER_SIZE];
    block2[0]='\0';
    strcpy(block2,B.block2);
    pos2=B.pos2;
    freeSpace2=B.freeSpace2;
    mFullPathName2=B.mFullPathName2;
    if(B.block2)delete [] B.block2;
  }
  mIsCompressed=B.mIsCompressed;
}

Buffer::Buffer(int paired,const string &fullPathName1,const string &fullPathName2,bool isCompressed)
{
  mPaired=paired;
  if(mPaired&0x1){
    block1=new char[OUT_BUFFER_SIZE];
    block1[0]='\0';
    pos1=block1;
    freeSpace1=OUT_BUFFER_SIZE;
    mFullPathName1=fullPathName1;
  }
  if(mPaired&0x2){
    block2=new char[OUT_BUFFER_SIZE];
    block2[0]='\0';
    pos2=block2;
    freeSpace2=OUT_BUFFER_SIZE;
    mFullPathName2=fullPathName2;
  }
  mIsCompressed=isCompressed;
}

Buffer& Buffer::operator=(const Buffer &B)
{
  if(this==&B)
    return *this;
  mPaired=B.mPaired;
  if(mPaired&0x1){
    if(block1)delete [] block1;
    block1=new char[OUT_BUFFER_SIZE];
    block1[0]='\0';
    strcpy(block1,B.block1);
    pos1=block1+strlen(block1);
    freeSpace1=B.freeSpace1;
    mFullPathName1=B.mFullPathName1;
  }
  if(mPaired&0x2){
    if(block2)delete [] block2;
    block2=new char[OUT_BUFFER_SIZE];
    block2[0]='\0';
    strcpy(block2,B.block2);
    pos2=block2+strlen(block2);
    freeSpace2=B.freeSpace2;
    mFullPathName2=B.mFullPathName2;
  }
  mIsCompressed=B.mIsCompressed;
  return *this;
}

void Buffer::storeStr(string str1,string str2)
{
  if((mPaired&0x3)==0x3){
    int len1=str1.length();
    int len2=str2.length();
    unique_lock<mutex> lck(mtx);
    if(len1>=freeSpace1 || len2>=freeSpace2){
      Writer writer1(&mFullPathName1,block1,OUT_BUFFER_SIZE-freeSpace1,mIsCompressed);
      writer1.write();
      freeSpace1=OUT_BUFFER_SIZE;
      block1[0]='\0';
      pos1=block1;

      Writer writer2(&mFullPathName2,block2,OUT_BUFFER_SIZE-freeSpace2,mIsCompressed);
      writer2.write();
      freeSpace2=OUT_BUFFER_SIZE;
      block2[0]='\0';
      pos2=block2;
    }
    strcat(pos1,str1.c_str());
    freeSpace1-=len1;
    pos1+=len1;

    strcat(pos2,str2.c_str());
    freeSpace2-=len2;
    pos2+=len2;
    lck.unlock();
  }else if(mPaired&0x1){
    int len1=str1.length();
    unique_lock<mutex> lck(mtx);
    if(len1>=freeSpace1){
      Writer writer1(&mFullPathName1,block1,OUT_BUFFER_SIZE-freeSpace1,mIsCompressed);
      writer1.write();
      freeSpace1=OUT_BUFFER_SIZE;
      block1[0]='\0';
      pos1=block1;
    }
    strcat(pos1,str1.c_str());
    freeSpace1-=len1;
    pos1+=len1;
    lck.unlock();
  }else if(mPaired&0x2){
    int len2=str2.length();
    unique_lock<mutex> lck(mtx);
    if(len2>=freeSpace2){
      Writer writer2(&mFullPathName2,block2,OUT_BUFFER_SIZE-freeSpace2,mIsCompressed);
      writer2.write();
      freeSpace2=OUT_BUFFER_SIZE;
      block2[0]='\0';
      pos2=block2;
    }
    strcat(pos2,str2.c_str());
    freeSpace2-=len2;
    pos2+=len2;
    lck.unlock();
  }
}

void Buffer::flush()
{
  unique_lock<mutex> lck(mtx);
  if(freeSpace1<OUT_BUFFER_SIZE){
    Writer writer1(&mFullPathName1,block1,OUT_BUFFER_SIZE-freeSpace1,mIsCompressed);
    writer1.write();
  }
  if(freeSpace2<OUT_BUFFER_SIZE){
    Writer writer2(&mFullPathName2,block2,OUT_BUFFER_SIZE-freeSpace2,mIsCompressed);
    writer2.write();
  }
  lck.unlock();
}