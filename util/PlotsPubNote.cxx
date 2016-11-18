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
#include "PlotFunctions/AtlasStyle.h"
#include "PlotFunctions/AtlasUtils.h"
#include "PlotFunctions/AtlasLabels.h"

using namespace ChrisLib;
using namespace std;
using std::map;

int main()
{

  SetAtlasStyle();
  unsigned int nEntries, bin, nBins;
  map <string, double> mapDouble;
  map <string, long long int> mapLongLong;
  map <string, TH1D*> mapHist;
  vector <string> vectNElec;
  vector <TLorentzVector> VLElec(2);
  double SF, phi, eta, pt, deltaSF;
  TLorentzVector VLZ;
  string type;
  TFile *inFile=0;
  TTree *inTree=0;
  TCanvas *cRatio=0;
  vector <string> inFileNames;
  string fileName, pattern, year, letter, lumi;
  TRandom randN;
  TH1D* systHist=0;
  bool isEndcap=0;
  double etaEC=1.55;

  double x = 0.7;
  double y = 0.82;
  double lsize = 0.04;
  float sizeText=0.065;

  vector <TH1D*> vectHist;
  string typeHist;

  vectNElec.push_back("1");
  vectNElec.push_back("2");

  TH1::AddDirectory(kFALSE);

  TFile *scalesFile= TFile::Open( "/sps/atlas/c/cgoudet/Calibration/ScaleResults/160519/EnergyScaleFactors.root" );  
  TH1D *histAlphaVal= (TH1D*) scalesFile->Get("centVal_alpha");
  TH1D *histAlphaSyst= (TH1D*) scalesFile->Get("totSyst_alpha");
  TH1D *histCVal= (TH1D*) scalesFile->Get("centVal_c");
  TH1D *histCSyst= (TH1D*) scalesFile->Get("totSyst_c");
  nBins= 20;

  string savePath= "/afs/in2p3.fr/home/a/aguergui/public/Calibration/PlotFunctions/PubNote/";

  year="2015";
  letter="b";
  lumi="3.2";
  
  // year="2016";
  // letter="c";
  // lumi="2.7";

  // for (unsigned int isData= 0; isData<2; isData++)
  //   {
  //     if (isData==0)
  // 	{
  // 	  pattern="Data_13TeV_Zee_"+year+"_Lkh1_scaled";
  // 	  inFile= TFile::Open( ( "/sps/atlas/a/aguerguichon/Calibration/DataxAOD/"+pattern+"/"+pattern+"_0.root").c_str() );
  // 	  inTree= (TTree*) inFile->Get( (pattern+"_0_selectionTree").c_str() );
  // 	  cout<<(pattern+"_selectionTree").c_str()<<endl;
  // 	  type= "Data";

  // 	  MapBranches mapBranches;
  // 	  mapBranches.LinkTreeBranches(inTree);

  // 	  nEntries= inTree->GetEntries();
  
  // 	  cout<<"nEntries: "<<nEntries<<endl;
     
  // 	  for (unsigned int iEntry=0; iEntry<nEntries; iEntry++)
  // 	    {
  // 	      inTree->GetEntry(iEntry);
  // 	      mapDouble=mapBranches.GetMapDouble();
  // 	      mapLongLong=mapBranches.GetMapLongLong();
      
  // 	      for (int signDelta=-1; signDelta<2; signDelta++)
  // 		{
  // 		  for (unsigned int iElec=0; iElec<vectNElec.size(); iElec++)
  // 		    {
  // 		      eta=mapDouble.at("eta_calo_"+vectNElec[iElec]);
  // 		      phi=mapDouble.at("phi_"+vectNElec[iElec]);
  // 		      bin= histAlphaVal->FindFixBin(eta);
  // 		      SF=histAlphaVal->GetBinContent(bin);
  // 		      //deltaSF=sqrt( pow(histAlphaVal->GetBinError(bin),2) + pow(histAlphaSyst->GetBinContent(bin),2) );
  // 		      deltaSF=histAlphaSyst->GetBinContent(bin);
  // 		      pt=mapDouble.at("pt_"+vectNElec[iElec])/(1+SF+signDelta*deltaSF);
  // 		      VLElec[iElec].SetPtEtaPhiM(pt, eta, phi, 0.511);//in MeV
  // 		    }
  // 		  VLZ=VLElec[0]+VLElec[1];
  // 		  string histName= type+to_string(signDelta);

  // 		  if (mapHist.count(histName)==0) {mapHist.insert( make_pair(histName, new TH1D(histName.c_str(), "", nBins, 80, 100)) );}
  // 		  mapHist[histName]->Fill(VLZ.M()/1000); 

  // 		}
  // 	    }
  // 	  inFile->Close();

  // 	}

  //     else //for MC
  // 	{
  // 	  type="MC";
  // 	  pattern="MC_13TeV_Zee_2015"+letter+"_Lkh1_scaled";
  // 	  for (unsigned int i=0; i<100; i++)
  // 	    {
  // 	      fileName= "/sps/atlas/a/aguerguichon/Calibration/DataxAOD/"+pattern+"/"+pattern+"_"+to_string(i)+".root";
  // 	      inFile= TFile::Open( fileName.c_str() );
  // 	      if (!inFile) {break;}
  // 	      else inFileNames.push_back(fileName);
  // 	      inFile->Close();
  // 	    }

  // 	  for (unsigned int iFile=0; iFile<inFileNames.size(); iFile++)
  // 	    {
	      
  // 	      inFile= TFile::Open( inFileNames[iFile].c_str() );
  // 	      cout<<(pattern+"_"+to_string(iFile)+"_selectionTree").c_str()<<endl;
  // 	      inTree= (TTree*) inFile->Get( (pattern+"_"+to_string(iFile)+"_selectionTree").c_str() );
	      
  // 	      MapBranches mapBranches;
  // 	      mapBranches.LinkTreeBranches(inTree);

  // 	      nEntries= inTree->GetEntries();

  // 	      cout<<"nEntries: "<<nEntries<<endl;

  // 	      for (unsigned int iEntry=0; iEntry<nEntries; iEntry++)
  // 		{
  // 		  inTree->GetEntry(iEntry);
  // 		  mapDouble=mapBranches.GetMapDouble();
  // 		  mapLongLong=mapBranches.GetMapLongLong();

  // 		  for (int signDelta=-1; signDelta<2; signDelta++)
  // 		    {
  // 		      randN.SetSeed();
  // 		      for (unsigned int iElec=0; iElec<vectNElec.size(); iElec++)
  // 			{
  // 			  eta=mapDouble.at("eta_calo_"+vectNElec[iElec]);
  // 			  phi=mapDouble.at("phi_"+vectNElec[iElec]);
  // 			  bin= histCVal->FindFixBin(eta);
  // 			  SF=histCVal->GetBinContent(bin);
  // 			  //deltaSF=sqrt( pow(histCVal->GetBinError(bin),2) + pow(histCSyst->GetBinContent(bin),2) );
  // 			  deltaSF=histCSyst->GetBinContent(bin);
  // 			  pt=mapDouble.at("pt_"+vectNElec[iElec])*( 1+(SF+signDelta*deltaSF)*randN.Gaus(0,1) );
  // 			  VLElec[iElec].SetPtEtaPhiM(pt, eta, phi, 0.511);//in MeV
  // 			}
  // 		      VLZ=VLElec[0]+VLElec[1];
  // 		      string histName= type+to_string(signDelta);

  // 		      if (mapHist.count(histName)==0) {mapHist.insert( make_pair(histName, new TH1D(histName.c_str(), "", nBins, 80, 100)) );}
  // 		      mapHist[histName]->Fill(VLZ.M()/1000); 

  // 		    }
  // 		}

  // 	      inFile->Close();
  // 	    }

  // 	}
  //   }//end data

  // TFile outFile((savePath+"histNotePub_"+year+".root").c_str(), "RECREATE");
  // systHist= new TH1D("systTot","",nBins,80,100 );
  // for (auto it: mapHist)
  //   {
  //     it.second->Write();
  //   }

  // double maxC, maxAlpha;

           
  // for(bin=1; bin<=nBins; bin++)
  //   {
  //     maxC=max( abs(mapHist["MC0"]->GetBinContent(bin)/mapHist["MC-1"]->GetBinContent(bin)),abs(mapHist["MC0"]->GetBinContent(bin)/mapHist["MC1"]->GetBinContent(bin)) );
  //     maxAlpha=max( abs(mapHist["Data0"]->GetBinContent(bin)/mapHist["Data-1"]->GetBinContent(bin)), abs(mapHist["Data0"]->GetBinContent(bin)/mapHist["Data1"]->GetBinContent(bin)) );
  //     //      cout<<maxC<<" alpha: "<<maxAlpha<<" q sum "<<sqrt(pow(maxAlpha,2)+pow(maxC,2))<<endl;

  //     systHist->SetBinError( bin, sqrt(pow(maxAlpha-1,2)+pow(maxC-1,2)));
  //   }

  // systHist->Write();

  // outFile.Close();
  

  //============================RATIO PLOTS==========================//

  // TFile *fileSyst=TFile::Open((savePath+"histNotePub_"+year+".root").c_str());
  // systHist=(TH1D*)fileSyst->Get("systTot");

  // inFile=TFile::Open((savePath+"MCDataRatio_"+year+"_m12.root").c_str());
  // cRatio=(TCanvas*)inFile->Get("c1");
  // TH1D *histRatio=(TH1D*)((TPad*)(cRatio->GetListOfPrimitives()->At(1)))->GetListOfPrimitives()->At(0);

  // TH1D *histData=(TH1D*)((TPad*)(cRatio->GetListOfPrimitives()->At(0)))->GetListOfPrimitives()->At(3);
  // TH1D *histMC=(TH1D*)((TPad*)(cRatio->GetListOfPrimitives()->At(0)))->GetListOfPrimitives()->At(2);

  // TH1D *histNew=(TH1D*)histRatio->Clone();

  // TCanvas *newC = new TCanvas("canvas", "");
  
  // TPad padUp( "padUp", "padUp", 0, 0.3, 1, 1 );
  // padUp.SetTopMargin( 0.08 );
  // padUp.SetBottomMargin( 0.02 );
  // TPad padDown( "padDown", "padDown", 0, 0, 1, 0.3 );
  // padDown.SetTopMargin( 0.05 );
  // padDown.SetBottomMargin( 0.3 );

  // newC->Divide(1, 2);
  // padUp.Draw();
  // newC->cd();
  // padDown.Draw();
  // padUp.cd();

  

  // histData->SetMarkerStyle(8);
  // histData->SetMarkerSize(1.3);
  // histData->SetMarkerColor(kBlue);
  // histData->SetLineColor(kBlue);
  // histMC->SetTitle("");
  // histMC->GetYaxis()->SetTitle("Entries / GeV");
  // histMC->GetYaxis()->SetTitleSize(0.06);
  // histMC->GetYaxis()->SetTitleOffset(0.8);
  // histMC->GetYaxis()->SetLabelSize(0.06);
  // histMC->SetLineColor(kBlack);
  // histMC->GetXaxis()->SetLabelSize(0);
  // histMC->Scale(histData->Integral()/histMC->Integral());
  // histMC->Draw("HIST");
  // histData->Draw("SAME");
  
  // ATLASLabel(0.15, 0.8, "Preliminary", 1, sizeText);
  // myText(0.15, 0.7, 1,("#sqrt{s}=13 TeV, L = "+lumi+" fb^{-1}").c_str(), sizeText);

  // myText(0.15, 0.6, 1, (year+" data").c_str(), sizeText);
  // if (isEndcap) myText(0.15, 0.5, 1, "|#eta| > 1.55", sizeText);
  
  
  // vectHist.push_back(histData);
  // vectHist.push_back(histMC);

  // for (unsigned int iHist=0; iHist<vectHist.size(); iHist++)
  //   {
  //     if (iHist==0) typeHist="Calibrated data";
  //     else typeHist="Corrected MC";
  //     myLineText( x, y-iHist*0.1, vectHist[iHist]->GetLineColor(), vectHist[iHist]->GetLineStyle(), "", sizeText, vectHist[iHist]->GetLineWidth(), lsize  ); 
  //     myMarkerText( x, y-iHist*0.1, vectHist[iHist]->GetMarkerColor(), vectHist[iHist]->GetMarkerStyle(), typeHist.c_str(), sizeText, vectHist[iHist]->GetMarkerSize(), lsize);    }
  
  // padDown.cd();

  // systHist->SetLineColorAlpha(0,0);
  // systHist->SetMarkerColorAlpha(0,0);
  // systHist->SetFillColor(kGreen-10);
  // systHist->Draw("E2");

  // systHist->SetStats(0);
  // systHist->GetXaxis()->SetLabelSize(0.14);
  // systHist->GetXaxis()->SetTitleSize(0.15);
  // systHist->GetXaxis()->SetTitleOffset(0.92);
  // systHist->GetXaxis()->SetTitle("m_{ee} [GeV]");

  // systHist->GetYaxis()->SetTitle("(Data / MC) -1");
  // systHist->GetYaxis()->SetTitleOffset(0.35);
  // systHist->GetYaxis()->SetLabelSize(0.12);
  // systHist->GetYaxis()->SetTitleSize(0.13);
  // systHist->GetYaxis()->SetRangeUser(-0.052, 0.052);
  // if (isEndcap)   systHist->GetYaxis()->SetRangeUser(-0.11, 0.11);
    
  // TLine *line= new TLine(80, 0, 100, 0);
  // line->SetLineColor( kBlack);
  // line->SetLineStyle(3);
  // line->Draw();

  // histNew->Draw("SAME");
 
  // if (isEndcap)   newC->SaveAs((savePath+"Ratio_"+year+"_EC.pdf").c_str());
  // else   newC->SaveAs((savePath+"Ratio_"+year+".pdf").c_str());


  // //========================STABILITY PLOTS==========================//

  // TFile *muFile=TFile::Open((savePath+"Mu.root").c_str());
  // TCanvas *c1=(TCanvas*)muFile->Get("c1");
  // vectHist.clear(); 
  // vectHist.push_back( (TH1D*)c1->GetListOfPrimitives()->At(2) );
  // vectHist.push_back( (TH1D*)c1->GetListOfPrimitives()->At(4) );

  // c1->cd();
  // string yearHist;
  // sizeText=0.05;
  // x=0.3;
  // y=0.73;

  // for (unsigned int iHist=0; iHist<vectHist.size(); iHist++)
  //   {
  //     if (iHist == 0)
  // {
  //   vectHist[iHist]->SetMarkerStyle(8);
  //   yearHist="2015";
  // }
  //     if (iHist==1)
  // {
  //   vectHist[iHist]->SetMarkerStyle(25);
  //   vectHist[iHist]->SetMarkerColor(kRed);
  //   vectHist[iHist]->SetLineColor(kRed);
  //   yearHist="2016";
  // }

  //     //      vectHist[iHist]->GetYaxis()->SetRangeUser(0.997, 1.003);
  //     vectHist[iHist]->SetMarkerSize(1);
  //     myLineText( x, y-iHist*0.08, vectHist[iHist]->GetLineColor(), vectHist[iHist]->GetLineStyle(), "", sizeText, vectHist[iHist]->GetLineWidth(), lsize  ); 
  //     myMarkerText( x, y-iHist*0.08, vectHist[iHist]->GetMarkerColor(), vectHist[iHist]->GetMarkerStyle(), yearHist.c_str(), sizeText, vectHist[iHist]->GetMarkerSize(), lsize); 
  //     vectHist[iHist]->GetXaxis()->SetTitle("#mu");
  //     vectHist[iHist]->GetXaxis()->SetTitleSize(0.05);
  //     vectHist[iHist]->GetXaxis()->SetTitleOffset(1.20);
  //     vectHist[iHist]->GetXaxis()->SetLabelSize(0.05);
  //     vectHist[iHist]->GetYaxis()->SetTitle("m_{ee} / <m_{ee}(2015)>");
  //     vectHist[iHist]->GetYaxis()->SetLabelSize(0.04);
  //     vectHist[iHist]->GetYaxis()->SetTitleSize(0.05);
  //     vectHist[iHist]->GetYaxis()->SetTitleOffset(1.10);
  //     //cout<<"mean "<<yearHist<<": "<<vectHist[iHist]->GetMean()<<endl;
  //   }



  // ATLASLabel(0.22, 0.87, "Preliminary", 1, 0.06);
  // myText(0.22, 0.79, 1,"#sqrt{s}=13 TeV, L = 3.2 (2015) + 2.7 (2016) fb^{-1}", sizeText);
  // if (isEndcap)   myText(0.43, y, 1,"|#eta| > 1.55", sizeText);

  // c1->Update();
  // c1->SaveAs((savePath+"Mee_mu.root").c_str());

  // //  muFile->Close();



  // TFile *timeFile=TFile::Open((savePath+"Time.root").c_str());
  // c1=(TCanvas*)timeFile->Get("c1");
  // c1->cd();

  // ATLASLabel(0.22, 0.87, "Preliminary", 1, 0.06);
  // myText(0.22, 0.79, 1,"#sqrt{s}=13 TeV, L = 3.2 (2015) + 2.7 (2016) fb^{-1}", sizeText);
  // c1->SaveAs((savePath+"Mee_time.root").c_str());





  // //===================CROSS-CHECKS=========================//
  // vectHist.clear();
  // vector <string> vectType;
  // vector <TH1D*> vectHistRatio;
  // vectType.push_back("data");
  // vectType.push_back("MC");
  // for (unsigned int iType=0; iType<vectType.size(); iType++)
  //   {
  //     inFile=TFile::Open((savePath+"MCDataRatio_"+vectType[iType]+"_m12.root").c_str());
  //     cRatio=(TCanvas*)inFile->Get("c1");
  //     cRatio->Draw();
  //     TH1D *histRatio=(TH1D*)((TPad*)(cRatio->GetListOfPrimitives()->At(1)))->GetListOfPrimitives()->At(0);
  //     TH1D *hTmp=(TH1D*)histRatio->Clone();
  //     vectHistRatio.push_back(hTmp);

  //     TH1D *hist15=(TH1D*)((TPad*)(cRatio->GetListOfPrimitives()->At(0)))->GetListOfPrimitives()->At(3);
  //     TH1D *hist16=(TH1D*)((TPad*)(cRatio->GetListOfPrimitives()->At(0)))->GetListOfPrimitives()->At(2);
      
  //     histNew=(TH1D*)histRatio->Clone();

  //     newC = new TCanvas(("canvas"+vectType[iType]).c_str(), "");
  
  //     TPad padUp( "padUp", "padUp", 0, 0.3, 1, 1 );
  //     padUp.SetTopMargin( 0.08 );
  //     padUp.SetBottomMargin( 0.02 );
  //     TPad padDown( "padDown", "padDown", 0, 0, 1, 0.3 );
  //     padDown.SetTopMargin( 0.05 );
  //     padDown.SetBottomMargin( 0.3 );

  //     newC->Divide(1, 2);
  //     padUp.Draw();
  //     newC->cd();
  //     padDown.Draw();
  //     padUp.cd();

  //     float sizeText=0.065;

  //     hist15->SetMarkerStyle(8);
  //     hist15->SetMarkerSize(1);
  //     hist15->SetMarkerColor(kBlack);
  //     hist15->SetLineColor(kBlack);
  //     hist16->SetMarkerStyle(25);
  //     hist16->SetMarkerSize(1);
  //     hist16->SetMarkerColor(kRed);
  //     hist16->SetLineColor(kRed);

  //     hist16->SetTitle("");
  //     hist16->GetYaxis()->SetTitle("Entries / GeV");
  //     hist16->GetYaxis()->SetTitleSize(0.05);
  //     hist16->GetYaxis()->SetTitleOffset(0.8);
  //     hist16->GetYaxis()->SetRangeUser(0, 200e3);
  //     hist16->GetXaxis()->SetLabelSize(0);
  //     hist16->Scale(hist15->Integral()/hist16->Integral());
  //     hist16->Draw();
  //     hist15->GetYaxis()->SetRangeUser(0, 200e3);
  //     hist15->Draw("SAME");
  
  //     ATLASLabel(0.13, 0.8, "Preliminary", 1, sizeText);
  //     if (vectType[iType]!="MC") myText(0.13, 0.7, 1,"#sqrt{s}=13 TeV, L = 3.2 (2015) + 2.7 (2016) fb^{-1}", sizeText);

  //     vector <TH1D*> vectHist;
  //     string typeHist;
  //     double x = 0.22;
  //     double y = 0.6;
  //     double lsize = 0.04;
      
  //     vectHist.push_back(hist15);
  //     vectHist.push_back(hist16);

  //     for (unsigned int iHist=0; iHist<vectHist.size(); iHist++)
  // 	{
  // 	  if (iHist==0) typeHist="2015 "+vectType[iType];
  // 	  else typeHist="2016 "+vectType[iType];
  // 	  if (vectType[iType]=="MC")
  // 	    {
  // 	      if (iHist==0) typeHist="2015b "+vectType[iType];
  // 	      else typeHist="2015c "+vectType[iType];
  // 	    }
  // 	  myLineText( x, y-iHist*0.1, vectHist[iHist]->GetLineColor(), vectHist[iHist]->GetLineStyle(), "", sizeText, vectHist[iHist]->GetLineWidth(), lsize  ); 
  // 	  myMarkerText( x, y-iHist*0.1, vectHist[iHist]->GetMarkerColor(), vectHist[iHist]->GetMarkerStyle(), typeHist.c_str(), sizeText, vectHist[iHist]->GetMarkerSize(), lsize); 
  // 	}
  
  //     padDown.cd();
  //     //      histNew->SetLineColorAlpha(0,0);
  //     //      histNew->SetMarkerColorAlpha(0,0);
  //     histNew->SetStats(0);
  //     histNew->GetXaxis()->SetLabelSize(0.12);
  //     histNew->GetXaxis()->SetTitleSize(0.12);
  //     histNew->GetXaxis()->SetTitleOffset(1);
  //     histNew->GetXaxis()->SetTitle("m_{ee} [GeV]");

  //     if (vectType[iType]=="MC") histNew->GetYaxis()->SetTitle("(2015b / 2015c) -1");
  //     else histNew->GetYaxis()->SetTitle("(2015 / 2016) -1");
  //     histNew->GetYaxis()->SetTitleOffset(0.35);
  //     histNew->GetYaxis()->SetLabelSize(0.09);
  //     histNew->GetYaxis()->SetTitleSize(0.12);
  //     histNew->GetYaxis()->SetRangeUser(-0.052, 0.052);
  //     TLine *line= new TLine(80, 0, 100, 0);
  //     line->SetLineColor( kBlack);
  //     line->SetLineStyle(3);

  //     histNew->SetLineColor(kBlack);
  //     histNew->Draw();
  //     line->Draw();
  //     newC->SaveAs((savePath+"Ratio_"+vectType[iType]+".root").c_str());

  //     inFile->Close();
  //   }

  // TCanvas *ratioOfRatio= new TCanvas("ratioOfRatio","");

  // vectHistRatio[0]->Divide(vectHistRatio[1]);
  // vectHistRatio[0]->GetYaxis()->SetRangeUser(-3, 5);
  // vectHistRatio[0]->GetYaxis()->SetTitle("Data ratio / MC ratio");
  // vectHistRatio[0]->GetYaxis()->SetLabelSize(0.04);
  // vectHistRatio[0]->GetYaxis()->SetTitleSize(0.05);
  // vectHistRatio[0]->GetYaxis()->SetTitleOffset(1.1);

  // vectHistRatio[0]->GetXaxis()->SetTitle("m_{ee} [GeV]");
  // vectHistRatio[0]->GetXaxis()->SetLabelSize(0.05);
  // vectHistRatio[0]->GetXaxis()->SetTitleSize(0.05);
  // vectHistRatio[0]->GetXaxis()->SetTitleOffset(1.2);
  
  
  // line= new TLine(80, 1, 100, 1);
  // line->SetLineColor( kBlack);
  // line->SetLineStyle(3);

  // vectHistRatio[0]->Draw();
  // line->Draw();
  // ratioOfRatio->cd();
  // ATLASLabel(0.6, 0.87, "Preliminary", 1, 0.06);
  // ratioOfRatio->SaveAs((savePath+"RatioOfRatio.pdf").c_str());
  // // for (int i=1; i<21; i++)
  // //   {
  // //     cout<<i<<endl;
  // //     cout<<vectHistRatio[0]->GetBinContent(i)<<endl;
  // //   }
  



  
  // //  ===========End of program
  // delete scalesFile;
  // delete inFile, fileSyst;
  // delete cRatio;
  // delete newC;
  // delete histRatio, histData, histMC, histNew;
  // delete leg;
  // delete line;
    
  cout<<"Plots done."<<endl;
  return 0;
}
