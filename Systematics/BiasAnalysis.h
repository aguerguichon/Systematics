#include <iostream>
#include <istream>
#include <fstream>
#include <map>
#include <vector>
#include <string>
#include "boost/multi_array.hpp"
#include "TH1.h"
#include "TMatrixD.h"
#include "RooDataSet.h"
#include "RooGaussian.h"

using namespace RooFit;


class BiasAnalysis
{

public:
  BiasAnalysis(std::string configFileName);
  ~BiasAnalysis();
  /* - Read files, link tree branches to local variables and sorting variables according to the ones selected.                                                                               
    - Fill the map m_mapHist with unique histogramms for each combination of all possible values of each variable.                                                                          
   - Fill m_mapCij and m_mapErrCij for the inversion procedure
 */ 
  void SelectVariables (std::vector <std::string> dataFiles);

  //For each histogram, fill the 2D multi_array with:                                           
  // - 1st dim: histogram                                                                       
  // - 2nd dim: mean (for a given method), mean error, rms...                                   
  //Fill a csv file with those values. 

  void SaveBiasInfo (std::string outName);

  std::vector<double> GetBiasStat( TH1* hist, std::string histName, unsigned int method=1);

  void MakeBiasPlots (std::string path, std::string latexFileName, std::string comment=" ");

  void MakePdf (std::string latexFileName, std::vector <std::string> vectHistNames ,std::string comment=" ");

  void InvertCijMatrix (std::string path, unsigned int inversionProcedure=11);


  void MakeCompStatPlot(std::string csvFileName, std::string plotName, unsigned int input, bool isConf=1);

  void RemoveExtremalBins(TH1D* &hist);

 private:
  std::vector <std::string> m_variablesBias;
  std::vector <unsigned int> m_variablesStats;

  std::map <std::string, TH1D*> m_mapHist;
  std::map <std::string, double> m_mapSumX;
  std::map <std::string, double> m_mapSumXM;
  std::map <std::string, std::vector<double> > m_mapStatHist;
  std::map <std::string, RooRealVar*> m_mapRooVar;
  std::map <std::string, RooDataSet*> m_mapRooDataSet;
  std::map <std::string, RooGaussian*> m_mapRooGauss;
  std::map <unsigned long long, TMatrixD> m_mapCij;
  std::map <unsigned long long, TMatrixD> m_mapErrCij; 
  std::map <unsigned long long, TMatrixD> m_mapCi;
  std::map <unsigned long long, TMatrixD> m_mapErrCi; 

  std::string m_inTreeName;

  unsigned int m_nBins;
  unsigned int m_methodStats;
  double m_minX, m_maxX;
};
