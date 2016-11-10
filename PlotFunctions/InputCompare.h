#ifndef INPUTCOMPARE_H
#define INPUTCOMPARE_H

#include <vector>
#include <string>
#include <map>

/* using std::map; */
/* using std::vector; */
/* using std::string; */

namespace ChrisLib {
  class InputCompare 
  {

  public : 
    InputCompare();
    InputCompare( std::string fileName );

    //new style
    const std::vector< std::vector< std::string > > &GetObjName() const { return m_objName; }

    const std::vector<std::vector<std::string>> &GetRootFilesName() const { return m_rootFilesName; }
    const std::vector< std::string > &GetSelectionCut() const { return m_selectionCut;}
    const std::vector< double > &GetVarMin() const { return m_varMin; }
    const std::vector< double > &GetVarMax() const { return m_varMax; }
    const std::vector< std::vector<std::string> > &GetVarName() const { return m_varName; }
    const std::vector< std::vector<std::string> > &GetVarWeight() const { return m_varWeight; }
    const std::vector<std::vector< double >> &GetXBinning() const { return m_xBinning; }

    std::string GetOption( std::string option ) const { return m_mapOptions.at(option); }    
    std::string GetOutName() const { return m_outName; }

    std::vector<std::string> CreateVectorOptions() const;

    //old (wrong style
    std::vector< std::vector< std::string > > &GetRootFileName() { return m_rootFilesName; }
    std::vector< std::vector< std::string > > &GetObjName() { return m_objName; }
    std::vector< std::string > &GetLegend() { return m_legend; }
    std::vector< std::vector<std::string> > &GetVarName()  { return m_varName; }
    std::vector< std::vector<std::string> > &GetVarErrX() { return m_varErrX; }
    std::vector< std::vector<std::string> > &GetVarErrY() { return m_varErrY; }
    std::vector< double > &GetVarMin() { return m_varMin; }
    std::vector< double > &GetVarMax() { return m_varMax; }
    std::vector<std::vector< double >> &GetXBinning() { return m_xBinning; }
    std::vector< std::string > &GetLatex() { return m_latex; }
    std::vector< std::string > &GetSelectionCut() { return m_selectionCut;}
    std::vector< std::string > &GetEventID() { return m_eventID; }
    std::vector< std::string > &GetLatexOpt() { return m_latexOpt; }
       std::vector< std::vector<std::string> > &GetVarWeight() { return m_varWeight; }

    //    std::string &GetOutName() { return m_outName; }

    void  LoadFile( std::string fileName );
  
  private : 
    /**\brief names of the root files

       Files with the same first index will be added.
       Histograms generated for each index will be superimposed in the plot.
    */
    std::vector< std::vector< std::string > > m_rootFilesName;

    /**\brief names of the objects to print in each root file

       No default behaviour defined. Number of files and nae must be identical.
    */
    std::vector< std::vector< std::string > > m_objName;

    /**\brief legend texts 

       No default behaviour defined. m_legend size must be either 0 or equal to m_rootfilesname.size()
    */
    std::vector< std::string > m_legend;

    /**\brief List of branches to print in a TTree
     */
    std::vector< std::vector<std::string> > m_varName;
    std::vector<std::vector<std::string> > m_varErrX;
    std::vector<std::vector<std::string> > m_varErrY;
    /**\brief Vector of latex texts to print
     */
    std::vector< std::string > m_latex;

    /**\brief Options for latex printing
       0 : latex X
       1 : latex Y
       2 : text size
    */

    /**\brief low bound for histogram X axis for TTree drawing
     */
    std::vector< double > m_varMin;

    /**\brief high bound for histogram X axis for TTree drawing
     */
    std::vector< double > m_varMax;

    /**\brief Name of the plot without suffix or prefix
     */
    std::string m_outName ;
    std::vector< std::string > m_latexOpt;
  
    /**\brief std::string of selection for tree events
     */
    std::vector< std::string > m_selectionCut;
  

    /**\brief std::string of branch names to id the event

       branch must hold long long int
    */
    std::vector< std::string > m_eventID;

    /**\brief std::string for name of weight branch.

       Put X for no weight
    */
    std::vector< std::vector<std::string> > m_varWeight;
    std::vector< std::string > m_loadFiles;
    std::map<std::string,std::string>  m_mapOptions;
    std::vector<std::vector<double>> m_xBinning;
    //  bool m_doTabular;

  };

}
#endif
