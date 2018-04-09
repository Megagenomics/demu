#include "reader.h"

Reader::Reader(){}

void Reader::init(int inputFlag,int threads,int readNum4Buffer,int lineLength,int sampleSize,string *inputFileName)
{
  mInputFlag=inputFlag;
  mFileNum=mInputFlag/3+1;
  mThreads=threads;
  mReadNum4Buffer=readNum4Buffer;
  mLineLength=lineLength;
  mSampleSize=sampleSize;

  mInputFileName=new string[mFileNum];
  for(int i=0;i<mFileNum;i++)
    mInputFileName[i]=inputFileName[i];
  mGzFile=new gzFile[mFileNum];
  mFile=new ifstream[mFileNum];
  mIsGz=new bool[mFileNum];

  
  /*创建文件描述符*/
  for(int i=0;i<mFileNum;i++){
    if(mInputFileName[i].substr(mInputFileName[i].length()-3)==".gz"){
      mGzFile[i] = gzopen(mInputFileName[i].c_str(), "r");
      gzbuffer(mGzFile[i],131072);//128K=131072
      mIsGz[i] = true;
	    gzrewind(mGzFile[i]);
    }else{
      mFile[i].open(mInputFileName[i], ifstream::in);
		  mIsGz[i] = false;
    }
  }

  /*buffer相关资源申请*/
  mtx_buf=new mutex[mThreads];
  cv_buf=new condition_variable[mThreads];

  /*read仓库相关资源申请*/
  mReadNum=new int*[mThreads];
  for(int i=0;i<mThreads;i++){
    mReadNum[i]=new int[mFileNum];
    for(int j=0;j<mFileNum;j++)mReadNum[i][j]=0;
  }
  mReadRepository=new char*[mThreads*mFileNum*mReadNum4Buffer*4];
  for(int i=0;i<mThreads*mFileNum*mReadNum4Buffer*4;i++){
    mReadRepository[i]=new char[mLineLength];
  }
  mReadRepositoryFlag=new bool[mThreads];
  for(int i=0;i<mThreads;i++) mReadRepositoryFlag[i]=false;
  
  /*开启输入线程*/
  startUpInputThread();

  /*开启消费线程*/
  setConsumer();
}

void Reader::garbageCollection()
{
  /*等待消费线程结束*/
  destroyConsumer();
  /*等待输入线程结束*/
  joinInputThread();

  /*buffer相关资源销毁*/
  if(mReadRepositoryFlag)delete [] mReadRepositoryFlag;
  for(int i=0;i<mThreads*mFileNum*mReadNum4Buffer*4;i++){
    if(mReadRepository[i])delete [] mReadRepository[i];
  }
  if(mReadRepository)delete [] mReadRepository;
  for(int i=0;i<mThreads;i++){
    if(mReadNum[i])delete [] mReadNum[i];
  }
  if(mReadNum)delete [] mReadNum;

  if(cv_buf)delete [] cv_buf;
  if(mtx_buf)delete [] mtx_buf;

  /*关闭文件描述符*/
  for(int i=0;i<mFileNum;i++){
    if(mIsGz[i]){
      gzclose(mGzFile[i]);
		  mGzFile[i] = NULL;
    }else{
      if (mFile[i].is_open()){
		  	mFile[i].close();
	    }
    }
  }/*for i*/

  /*释放文件描述符资源*/
  if(mIsGz)delete [] mIsGz;
  if(mFile)delete [] mFile;
  if(mGzFile)delete [] mGzFile;
  if(mInputFileName)delete [] mInputFileName;

  /*将缓冲区的数据写出*/
  flush();
}

void Reader::startUpInputThread()
{
  mInputFileThread  = new thread*[mFileNum];
  for(int t=0;t<mFileNum;t++){ 
    mInputFileThread[t] =new thread(bind(&Reader::fillPack, this,t));
  }
}

void Reader::fillPack(int id)
{
  char pName[16];
  sprintf(pName,"filling-%02d",id);
  prctl(PR_SET_NAME,pName);
  /*cout<<"进入输入线程"<<id<<endl;//测试*/
//   int i=0;
  while(true){
    unique_lock<mutex> lck1(mtx_input2input);
    while(TFlag<0){
      unique_lock<mutex> lck2(mtx_buf[DFlag]);
      if(!mReadRepositoryFlag[DFlag]) TFlag=DFlag;
      lck2.unlock();
      if(DFlag==(mThreads-1)){
        DFlag=0;
      }else{
        DFlag++;
      }
    }
    /*cout<<"F->id:"<<id<<",fill:"<<DFlag<<endl;//测试*/
    lck1.unlock();
    
    int j=0,k=mFileNum;
    char **T=mReadRepository+TFlag*k*mReadNum4Buffer*4+id;
    int r=k*mReadNum4Buffer*4;
    if(mIsGz[id]){/*gz文件*/
      while(j<r && gzgets(mGzFile[id],T[j],mLineLength)!=NULL){j+=k;};
    }else{/*普通文件*/
      while(j<r && mFile[id].getline(T[j], mLineLength)){j+=k;};
    }

    if(!(mInputFlag^0x3)){
      unique_lock<mutex> lck3(mtx_input2input);
      IFlag|=(1<<id);
      mReadNum[TFlag][id]=j;
      if(j<=0){
        mIsCompleted|=(1<<id);
        lck3.unlock();
        break;
      }
     
      if(IFlag^mInputFlag){
        cv_input2input.wait(lck3);
        goto OUT;
      }else{
        lck3.unlock();
        
        unique_lock<mutex> lck4(mtx_buf[TFlag]);
        mReadRepositoryFlag[TFlag]=true;
        lck4.unlock();
        cv_buf[TFlag].notify_all();

        IFlag=0;
        TFlag=-1;
        
        cv_input2input.notify_all();
      }
      OUT:;
    }else{
      if(j<=0)mIsCompleted|=(1<<id);
      TFlag=-1;
    }
  }/*while*/
}/*fill_reads*/

void Reader::joinInputThread()
{
  for(int t=0;t<mFileNum;t++){
    mInputFileThread[t]->join();
  }
  for(int t=0;t<mFileNum;t++){ 
    if(mInputFileThread[t]) delete mInputFileThread[t];
  }
  delete [] mInputFileThread;
}

void Reader::setConsumer()
{
  mConsumerThread  = new thread*[mThreads];
  for(int t=0;t<mThreads;t++){ 
    mConsumerThread[t] =new thread(bind(&Reader::consumer,this,t));
  }
}

void Reader::destroyConsumer()
{
  for(int t=0;t<mThreads;t++){
    mConsumerThread[t]->join();
  }
  for(int t=0;t<mThreads;t++){
    if(mConsumerThread[t])delete mConsumerThread[t];
  }
  delete [] mConsumerThread;
}

/*各线程工作台*/
void Reader::consumer(int id)
{
  /*cout<<"进入消费线程"<<id<<endl;//测试*/
  char pName[16];
  memset(pName, 0, 16);  
  snprintf(pName, 16, "consuming-%02d",id);  
  prctl(PR_SET_NAME, pName);

  int range=id*mFileNum*mReadNum4Buffer*4;

  while(true){
    unique_lock<mutex> lck1(mtx_buf[id]);
    while(!mReadRepositoryFlag[id]){
      if(mIsCompleted==mInputFlag){
        lck1.unlock();
        goto OUT2;
      }
      cv_buf[id].wait_for(lck1,std::chrono::seconds(1)) == std::cv_status::timeout;
    }
    lck1.unlock();
    
    //这里不对数量和成对做检查了
    if(!(mInputFlag^0x3) && mReadNum[id][0]!=mReadNum[id][1])break;
    char **S=mReadRepository+range;
    //cout<<"C->id:"<<id<<",consuming:"<<mReadNum[id][0]<<","<<mReadNum[id][1]<<endl;//测试*/
    
    int len=mReadNum[id][0];
    if(!(mInputFlag^0x3)){
     for(int m=0;m<len;m+=8){
        string R1(S[m]),R2(S[m+1]);
        R1+=S[m+2];
        R2+=S[m+3];
        R1+=S[m+4];
        R2+=S[m+5];
        R1+=S[m+6];
        R2+=S[m+7];
        char *ptr = strrchr(S[m],':');
        if(ptr){
          string key="";
          key.assign(ptr+1,strlen(ptr)-2);
          if(outer.count(key)>0){
            outer[key].storeStr(R1,R2);
          }else{
            outer[to_string(id)].storeStr(R1,R2);
          }
        }
      }
    }else if(mInputFlag&0x1){
      for(int m=0;m<len;m+=4){
        string R1(S[m]);
        R1+=S[m+1];
        R1+=S[m+2];
        R1+=S[m+3];
        char *ptr = strrchr(S[m],':');
        char *ptr2 = strrchr(ptr,'+');
        if(ptr){
          string key;
          if(ptr2){
            key.assign(ptr+1,ptr2-ptr);
          }else{
            key.assign(ptr+1,strlen(ptr)-2);
          }
          if(outer.count(key)>0){
            outer[key].storeStr(R1,"");
          }else{
            outer[to_string(id)].storeStr(R1,"");
          }
        }
      }
    }else if(mInputFlag&0x2){
      for(int m=0;m<len;m+=4){
        string R2(S[m]);
        R2+=S[m+1];
        R2+=S[m+2];
        R2+=S[m+3];
        char *ptr = strrchr(S[m],':');
        char *ptr2 = strrchr(ptr,'+');
        if(ptr){
          string key;
          if(ptr2){
            key.assign(ptr+1,ptr2-ptr);
          }else{
            key.assign(ptr+1,strlen(ptr)-2);
          }
          if(outer.count(key)>0){
            outer[key].storeStr("",R2);
          }else{
            outer[to_string(id)].storeStr("",R2);
          }
        }
      }
    }
    
    unique_lock<mutex> lck2(mtx_buf[id]);
    mReadRepositoryFlag[id]=false;
    lck2.unlock();
  }
  OUT2:;
}/*workbench*/

/*刷新缓冲区*/
void Reader::flush(){
  map<string,Buffer>::iterator it;
  it = outer.begin();
  while(it != outer.end()){
    it->second.flush();
    it++;
  }
}