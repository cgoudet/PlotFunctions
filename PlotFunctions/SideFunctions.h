#ifndef SIDEFUNCTIONS_H
#define SIDEFUNCTIONS_H
#include "THStack.h"
#include "TH1D.h"
#include "TTree.h"
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <map>
#include "boost/multi_array.hpp"
using boost::multi_array;
using boost::extents;
#include "PlotFunctions/SideFunctionsTpp.h"
#include "TGraphErrors.h"

using std::map;
using std::fstream;
using std::string;
using std::vector;
using std::cout;
using std::endl;
/**\brief Compute Chi2 of two matching histograms
   \param MCHist Histogram with MC role
   \param DataHist Histogram with Data role
   \return Chi2/ndf
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
//void WriteLatexMinipage( fstream &latexStream, vector<string> vect, unsigned int nPlotPerWidth=0, bool putNameUnder = false );

/**\brief Print latex header in tex file
   \param latexStream streamer to the output file
   \param author text to put in author option
 */
void WriteLatexHeader( fstream &latexStream, string title, string author="Christophe Goudet" );

/**\brief Remove suffix and prefix from a string
   \param inString string to be modified. 
   \param doPrefix remove everything before the last /
   \param doSuffix remove everything after the last .
   \return string copy of the modified string

   2 ways to get the results :
   - return value
   - input variable
 */
string StripString( string &inString, bool doPrefix=1, bool doSuffix=1 );

/**\brief Remove empty bins at the extremities of an histogram if any
   \param hist histogram to be cleaned
 */
void RemoveExtremalEmptyBins( TH1 *hist );

void ParseLegend( TH1* hist, string &legend );
void ParseLegend( TGraphErrors *graph, string &legend );
void ParseLegend( string &legend );

/**\brief Create a boostraped Tree out of inputTrees
   \param inTrees vector of input Trees
*/
TTree* Bootstrap( vector< TTree* > inTrees, unsigned int nEvents );

string FindDefaultTree( TFile* inFile, string type = "TTree" );
void AddTree( TTree *treeAdd, TTree *treeAdded );
void SaveTree( TTree *inTree, string prefix );

void DiffSystematics( string inFileName, unsigned int mode =0, bool update=0 );
void VarOverTime( string inFileName, bool update=0);
/* void LinkTreeBranches( TTree *inTree, TTree *outTree,  */
/* 		       map<string, double> &mapDouble,  */
/* 		       map<string, int> &mapInt,  */
/* 		       map<string, long long int > &mapLongLong */
/* 		       ); */
void RescaleStack( THStack *stack, double integral );


#endif

