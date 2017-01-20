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

//====================================================
BOOST_AUTO_TEST_CASE( DeltaTest ) {

  BOOST_CHECK( Delta(1, 1) );
  BOOST_CHECK( !Delta(0, 1) );

  BOOST_CHECK( Delta("abcd", "abcd") );
  BOOST_CHECK( !Delta("abscd", "gqf") );
}

//================================================
BOOST_AUTO_TEST_CASE( SearchVectorBinTest ) {
  vector<string> dumS = { "dada", "dodo", "dudu", "dodo" };
  string searchValue = "dodo";
  BOOST_CHECK_EQUAL( static_cast<int>(SearchVectorBin(searchValue, dumS)), 1 );
  searchValue="boubou";
  BOOST_CHECK_EQUAL( static_cast<int>(SearchVectorBin(searchValue, dumS)), 4 );
  
  vector<int> dumI = { 1, 7, 14, 22, 7 };
  int val = 7;
  BOOST_CHECK_EQUAL( static_cast<int>(SearchVectorBin(val, dumI)), 1 );
  val=12;
  BOOST_CHECK_EQUAL( static_cast<int>(SearchVectorBin(val, dumI) ), 5 );
}
//================================================
BOOST_AUTO_TEST_CASE( GetNConfigurationsTest ) {
  vector<vector<string>> levels;
  BOOST_CHECK_EQUAL( static_cast<int>(GetNConfigurations( levels )), 0 );

  levels.push_back( { "a", "b", "c", "d" } );
  BOOST_CHECK_EQUAL( static_cast<int>(GetNConfigurations( levels )), 4 );

  levels.push_back( { "e", "f", "g" } );
  levels.push_back( { "h", "i", "j", "k", "l" } );

  BOOST_CHECK_EQUAL( static_cast<int>(GetNConfigurations( levels )), 60 );
}
//================================================
BOOST_AUTO_TEST_CASE( GetLevelsSizeTest ) {
  vector<vector<string>> levels;
  vector<unsigned> outSizes;
  vector<unsigned> testSizes;

  GetLevelsSize(levels, outSizes );
  BOOST_CHECK( outSizes.empty() );

  levels.push_back( { "a", "b", "c", "d" } );
  testSizes.push_back( 4 );
  GetLevelsSize(levels, outSizes );
  BOOST_REQUIRE_EQUAL( outSizes.size(), testSizes.size() );
  BOOST_CHECK( equal( testSizes.begin(), testSizes.end(), outSizes.begin() ) );

  levels.push_back( { "e", "f", "g" } );
  levels.push_back( { "h", "i", "j", "k", "l" } );
  testSizes.push_back( 3 );
  testSizes.push_back( 5 );
  GetLevelsSize(levels, outSizes );
  BOOST_REQUIRE_EQUAL( outSizes.size(), testSizes.size() );
  BOOST_CHECK( equal( testSizes.begin(), testSizes.end(), outSizes.begin() ) );
}

//===============================================
BOOST_AUTO_TEST_CASE(RemoveNullPointersTest) {
  TH1D *h1 = new TH1D( "h1", "h1", 2, 0, 2 );

  vector<TH1D*> outVect = { h1 };
  vector<TH1D*> testVect = { h1 };;
  RemoveNullPointers( outVect );
  BOOST_REQUIRE_EQUAL( outVect.size(), testVect.size() );
  BOOST_CHECK( equal( testVect.begin(), testVect.end(), outVect.begin() ) );

  outVect = { 0 };
  testVect = {  };
  RemoveNullPointers( outVect );
  BOOST_REQUIRE_EQUAL( outVect.size(), testVect.size() );
  BOOST_CHECK( equal( testVect.begin(), testVect.end(), outVect.begin() ) );

  outVect = { 0, h1  };
  testVect = { h1 };
  RemoveNullPointers( outVect );
  BOOST_REQUIRE_EQUAL( outVect.size(), testVect.size() );
  BOOST_CHECK( equal( testVect.begin(), testVect.end(), outVect.begin() ) );

  outVect = { h1, 0 };
  testVect = { h1 };
  RemoveNullPointers( outVect );
  BOOST_REQUIRE_EQUAL( outVect.size(), testVect.size() );
  BOOST_CHECK( equal( testVect.begin(), testVect.end(), outVect.begin() ) );

  delete h1;
}
//===============================================
BOOST_AUTO_TEST_CASE(DeleteContainerTest) {
  TH1D *h1 = new TH1D( "h1", "h1", 2, 0, 2 );

  vector<TH1D*> outVect = { static_cast<TH1D*>(h1->Clone() ) };
  vector<TH1D*> testVect = { 0 };;
  DeleteContainer( outVect );
  BOOST_REQUIRE_EQUAL( outVect.size(), testVect.size() );
  BOOST_CHECK( equal( testVect.begin(), testVect.end(), outVect.begin() ) );

  outVect = { 0 };
  testVect = { 0 };
  DeleteContainer( outVect );
  BOOST_REQUIRE_EQUAL( outVect.size(), testVect.size() );
  BOOST_CHECK( equal( testVect.begin(), testVect.end(), outVect.begin() ) );

  outVect = { 0, static_cast<TH1D*>(h1->Clone() )  };
  testVect = { 0, 0 };
  DeleteContainer( outVect );
  BOOST_REQUIRE_EQUAL( outVect.size(), testVect.size() );
  BOOST_CHECK( equal( testVect.begin(), testVect.end(), outVect.begin() ) );

  outVect = { static_cast<TH1D*>(h1->Clone() ), 0 };
  testVect = { 0, 0 };
  DeleteContainer( outVect );
  BOOST_REQUIRE_EQUAL( outVect.size(), testVect.size() );
  BOOST_CHECK( equal( testVect.begin(), testVect.end(), outVect.begin() ) );

  delete h1;

}
BOOST_AUTO_TEST_CASE( ParseVectorTest ) {
  string testS;
  vector<string> outVectS;
  vector<string> testVectS;

  ParseVector( testS, outVectS );
  BOOST_REQUIRE_EQUAL( outVectS.size(), testVectS.size() );
  BOOST_CHECK( equal( testVectS.begin(), testVectS.end(), outVectS.begin() ) );

  testS = "a";
  testVectS.push_back( "a" );
  ParseVector( testS, outVectS );
  BOOST_REQUIRE_EQUAL( outVectS.size(), testVectS.size() );
  BOOST_CHECK( equal( testVectS.begin(), testVectS.end(), outVectS.begin() ) );

  testS = "aiohl qdpfomjq";
  outVectS.clear();
  testVectS = { "aiohl", "qdpfomjq" };
  ParseVector( testS, outVectS );
  BOOST_REQUIRE_EQUAL( outVectS.size(), testVectS.size() );
  BOOST_CHECK( equal( testVectS.begin(), testVectS.end(), outVectS.begin() ) );

  testS = "1 3";
  testVectS = { "1", "3" };
  outVectS.clear();
  ParseVector( testS, outVectS );
  BOOST_REQUIRE_EQUAL( outVectS.size(), testVectS.size() );
  BOOST_CHECK( equal( testVectS.begin(), testVectS.end(), outVectS.begin() ) );

  vector<int> testVectI= { 1, 3 };
  vector<int> outVectI;  
  ParseVector( testS, outVectI );
  BOOST_REQUIRE_EQUAL( outVectI.size(), testVectI.size() );
  BOOST_CHECK( equal( testVectI.begin(), testVectI.end(), outVectI.begin() ) );

  testS = " 3  0.0005 1e-3  ";
  vector<double> testVectD= {  3, 0.0005, 1e-3 };
  vector<double> outVectD;
  ParseVector( testS, outVectD );
  BOOST_REQUIRE_EQUAL( outVectD.size(), testVectD.size() );
  BOOST_CHECK( equal( testVectD.begin(), testVectD.end(), outVectD.begin() ) );

  testS = "1e-3, 3, 0.0005 ";
  testVectD= { 1e-3, 3, 0.0005 };
  outVectD.clear();
  ParseVector( testS, outVectD );
  BOOST_REQUIRE_EQUAL( outVectD.size(), testVectD.size() );
  BOOST_CHECK( equal( testVectD.begin(), testVectD.end(), outVectD.begin() ) );

}

BOOST_AUTO_TEST_CASE(SumSqTest) {
  list<double> inputs { 3, 4, 5 };
  BOOST_REQUIRE_EQUAL( SumSq(inputs), 50 );
  
  inputs = { 3, -4, 5 };
  BOOST_REQUIRE_EQUAL( SumSq(inputs), 50 );
}

BOOST_AUTO_TEST_CASE(OplusTest) {
  list<double> inputs { 3, 4, 5 };
  BOOST_REQUIRE_EQUAL( Oplus(inputs), sqrt(50) );
}

BOOST_AUTO_TEST_SUITE_END()
