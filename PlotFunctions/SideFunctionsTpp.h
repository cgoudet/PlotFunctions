#ifndef SIDEFUNCTIONS_TPP
#define SIDEFUNCTIONS_TPP
#include "PlotFunctions/SideFunctions.h"
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <iterator>
#include <algorithm>

namespace ChrisLib {
  //============================================
  /**\brief Get the number of possible configurations containing a single key per vector line
     Tested.
   */
  template<typename Type1 > unsigned int GetNConfigurations( const std::vector<std::vector<Type1>> &inVect ) {
    if ( inVect.empty() ) return 0;
    unsigned int nConfig = 1;
    for ( auto itList = inVect.begin(); itList!=inVect.end(); ++itList ) nConfig*=itList->size();
    return nConfig;
  }

  //============================================
  /**\brief Get the size of the each component of a 2D vector
Tested.
   */
  template<typename Type1 > void GetLevelsSize( const std::vector<std::vector<Type1>> &inVect, std::vector<unsigned> &outVect ) {
    outVect.clear();
    for ( auto itList = inVect.begin(); itList!=inVect.end(); ++itList ) outVect.push_back( itList->size() );
  }
  //============================================
  /**\brief Plot the content of a vector
   */
  template< typename Type1 > void PrintVector( std::vector< Type1 > &vect ) { std::copy( vect.begin(), vect.end(), std::ostream_iterator<Type1>( std::cout, "\n" ) ); }

  //============================================
  /**\brief Search for the bin number of an element in a vector
     \param val value to be searched
     \param vect vector to look into
     \return int position of the value
     Tested.
  */
  template< typename Type1 > unsigned int SearchVectorBin( const Type1 &val, const std::vector< Type1 > &vect ) {
    unsigned int outBin= 0;
    for ( ;  outBin < vect.size(); ++outBin )
      if ( vect[outBin] == val ) break;
    return outBin;
  }

  //============================================
  /**\brief Dirac Function
     Tested.
   */
  template< typename Type1 > unsigned int Delta( Type1 val1, Type1 val2 ) {
    return ( val1==val2 ) ? 1 : 0;
  }

  //============================================
  /**\brief Parse a string into a vector of a given type elements
     \param string string to be parsed
     \param result vector
     Tested.
  */
  template< typename Type1 > void ParseVector( const std::string &stringVector, std::vector< Type1 > &outVector, bool doClear=1 );
  template< typename Type1 > void ParseVector( const std::string &stringVector, std::vector< Type1 > &outVector, bool doClear ) {

    if ( doClear )  outVector.clear();
  
    std::stringstream stream;
    stream << stringVector;  

    Type1 value;
    while ( stream >> value ) {
      outVector.push_back( value );
    }
  }

  //============================================
  /**\brief Write the latex text to plot several pictures per line
     \latexStream stream to write on
     \param vect List of figures to plot
     \param nPlotPerWidth Number of figures per line
     \param putNameUnder Write the figure name underneath
  */
  template< typename Type1 > void WriteLatexMinipage( Type1 &latexStream, std::vector<std::string> vect, unsigned int nPlotPerWidth = 0, bool putNameUnder =false ) {

    if ( !vect.size() ) return;
    if ( !nPlotPerWidth ) nPlotPerWidth = vect.size();

    for ( unsigned int iPlot = 0; iPlot < vect.size(); iPlot++ ) {
      //    vect[iPlot]+=".pdf";
      latexStream << "\\begin{minipage}{" << 1./nPlotPerWidth -0.01 << "\\linewidth} " << std::endl;
      if ( vect[iPlot] != "" ) latexStream << "\\includegraphics[width=\\linewidth]{" << vect[iPlot] << "}\\\\" << std::endl;
      TString dum = vect[iPlot].substr( vect[iPlot].find_last_of( "/" )+1 );
      dum = std::string(dum).substr( 0, std::string(dum).find_last_of( "." ) );
      dum.ReplaceAll( "_", "\\_" );
      if ( putNameUnder )   latexStream << dum  << std::endl;
      latexStream << "\\end{minipage}" << std::endl;
      if ( iPlot % nPlotPerWidth != nPlotPerWidth-1 ) latexStream << "\\hfill" << std::endl;

    }
  }

  //============================================
  /**\brief Print keys of a map
   */
  template< typename Type1 > void PrintMapKeys( std::map<std::string, Type1> &inMap ) {
    for ( auto itKey = inMap.begin(); itKey!=inMap.end(); ++itKey ) std::cout << itKey->first << " ";
    std::cout << std::endl;
  }

  //============================================
  template< typename cont > void RemoveNullPointers( cont &c ) {
    for ( auto it = c.begin(); it!=c.end(); ++it ) {
      if ( *it ) continue;
      c.erase( it );
      --it;
    }
  }
  //=============================================
  template< typename cont > void DeleteContainer( cont &c ) {
    for ( auto it = c.begin(); it!=c.end(); ++it ) {
      if ( !*it ) continue;
      delete *it;
      *it = 0;
    }
  }

}
#endif
