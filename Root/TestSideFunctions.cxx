#define BOOST_TEST_MODULE SideFunctionsTestSuite

#include "PlotFunctions/SideFunctions.h"

#include <boost/test/unit_test.hpp>
#include <algorithm>
#include <iterator>
#include <iostream>
using std::ostream_iterator;
using std::cout;
using std::endl;
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

BOOST_AUTO_TEST_SUITE_END()

