#include "PlotFunctions/CatalogFunctions.h"
using namespace ChrisLib;

#include <boost/program_options.hpp>
namespace po = boost::program_options;

#include <string>
using std::string;
#include <iostream>
using std::cout;
using std::endl;

int main( int argc, char* argv[] ) {
  po::options_description desc("LikelihoodProfiel Usage");

  int mode;
  string confFile;
  desc.add_options()
    ("help", "Display this help message")
    ("mode", po::value<int>(&mode)->default_value(0), "" )
    ("confFile", po::value<string>(&confFile)->default_value("") , "" )
    ;

  // Create a map vm that contains options and all arguments of options       
  po::variables_map vm;
  po::store(po::command_line_parser(argc, argv).options(desc).style(po::command_line_style::unix_style ^ po::command_line_style::allow_short).run(), vm);
  po::notify(vm);
  
  if (vm.count("help")) {cout << desc; return 0;}
  //########################################

  if ( mode == 0 ) CompareSystModel();

  return 0;
}
