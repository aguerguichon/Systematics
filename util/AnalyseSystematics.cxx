#include <iostream>
#include <istream>
#include <fstream>
#include <map>
#include <vector>
#include <string>

#include "THStack.h"

#include "Systematics/EWAnalysis.h"

using namespace std;

int main()
{

  EWAnalysis EWA("Systematics/ConfigFile/EWConfig.boost");

  THStack *stackEW=EWA.AddEW(65, 115);
  //  stackEW->Draw();

  cout<<"End of program."<<endl;
  return 0;
}
