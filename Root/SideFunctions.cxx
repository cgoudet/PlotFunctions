#include "PlotFunctions/SideFunctions.h"
#include <iostream>
#include "TF1.h"
#include "TCanvas.h"
#include "TLegend.h"
using std::vector;
using std::cout;
using std::endl;
using std::stringstream;

double ComputeChi2( TH1 *MCHist, TH1 *DataHist ) {

  if ( MCHist->GetNbinsX() != DataHist->GetNbinsX() 
       || MCHist->GetXaxis()->GetXmin() != DataHist->GetXaxis()->GetXmin() 
       || MCHist->GetXaxis()->GetXmax() != DataHist->GetXaxis()->GetXmax() ) 
{
    cout << "Histograms for chi do not match" << endl;
    exit(1);
 }
  
  double chi2 = 0;
  for ( int i = 0; i < MCHist->GetNbinsX(); i++ ) {
    double valdif = ( MCHist->GetBinError( i+1 )==0 && DataHist->GetBinError( i+1 )==0 ) ? 0 : MCHist->GetBinContent( i+1 ) - DataHist->GetBinContent( i+1 );
    double sigma2 = ( valdif != 0 ) ? MCHist->GetBinError( i+1 ) * MCHist->GetBinError( i+1 ) + DataHist->GetBinError( i+1 ) * DataHist->GetBinError( i+1 ) : 1;
    chi2 += valdif * valdif / sigma2; 
  }
    
  return chi2;
}

//=======================================================
/**\brief Find the range to fit between +/- 3sigma
   \return 0 OK

Lower and upper bins are put into binMin and binMax variables
*/
int FindFitBestRange( TH1D *hist, int &binMin, int &binMax, double chiMinLow, double chiMinUp ) {


  double min = hist->GetMinimum();
  int centralBin = hist->GetMinimumBin();

  if ( chiMinLow == 0 ) binMin = 1; 
  else { 
  binMin = centralBin;
  while ( binMin > 1 && hist->GetBinContent( binMin ) - min < chiMinLow ) binMin--;
  }

  if ( chiMinUp == 0 ) binMax = hist->GetNbinsX();
  else {
    binMax = centralBin;
    while ( binMax < hist->GetNbinsX()  && hist->GetBinContent( binMax ) - min < chiMinUp ) binMax++;
  }

  return 0;
}

//=============================================
void WriteLatexMinipage( fstream &latexStream, vector<string> vect, unsigned int nPlotPerWidth, bool putNameUnder ) {

  if ( !vect.size() ) return;
  if ( !nPlotPerWidth ) nPlotPerWidth = vect.size();

  for ( unsigned int iPlot = 0; iPlot < vect.size(); iPlot++ ) {
    vect[iPlot]+=".pdf";
    latexStream << "\\begin{minipage}{" << 0.99/nPlotPerWidth << "\\linewidth} " << endl;
    latexStream << "\\includegraphics[width=\\linewidth]{" << vect[iPlot] << "}\\\\" << endl;
    TString dum = StripString( vect[iPlot] );
    dum.ReplaceAll( "_", "\\_" );
    if ( putNameUnder )   latexStream << dum  << endl;
    latexStream << "\\end{minipage}" << endl;
    if ( iPlot % nPlotPerWidth == 0 )     latexStream << "\\hfill" << endl;

    }
  }

//=============================================
void WriteLatexHeader( fstream &latexStream, string author ) {

  latexStream << "\\documentclass[a4paper,12pt]{article}" << endl;
  latexStream << "\\usepackage{graphicx}" << endl;
  latexStream << "\\usepackage{xcolor}" << endl;
  latexStream << "\\usepackage[a4paper, textwidth=0.9\\paperwidth, textheight=0.9\\paperheight]{geometry}" << endl;
  latexStream << "\\usepackage[toc]{multitoc}" << endl;
  latexStream << "\\title{Scale Factor Extraction}" << endl;
  latexStream << "\\author{" << author << "}" << endl;
  latexStream << "\\date{\\today}" <<endl;
  latexStream << "\\begin{document}\\maketitle" << endl;

}
  //======================================
string StripString( string &inString, bool doPrefix, bool doSuffix ) {
  
  if ( doPrefix ) inString = inString.substr( inString.find_last_of( "/" )+1 );
  if ( doSuffix ) inString = inString.substr( 0, inString.find_last_of( "." ) );
  
  return inString;
}

//=======================================
void RemoveExtremalEmptyBins( TH1 *hist ) {

  int lowBin = 1, upBin = hist->GetNbinsX();
  while ( hist->GetBinContent( lowBin ) == 0 ) lowBin++;
  while ( hist->GetBinContent( upBin ) == 0 ) upBin--;

  hist->GetXaxis()->SetRangeUser( hist->GetXaxis()->GetBinLowEdge( lowBin ), hist->GetXaxis()->GetBinUpEdge( upBin ) );
}

//===============================
// int ParseVector( string &stringVector, vector<double> &outVector ) {
  
//   outVector.clear();
  
//   stringstream stream;
//   stream << stringVector;  

//   double value = 0;
//   while ( stream >> value ) {
//     outVector.push_back( value );
//   }

//   return 0;
// }
//========================================================
void ParseLegend( TH1* hist, string &legend ) {

  TString dumString = legend;
  dumString.ReplaceAll( "__Entries", TString::Format( "%1.0f", hist->GetEntries() ) );
  dumString.ReplaceAll( "__MEAN", TString::Format( "%1.2e", hist->GetMean() ) );
  dumString.ReplaceAll( "__STDEV", TString::Format( "%1.2e", hist->GetStdDev() ) );
  legend = dumString;

}
