#include "PlotFunctions/DrawOptions.h"
#include "PlotFunctions/SideFunctionsTpp.h"
#include "PlotFunctions/AtlasStyle.h"
#include "PlotFunctions/AtlasUtils.h"
#include "PlotFunctions/AtlasLabels.h"
#include "PlotFunctions/SideFunctions.h"

#include <TROOT.h>
#include "TF1.h"
#include "TCanvas.h"
#include "RooPlot.h"
#include "RooDataSet.h"
#include "RooAbsPdf.h"

#include <exception>
#include <iostream>
#include <functional>
#include <algorithm>
#include <list>

using std::min;
using std::max;
using std::list;
using std::invalid_argument;
using std::runtime_error;
using std::cout;
using std::cerr;
using std::endl;
using std::vector;
using std::string;

using namespace ChrisLib;
using namespace RooFit;

ChrisLib::DrawOptions::DrawOptions() : m_legendCoord {0.7,0.9}, m_debug(0),
                                       m_colors {1, 632, 600, 616, 416, 800, 921, 629, 597, 613, 413, 797, 635, 603, 619, 419, 807 },
                                       m_fillColors { 3, 5 }
{

  std::list<string> keys { "doChi2", "logy", "orderX" };
  for ( auto vKey : keys ) m_bools[vKey]=0;
  m_bools["forceStyle"]=1;

  keys = { "doRatio", "drawStyle", "shiftColor", "grid" };
  for ( auto vKey : keys ) m_ints[vKey]=0;

  keys = { "scale", "line", "clean", "normalize", "topMargin", "bottomMargin" };
  for ( auto vKey : keys ) m_doubles[vKey]=-99;
  m_doubles["extendUp"]=0;
  m_doubles["offset"]=0;
  m_doubles["xTitleOffset"]=1;
  m_doubles["yTitleOffset"]=1;

  keys = { "xTitle", "yTitle" };
  for ( auto vKey : keys ) m_strings[vKey]="";
  m_strings["extension"]="pdf";
  m_strings["outName"]="plot";
}
//==========================================================
ChrisLib::DrawOptions::DrawOptions( const string &outName ) : DrawOptions() {
  m_strings["outName"]=outName;
}
//==========================================================
void ChrisLib::DrawOptions::AddOption( const string &option ) {
  string key = option.substr( 0, option.find_first_of('=' ) );
  string value = option.substr( option.find_first_of("=")+1);
  AddOption( key, value);
}

//==========================================================
void ChrisLib::DrawOptions::AddOption( const string &key, const string &value ) {
  if ( m_bools.find(key) != m_bools.end() ) m_bools[key] = std::atoi( value.c_str() );
  else if ( m_ints.find(key) != m_ints.end() ) m_ints[key] = std::atoi( value.c_str() );
  else if ( m_strings.find(key) != m_strings.end() ) m_strings[key] = value;
  else if ( m_doubles.find(key) != m_doubles.end() ) m_doubles[key] = std::stod( value.c_str() );
  else if ( key == "legend" ) m_legends.push_back( value );
  else if ( key == "latex" ) m_latex.push_back( value );
  else if ( key == "legendPos" ) {
    m_legendCoord.clear();
    ParseVector( value, m_legendCoord );
    CheckLegendCoord();
  }
  else if ( key == "rangeUserX" ) {
    m_rangeUserX.clear();
    ParseVector( value, m_rangeUserX );
  }
  else if ( key == "rangeUserY" ) {
    m_rangeUserY.clear();
    ParseVector( value, m_rangeUserY );
  }
  else if ( key == "latexOpt" ) {
    m_latexPos.push_back( vector<double>() );
    ParseVector( value, m_latexPos.back() );
  }
  else if ( key == "labels") ParseVector( value, m_labels );
  else cout << "DrawPlotOption : " << key << " not known" << endl;
}

//==========================================================
void ChrisLib::DrawOptions::AddOption( const vector<string> &options ) {
  std::for_each( options.begin(), options.end(), [this](const string &s ){AddOption(s);});
}

//==========================================================
void ChrisLib::DrawOptions::CheckLegendCoord() {
  if ( m_legendCoord.empty() ) m_legendCoord = { -99, 99 };
  else if ( m_legendCoord.size() != 2 ) throw invalid_argument( "DrawOptions::CheckLegendCoord : Wrong number of inputs in legendCoord. Enter either 0 or 2." );
}

//==============================================
void ChrisLib::DrawOptions::SetHistProperties( TH1* hist ) {
  vector<string> functionNames = { "cubicFit", "quadraticFit" };
  TIter next(hist->GetListOfFunctions());
  while (TObject *obj = next()) {
    if ( strcmp( hist->GetFunction( obj->GetName() )->ClassName(), "TF1" ) ) continue;
    hist->GetFunction( obj->GetName() )->SetLineColor( hist->GetLineColor() );
  }

  if ( hist->GetNbinsX()>30 ) hist->GetXaxis()->LabelsOption("v");
}
//================================================
void ChrisLib::DrawOptions::SetProperties( TObject* obj, int iHist ) {

  const vector<string> labels = GetLabels();

  TH1* hist=0;
  TGraphErrors *graph=0;
  if (  !IsHist( obj ) ) graph = static_cast<TGraphErrors*>(obj);
  else hist=static_cast<TH1*>(obj);

  if ( m_bools["forceStyle" ] ) obj->UseCurrentStyle();
  if ( !iHist ) {
    for ( unsigned iAxis=0; iAxis<2; ++iAxis ) {
      string title = iAxis ? GetYTitle() : GetXTitle();
      if ( title!= "" ) {
        title = ParseLegend( title );
        TAxis *axis = 0;
        if ( hist ) axis = iAxis ? hist->GetYaxis() : hist->GetXaxis();
        else axis = iAxis ? graph->GetYaxis() : graph->GetXaxis();
        axis->SetTitle( title.c_str() );
        axis->SetTitleOffset( GetTitleOffset(iAxis) );

        if ( !labels.empty() && hist && !iAxis ) {
          if ( axis->GetNbins() != static_cast<int>(labels.size()) ) cout << "ChrisLib::DrawOptions::SetProperties : label and bins size do not match." <<endl;
          else {
            for ( unsigned iBin=0; iBin<labels.size(); ++iBin )
              axis->SetBinLabel( iBin+1, labels[iBin].c_str());
          }
        }
      }
    }
    if ( graph && GetOrderX() ) graph->Sort();
  }

  //If only one histograms is plotted, plot it in red
  TAttLine *attLine= hist ? static_cast<TAttLine*>(hist) : static_cast<TAttLine*>(graph);
  TAttMarker *attMarker = hist ? static_cast<TAttMarker*>(hist) : static_cast<TAttMarker*>(graph);
  attMarker->SetMarkerSize( 0.5 );

  int shiftColor = GetShiftColor();
  switch ( GetDrawStyle()/10 ) {
  case 1 :
    attLine->SetLineColor( m_colors[ max( 0, iHist/2 + shiftColor )] );
    attLine->SetLineStyle( 1 + iHist%2 );
    attMarker->SetMarkerColor( m_colors[ max( 0 , static_cast<int>(iHist/2 + shiftColor)) ] );
    attMarker->SetMarkerStyle( (iHist%2) ? 4 : 8 );
    break;
  default :
    attLine->SetLineColor( m_colors[iHist+shiftColor] );
    attMarker->SetMarkerColor( m_colors[ iHist + shiftColor ] );
    attMarker->SetMarkerStyle( 8 );
  }

  if ( hist ) {
    SetHistProperties( hist );
    double normalize = GetNormalize();
    double scale = GetScale();
    if ( hist && normalize!=-99 && hist->Integral() )  {
      hist->Sumw2();
      hist->Scale( normalize/hist->Integral() );
    }
    else if ( hist && scale!=-99 ) {
      hist->Sumw2();
      hist->Scale( scale );
    }
  }
  else {
    double offset = GetOffset();
    if ( offset ) {
      double minY=graph->GetMinimum();
      for ( int bin = 0; bin < graph->GetN(); bin++ ) {
        double x, y;
        graph->GetPoint( bin, x, y );
        graph->SetPoint( bin, x, y-( offset==-99 ? minY : offset ) );
      }
    }
  }


}
//==============================================
void ChrisLib::DrawOptions::GetMaxValue( TObject *obj, double &minVal, double &maxVal, double &minX, double &maxX, bool takeError, bool isRef ) {

  TH1* hist=0;
  TGraphErrors *graph=0;
  if (  !IsHist( obj ) ) graph = static_cast<TGraphErrors*>(obj);
  else hist=static_cast<TH1*>(obj);

  if ( isRef ) {
    if ( hist ) {
      minVal = hist->GetBinContent(1);
      minX = hist->GetXaxis()->GetXmin();
    }
    else graph->GetPoint( 0, minX, minVal );
    maxVal=minVal;
    maxX = minX;
  }

  if ( hist ) {
    minX = min( minX, hist->GetXaxis()->GetXmin() );
    maxX = max( maxX, hist->GetXaxis()->GetXmax() );
  }

  //Update the maximum range of the plot with extremum of current plot
  int nBins = hist ? hist->GetNbinsX() : graph->GetN();
  for ( int bin = 0; bin < nBins; ++bin ) {
    double val = 0, err=0;
    if ( hist ) {
      val = hist->GetBinContent( bin+1 );
      if ( takeError ) err = hist->GetBinError( bin+1 );
    }
    else {
      double xGraph=0;
      graph->GetPoint(bin, xGraph, val);
      if ( takeError ) err = graph->GetErrorY(bin);
      double errX = takeError ? graph->GetErrorX(bin) : 0;
      minX = min( minX, xGraph - errX );
      maxX = max( maxX, xGraph + errX );
    }
    minVal = min( val - err , minVal );
    maxVal = max( val + err , maxVal );
  }
}
//==============================================
void ChrisLib::DrawOptions::DrawLatex() {
  vector<string> inLatex(GetLatex());
  const vector<vector<double>> latexPos = GetLatexPos();
  for ( unsigned int iLatex = 0; iLatex < inLatex.size(); iLatex++ ) {
    if ( latexPos[iLatex].size() != 2 ) continue;
    bool doLabel = inLatex[iLatex].find("__ATLAS") != string::npos;
    inLatex[iLatex] = ParseLegend( inLatex[iLatex] );
    if ( doLabel ) ATLASLabel( latexPos[iLatex][0], latexPos[iLatex][1], inLatex[iLatex].c_str(),1 , 0.04 );
    else myText( latexPos[iLatex][0], latexPos[iLatex][1], 1, inLatex[iLatex].c_str() );
  }
}
//==============================================
void ChrisLib::DrawOptions::DrawText( vector<TObject*> &inHist ) {
  DrawLegend(inHist);
  DrawLatex();
}
//==============================================
void ChrisLib::DrawOptions::DrawLegend( vector<TObject*> &inHist ) {

  const vector<double> legendCoord = GetLegendCoord();
  int drawStyle = GetDrawStyle();

  for ( unsigned int iLegend=0; iLegend<m_legends.size(); iLegend++ ) {
    if ( !inHist[iLegend] ) continue;
    TH1* hist=0;
    TGraphErrors *graph=0;
    if ( !IsHist(inHist[iLegend] ) ) graph = static_cast<TGraphErrors*>(inHist[iLegend]);
    else hist=static_cast<TH1*>(inHist[iLegend]);
    TAttLine *attLine= hist ? static_cast<TAttLine*>(hist) : static_cast<TAttLine*>(graph);
    TAttMarker *attMarker = hist ? static_cast<TAttMarker*>(hist) : static_cast<TAttMarker*>(graph);
    int color = attLine->GetLineColor();
    int lineStyle = attLine->GetLineStyle();
    int markerStyle = attMarker->GetMarkerStyle();
    bool doFill = m_legends.size() > iLegend && m_legends[iLegend].find("__FILL")!=string::npos;
    m_legends[iLegend] =  hist ? ParseLegend( hist, m_legends[iLegend] ) : ParseLegend( graph, m_legends[iLegend] );
    if ( doFill ) {
      color = hist ? hist->GetFillColor() : graph->GetFillColor();
      myBoxText( legendCoord[0], legendCoord[1]-0.04*iLegend, color, m_legends[iLegend].c_str() );
    }
    else if ( drawStyle ) myMarkerText( legendCoord[0], legendCoord[1]-0.05*iLegend, color, markerStyle, m_legends[iLegend].c_str()  );
    else myLineText( legendCoord[0], legendCoord[1]-0.05*iLegend, color, lineStyle, m_legends[iLegend].c_str()  );
    //Added by Antinea but don't know the impact
    // if (drawStyle==4) {
    //  myLineText( legendCoord[0]-0.005, legendCoord[1]-0.05*iLegend, color, lineStyle, ""  );
    //  myMarkerText( legendCoord[0], legendCoord[1]-0.05*iLegend, color, markerStyle, inLegend[iLegend].c_str()  );
    // }
  }
  if ( m_debug )  cout << "legend drawn" << endl;
}
//==============================================
void ChrisLib::DrawOptions::Draw( TH1* inHist ) {
  vector<TObject*> v{inHist};
  Draw( v );
}
//==============================================
void ChrisLib::DrawOptions::Draw( const vector< TH1* > &inHist ) {
  vector<TObject*> histObj(inHist.size());
  copy( inHist.begin(), inHist.end(), histObj.begin());
  Draw( histObj );
}
//==============================================
void ChrisLib::DrawOptions::Draw( const vector< TGraphErrors* > &inHist ) {
  vector<TObject*> histObj(inHist.size());
  copy( inHist.begin(), inHist.end(), histObj.begin());
  Draw( histObj );
}
//==============================================
void ChrisLib::DrawOptions::Draw( vector< TObject* > &inHist ) {

  if ( m_debug ) cout << "ChrisLib::DrawOptions::Draw( vector< TObject* > )" << endl;
  SetAtlasStyle();

  if ( inHist.size() && inHist.front() && TString(inHist.front()->ClassName()).Contains("TH2")) {
    Draw( static_cast<TH2*>(inHist.front()) );
    return;
  }
  if ( inHist.size()==1 ) m_ints["shiftColor"] = 1;
  vector< TH1* > ratio;

  //================ PAD DEFINITION
  TCanvas canvas;
  double topMargin = GetTopMargin();
  if (topMargin!=-99) canvas.SetTopMargin(topMargin);
  double bottomMargin = GetBottomMargin();
  if (bottomMargin!=-99) canvas.SetBottomMargin(bottomMargin);
  int doGrid = GetGrid();
  canvas.SetGrid( doGrid%2, doGrid/2 );

  TPad padUp( "padUp", "padUp", 0, 0.35, 1, 1 );
  padUp.SetTopMargin( 0.08 );
  padUp.SetBottomMargin( 0.008 );
  TPad padDown( "padDown", "padDown", 0, 0, 1, 0.35 );
  padDown.SetTopMargin( 0.08 );
  padDown.SetBottomMargin( 0.3 );

  int doRatio = GetDoRatio();
  if ( doRatio ) {
    padUp.Draw();
    padUp.SetGrid( doGrid%2, doGrid/2 );
    padDown.SetGrid( doGrid%2, doGrid/2 );
    canvas.cd();
    padDown.Draw();
    padUp.cd();
  }

  TLine line( 0, 0.005, 100, 0.005);
  line.SetLineColor( kBlack );
  line.SetLineStyle( 3 );
  if ( m_debug ) cout << "defined pads" << endl;

  //============ LOOP OTHER INPUT HIST
  //Find the extremum of the histograms to choose rangeUser if not given
  double minVal=0, maxVal=0, minX=0, maxX=0;
  int refHist= -1;

  TAxis *refXAxis=0, *refYAxis=0;

  if ( GetClean() !=-99 ) {
    vector<TH1*> hists;
    for ( auto it=inHist.begin(); it!=inHist.end(); ++it ) hists.push_back( static_cast<TH1*>(*it));
    CleanHist( hists, GetClean() );
    copy( hists.begin(), hists.end(), inHist.begin() );
  }

  if ( !m_legends.empty() && m_legends.size() != inHist.size() ) throw invalid_argument( "DrawPlot : Number of legend must match the one of histograms." );
  const vector<double> legendCoord = GetLegendCoord();


  for ( unsigned int iHist = 0; iHist < inHist.size(); ++iHist ) {
    if ( !inHist[iHist] ) {
      cerr << "ChrisLib::DrawOptions::Draw( vector< TObject* > ) : skipping null histogram " << iHist << endl;
      continue;
    }

    TH1* hist=0;
    TGraphErrors *graph=0;
    if ( !IsHist(inHist[iHist] ) ) graph = static_cast<TGraphErrors*>(inHist[iHist]);
    else hist=static_cast<TH1*>(inHist[iHist]);

    if ( refHist == -1 ) {
      refHist = iHist;
      refXAxis = hist ? hist->GetXaxis() : graph->GetXaxis();
      refYAxis = hist ? hist->GetYaxis() : graph->GetYaxis();
    }

    SetProperties( inHist[iHist], iHist-refHist );

    GetMaxValue( inHist[iHist], minVal, maxVal, minX, maxX, 1, static_cast<int>(iHist)==refHist );

    if ( hist && GetDoChi2() && m_legends.size() && iHist ){
      m_tmpLegends = m_legends;
      TH1* refObj=0;
      switch ( GetDrawStyle()/10 ) {
      case 1 :
        if ( !IsHist(inHist[iHist-1]) ) throw runtime_error( "DrawOptions::DrawPlot : Chi2 on different types" );
        refObj  = static_cast<TH1*>( inHist[iHist-1] );
        if ( iHist % 2 ) m_legends[iHist] += " : chi2=" + TString::Format( "%2.2f", ComputeChi2( hist, static_cast<TH1*>(inHist[iHist-1]) )/hist->GetNbinsX() );
        break;
      default :

        if ( !IsHist( inHist[refHist] ) ) throw runtime_error( "DrawOptions::DrawPlot : Chi2 on different types" );
        refObj  = static_cast<TH1*>( inHist[refHist] );
        m_legends[iHist] += " : chi2=" + TString::Format( "%2.2f", ComputeChi2( hist, refObj)/refObj->GetNbinsX() );

      }
    }

  }//end for iHist

  if (refHist==-1) throw runtime_error( "ChrisLib::DrawOptions::Draw( vector< TObject* > ) : all histograms are null." );

  if ( m_debug ) cout << "setting range" << endl;
  vector<double> rangeUserY { GetRangeUserY() };
  while ( rangeUserY.size() < 2 ) rangeUserY.push_back( pow(-1, rangeUserY.size()+1)*0.99 );
  if ( rangeUserY.front() == -0.99 ) rangeUserY.front() = minVal - ( maxVal - minVal ) *0.05;
  if ( rangeUserY.back() == 0.99 ) rangeUserY.back() = maxVal + ( maxVal - minVal ) *0.05;
  rangeUserY.back() += (rangeUserY.back() - rangeUserY.front()) * GetExtendUp();

  vector<double> rangeUserX(GetRangeUserX());
  if ( rangeUserX.size() == 2 ) refXAxis->SetRangeUser( rangeUserX[0], rangeUserX[1] );
  else rangeUserX = { minX, maxX };

  TH1F* dumHist = 0;
  if ( !strcmp( refXAxis->GetBinLabel(1), "" ) ) {//If no label
    if ( doRatio ) dumHist = padUp.DrawFrame( rangeUserX.front(), rangeUserY.front(), rangeUserX.back(), rangeUserY.back() );
    else dumHist = canvas.DrawFrame( rangeUserX.front(), rangeUserY.front(), rangeUserX.back(), rangeUserY.back() );

    dumHist->SetLineColorAlpha( 0, 0 );
    dumHist->SetMarkerColorAlpha( 0, 0 );
    dumHist->GetXaxis()->SetTitle( refXAxis->GetTitle() );
    dumHist->GetYaxis()->SetTitle( refYAxis->GetTitle() );
    dumHist->GetXaxis()->SetTitleOffset( refXAxis->GetTitleOffset() );
    dumHist->GetYaxis()->SetTitleOffset( refYAxis->GetTitleOffset() );

    if (doRatio) {
      dumHist->GetXaxis()->SetLabelSize(0);
      dumHist->GetYaxis()->SetTitleOffset( 0.75 );
      dumHist->GetYaxis()->SetTitleSize( 0.06 );
    }
  }
  else refYAxis->SetRangeUser( rangeUserY.front(), rangeUserY.back() );


  //Plotting histograms
  for ( unsigned int iHist = refHist; iHist < inHist.size(); ++iHist ) {
    if ( !inHist[iHist] ) continue;
    TH1* hist=0;
    TGraphErrors *graph=0;
    if ( !IsHist(inHist[iHist] ) ) graph = static_cast<TGraphErrors*>(inHist[iHist]);
    else hist=static_cast<TH1*>(inHist[iHist]);

    string drawOption = strcmp( refXAxis->GetBinLabel(1), "" ) && static_cast<int>(iHist)==refHist ?  "" :"SAME,";
    switch ( GetDrawStyle()%10 ){
    case 1 : drawOption += "HIST"; break;
    case 2 : drawOption += "HISTL"; break;
      // case 4 :
      //   inHist[0]->SetMarkerStyle(8);
      //   inHist[1]->SetMarkerStyle(25);
      //   inHist[iHist]->SetMarkerSize(1.3);
      //   break;
    default : drawOption += "E";
    }

    if ( m_legends.size() > iHist && m_legends[iHist].find("__NOPOINT")!=string::npos ) {
      cout << "transparent" << endl;
      if ( hist ) {
        hist->SetLineColorAlpha( 0, 0 );
        hist->SetMarkerColorAlpha( 0, 0 );
      }
      else {
        graph->SetLineColorAlpha( 0, 0 );
        graph->SetMarkerColorAlpha( 0, 0 );
      }
    }

    if ( m_legends.size() > iHist && m_legends[iHist].find("__FILL")!=string::npos ) {
      drawOption += "2";
      hist ? hist->SetFillColor( m_fillColors[iHist] ) : graph->SetFillColor( m_fillColors[iHist] );
    }


    inHist[iHist]->Draw( drawOption.c_str() );
    //========== ADD HISTOGRAM TO LEGEND
  }//end iHist

  if ( m_debug ) cout << "drawn" << endl;
  if ( GetLogY() ) {
    if ( doRatio ) padUp.SetLogy(1);
    else canvas.SetLogy(1);
    if ( m_debug ) cout << "logy done" << endl;
  }


  // =========== PRINT LEGENDS AND LATEX
  double lineVal = GetLine();
  if( lineVal != -99
      && ( lineVal > rangeUserY.front() && lineVal < rangeUserY.back() )
      ) {
    double rangeMin = rangeUserX.size()== 2 ? rangeUserX[0] : minX;
    double rangeMax = rangeUserX.size()== 2 ? rangeUserX[1] : maxX;
    line.DrawLine( rangeMin , lineVal, rangeMax, lineVal);
  }

  canvas.cd();
  DrawText( inHist );
  if ( m_debug ) cout << "latex drawn" << endl;

  //===============  CREATE RATIO PLOTS
  if ( doRatio ) {
    padDown.cd();
    double minValRatio = 0;
    double maxValRatio = 0;
    bool setTitle=0;
    for ( unsigned int iHist = refHist+1; iHist < inHist.size(); iHist++ ) {
      if ( !inHist[iHist] ) continue;

      string yTitle;
      //Decide how to pair histogram for ratio
      switch ( GetDrawStyle() ) {
      case 1 :
        if ( !(iHist % 2) || !inHist[iHist-1]) continue;
        ratio.push_back( 0 );
        ratio.back() = static_cast<TH1*>(inHist[iHist]->Clone());
        ratio.back()->Add( static_cast<TH1*>(inHist[iHist-1]), -1 );
        if ( doRatio == 1 ) ratio.back()->Divide( static_cast<TH1*>(inHist[iHist-1]) );
        yTitle = ( doRatio==1 ) ? "#frac{h_{2n+1}-h_{2n}}{h_{2n}}" : "h_{2n+1}-h_{2n}";
        break;
      default :
        ratio.push_back( 0 );
        ratio.back() = static_cast<TH1*>(inHist[iHist]->Clone());
        ratio.back()->Add( static_cast<TH1*>(inHist[refHist]), -1 );
        if ( doRatio == 1 ) ratio.back()->Divide( static_cast<TH1*>(inHist[refHist]) );
        yTitle = ( doRatio==1 ) ? "#frac{h_{n}-h_{0}}{h_{0}}" : "h_{n}-h_{0}";
      }//end switch

      if ( m_debug ) cout << "ratio created" << endl;
      //Set graphics properties of first hitogram
      if ( !setTitle ) {
        ratio.front()->GetXaxis()->SetTitle( refXAxis->GetTitle() );
        ratio.front()->GetXaxis()->SetLabelSize( 0.09 );
        ratio.front()->GetXaxis()->SetTitleSize( 0.13 );
        ratio.front()->GetXaxis()->SetTitleOffset( 0.95 );
        ratio.front()->GetYaxis()->SetLabelSize( 0.065 );
        ratio.front()->GetYaxis()->SetTitleSize( 0.13 );
        ratio.front()->GetYaxis()->SetTitleOffset( 0.35 );
        ratio.front()->SetTitle("");
        ratio.front()->GetYaxis()->SetTitle( yTitle.c_str() );
        setTitle = 1;
      }
      if ( m_debug ) cout << "ratio front title done" << endl;
      //Update the values of Y axis range
      for ( int bin = 1; bin <= ratio.front()->GetNbinsX(); bin++ ) {
        minValRatio = min( ratio.back()->GetBinContent(bin) - ratio.back()->GetBinError( bin), minValRatio );
        maxValRatio = max( ratio.back()->GetBinContent(bin)+ ratio.back()->GetBinError( bin ), maxValRatio );
      }

    }// end iHist

    if ( ratio.size() ) {
      if ( m_debug ) cout << "ratio ranges " << endl;
      //Plot all the ratio plots
      ratio.front()->GetYaxis()->SetRangeUser( minValRatio - (maxValRatio-minValRatio)*0.05, maxValRatio+(maxValRatio-minValRatio)*0.05 );
      if ( rangeUserX.size() == 2 ) ratio.front()->GetXaxis()->SetRangeUser( rangeUserX[0], rangeUserX[1] );
      // centerzoom option removed. Must check if comment breaks the code
      //      else if ( drawOptmapOptionsInt["centerZoom"] ) ratio.front()->GetXaxis()->SetRangeUser( minX, maxX );
      if ( m_debug ) cout << "plot ratio" << endl;
      for ( unsigned int iHist = 0; iHist < ratio.size(); iHist++ ) {
        ratio[iHist]->Draw( ( iHist ) ? "e,same" : "e" );
      }
      //Create a line at 0 to visualize deviations
      //      line.DrawLine( rangeUserXratio.front()->GetXaxis()->GetXmin(), 0, ratio.front()->GetXaxis()->GetXmax(), 0);
      line.DrawLine( rangeUserX[0], 0, rangeUserX[1], 0);
    }
  }//end doRatio

  if ( m_debug ) cout << "saving" << endl;
  string canOutName = GetOutName() + "." + GetExtension();
  canvas.SaveAs( canOutName.c_str() );

  //Restoring legend if necessary
  if ( !m_tmpLegends.empty() ) {
    std::swap(m_legends,m_tmpLegends);
    m_tmpLegends.clear();
  }
}
//=========================================
void ChrisLib::DrawOptions::Draw( TH2* hist ) {
  TCanvas canvas;
  canvas.SetRightMargin(0.1);
  if ( m_bools["forceStyle" ] ) hist->UseCurrentStyle();
  hist->Draw( "COLZ" );
  vector<TObject*> v{hist};
  DrawText( v );
  string canOutName = GetOutName() + "." + GetExtension();
  canvas.SaveAs( canOutName.c_str() );
}
//=========================================
void ChrisLib::DrawOptions::Draw( RooRealVar *frameVar, vector<TObject*> &inObj ) {

  TCanvas *canvas = new TCanvas();
  if ( m_rangeUserX.size() == 2 ) frameVar->setRange( m_rangeUserX.front(), m_rangeUserX.back() );

  RooPlot* frame=frameVar->frame( frameVar->getBins());
  frame->SetTitle(""); //empty title to prevent printing "A RooPlot of ..."
  frame->SetXTitle(frameVar->GetTitle());

  int shiftColor = GetShiftColor();
  vector<map<string,int>> legendInfo;
  for ( unsigned int iHist=0; iHist<inObj.size(); iHist++ ) {
    legendInfo.push_back( map<string, int>());
    legendInfo.back()["color"] = m_colors[ iHist + shiftColor ];
    if ( string(inObj[iHist]->ClassName() ) == "RooDataSet" ) {
      //      ( (RooDataSet*) inObj[iHist])->Print();
      static_cast<RooDataSet*>(inObj[iHist])->plotOn( frame, LineColor( m_colors[ iHist + shiftColor ] ), DataError( RooAbsData::SumW2 ) );
      legendInfo.back()["doLine"] = 0;
      legendInfo.back()["style"] = frame->getAttLine(frame->getObject(iHist)->GetName())->GetLineStyle();
    }
    else {
      //      ( (RooAbsPdf*) inObj[iHist])->Print();
      static_cast<RooAbsPdf*>(inObj[iHist])->plotOn( frame, LineColor( m_colors[ iHist + shiftColor ] ) );
      legendInfo.back()["doLine"] = 1;
      legendInfo.back()["style"] = frame->getAttMarker(frame->getObject(iHist)->GetName())->GetMarkerStyle();
    }
  }

  frame->Draw();
  // for ( unsigned int iHist=0; iHist<inObj.size(); iHist++ ) {
  //   if ( legendInfo[iHist]["doLine"] )    myMarkerText( 0.7, 0.9-0.05*iHist, legendInfo[iHist]["color"], legendInfo[iHist]["style"], inLegend.size() ? inLegend[iHist].c_str() : "" );
  //   else  myLineText( 0.7, 0.9-0.05*iHist, legendInfo[iHist]["color"], legendInfo[iHist]["style"], inLegend.size() ? inLegend[iHist].c_str() : "" );

  // }

  DrawLatex();

  string canOutName = GetOutName() + "." + GetExtension();
  canvas->SaveAs( canOutName.c_str() );

  //  canvas->SaveAs( TString( outName + ".pdf") );
  delete frame;
  delete canvas; canvas=0;

}
