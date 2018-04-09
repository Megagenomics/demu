# demu
fastq data Demultiplexing

usage: demu --sample-sheet=string [options] ... <br> 
options:<br> 
  -i, --input1               read 1 input file name (string [=])<br> 
  -I, --input2               read 2 input file name (string [=])<br> 
  -s, --sample-sheet         a CSV file contains three columns (filename, index1, index2) (string)<br> 
  -o, --out-folder           output folder, default is current working directory (string [=.])<br> 
  -c, --compressed           wether compressed. default is no (bool [=0])<br> 
  -u, --undetermined1        the read 1 output file name of undetermined data, default is Undetermined (string [=Undetermined-R1])<br> 
  -U, --undetermined2        the read 2 output file name of undetermined data, default is Undetermined (string [=Undetermined-R2])<br> 
  -z, --compression-level    compression level for gzip output (1 ~ 9). 1 is fastest, 9 is smallest, default is 2. (int [=2])<br> 
  -t, --thread               used thread count (int [=1])<br> 
  -?, --help                 print this message<br> 
  <br> 
  
# build
cd defq<br> 
make<br> 

# install
sudo make install<br> 
```
