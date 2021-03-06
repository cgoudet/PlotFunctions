#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#include "PlotFunctions/SideFunctions.h"
using namespace ChrisLib;

#include "TH1D.h"

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
using boost::multi_array;
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

//====================================================
BOOST_AUTO_TEST_CASE( CombineNamesTest ) {
  list<string> prefix = { "pref1", "pref2" };
  list<string> suffix = { "suff1", "suff2" };
  list<list<string>> inCombi = { prefix, suffix };
  list<string> testList = { "pref1_suff1", "pref1_suff2", "pref2_suff1", "pref2_suff2" };
  list<string> outList;
  CombineNames( inCombi, outList );
  BOOST_CHECK( outList == testList );
}

//====================================================
BOOST_AUTO_TEST_CASE( CompareDoubleTest ) {
  double a = 987654321;
  double b= 9.87654321e8;
  BOOST_CHECK( CompareDouble( a, b ) );
  BOOST_CHECK( !CompareDouble( a, 987653333.3 ) );

  a=-a;
  b=-b;
  BOOST_CHECK( CompareDouble( a, b ) );
  BOOST_CHECK( CompareDouble( 0, 0 ) );

}
//====================================================
BOOST_AUTO_TEST_CASE( ComputeChi2Test ) {
  TH1D *h1 = new TH1D( "h1", "h1", 3, 0, 3);
  h1->Sumw2();
  h1->Fill( 0.5, 1.1 );
  h1->Fill( 1.5, 2.3 );
  h1->Fill( 2.5, 0.8 );
  TH1D *h2 = new TH1D( "h2", "h2", 3, 0, 3);
  h2->Fill( 0.5, 0.7 );
  h2->Fill( 1.5, 0.6 );
  h2->Fill( 2.5, 2. );

  TH1D *h3 = new TH1D( "h3", "h3", 3, 0, 3 );
  h3->Fill( 0.5 );
  h3->Fill( 1.5 );
  h3->Fill( 2.5 );

  TH1D *h4 = new TH1D( "h4", "h4", 3, 0, 3 );

  double result = ComputeChi2( h1, h2 );
  BOOST_CHECK( CompareDouble( result, 0.9159668994 ) );
  BOOST_CHECK( CompareDouble( ComputeChi2( h1, h4 ), 3. ) );
  BOOST_CHECK( CompareDouble( ComputeChi2( h4, h3 ), 3. ) );
  BOOST_CHECK( CompareDouble( ComputeChi2( h4, h4 ), 0 ) );

  delete h1; h1=0;
  delete h2; h2=0;
  delete h3; h3=0;
  delete h4; h4=0;
}
//==================================================
BOOST_AUTO_TEST_CASE( ComparableHistsTest ) {
  TH1D *h1 = new TH1D( "h1", "h1", 3, 0, 3);
  h1->Fill( 0.5, 1.1 );
  h1->Fill( 1.5, 2.3 );
  h1->Fill( 2.5, 0.8 );
  TH1D *h2 = new TH1D( "h2", "h2", 3, 0, 3);
  h1->Fill( 0.5, 0.7 );
  h1->Fill( 1.5, 0.6 );
  h1->Fill( 2.5, 2. );

  TH1D *h3 = new TH1D( "h3", "h3", 4, 0, 4 );
  TH1D *h4 = new TH1D( "h4", "h4", 3, 0, 4 );
  TH1D *h5 = new TH1D( "h5", "h5", 3, 1, 3 );

  double x[]={0, 0.2, 2, 3};
  TH1D *h6 = new TH1D( "h6", "h6", 3, x );

  double y[]={0, 0.3, 2, 3};
  TH1D *h7 = new TH1D( "h7", "h7", 3, y );

  BOOST_CHECK_THROW( ComparableHists( h1, 0 ), invalid_argument );
  BOOST_CHECK_THROW( ComparableHists( 0, h2 ), invalid_argument );
  BOOST_CHECK_THROW( ComparableHists( 0, 0 ), invalid_argument );

  BOOST_CHECK( ComparableHists( h1, h2 ) );
  BOOST_CHECK( !ComparableHists( h1, h3 ) );
  BOOST_CHECK( !ComparableHists( h1, h4 ) );
  BOOST_CHECK( !ComparableHists( h1, h5 ) );
  BOOST_CHECK( !ComparableHists( h1, h6 ) );
  BOOST_CHECK( !ComparableHists( h7, h6 ) );
  BOOST_CHECK( ComparableHists( h6, h6 ) );

  delete h1; h1=0;
  delete h2; h2=0;
  delete h3; h3=0;
  delete h4; h4=0;
  delete h5; h5=0;
  delete h6; h6=0;
  delete h7; h7=0;

}
//==========================================
BOOST_AUTO_TEST_CASE( GetCoordFromLinearTest ) {
  const vector<unsigned> levelsSize = { 3, 2, 5 };

  vector<unsigned> outCoords;
  vector<unsigned> testCoords;

  GetCoordFromLinear( levelsSize, 0, outCoords );
  testCoords = { 0, 0, 0 };
  BOOST_CHECK_EQUAL( outCoords.size(), testCoords.size() );
  BOOST_CHECK( equal( testCoords.begin(), testCoords.end(), outCoords.begin() ) );

  GetCoordFromLinear( levelsSize, 21, outCoords );
  testCoords = { 2, 0, 1 };
  BOOST_CHECK_EQUAL( outCoords.size(), testCoords.size() );
  BOOST_CHECK( equal( testCoords.begin(), testCoords.end(), outCoords.begin() ) );

  BOOST_CHECK_THROW( GetCoordFromLinear( levelsSize, 55, outCoords ), runtime_error );
}

//==========================================
BOOST_AUTO_TEST_CASE( GetLinearCoordTest ) {
  const vector<unsigned> levelsSize = { 3, 2, 5 };
  vector<unsigned> coords;

  BOOST_CHECK_THROW( GetLinearCoord( levelsSize, coords), invalid_argument );
  BOOST_CHECK_THROW( GetLinearCoord( coords, coords), invalid_argument );

  coords = { 2, 2, 2 };
  BOOST_CHECK_THROW( GetLinearCoord( levelsSize, coords), runtime_error );

  coords = { 2, 0, 2 };
  BOOST_CHECK_EQUAL( GetLinearCoord( levelsSize, coords ), static_cast<unsigned>(22) );
}

//==========================================
BOOST_AUTO_TEST_CASE( CleanHistTest ) {

  TH1D *h1 = new TH1D( "h1", "h1", 2, 0, 2);
  TH1D *h2 = new TH1D( "h2", "h2", 2, 0, 2);
  vector<TH1*> vectHist;

  h1->SetBinContent( 1, -99 );
  h1->SetBinContent( 2, -99 );
  vectHist = { h1 };
  BOOST_CHECK_THROW( CleanHist( vectHist, -99 ), runtime_error );

  h1->SetBinContent( 1, 0 );
  h1->SetBinError( 1, 2 );
  h1->GetXaxis()->SetBinLabel( 1, "binLabel1" );
  h1->SetBinContent( 2, -99 );
  vectHist = { static_cast<TH1D*>(h1->Clone() ) };
  CleanHist( vectHist, -99 );
  BOOST_REQUIRE_EQUAL( static_cast<int>(vectHist.size()), 1 );
  BOOST_CHECK_EQUAL( vectHist.front()->GetNbinsX(), 1 );
  BOOST_CHECK_EQUAL( vectHist.front()->GetBinContent(1), 0 );
  BOOST_CHECK_EQUAL( vectHist.front()->GetBinError(1), 2 );
  BOOST_CHECK_EQUAL( string(vectHist.front()->GetXaxis()->GetBinLabel(1)), "binLabel1" );
  delete vectHist.front();

  h1->SetBinContent( 1, 0 );
  h1->SetBinContent( 2, -99 );
  vectHist = { static_cast<TH1D*>(h1->Clone() ), 0 };
  CleanHist( vectHist, -99 );
  BOOST_REQUIRE_EQUAL( static_cast<int>(vectHist.size()), 1 );
  BOOST_CHECK_EQUAL( vectHist.front()->GetNbinsX(), 1 );
  BOOST_CHECK_EQUAL( vectHist.front()->GetBinContent(1), 0 );
  delete vectHist.front();

  h1->SetBinContent( 1, 0 );
  h1->SetBinContent( 2, 1 );
  vectHist = { static_cast<TH1D*>(h1->Clone() ) };
  TH1* dum = vectHist.front();
  CleanHist( vectHist, -99 );
  BOOST_REQUIRE_EQUAL( vectHist.front(), dum );
  delete vectHist.front();

  h1->SetBinContent( 1, -99 );
  h1->SetBinContent( 2, 1 );
  h2->SetBinContent( 1, -99 );
  h2->SetBinContent( 2, 1 );
  vectHist = { static_cast<TH1D*>(h1->Clone() ), static_cast<TH1D*>(h2->Clone() ) };
  CleanHist( vectHist, -99 );
  BOOST_REQUIRE_EQUAL( static_cast<int>(vectHist.size()), 2 );
  BOOST_CHECK_EQUAL( vectHist.front()->GetNbinsX(), 1 );
  BOOST_CHECK_EQUAL( vectHist.back()->GetNbinsX(), 1 );
  BOOST_CHECK_EQUAL( vectHist.front()->GetBinContent(1), 1 );
  BOOST_CHECK_EQUAL( vectHist.back()->GetBinContent(1), 1 );
  delete vectHist.front();
  delete vectHist.back();

  h1->SetBinContent( 1, 0 );
  h1->SetBinContent( 2, -99 );
  h2->SetBinContent( 1, -99 );
  h2->SetBinContent( 2, 1 );
  vectHist = { static_cast<TH1D*>(h1->Clone() ), static_cast<TH1D*>(h2->Clone() ) };
  TH1 *dum2 = vectHist.back();
  dum = vectHist.front();
  CleanHist( vectHist, -99 );
  BOOST_REQUIRE_EQUAL( vectHist.front(), dum );
  BOOST_REQUIRE_EQUAL( vectHist.back(), dum2 );
  delete vectHist.front();
  delete vectHist.back();

  double x[]={0, 0.2, 2, 3};
  TH1D *h3 = new TH1D( "h3", "h3", 3, x );
  vectHist = { static_cast<TH1D*>(h1->Clone() ), static_cast<TH1D*>(h3->Clone() ) };
  BOOST_CHECK_THROW( CleanHist( vectHist, -99 ), invalid_argument );
  delete vectHist.front();
  delete vectHist.back();

  delete h1;
  delete h2;
  delete h3;
}

//========================================
BOOST_AUTO_TEST_CASE( RebinHistTest ) {
  TH1D *h1 = new TH1D( "h1", "h1", 2, 0, 2);
  h1->SetBinContent( 1, 0 );
  h1->SetBinContent( 2, 1 );

  TH1D *h2 = new TH1D( "h2", "h2", 2, 0, 2);
  h2->SetBinContent( 1, 0 );
  h2->SetBinContent( 2, 1 );

  double x[] = { 0, 0.5, 1, 2 };
  TH1D* h3 = new TH1D( "h3", "h3", 3, x );
  h3->SetBinContent( 1, 0 );
  h3->SetBinContent( 2, 1 );
  h3->SetBinContent( 3, 2 );

  vector<TH1*> vectHist;

  vectHist = { static_cast<TH1D*>(h1->Clone() ), 0 };
  RebinHist( vectHist );
  BOOST_REQUIRE_EQUAL( static_cast<int>(vectHist.size() ), 1 );
  BOOST_CHECK_EQUAL( vectHist.front()->GetNbinsX(), 2 );
  BOOST_CHECK_EQUAL( vectHist.front()->GetBinContent(1), 0 );
  BOOST_CHECK_EQUAL( vectHist.front()->GetBinContent(2), 1 );
  delete vectHist.front();

  vectHist = { static_cast<TH1D*>(h1->Clone() ), static_cast<TH1D*>(h2->Clone() ) };
  RebinHist( vectHist );
  BOOST_REQUIRE_EQUAL( static_cast<int>(vectHist.size() ), 2 );
  BOOST_CHECK_EQUAL( vectHist.front()->GetNbinsX(), 2 );
  BOOST_CHECK_EQUAL( vectHist.back()->GetNbinsX(), 2 );
  BOOST_CHECK_EQUAL( vectHist.front()->GetBinContent(1), 0 );
  BOOST_CHECK_EQUAL( vectHist.front()->GetBinContent(2), 1 );
  BOOST_CHECK_EQUAL( vectHist.back()->GetBinContent(1), 0 );
  BOOST_CHECK_EQUAL( vectHist.back()->GetBinContent(2), 1 );
  delete vectHist.front();
  delete vectHist.back();

  vectHist = { static_cast<TH1D*>(h1->Clone() ), static_cast<TH1D*>(h3->Clone() ) };
  RebinHist( vectHist );
  BOOST_REQUIRE_EQUAL( static_cast<int>(vectHist.size() ), 2 );
  BOOST_CHECK_EQUAL( vectHist.front()->GetNbinsX(), 3 );
  BOOST_CHECK_EQUAL( vectHist.back()->GetNbinsX(), 3 );
  BOOST_CHECK_EQUAL( vectHist.front()->GetBinContent(1), 0 );
  BOOST_CHECK_EQUAL( vectHist.front()->GetBinContent(2), 0 );
  BOOST_CHECK_EQUAL( vectHist.front()->GetBinContent(3), 1 );
  BOOST_CHECK_EQUAL( vectHist.back()->GetBinContent(1), 0 );
  BOOST_CHECK_EQUAL( vectHist.back()->GetBinContent(2), 1 );
  BOOST_CHECK_EQUAL( vectHist.back()->GetBinContent(3), 2 );
  delete vectHist.front();
  delete vectHist.back();

  delete h1;
  delete h2;
  delete h3;
}

//============================
BOOST_AUTO_TEST_CASE( FillDefaultFrontiersTest ) {
  vector<double> testVect = { 0., 1., 2., 3. };
  vector<double> outVect;

  FillDefaultFrontiers( outVect, 3, 0, 3 );
  BOOST_CHECK_EQUAL( outVect.size(), testVect.size() );
  BOOST_CHECK( equal( outVect.begin(), outVect.end(), testVect.begin() ) );
}

//============================
BOOST_AUTO_TEST_CASE( RemoveSeparatorTest ) {
  BOOST_CHECK_EQUAL( RemoveSeparator( "a__b" ), "a_b" );
  BOOST_CHECK_EQUAL( RemoveSeparator( "a___b" ), "a_b" );
  BOOST_CHECK_EQUAL( RemoveSeparator( "a_b_" ), "a_b" );

  BOOST_CHECK_EQUAL( RemoveSeparator( "azzzzzb", "zz" ), "azzzb" );
  BOOST_CHECK_EQUAL( RemoveSeparator( "a_bzzzz", "zz" ), "a_b" );
  BOOST_CHECK_EQUAL( RemoveSeparator( "a_bzzzzz", "zz" ), "a_bz" );

  BOOST_CHECK_EQUAL( RemoveSeparator( "" ), "" );
  BOOST_CHECK_EQUAL( RemoveSeparator( "_" ), "" );
}
//============================
BOOST_AUTO_TEST_CASE( RemoveWordsTest ) {
  const list<string> words = { "baba", "barn", "zzzz" };

  BOOST_CHECK_EQUAL( RemoveWords( "babaty", words ), "ty" );
  BOOST_CHECK_EQUAL( RemoveWords( "babazzzzty", words ), "ty" );
  BOOST_CHECK_EQUAL( RemoveWords( "zzzzbabaty", words ), "ty" );
  BOOST_CHECK_EQUAL( RemoveWords( "babarnty", words ), "rnty" );
}

//===============================
BOOST_AUTO_TEST_CASE( ParseLegendTest ) {
  string legend { "" };
  BOOST_CHECK_EQUAL( ParseLegend(legend), "" );

  legend = "rty_uio";
  BOOST_CHECK_EQUAL( ParseLegend(legend), legend );

  legend = "rty_uio__HASHTAG__FILL_____NOPOINT__ATLAS__STACK__ETA_CALOETA_CALO_______";
  BOOST_CHECK_EQUAL( ParseLegend(legend), "rty_uio#___#eta_{CALO}ETA_CALO_______" );

}

//===============================
BOOST_AUTO_TEST_CASE( TestDoubleTreeTest ) {

  double val1{1};
  int val2{2};
  TTree *tree1= new TTree( "tree1", "tree1" );
  tree1->Branch( "branch11", &val1 );
  tree1->Branch( "branch12", &val2 );

  BOOST_CHECK_THROW( TestDoubleTree( 0, "" ), invalid_argument );
  BOOST_CHECK_THROW( TestDoubleTree( tree1, "branch11" ), runtime_error );
  tree1->Fill();
  BOOST_CHECK_THROW( TestDoubleTree( tree1, "dummy" ), runtime_error );
  BOOST_CHECK_THROW( TestDoubleTree( tree1, "branch12" ), runtime_error );

  BOOST_CHECK_EQUAL( TestDoubleTree( tree1, "branch11" ), val1 );
}

//===============================
BOOST_AUTO_TEST_CASE( CreateSystHistTest ) {

  BOOST_CHECK_THROW( CreateSystHist(0,0,0), invalid_argument );

  TH1D *hist1 = new TH1D( "hist1", "hist1", 1, 0, 1 );
  hist1->SetBinContent( 1, 1 );

  TH1D *h2 = new TH1D( "h2", "h2", 1, 0, 1 );
  h2->SetBinContent( 1, -2 );

  TH1D *hist2 = static_cast<TH1D*>(h2->Clone());
  CreateSystHist( hist2, hist1, 0 );
  BOOST_CHECK_EQUAL( hist2->GetBinContent(1), sqrt(5));

  // Test of bins by bins modes
  delete hist2; hist2=0;
  hist2 = static_cast<TH1D*>(h2->Clone());
  CreateSystHist( hist2, hist1, 1 );
  BOOST_CHECK_EQUAL( hist2->GetBinContent(1), 3);

  delete hist2; hist2=0;
  hist2 = static_cast<TH1D*>(h2->Clone());
  CreateSystHist( hist2, hist1, 2 );
  BOOST_CHECK_EQUAL( hist2->GetBinContent(1), -1);

  delete hist2; hist2=0;
  hist2 = static_cast<TH1D*>(h2->Clone());
  CreateSystHist( hist2, hist1, 3 );
  BOOST_CHECK_EQUAL( hist2->GetBinContent(1), -3);

  delete hist2; hist2=0;
  hist2 = static_cast<TH1D*>(h2->Clone());
  CreateSystHist( hist2, hist1, 4 );
  BOOST_CHECK_EQUAL( hist2->GetBinContent(1), 1);

  delete hist2; hist2=0;
  hist2 = static_cast<TH1D*>(h2->Clone());
  CreateSystHist( hist2, hist1, 5 );
  BOOST_CHECK_EQUAL( hist2->GetBinContent(1), 3);

  delete hist2; hist2=0;
  hist2 = static_cast<TH1D*>(h2->Clone());
  CreateSystHist( hist2, 0, 0 );
  BOOST_CHECK_EQUAL( hist2->GetBinContent(1), 4);

  delete hist2; hist2=0;
  hist2 = static_cast<TH1D*>(h2->Clone());
  CreateSystHist( hist2, 0, 1 );
  BOOST_CHECK_EQUAL( hist2->GetBinContent(1), 2);

  delete hist2; hist2=0;
  hist2 = static_cast<TH1D*>(h2->Clone());
  CreateSystHist( hist2, 0, 3 );
  BOOST_CHECK_EQUAL( hist2->GetBinContent(1), 2);

  delete hist2; hist2=0;
  hist2 = static_cast<TH1D*>(h2->Clone());
  CreateSystHist( hist2, 0, 13 );
  BOOST_CHECK_EQUAL( hist2->GetBinContent(1), 2);



  //test of symetrized mode
  delete hist2; hist2=0;
  hist2 = static_cast<TH1D*>(h2->Clone());
  CreateSystHist( hist2, hist1, 15 );
  BOOST_CHECK_EQUAL( hist2->GetBinContent(1), 3);

  delete hist1; hist1=0;
  hist1 = new TH1D( "hist1", "hist1", 2, 0, 1 );
  hist1->SetBinContent( 1, 1 );
  hist1->SetBinContent( 2, 2 );
  delete h2; h2=0;
  delete hist2; hist2=0;
  h2 = new TH1D( "h2", "h2", 2, 0, 1 );
  h2->SetBinContent( 1, 3 );
  h2->SetBinContent( 2, 4 );
  hist2 = static_cast<TH1D*>(h2->Clone());
  CreateSystHist( hist2, hist1, 14 );
  BOOST_CHECK_EQUAL( hist2->GetBinContent(1), 5);
  BOOST_CHECK_EQUAL( hist2->GetBinContent(2), 5);

  delete hist2; hist2=0;
  hist2 = static_cast<TH1D*>(h2->Clone());
  CreateSystHist( hist2, hist1, 0 );
  BOOST_CHECK_EQUAL( hist2->GetBinContent(1), sqrt(10));
  BOOST_CHECK_EQUAL( hist2->GetBinContent(2), sqrt(20));

  delete hist2; hist2=0;
  hist2 = static_cast<TH1D*>(h2->Clone());
  CreateSystHist( hist2, 0, 10 );
  double val=3.5*3.5;
  BOOST_CHECK_EQUAL( hist2->GetBinContent(1), val);
  BOOST_CHECK_EQUAL( hist2->GetBinContent(2), val);

  delete hist1;
  delete hist2;
  delete h2;
}

BOOST_AUTO_TEST_CASE( ReverseErrValTest ) {
  TH1D *hist2 = new TH1D( "hist2", "hist2", 1, 0, 1 );
  hist2->SetBinContent( 1, 2 );
  hist2->SetBinError( 1, 1 );
  ReverseErrVal( hist2 );
  BOOST_CHECK_EQUAL( hist2->GetBinContent(1), 1);
  BOOST_CHECK_EQUAL( hist2->GetBinError(1), 2);
  delete hist2;
}

BOOST_AUTO_TEST_CASE( AddSlashTest ) {
  string in{ "dodo" };
  BOOST_CHECK_EQUAL( AddSlash(in), "dodo/");
  in = "dodo ";
  BOOST_CHECK_EQUAL( AddSlash(in), "dodo /");
  in = "dodo/";
  BOOST_CHECK_EQUAL( AddSlash(in), in );
}

//============================
BOOST_AUTO_TEST_CASE( CompareArrayColumnsTest ) {
  multi_array<double, 2> diffArr(boost::extents[5][6]), throwArr(boost::extents[5][5]);
  multi_array<double, 2> diffRelArr(diffArr), initArr(boost::extents[5][4]);
  
  for ( int j=0; j<4; ++ j ) {
    int shift = j / 2;
    for ( int i=0; i<5; ++i ) {
      initArr[i][j] = i*10 + j+1;
      diffArr[i][j+shift] = initArr[i][j];
      diffRelArr[i][j+shift] = initArr[i][j];
      if ( j%2 ) {
	diffArr[i][j+1+shift] = initArr[i][j-1] - initArr[i][j];
	diffRelArr[i][j+1+shift] = ( initArr[i][j-1] - initArr[i][j] )/ initArr[i][j-1];
      }
    }
  }
  
  BOOST_CHECK_THROW( CompareArrayColumns(throwArr ), runtime_error );
  
  multi_array<double, 2> outArr = CompareArrayColumns(initArr, 0);
  BOOST_CHECK( outArr == diffArr );

  outArr = CompareArrayColumns( initArr, 1 );
  BOOST_CHECK( outArr == diffRelArr );
}


BOOST_AUTO_TEST_SUITE_END()
