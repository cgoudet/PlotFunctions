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

//using std::fstream;
namespace po = boost::program_options;
using namespace ChrisLib;

int main( int argc, char* argv[] ) {

  po::options_description desc("LikelihoodProfiel Usage");

  vector<string> inFiles;
  string outFileName;
  //define all options in the program
  desc.add_options()
    ( "help", "Display this help message")
    ( "inFiles", po::value<vector <string> >(&inFiles), "" )
    ( "outFileName", po::value<string>( &outFileName )->default_value(""), "" )
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
    
    try {
      if ( rootFilesName.empty() ) throw invalid_argument( "PlotTree : No input file." );
      if ( inputType==0 ) PlotHist( input, vectHist );
      else if ( inputType<3 ) PlotTree( input, vectHist, vectGraph );
      else if ( inputType==4 ) PlotTextFile( input, vectHist );
      else if ( inputType==5 ) SplitTree( input );
      else throw invalid_argument( "CompareHist : No valid inputType provided." );
      
      DrawVect( vectHist, input );
    }
    catch( const invalid_argument &e ) {
      cout << e.what() << endl;
    }
    catch( const runtime_error &e ) {
      cout << e.what() << endl;
    }
    for ( auto it=vectHist.begin(); it!=vectHist.end(); ++it ) DeleteContainer( *it );
    for ( auto it=vectGraph.begin(); it!=vectGraph.end(); ++it ) DeleteContainer( *it );  
      
  }

}