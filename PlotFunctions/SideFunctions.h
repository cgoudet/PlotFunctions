#ifndef SIDEFUNCTIONS_H
#define SIDEFUNCTIONS_H
#include "THStack.h"
#include "TH1D.h"
#include "TTree.h"
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <map>
#include "boost/multi_array.hpp"

#include <cstdio>

#include "TGraphErrors.h"
#include "TString.h"


using std::map;
using std::fstream;
using std::string;
using std::vector;
using std::cout;
using std::endl;

#include "TGraphErrors.h"
#include "TString.h"
//#include "TXMLNode.h"
#include <list>

/** \brief Namespace wrapping all C++ library
 */
namespace ChrisLib {

  /*\brief Modify histograms to have a common binning as the one with most granularity.
    Tested
   */
  void RebinHist( std::vector<TH1*> &vectHist );

  /**\brief Remove some bins from a list of histograms its values is equal to input
     Tested.
   */
  void CleanHist( std::vector<TH1*> &vect, const double removeVal );

  /**\brief Get Multidimentional coordinates of an element fro mits vector index
     \param levelsSize list of elements in each dimension
     \param objIndex index of the element
     Tested.
  */
  void GetCoordFromLinear( const std::vector<unsigned int> &levelsSize, const unsigned int objIndex, std::vector<unsigned> &coords );

  /**\brief Get index of element in a vector from multidimensionnal coordinates
     \param levelsSize list of elements in each dimension
     \param Coordinates of the element
     Tested
  */
  unsigned int GetLinearCoord( const std::vector<unsigned int> &levelsSize, const std::vector<unsigned int> &objCoords );

  /**\brief Print allvariables of the workspace in a csv file
     \param inFileName input root file
     \param outFileName name of the output csv file
     \param inFunctions Name of the functions to be printed
     \param inWSName Name of the input workspace (useFindDefaultTree if empty)
  */
  std::string PrintWorkspaceVariables( std::string inFileName, std::string outFileName="", std::vector<std::string> inFunctionsName=std::vector<std::string>(), std::string inWSName="" );

  /**\brief Print the correlation model of a pdf in a csv file
     \param inFileName input root file
     \param outFileName name of the csv file
     \param inConfigurationsName Name decomposition of the categories
     \param NPPrefix prefix for the systematic effect of the nuisance parameter
     \param inWSName Name of the input workspace (useFindDefaultTree if empty)
     \param inMCName Name of the ModelConfig

  */
  std::string PrintWorkspaceCorrelationModel(std::string inFileName, std::string outFileName, std::vector<std::vector<std::string>> inConfigurationsName, std::string NPPrefix, std::string inWSName="", std::string inMCName="mconfig" );

  /**\brief Compute Chi2 of two matching histograms
     \param MCHist Histogram with MC role
     \param DataHist Histogram with Data role
     \return Chi2/ndf
     Tested.
  */
  double ComputeChi2( TH1 *MCHist, TH1 *DataHist );

  /**\brief optimize fit ranges using Y values left and right to the minimum
     \param hist histogram to fit
     \param binMin lowest histogram bin to be included
     \param binMax highest histogram bin to be included
     \param chiMinLow Maximum histogram value to include in fit on the left
     \param chiMinUp Maximum histogram value to include in fit on the right
  */
  int FindFitBestRange( TH1D *hist, int &binMin, int &binMax, double chiMinLow=9, double chiMinUp=25);

  /**\brief Automatisation of writting minipages in a latex file
     \param latexStream streamer to the output file
     \param vect list of plots name to include in pdf file
     \param nPlotPerWidth number of minipages per line
     \param putNameUnder Add the name of the plot below it
  */
  //void WriteLatexMinipage( fstream &latexStream, std::vector<string> vect, unsigned int nPlotPerWidth=0, bool putNameUnder = false );

  /**\brief Print latex header in tex file
     \param latexStream streamer to the output file
     \param author text to put in author option
  */
  void WriteLatexHeader( std::fstream &latexStream, std::string title, std::string author="Christophe Goudet", int mode=0 );

  /**\brief Remove suffix and prefix from a string
     \param inString string to be modified. 
     \param doPrefix remove everything before the last /
     \param doSuffix remove everything after the last .
     \return string copy of the modified string

     Tested.
  */
  std::string StripString( const std::string &inString, bool doPrefix=1, bool doSuffix=1 );

  /**\brief Remove empty bins at the extremities of an histogram if any
     \param hist histogram to be cleaned
  */
  void RemoveExtremalEmptyBins( TH1 *hist );

  void ParseLegend( TH1* hist, std::string &legend );
  void ParseLegend( TGraphErrors *graph, std::string &legend );
  void ParseLegend( std::string &legend );

  /**\brief Create a boostraped Tree out of inputTrees
     \param inTrees vector of input Trees
  */
  TTree* Bootstrap( std::vector< TTree* > inTrees, unsigned int nEvents=0, unsigned long seed = 0, int mode = 0 );

  /**\brief Find the name of an object from the class type in the given tfile
     \param inFile 
     \param type Class name of the looked for object
     \param keyword keyword to be present in the searched object name
   */
  std::string FindDefaultTree( const TFile* inFile, std::string type = "TTree", std::string keyWord = "" );

  void AddTree( TTree *treeAdd, TTree *treeAdded );
  void SaveTree( TTree *inTree, std::string prefix );

  void DiffSystematics( std::string inFileName, bool update=0 );
  void VarOverTime( std::string inFileName, bool update=0);

  void RescaleStack( THStack *stack, double integral );

  //  std::map<std::string,std::string> MapAttrNode( TXMLNode* node );

  /*\brief Create the list with the all the possible combination of given name.
    Tested.
  */
  void CombineNames( const std::list< std::list<std::string> > &components, std::list<std::string> &outNames, std::string separator="_" );

  /*\brief Check the equality of double numbers by comparing them up to the 7th digit
    Tested.
  */
  inline bool CompareDouble( double a, double b )  {
    if ( a == 0 ) return ( fabs(b)<1e-7 );
    else return ( fabs((a-b)/a) < 1e-7 );
  }

  /*\brief Print a multi_array into a csv file.
    \param outName Name of the output file
    \param array Multi_array containing the double values
    \param linesTitle Vector of names for all the lines
    \param colsTitle Vector of column names. Must take into consideration possible line title.
  */
  void PrintArray( const std::string &outName, const boost::multi_array<double,2> &array, const std::vector<std::string> &linesTitle, const std::vector<std::string> &colsTitle );

  /*\brief Check if two histograms are comparable in term of binning
    Tested
  */
  bool ComparableHists( TH1* a, TH1* b );

  //Converts Epoch/Unix time into a readable date year/month
  string ConvertEpochToDate( int epochTime );
  //string GetMinMaxBranch ( vector <string> fileNames, &minVal, &maxVal );

  /**\brief Fill the values to create bin frontiers from extramal values and number of bins
     Tested.
   */
  void FillDefaultFrontiers( vector<double> &list, const int nBins, double xMin, double xMax );

  /**\brief Clean a sstring by removing successive occurences of the separator
     Tested.
   */
  std::string RemoveSeparator( std::string name, const std::string sep="_" );

  /**\brief Remove a list of words from a string
     Tested.
   */
  std::string RemoveWords( std::string name, const std::list<std::string> &toRemove );

  /**\brief Print the content of histograms into a csv file
     \param outName Name of the output file without extension
     \param vectHist 
     \param mode If 2 also print the histograms uncertainties
   */
  void PrintHist( std::vector<TObject*> &vectHist, std::string outName, int mode );

  /**\brief Replace input pointer by a copy after applying a selection
   */
  void CopyTreeSelection( TTree** inTree, const std::string &selection );

  /**\brief Write a vector of histogram into a TFile
   */
  void WriteVect( const std::vector<TObject*> &vectHist, const std::string &outName );
}

#endif


