#include "PlotFunctions/CatalogFunctions.h"
#include "PlotFunctions/SideFunctions.h"

using namespace ChrisLib;

#include <boost/program_options.hpp>
namespace po = boost::program_options;

#include <vector>
#include <iostream>
#include <string>
#include <algorithm>
using std::string;
using std::for_each;
using std::cout;
using std::endl;
using std::vector;

int main( int argc, char* argv[] ) {
  po::options_description desc("LikelihoodProfiel Usage");

  int mode;
  vector<string> inputFiles;
  desc.add_options()
    ("help", "Display this help message")
    ("mode", po::value<int>(&mode)->default_value(0), "" )
    ( "inputFile", po::value< vector<string> >( &inputFiles )->multitoken(), "" )
    ;

  // create a map vm that contains options and all arguments of options
  po::positional_options_description p;
  p.add("inputFile", -1);

  // create a map vm that contains options and all arguments of options
  po::variables_map vm;
  po::store(po::command_line_parser(argc, argv).options(desc).positional(p).style(po::command_line_style::unix_style ^ po::command_line_style::allow_short).run(), vm);
  po::notify(vm);
  
  if (vm.count("help")) {cout << desc; return 0;}
  //########################################
  if ( mode == 0 ) CompareSystModel();
  else if ( mode==1 ) {
    for_each( inputFiles.begin(), inputFiles.end(), DiffSystematics );
    cout<<"DiffSyst done.\n";
  }
  return 0;
}
