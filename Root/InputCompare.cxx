#include <iostream>
#include "PlotFunctions/SideFunctions.h"
#include "PlotFunctions/InputCompare.h"
#include <fstream>
#include <boost/program_options.hpp>

namespace po = boost::program_options;
using std::ifstream;


InputCompare::InputCompare() 
{
}

//##################################
InputCompare::InputCompare( string fileName ) : InputCompare()
{
  LoadFile( fileName );
  string name = m_outName;
  while( m_loadFiles.size() ) {
    string dumName = m_loadFiles.front();
    m_loadFiles.erase( m_loadFiles.begin() );
    LoadFile( dumName );
  }
    m_outName=name;
    cout << "rootFileName : " << m_rootFileName.size() << endl;
}

/**
   Read a configuration file.
   Options not documented here are documented in DrawPlot.cxx.

   rootFileName= file1 file2 ... \n
   Names of the rootFiles whose objects will be added.
   \n
   \n
   objName= name1 name2 ... \n
   Names of the objects to be added.
   \n
   \n
   inputType=number\n
   Number of the rountine in CompareHist to call. Routines and requirede parameters defined in CompareHist.cxx.
   \n
   \n
   varName= name1 name2 ...\n
   Names of tree branches to be read
   \n
   \n
   varMin= value1 value2 ... \n
   Low range limit for histograms when reading a ttre. Several values correspond to different variable/
   \n
   \n
   varMax= value1 value2 ...  \n
   High range limit for histograms when reading a TTree.
   \n
   \n
   selectionCut= cut \n
   Perform the given cut on a TTree with same index.
   \n
   \n
   eventID= value1 value2 ...\n
   branch names of variables used to identify an event.
   \n
   \n
   nComparedEvents=value1 \n
   Integer with various use depending on routine.
   \n
   \n
   varWeight= var1 var2 ...\n
   branch names to be used as weights. If several values, they are multiplied.
   \n
   \n
   xBinning= frontier1 frontier2 ... \n
   When plotting a tree, one can decide a non regular binning. Number of bins will be the number of entries in xBinning -1.
   \n
   \n
   path= value \n
   Directory in which to write the plot
 */
//###########################

void  InputCompare::LoadFile( string fileName ) {
  string inLatexPos, varMin, varMax, eventID;
  vector< string > rootFileName, objName, varName, varWeight, latex, latexOpt, xBinning;
  po::options_description configOptions("configOptions");
  configOptions.add_options()
    ( "rootFileName", po::value< vector< string > >( &rootFileName )->multitoken(), "" )
    ( "objName", po::value< vector< string > >( &objName )->multitoken(), "" )
    ( "legend", po::value< vector< string > >( &m_legend )->multitoken(), "" )
    ( "legendPos", po::value<string>( &m_mapOptions["legendPos"] ), "" )
    ( "rangeUserX", po::value<string>( &m_mapOptions["rangeUserX"] ), "" )
    ( "rangeUserY", po::value<string>( &m_mapOptions["rangeUserY"] ), "" )
    ( "inputType", po::value<string>( &m_mapOptions["inputType"] ),"" )
    ( "doRatio", po::value<string>( &m_mapOptions["doRatio"] ), "" )
    ( "normalize", po::value<string>( &m_mapOptions["normalize"] ), "" )
    ( "doChi2", po::value<string>( &m_mapOptions["doChi2"] ), "" )
    ( "centerZoom", po::value<string>( &m_mapOptions["centerZoom"] ), "" )
    ( "varName", po::value< vector<string> >( &varName )->multitoken(), "" )
    ( "varMin", po::value< string >( &varMin ), "" )
    ( "varMax", po::value< string >( &varMax ), "" )
    ( "latex", po::value< vector< string > >( &latex )->multitoken(), "" )
    ( "latexOpt", po::value< vector<string> >( &latexOpt )->multitoken(), "")
    ( "drawStyle", po::value< string >( &m_mapOptions["drawStyle"] ), "" )
    ( "selectionCut", po::value< vector< string > >( & m_selectionCut ), "TFormula to select tree events" )
    ( "eventID", po::value< string >( &eventID ), "" )
    ( "nComparedEvents", po::value< string >( &m_mapOptions["nComparedEvents"] ), "" )
    ( "varWeight", po::value< vector<string> >(&varWeight)->multitoken(), "" )
    ( "shiftColor", po::value< string >( &m_mapOptions["shiftColor"] ), "" )
    ( "line", po::value<string>( &m_mapOptions["line"] ), "" )
    ( "diagonalize", po::value<string>( &m_mapOptions["diagonalize"] ), "" )
    ( "loadFiles", po::value< vector<string > >( &m_loadFiles )->multitoken(), "" )
    ( "extendUp", po::value<string>( &m_mapOptions["extendUp"] ), "" )
    ( "xBinning", po::value< vector<string> >( &xBinning )->multitoken(), "" )
    ( "xTitle", po::value<string>( &m_mapOptions["xTitle"] ), "" )
    ( "yTitle", po::value<string>( &m_mapOptions["yTitle"] ), "" )
    ( "doTabular", po::value<string>( &m_mapOptions["doTabular"] ), "" )
    ( "logy", po::value<string>( &m_mapOptions["logy"] ), "" )
    ( "stack", po::value<string>( &m_mapOptions["stack"]), "" )
    ( "removeVal", po::value<string>( &m_mapOptions["removeVal"] ), "" )
    ( "plotDirectory", po::value<string>( &m_mapOptions["plotDirectory"] ), "" )
    ;

  po::variables_map vm;
  ifstream ifs( fileName, ifstream::in );
  po::store(po::parse_config_file(ifs, configOptions), vm);
  po::notify( vm );

  m_outName = StripString( fileName );
  
  if ( m_mapOptions["plotDirectory"] != "" && m_mapOptions["plotDirectory"].back() != '/' ) m_mapOptions["plotDirectory"] += "/";

  for ( unsigned int iHist = 0; iHist < rootFileName.size(); iHist++ ) {
    m_rootFileName.push_back( vector< string >() );
    ParseVector( rootFileName[iHist], m_rootFileName.back(), 0 );
  }
  
  for ( unsigned int iHist = 0; iHist < objName.size(); iHist++ ) {
    m_objName.push_back( vector< string >() );
    ParseVector( objName[iHist], m_objName.back(), 0 );
  }


  for ( unsigned int iName = 0; iName < varName.size(); iName++ ) {
    m_varName.push_back( vector<string>() );
    ParseVector( varName[iName], m_varName.back(), 0 );
  }

  while ( m_varName.size() && m_varName.size() < m_rootFileName.size() ) m_varName.push_back( m_varName.back() );

  for ( unsigned int iPlot = 0; iPlot < m_varName.size(); iPlot++ ) {
    if ( m_varName.front().size() <= m_varName[iPlot].size() ) continue;
    cout << "Missing variables for dataset " << iPlot << endl;
    exit(0);
  }

  if ( eventID != "" ) ParseVector( eventID, m_eventID, 0 );
  if ( varMax != "" ) {
    ParseVector( varMax, m_varMax, 0 );
    while ( m_varMax.size() < m_varName.front().size() ) m_varMax.push_back( m_varMax.back() );
  }

  if ( varMin != "" ) {
    ParseVector( varMin, m_varMin, 0 );
    while ( m_varMin.size() < m_varName.front().size() ) m_varMin.push_back( m_varMin.back() );
  }

  for ( unsigned int iPlot = 0; iPlot < xBinning.size(); iPlot++ ) {
    m_xBinning.push_back( vector<double>() );
    ParseVector( xBinning[iPlot], m_xBinning.back(), 0 );
  }
  //  while ( m_xBinning.size() && m_xBinning.size() < m_varName.front().size() ) m_xBinning.push_back( m_xBinning.back() );

  for ( unsigned int iPlot = 0; iPlot < varWeight.size(); iPlot++ ) {
    m_varWeight.push_back( vector<string>() );
    ParseVector( varWeight[iPlot], m_varWeight.back(), 0 );
  }

  for ( auto vLatex = latex.begin(); vLatex!= latex.end(); vLatex++ ) m_latex.push_back( *vLatex );
  for ( auto vLatexOpt = latexOpt.begin(); vLatexOpt!= latexOpt.end(); vLatexOpt++ ) m_latexOpt.push_back( *vLatexOpt );
  if ( m_latex.size() != m_latexOpt.size() ) {
    cout << "laetx names and options have different sizes" << endl;
    exit(0);
  }

  if ( m_selectionCut.size() && m_selectionCut.size() != m_rootFileName.size() ) {
    cout << "selectionCuts have non-zero size and diferent from rootFileName" << endl;
    exit(0);
  }


}


vector<string> InputCompare::CreateVectorOptions() {

  vector<string> outVect;
  for ( map<string, string>::iterator it = m_mapOptions.begin(); it != m_mapOptions.end(); it++) {
    if ( it->second == "" 
	 || it->first == "inputType" 
	 || it->first == "nComparedEvents" 
	 || it->first == "diagonalize" 
	 || it->first == "doTabular" 
	 || it->first == "plotDirectory" 
	 ) continue;
    outVect.push_back( it->first +"=" + it->second );
  }
  for ( auto legend : m_legend ) outVect.push_back( "legend=" + legend );
  for ( auto latex : m_latex ) outVect.push_back( "latex=" + latex );
  for ( auto latexOpt : m_latexOpt ) outVect.push_back( "latexOpt=" + latexOpt );


  return outVect;


}
