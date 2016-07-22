#include <iostream>
#include <istream>
#include <fstream>
#include <map>
#include <vector>
#include <string>
#include <sstream>
#include <boost/program_options.hpp>
#include <boost/multi_array.hpp>

#include "TH1.h"
#include "TFile.h"
#include "TTree.h"
#include "TBranch.h"
#include "TCanvas.h"
#include "THStack.h"

#include "Systematics/EWAnalysis.h"
#include "PlotFunctions/MapBranches.h"
#include "PlotFunctions/DrawPlot.h"

using namespace std;
namespace po = boost::program_options;



//==============================
//Read configuration options from a configuration file

EWAnalysis::EWAnalysis(string configFileName)
{
  po::options_description desc("");

  desc.add_options()
    ("help", "Display this help message")
    ("dataPattern", po::value<string> (&m_dataPattern), "Data file pattern" ) 
    ("background", po::value<vector <string> >(&m_vectBkg)->multitoken(), "List of bkg" )
    ("scaled", po::value<bool> (&m_isScaled), "Scaled or not" ) 
    ;

  po::variables_map vm;
  ifstream ifs( configFileName, ifstream::in );
  po::store(po::parse_config_file(ifs, desc), vm);
  po::notify(vm);

  if (vm.count("help")) {cout << desc;}
  cout<<"Configuration file "<<configFileName<<" loaded."<<endl;
}

//================================
EWAnalysis::~EWAnalysis()
{
}

//================================
void EWAnalysis::AddEW(double mMin, double mMax)
{
  string path="/sps/atlas/a/aguerguichon/Calibration/DataxAOD/";
  
  TFile *bkgFile=0;
  TTree *bkgTree=0;
  unsigned int nEntries;
  string bkgPattern;
  double m12;
  map <string, double> mapDouble;
  map <string, TH1D*> mapHist;
  vector <TH1*> vectHist;

  for (unsigned int iBkg=0; iBkg<m_vectBkg.size(); iBkg++)
    {
      if (m_isScaled) bkgPattern="MC_13TeV_"+m_vectBkg[iBkg]+"_2015c_Lkh1_scaled";
      else bkgPattern="MC_13TeV_"+m_vectBkg[iBkg]+"_2015c_Lkh1"; 
      bkgFile= TFile::Open( (path+bkgPattern+"/"+bkgPattern+"_0.root").c_str() );
      bkgTree= (TTree*)bkgFile->Get( (bkgPattern+"_0_selectionTree").c_str() );
    
      MapBranches mapBranches;
      mapBranches.LinkTreeBranches(bkgTree);
      nEntries= bkgTree->GetEntries();

      for (unsigned int iEntry=0; iEntry<nEntries; iEntry++)
	{
	  bkgTree->GetEntry(iEntry);
	  mapDouble=mapBranches.GetMapDouble();
	  m12=mapDouble.at("m12");
	  
	  if (m12<mMin || m12>mMax) continue;

	  if (m_vectBkg[iBkg]!="Ztautau" && m_vectBkg[iBkg]!="ttbar")
	    {
	      if (mapHist.count("Diboson")==0) mapHist.insert( make_pair("Diboson", new TH1D("Diboson", "",mMax-mMin, mMin, mMax ) ) );
	      mapHist["Diboson"]->Fill(m12);
	    } 
	  else
	    {
	      if (mapHist.count(m_vectBkg[iBkg])==0) mapHist.insert( make_pair(m_vectBkg[iBkg], new TH1D(m_vectBkg[iBkg].c_str(), "",mMax-mMin, mMin, mMax ) ) );
	      mapHist[m_vectBkg[iBkg]]->Fill(m12);
	    }
	}
      cout<<m_vectBkg[iBkg]<<" added."<<endl;
    }

  bkgFile->Close();

  for (auto it: mapHist)                                             
    {                                                                
      vectHist.push_back(it.second);
    }


  TFile *dataFile=TFile::Open( (path+m_dataPattern+"/"+m_dataPattern+"_0.root").c_str() );
  TTree *dataTree=(TTree*)dataFile->Get( (m_dataPattern+"_0_selectionTree").c_str() ); 

  MapBranches mapBranches;
  mapBranches.LinkTreeBranches(dataTree);
  nEntries= dataTree->GetEntries();

  TH1D *dataHist=new TH1D("data","", mMax-mMin, mMin, mMax);

  for (unsigned int iEntry=0; iEntry<nEntries; iEntry++)
    {
      dataTree->GetEntry(iEntry);
      mapDouble=mapBranches.GetMapDouble();
      m12=mapDouble.at("m12");
      
      if (m12<mMin || m12>mMax) continue;
      dataHist->Fill(m12);
    }
  
  cout<<"Data added."<<endl;
  vectHist.push_back(dataHist);
  
  vector <string> vectOpt;
  vectOpt.push_back("stack=1");

  DrawPlot(vectHist, "testStack", vectOpt); 

  //===================End
  cout<< "EWAnalysis::AddEW done."<<endl;
  delete bkgFile;
  return;
}
