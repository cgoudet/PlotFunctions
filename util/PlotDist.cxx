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
    vector<vector<TObject*>> vectObj;
    try {
      if ( rootFilesName.empty() ) throw invalid_argument( "PlotTree : No input file." );
      if ( inputType==0 ) PlotHist( input, vectHist );
      else if ( inputType<5 ) PlotTree( input, vectObj );
      else if ( inputType<9 ) PlotTextFile( input, vectObj );
      else if ( inputType==9 ) SplitTree( input );
      else throw invalid_argument( "CompareHist : No valid inputType provided." );


      list<int> histType = { 0, 1, 2, 3 };
      for ( unsigned int iCan=0; iCan<vectObj.size(); ++iCan ) {
	for ( unsigned iObj=0; iObj<vectObj[iCan].size(); ++iObj ) {
	  if ( find( histType.begin(), histType.end(), inputType ) != histType.end() ) {
	    cout << "hist" << endl;
	    while ( vectHist.size()<=iCan ) vectHist.push_back( vector<TH1*>() );
	    while ( vectHist[iCan].size()<=iObj ) vectHist[iCan].push_back( 0);
	    vectHist[iCan][iObj] = static_cast<TH1*>( vectObj[iCan][iObj] );
	  }
	  else {
	    cout << "graph" << endl;
	    while ( vectGraph.size()<=iCan ) vectGraph.push_back( vector<TGraphErrors*>() );
	    while ( vectGraph[iCan].size()<=iObj ) vectGraph[iCan].push_back( 0);
	    cout << vectObj[iCan][iObj] << endl;
	    if ( vectObj[iCan][iObj] ) vectGraph[iCan][iObj] = static_cast<TGraphErrors*>( vectObj[iCan][iObj] );
	  }
	}}
      if ( !vectHist.empty() ) DrawVect( vectHist, input );
      if ( !vectGraph.empty() ) DrawVect( vectGraph, input );
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
