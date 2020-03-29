#include <iostream>
#include <fstream>
using namespace std;
int main(int argc, char const *argv[])
{
  /* usage: ./check 1.val */
  int i=0;
  int j=0;
  ifstream fin(argv[1],ios::binary);
  while (fin.good())
  {
    unsigned char byte;
    fin.read((char*)&byte,1);
    if(byte==0x00) j++;
    i++;
  }
  printf("size: %dbytes, broken: %dbytes, %.2lf%% correct\n",i,j,
  ((double(i)-double(j))/double(i))*100.0);  
  return 0;
}
