#include "demu.h"

Options opt;
Reader reader;

int main(int argc, char* argv[]){
  if(argc == 1) {
    cout << "demu: multi-threaded FASTQ demultiplexing\n"
         << COPYRIGHT << ", version " << VERSION << endl;
    return 0;
  }
  
  cmdline::parser cmd;
  
  cmd.add<string>("input1", 'i', "read 1 input file name", false, "");
  cmd.add<string>("input2", 'I', "read 2 input file name", false, "");
  cmd.add<string>("sample-sheet", 's', "a CSV file contains three columns (filename, index1, index2)", true, "");
  cmd.add<string>("out-folder", 'o', "output folder, default is current working directory", false, ".");
  cmd.add<bool>("compressed", 'c', "wether compressed. default is no", false, false);
  cmd.add<string>("undetermined1", 'u', "the read 1 output file name of undetermined data, default is Undetermined", false, "Undetermined-R1");
  cmd.add<string>("undetermined2", 'U', "the read 2 output file name of undetermined data, default is Undetermined", false, "Undetermined-R2");
  cmd.add<int>("compression-level", 'z', "compression level for gzip output (1 ~ 9). 1 is fastest, 9 is smallest, default is 2.", false, 2);
  cmd.add<int>("thread", 't', "used thread count", false, 1);
  cmd.parse_check(argc, argv);
  
  if(!cmd.get<string>("input1").empty() && !cmd.get<string>("input2").empty()){
    opt.input=new string[2];
    opt.paired|=0x3;
    opt.input[0] = cmd.get<string>("input1");
    opt.input[1] = cmd.get<string>("input2");
  }else if(!cmd.get<string>("input1").empty()){
    opt.input=new string[1];
    opt.paired|=0x1;
    opt.input[0] = cmd.get<string>("input1");
  }else if(!cmd.get<string>("input2").empty()){
    opt.input=new string[1];
    opt.paired|=0x2;
    opt.input[0] = cmd.get<string>("input2");
  }
  opt.sampleSheet = cmd.get<string>("sample-sheet");
  opt.outFolder = cmd.get<string>("out-folder");
  opt.compressed = cmd.get<bool>("compressed");
  opt.umdeterminedFileName1 = cmd.get<string>("undetermined1");
  opt.umdeterminedFileName2 = cmd.get<string>("undetermined2");
  opt.compression_level = cmd.get<int>("compression-level");
  opt.threadCount = cmd.get<int>("thread");
  
  opt.validate();


  for(int i=0;i<opt.samples.size();i++){
    Sample s=opt.samples[i];
    if( (opt.paired&0x3)==0x3 ){
      reader.outer[s.index1+"+"+s.index2]= Buffer(
      opt.paired,
      opt.compressed?opt.outFolder+s.title+"-R1.fastq.gz":opt.outFolder+s.title+"-R1.fastq",
      opt.compressed?opt.outFolder+s.title+"-R2.fastq.gz":opt.outFolder+s.title+"-R2.fastq",
      opt.compressed);
    }else if(opt.paired&0x1){
      reader.outer[s.index1]= Buffer(
      opt.paired,
      opt.compressed?opt.outFolder+s.title+"-R1.fastq.gz":opt.outFolder+s.title+"-R1.fastq",
      "",
      opt.compressed);
    }else if(opt.paired&0x2){
      reader.outer[s.index2]= Buffer(
      opt.paired,
      "",
      opt.compressed?opt.outFolder+s.title+"-R2.fastq.gz":opt.outFolder+s.title+"-R2.fastq",
      opt.compressed);
    }
  }
  /*未知序列保存*/
  for(int j=0;j<opt.threadCount;j++){
    if( (opt.paired&0x3)==0x3 ){
      reader.outer[to_string(j)]= Buffer(
      opt.paired,
      opt.compressed?opt.outFolder+opt.umdeterminedFileName1+"-"+to_string(j)+"-fastq.gz":opt.outFolder+opt.umdeterminedFileName1+"-"+to_string(j)+"-fastq",
      opt.compressed?opt.outFolder+opt.umdeterminedFileName2+"-"+to_string(j)+"-fastq.gz":opt.outFolder+opt.umdeterminedFileName2+"-"+to_string(j)+"-fastq",
      opt.compressed);
    }else if(opt.paired&0x1){
      reader.outer[to_string(j)]= Buffer(
      opt.paired,
      opt.compressed?opt.outFolder+opt.umdeterminedFileName1+"-"+to_string(j)+"-fastq.gz":opt.outFolder+opt.umdeterminedFileName1+"-"+to_string(j)+"-fastq",
      "",
      opt.compressed);
    }else if(opt.paired&0x2){
      reader.outer[to_string(j)]= Buffer(
      opt.paired,
      "",
      opt.compressed?opt.outFolder+opt.umdeterminedFileName2+"-"+to_string(j)+"-fastq.gz":opt.outFolder+opt.umdeterminedFileName2+"-"+to_string(j)+"-fastq",
      opt.compressed);
    }
  }

  run();
  
  return 0;
}

 /*处理数据入口*/
void run(){
  reader.init(opt.paired,opt.threadCount,READ_NUM,LINE_LENGTH,opt.samples.size(),opt.input);
  reader.garbageCollection();
}/*for run*/

