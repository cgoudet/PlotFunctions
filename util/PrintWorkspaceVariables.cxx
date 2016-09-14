#include <iostream>
#include "TFile.h"
#include <string>
#include <boost/program_options.hpp>
#include "RooWorkspace.h"
#include "RooArgSet.h"
#include <fstream>
#include "TIterator.h"
#include "RooRealVar.h"
#include "PlotFunctions/SideFunctions.h"

namespace po = boost::program_options;

using std::string;
using std::cout;
using std::endl;
using std::fstream;
void Style_Christophe();


int main( int argc, char* argv[] ) {
  Style_Christophe();

  po::options_description desc("PrintWorspaceVariables");

  string inFileName, outName;
  vector<string> functionsName, modes;
  //define all options in the program
  desc.add_options()
    ("help", "Display this help message")
    ("mode", po::value< vector< string > >(&modes), "" )
    ("inFile", po::value<string>(&inFileName), "" )
    ("function", po::value<vector<string>>(&functionsName)->multitoken(), "" )
    ("outName", po::value<string>(&outName)->default_value("/sps/atlas/c/cgoudet/Plots/PrintWorkspaceVariables"), "" )
    ;
  
  //Define options gathered by position                                                          
  po::positional_options_description p;

  // create a map vm that contains options and all arguments of options       
  po::variables_map vm;
  po::store(po::command_line_parser(argc, argv).options(desc).positional(p).style(po::command_line_style::unix_style ^ po::command_line_style::allow_short).run(), vm);
  po::notify(vm);
  
  if (vm.count("help")) {cout << desc; return 0;}
  //===========================================
  
  if ( !vm.count("inFile") ) { cout << "no input file given" << endl; exit(0); }
  for ( auto vMode : modes ) {
    if ( vMode == "corrModel" ) {
      vector<string> categories = { "ggH_CenLow", "ggH_CenHigh", "ggH_FwdLow", "ggH_FwdHigh", "VBFloose", "VBFtight", "VHMET", "VHlep", "VHdilep", "VHhad_loose", "VHhad_tight",  "ttHhad", "ttHlep" };
      vector<string> processes = {"ggH", "VBF", "VH", "ttH", "bbH", "tHjb", "tWH", "all" };
      vector<string> sensitiveParameters = {"yieldFactor", "meanCB", "sigmaCB"};
      vector<vector<string>> configurations = { sensitiveParameters, processes, categories};
      PrintWorkspaceCorrelationModel( inFileName, outName+"_corrModel.csv", configurations,  "systematic" );
    }
    else if ( vMode == "printWS" ) {
      PrintWorkspaceVariables( inFileName, outName+"_printWS.csv", functionsName );
    }
  }

  return 0;
}
