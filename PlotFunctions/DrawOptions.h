#ifndef DRAWOPTIONS_H
#define DRAWOPTIONS_H

#include "TH1.h"
#include "TGraphErrors.h"
#include "TObject.h"
#include "TH1.h"
#include "TLine.h"
#include "TLatex.h"
#include "TH2.h"
#include "RooRealVar.h"

#include <string>
#include <vector>
#include <list>
#include <map>

namespace ChrisLib {
  
  /**\brief Class containing drawing options

     This class provides a set of options to plot a set of histograms of TGraphs efficiently.
     To use this class, instanciate an object, fill it with the desired options and call the @Draw method with the your vector of histograms.
     
     ```
     DrawOptions drawOpt;
     drawOpt.AddOption( "outName", "/home/nicePlot" ) #Changing the name of the output plot (see options for extensions)
     vector<TObject*> hists = {...}
     drawOpt.Draw( hists ); # Perform the drawing of the plot with intered options.
     ```
     If several histograms are provided, they will be superimposed to the same plot. 
     Several histograms can also be quantitatively compared (see options).
     The input object must be either TH1 (TH1D, TProfile) or TGraphErrors. 
     TH1 and TGraphErrors should not be mixed.

     Adding a wrong option to the class will print a message but procedure will continue witout the option.

     Keywords exist to put dynamic content in latex labels or legends such as histogram properties.
     These keywords are described in ChrisLib::ParseLegend.

     ### Options Description
     - outName=<string> : name of the output plot
   
     - doRatio=<int> : Create a bin by bin ratio pad of plotted histograms. 
     By default, comparison is performed with respect to the first plotted histogram (see drawStyle for other mode).
     Two values accepted : doRatio=1 performs the comparison (y2-y1)/y1 and doRatio=2 performs the per bin difference.
   
     - doChi2=<int> : Compute the chi2 between current histogram and firt drawn histogram (see drawStyle for other mode).
     The <int> is interpreted as a boolean
   
     - drawStyle=<int> : Change the behaviour of histogram comparison.
     By default (0), histograms are compared to the first drawn histogram.
     drawStyle=1 matches histogram by successive pairs and comparison are performed in between the paired histograms.
   
     - grid=<int> : Print the grid on the plot.
     By default (0) no grid is drawn. 
     An odd <int> switches on the grid for the x axis.
     <int>>1 switches on the y axis grid.
   
     - logy=<int> : Set log scale on y axis.
     <Int> is interpreted as a boolean.
   
     - orderX=<int> : Order the histogram points in increasing X.
     <Int> is interpreted as a boolean.
   
     - plotDirectory=<directory_path> : Points to the directory where the plot shoulb be saved. 

     - legend=<string> (multitoken) : Text to put in the legend of a given histogram. 
     Option is matched to the object with the same index in the file.
     If the string is empty or no legend provided, the name of the object is displayed in the legend.
     The legend option can be either not specified, or specified the exact number of object present in the plot.
     Legend accepts some special code to include histograms properties (see ChrisLib::ParseLegend).
     Some keywords change the plotting options : __FILL will fill the area between error bars,  __NOPOINT will make the histogram plotted without marker.

   
     - legendPos=<int>X <int>X : Top left relative coordinate (x,y) of the legend box.
     By default it is set to (0.7, 0.9).
   
     - latex=<string> (multitoken) : Additionnal text to be dispayed on the plot.
   
     - latexOpt=<int>X <int>Y (multitoken) : Position in relative coordinate (x,y) of the latex option with same index.
     latexOpt must be specified the same number of times than latex option.

     - extendUp=<double> : allows for an increase of <double>(%) of blank space between the top of the curve and the border of the plot.
   
     - line=<double> : draw a line accross the main pad at constant y=<double>

     - shiftColor=<int> : Translate the color of histograms by <int> unit.

     - clean<double> : remove from histograms all bins with value <double> that they all have in common.

     - offset=<double> : Translate a graph points by the value <double>. 
     If the value is set to -99, the graph is translated so its minimum is at 0.

     - orderX=<int> : Order the points of graph in increasing order. 
     <int> is interpreted as boolean.

     - forceStyle=<int> : Force the style of the inputs into the default one. True by default.

     - topMargin=<double> : Sets the top margin of the main canvas to non-default value

     - bottomMargin=<double> : Sets the bottom margin of the main cancas to non-default value
  */
  class DrawOptions {
  public :
    DrawOptions();
    DrawOptions( const std::string &outName );

    bool GetDoChi2() const { return m_bools.at("doChi2"); }
    bool GetLogY() const { return m_bools.at("logy"); }
    bool GetOrderX() const { return m_bools.at("orderX"); }

    int GetGrid() const { return m_ints.at("grid"); }
    int GetDoRatio() const { return m_ints.at("doRatio"); }
    int GetDrawStyle() const { return m_ints.at("drawStyle"); }
    int GetShiftColor() const { return m_ints.at("shiftColor"); }

    double GetExtendUp() const { return m_doubles.at("extendUp"); }
    double GetNormalize() const { return m_doubles.at("normalize"); }
    double GetScale() const { return m_doubles.at("scale"); }
    double GetLine() const { return m_doubles.at("line"); }
    double GetClean() const { return m_doubles.at("clean"); }
    double GetOffset() const { return m_doubles.at("offset"); }
    double GetTopMargin() const { return m_doubles.at("topMargin"); }
    double GetBottomMargin() const { return m_doubles.at("bottomMargin"); }
  
    
    const std::string &GetXTitle() const { return m_strings.at("xTitle"); }
    const std::string &GetYTitle() const { return m_strings.at("yTitle"); }
    const std::string &GetExtension() const { return m_strings.at("extension"); }
    const std::string &GetOutName() const { return m_strings.at("outName"); }

    const std::vector<double> &GetLegendCoord() const { return m_legendCoord; }
    const std::vector<double> &GetRangeUserX() const { return m_rangeUserX; }
    const std::vector<double> &GetRangeUserY() const { return m_rangeUserY; }

    const std::vector<std::string> &GetLegends() const { return m_legends; }
    const std::vector<std::string> &GetLatex() const { return m_latex; }
  
    const std::vector<std::vector<double>> &GetLatexPos() const { return m_latexPos; }

    void SetDebug( bool debug ) { m_debug = debug; }
  
    void SetLegends( const std::vector<std::string> &legends ) { m_legends=legends; }
    void ResetLegends() { m_legends.clear(); }
    
    /**\brief Fill the class options 
     */
    void FillOptions( const std::vector<std::string> &options );

    /**\brief Add an option to the object
     */
    void AddOption( const std::string &key, const std::string &value );

    /**\brief Add an option to the object through the use of string.
       The option name and value must be separated by a space such as : "optionName=optionLabel"
     
    */
    void AddOption( const std::string &option );

    /** \brief Plot a set of histograms on the same pad.
    
	Common algorithm to perform simple plotting of histograms, TProfile and TGraphErrors. 
	There should be no mixing of TGraph's with TH1's in the same vector.
     
	The available options accepted by the algorithm are documented in ChrisLib::DrawOptions. 
	A wrong option will create a printed warning and be ignored for the remaining of the algorithm.
     
     
	To add an option to a DrawOption object, see ChrisLib::DrawOption::AddOption.
    */
    void Draw( std::vector< TObject* > &inHist );
    void Draw( const std::vector< TH1* > &inHist );
    void Draw( const std::vector< TGraphErrors* > &inHist );
    void Draw( TH2* hist );
    void Draw( TH1* hist );
    void Draw( RooRealVar *frameVar, std::vector<TObject*> &inObj );


  private :
    
    void SetHistProperties( TH1* hist );
    void SetProperties( TObject* obj, int iHist );
    void CheckLegendCoord();
    void GetMaxValue( TObject *obj, double &minVal, double &maxVal, double &minX, double &maxX, bool takeError, bool isRef );
    void DrawText( std::vector<TObject*> &inHist );
    void DrawLegend( std::vector<TObject*> &inHist );
    void DrawLatex();
  
    std::map<std::string,bool> m_bools;
    std::map<std::string,int> m_ints;
    std::map<std::string,double> m_doubles;
    std::map<std::string,std::string> m_strings;

    std::vector<double> m_legendCoord;
    std::vector<double> m_rangeUserX;
    std::vector<double> m_rangeUserY;
  
    std::vector<std::string> m_legends;
    std::vector<std::string> m_tmpLegends;
    std::vector<std::string> m_latex;

    std::vector<std::vector<double>> m_latexPos;

    bool m_debug;

    const std::vector<int> m_colors; 
    const std::vector<int> m_fillColors;

  };

}
#endif
