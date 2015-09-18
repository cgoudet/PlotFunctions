#include <iostream>
#include "PlotFunctions/SideFunctions.h"
#include "PlotFunctions/InputCompare.h"
#include <fstream>
#include <boost/program_options.hpp>

namespace po = boost::program_options;
using std::ifstream;

InputCompare::InputCompare() : m_inputType( 0 ), m_doRatio( 0 ), m_normalize(0), m_doChi2( 0 ), m_centerZoom( 0 ),
			       m_drawStyle(0)
{}

//##################################
InputCompare::InputCompare( string fileName ) : InputCompare()
{



  string legendPos, rangeUser, inLatexPos, varMin, varMax, eventID;
  vector< string > rootFileName, objName, latexOpt, varName;

  po::options_description configOptions("configOptions");
  configOptions.add_options()
    ( "rootFileName", po::value< vector< string > >( &rootFileName )->multitoken(), "" )
    ( "objName", po::value< vector< string > >( &objName )->multitoken(), "" )
    ( "legend", po::value< vector< string > >( &m_legend )->multitoken(), "" )
    ( "legendPos", po::value<string>( &legendPos ), "" )
    ( "rangeUser", po::value<string>( &rangeUser ), "" )
    ( "inputType", po::value<unsigned int>( &m_inputType ),"" )
    ( "doRatio", po::value<unsigned int>( &m_doRatio ), "" )
    ( "normalize", po::value<unsigned int>( &m_normalize ), "" )
    ( "doChi2", po::value<unsigned int>( &m_doChi2 ), "" )
    ( "centerZoom", po::value<unsigned int>( &m_centerZoom ), "" )
    ( "varName", po::value< vector<string> >( &varName )->multitoken(), "" )
    ( "varMin", po::value< string >( &varMin ), "" )
    ( "varMax", po::value< string >( &varMax ), "" )
    ( "latex", po::value< vector< string > >( &m_latex )->multitoken(), "" )
    ( "latexOpt", po::value< vector< string > >( &latexOpt )->multitoken(), "")
    ( "drawStyle", po::value< unsigned int >( &m_drawStyle ), "" )
    ( "selectionCut", po::value< vector< string > >( & m_selectionCut ), "TFormula to select tree events" )
    ( "eventID", po::value< string >( &eventID ), "" )
    ( "nComparedEvents", po::value< unsigned int >( &m_nComparedEvents ), "" )
    ;
  
  po::variables_map vm;
  ifstream ifs( fileName, ifstream::in );
  po::store(po::parse_config_file(ifs, configOptions), vm);
  po::notify( vm );

  m_outName = StripString( fileName );

  //Some checks
  if ( rootFileName.size() != objName.size() 
       || ( m_legend.size() && rootFileName.size() != m_legend.size() )
       ) {
    cout << "Wrong vector sizes" << endl;
    exit( 0 );
  }

  for ( unsigned int iHist = 0; iHist < rootFileName.size(); iHist++ ) {
    m_rootFileName.push_back( vector< string >() );
    ParseVector( rootFileName[iHist], m_rootFileName[iHist] );
    m_objName.push_back( vector< string >() );
    ParseVector( objName[iHist], m_objName[iHist] );
  }

  ParseVector( eventID, m_eventID );
  ParseVector( varMax, m_varMax );
  ParseVector( varMin, m_varMin );

  if ( m_inputType && varName.size() != rootFileName.size() && varName.size() != 1 ) {
    cout << "varName does not have right size. It must be either 1 (if all trees have same branch name) or equal to number of input trees." << endl;
    exit(1);
  }
  cout << "m_inputType : " << m_inputType << endl;
  unsigned int nVariables = 0;
  for ( unsigned int iName = 0; iName < ( varName.size() ? rootFileName.size() : 0 ); iName++ ) {
    if ( iName <= varName.size() ) {
      m_varName.push_back( vector<string>() );
      ParseVector( varName[iName], m_varName.back() );
      if ( !nVariables ) nVariables = m_varName.back().size();
      if ( m_varName.back().size() != nVariables || !nVariables ) {
	cout << "number of variable in each tree do not match or no variable at all" << endl;
	exit(2);
      }
    }
    else m_varName.push_back( m_varName.front() );
  }

  ParseVector( legendPos, m_legendPos );
  if ( m_legendPos.size() && m_legendPos.size() != 4 ) {
    cout << "Wrong size for legendPos" << endl;
    exit(0);
  }

  ParseVector( rangeUser, m_rangeUser );
  if ( m_rangeUser.size() && ( m_rangeUser.size() !=2 && m_rangeUser.size()!=4 ) ) {
    cout << "Wrong size of rangeUser" << endl;
    exit(0);
  }


  if ( m_latex.size() != latexOpt.size() ) {
    cout << "laetx names and options have different sizes" << endl;
    exit(0);
  }
  for ( unsigned int iLatex = 0; iLatex < m_latex.size(); iLatex++ ) {
    m_latexOpt.push_back( vector< double >() );
    ParseVector( latexOpt[iLatex], m_latexOpt[iLatex] );
  }

  if ( m_selectionCut.size() && m_selectionCut.size() != m_rootFileName.size() ) {
    cout << "selectionCuts have non-zero size and diferent from rootFileName" << endl;
    exit(0);
  }

}
