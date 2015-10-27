#include <iostream>
#include "PlotFunctions/SideFunctions.h"
#include "PlotFunctions/InputCompare.h"
#include <fstream>
#include <boost/program_options.hpp>

namespace po = boost::program_options;
using std::ifstream;


InputCompare::InputCompare() {
  m_mapOptions["inputType"]="";
  m_mapOptions["doRatio"]="";
  m_mapOptions["normalize"]="";
  m_mapOptions["doChi2"]="";
  m_mapOptions["centerZoom"]="";
  m_mapOptions["drawStyle"]="";
  m_mapOptions["shiftColor"]="";
  m_mapOptions["nComparedEvents"]="";
  m_mapOptions["legendPos"]="";
  m_mapOptions["rangeUser"]="";
}

//##################################
InputCompare::InputCompare( string fileName ) : InputCompare()
{



  string inLatexPos, varMin, varMax, eventID;
  vector< string > rootFileName, objName, latexOpt, varName, varWeight;

  po::options_description configOptions("configOptions");
  configOptions.add_options()
    ( "rootFileName", po::value< vector< string > >( &rootFileName )->multitoken(), "" )
    ( "objName", po::value< vector< string > >( &objName )->multitoken(), "" )
    ( "legend", po::value< vector< string > >( &m_legend )->multitoken(), "" )
    ( "legendPos", po::value<string>( &m_mapOptions["legendPos"] ), "" )
    ( "rangeUser", po::value<string>( &m_mapOptions["rangeUser"] ), "" )
    ( "inputType", po::value<string>( &m_mapOptions["inputType"] ),"" )
    ( "doRatio", po::value<string>( &m_mapOptions["doRatio"] ), "" )
    ( "normalize", po::value<string>( &m_mapOptions["normalize"] ), "" )
    ( "doChi2", po::value<string>( &m_mapOptions["doChi2"] ), "" )
    ( "centerZoom", po::value<string>( &m_mapOptions["centerZoom"] ), "" )
    ( "varName", po::value< vector<string> >( &varName )->multitoken(), "" )
    ( "varMin", po::value< string >( &varMin ), "" )
    ( "varMax", po::value< string >( &varMax ), "" )
    ( "latex", po::value< vector< string > >( &m_latex )->multitoken(), "" )
    ( "latexOpt", po::value< vector<string> >( &latexOpt )->multitoken(), "")
    ( "drawStyle", po::value< string >( &m_mapOptions["drawStyle"] ), "" )
    ( "selectionCut", po::value< vector< string > >( & m_selectionCut ), "TFormula to select tree events" )
    ( "eventID", po::value< string >( &eventID ), "" )
    ( "nComparedEvents", po::value< string >( &m_mapOptions["nComparedEvents"] ), "" )
    ( "varWeight", po::value< vector<string> >(&m_varWeight)->multitoken(), "" )
    ( "shiftColor", po::value< string >( &m_mapOptions["shiftColor"] ), "" )
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


  ParseVector( eventID, m_eventID );
  ParseVector( varMax, m_varMax );
  ParseVector( varMin, m_varMin );

  for ( unsigned int iName = 0; iName < varName.size(); iName++ ) {
    m_varName.push_back( vector<string>() );
    ParseVector( varName[iName], m_varName.back() );
  }


  if ( m_latex.size() != latexOpt.size() ) {
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
    if ( it->first == "inputType" || it->first == "nComparedEvents" ) continue;
    outVect.push_back( it->first +"=" + it->second );
  }
  for ( auto legend : m_legend ) outVect.push_back( "legend=" + legend );
  for ( auto latex : m_latex ) outVect.push_back( "latex=" + latex );
  for ( auto latexOpt : m_latexOpt ) outVect.push_back( "latexOpt=" + latexOpt );


  return outVect;


}
