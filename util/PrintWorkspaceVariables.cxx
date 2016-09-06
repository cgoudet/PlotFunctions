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
  vector<string> functionsNames;
  //define all options in the program
  desc.add_options()
    ("help", "Display this help message")
    ("inFile", po::value<string>(&inFileName), "" )
    ("function", po::value<vector<string>>(&functionsNames)->multitoken(), "" )
    ("outName", po::value<string>(&outName), "" )
    ;
  
  //Define options gathered by position                                                          
  po::positional_options_description p;
  p.add("inFileName", 1);

  // create a map vm that contains options and all arguments of options       
  po::variables_map vm;
  po::store(po::command_line_parser(argc, argv).options(desc).positional(p).style(po::command_line_style::unix_style ^ po::command_line_style::allow_short).run(), vm);
  po::notify(vm);
  
  if (vm.count("help")) {cout << desc; return 0;}
  //===========================================


  TFile *inFile = new TFile( inFileName.c_str() );
  if ( !inFile ) { cout << inFileName << " not found." << endl; exit(0); }

  RooWorkspace *ws = (RooWorkspace*) inFile->Get( FindDefaultTree( inFile, "RooWorkspace" ).c_str() );
  if ( !ws ) { cout << "Workspace not found in " << inFileName << endl; exit(0); }

  RooArgSet allVars = ws->allVars();

  fstream stream;
  string streamName = "/sps/atlas/c/cgoudet/Plots/PrintWorkspaceVariables" + ( outName == "" ? "" : "_"+outName ) + ".csv";
  cout << "Writing in : " << streamName << endl;
  stream.open( streamName.c_str(), fstream::out | fstream::trunc );
  
  TIterator* iter = allVars.createIterator();
  while ( RooRealVar* v = (RooRealVar* ) iter->Next() ) {
    stream << v->GetName() << "," << v->getVal() << "," << v->getError() << endl;
  }

  for ( auto vName : functionsNames ) {
    RooAbsReal *var = ws->function( vName.c_str() );
    if ( var ) stream << vName << "," << var->getVal() << ",0"  << endl;
  }
  stream.close();

  return 0;
}
