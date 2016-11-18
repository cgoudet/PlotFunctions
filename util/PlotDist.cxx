#include "PlotFunctions/SideFunctionsTpp.h"
#include "PlotFunctions/PlotFunctions.h"

#include <boost/program_options.hpp>

#include <iostream>

using std::string;
using std::cout;
using std::endl;
using std::vector;
using std::list;
using std::runtime_error;
using std::invalid_argument;

namespace po = boost::program_options;
using namespace ChrisLib;

/**                                                                                                                 
   \brief Dispatch configuration files to plotting functions.                                                       
                                                                                                                    
   Read file as a boost configuration file, call the proper histogram-creating function, then call DrawPlot.
   Configuration files are read into an InputCompare object and are dispatch according to the value of its variable inputCompare.
   Documentation for the configurations options are available in ChrisLib::InputCompare.

*/

int main( int argc, char* argv[] ) {

  po::options_description desc("LikelihoodProfiel Usage");

  vector<string> inFiles;
  //define all options in the program
  desc.add_options()
    ( "help", "Display this help message")
    ( "inFiles", po::value<vector <string> >(&inFiles), "" )
    ;
  
  //Define options gathered by position                                                          
  po::positional_options_description p;
  p.add("inFiles", -1);
  
  // create a map vm that contains options and all arguments of options       
  po::variables_map vm;
  po::store(po::command_line_parser(argc, argv).options(desc).positional(p).style(po::command_line_style::unix_style ^ po::command_line_style::allow_short).run(), vm);
  po::notify(vm);
  
  if (vm.count("help")) {cout << desc; return 0;}
  //=============================================
  
  for ( unsigned int iFile = 0; iFile < inFiles.size(); iFile++ ) {
    cout << "iFile : " << iFile << " " << inFiles[iFile] << endl;
    InputCompare input( inFiles[iFile] );
    int inputType = atoi(input.GetOption("inputType").c_str());
    cout << "inputType : " << inputType << endl;
    const vector<vector<string>> &rootFilesName = input.GetRootFilesName();

    vector<vector<TH1*>> vectHist;
    vector<vector<TGraphErrors*>> vectGraph;
    vector<vector<TObject*>> vectObj;
    try {
      if ( rootFilesName.empty() ) throw invalid_argument( "PlotTree : No input file." );
      if ( inputType==0 ) PlotHist( input, vectObj );
      else if ( inputType<5 ) PlotTree( input, vectObj );
      else if ( inputType<9 ) PlotTextFile( input, vectObj );
      else if ( inputType==9 ) SplitTree( input );
      else if ( inputType==10 ) PlotMatrix( input, vectObj );
      else throw invalid_argument( "CompareHist : No valid inputType provided." );

      if ( !vectObj.empty() ) DrawVect( vectObj, input );

    }
    catch( const invalid_argument &e ) {
      cout << e.what() << endl;
    }
    catch( const runtime_error &e ) {
      cout << e.what() << endl;
    }
    for ( auto it=vectObj.begin(); it!=vectObj.end(); ++it ) DeleteContainer( *it );
      
  }

}
