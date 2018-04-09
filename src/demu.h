#ifndef DEMU_H
#define DEMU_H

#define VERSION "1.0.0"
#define COPYRIGHT "BT team, 14 Mar 18, www.megagenomics.cn"

#include <zlib.h>
#include "cmdline.h"
#include "options.h"
#include "reader.h"
//#include "buffer.h"
#include "util.h"
#include "common.h"

#include <iostream>
#include <cstring>
#include <string>
#include <thread>
#include <mutex>
#include <map> 

using namespace std;

extern Options opt;
extern void run();
extern void workbench(int id);

#endif
