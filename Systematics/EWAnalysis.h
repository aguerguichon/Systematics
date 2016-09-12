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
  std::string m_dataPattern;
  std::vector <std::string> m_vectMC;
  bool m_isScaled;
  
};
