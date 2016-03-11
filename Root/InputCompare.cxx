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
    ( "nComparedEvents", po::value< string >( &m_mapOptions["nComparedEvents"] )->default_value("100"), "" )
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
    ;
  
  po::variables_map vm;
  ifstream ifs( fileName, ifstream::in );
  po::store(po::parse_config_file(ifs, configOptions), vm);
  po::notify( vm );

  m_outName = StripString( fileName );
  

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
	 ) continue;
    outVect.push_back( it->first +"=" + it->second );
  }
  for ( auto legend : m_legend ) outVect.push_back( "legend=" + legend );
  for ( auto latex : m_latex ) outVect.push_back( "latex=" + latex );
  for ( auto latexOpt : m_latexOpt ) outVect.push_back( "latexOpt=" + latexOpt );


  return outVect;


}
