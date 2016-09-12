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
#include "TGaxis.h"

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
    ("MC", po::value<vector <string> >(&m_vectMC)->multitoken(), "List of bkg" )
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
  
  TFile *MCFile=0;
  TTree *MCTree=0;
  unsigned int nEntries;
  string MCPattern;
  double m12, weight;
  map <string, double> mapDouble;
  map <string, TH1*> mapHist;
  vector <TH1*> vectHist;
  THStack *stack=new THStack("stackEW", "");
  double weightLumi= ( 19920600/(1.901e6) ) / 2.7;
  cout<< "weightLumi: "<<weightLumi<<endl;

  TH1::AddDirectory(kFALSE);

  unsigned  int colors[] = {1,
                  632, 600, 616, 416, 800,
                  921,
                  629, 597, 613, 413, 797,
                  635, 603, 619, 419, 807 };

  for (unsigned int iMC=0; iMC<m_vectMC.size(); iMC++)
    {
      if (m_isScaled) MCPattern="MC_13TeV_"+m_vectMC[iMC]+"_2015c_Lkh1_scaled";
      else MCPattern="MC_13TeV_"+m_vectMC[iMC]+"_2015c_Lkh1"; 

      MCFile= TFile::Open( (path+MCPattern+"/"+MCPattern+"_0.root").c_str() );

      MCTree= (TTree*)MCFile->Get( (MCPattern+"_0_selectionTree").c_str() );

      MapBranches mapBranches;
      mapBranches.LinkTreeBranches(MCTree);
      nEntries= MCTree->GetEntries();

      for (unsigned int iEntry=0; iEntry<nEntries; iEntry++)
	{
	  MCTree->GetEntry(iEntry);
	  mapDouble=mapBranches.GetMapDouble();
	  m12=mapDouble.at("m12");
	  weight=mapDouble.at("weight")/weightLumi;
	  
	  if (m12<mMin || m12>mMax) continue;

	  if (m_vectMC[iMC]=="Ztautau" || m_vectMC[iMC]=="ttbar" ||m_vectMC[iMC]=="Zee")
	    {
	      string histName=m_vectMC[iMC];
	      if (mapHist.count(m_vectMC[iMC])==0) mapHist[m_vectMC[iMC]]=new TH1D(histName.c_str(), "",mMax-mMin, mMin, mMax );   
	      mapHist[m_vectMC[iMC]]->Fill(m12, weight);
	    } 
	  else
	    {
	      if (mapHist.count("Diboson")==0) mapHist["Diboson"]= new TH1D("Diboson", "",mMax-mMin, mMin, mMax );  
	      mapHist["Diboson"]->Fill(m12, weight/3);
	    }
	}
      cout<<m_vectMC[iMC]<<" added."<<endl;
      MCFile->Close();
    }


  unsigned iColor=1;
  for (auto it: mapHist)                                             
    { 
      vectHist.push_back(it.second);
      it.second->SetFillColor(colors[iColor]);
      it.second->SetLineColor(colors[iColor]);
      iColor++;
    }

  stack->Add(mapHist["ttbar"], "HIST F");
  stack->Add(mapHist["Ztautau"], "HIST F");
  stack->Add(mapHist["Diboson"], "HIST F");
  stack->Add(mapHist["Zee"], "HIST F");

  TFile *dataFile=TFile::Open( (path+m_dataPattern+"/"+m_dataPattern+"_0.root").c_str() );
  TTree *dataTree=(TTree*)dataFile->Get( (m_dataPattern+"_0_selectionTree").c_str() ); 

  MapBranches mapBranches;
  mapBranches.LinkTreeBranches(dataTree);
  nEntries= dataTree->GetEntries();

  TH1 *dataHist=new TH1D("data","", mMax-mMin, mMin, mMax);

  for (unsigned int iEntry=0; iEntry<nEntries; iEntry++)
    {
      dataTree->GetEntry(iEntry);
      mapDouble=mapBranches.GetMapDouble();
      m12=mapDouble.at("m12");
      
      if (m12<mMin || m12>mMax) continue;
      dataHist->Fill(m12); 
    }

  vectHist.push_back(dataHist);
 
  cout<<"Data superimposed."<<endl;

  double yMin=10;
  double yMax=1e6;

  // vector <string> vectOpt;
  // vectOpt.push_back("stack=1");

  //Draw stacked plot

  TCanvas *canvas= new TCanvas ("canvas", "");
  canvas->DrawFrame(mMin, yMin, mMax, yMax);
  canvas->SetLogy();
  stack->Draw("same");
  
  dataHist->SetMarkerStyle(8);
  dataHist->SetMarkerSize(0.7);
  dataHist->SetLineColor(kBlack);
  dataHist->Draw("same P");

  TGaxis *axisX = new TGaxis(mMin,yMin,mMin,yMax,yMin,yMax,stack->GetYaxis()->GetNdivisions(),"-G");
  axisX->ImportAxisAttributes(stack->GetYaxis());
  axisX->SetTitle("");
  axisX->SetMoreLogLabels();
  axisX->SetLineColor(kBlack);
  axisX->SetLabelSize(0);
  axisX->Draw(); 

  TGaxis *axisY = new TGaxis(mMin,yMin,mMax,yMin,mMin,mMax,stack->GetXaxis()->GetNdivisions());
  axisY->ImportAxisAttributes(stack->GetXaxis());
  axisY->SetTitle("");
  axisY->SetLineColor(kBlack);
  axisY->SetLabelSize(0);
  axisY->Draw(); 


  canvas->SaveAs("testStack.pdf");

  dataFile->Close();

  //  DrawPlot(vectHist, "testStack", vectOpt); 

  //===================End
  cout<< "EWAnalysis::AddEW done."<<endl;
  delete MCFile;
  delete dataFile;
  delete dataHist;
  delete stack;
  delete axisX;
  delete axisY;
  delete canvas;
  return;
}
