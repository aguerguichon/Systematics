#include <iostream>
#include "TFile.h"
#include "TCanvas.h"
#include "PlotFunctions/SideFunctions.h"
#include <boost/program_options.hpp>
#include <fstream>
#include "PlotFunctions/AtlasStyle.h"
#include "PlotFunctions/AtlasUtils.h"
#include "PlotFunctions/AtlasLabels.h"


using std::cout;
using std::endl;
using std::vector;
namespace po = boost::program_options;
using std::ifstream;


int main( int argc, char* argv[] ) {
  po::options_description desc("LikelihoodProfiel Usage");

  //define all options in the program
  desc.add_options()
    ("help", "Display this help message")
    ;

  // Create a map vm that contains options and all arguments of options       
  po::variables_map vm;
  po::store(po::command_line_parser(argc, argv).options(desc).style(po::command_line_style::unix_style ^ po::command_line_style::allow_short).run(), vm);
  po::notify(vm);
  
  if (vm.count("help")) {cout << desc; return 0;}
  //########################################
  SetAtlasStyle();
  vector<double> frameUp = { -2.7, 2.7, 0, 0};
  vector<double> frameDown = { frameUp[0], frameUp[1], 0, 0};
  TFile *inFileSyst = new TFile( "/sps/atlas/c/cgoudet/Calibration/ScaleResults/160519/EnergyScaleFactors.root" );
  TH1::AddDirectory(kFALSE);
  bool doReduce = 0;
  string year="16";

  for ( unsigned int iScale =0; iScale < 2; iScale++ ) {
    string var = iScale ?  "_c" : "";
    TFile *inFileICHEP= new TFile( ("/sps/atlas/c/cgoudet/Calibration/ScaleResults/160519/DataOff_13TeV_25ns"+var+".root").c_str() );
    TFile *inFile = new TFile( ("/sps/atlas/a/aguerguichon/Calibration/PreRec/Results/DataOff_13TeV_"+year+var+".root").c_str() );
    var = iScale ?  "c" : "alpha";
    TH1D* histScale = (TH1D*) inFile->Get( ("measScale_"+var).c_str() );
    TH1D* histSyst = (TH1D*) inFileSyst->Get( ("totSyst_"+var).c_str() );
    TH1D* histICHEP= (TH1D*) inFileICHEP->Get( ("measScale_"+var).c_str() );

    if ( doReduce )  {
      const TArrayD *binning = histScale->GetXaxis()->GetXbins();
      vector<double> vectBin;
      for ( int i=1; i<binning->GetSize(); i++ ) vectBin.push_back( binning->GetAt(i) );
      vectBin.pop_back();

      TH1D *dumScale = new TH1D( "dumScale", "dumScale", vectBin.size()-1, &vectBin[0] );
      TH1D *dumSyst = new TH1D( "dumSyst", "dumSyst", vectBin.size()-1, &vectBin[0] );

      for ( int i = 1; i<=dumScale->GetNbinsX(); ++i ) {
	dumScale->SetBinContent( i, histScale->GetBinContent(i+1) );
	dumSyst->SetBinContent( i, histSyst->GetBinContent(i+1) );
	dumScale->SetBinError( i, histScale->GetBinError(i+1) );
	dumSyst->SetBinError( i, histSyst->GetBinError(i+1) );
      }

      histScale=dumScale;
      histSyst = dumSyst;
    }


    histScale->SetLineColor( kRed );
    histScale->SetMarkerColor(kRed);
    histScale->SetMarkerStyle(20);

    histICHEP->SetLineColor( kBlue );
    histICHEP->SetMarkerColor( kBlue );
    histICHEP->SetMarkerStyle(20);
    
    histSyst->SetLineColor( kBlue );
    histSyst->SetLineWidth( 2.5 );
    histSyst->SetMarkerColor( kBlue );
    histSyst->SetMarkerStyle(20);

    TH1D *histScaleTot = (TH1D*) histICHEP->Clone();
    histScaleTot->SetFillColor( kBlue-9 );
    TH1D* histStat = (TH1D*) histScale->Clone();
    histStat->ResetAttLine();
    histStat->ResetAttMarker();
    //    histStat->SetLineStyle(7);
    histStat->SetLineWidth(1.5);
    histStat->SetLineColor( kRed );
    for ( int iBin = 1; iBin<histStat->GetNbinsX()+1; iBin++ ) {
      histStat->SetBinContent( iBin, histScale->GetBinError( iBin ) );
      histStat->SetBinError( iBin, 0 );
      //      histICHEP->SetBinError( iBin, histSyst->GetBinContent(iBin) );
      histScaleTot->SetBinError( iBin, histSyst->GetBinContent(iBin) );
      
      //      histScaleTot->SetBinError( iBin, sqrt( histStat->GetBinContent(iBin)*histStat->GetBinContent(iBin)+ histSyst->GetBinContent(iBin)*histSyst->GetBinContent(iBin) ) );
      //      cout << histSyst->GetBinContent(iBin) << " " << histStat->GetBinContent(iBin) << " " << sqrt( histStat->GetBinContent(iBin)*histStat->GetBinContent(iBin)+ histSyst->GetBinContent(iBin)*histSyst->GetBinContent(iBin) ) << " " ;
      histSyst->SetBinContent( iBin, histScaleTot->GetBinError(iBin) );
    }

    if ( iScale || doReduce ) {
      histSyst->Scale( 1e3 );
      histStat->Scale( 1e3 );
    }    

    frameUp[2]   = iScale ? 0     : -0.03;
    frameUp[3]   = iScale ? 0.05 : ( doReduce ? 0.06 : 0.1 );
    frameDown[3] = iScale ? 7 : ( !doReduce ? 2e-2: 6 );
    frameDown[2] = ( !iScale && !doReduce ) ? 1e-4 : 0;

    TCanvas *canvas = new TCanvas();
    TPad padUp( "padUp", "padUp", 0, 0.3, 1, 1 );
    padUp.SetBottomMargin( 0.03 );
    padUp.Draw();
    padUp.cd();
    TH1F* dumUp = padUp.DrawFrame( frameUp[0], frameUp[2], frameUp[1], frameUp[3] );
    dumUp->GetYaxis()->SetTitleSize( 0.09);
    dumUp->GetYaxis()->SetLabelSize( 0.06);
    dumUp->GetYaxis()->SetTitleOffset( 0.6);
    dumUp->GetXaxis()->SetLabelSize( 0 );
    TPad padDown( "padDown", "padDown", 0, 0, 1, 0.3 );
    padDown.SetTopMargin( 0.04 );
    padDown.SetBottomMargin( 0.2 );
    canvas->cd();
    padDown.Draw();
    padDown.cd();
    TH1F* dumDown = padDown.DrawFrame( frameDown[0], frameDown[2], frameDown[1], frameDown[3] );
    dumDown->GetYaxis()->SetLabelSize( 0.09 );
    dumDown->GetYaxis()->SetTitleSize( 0.12 );
    dumDown->GetYaxis()->SetTitleOffset( 0.4 );
    //xAxis
    dumDown->GetXaxis()->SetTitle( "#eta" );
    dumDown->GetXaxis()->SetTitleSize( 0.12 );
    dumDown->GetXaxis()->SetLabelSize( 0.12 );
    dumDown->GetXaxis()->SetTitleOffset( 0.45 );


    string legendVar = iScale ? "c'" : "#alpha";
    dumUp->GetYaxis()->SetTitle( legendVar.c_str() );   
    legendVar = "#delta"+legendVar;
    if ( frameDown[3] > 1 ) legendVar+= " (10^{-3})";
    dumDown->GetYaxis()->SetTitle( legendVar.c_str() );

    padUp.cd();
    histScaleTot->Draw("E2, same");
    histICHEP->Draw("same");
    histScale->Draw("SAME");
    
    double x = 0.4;
    double y = 0.75;
    double s = 0.08;
    double lsize = 0.03;

    myLineText( x, y, histScale->GetLineColor(), histScale->GetLineStyle(), "", s, histScale->GetLineWidth(), lsize ); 
    myMarkerText( x, y, histScale->GetMarkerColor(), histScale->GetMarkerStyle(), "2016", s, histScale->GetMarkerSize(), lsize ); 

    myBoxText( x, y-0.1, histScaleTot->GetFillColor(), "", s, lsize ); 
    myLineText( x, y-0.1, histICHEP->GetLineColor(), histICHEP->GetLineStyle(), "", s, histICHEP->GetLineWidth(), lsize ); 
    myMarkerText( x, y-0.1, histICHEP->GetMarkerColor(), histICHEP->GetMarkerStyle(), "ICHEP recommandations", s, histICHEP->GetMarkerSize(), lsize ); 


    padDown.cd();
    histSyst->Draw("same");
    histStat->Draw("same");
    cout<<"bin"<<histStat->GetBinContent(5)<<endl;
    if ( !iScale && !doReduce ) padDown.SetLogy(1);

    myLineText( 0.48, 0.85, histSyst->GetLineColor(), histSyst->GetLineStyle(), "ICHEP Syst." ,0.11, histSyst->GetLineWidth() ); 
    myLineText( 0.48, 0.75, histStat->GetLineColor(), histStat->GetLineStyle(), "2016 Stat.", 0.11, histStat->GetLineWidth() ); 

    canvas->cd();
    //    ATLASLabel( 0.16, 0.9, "Work in progress", 1, 0.06 );
    //myText( 0.5, 0.9, 1, "#sqrt{s} = 13 TeV, L = 33.9 fb^{-1}", 0.05 );
    ATLASLabel( 0.16, 0.9, "Internal", 1, 0.06 );
    myText( 0.5, 0.9, 1, "#sqrt{s} = 13 TeV, L = 33.9 fb^{-1}", 0.05 );

    string suffix = doReduce ? "_reduced" : "";
    suffix+=year;
    canvas->SaveAs( ("/sps/atlas/a/aguerguichon/Calibration/Plots/ScaleFactors_"+var+suffix+".eps").c_str() );
  }



  //Comparison Run1/run2


  return 0;

}
