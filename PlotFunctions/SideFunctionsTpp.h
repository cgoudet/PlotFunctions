#ifndef SIDEFUNCTIONS_TPP
#define SIDEFUNCTIONS_TPP
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
template< typename Type1 > int ParseVector( string &stringVector, vector< Type1 > &outVector ) {

  outVector.clear();
  
  stringstream stream;
  stream << stringVector;  

  Type1 value;
  while ( stream >> value ) {
    outVector.push_back( value );
  }

  return 0;
}

#endif
