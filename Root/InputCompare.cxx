#include <iostream>
#include "PlotFunctions/SideFunctions.h"
#include "PlotFunctions/InputCompare.h"
#include <fstream>
#include <boost/program_options.hpp>

namespace po = boost::program_options;
using std::ifstream;


InputCompare::InputCompare() 
{
  m_mapOptions["inputType"]="";
  m_mapOptions["doRatio"]="";
  m_mapOptions["normalize"]="";
  m_mapOptions["doChi2"]="";
  m_mapOptions["centerZoom"]="";
  m_mapOptions["drawStyle"]="";
  m_mapOptions["shiftColor"]="";
  m_mapOptions["nComparedEvents"]="";
  m_mapOptions["legendPos"]="";
  m_mapOptions["rangeUserX"]="";
  m_mapOptions["rangeUserY"]="";
  m_mapOptions["line"]="";
  m_mapOptions["diagonalize"]="";
  m_mapOptions["extendUp"]="";
}

//##################################
InputCompare::InputCompare( string fileName ) : InputCompare()
{
  LoadFile( fileName );
  string name = m_outName;
  while( m_loadFiles.size() ) {
    string dumName = m_loadFiles.back();
    m_loadFiles.pop_back();
    LoadFile( dumName );
  }
  m_outName=name;
}

//###########################
void  InputCompare::LoadFile( string fileName ) {
  string inLatexPos, varMin, varMax, eventID, xBinning;
  vector< string > rootFileName, objName, varName, varWeight;

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
    ( "latex", po::value< vector< string > >( &m_latex )->multitoken(), "" )
    ( "latexOpt", po::value< vector<string> >( &m_latexOpt )->multitoken(), "")
    ( "drawStyle", po::value< string >( &m_mapOptions["drawStyle"] ), "" )
    ( "selectionCut", po::value< vector< string > >( & m_selectionCut ), "TFormula to select tree events" )
    ( "eventID", po::value< string >( &eventID ), "" )
    ( "nComparedEvents", po::value< string >( &m_mapOptions["nComparedEvents"] ), "" )
    ( "varWeight", po::value< vector<string> >(&m_varWeight)->multitoken(), "" )
    ( "shiftColor", po::value< string >( &m_mapOptions["shiftColor"] ), "" )
    ( "line", po::value<string>( &m_mapOptions["line"] ), "" )
    ( "diagonalize", po::value<string>( &m_mapOptions["diagonalize"] ), "" )
    ( "loadFiles", po::value< vector<string > >( &m_loadFiles )->multitoken(), "" )
    ( "extendUp", po::value<string>( &m_mapOptions["extendUp"] ), "" )
    ( "xBinning", po::value< string >( &xBinning ), "" )
    ;
  
  po::variables_map vm;
  ifstream ifs( fileName, ifstream::in );
  po::store(po::parse_config_file(ifs, configOptions), vm);
  po::notify( vm );

  m_outName = StripString( fileName );


  for ( unsigned int iHist = 0; iHist < rootFileName.size(); iHist++ ) {
    m_rootFileName.push_back( vector< string >() );
    ParseVector( rootFileName[iHist], m_rootFileName[iHist] );
  }
  
  for ( unsigned int iHist = 0; iHist < objName.size(); iHist++ ) {
    m_objName.push_back( vector< string >() );
    ParseVector( objName[iHist], m_objName[iHist] );
  }


  if ( eventID != "" ) ParseVector( eventID, m_eventID );
  if ( varMax != "" ) ParseVector( varMax, m_varMax );
  if ( varMin != "" ) ParseVector( varMin, m_varMin );
  if ( xBinning !="" ) ParseVector( xBinning, m_xBinning );

  for ( unsigned int iName = 0; iName < varName.size(); iName++ ) {
    m_varName.push_back( vector<string>() );
    ParseVector( varName[iName], m_varName.back() );
  }


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
	 ) continue;
    outVect.push_back( it->first +"=" + it->second );
  }
  for ( auto legend : m_legend ) outVect.push_back( "legend=" + legend );
  for ( auto latex : m_latex ) outVect.push_back( "latex=" + latex );
  for ( auto latexOpt : m_latexOpt ) outVect.push_back( "latexOpt=" + latexOpt );


  return outVect;


}
