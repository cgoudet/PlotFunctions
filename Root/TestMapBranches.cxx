#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#include "PlotFunctions/MapBranches.h"
#include "PlotFunctions/SideFunctions.h"
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

  mapBr.SetVal( "testFloat", 3. );
  cout << "testFloat" << mapBr.GetFloat(testFloat) << endl;
}

//=======================================
BOOST_AUTO_TEST_CASE( LinkTreeBranchesTest ) {
  MapBranches mapBr;

  BOOST_CHECK_THROW( mapBr.LinkTreeBranches(0, 0 ), invalid_argument );

  double val1{1}, val2{2};
  TTree *tree1= new TTree( "tree1", "tree1" );
  tree1->Branch( "branch11", &val1 );
  tree1->Branch( "branch12", &val2 );
  tree1->Fill();
  list<string> linkedBranches { "branch11" };

  //Testing cas of simple linking
  BOOST_CHECK_NO_THROW(mapBr.LinkTreeBranches(tree1, 0, linkedBranches ));
  tree1->GetEntry(0);
  BOOST_CHECK( mapBr.IsLinked() );
  BOOST_CHECK_THROW( mapBr.GetDouble("branch12"), std::range_error );
  BOOST_CHECK_NO_THROW( mapBr.GetDouble("branch11") );
  BOOST_CHECK_EQUAL( mapBr.GetDouble("branch11"), 1 );

  TTree *tree2 = new TTree( "tree2", "tree2" );
  //Can technically throw if one type of variable is in tree1 and not coded.
  BOOST_CHECK_NO_THROW( mapBr.LinkTreeBranches( tree1, tree2, linkedBranches ) );
  tree1->GetEntry(0);
  tree2->Fill();
  BOOST_CHECK_EQUAL( TestDoubleTree(tree2, "branch11" ), 1 );
  BOOST_CHECK_THROW( TestDoubleTree(tree2, "branch12" ), runtime_error );

  delete tree1;
  delete tree2;

  TTree *tree2 = new TTree( "tree2", "tree2" );
  list<string> emptyList;
  BOOST_CHECK_THROW( mapBr.LinkTreeBranches( 0, tree2, emptyList ), invalid_argument );
  BOOST_CHECK_NO_THROW( mapBr.LinkTreeBranches( 0, tree2, linkedBranches ) );
  BOOST_CHECK( mapBr.IsLinked() );

  mapBr.SetVal( "branch11", 3. );
}

BOOST_AUTO_TEST_SUITE_END()
