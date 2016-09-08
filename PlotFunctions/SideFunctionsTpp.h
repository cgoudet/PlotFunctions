#ifndef SIDEFUNCTIONS_TPP
#define SIDEFUNCTIONS_TPP
#include "PlotFunctions/SideFunctions.h"
#include <iostream>
#include <string>
#include <vector>
#include <map>
using std::map;
using std::vector;
using std::stringstream;
using std::string;
using std::cout;
using std::endl;

//============================================
/**\brief Get the number of possible configurations containing a single key per vector line
 */
template<typename Type1 > unsigned int GetNConfigurations( vector<vector<Type1>> &inVect ) {
  unsigned int nConfig = 1;
  for ( auto vList : inVect ) nConfig*=vList.size();
  return nConfig;
}

//============================================
/**\brief Get the size of the each component of a 2D vector
 */
template<typename Type1 > vector<unsigned int> GetLevelsSize( vector<vector<Type1>> &inVect ) {
  vector<unsigned int> outVect;
  for ( auto vList : inVect ) {
    outVect.push_back( vList.size() );
  }
  return outVect;
}
//============================================
/**\brief Plot the content of a vector
 */
template< typename Type1 > void PrintVector( vector< Type1 > &vect ) {
  for ( auto vVal : vect ) cout << vVal << " ";
  cout << endl;
}

//============================================
/**\brief Search for the bin number of an element in a vector
   \param val value to be searched
   \param vect vector to look into
   \return int position of the value

   In case of absence of the vaue in the vector, the value is added at the end and its position returned
*/
template< typename Type1 > unsigned int SearchVectorBin( Type1 val, vector< Type1 > &vect ) {
  unsigned int outBin= 0;
  for ( ;  outBin < vect.size(); outBin++ )
    if ( vect[outBin] == val ) break;
  
  //  if ( outBin == vect.size() ) vect.push_back( val );
  
  return outBin;
}

//============================================
/**\brief Dirac Function
 */
template< typename Type1 > unsigned int Delta( Type1 val1, Type1 val2 ) {
  return ( val1==val2 ) ? 1 : 0;
}

//============================================
/**\brief Parse a string into a vector of a given type elements
   \param string string to be parsed
   \param result vector
 */
template< typename Type1 > int ParseVector( string &stringVector, vector< Type1 > &outVector, bool doClear=1 );
template< typename Type1 > int ParseVector( string &stringVector, vector< Type1 > &outVector, bool doClear ) {

  if ( doClear )  outVector.clear();
  
  stringstream stream;
  stream << stringVector;  

  Type1 value;
  while ( stream >> value ) {
    outVector.push_back( value );
  }

  return 0;
}

//============================================
/**\brief Write the latex text to plot several pictures per line
   \latexStream stream to write on
   \param vect List of figures to plot
   \param nPlotPerWidth Number of figures per line
   \param putNameUnder Write the figure name underneath
 */
template< typename Type1 > void WriteLatexMinipage( Type1 &latexStream, vector<string> vect, unsigned int nPlotPerWidth = 0, bool putNameUnder =false ) {

  if ( !vect.size() ) return;
  if ( !nPlotPerWidth ) nPlotPerWidth = vect.size();

  for ( unsigned int iPlot = 0; iPlot < vect.size(); iPlot++ ) {
    //    vect[iPlot]+=".pdf";
    latexStream << "\\begin{minipage}{" << 1./nPlotPerWidth -0.01 << "\\linewidth} " << endl;
    if ( vect[iPlot] != "" ) latexStream << "\\includegraphics[width=\\linewidth]{" << vect[iPlot] << ".pdf}\\\\" << endl;
    TString dum = vect[iPlot].substr( vect[iPlot].find_last_of( "/" )+1 );
    dum = string(dum).substr( 0, string(dum).find_last_of( "." ) );
    dum.ReplaceAll( "_", "\\_" );
    if ( putNameUnder )   latexStream << dum  << endl;
    latexStream << "\\end{minipage}" << endl;
    if ( iPlot % nPlotPerWidth != nPlotPerWidth-1 ) latexStream << "\\hfill" << endl;

  }
}

//============================================
/**\brief Print keys of a map
 */
template< typename Type1 > void PrintMapKeys( map<string, Type1> &inMap ) {
  for ( auto vKey : inMap ) cout << vKey.first << " ";
  cout << endl;
}

//============================================
/**\brief Find the first key containing the tag in a map
 */
template< typename Type1 > string FindKey( string &tag, map<string, Type1> &inMap ) {
  for ( auto vKey : inMap ) {
    if ( vKey.first.find( tag ) == string::npos ) continue;
    return vKey.first;
  }
  return "";
}

#endif
