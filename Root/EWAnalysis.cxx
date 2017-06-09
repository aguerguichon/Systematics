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
#include "TLegend.h"

#include "Systematics/EWAnalysis.h"
#include "PlotFunctions/MapBranches.h"
#include "PlotFunctions/DrawPlot.h"
#include "PlotFunctions/SideFunctions.h"
#include "PlotFunctions/SideFunctionsTpp.h"
#include "PlotFunctions/AtlasStyle.h"
#include "PlotFunctions/AtlasUtils.h"
#include "PlotFunctions/AtlasLabels.h"

using namespace ChrisLib;
using namespace std;
namespace po = boost::program_options;



//==============================
//Read configuration options from a configuration file

EWAnalysis::EWAnalysis(string configFileName)
{
  po::options_description desc("");
  vector <string> vectData;
  vector <string> vectMC;

  desc.add_options()
    ("help", "Display this help message")
    ("MC", po::value<vector <string> >(&vectMC)->multitoken(), "List of bkg" )
    ("MCName", po::value<vector <string> >(&m_vectMCName)->multitoken(), "Names" )
    ("dataFiles", po::value<vector <string> >(&vectData)->multitoken(), "Data files" )
    ;

  po::variables_map vm;
  ifstream ifs( configFileName, ifstream::in );
  po::store(po::parse_config_file(ifs, desc), vm);
  po::notify(vm);

  if (vm.count("help")) {cout << desc;}

  for (unsigned int iMC=0; iMC < vectMC.size(); iMC++){
    m_vectMC.push_back( vector<string>() );
    ParseVector( vectMC[iMC], m_vectMC.back(), 0  );
  }

  for (unsigned int iData=0; iData < vectData.size(); iData++){
    m_vectData.push_back( vector<string>() );
    ParseVector( vectData[iData], m_vectData.back(), 0  );
  }
  
  cout<<"Configuration file "<<configFileName<<" loaded."<<endl;
}

//================================
EWAnalysis::~EWAnalysis()
{
}

//================================
void EWAnalysis::AddEW(double mMin, double mMax)
{
  //  string path="/sps/atlas/a/aguerguichon/Calibration/DataxAOD/";
  SetAtlasStyle();
  
  TFile *MCFile=0;
  TTree *MCTree=0;
  unsigned int nEntries;
  double m12, weight;
  vector <TH1*> vectHist;
  THStack *stack=new THStack("stackEW", "");
  double weightMCtoData= ( 79949000/(1.901e6) ) / (32.9+3.2);
  
  TH1::AddDirectory(kFALSE);

  unsigned int colors[] = {801, 825, 861, 390 };

  for (unsigned int iMC=0; iMC<m_vectMC.size(); iMC++){
    m_vectMCName[iMC]=ParseLegend( m_vectMCName[iMC] ); 

    vectHist.push_back( new TH1D(m_vectMCName[iMC].c_str(), "",(mMax-mMin)/1000, mMin, mMax ) );   
    for (unsigned int iBkg=0; iBkg <m_vectMC[iMC].size(); iBkg++){
      MCFile= TFile::Open( (m_vectMC[iMC][iBkg]).c_str()  );
      MCTree= (TTree*)MCFile->Get( "correctedMC" );

      MapBranches mapBranches;
      mapBranches.LinkTreeBranches(MCTree, 0, {"m12", "weight_1516", "weightNorm_1516"});
      nEntries= MCTree->GetEntries();

      for (unsigned int iEntry=0; iEntry<nEntries; iEntry++)
	{
	  MCTree->GetEntry(iEntry);
	  m12=mapBranches.GetDouble("m12");
	  weight=mapBranches.GetDouble("weight_1516")*mapBranches.GetDouble("weightNorm_1516");
	  
	  if (m12<mMin || m12>mMax) continue;

	  vectHist[iMC]->Fill(m12, weight*weightMCtoData);
	}
      cout<<m_vectMC[iMC][iBkg]<<" added."<<endl;
      MCFile->Close();
    }
    vectHist[iMC]->SetFillColor(colors[iMC]);
    vectHist[iMC]->SetLineColor(kBlack);
    vectHist[iMC]->SetLineWidth(1);
    vectHist[iMC]->GetXaxis()->SetTitle("Mass [MeV]");
    vectHist[iMC]->GetYaxis()->SetTitle("Number of events / GeV");
    stack->Add(vectHist[iMC], "HIST F");
  }


  TFile *dataFile{0};
  TTree *dataTree{0};
  TH1 *dataHist{0};

  for (unsigned int iData=0; iData<m_vectData.size(); iData++){

    dataHist=new TH1D(("data"+to_string(iData)).c_str(),"", (mMax-mMin)/1000, mMin, mMax);

    for (unsigned int iFile=0; iFile<m_vectData[iData].size(); iFile++){

      dataFile=TFile::Open( (m_vectData[iData][iFile]).c_str() );
      dataTree= (TTree*)dataFile->Get( "correctedData" ); 
      MapBranches mapBranches;
      mapBranches.LinkTreeBranches(dataTree, 0, {"m12"});
      nEntries= dataTree->GetEntries();


      for (unsigned int iEntry=0; iEntry<nEntries; iEntry++)
	{
	  dataTree->GetEntry(iEntry);
	  m12=mapBranches.GetDouble("m12");
      
	  if (m12<mMin || m12>mMax) continue;
	  dataHist->Fill(m12); 
	}

    }
    vectHist.push_back(dataHist);
 
    cout<<"Data superimposed."<<endl;
      
  }
  double yMin=10e2;
  double yMax=1e8;


  //Draw stacked plot

  TCanvas *canvas= new TCanvas ("canvas", "");
  canvas->DrawFrame(mMin, yMin, mMax, yMax);
  canvas->SetLogy();
  stack->SetMinimum(yMin);
  stack->SetMaximum(yMax);
  stack->Draw();
  stack->GetXaxis()->SetTitle("Mass [MeV]");
  stack->GetYaxis()->SetTitle("Number of events / GeV");
  canvas->Update();

  dataHist->SetMarkerStyle(8);
  dataHist->SetMarkerSize(0.7);
  dataHist->SetLineColor(kBlack);
  dataHist->GetXaxis()->SetTitle("Mass [MeV]");
  dataHist->GetYaxis()->SetTitle("Number of events / GeV");

  dataHist->Draw("same P");
  
  ATLASLabel( 0.16, 0.85, "Work in progress", 1, 0.04 );
  myText( 0.16, 0.8, 1, "#sqrt{s} = 13 TeV, L = 3.2 (2015) + 32.9 (2016) fb^{-1}", 0.035 );

  auto legend = new TLegend(0.7, 0.7, 0.9, 0.9);
  for (unsigned int iMC=0; iMC<m_vectMC.size(); iMC++){  legend->AddEntry(vectHist[iMC], m_vectMCName[iMC].c_str(), "f");}
  legend->AddEntry(dataHist, "Data", "p");
  legend->SetBorderSize(0);
  legend->SetTextSize(0.035);
  legend->Draw();

  // TGaxis *axisX = new TGaxis(mMin,yMin,mMin,yMax,yMin,yMax,stack->GetYaxis()->GetNdivisions(),"-G");
  // axisX->ImportAxisAttributes(stack->GetYaxis());
  // axisX->SetTitle("");
  // axisX->SetMoreLogLabels();
  // axisX->SetLineColor(kBlack);
  // axisX->SetLabelSize(0);
  //  axisX->Draw(); 

  // TGaxis *axisY = new TGaxis(mMin,yMin,mMax,yMin,mMin,mMax,stack->GetXaxis()->GetNdivisions());
  // axisY->ImportAxisAttributes(stack->GetXaxis());
  // axisY->SetTitle("");
  // axisY->SetLineColor(kBlack);
  // axisY->SetLabelSize(0);
  //axisY->Draw(); 


  canvas->SaveAs("EWBackground.eps");

  dataFile->Close();

  //  DrawPlot(vectHist, "testStack", vectOpt); 

  //===================End
  cout<< "EWAnalysis::AddEW done."<<endl;
  delete MCFile;
  delete dataFile;
  delete dataHist;
  delete stack;
  //delete axisX;
  //delete axisY;
  delete canvas;
  return;
}
