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

  list<string> keys;
  list<string> testKeys { "int", "float", "double", "string" };
  mapBr.GetKeys( keys );
  BOOST_CHECK( keys==testKeys );
}

//=======================================
BOOST_AUTO_TEST_CASE( LinkTreeBranchesTest ) {
  MapBranches mapBr;

  TTree *tree1=0, *tree2=0;
  BOOST_CHECK_THROW( mapBr.LinkTreeBranches(tree1, tree2 ), invalid_argument );

  double val;
  tree1= new TTree( "tree1", "tree1" );
  tree1->Branch( "branch11", &val );
  tree1->Branch( "branch12", &val );

  tree2= new TTree( "tree2", "tree2" );
  tree2->Branch( "branch21", &val );
  tree2->Branch( "branch22", &val );

  delete tree1;
  delete tree2;
}

BOOST_AUTO_TEST_SUITE_END()
