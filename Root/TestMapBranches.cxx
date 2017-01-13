#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#include "PlotFunctions/MapBranches.h"
using namespace ChrisLib;


#include <algorithm>
#include <iterator>
#include <iostream>

using std::list;
using std::ostream_iterator;
using std::cout;
using std::endl;
using std::string;
using std::invalid_argument;
using std::vector;
using std::runtime_error;
using std::out_of_range;

BOOST_AUTO_TEST_SUITE( MapBranchesSuite )
BOOST_AUTO_TEST_CASE( IsLinkedTest) {
  MapBranches mapBr;
  BOOST_CHECK( !mapBr.IsLinked() );
  mapBr.SetVal( "string", "string" );  
  BOOST_CHECK( mapBr.IsLinked() );
  mapBr.ClearMaps();
  BOOST_CHECK( !mapBr.IsLinked() );
}
//==========================================
BOOST_AUTO_TEST_CASE( SetValTest) {

  MapBranches mapBr;
  mapBr.SetVal( "string", "string" );
  BOOST_CHECK_EQUAL( mapBr.GetString("string"), "string" );
  BOOST_CHECK_EQUAL( mapBr.GetLabel("string"), "string" );
  BOOST_CHECK_THROW( mapBr.GetLabel("doudou"), out_of_range );

  int valI = 2;
  mapBr.SetVal( "int", valI );
  BOOST_CHECK_EQUAL( mapBr.GetInt("int"), valI );
  BOOST_CHECK_EQUAL( mapBr.GetLabel("int"), "2" );

  double valD = 3;
  float valF =4;
  mapBr.SetVal( "double", valD );
  mapBr.SetVal( "float", valF );
  BOOST_CHECK_EQUAL( mapBr.GetFloat("float"), valF );
  BOOST_CHECK_EQUAL( mapBr.GetDouble("double"), valD );
  BOOST_CHECK_EQUAL( mapBr.GetDouble("float"), static_cast<double>(valF) );
  
}

BOOST_AUTO_TEST_SUITE_END()
