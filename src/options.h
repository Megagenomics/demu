#ifndef OPTIONS_H
#define OPTIONS_H

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <regex>

using namespace std;

class Sample{
public:
  string index1;
  string index2;
  string title;
};

class Options{
public:
  int paired=0;
  string *input=nullptr;
  string sampleSheet;
  string outFolder;
  bool compressed=false;
  string umdeterminedFileName1;
  string umdeterminedFileName2;
  int threadCount;
  int compression_level;
  vector<Sample> samples;

public:
  Options();
  ~Options();
  bool validate();
  friend ostream & operator<<(ostream &os,const Options &opt);
private:
  void parseSampleSheet();

};

#endif