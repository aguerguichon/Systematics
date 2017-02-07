#include <iostream>
#include <istream>
#include <fstream>
#include <map>
#include <vector>
#include <string>
#include <sstream>
#include <stdio.h>
#include <time.h>
#include <boost/program_options.hpp>
#include <boost/multi_array.hpp>

#include "TH1.h"
#include "TFile.h"
#include "TTree.h"
#include "TBranch.h"
#include "TCanvas.h"
#include "TProfile.h"
#include "TLine.h"

#include "PlotFunctions/MapBranches.h"
#include "PlotFunctions/DrawPlot.h"
#include "PlotFunctions/SideFunctions.h"
#include "PlotFunctions/AtlasStyle.h"
#include "PlotFunctions/AtlasUtils.h"
#include "PlotFunctions/AtlasLabels.h"

using namespace ChrisLib;
using namespace std;


int main()
{

  TH1::AddDirectory(kFALSE);
  SetAtlasStyle();
  string path= "/sps/atlas/a/aguerguichon/Calibration/DataxAOD/";
  string savePath= "/sps/atlas/a/aguerguichon/Calibration/Plots/";
  string fileName, pattern, name, yearHist;  
  vector <string> vectYear (2);
  vector <string> vectOpt;
  vector <TH1*> vectProf; 
  vector <TH1*> vectHist; 
  TFile *inFile=0;
  TTree *inTree=0;
  TProfile *prof=0;
  TCanvas *c1=0;
  TH1D *histTmp=0;

  unsigned long long timeMin=1e11;
  unsigned int timeMax=0;
  double meanZDistri=0;
  double m12, muPU, timeStamp, weight; 
  double counter;
  double nZ2015=0;

  float sizeText=0.05;
  float x=0.3;
  float y=0.73;
  float lsize = 0.04;
  
  vector <double> mean (2);

  bool isMuPU=0;
  DrawOptions drawOpt;

  if (isMuPU) name=savePath+"Mu";
  else name=savePath+"Time";
  vectYear[0]= "15";
  vectYear[1]= "16";

  for (unsigned int year=0; year<vectYear.size(); year++)
    {
      counter=0;
      mean[year]=0;
      pattern="Data"+vectYear[year]+"_13TeV_Zee_noGain_Lkh1";
      if (isMuPU) prof = new TProfile ("hprof", "", 50, 0, 50 );
      for (int iFile=0; iFile<50; iFile++) 
  	{
  	  fileName=path+pattern+"/"+pattern+"_"+to_string(iFile)+".root";
  	  inFile= TFile::Open(fileName.c_str());

  	  if (!inFile) break;
  	  cout<<"File: "<<fileName.c_str()<<endl;
  	  inTree= (TTree*) inFile->Get( (pattern+"_"+to_string(iFile)+"_selectionTree").c_str() );

  	  MapBranches mapBranches;
  	  mapBranches.LinkTreeBranches(inTree, 0, {"m12", "timeStamp", "muPU", "eta_calo_1", "eta_calo_2"});


  	  for (unsigned int iEntry=0; iEntry<inTree->GetEntries(); iEntry++)
  	    {
  	      inTree->GetEntry(iEntry);

  	      m12=mapBranches.GetDouble("m12");
  	      muPU=mapBranches.GetDouble("muPU");
  	      timeStamp=mapBranches.GetLongLong("timeStamp");

  	      if (m12<80 || m12>100) continue;
  	      if (year==0){meanZDistri+=m12; nZ2015++;}
  	      //if ( fabs( mapBranches.GetDouble("eta_calo_1") )>1.37 || fabs( mapBranches.GetDouble("eta_calo_2") )>1.37) continue;
  	      //if ( mapBranches.GetDouble("eta_calo_1") <1.55 || mapBranches.GetDouble("eta_calo_2") <1.55) continue;
  	      if ( mapBranches.GetDouble("eta_calo_1") > -1.55 || mapBranches.GetDouble("eta_calo_2") > -1.55) continue;
	      //    cout<<mapBranches.GetDouble("eta_calo_1")<<" "<<mapBranches.GetDouble("eta_calo_2")<<endl;
  	      mean[year]+=m12;
  	      counter++;
  	      if (isMuPU) prof->Fill(muPU, m12);
  	      else
  		{
  		  if (timeStamp < timeMin) timeMin=timeStamp;
  		  if (timeStamp > timeMax) timeMax=timeStamp;
  		}
	  
  	    }
  	  inFile->Close();
  	}//end iFile
      if (year==0) meanZDistri/=nZ2015;
      mean[year]/=(counter*meanZDistri);
      cout<<"year: "<<year<<" mean: "<<mean[year]<<endl;

      if (isMuPU)
  	{
  	  prof->Scale(1/meanZDistri);
  	  if (year==0) 
  	    {
  	      prof->SetMarkerStyle(8);
  	      prof->SetMarkerSize(1.3);
  	    }
  	  else
  	    {
  	      prof->SetMarkerStyle(25);
  	      prof->SetMarkerSize(1.3);
  	    }
  	  vectProf.push_back(prof);
  	} 
   }//end year


  if (isMuPU)
    {
      vectOpt.push_back("xTitle= #mu");
      vectOpt.push_back("yTitle= m_{ee} / <m_{ee}(2015)>");
      vectOpt.push_back("rangeUserY= 0.998 1.005");
      vectOpt.push_back("rangeUserX= 5 46");
      vectOpt.push_back("line=1");
      vectOpt.push_back("drawStyle=4");
      vectOpt.push_back("extension=root");
      drawOpt.FillOptions(vectOpt);
      drawOpt.AddOption("outName", name);
      drawOpt.Draw(vectProf);
      //      DrawPlot(vectProf, name, vectOpt);
    
      //Cosmetics
      TFile *muFile=TFile::Open((savePath+"Mu.root").c_str());
      c1=(TCanvas*)muFile->Get("c1");
      c1->cd();
      
      ATLASLabel(0.22, 0.87, "Internal", 1, 0.06);
      myText(0.22, 0.79, 1,"#sqrt{s}=13 TeV, L = 3.1 (2015) + 33.9 (2016) fb^{-1}", sizeText);
      //      myText(0.22, 0.79, 1,"#sqrt{s}=13 TeV, L = 33.9 fb^{-1}", sizeText);
      //myText(0.22, 0.79, 1,"2016 data", sizeText);

      histTmp=(TH1D*)c1->GetListOfPrimitives()->At(4);
      histTmp->SetMarkerStyle(25);
      histTmp->SetMarkerColor(kRed);
      histTmp->SetLineColor(kRed);
      histTmp->SetMarkerSize(1);
      yearHist="2016";
      myLineText( x, y, histTmp->GetLineColor(), histTmp->GetLineStyle(), "", sizeText, histTmp->GetLineWidth(), lsize  ); 
      myMarkerText( x, y, histTmp->GetMarkerColor(), histTmp->GetMarkerStyle(), yearHist.c_str(), sizeText, histTmp->GetMarkerSize(), lsize); 
          
      histTmp =(TH1D*)c1->GetListOfPrimitives()->At(2);  
      histTmp->SetMarkerStyle(8);
      histTmp->SetMarkerSize(1);
      yearHist="2015";
      myLineText( x, y-0.08, histTmp->GetLineColor(), histTmp->GetLineStyle(), "", sizeText, histTmp->GetLineWidth(), lsize  ); 
      myMarkerText( x, y-0.08, histTmp->GetMarkerColor(), histTmp->GetMarkerStyle(), yearHist.c_str(), sizeText, histTmp->GetMarkerSize(), lsize); 
      
      histTmp->GetYaxis()->SetTitleSize(0.05);
      histTmp->GetYaxis()->SetTitleOffset(1.10);
      histTmp->GetYaxis()->SetLabelSize(0.04);
      histTmp->GetXaxis()->SetTitleSize(0.5);
      histTmp->GetXaxis()->SetTitleOffset(1.20);
      histTmp->GetXaxis()->SetLabelSize(0.05);
      c1->Draw();
      c1->SaveAs((savePath+"Mee_mu.pdf").c_str());

      muFile->Close();
      delete muFile;
    }
  
  else
    {
      unsigned int nBins= (timeMax-timeMin)/(7*24*60*60); //nb of weeks
      for (unsigned int year=0; year<vectYear.size(); year++)
	{
	  prof = new TProfile ("hprof", "", nBins, timeMin, timeMax);
	  pattern="Data"+vectYear[year]+"_13TeV_Zee_Lkh1";
	  for (int iFile=0; iFile<10; iFile++)
	    {
	      fileName=path+pattern+"/"+pattern+"_"+to_string(iFile)+".root";
	      inFile= TFile::Open(fileName.c_str());
	      if (!inFile) break;

	      inTree= (TTree*) inFile->Get( (pattern+"_"+to_string(iFile)+"_selectionTree").c_str() );
	      MapBranches mapBranches;
	      mapBranches.LinkTreeBranches(inTree, 0, {"m12", "timeStamp", "eta_calo_1", "eta_calo_2"});
	      for (unsigned int iEntry=0; iEntry<inTree->GetEntries(); iEntry++)
		{
		  inTree->GetEntry(iEntry);
	
		  m12=mapBranches.GetDouble("m12");
		  if (m12<80 || m12>100) continue;
		  //if ( fabs( mapBranches.GetDouble("eta_calo_1") )>1.37 || fabs( mapBranches.GetDouble("eta_calo_2") )>1.37) continue;
		  //if ( mapBranches.GetDouble("eta_calo_1") <1.55 || mapBranches.GetDouble("eta_calo_2") <1.55) continue;
		  if ( mapBranches.GetDouble("eta_calo_1") > -1.55 || mapBranches.GetDouble("eta_calo_2") > -1.55) continue;
		  prof->Fill( mapBranches.GetLongLong("timeStamp"), m12);
		}
	    }//end iFile
	  prof->Scale(1/meanZDistri);
	  vectProf.push_back(prof);
	}//end year
 
      for ( unsigned int iProf=0; iProf< vectProf.size(); iProf++)
	{
	  histTmp=(TH1D*)vectProf[iProf];
	  for (int iBin=1; iBin<= vectProf[0]->GetXaxis()->GetNbins(); iBin++)
	    {
	      histTmp->GetXaxis()->SetBinLabel( iBin, ConvertEpochToDate(histTmp->GetBinCenter(iBin)).c_str() );
	    }
	}

      CleanHist(vectProf, 0);

      for ( unsigned int iProf=0; iProf< vectProf.size(); iProf++)
	{
	  for (int iBin=1; iBin<= vectProf[iProf]->GetXaxis()->GetNbins(); iBin++)
	    {
	      if (iBin%4!=0 && iBin!=1 && iBin!=vectProf[iProf]->GetXaxis()->GetNbins())  vectProf[iProf]->GetXaxis()->SetBinLabel( iBin, " " );
	    }
	}
      vectOpt.push_back("xTitle= Date (day/month/year) ");
      vectOpt.push_back("yTitle= m_{ee} / <m_{ee}(2015)>");
      //      vectOpt.push_back("rangeUserY= 0.998 1.004");
      vectOpt.push_back("line=1");
      vectOpt.push_back("extension=root");
      drawOpt.FillOptions(vectOpt);
      drawOpt.AddOption("outName", name);
      drawOpt.Draw(vectProf);
      //DrawPlot(vectProf, name, vectOpt);


      //Cosmetics
      TFile *timeFile=TFile::Open((savePath+"Time.root").c_str());
      c1=(TCanvas*)timeFile->Get("c1");
      c1->cd();
      histTmp =(TH1D*)c1->GetListOfPrimitives()->At(2);

      ATLASLabel(0.22, 0.87, "Internal", 1, 0.06);
      myText(0.22, 0.79, 1,"#sqrt{s}=13 TeV, L = 3.2 (2015) + 33.9 (2016) fb^{-1}", sizeText);
      myText(0.43, 0.73-0.4, 1,"|#eta| < 1.37 (EC(C)-EC(C) events)", 0.05);
      histTmp->SetLineColor(kRed);
      histTmp->SetMarkerColor(kRed);
      histTmp =(TH1D*)c1->GetListOfPrimitives()->At(0);
      histTmp->GetYaxis()->SetTitleSize(0.05);
      histTmp->GetYaxis()->SetTitleOffset(1.10);
      histTmp->GetYaxis()->SetLabelSize(0.04);

      histTmp->GetXaxis()->SetTitleSize(0.05);
      histTmp->GetXaxis()->SetTitleOffset(1.20);
      histTmp->GetXaxis()->SetLabelSize(0.05);
     
      TLine *line= new TLine (histTmp->GetBinCenter(1), mean[0], histTmp->GetBinCenter(histTmp->GetNbinsX()), mean[0]);
      line->SetLineColor(kBlack);
      line->Draw();
      line= new TLine (histTmp->GetBinCenter(1), mean[1], histTmp->GetBinCenter(histTmp->GetNbinsX()) , mean[1]);
      line->SetLineColor(kRed);
      line->Draw();

      
      c1->SaveAs((savePath+"Mee_time_ECC.root").c_str());
      timeFile->Close();
      delete timeFile;


    }//end time plot

  cout<<"End of plotting"<<endl;
  delete inFile;
  return 0;
}





