// #include <iostream>
// #include <istream>
// #include <fstream>
// #include <map>
// #include <vector>
// #include <string>
// #include <sstream>
// #include <boost/program_options.hpp>
// #include <boost/multi_array.hpp>

// #include "TH1.h"
// #include "TFile.h"
// #include "TTree.h"
// #include "TBranch.h"
// #include "TCanvas.h"

// #include "Bias/BiasAnalysis.h"

// using namespace std;
// namespace po = boost::program_options;

// int main(int argc, char *argv[])
// {
  
//   //=================
//   //Define options to read name of files from the command line
//   po::options_description desc("Input data files (.root)");
//   vector<string> dataFiles;
  
//   desc.add_options()
//     ("help", "Display this help message")
//     ("dataFiles", po::value<vector <string> >(&dataFiles), "Absolute path" )
//     ;
                                   
//   po::positional_options_description p;
//   p.add("dataFiles", -1);

//   po::variables_map vm;
//   po::store(po::command_line_parser(argc, argv).options(desc).positional(p).style(po::command_line_style::unix_style ^ po::command_line_style::allow_short).run(), vm);
//   po::notify(vm);
  
//   if (vm.count("help")) {cout << desc; return 0;}


//   //================
//   string BinOrConf;
//   bool isConf=1;

//   if (isConf) BinOrConf= "Conf";
//   else BinOrConf="Bin";

//   BiasAnalysis BA("Bias/ConfigFile/Bias"+BinOrConf+".boost");

//   string path= "/sps/atlas/a/aguerguichon/Calibration/Bias/";
//   string latexFileName= "Bias"+BinOrConf+"_bootstrap0";

//   string csvFile= path+"Stats/"+latexFileName+".csv";

//   BA.SelectVariables(dataFiles);
//   BA.MeasureBias(csvFile, path+"RootFiles/"+latexFileName+".root");

//   path+= "Plots/";
  
//   BA.MakeBiasPlots(csvFile, path, latexFileName, "Bootstrap avec remise (mode0)");

//   string commandLine = "mv ./"+latexFileName+".pdf "+path;
//   system ( commandLine.c_str() );
//   commandLine = "rm "+latexFileName+"*";
//   system ( commandLine.c_str() );
  
//   // path= "/sps/atlas/a/aguerguichon/Calibration/Bias/Inversion/";
//   // BA.InvertCijMatrix(path, 1);
  
//   //BA.MakeCompStatPlot(csvFile, "Bias"+BinOrConf+"CompStat_7000", 7000, 0);
//   //================
//   //End of program
//   cout <<"End of programm."<<endl;
//   return 0;
// }
