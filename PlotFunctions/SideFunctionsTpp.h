#ifndef SIDEFUNCTIONS_TPP
#define SIDEFUNCTIONS_TPP
#include "PlotFunctions/SideFunctions.h"
#include <iostream>
#include <string>
#include <vector>
using std::vector;
using std::stringstream;
using std::string;
using std::cout;
using std::endl;

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
/**\Dirac Function
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


#endif
