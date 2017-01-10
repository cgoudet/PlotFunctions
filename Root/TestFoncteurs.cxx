#include <boost/test/unit_test.hpp>
#include "PlotFunctions/Foncteurs.h"
using namespace ChrisLib;

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
}
BOOST_AUTO_TEST_SUITE_END()
