#include <iostream>
#include <istream>
#include <fstream>
#include <map>
#include <vector>
#include <string>
#include <sstream>
#include <boost/program_options.hpp>
#include <boost/multi_array.hpp>
#include <iomanip>

#include "TMatrixD.h"
#include "TH1.h"
#include "TF1.h"
#include "TFile.h"
#include "TTree.h"
#include "TBranch.h"
#include "TCanvas.h"
#include "TString.h"
#include "TGraphErrors.h"

#include "RooDataSet.h"
#include "RooRealVar.h"
#include "RooArgSet.h"
#include "RooGaussian.h"

#include "Systematics/BiasAnalysis.h"
#include "PlotFunctions/DrawPlot.h"
#include "PlotFunctions/SideFunctionsTpp.h"
#include "PlotFunctions/SideFunctions.h"
#include "PlotFunctions/MapBranches.h"
#include "PlotFunctions/InvertMatrix.h"

using namespace ChrisLib;
using namespace std;
using namespace RooFit;
namespace po = boost::program_options;

using boost::extents;
using boost::multi_array;

using std::logic_error;
using std::runtime_error;
using std::invalid_argument;

//====================================================
//Read configuration options from a configuration file
BiasAnalysis::BiasAnalysis(string configFileName)
{  
  po::options_description configOptions("Configuration options");
  configOptions.add_options()
    ("help", "Display this help message")
    ("variablesBias", po::value<vector<string>>(&m_variablesBias)->multitoken(),"Variables to study bias")
    /*accepted variables for ConfugurationsCTree: 
      - double: sigma, errSigma, inputC, dataRMS, nOptim;
      - unsigned int: iConf, jConf, statConf, statTree, indepDistorded, indepTemplates, runNumber, nBins, bootstrap, fitMethod;
      accepted variables for scalesTree:
      - double: sigma, errSigma, inputC, dataRMS, nOptim;
      - unsigned int: iBin, statTree, indepDistorded, indepTemplates, runNumber, nBins, bootstrap, fitMethod, inversionMethod;
    */
    ("variablesStats", po::value<vector<unsigned int>>(&m_variablesStats)->multitoken(),"Variables for the csv file")
    /*0: mean, rms, errMean
     */
    ("methodStats", po::value<unsigned int>(&m_methodStats), "")
    /*0: compute mean and rms
      1: h->GetMean(), h->GetRMS()
      2: get mean given by the Gaussian fit of the histogram
      3: RooFit gauss
    */
    ("selectTree", po::value<string>(&m_inTreeName), "")
    /*ConfigurationsCTree or scalesTree
     */    
    // ("checkDistri", po::value<unsigned int>(&m_checkDistri)->default_value(0),"")
    // /*0= bias study
    //   1= errSigma distribution
    // */
    
    ("nBins", po::value<unsigned int>(&m_nBins),"")
    
    ;
    
  po::variables_map vm;
  ifstream ifs( configFileName, ifstream::in );
  po::store(po::parse_config_file(ifs, configOptions), vm);
  po::notify(vm);  

  cout<<"Configuration file "<<configFileName<<" loaded."<<endl;
  cout<<m_methodStats<<endl;
}


//==================================================
//Clear attributes
BiasAnalysis::~BiasAnalysis()
{
  m_variablesBias.clear();
  m_variablesStats.clear();

  m_mapHist.clear();
  m_mapSumX.clear();
  m_mapSumXM.clear();
  m_mapStatHist.clear();
  m_mapRooVar.clear();
  m_mapRooDataSet.clear();
  m_mapRooGauss.clear();
  m_mapCij.clear();
  m_mapErrCij.clear();
  m_mapCi.clear();
  m_mapErrCi.clear();
 
  m_inTreeName.clear();
  
  m_methodStats=-999;
  m_nBins=-999;
  
  cout<<"Attributes cleaned."<<endl;
}

//===================================================

void BiasAnalysis::SelectVariables(vector <string> dataFiles)
{
  cout<<"BiasAnalysis::SelectVariables"<<endl;

  map <string, RooArgSet*> mapArgSet; 
  TTree *inTree=0;
  TFile *inFile=0;
  double bias{0};
  string histName, rooName, errSigma, value;

  TH1::AddDirectory(kFALSE);

  for ( unsigned int iFile=0; iFile <dataFiles.size(); iFile++ ) { 
    inFile= TFile::Open(dataFiles[iFile].c_str());
    if ( !inFile ) throw invalid_argument( "BiasAnalysis::SelectVariables: Unknown input file "+dataFiles[iFile] );

    inTree = (TTree*) inFile->Get(m_inTreeName.c_str());  
    if ( !inTree ) throw runtime_error( "BiasAnalysis::SelectVariables: "+m_inTreeName + " in " + dataFiles[iFile] + " does not exist." );
    MapBranches mapBranches; 
    mapBranches.LinkTreeBranches(inTree);

    for ( unsigned int iEntry=0; iEntry<inTree->GetEntries(); iEntry++ ) {
      inTree->GetEntry(iEntry);
      histName =""; 
      if ( mapBranches.GetUnsigned( "nBins" )!=m_nBins ) continue;
      bias = mapBranches.GetDouble( "sigma" )-mapBranches.GetDouble( "inputC" );
	  
      //Combine all possible values of each variable
      for ( unsigned int iVar =0; iVar < m_variablesBias.size(); iVar++ ){
	value=mapBranches.GetLabel( m_variablesBias[iVar] );

	if ( iVar == m_variablesBias.size()-1 ) {
	  histName+= m_variablesBias[iVar]+"_"+value; 
       	  if ( !m_mapHist.count(histName) ) {
	    m_mapHist[histName] = new TH1D( histName.c_str(), "", 2000, -0.1, 0.1 );
	    m_mapHist[histName]->Sumw2();

	    //Create elements needed to fit using RooFit
	    rooName= "bias_"+histName;
	    m_mapRooVar[histName] = new RooRealVar( rooName.c_str(), "C^{meas}-C^{input}", -0.1, 0.1 );
	    rooName= "set_"+histName;
	    mapArgSet[histName] = new RooArgSet( rooName.c_str() );
	    rooName= "data_"+histName;
	    m_mapRooDataSet[histName] = new RooDataSet( rooName.c_str(), "data", *mapArgSet[histName] );
	  }

	  m_mapHist[histName]->Fill( bias );
	  m_mapRooVar[histName]->setVal( bias );
	  mapArgSet[histName]->add( *m_mapRooVar[histName] );
	  m_mapRooDataSet[histName]->add( *mapArgSet[histName] );
	}
	histName += m_variablesBias[iVar]+"_"+value+"_";   
      }//end iVar (1st loop)
    }//end iEntry (1st loop)

    inFile->Close(); //close file and delete tree
    delete inFile;
    
  }//end iFile

  cout<<"BiasAnalysis::SelectVariables Done"<<endl;

  return;
}


//====================================================
vector<double> BiasAnalysis::GetBiasStat( TH1* hist, string histName, unsigned int method)
{
  cout<<"BiasAnalysis::GetBiasStat"<<endl;
  double mean{0}, errMean{0}, rms{0};
  vector<double> vectStat;
  unsigned int nBins=hist->GetNbinsX();
  double xMin = hist->GetXaxis()->GetBinCenter(2);
  double xMax = hist->GetXaxis()->GetBinCenter(nBins-1);
    
  switch (method) {
    // case 0: {//compute mean and rms
    // 	mean= m_mapSumX[histName]/m_mapNEff[histName];
    // 	rms= sqrt( m_mapSumXM[histName]/m_mapNEff[histName] );
    // 	errMean= rms/sqrt(m_mapNEff[histName]);
    // 	break;
    // }

  case 1:{//get mean and rms from histogram
    mean= hist->GetMean();
    rms= hist->GetRMS();
    errMean= rms/sqrt( hist->GetEntries() );
    break;
  }

  case 2: {//get from gaussian fit
    TF1 *f0 = new TF1("f0", "gaus", xMin, xMax);
    hist->Fit("f0","R");
    mean = hist->GetFunction("f0")->GetParameter(1);
    rms = hist->GetFunction("f0")->GetParameter(2);
    if (mean-1.5*rms>=xMin) xMin= mean-1.5*rms;
    if (mean+1.5*rms<=xMax && mean+1.5*rms>xMin) xMax= mean+1.5*rms;
    TF1 *f1= new TF1("f1", "gaus", xMin, xMax);
    hist->Fit("f1","R");
      
    mean = hist->GetFunction("f1")->GetParameter(1);
    rms = hist->GetFunction("f1")->GetParameter(2);
    errMean = hist->GetFunction("f1")->GetParError(1); 
    
    delete f1; f1=0;
    delete f0; f0=0;
    break;
  }
  case 3: {//gauss RooFit
    RooRealVar* meanFit = new RooRealVar("meanFit", "mean", -1, 1);
    RooRealVar* sigmaFit= new RooRealVar("sigmaFit", "sigma",0, 1);
    m_mapRooGauss[histName]= new RooGaussian("gauss", "gauss", *m_mapRooVar[histName], *meanFit, *sigmaFit);    
    m_mapRooGauss[histName]->fitTo( *m_mapRooDataSet[histName], Range( xMin+(xMax-xMin)*0.02, xMax ) );
    rms= sigmaFit->getValV();
    mean= meanFit->getValV();
    m_mapRooGauss[histName]->fitTo( *m_mapRooDataSet[histName], Range( mean-1.5*rms, mean+1.5*rms) );
    delete meanFit;
    delete sigmaFit;
    break;
  }
  }//end switch

  vectStat.push_back(mean);
  vectStat.push_back(rms);
  vectStat.push_back(errMean);
  cout<<"BiasAnalysis::GetBiasStat Done"<<endl;
  return vectStat;
}



//=====================================================

void BiasAnalysis::SaveBiasInfo( string outName )
{
  cout<<"BiasAnalysis::SaveBiasInfo"<<endl;
  vector <string> vectInfo;
  vector <double> vectStat;
  unsigned int skip{0};
  string histName;
  TH1D *hist=0;
  TFile *outRootFile = new TFile( (outName+".root").c_str(), "RECREATE" ); 
  ofstream outputFile( (outName+".csv").c_str(), ios::out );

  for ( auto it=m_mapHist.begin(); it!=m_mapHist.end(); it++ )  {
    histName= it->first;
    hist=it->second;
    hist->Write();

    //writing the csv file
    if (!skip) {
      outputFile<<"HistogramName"<<" "<<"NumberEntries"<<" ";
      for ( unsigned int iVarBias=0; iVarBias<m_variablesBias.size(); iVarBias++ ) outputFile<<m_variablesBias[iVarBias]<<" ";
      outputFile<<"Mean"<<" "<<"RMS"<<" "<<"ErrorMean"<<"\n";
      }
    skip++;
    outputFile<<histName<<" "<<hist->GetEntries()<<" ";
    cout<<histName<<endl;
    ParseVector(histName, vectInfo, '_');
    for (unsigned int i=1; i<vectInfo.size(); i+=2) outputFile<<vectInfo[i]<<" ";
    vectInfo.clear();

    vectStat=GetBiasStat( hist, histName, m_methodStats );
    for (unsigned int i=0; i<vectStat.size(); i++) outputFile<<vectStat[i]<<" ";
    outputFile<<"\n";
    m_mapStatHist[histName]=vectStat;
  }//end iteration over histograms

  outRootFile->Close();
  delete outRootFile;
  delete hist;
  cout<<"BiasAnalysis::SaveBiasInfo Done"<<endl;
  return;
}





// //==================================================
// //Draw plots and save them into a pdf file

// void BiasAnalysis::MakeBiasPlots(string csvFileName, string path, string latexFileName, string comment)
// {
//   cout<<"\t BiasAnalysis::MakeBiasPlots"<<endl;

//   //Prepare latex file to store plots 
//   fstream stream;
//   string latexTitle = "Bias study";
//   latexFileName+=".tex";
//   stream.open( latexFileName.c_str(), fstream::out | fstream::trunc );
//   WriteLatexHeader( stream, latexTitle , "Antinea Guerguichon" );

//   //Draw plots of bias distribution
//   //Options to draw the histograms (cf DrawPlot.cxx)
//   vector <string> vectStatNames, vectOptDraw, vectLineContent, vectTmp;
//   vectStatNames.push_back("Mean");
//   vectStatNames.push_back("RMS");
//   vectStatNames.push_back("Error mean");

//   string histName;
//   unsigned int iHist=0;
//   unsigned int iLine=0;
//   TString statVal, statPos;
//   string legLatex, line;
//   map <unsigned int, vector<string>> mapColumnContent;

//   //Reading the csv file
//   ifstream csvFile(csvFileName, ios::in);
//   if (!csvFile) {cout<<"Cannot open "<<csvFileName<< " for reading."<<endl; return;}
  
//   while (getline(csvFile, line))
//     {
//       vectLineContent.push_back(line);
//       ParseVector(vectLineContent[iLine], vectTmp);
//       mapColumnContent.insert(pair<unsigned int, vector<string>>(iLine, vectTmp));
//       vectTmp.clear();
//       iLine++;
//     }

//   for (unsigned int iName=0; iName<m_histNames.size(); iName++)
//     {
//       histName = m_histNames[iName];
//       m_histNames[iName]=path+m_histNames[iName];

//       for (unsigned int i=0; i<vectStatNames.size(); i++)
//       	{
//       	  statVal= TString::Format("%.3e", m_histStats[iHist][i]);
// 	  legLatex= "latex="+ vectStatNames[i]+ ": " +statVal;
//       	  vectOptDraw.push_back(legLatex.c_str());
// 	  statPos= TString::Format("%.3f", 0.9-i*0.05);
// 	  legLatex= "latexOpt= 0.65 "+ statPos;
// 	  vectOptDraw.push_back(legLatex.c_str());
//       	}
      
//       vectOptDraw.push_back("yTitle=#Events");
      	
//       for (unsigned int indexMap=1; indexMap<mapColumnContent.size(); indexMap++)
// 	{
// 	  for (unsigned int iVar=0; iVar<m_variablesBias.size();iVar++)
// 	    {
// 	      if (histName==mapColumnContent[indexMap][0])
// 		{
// 		  legLatex= "latex="+m_variablesBias[iVar]+": "+mapColumnContent[indexMap][iVar+3];
// 		  vectOptDraw.push_back(legLatex.c_str());
// 		  statPos= TString::Format("%f", 0.9-iVar*0.05);
// 		  legLatex= "latexOpt= 0.15 "+ statPos;
// 		  vectOptDraw.push_back(legLatex.c_str());
// 		}
// 	    }
// 	}

      
//       vectOptDraw.push_back("extendUp= 0.4");
      
//       if (m_methodStats == 3)  DrawPlot(m_mapRooVar[histName], {m_mapRooDataSet[histName], m_mapRooGauss[histName]}, path+histName,{vectOptDraw} );
      
//       switch (m_checkDistri)
// 	{
// 	case 0:
// 	  {
// 	    vectOptDraw.push_back("xTitle=C^{meas}-C^{input}");
// 	    break;
// 	  }
// 	case 1:
// 	  {
// 	    vectOptDraw.push_back("xTitle=dataRMS");
// 	    break;
// 	  }
// 	default:
// 	  vectOptDraw.push_back("xTitle=C^{meas}-C^{input}");
// 	}

//       vector <TH1*> vectHistTmp;
//       vectHistTmp.push_back(m_mapHist[histName]);
//       if (m_methodStats!=3) DrawPlot(vectHistTmp, path+histName, {vectOptDraw});       
//       vectOptDraw.clear();
//       vectHistTmp.clear();
      
//       iHist++;
//     }

//   //  Store plots into the file
//   //stream << "\\section{Method to get stats: "<<m_methodStats<<"}"<< endl;  
//   stream <<comment <<"\\newline"<<endl;
//   stream << "\\indent Tree: "<< m_inTreeName<<"\\newline  "<<endl;
//   stream << "\\indent Variables: ";
//   for (unsigned int iVar=0; iVar< m_variablesBias.size(); iVar++)
//     {
//       if (iVar == m_variablesBias.size()-1) stream<<m_variablesBias[iVar] <<"\\newline  ";
//       else  stream  << m_variablesBias[iVar] <<", ";
//     }

//   if (m_checkDistri ==1) stream << "\\indent Check for errSigma distribution\\newline"<<endl;
//   WriteLatexMinipage( stream, m_histNames, 2);
//   stream << "\\end{document}" << endl;
//   string commandLine = "pdflatex  -interaction=batchmode " + latexFileName;
//   system( commandLine.c_str() );
//   system( commandLine.c_str() );
//   system( commandLine.c_str() );

//   commandLine = "rm " + path+ m_variablesBias[0]+ "*";
//   system( commandLine.c_str() );
 
//   cout<<"Plots drawn and stored into "<<path+latexFileName<<endl;
//   csvFile.close();
//   return;
// }



// //============================================
// //Invert Cij matrix and store histograms in a root file. Two types of histograms: nb events vs ci and <ci> vs iBin
 
// void BiasAnalysis::InvertCijMatrix(string path, unsigned int inversionProcedure)
// {
//   cout<<"\t BiasAnalysis::InvertCijMatrix"<<endl;

//   if (m_inTreeName!="ConfigurationsCTree") {cout<<m_inTreeName<<" Option selectTree different from ConfigurationsCTree. Cij matrices not inverted."<<endl; return;}

//   TString rootFileName= TString::Format ((path+"CiMatricesInv_%u.root").c_str(),inversionProcedure);
//   TFile *outRootFile = new TFile (rootFileName, "RECREATE");
//   if (outRootFile == 0) { cout<<"Error: cannot open "<<rootFileName<<endl; return;}

//   TString outFileName= TString::Format ((path+"CiMatricesInv_%u.csv").c_str(),inversionProcedure);
//   ofstream outputFile(outFileName, ios::out);
//   if (outputFile == 0) {cout<<"Error: cannot open "<<outFileName<<endl; return ;}

//   unsigned long long toyNumber;
//   int nRows;
//   TString histName; 
//   map < int, double> mapXMin, mapXMax;
//   map < int, TH1D*> mapHistBias; //mapHistBias: plot nb events vs ci for a given bin
//   TH1D *histBin=NULL;

//   map <unsigned long long, TMatrixD>::iterator it =m_mapCij.begin();

//   //ofstream csvConf("conf.csv", ios::out);

//   while(it !=m_mapCij.end())
//     {
      
//       toyNumber= it->first;
//       nRows=m_mapCij[toyNumber].GetNrows();
      
//       TMatrixD resultMatrix(nRows, 1);
//       TMatrixD resultErrMatrix(nRows, 1);
	  
//       InvertMatrix( m_mapCij[toyNumber], m_mapErrCij[toyNumber], resultMatrix, resultErrMatrix, inversionProcedure);
	
//       m_mapCi.insert( pair <unsigned long long, TMatrixD>(toyNumber, resultMatrix) );
//       m_mapErrCi.insert( pair <unsigned long long, TMatrixD>(toyNumber, resultErrMatrix) );

//       for (int iBin=0; iBin<nRows; iBin++)
// 	{
// 	  //csvConf<<"toyNumber: "<<toyNumber<<" iBin: "<<iBin<<" sigma: "<<resultMatrix[iBin][0]<<endl;
// 	  if (mapXMin.count(iBin)==0 && mapXMax.count(iBin)==0) 
// 	    {
// 	      mapXMin.insert(pair<int, double>(iBin, resultMatrix[iBin][0]-mapInput[toyNumber]));
// 	      mapXMax.insert(pair<int, double>(iBin, resultMatrix[iBin][0]-mapInput[toyNumber]));
// 	    }
// 	  else
// 	    {
// 	      if (resultMatrix[iBin][0]-mapInput[toyNumber]<mapXMin[iBin]) mapXMin[iBin]=resultMatrix[iBin][0]-mapInput[toyNumber];
// 	      if (resultMatrix[iBin][0]-mapInput[toyNumber]>mapXMax[iBin]) mapXMax[iBin]=resultMatrix[iBin][0]-mapInput[toyNumber];
// 	    }
// 	}

//       it++;
//     }

//   cout<<"Matrices inverted for all toys. Drawing histograms..."<<endl;
 
//   for (unsigned long long toyNumber=0; toyNumber <m_mapCi.size(); toyNumber++)
//     {
//       for (int iBin=0; iBin<m_mapCi[toyNumber].GetNrows(); iBin++)
// 	{
// 	  if (mapHistBias.count(iBin)==0)
// 	    {
// 	      histName= TString::Format("biasBin_%u",iBin);
// 	      mapHistBias.insert( pair<int, TH1D*> (iBin, new TH1D(histName,"", 100 , mapXMin[iBin], mapXMax[iBin])));
// 	      mapHistBias[iBin]->Fill(m_mapCi[toyNumber][iBin][0]-mapInput[toyNumber]);
// 	      mapHistBias[iBin]->SetBinError(mapHistBias[iBin]->GetXaxis()->FindBin(m_mapCi[toyNumber][iBin][0]), m_mapErrCi[toyNumber][iBin][0]);
// 	    }
// 	  else 
// 	    {
// 	      mapHistBias[iBin]->Fill(m_mapCi[toyNumber][iBin][0]-mapInput[toyNumber]);
// 	      mapHistBias[iBin]->SetBinError(mapHistBias[iBin]->GetXaxis()->FindBin(m_mapCi[toyNumber][iBin][0]), m_mapErrCi[toyNumber][iBin][0]);
// 	    }
// 	  if (toyNumber==m_mapCi.size()-1) 
// 	    {
// 	      mapHistBias[iBin]->Write();
// 	      if (iBin==0) 
// 		{
// 		  histBin= new TH1D("histBin", "", m_mapCi[toyNumber].GetNrows(), 0, m_mapCi[toyNumber].GetNrows()); 
// 		  outputFile<<"InversionProcedure "<<"Bin "<<"Mean "<<"RMS "<<"ErrorMean "<<endl;
// 		}
// 	      histBin->SetBinContent( histBin->GetXaxis()->FindBin(iBin), mapHistBias[iBin]->GetMean() );
// 	      histBin->SetBinError( histBin->GetXaxis()->FindBin(iBin), (mapHistBias[iBin]->GetRMS())/(mapHistBias[iBin]->GetEntries()) ); 

// 	      outputFile<<inversionProcedure<<" "<<iBin<<" "<<mapHistBias[iBin]->GetMean()<<" "<<mapHistBias[iBin]->GetRMS()<<" "<<(mapHistBias[iBin]->GetRMS())/(mapHistBias[iBin]->GetEntries())<<endl;
	    
// 	      cout<<"bin: "<<iBin<<" mean: "<<mapHistBias[iBin]->GetMean()<<" +/-  " <<(mapHistBias[iBin]->GetRMS())/(mapHistBias[iBin]->GetEntries())<<endl;
// 	    }
// 	}
//     }

//   histBin->Write();
//   cout<<"Histograms saved into "<<rootFileName<<endl;
//   cout<<"File "<<outFileName<<" written."<<endl;
//   outRootFile->Close();
//   outputFile.close();
//   delete outRootFile;
//   delete histBin;
//   return;
// }


// //======================================================
// //Read csv file and plot bias vs stat

// void BiasAnalysis::MakeCompStatPlot(string csvFileName, string plotName, unsigned int input, bool isConf)
// {
//   cout<<"\t BiasAnalysis::MakeCompStatPlot"<<endl;
//   ifstream csvFile(csvFileName, ios::in);
//   if (!csvFile) {cout<<"Cannot open "<<csvFileName<< " for reading."<<endl; return;}
//   string absBiasFileName;
//   if (isConf) absBiasFileName= "/sps/atlas/a/aguerguichon/Calibration/Bias/Stats/BiasConf_ref.csv"; 
//   else absBiasFileName= "/sps/atlas/a/aguerguichon/Calibration/Bias/Stats/BiasBin_ref.csv";

//   ifstream absBiasFile(absBiasFileName, ios::in);
//   if (!absBiasFile) {cout<<"Cannot open refFile for reading."<<endl; return;}


//   string line, stat, iConf, jConf, iBin, strInput;
//   vector <string> vectLineContent, vectAbsBias, vectTmp, vectOpt;
//   vector <TGraphErrors*> vectGraph;
//   unsigned int iLine=0;
//   unsigned int iStat, statCol, meanCol, errCol, iConfCol, jConfCol, iBinCol;
//   map <unsigned int, TGraphErrors*> mapGraph;
//   map <unsigned int, vector <string>> mapLineContent, mapAbsBias; 

//   //Read csv file
//   while (getline(csvFile, line))
//     {
//       vectLineContent.push_back(line);
//       ParseVector(vectLineContent[iLine], vectTmp);
//       mapLineContent.insert(pair<unsigned int, vector<string>>(iLine, vectTmp));
//       vectTmp.clear();
//       iLine++;
//     }

//   iLine=0;
//   while (getline(absBiasFile, line))
//     {
//       vectAbsBias.push_back(line);
//       ParseVector(vectAbsBias[iLine], vectTmp);
//       mapAbsBias.insert(pair<unsigned int, vector<string>>(iLine, vectTmp));
//       vectTmp.clear();
//       iLine++;
//     }


//   //Fill TGraph 
//   vector <double> vectStat, vectBias, vectErrBias, vectErrX;

//   strInput= to_string(input);
//   cout<<"input: "<<strInput<<endl;
//   string histNameTest, histNameComp, histNameAbs, substrTest, substrComp, substrAbs, statTest, statComp; 
//   map <string, unsigned int> mapInput;
//   size_t pos;

//   for (unsigned int iLineTest=1; iLineTest<mapLineContent.size(); iLineTest++)
//     {
//       iStat=0;
//       vectStat.clear();
//       vectBias.clear();
//       vectErrBias.clear();

//       if (iLineTest==1) 
// 	{ 
// 	  for(unsigned int iCol=0; iCol<mapLineContent[iLineTest].size(); iCol++)
// 	    {
// 	      if (mapLineContent[0][iCol]=="statTree") statCol=iCol;
// 	      if (mapLineContent[0][iCol]=="Mean") meanCol=iCol;
// 	      if (mapLineContent[0][iCol]=="ErrorMean") errCol=iCol;
// 	      if (isConf){if (mapLineContent[0][iCol]=="iConf") iConfCol=iCol; if (mapLineContent[0][iCol]=="jConf") jConfCol=iCol;} 
// 	      else{if (mapLineContent[0][iCol]=="iBin") iBinCol=iCol;}  
// 	    }
// 	}
//       histNameTest= mapLineContent[iLineTest][0];

//       if (histNameTest.find("inputC_"+strInput+"_")==string::npos) {continue;}

   
//       if (mapInput.count(mapLineContent[iLineTest][statCol])==0 ) {statTest=mapLineContent[iLineTest][statCol];mapInput.insert(pair<string, unsigned int>(statTest, iStat));vectStat.push_back(stod(statTest));iStat++;}
  
//       pos=histNameTest.find("indepTemplates");
//       substrTest=histNameTest.substr(pos);

//       if(isConf) {iConf=mapLineContent[iLineTest][iConfCol];jConf=mapLineContent[iLineTest][jConfCol];}
//       else {iBin=mapLineContent[iLineTest][iBinCol];} 


//       for (unsigned int iLineComp=iLineTest; iLineComp<mapLineContent.size(); iLineComp++)
// 	{
// 	  histNameComp= mapLineContent[iLineComp][0];
	  
// 	  if (histNameComp.find("inputC_"+strInput+"_")==string::npos || histNameComp.find(substrTest)==string::npos || histNameComp==histNameTest) {continue;}
	  

// 	  if (mapInput.count(mapLineContent[iLineComp][statCol])==0 ) {statComp= mapLineContent[iLineComp][statCol];mapInput.insert(pair<string, unsigned int>(statComp, iStat)); vectStat.push_back(stod(statComp)); vectErrX.push_back(0.);}
    
// 	  for (unsigned int iLineAbs=1; iLineAbs<mapAbsBias.size(); iLineAbs++)
// 	    {
// 	      histNameAbs= mapAbsBias[iLineAbs][0];

// 	      if (histNameAbs.find("inputC_"+strInput+"_")==string::npos) continue;
// 	      if (isConf) pos=histNameAbs.find("iConf_");
// 	      else pos=histNameAbs.find("iBin_"); 
// 	      substrAbs=histNameAbs.substr(pos);
// 	      if (histNameTest.find(substrAbs)!=string::npos) //for the considered config/bin
// 		{
// 		  vectBias.push_back(stod(mapLineContent[iLineTest][meanCol])-stod(mapAbsBias[iLineAbs][meanCol]));
// 		  vectBias.push_back(stod(mapLineContent[iLineComp][meanCol])-stod(mapAbsBias[iLineAbs][meanCol]));
// 		  vectErrBias.push_back(sqrt( pow(stod(mapLineContent[iLineTest][errCol]),2)+pow(stod(mapAbsBias[iLineAbs][errCol]),2) ));
// 		  vectErrBias.push_back(sqrt( pow(stod(mapLineContent[iLineComp][errCol]),2)+pow(stod(mapAbsBias[iLineAbs][errCol]),2) ));

// 		  // vectBias.push_back(stod(mapLineContent[iLineTest][meanCol]));
// 		  // vectBias.push_back(stod(mapLineContent[iLineComp][meanCol]));
// 		  // vectErrBias.push_back(stod(mapLineContent[iLineTest][errCol]));
// 		  // vectErrBias.push_back(stod(mapLineContent[iLineComp][errCol]));
		  
		  

// 		  vectGraph.push_back(new TGraphErrors(vectBias.size(), &vectStat[0], &vectBias[0], &vectErrX[0], &vectErrBias[0]));
// 		  if (isConf) vectOpt.push_back("legend= ("+iConf+", "+jConf+")");
// 		  else vectOpt.push_back("legend= Bin "+iBin);
// 		}
	      
// 	    }//end iLineAbs
    
// 	} //end iLineComp
            
//     }   //end iLineTest   

//   vectOpt.push_back("latex=Input "+to_string(input*1e-6));
//   vectOpt.push_back("latexOpt=0.15 0.9");
//   vectOpt.push_back("legendPos= 0.85 0.9");
//   vectOpt.push_back("xTitle=statistics");
//   vectOpt.push_back("yTitle=bias");
//   vectOpt.push_back("rangeUserY= -0.008 0.01");
//   vectOpt.push_back("rangeUserX= 1e3 1.2e6");
//   DrawPlot(vectGraph, plotName, vectOpt);

//   csvFile.close();
//   return;
// }
















      // //Fill maps for inversion procedure
      // if (m_inTreeName=="ConfigurationsCTree") {
      // 	toyNumber= mapBranches.GetULongLong("toyNumber");
      // 	i=mapBranches.GetUnsigned("iConf");
      // 	j=mapBranches.GetUnsigned("jConf");
      // 	if (m_mapCij.count(toyNumber)==0)	{
      // 	  m_mapCij.insert( pair<unsigned long long, TMatrixD>(toyNumber, TMatrixD(mapBranches.GetUnsigned("nBins"), mapBranches.GetUnsigned("nBins"))) );
      // 	  m_mapErrCij.insert(pair<unsigned long long, TMatrixD>(toyNumber, TMatrixD(mapBranches.GetUnsigned("nBins"), mapBranches.GetUnsigned("nBins"))) );
      // 	  mapInput.insert(pair<unsigned long long, double>(toyNumber, mapBranches.GetDouble("inputC")));

      // 	  m_mapCij[toyNumber][i][j]=mapBranches.GetDouble("sigma");
      // 	  m_mapCij[toyNumber][j][i]=mapBranches.GetDouble("sigma");
      // 	  m_mapErrCij[toyNumber][i][j]=mapBranches.GetDouble("errSigma");
      // 	  m_mapErrCij[toyNumber][j][i]=mapBranches.GetDouble("errSigma");
      // 	}
      // 	else {
      // 	  m_mapCij[toyNumber][i][j]=mapBranches.GetDouble("sigma");
      // 	  m_mapCij[toyNumber][j][i]=mapBranches.GetDouble("sigma");
      // 	  m_mapErrCij[toyNumber][i][j]=mapBranches.GetDouble("errSigma");
      // 	  m_mapErrCij[toyNumber][j][i]=mapBranches.GetDouble("errSigma");
      // 	}
	      
      // 	if ( i==(mapBranches.GetUnsigned("nBins")-1) && j==(mapBranches.GetUnsigned("nBins")-1) ) {
      // 	  for (unsigned int iRow=0; iRow<=i; iRow++) {
      // 	    for(unsigned int iCol=0; iCol<=i; iCol++) {
      // 	      if ( m_mapCij[toyNumber][iCol][iRow]==0 && m_mapErrCij[toyNumber][iCol][iRow]==0 ) m_mapErrCij[toyNumber][iCol][iRow]=100;
      // 	    }
      // 	  }
      // 	}
      // }
