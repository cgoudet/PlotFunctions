#include "PlotFunctions/DrawOptions.h"
#include "PlotFunctions/SideFunctionsTpp.h"

#include <iostream>
#include <functional>
#include <algorithm>
#include <list>

using std::cout;
using std::endl;
using std::vector;
using std::string;

using namespace ChrisLib;

ChrisLib::DrawOptions::DrawOptions() {

  std::list<string> keys { "doChi2", "grid", "logy", "offset", "orderX" };
  for ( auto vKey : keys ) m_bools[vKey]=0;

  keys = { "doRatio", "drawStyle", "shiftColor" };
  for ( auto vKey : keys ) m_ints[vKey]=0;  

  keys = { "scale", "line", "clean", "normalize" };
  for ( auto vKey : keys ) m_doubles[vKey]=-99;
  m_doubles["extendUp"]=0;

  keys = { "xTitle", "yTitle" };
  for ( auto vKey : keys ) m_strings[vKey]="";
  m_strings["extension"]="pdf";
}


//==========================================================
void ChrisLib::DrawOptions::AddOption( const string &option ) {

  string key = option.substr( 0, option.find_first_of('=' ) );
  string value = option.substr( option.find_first_of("=")+1);
  if ( m_ints.find(key) != m_ints.end() ) m_ints[key] = std::atoi( value.c_str() );
  else if ( m_strings.find(key) != m_strings.end() ) m_strings[key] = value;
  else if ( m_doubles.find(key) != m_doubles.end() ) m_doubles[key] = std::stod( value.c_str() );
  else if ( key == "legend" ) m_legends.push_back( value );
  else if ( key == "latex" ) m_latex.push_back( value );
  else if ( key == "legendPos" ) ParseVector( value, m_legendCoord );
  else if ( key == "rangeUserX" ) ParseVector( value, m_rangeUserX );
  else if ( key == "rangeUserY" ) ParseVector( value, m_rangeUserY );
  else if ( key == "latexOpt" ) {
    m_latexPos.push_back( vector<double>() );
    ParseVector( value, m_latexPos.back() );
  }
  else cout << "DrawPlotOption : " << key << " not known" << endl;

}

//==========================================================
void ChrisLib::DrawOptions::FillOptions( const vector<string> &options ) {
  std::for_each( options.begin(), options.end(), [this](const string &s ){AddOption(s);});
}
