#ifndef INPUTCOMPARE_H
#define INPUTCOMPARE_H

#include <vector>
#include <string>

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
  vector< double > &GetLegendPos() { return m_legendPos; }
  vector< double > &GetRangeUser() { return m_rangeUser; }
  vector< double > &GetVarMin() { return m_varMin; }
  vector< double > &GetVarMax() { return m_varMax; }
  vector< string > &GetLatex() { return m_latex; }
  vector< string > &GetSelectionCut() { return m_selectionCut;}
  vector< string > &GetEventID() { return m_eventID; }
  vector< vector< double > > &GetLatexOpt() { return m_latexOpt; }
  vector< string > &GetVarWeight() { return m_varWeight; }

  string &GetOutName() { return m_outName; }
  unsigned int GetInputType() { return m_inputType; }
  unsigned int GetDoRatio() { return m_doRatio; }
  unsigned int GetNormalize() { return m_normalize; }
  unsigned int GetDoChi2() { return m_doChi2; }
  unsigned int GetCenterZoom() { return m_centerZoom; }
  unsigned int GetDrawStyle() { return m_drawStyle; }
  unsigned int GetNComparedEvents() { return m_nComparedEvents; }
  unsigned int GetShiftColor() { return m_shiftColor; }

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
  vector< double > m_legendPos;

  /**\brief display Y range for the plot
   */
  vector< double > m_rangeUser;

  /**\brief low bound for histogram X axis for TTree drawing
   */
  vector< double > m_varMin;

  /**\brief high bound for histogram X axis for TTree drawing
   */
  vector< double > m_varMax;

  /**\brief Name of the plot without suffix or prefix
   */
  string m_outName ;
  vector< vector< double > > m_latexOpt;
  
  /**\brief Type of object that will be plotted
    0 : TH1
    1 : TTree
    2 : comparison event by event
    3 : plot text files
    4 : merging some histograms into 1 root file
    5 : separate a set of tree into two subTree according to a selection
  */
  unsigned int m_inputType;

  /**\brief decide wether perform the ratio of histograms.
   */
  unsigned int m_doRatio;

  /**\brief normalize histograms
   */
  unsigned int m_normalize;

  /**\brief compute chi2 betwwen histograms
   */
  unsigned int m_doChi2;

  /**\brief center X axis to non empty bins
   */
  unsigned int m_centerZoom;

  /**\brief Scheme to pair histograms
   */
  unsigned int m_drawStyle;

  /**\brief string of selection for tree events
   */
  vector< string > m_selectionCut;
  
  /**\brief number of event to compare in inputType2
   */
  unsigned int m_nComparedEvents;

  /**\brief string of branch names to id the event

     branch must hold long long int
  */
  vector< string > m_eventID;

  /**\brief string for name of weight branch.

     Put X for no weight
  */
  vector< string > m_varWeight;

  /**\value to shift colors of histograms
   */
  unsigned int m_shiftColor;
};

#endif
