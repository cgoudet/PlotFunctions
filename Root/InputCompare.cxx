#define DEBUG 0
#include "PlotFunctions/SideFunctions.h"
#include "PlotFunctions/SideFunctionsTpp.h"
#include "PlotFunctions/InputCompare.h"

#include <boost/program_options.hpp>
namespace po = boost::program_options;

#include <iostream>
#include <fstream>
#include <stdexcept>
#include <list>

using std::runtime_error;
using std::invalid_argument;
using std::ifstream;
using std::vector;
using std::string;
using std::cout;
using std::endl;
using std::list;
using namespace ChrisLib;

ChrisLib::InputCompare::InputCompare() 
{
}

//##################################
ChrisLib::InputCompare::InputCompare( string fileName ) : InputCompare()
{
  LoadFile( fileName );
  string name = m_outName;
  while( m_loadFiles.size() ) {
    string dumName = m_loadFiles.front();
    m_loadFiles.erase( m_loadFiles.begin() );
    LoadFile( dumName );
  }
    m_outName=name;
    cout << "rootfilesname : " << m_rootFilesName.size() << endl;
}

//###########################
/**
   \brief Load the configuration file using the boost::program_options format.
 */
void  ChrisLib::InputCompare::LoadFile( string fileName ) {
  string inLatexPos, varMin, varMax, eventID;
  vector< string > rootFilesName, objName, varName, varWeight, latex, latexOpt, xBinning, varErrX, varErrY, varYName;
  po::options_description configOptions("configOptions");
  configOptions.add_options()
    ( "rootFileName", po::value< vector< string > >( &rootFilesName )->multitoken(), "" )
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
    ( "varYName", po::value< vector<string> >( &varYName )->multitoken(), "" )
    ( "varErrX", po::value< vector<string> >( &varErrX )->multitoken(), "" )
    ( "varErrY", po::value< vector<string> >( &varErrY )->multitoken(), "" )
    ( "varMin", po::value< string >( &varMin ), "" )
    ( "varMax", po::value< string >( &varMax ), "" )
    ( "latex", po::value< vector< string > >( &latex )->multitoken(), "" )
    ( "latexOpt", po::value< vector<string> >( &latexOpt )->multitoken(), "")
    ( "drawStyle", po::value< string >( &m_mapOptions["drawStyle"] ), "" )
    ( "selectionCut", po::value< vector< string > >( & m_selectionCut ), "TFormula to select tree events" )
    ( "eventID", po::value< string >( &eventID ), "" )
    ( "nEvents", po::value< string >( &m_mapOptions["nEvents"] ), "" )
    ( "nBins", po::value< string >( &m_mapOptions["nBins"] ), "" )
    ( "varWeight", po::value< vector<string> >(&varWeight)->multitoken(), "" )
    ( "shiftColor", po::value< string >( &m_mapOptions["shiftColor"] ), "" )
    ( "line", po::value<string>( &m_mapOptions["line"] ), "" )
    ( "triangular", po::value<string>( &m_mapOptions["triangular"] ), "" )
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
    ( "offset", po::value<string>( &m_mapOptions["offset"] ), "" )
    ( "orderX", po::value<string>( &m_mapOptions["orderX"] ), "" )
    ( "clean", po::value<string>( &m_mapOptions["clean"] ), "" )
    ( "extension", po::value<string>( &m_mapOptions["extension"] ), "" )
    ( "saveRoot", po::value<string>( &m_mapOptions["saveRoot"] ), "" )
    ( "doLabels", po::value<string>( &m_mapOptions["doLabels"]) , "" )
    ;

  po::variables_map vm;
  ifstream ifs( fileName, ifstream::in );
  po::store(po::parse_config_file(ifs, configOptions), vm);
  po::notify( vm );

  m_outName = StripString( fileName );

  if ( DEBUG ) cout << "plotDirectory : " << m_mapOptions["plotDirectory"] << endl;
  if ( m_mapOptions["plotDirectory"] != "" && m_mapOptions["plotDirectory"].back() != '/' ) m_mapOptions["plotDirectory"] += "/";

  if ( DEBUG ) cout << "rootFilesName" << endl;
  unsigned nPlots = rootFilesName.size();
  m_rootFilesName = vector<vector<string>>( nPlots );
  for ( unsigned int iHist = 0; iHist < nPlots; ++iHist ) ParseVector( rootFilesName[iHist], m_rootFilesName[iHist], 0 );

  if ( DEBUG ) cout << "objName" << endl;
  m_objName = vector<vector<string>>( nPlots );
  for ( unsigned int iHist = 0; iHist < objName.size(); ++iHist ) ParseVector( objName[iHist], m_objName[iHist], 0 );

  if ( DEBUG ) cout << "varName" << endl;
  m_varName = vector<vector<string>>( varName.size() );
  for ( unsigned int iName = 0; iName < varName.size(); ++iName ) {
    ParseVector( varName[iName], m_varName[iName], 0 );
    if ( m_varName[iName].size() != m_varName[0].size() ) throw runtime_error( "InputConpare::LoadFiles : varName structure not identical for all files." );
  }
  while ( m_varName.size() && m_varName.size() < nPlots ) m_varName.push_back( m_varName.back() );

  if ( DEBUG ) cout << "varYName" << endl;
  cout << "varYName.size() : " << varYName.size() << endl;
  m_varYName = vector<vector<string>>( varYName.size() );
  for ( unsigned int iYName = 0; iYName < varYName.size(); ++iYName ) {
    ParseVector( varYName[iYName], m_varYName[iYName], 0 );
    if ( m_varYName[iYName].size() != m_varYName[0].size() ) throw runtime_error( "InputConpare::LoadFiles : varYName structure not identical for all files." );
  }
  while ( m_varYName.size() && m_varYName.size() < nPlots ) m_varYName.push_back( m_varYName.back() );


  m_varWeight = vector<vector<string>>( nPlots );
  for ( unsigned int iPlot = 0; iPlot < varWeight.size(); iPlot++ ) ParseVector( varWeight[iPlot], m_varWeight[iPlot], 0 );

  m_varErrX = vector<vector<string>>( nPlots );
  for ( unsigned int iPlot = 0; iPlot < varErrX.size(); iPlot++ ) ParseVector( varErrX[iPlot], m_varErrX[iPlot], 0 );
  m_varErrY = vector<vector<string>>( nPlots );
  for ( unsigned int iPlot = 0; iPlot < varErrY.size(); iPlot++ ) ParseVector( varErrY[iPlot], m_varErrY[iPlot], 0 );

  if ( eventID != "" ) ParseVector( eventID, m_eventID, 0 );


  if ( varMax != "" ) {
    ParseVector( varMax, m_varMax, 0 );
    while ( m_varName.size() && m_varMax.size() < m_varName.front().size() ) m_varMax.push_back( m_varMax.back() );
  }

  if ( varMin != "" ) {
    ParseVector( varMin, m_varMin, 0 );
    while ( m_varName.size() && m_varMin.size() < m_varName.front().size() ) m_varMin.push_back( m_varMin.back() );
  }
  //  if ( m_varMin.size() != m_varMax.size() ) throw invalid_argument( "InputCompare::LoadFiles : VarMin and VarMax Variables must be either both empty or both filled" );

  m_xBinning = vector<vector<double>>( xBinning.size(), vector<double>() );
  for ( unsigned int iPlot = 0; iPlot < xBinning.size(); ++iPlot ) ParseVector( xBinning[iPlot], m_xBinning[iPlot], 0 );
  while ( xBinning.size() && xBinning.size() < m_rootFilesName.size() ) xBinning.push_back( xBinning.back() );

  if ( DEBUG ) cout << "latex" << endl;
  for ( auto vLatex = latex.begin(); vLatex!= latex.end(); vLatex++ ) m_latex.push_back( *vLatex );
  for ( auto vLatexOpt = latexOpt.begin(); vLatexOpt!= latexOpt.end(); vLatexOpt++ ) m_latexOpt.push_back( *vLatexOpt );
  if ( m_latex.size() != m_latexOpt.size() ) {
    cout << "laetx names and options have different sizes" << endl;
    exit(0);
  }
}

//==========================================
vector<string> ChrisLib::InputCompare::CreateVectorOptions() const {

    list<string> nonDrawOptions = { "", "inputType", "diagonalize", "doTabular", "plotDirectory", "saveRoot", "nEvents", "nBins", "doLabels" };

  vector<string> outVect;
  for ( map<string, string>::const_iterator it = m_mapOptions.begin(); it != m_mapOptions.end(); it++) {
    if ( it->second == "" ) continue;
    if ( find ( nonDrawOptions.begin(), nonDrawOptions.end(), it->first ) != nonDrawOptions.end() ) continue;
    outVect.push_back( it->first +"=" + it->second );
  }

  for ( auto legend : m_legend ) outVect.push_back( "legend=" + legend );
  for ( auto latex : m_latex ) outVect.push_back( "latex=" + latex );
  for ( auto latexOpt : m_latexOpt ) outVect.push_back( "latexOpt=" + latexOpt );

  return outVect;


}
