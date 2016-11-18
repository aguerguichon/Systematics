// #include <iostream>
// #include <istream>
// #include <fstream>
// #include <map>
// #include <vector>
// #include <string>
// #include <sstream>
// #include <boost/program_options.hpp>
// #include <boost/multi_array.hpp>
// #include <iomanip>

// #include "TMatrixD.h"
// #include "TH1.h"
// #include "TF1.h"
// #include "TFile.h"
// #include "TTree.h"
// #include "TBranch.h"
// #include "TCanvas.h"
// #include "TString.h"
// #include "TGraphErrors.h"

// #include "RooDataSet.h"
// #include "RooRealVar.h"
// #include "RooArgSet.h"
// #include "RooGaussian.h"

// #include "Bias/BiasAnalysis.h"
// #include "PlotFunctions/DrawPlot.h"
// #include "PlotFunctions/SideFunctionsTpp.h"
// #include "PlotFunctions/SideFunctions.h"
// #include "PlotFunctions/MapBranches.h"
// #include "PlotFunctions/InvertMatrix.h"

// using namespace ChrisLib;
// using namespace std;
// using namespace RooFit;
// namespace po = boost::program_options;

// using boost::extents;
// using boost::multi_array;
// //====================================================
// //Read configuration options from a configuration file
// BiasAnalysis::BiasAnalysis(string configFileName)
// {  
//   po::options_description configOptions("Configuration options");
//   configOptions.add_options()
//     ("help", "Display this help message")
//     ("variablesBias", po::value<vector<string>>(&m_variablesBias)->multitoken(),"Variables to study bias")
//     /*accepted variables for ConfugurationsCTree: 
//        - double: sigma, errSigma, inputC, dataRMS, nOptim;
//        - unsigned int: iConf, jConf, statConf, statTree, indepDistorded, indepTemplates, runNumber, nBins, bootstrap, fitMethod;
//     accepted variables for scalesTree:
//     - double: sigma, errSigma, inputC, dataRMS, nOptim;
//     - unsigned int: iBin, statTree, indepDistorded, indepTemplates, runNumber, nBins, bootstrap, fitMethod, inversionMethod;
//     */
//     ("variablesStats", po::value<vector<unsigned int>>(&m_variablesStats)->multitoken(),"Variables for the csv file")
//     /*0: mean, rms, errMean
//      */
//     ("methodStats", po::value<unsigned int>(&m_methodStats), "")
//     /*0: compute mean and rms
//       1: h->GetMean(), h->GetRMS()
//       2: get mean given by the Gaussian fit of the histogram
//       3: RooFit gauss
//     */
//     ("selectTree", po::value<string>(&m_inTreeName), "")
//     /*ConfigurationsCTree or scalesTree
//      */    
//     ("checkDistri", po::value<unsigned int>(&m_checkDistri)->default_value(0),"")
//     /*0= bias study
//       1= errSigma distribution
//      */
    
//     ;
    
//   po::variables_map vm;
//   ifstream ifs( configFileName, ifstream::in );
//   po::store(po::parse_config_file(ifs, configOptions), vm);
//   po::notify(vm);  

//   cout<<"Configuration file "<<configFileName<<" loaded."<<endl;
//   cout<<m_methodStats<<endl;
//   m_nHist=0;
// }


// //==================================================
// //Clear attributes
// BiasAnalysis::~BiasAnalysis()
// {
//   m_variablesBias.clear();
//   m_variablesStats.clear();
//   m_histNames.clear();
//   m_bias.clear();
//   m_errBias.clear();


//   m_mapHist.clear();
//   m_mapXMin.clear();
//   m_mapXMax.clear();
//   m_mapInput.clear();
//   m_mapHistPosition.clear();
//   m_mapSumX.clear();
//   m_mapSumXM.clear();
//   m_mapNEff.clear();
//   m_mapBias.clear();
//   m_mapDataSet.clear();
//   m_mapGauss.clear();
//   m_mapCij.clear();
//   m_mapErrCij.clear();
//   m_mapCi.clear();
//   m_mapErrCi.clear();

 
//   m_inTreeName.clear();
  
//   m_nHist=-999;
//   m_methodStats=-999;
//   m_checkDistri=-999;

  
//   for (unsigned int iHist=0; iHist<m_nHist; iHist++)
//     {
//       for(unsigned int iVar=0; iVar<m_variablesStats.size()+2; iVar++)
//   	{
//   	  m_histStats[iHist][iVar]=-999;
//   	}
//     }

  
//   cout<<"Cleaning ok"<<endl;

// }



// //===================================================
// // - Read files, link tree branches to local variables and sorting variables according to the ones selected.
// // - Fill the map m_mapHist with unique histogramms for each combination of all possible values of each variable.
// // - Fill m_mapCij and m_mapErrCij for the inversion procedure

// void BiasAnalysis::SelectVariables(vector <string> dataFiles)
// {
//   cout<<"\t BiasAnalysis::SelectVariables"<<endl;
//   //ofstream csvBin("dataRMS.csv", ios::out);

//   map <string, double> mapMean;
//   map <string, RooArgSet*> mapArgSet;
   
//   TTree *inTree;
//   TFile *inFile;
//   unsigned int nEntries, i, j;
//   unsigned long long toyNumber;
//   double bias;

//   string histName, rooName, errSigma;
//   TString value, rooNum, runNumber;

//   TH1::AddDirectory(kFALSE);

//   //1st loop over files: get min & max of each histogram, fill maps needed to compute mean an rms
//   for (unsigned int iFile=0; iFile <dataFiles.size(); iFile++)
//     { 
//       inFile= TFile::Open(dataFiles[iFile].c_str());
//       if (inFile == 0) { cout<<"Error: cannot open "<<dataFiles[iFile]<<" file\n"<<endl; return;}

//       inTree = (TTree*) inFile->Get(m_inTreeName.c_str());  
//       MapBranches mapBranches; 
//       mapBranches.LinkTreeBranches(inTree);
//       nEntries= inTree->GetEntries();
     
//       for (unsigned int iEntry=0; iEntry<nEntries; iEntry++)
//       	{
//       	  inTree->GetEntry(iEntry);
// 	  histName ="";
	  
// 	  //Preliminary study for 6 bins
// 	  if (mapBranches.GetUnsigned("nBins")!=6) continue;

// 	  //Fill maps for inversion procedure
// 	  if (m_inTreeName=="ConfigurationsCTree")
// 	    {
// 	      toyNumber= mapBranches.GetULongLong("toyNumber");
// 	      i=mapBranches.GetUnsigned("iConf");
// 	      j=mapBranches.GetUnsigned("jConf");
// 	      if (m_mapCij.count(toyNumber)==0)
// 	  	{
// 	  	  m_mapCij.insert( pair<unsigned long long, TMatrixD>(toyNumber, TMatrixD(mapBranches.GetUnsigned("nBins"), mapBranches.GetUnsigned("nBins"))) );
// 	  	  m_mapErrCij.insert(pair<unsigned long long, TMatrixD>(toyNumber, TMatrixD(mapBranches.GetUnsigned("nBins"), mapBranches.GetUnsigned("nBins"))) );
// 		  m_mapInput.insert(pair<unsigned long long, double>(toyNumber, mapBranches.GetDouble("inputC")));

// 	  	  m_mapCij[toyNumber][i][j]=mapBranches.GetDouble("sigma");
// 	  	  m_mapCij[toyNumber][j][i]=mapBranches.GetDouble("sigma");
// 	  	  m_mapErrCij[toyNumber][i][j]=mapBranches.GetDouble("errSigma");
// 	  	  m_mapErrCij[toyNumber][j][i]=mapBranches.GetDouble("errSigma");
// 		}
// 	      else
// 	  	{
// 	  	  m_mapCij[toyNumber][i][j]=mapBranches.GetDouble("sigma");
// 	  	  m_mapCij[toyNumber][j][i]=mapBranches.GetDouble("sigma");
// 	  	  m_mapErrCij[toyNumber][i][j]=mapBranches.GetDouble("errSigma");
// 	  	  m_mapErrCij[toyNumber][j][i]=mapBranches.GetDouble("errSigma");
// 	  	}
	      
// 	      if ( i==(mapBranches.GetUnsigned("nBins")-1) && j==(mapBranches.GetUnsigned("nBins")-1) )
// 	  	{
// 	  	  for (unsigned int iRow=0; iRow<=i; iRow++)
// 	  	    {
// 	  	      for(unsigned int iCol=0; iCol<=i; iCol++)
// 	  		{
// 	  		  if ( m_mapCij[toyNumber][iCol][iRow]==0 && m_mapErrCij[toyNumber][iCol][iRow]==0 ) m_mapErrCij[toyNumber][iCol][iRow]=100;
// 	  		}
// 	  	    }
// 	  	}
// 	    }
// 	  //else csvBin<<"toyNumber: "<<mapBranches.GetULongLong("toyNumber")<<" iBin: "<<mapBranches.GetUnsigned("iBin")<<" sigma: "<<mapBranches.GetDouble("sigma")<<endl;

// 	  //Bias study or errSigma study  
// 	  switch (m_checkDistri)
// 	    {
// 	    case 0:
// 	      { 
// 		bias = mapBranches.GetDouble("sigma")-mapBranches.GetDouble("inputC");
// 		break;
// 	      }
// 	    case 1:
// 	      {
// 		bias = mapBranches.GetDouble("dataRMS");
// 		break;
// 	      }
// 	    default:
// 	      bias =  mapBranches.GetDouble("sigma")-mapBranches.GetDouble("inputC");
// 	    }
	  
// 	  //Combine all possible values of each variable
// 	  for (unsigned int iVar =0; iVar < m_variablesBias.size(); iVar++)
//       	    {
// 	      if (mapUInt.count(m_variablesBias[iVar])>0) value = TString::Format("%d",mapUInt.find(m_variablesBias[iVar])->second);
// 	      if (mapDouble.count(m_variablesBias[iVar])>0) value = TString::Format("%d",(int) floor ( (mapDouble.find(m_variablesBias[iVar])->second)*1e6) );
// 	      if (mapBranches.GetUnsigned("statTree")>=2E6)
// 	      {
// 		if (m_variablesBias[iVar] == "statTree") value = TString::Format("%d", 2774685);	      
// 	      }
	           
// 	      if (iVar == m_variablesBias.size()-1) 
// 		{
// 		  histName+= m_variablesBias[iVar]+"_"+value;
// 		  if (m_mapNEff.count(histName) == 0)
// 		    {
// 		      m_mapXMin.insert(pair<string, double>(histName, bias));
// 		      m_mapXMax.insert(pair<string, double>(histName, bias));
		      
// 		      m_mapSumX.insert(pair<string, double>(histName, bias));
// 		      // m_mapSumXSquare.insert(pair<string, double>(histName, bias*bias));
// 		      m_mapNEff.insert(pair<string, unsigned int>(histName, 1));
// 		      m_mapHistPosition.insert(pair<string, unsigned int>(histName, m_nHist));
		           		      
// 		      m_nHist++;
// 		    }

// 		  if (m_mapNEff.count(histName) > 0)
// 		    {		     	      
// 		      if(bias<m_mapXMin[histName]) m_mapXMin[histName]=bias;
// 		      if(bias>m_mapXMax[histName]) m_mapXMax[histName]=bias;      
// 		      m_mapSumX[histName]+=bias;
// 		      //m_mapSumXSquare[histName]+=bias*bias;
// 		      m_mapNEff[histName]+=1;
// 		    }
// 		}
// 	      histName += m_variablesBias[iVar]+"_"+value+"_";
	      
// 	    }//end iVar (1st loop)

// 	}//end iEntry (1st loop)

//       inFile->Close(); //close file and delete tree
//       delete inFile;

//     }//end iFile

//   //2nd loop over files: fill m_mapHist     
//   for (unsigned int iFile=0; iFile <dataFiles.size(); iFile++)
//     { 
//       inFile= TFile::Open(dataFiles[iFile].c_str());
//       if (inFile == 0) { cout<<"Error: cannot open "<<dataFiles[iFile]<<" file\n"<<endl; return;}

//       inTree = (TTree*) inFile->Get(m_inTreeName.c_str());  
      
//       MapBranches mapBranches; 
//       mapBranches.LinkTreeBranches(inTree);
//       nEntries= inTree->GetEntries();
      
//       for (unsigned int iEntry=0; iEntry<nEntries; iEntry++)
// 	{
// 	  inTree->GetEntry(iEntry);
// 	  histName ="";
	  
// 	  if (mapBranches.GetUnsigned("nBins")!=6) continue;

// 	  switch (m_checkDistri)
// 	    {
// 	    case 0:
// 	      { 
// 		bias = mapBranches.GetDouble("sigma")-mapBranches.GetDouble("inputC");
// 		break;
// 	      }
// 	    case 1:
// 	      {
// 		bias = mapBranches.GetDouble("dataRMS");
// 		break;
// 	      }
// 	    default:
// 	      bias =  mapBranches.GetDouble("sigma")-mapBranches.GetDouble("inputC");
// 	    }


// 	  for (unsigned int iVar =0; iVar < m_variablesBias.size(); iVar++)
// 	    {
// 	      if (mapUInt.count(m_variablesBias[iVar])>0) value = TString::Format("%d",mapUInt.find(m_variablesBias[iVar])->second);
// 	      if (mapDouble.count(m_variablesBias[iVar])>0) value = TString::Format("%d",(int) floor ( (mapDouble.find(m_variablesBias[iVar])->second)*1e6) );
// 	      if (mapBranches.GetUnsigned("statTree")>=2E6)
// 	      {
// 		if (m_variablesBias[iVar] == "statTree") value = TString::Format("%d", 2774685);	      
// 	      }
// 	      if (iVar == m_variablesBias.size()-1) 
// 		{
// 		  histName+= m_variablesBias[iVar]+"_"+value; 
		  
// 		  if(m_mapHist.count(histName)==0)
// 		    {
// 		      if (m_mapXMin[histName] == m_mapXMax[histName]) m_mapHist.insert(pair<string, TH1D*> (histName, new TH1D(histName.c_str(), "", 100, -0.1, 0.1)));
// 		      else m_mapHist.insert(pair<string, TH1D*> (histName, new TH1D(histName.c_str(), "", 100, m_mapXMin[histName], m_mapXMax[histName])));
// 		      m_mapHist[histName]->Sumw2();
// 		      m_mapHist[histName]->Fill(bias);
// 		      mapMean.insert(pair <string, double> (histName, m_mapSumX[histName]/m_mapNEff[histName]));
// 		      m_mapSumXM.insert(pair<string, double> (histName, pow (bias-mapMean[histName], 2)) );
// 		      m_histNames.push_back(histName);

// 		      //Fill m_mapDataSet to fit using RooFit
// 		      rooName= "bias_"+histName;
// 		      m_mapBias.insert(pair<string, RooRealVar*>(histName, new RooRealVar(rooName.c_str(), "C^{meas}-C^{input}", m_mapXMin[histName], m_mapXMax[histName])));
// 		      m_mapBias[histName]->setVal(bias);
		      
// 		      rooName= "set_"+histName;
// 		      mapArgSet.insert(pair<string, RooArgSet*>(histName, new RooArgSet(rooName.c_str())));
// 		      mapArgSet[histName]->add(*m_mapBias[histName]);
		      
// 		      rooName= "data_"+histName;
// 		      m_mapDataSet.insert(pair<string, RooDataSet*> (histName, new RooDataSet(rooName.c_str(), "data", *mapArgSet[histName])));
// 		      m_mapDataSet[histName]->add(*mapArgSet[histName]);
// 		    }

// 		  if (m_mapHist.count(histName) > 0)
// 		    {
// 		      m_mapHist[histName]->Fill(bias);
// 		      m_mapSumXM[histName]+= pow (bias-mapMean[histName], 2);
// 		      m_mapBias[histName]->setVal(bias);
// 		      mapArgSet[histName]->add(*m_mapBias[histName]);
// 		      m_mapDataSet[histName]->add(*mapArgSet[histName]);
// 		    }
// 		}

// 	      histName += m_variablesBias[iVar]+"_"+value+"_";
	      
// 	    }//end iVar (2nd loop)
// 	}//end iEntry (2nd loop)

//   inFile->Close(); //close file and delete tree
//   delete inFile;

//     }//end iFile

//   cout << "End of selection."<<endl;
//   return;
// }



// //=====================================================
// //For each histogram, fill the 2D multi_array with:
// // - 1st dim: histogram
// // - 2nd dim: mean (for a given method), mean error, rms...
// //Fill a csv file with those values.

// void BiasAnalysis::MeasureBias( string outFileName, string outRootFileName)
// {
//   m_histStats.resize(extents[m_nHist][m_variablesStats.size()+2]);
//   //m_outFileName=outFileName;
//   unsigned int iHist=0;
//   unsigned int nBins; 
//   unsigned int skip=0;
//   double mean=0.;
//   double errMean=0.;
//   double rms=0; 
//   double xMin=0.;
//   double xMax=0.;
//   string histName;
//   char *token;
//   //  vector <double> mean100k, mean2M, mean1M, errMean100k, errMean2M, errMean1M, errBias1M, errBias100k;

//   TFile *outRootFile = new TFile(outRootFileName.c_str(), "RECREATE"); 

//   ofstream outputFile(outFileName, ios::out);
//   if (outputFile == 0) {cout<<"Error while opening outputFile"<<endl; return ;}
 
//   map <string, unsigned int>::iterator it=m_mapHistPosition.begin();
//   while(it != m_mapHistPosition.end())
//     {
//       histName= it->first;
//       iHist= it->second;
   
//       for (unsigned int iVar=0; iVar<m_variablesStats.size(); iVar++)
//   	{
// 	  switch (m_methodStats)
// 	    {

// 	    case 0://compute mean and rms
// 	      {
// 		if (m_variablesStats[iVar] == 0) 
// 		  {
// 		    mean= m_mapSumX[histName]/m_mapNEff[histName];
// 		    rms= sqrt( m_mapSumXM[histName]/m_mapNEff[histName] );
// 		    errMean= rms/sqrt(m_mapNEff[histName]);
// 		    m_mapHist[histName]->Write();
// 		  }
// 		break;
// 	      }

// 	    case 1://get mean and rms from histogram
// 	      {
// 		if (m_variablesStats[iVar]== 0)
// 		  {
// 		    mean= m_mapHist[histName]->GetMean();
// 		    rms= m_mapHist[histName]->GetRMS();
// 		    errMean= rms/sqrt(m_mapNEff[histName]);
// 		    m_mapHist[histName]->Write();
// 		  } 
// 		break;
// 	      }

// 	    case 2://get from gaussian fit
// 	      {
// 		nBins = m_mapHist[histName]->GetNbinsX();
		
// 		xMin = m_mapHist[histName]->GetXaxis()->GetBinCenter(2);
// 		xMax = m_mapHist[histName]->GetXaxis()->GetBinCenter(nBins-1);
// 		TF1 *f0 = new TF1("f0", "gaus", xMin, xMax);
// 		m_mapHist[histName]->Fit("f0","R");
// 		mean = m_mapHist[histName]->GetFunction("f0")->GetParameter(1);
// 		rms = m_mapHist[histName]->GetFunction("f0")->GetParameter(2);
// 		if (mean-1.5*rms>=xMin) xMin= mean-1.5*rms;
// 		if (mean+1.5*rms<=xMax && mean+1.5*rms>xMin) xMax= mean+1.5*rms;
// 		TF1 *f1= new TF1("f1", "gaus", xMin, xMax);
// 		m_mapHist[histName]->Fit("f1","R");
		
// 		if (m_variablesStats[iVar]== 0)
// 		  {
// 		    mean = m_mapHist[histName]->GetFunction("f1")->GetParameter(1);
// 		    rms = m_mapHist[histName]->GetFunction("f1")->GetParameter(2);
// 		    errMean = m_mapHist[histName]->GetFunction("f1")->GetParError(1); 
// 		  }
// 		m_mapHist[histName]->Write();
// 		delete f1; f1=0;
// 		delete f0; f0=0;
// 		break;
// 	      }
// 	    case 3://gauss RooFit
// 	      {
// 		RooRealVar* meanFit = new RooRealVar("meanFit", "mean", -1, 1);
// 		RooRealVar* sigmaFit= new RooRealVar("sigmaFit", "sigma",0, 1);
// 		m_mapGauss.insert(pair<string, RooGaussian*>(histName, new RooGaussian("gauss", "gauss", *m_mapBias[histName], *meanFit, *sigmaFit) ));

// 		m_mapGauss[histName]->fitTo(*m_mapDataSet[histName], Range( m_mapXMin[histName]+(m_mapXMax[histName]-m_mapXMin[histName])*0.02, m_mapXMax[histName] ));
// 		rms= sigmaFit->getValV();
// 		mean= meanFit->getValV();
// 		m_mapGauss[histName]->fitTo(*m_mapDataSet[histName], Range(mean-1.5*rms, mean+1.5*rms));
// 		m_mapHist[histName]->Write();
// 		delete meanFit;
// 		delete sigmaFit;
// 		break;
// 	      }
// 	    }//end switch

// 	  m_histStats[iHist][0]=mean;
// 	  m_histStats[iHist][1]=rms;
// 	  m_histStats[iHist][2]=errMean;
//   	}//end iVar

//       //writing the csv file
//       if (skip==0) 
// 	{
// 	  outputFile<<"HistogramName"<<" "<<"HistogramIndex"<<" "<<"NumberEntries"<<" ";
// 	  for (unsigned int iVarBias=0; iVarBias<m_variablesBias.size(); iVarBias++)
// 	    {
// 	      outputFile<<m_variablesBias[iVarBias]<<" ";
// 	    }
// 	  outputFile<<"Mean"<<" "<<"RMS"<<" "<<"ErrorMean"<<"\n";
// 	}

//       outputFile<<histName<<" "<<iHist<<" "<<m_mapNEff[histName]<<" ";
//       token = strtok((char*)histName.c_str(), "_");
//       skip=1;
//       while(token !=NULL)
//       	{
// 	  //int iVarBias=0;
// 	  if (skip>m_variablesBias.size()*2) break;
//       	  if (skip%2==0) outputFile<<token<<" ";
// 	  skip++;
// 	  token=strtok(NULL, "_");
//       	}
//       outputFile<<mean<<" "<<rms<<" "<<errMean<<"\n";

//       //next histogram
//       it++;
//     }//end iteration over histograms (while loop)

//   cout<<"csv file "<<outFileName <<" written."<<endl;
  
//   cout<<"End of bias measure."<<endl;
//   outRootFile->Close();
//   delete outRootFile;
//   return;
// }





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
      
//       if (m_methodStats == 3)  DrawPlot(m_mapBias[histName], {m_mapDataSet[histName], m_mapGauss[histName]}, path+histName,{vectOptDraw} );
      
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
// 	      mapXMin.insert(pair<int, double>(iBin, resultMatrix[iBin][0]-m_mapInput[toyNumber]));
// 	      mapXMax.insert(pair<int, double>(iBin, resultMatrix[iBin][0]-m_mapInput[toyNumber]));
// 	    }
// 	  else
// 	    {
// 	      if (resultMatrix[iBin][0]-m_mapInput[toyNumber]<mapXMin[iBin]) mapXMin[iBin]=resultMatrix[iBin][0]-m_mapInput[toyNumber];
// 	      if (resultMatrix[iBin][0]-m_mapInput[toyNumber]>mapXMax[iBin]) mapXMax[iBin]=resultMatrix[iBin][0]-m_mapInput[toyNumber];
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
// 	      mapHistBias[iBin]->Fill(m_mapCi[toyNumber][iBin][0]-m_mapInput[toyNumber]);
// 	      mapHistBias[iBin]->SetBinError(mapHistBias[iBin]->GetXaxis()->FindBin(m_mapCi[toyNumber][iBin][0]), m_mapErrCi[toyNumber][iBin][0]);
// 	    }
// 	  else 
// 	    {
// 	      mapHistBias[iBin]->Fill(m_mapCi[toyNumber][iBin][0]-m_mapInput[toyNumber]);
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
