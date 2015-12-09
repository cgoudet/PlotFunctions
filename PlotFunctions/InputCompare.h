#ifndef INPUTCOMPARE_H
#define INPUTCOMPARE_H

#include <vector>
#include <string>
#include <map>

using std::map;
using std::vector;
using std::string;

class InputCompare 
{

 public : 
  InputCompare();
  InputCompare( string fileName );

  vector< vector< string > > &GetRootFileName() { return m_rootFileName; }
  vector< vector< string > > &GetObjName() { return m_objName; }
  vector< string > &GetLegend() { return m_legend; }
  vector< vector<string> > &GetVarName() { return m_varName; }
  vector< double > &GetVarMin() { return m_varMin; }
  vector< double > &GetVarMax() { return m_varMax; }
  vector< double > &GetXBinning() { return m_xBinning; }
  vector< string > &GetLatex() { return m_latex; }
  vector< string > &GetSelectionCut() { return m_selectionCut;}
  vector< string > &GetEventID() { return m_eventID; }
  vector< string > &GetLatexOpt() { return m_latexOpt; }
  vector< string > &GetVarWeight() { return m_varWeight; }

  string &GetOutName() { return m_outName; }
  string GetOption( string option ) { return m_mapOptions[option]; }
  void  LoadFile( string fileName );
  vector<string> CreateVectorOptions();
 private : 
  /**\brief names of the root files

     Files with the same first index will be added.
     Histograms generated for each index will be superimposed in the plot.
  */
  vector< vector< string > > m_rootFileName;

  /**\brief names of the objects to print in each root file

     No default behaviour defined. Number of files and nae must be identical.
  */
  vector< vector< string > > m_objName;

  /**\brief legend texts 

     No default behaviour defined. m_legend size must be either 0 or equal to m_rootFileName.size()
  */
  vector< string > m_legend;

  /**\brief List of branches to print in a TTree
   */
  vector< vector<string> > m_varName;

  /**\brief Vector of latex texts to print
   */
  vector< string > m_latex;

  /**\brief Options for latex printing
     0 : latex X
     1 : latex Y
     2 : text size
   */

  /**\brief low bound for histogram X axis for TTree drawing
   */
  vector< double > m_varMin;

  /**\brief high bound for histogram X axis for TTree drawing
   */
  vector< double > m_varMax;

  /**\brief Name of the plot without suffix or prefix
   */
  string m_outName ;
  vector< string > m_latexOpt;
  
  /**\brief Type of object that will be plotted
    0 : TH1
    1 : TTree
    2 : comparison event by event
    eventID : names of branches that identify an event
    varName : names of the branches to compare
    nComparedEvents : number of events to be compared
    lengend : name of the column in csv file

    3 : plot text files
    4 : merging some histograms into 1 root file
    5 : separate a set of tree into two subTree according to a selection
    6 : cmopare all bins of histograms ( of the same size )  as a function of the legend (as label)
    7 : Compare TMatrixD
    8 : TProfile
    varName : xAxis yAxis yAxis ....
    
  */

  /**\brief string of selection for tree events
   */
  vector< string > m_selectionCut;
  

  /**\brief string of branch names to id the event

     branch must hold long long int
  */
  vector< string > m_eventID;

  /**\brief string for name of weight branch.

     Put X for no weight
  */
  vector< string > m_varWeight;
  vector< string > m_loadFiles;
  map<string,string>  m_mapOptions;
  vector<double> m_xBinning;
};

#endif
