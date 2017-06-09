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
#include "TProfile.h"
#include "TLorentzVector.h"
#include "TRandom.h"
#include "TPad.h"
#include "TROOT.h"
#include "TLine.h"
#include "TLegend.h"

#include "PlotFunctions/MapBranches.h"
#include "PlotFunctions/DrawPlot.h"
#include "PlotFunctions/InputCompare.h"
#include "PlotFunctions/AtlasStyle.h"
#include "PlotFunctions/AtlasUtils.h"
#include "PlotFunctions/AtlasLabels.h"

using namespace ChrisLib;
using namespace std;
using std::map;
namespace po = boost::program_options;

int main(int argc, char *argv[])
{


  //=================                                                                      
  //Define options to read name of files from the command line                             
  po::options_description desc("Input data files (.root)");                                
  bool doSyst;

  desc.add_options()                                                                       
    ("help", "Display this help message")                                                  
    ("doSyst", po::value<bool>(&doSyst)->default_value(0), "Create the scales uncertainty band histogram" )               
    //    ("year", po::value<string>(&year)->default_value("16"), "15 or 16" )               
    ;                                                                                      

  po::variables_map vm;                                                                    
  po::store(po::command_line_parser(argc, argv).options(desc).style(po::command_line_style::unix_style ^ po::command_line_style::allow_short).run(), vm);                 
  po::notify(vm);                                                                         
  if (vm.count("help")) {cout << desc; return 0;}    
                                      
  //=================================================

  SetAtlasStyle();
  //  TH1::AddDirectory(kFALSE);

  unsigned int bin;
  double SF, phi, eta, pt, deltaSF, errScale;
  string fileName, pattern, lumi, histName, year;

  vector < vector<string> > rootFilesName, inObjName;
  vector <TLorentzVector> VLElec(3), VLZ(3);
  vector <TH1D*> vectHist;
  vector < vector<TH1D*> > tmpHist(3, vector<TH1D*>(3));

  TFile *inFile=0;
  TTree *inTree=0;
  TCanvas *cRatio=0;
  TRandom randN;
  TH1D *systHist=0;
  TH1D *scaleHist=0;

  double x = 0.75;
  double y = 0.82;
  double lsize = 0.04;
  float sizeText=0.065;
  unsigned nBins= 40;

  TFile *scalesFile= TFile::Open( "/sps/atlas/a/aguerguichon/Calibration/ScaleResults/170125/EnergyScaleFactors.root" );  
  TFile *systFile= TFile::Open( "/sps/atlas/a/aguerguichon/Calibration/ScaleResults/170601/EnergyScaleFactors.root" );  

  string savePath= "/sps/atlas/a/aguerguichon/Calibration/Plots/";
  string dataPath="/sps/atlas/a/aguerguichon/Calibration/DataxAOD/eosNtuples/NominalZeeSelection/";

  
  //=========================GET THE SCALE UNCERTAINTY BAND
  if (doSyst)
    {
      for (int isData= 0; isData<3; isData++)
	{
	  histName= isData? "data" :"mc";
	  for (int i=0; i<3; i++){  tmpHist[isData][i] = new TH1D ((histName+to_string(i)).c_str(), "", nBins, 80, 100) ;  } //intialize histograms for +, - and no deltaSF      
	  if (isData ==1) year = "16";
	  if (isData ==2) year = "15";
	  
	  histName= isData? "centVal_alpha_"+year :"centVal_c_1516";
	  scaleHist= static_cast<TH1D*>(scalesFile->Get(histName.c_str()) );
	  histName= isData? "totSyst_alpha" :"totSyst_c";
	  systHist= static_cast<TH1D*>( systFile->Get(histName.c_str()) ) ;
  
	  inFile=TFile::Open(( isData? dataPath+"data"+year+".root": dataPath+"mcZee.root"  ).c_str() );

	  cout<<inFile->GetName()<<endl;
	  inTree=(TTree*) inFile->Get( "CollectionTree" );
	
	  MapBranches mapBranches;
	  mapBranches.LinkTreeBranches(inTree);

	  for(int iEntry=0; iEntry<inTree->GetEntries(); iEntry++)
	    {
	      inTree->GetEntry(iEntry);
	      randN.SetSeed();
	      for(int iElec=1; iElec<3; iElec++)
		{
		  eta=mapBranches.GetDouble("el"+to_string(iElec)+"_etaCalo");
		  phi=mapBranches.GetDouble("phi"+to_string(iElec));
		  bin=scaleHist->FindFixBin(eta);
		  SF=scaleHist->GetBinContent(bin);
		  //deltaSF=sqrt( pow(histAlphaVal->GetBinError(bin),2) + pow(histAlphaSyst->GetBinContent(bin),2) );
		  deltaSF=systHist->GetBinContent(bin);
		  for (int sign=0; sign<3; sign++ )
		    {
		      if (iElec==1) VLZ[sign].SetPtEtaPhiM(0,0,0,0);
		      if (isData) pt=mapBranches.GetDouble( "pt"+to_string(iElec) ) / ( 1+SF+(sign-1)*deltaSF );
		      else pt=mapBranches.GetDouble("pt"+to_string(iElec))*( 1+(SF+(sign-1)*deltaSF)*randN.Gaus(0,1) );
		      VLElec[sign].SetPtEtaPhiM(pt, eta, phi, 0.511);//in MeV
		      VLZ[sign]+=VLElec[sign];
		      if (iElec==2) tmpHist[isData][sign]->Fill(VLZ[sign].M()/1000);
		    }//end sign
		}//end iElec
	    }//end iEntry
	      inFile->Close();

	}//end isData

      TFile outFile((savePath+"ratio.root").c_str(), "RECREATE");
      systHist= new TH1D("systTot","",nBins,80,100 );

      for(bin=1; bin<=nBins; bin++)
	{
	  errScale=pow (max( fabs(tmpHist[0][1]->GetBinContent(bin)/tmpHist[0][0]->GetBinContent(bin)) -1 ,fabs(tmpHist[0][1]->GetBinContent(bin)/tmpHist[0][2]->GetBinContent(bin)) -1 ), 2 ); //for MC
	  double maxData1= max( fabs(tmpHist[1][1]->GetBinContent(bin)/tmpHist[1][0]->GetBinContent(bin)) -1 ,fabs(tmpHist[2][1]->GetBinContent(bin)/tmpHist[2][0]->GetBinContent(bin)) -1 );
	  double maxData2= max( fabs(tmpHist[1][1]->GetBinContent(bin)/tmpHist[1][2]->GetBinContent(bin)) -1 ,fabs(tmpHist[2][1]->GetBinContent(bin)/tmpHist[2][2]->GetBinContent(bin)) -1 ) ;
	  
	  errScale+=pow ( max(maxData1, maxData2), 2 );

	  // for (int isData=0; isData<2; isData++)
	  //   {
	  
	  //     errScale+=pow (max( fabs(tmpHist[isData][1]->GetBinContent(bin)/tmpHist[isData][0]->GetBinContent(bin)) -1 ,fabs(tmpHist[isData][1]->GetBinContent(bin)/tmpHist[isData][2]->GetBinContent(bin)) -1 ), 2 );
	      // cout<<"isData: "<<isData<<"iBin: "<<bin<<"errScale: "<<errScale<<endl;
	  //}
	  systHist->SetBinError( bin, sqrt( errScale ));
	}

      systHist->Write();
      cout<<"systTot saved in "<<outFile.GetName()<<endl;
      outFile.Close();
    }

  // ============================RATIO PLOTS COSMETICS==========================//

  //  inFile=TFile::Open((savePath+"Correction_m12.root").c_str());

  //  cRatio=(TCanvas*)inFile->Get("c1");
  // TH1D *histRatio=(TH1D*)((TPad*)(cRatio->GetListOfPrimitives()->At(1)))->GetListOfPrimitives()->At(0);

  // TH1D *histData=(TH1D*)((TPad*)(cRatio->GetListOfPrimitives()->At(0)))->GetListOfPrimitives()->At(3);
  // TH1D *histMC=(TH1D*)((TPad*)(cRatio->GetListOfPrimitives()->At(0)))->GetListOfPrimitives()->At(2);

  // TH1D *histNew=(TH1D*)histRatio->Clone();

  TCanvas *newC = new TCanvas("canvas", "");
  
  TPad padUp( "padUp", "padUp", 0, 0.3, 1, 1 );
  padUp.SetTopMargin( 0.08 );
  padUp.SetBottomMargin( 0.02 );
  padUp.Draw();
  padUp.cd();


  inFile=TFile::Open("Correction_m12.root");
  //  inFile=new TFile("/sps/atlas/a/aguerguichon/Calibration/Test/Distri_m12.root");
  TH1D* histData= (TH1D*) inFile->Get("data15_corrected_m12_1");
  TH1D* histMC= (TH1D*) inFile->Get("mcZee_corrected_m12_0");
  
  histData->SetMarkerStyle(8);
  histData->SetMarkerSize(1.3);
  histData->SetMarkerColor(kBlue);
  histData->SetLineColor(kBlue);
  histMC->SetTitle("");
  histMC->GetYaxis()->SetTitle("Entries / 0.5 GeV");
  histMC->GetYaxis()->SetTitleSize(0.06);
  histMC->GetYaxis()->SetTitleOffset(0.8);
  histMC->GetYaxis()->SetLabelSize(0.06);
  histMC->SetLineColor(kBlack);
  histMC->GetXaxis()->SetLabelSize(0);
  histMC->Scale(histData->Integral()/histMC->Integral());
  histMC->GetYaxis()->SetRangeUser(0, 0.1);
  histMC->Draw("HIST");
  histData->Draw("SAME");
  ATLASLabel(0.13, 0.8, "Work in progress", 1, sizeText);
  myText(0.13, 0.7, 1,"#sqrt{s}=13 TeV, L = 3.2 (2015) + 32.9 (2016) fb^{-1}", sizeText);

  vectHist.push_back(histData);
  vectHist.push_back(histMC);

  for (unsigned int iHist=0; iHist<vectHist.size(); iHist++)
    {
      if (iHist==0) histName="Data";
      else histName="MC";
      myLineText( x, y-iHist*0.1, vectHist[iHist]->GetLineColor(), vectHist[iHist]->GetLineStyle(), "", sizeText, vectHist[iHist]->GetLineWidth(), lsize  ); 
      myMarkerText( x, y-iHist*0.1, vectHist[iHist]->GetMarkerColor(), vectHist[iHist]->GetMarkerStyle(), histName.c_str(), sizeText, vectHist[iHist]->GetMarkerSize(), lsize);    }
  

  TPad padDown( "padDown", "padDown", 0, 0, 1, 0.3 );
  padDown.SetTopMargin( 0.05 );
  padDown.SetBottomMargin( 0.3 );

  newC->cd();
  padDown.Draw();

  padDown.cd();

  TFile *fileSyst=TFile::Open((savePath+"ratio.root").c_str());
  systHist=(TH1D*)fileSyst->Get("systTot");

  TH1D* histNew=(TH1D*)histData->Clone();
  histNew->Add(histMC, -1);
  histNew->Divide(histMC);

  TH1D* histRatio = static_cast<TH1D*>(systHist->Clone());
  for ( int i=1; i<=systHist->GetNbinsX(); i++) {
    histRatio->SetBinContent(i, histNew->GetBinContent(i));
    histRatio->SetBinError( i, histNew->GetBinError(i));
  }

  systHist->SetMarkerColor(kGreen-10);
  systHist->SetFillColor(kGreen-10);
  newC->Update();

  systHist->SetStats(0);
  systHist->GetXaxis()->SetLabelSize(0.14);
  systHist->GetXaxis()->SetTitleSize(0.15);
  systHist->GetXaxis()->SetTitleOffset(0.92);
  systHist->GetXaxis()->SetTitle("m_{ee} [GeV]");

  systHist->GetYaxis()->SetTitle("(Data / MC) -1");
  systHist->GetYaxis()->SetTitleOffset(0.35);
  systHist->GetYaxis()->SetLabelSize(0.12);
  systHist->GetYaxis()->SetTitleSize(0.13);
  systHist->GetYaxis()->SetRangeUser(-0.06, 0.06);
  //  histRatio->GetYaxis()->SetRangeUser(-0.06, 0.06);
  newC->Update();

  TLine *line= new TLine(80, 0, 100, 0);
  line->SetLineColor( kBlack);
  line->SetLineStyle(3);
  systHist->Draw("E2");
  histRatio->Draw("same");
  line->Draw();
  newC->Update();

  cout<<histRatio->GetBinContent(1)<<endl;

  padDown.ls();
  
  newC->SaveAs((savePath+"Ratio.eps").c_str());
  inFile->Close();
  cout<<"Plots done."<<endl;
  return 0;



}
