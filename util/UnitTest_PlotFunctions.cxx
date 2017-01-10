#define BOOST_TEST_DYN_LINK
//#define BOOST_TEST_MAIN

//#define BOOST_TEST_STATIC_LINK

#define BOOST_TEST_MODULE Master

#include <boost/test/unit_test.hpp>
#include "PlotFunctions/SideFunctionsTpp.h"
using namespace ChrisLib;

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
