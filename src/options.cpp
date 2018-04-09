#include "options.h"
#include "util.h"

Options::Options(){
  umdeterminedFileName1="Undetermined-R1";
  umdeterminedFileName2="Undetermined-R2";
  threadCount=1;
  compression_level=2;
}

Options::~Options()
{
  if(input)delete [] input;
}

void Options::parseSampleSheet() {
  if(sampleSheet.empty()) {
    error_exit("sample sheet CSV file should be specified by -s or --sample_sheet");
  } else {
    check_file_valid(sampleSheet);
  }

  ifstream file;
  file.open(sampleSheet.c_str(), ifstream::in);
  const int maxLine = 1000;
  char line[maxLine];
  if(file.getline(line, maxLine)){
    regex headReg("^title,index1,index2.*", regex::icase);
    cmatch sRes;
    if(!regex_match(line, sRes, headReg)){
      file.close();
      error_exit("the head of sample sheet must be \"title,index1,index2\".");
    }
  }else{
    file.close();
    error_exit("the file is empty.");
  }
  while(file.getline(line, maxLine)){
    // trim \n, \r or \r\n in the tail
    int readed = strlen(line);
    if(readed >=2 ){
      if(line[readed-1] == '\n' || line[readed-1] == '\r'){
        line[readed-1] = '\0';
        if(line[readed-2] == '\r')
          line[readed-2] = '\0';
      }
    }
    string linestr(line);

    vector<string> splitted;
    split(linestr, splitted, ",");
    // a valid line need 4 columns: name, left, center, right
    if(splitted.size()<2)
        continue;

    Sample s;
    s.title = trim(splitted[0]);
    s.index1 = trim(splitted[1]);
    if(splitted.size()>=3)
        s.index2 = trim(splitted[2]);

    this->samples.push_back(s);
  }
  file.close();
}

bool Options::validate() {
  if((paired&0x3)==0x3 && input[0].empty() && input[1].empty()) {
    error_exit("No input file!");
  }else if((paired&0x3)==0x3 && !input[0].empty() && !input[1].empty()){
    check_file_valid(input[0]);
    check_file_valid(input[1]);
  }else if( (paired&0x1 || paired&0x2) && !input[0].empty()){
    check_file_valid(input[0]);
  }

  if(!file_exists(outFolder)) {
    mkdir(outFolder.c_str(), 0777);
  }

  if(file_exists(outFolder) && !is_directory(outFolder)) {
    error_exit(outFolder + " is a file, not a directory");
  }

  if(!file_exists(outFolder) || !is_directory(outFolder)) {
    error_exit(outFolder + " is not a directory, or cannot be created");
  }
  if(outFolder[outFolder.length()-1]!='/')outFolder+="/";

  parseSampleSheet();

  if(samples.size() == 0)
    error_exit("no sample found, did you provide a valid sample sheet CSV file by -s or --sample_sheet?");

  return true;
}

ostream & operator<<(ostream &os,const Options &opt)
{
  os<<"双端标识:"<<opt.paired<<endl;
  //os<<"输入文件R1:"<<opt.input1<<endl;
  //os<<"输入文件R2:"<<opt.input2<<endl;
  os<<"样本信息表:"<<opt.sampleSheet<<endl;
  os<<"输出文件夹:"<<opt.outFolder<<endl;
  os<<"是否压缩:"<<opt.compressed<<endl;
  os<<"Undetermined R1 文件名:"<<opt.umdeterminedFileName1<<endl;
  os<<"Undetermined R2 文件名:"<<opt.umdeterminedFileName2<<endl;
  os<<"线程数:"<<opt.threadCount<<endl;
  os<<"压缩等级:"<<opt.compression_level<<endl;
  return os;
}