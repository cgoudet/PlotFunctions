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

  /**\brief Ensures that a string finishes by a /
   */
  inline std::string AddSlash( std::string s ) { if (s.back()!='/') s+='/'; return s;}
     /**\brief Modify histograms to have a common binning as the one with most granularity.
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

  /**\brief Replace keywords with content in a string

     Two kind of keyword exist : object related and not object-related.
     The latters are documented in ChrisLib::ParseLegend( const std::string &legend ).

     - __MEAN is replaced with the mean of the histogram
     - __STDEV is replaced with the histogram standard deviation
     - __HASHTAG is replaced with a # otherwise comment caracter
     - __ENTRIES is replaced by the number of entries in the histo
     - __INTEGRAL is replaced by the integral
     - __SUMSQ is replaced with the sum of the square values of the bins.
     - __OPLUS is replaced with the quadratic sum of the bins value

  */
  string ParseLegend( TObject* obj, const string &legend );

  /**\brief replace keyword with another content in a string
     Accepted keywords and modifications : \n
   - __HASHTAG -> #
   - __ETA_CALO -> #eta_{CALO}
   - __ATLAS -> Official ATLAS text
   - __EMPTY -> null string

   Following options are replaced by an empty string : __FILL __NOPOINT __ATLAS __STACK
   */
  std::string ParseLegend( const std::string &legend );

  /**\brief Create a boostraped TTree out of inputTrees
     \param inTrees vector of input Trees
     \param nEvents number of events in the bootstraped TTree
     \param seed determine whether the TRandom generator's seed is the default one (0) or current time (1)
     \param mode determine whether the random sampling is done with (0) or without (1) replacement
  */
  TTree* Bootstrap( std::vector< TTree* > inTrees, unsigned int nEvents=0, unsigned long seed = 0, int mode = 1 );

  /**\brief Find the name of an object from the class type in the given tfile
     \param inFile
     \param type Class name of the looked for object
     \param keyword keyword to be present in the searched object name
   */
  std::string FindDefaultTree( const TFile* inFile, std::string type = "TTree", std::string keyWord = "" );

  void AddTree( TTree *treeAdd, TTree *treeAdded );
  void SaveTree( TTree *inTree, std::string prefix );

  /**\create a systematics using configFile from boost file
     Mandatory variables :
     - outFileName : Name of the output file
     - totSystName : Name of the ouput total systematic.

     Systematic variables : all described options must be entered the same number of time.
     - rootFileName : Name of the file whe an histogram of interest lies
     - histName : Name of the histogram of interest
     - systName : Name for the considered systematic.
     If the keyword "__ERR" is present in the name, the values of the histograms will be replaced by their respective error.
     - mode : Mode of the contribution of the histogram to the total uncertainty
     - update=<int> : If 0 (default) output file will be overwritten else new systematics will be added to the file.

     Modes :
     - 0XX : The systematic must be compared to another histogram.
     This other histogram is taken as the first given in the configuration file.
     The comparison is a signed difference.
     - 1XX : The histogram is directly a systematic
     - XX : options from ChrisLib::CreateSystHist for combination with total systematic.
     For example 10 will make the symmetrized root mean squatre between bins.

   */
  void DiffSystematics( std::string inFileName );

  void VarOverTime( std::string inFileName, bool update=0);

  void RescaleStack( THStack *stack, double integral );

  //  std::map<std::string,std::string> MapAttrNode( TXMLNode* node );

  /**\brief Create the list with the all the possible combination of given name.
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

  /**\brief Check if two histograms are comparable in term of x binning
   */
  bool ComparableHists( TH1* a, TH1* b );

  //Converts Epoch/Unix time into a readable date year/month
  string ConvertEpochToDate( int epochTime );
  //string GetMinMaxBranch ( vector <string> fileNames, &minVal, &maxVal );

  /**\brief Fill the values to create bin frontiers from extramal values and number of bins
   */
  void FillDefaultFrontiers( vector<double> &list, const int nBins, double xMin, double xMax );

  /**\brief Clean a sstring by removing successive occurences of the separator
   */
  std::string RemoveSeparator( std::string name, const std::string sep="_" );

  /**\brief Remove a list of words from a string
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

  double CompareVal( double nom, double par );
  double CompareErr( double nom, double par, double errNom, double errPar );
  bool IsHist( TObject* obj );

  /**\brief Function to test the content of TTree
     \param tree TTree under test
     \param branch Name of branch under test
     \return Value of first entry in branch under test

     This function throws invalid_argument for null TTree and runtime_error for empty TTree or wrong branch name.
   */
  double TestDoubleTree( TTree *tree, const string &branch );

  /**\brief Modify inHist into a systematic with respect to baseValue

     Description of the modes.
     The content in parathesis describe the action of the mode if baseValue is null.
     If no content in parenthesis, then the operation consists in comparing with a a uniformly 0 base value.

     mode%/10 :
     - 0 : quadratic sum (square)
     - 1 : sum of absolute values
     - 2 : sum
     - 3 : difference (sign flip)
     - 4 : absolute value of sum
     - 5 : absolute value of difference

     mode/10 :
     - 0 : Bin by bin
     - 1 : symmetrized bin

     Histograms must be comparable ( ChrisLib::ComparableHists ).

   */
  void CreateSystHist( TH1 *inHist, const TH1* baseValue, unsigned mode =0 );

  /**\brief REverse errors and content value
   */
  void ReverseErrVal( TH1* hist );

  /** brief Perform function on pair of columns and store the result in a column inserted after the two

      Applicable functions :
      - 0 : difference
      - 1 : relative difference
   */
  boost::multi_array<double,2> CompareArrayColumns( const boost::multi_array<double,2> &inputArray, int function=0 );

}

#endif
