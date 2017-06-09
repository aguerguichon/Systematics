#include <iostream>
#include <istream>
#include <fstream>
#include <map>
#include <vector>
#include <string>
#include "boost/multi_array.hpp"

#include "THStack.h"

class EWAnalysis
{

 public:
  EWAnalysis(std::string configFileName);
  ~EWAnalysis();
  
  void AddEW(double mMin, double mMax);

 private:
  std::vector <std::vector<std::string>> m_vectMC;
  std::vector <std::vector<std::string>> m_vectData;
  std::vector <std::string> m_vectMCName;
  
};
