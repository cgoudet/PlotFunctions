#define BOOST_TEST_MODULE SideFunctionsTestSuite

#include "PlotFunctions/SideFunctions.h"

#include <boost/test/unit_test.hpp>
#include <algorithm>
#include <iterator>
#include <iostream>
using std::list;
using std::ostream_iterator;
using std::cout;
using std::endl;
using std::string;

// The name of the suite must be a different name to your class                                                                                                                                      
BOOST_AUTO_TEST_SUITE( SideFunctionsSuite )

BOOST_AUTO_TEST_CASE( StripStringTest ) {
  BOOST_CHECK_EQUAL( StripString( "/path/file.ext", 1, 1), "file" );
  BOOST_CHECK_EQUAL( StripString( "/path/file.ext", 0, 1), "/path/file" );
  BOOST_CHECK_EQUAL( StripString( "/path/file.ext", 1, 0), "file.ext" );
  BOOST_CHECK_EQUAL( StripString( "/path/file.ext", 0, 0), "/path/file.ext" );

  BOOST_CHECK_EQUAL( StripString( "file.ext", 1, 0), "file.ext" );
  BOOST_CHECK_EQUAL( StripString( "/path/file", 0, 1), "/path/file" );
}

BOOST_AUTO_TEST_CASE( CombineNamesTest ) {
  list<string> prefix = { "pref1", "pref2" };
  list<string> suffix = { "suff1", "suff2" };
  list<list<string>> inCombi = { prefix, suffix };
  list<string> outList = { "pref1_suff1", "pref1_suff2", "pref2_suff1", "pref2_suff2" };

  BOOST_CHECK( outList == CombineNames( inCombi ) );

}

BOOST_AUTO_TEST_CASE( CompareDoubleTest ) {
  double a = 987654321;
  double b= 9.87654321e8;
  cout << ( fabs((a-b)/a) < 1e-7 ) << endl;
  cout << fabs((a-b)/a) << endl; 
  BOOST_CHECK( CompareDouble( a, b ) );
  BOOST_CHECK( !CompareDouble( a, 987653333.3 ) );

  a=-a;
  b=-b;
  BOOST_CHECK( CompareDouble( a, b ) );
}

BOOST_AUTO_TEST_SUITE_END()

