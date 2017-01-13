#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE Main

#include "PlotFunctions/SideFunctionsTpp.h"
#include "PlotFunctions/SideFunctions.h"
using namespace ChrisLib;

#include "TH1D.h"

#include <boost/test/unit_test.hpp>

#include <algorithm>
#include <iterator>
#include <iostream>
#include <stdexcept>

using std::list;
using std::ostream_iterator;
using std::cout;
using std::endl;
using std::string;
using std::invalid_argument;
using std::vector;
using std::runtime_error;


BOOST_AUTO_TEST_SUITE( SideFunctionsTppSuite )
BOOST_AUTO_TEST_CASE( DeltaTest ) {

  BOOST_CHECK( Delta(1, 1) );
  BOOST_CHECK( !Delta(0, 1) );

  BOOST_CHECK( Delta("abcd", "abcd") );
  BOOST_CHECK( !Delta("abscd", "gqf") );
}
BOOST_AUTO_TEST_SUITE_END()

//#######################################################

BOOST_AUTO_TEST_SUITE( SideFunctionsSuite )
//====================================================
BOOST_AUTO_TEST_CASE( StripStringTest ) {
  BOOST_CHECK_EQUAL( StripString( "/path/file.ext", 1, 1), "file" );
  BOOST_CHECK_EQUAL( StripString( "/path/file.ext", 0, 1), "/path/file" );
  BOOST_CHECK_EQUAL( StripString( "/path/file.ext", 1, 0), "file.ext" );
  BOOST_CHECK_EQUAL( StripString( "/path/file.ext", 0, 0), "/path/file.ext" );

  BOOST_CHECK_EQUAL( StripString( "file.ext", 1, 0), "file.ext" );
  BOOST_CHECK_EQUAL( StripString( "/path/file", 0, 1), "/path/file" );
}
BOOST_AUTO_TEST_SUITE_END()

//#######################################################
BOOST_AUTO_TEST_SUITE( FoncteursSuite )
BOOST_AUTO_TEST_CASE( ReplaceStringTest ) {
  ReplaceString rep1( "z" );
  BOOST_CHECK_EQUAL( rep1( "" ), "" );
  BOOST_CHECK_EQUAL( rep1( "z" ), "" );
  BOOST_CHECK_EQUAL( rep1( "azb" ), "ab" );
  BOOST_CHECK_EQUAL( rep1( "azzzb" ), "ab" );

  ReplaceString rep2( "zz" );
  BOOST_CHECK_EQUAL( rep2( "zz" ), "" );
  BOOST_CHECK_EQUAL( rep2( "azzb" ), "ab" );
  BOOST_CHECK_EQUAL( rep2( "azzzb" ), "azb" );
  BOOST_CHECK_EQUAL( rep2( "azzzzb" ), "ab" );

  ReplaceString rep3( "zz", "yy" );
  BOOST_CHECK_EQUAL( rep3( "zz" ), "yy" );
  BOOST_CHECK_EQUAL( rep3( "azzb" ), "ayyb" );
  BOOST_CHECK_EQUAL( rep3( "azzzb" ), "ayyzb" );
  BOOST_CHECK_EQUAL( rep3( "azzzzb" ), "ayyyyb" );

  ReplaceString rep4( "z", "yz" );
  BOOST_CHECK_EQUAL( rep4( "z" ), "yz" );
  BOOST_CHECK_EQUAL( rep4( "azb" ), "ayzb" );

  BOOST_CHECK_THROW( ReplaceString( "", "y" ), std::invalid_argument );
}
BOOST_AUTO_TEST_SUITE_END()
