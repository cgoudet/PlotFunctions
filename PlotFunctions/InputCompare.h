#ifndef INPUTCOMPARE_H
#define INPUTCOMPARE_H

#include <vector>
#include <string>
#include <map>


namespace ChrisLib {

  /**
     \class InputCompare
     \brief Stores the content of boost configuration files.

     ## Plot framework
     The interface of the plot framework is limited to a single configuration file in the boost::program_options format.
     The user can enter information about the source files, the desired input and the plotting options in a sinngle file.
     One line of configuration file consists in the name followed by '=' and the content of the option.
     the character '#' is used for comments.
     For example :
     ```
     rootFileName=dum.root
     # This is a comment
     ```

     To run the program, call :
     ```
     PlotDist <filename1> <filename2>
     ```

     The framework currenlty accepts the inputs : 
     - TTree
     - TH1
     - CSV file
     - TMatrixD
     CSV and TTree can be mixed in a sigle plot. 

     Depending on the input type and the wanted results, different types of output are available :
     - TH1D
     - TProfile
     - TGraphErrors
     - Event level comparison

     Simple options (i.e. not structured) are stored in string and must be check and converted upon utilization.
     They can all be retrieved using GetOption.
     Structured options may be subject to default modification. 
     If so the modification is documented in the option description.
     The set of all options is described below which the inputs types which accept them.

     The plotting of the output is performed using the algorithm ChrisLib::DrawPlot. 
     The options for this algorithm must also be included in the configuration file.
     The list of all possible options are documented in ChrisLib::DrawOptions

     ### Options Description
     
     InputCompare can store all options for both PlotDist.cxx and ChrisLib::DrawPlot.
     Only options related to PlotDist.cxx are documented here.
     Multitoken options match an option with the objects with the same rootFileName index.

     - inputType=number : ID of the routine to be called by PlotDist.cxx.
     Description of the possibilities and their meaning is documented below.
     
     - rootFileName= file1 file2 ... : (Multitoken) Names of the data files.
     Content of file1 and file2 will be merge into the same histogram/object.
     
     - objName= name1 name2 ...  : (Multitoken) Names of the objects within data files
     By default, the number of objName will be increase to match the number of rootFileName' with empty values.
     No default constraint of the number of names with an entry.
 
     - varName= name1 name2 ... : (Multitoken) Name ID of the data to be plotted.
     Number of names in each occurence must match.
     By default, the number of varNames will be increased to match the number of rootFileName' by copying last entry.
     
     - varYName= name1 name2 ... : (Multitoken) Name ID of the Y axis data.
     By default, the number of varNames will be increased to match the number of rootFileName' by copying last entry.
     
     - varErrX= name1 name2 ... : (Multitoken) Name ID of the variable representing X uncertainty for TGraphErrors. Empty means 0.
     - varErrY= name1 name2 ... : (Multitoken) Name ID of the variable representing Y uncertainty for TGraphErrors. Empty means 0.

     - varMin= value1 value2 ... : (Multitoken) Low range limit for histograms.
     - varMax= value1 value2 ... : (Multitoken) High range limit for histograms.
     - nBins=number : Number of bins for histograms (default 100)
     - nEvents=number : Number of events to consider in each dataset. 0 means all.
     
     - selectionCut= cut : (Multitoken) Selection to apply to all TTrees with same index in rootFileName
     Empty means no selection. 
     Variables are refered to their branch name.

     - eventID= value1 value2 ...: Name ID of variables used to identify a specific event. 

     - varWeight= var1 var2 ... : (Multitoken) Name ID of the data used as weights. Empty means 1.

     - xBinning= frontier1 frontier2 ... : (Multitoken) Frontiers of xAxis for variable bins histograms.

     - plotDirectory= path : Directory in which outputs will be saved.

     - doLabels=number : Boolean value changing histograms xAxis from numbers to string labels.
     
     - saveRoot=number : Boolean value saving the output objects in a rootFile.

     - doTabular=mode : Saves the content of output objects into csv file. 
     Authorized modes and effects are documented below. 0 means no csv file.

     - loadFiles=file : (Multitoken) Configuration files to be loaded into the class. 
     Files are loaded in the order of the occurence.

     - triangular=number : Boolean allowing to plot only triangular matrix.


     ### inputType Description
     The PlotDist framework allows for variouts operation on various types of input data.
     The inputType option define which operation will be performed.

     Options 1-4 allow the reading of a database to fill histogram or graphs. 
     Currently TTree and CSV files can be read indiferently.
     They are identified by the extension of the rootFileName (with or without '.root' ).

     rootFileName option is mandatory for all cases.
     The option Binning refer to either providing a xBinning or the combination of (nBins,varMin,varMax).


     - 0 : Plot histograms from ROOT files (ChrisLib::PlotHist)\n
     Mandatory variables : objName 

     - 1 : Read a variables of given branches and plot the distribution in an histogram.
     \n
     Mandatory variables : Binning, varName
     \n
     Optionnal variables : varWeight, doTabular, saveRoot, doLabels, objName

     - 2 : Compare the same event values across different datasets. Values are printed into a csv file.
     \n
     Mandatory variables : Binning, varName, eventID
     \n
     Optionnal variables : varWeight, doTabular, saveRoot, doLabels, objName

     - 3 : Fill a TProfile with varName as Xaxis, varYName as Yaxis.
     \n
     Mandatory variables : Binning, varName, varYName
     \n
     Optionnal variables : varWeight, doTabular, saveRoot, doLabels, objName

     - 4 : Fill a TGraph errors with varName as Xaxis, varYName as Yaxis. Errors are 0 if unspecified.
     \n
     Mandatory variables : varName, varYName
     \n
     Optionnal variables : varErrX, varErrY, doTabular, saveRoot, doLabels

     - 5 : Split input TTrees into two subTrees which either pass or fail a selection.
     \n
     Mandatory variables : selectionCut
     \n
     
     - 6 : Plot TMatrix values on an histogram with coordinates as labels.
     \n
     Mandatory variables : objName
     \n
     Optionnal variables : triangular, doTabular, saveRoot


     ### doTabular Description
     doTabular options allows to access values inside of the entries of an object (TH1D, TProfile or TGraphErrors) in the form of a csv file.
     The first line is a header with the title of the histogram, which can be changed through the use of the legend option of ChrisLib::DrawPlot.
     The first column is defined by either interval of the corresponding bin, the label of the bin, of the x value of the first point.

     It is possible to print the values of several objects of mixted types.
     They must all have the same number of bins/points.
     The first column will be defined by the first object.
     
     Three modes are possible :
     - 0 : No csv output
     - 1 : Print the value of each bin.
     - 2 : Print the value and the Y uncertainty of each bin
     - 3 : Print the value and the Y and X uncertainty of each bin. 
     
 */

  class InputCompare 
  {

  public : 
    InputCompare();
    InputCompare( std::string fileName );

    //new style
    const std::vector< std::string > &GetEventID() const { return m_eventID; }
    const std::vector< std::vector< std::string > > &GetObjName() const { return m_objName; }
    const std::vector<std::vector<std::string>> &GetRootFilesName() const { return m_rootFilesName; }
    const std::vector< std::string > &GetSelectionCut() const { return m_selectionCut;}
    const std::vector< double > &GetVarMin() const { return m_varMin; }
    const std::vector< double > &GetVarMax() const { return m_varMax; }
    const std::vector< std::vector<std::string> > &GetVarName() const { return m_varName; }
    const std::vector< std::vector<std::string> > &GetVarYName() const { return m_varYName; }
    const std::vector< std::vector<std::string> > &GetVarWeight() const { return m_varWeight; }
    const std::vector<std::vector< double >> &GetXBinning() const { return m_xBinning; }
    const std::vector< std::vector<std::string> > &GetVarErrX() const { return m_varErrX; }
    const std::vector< std::vector<std::string> > &GetVarErrY() const { return m_varErrY; }

    std::string GetOption( std::string option ) const { return m_mapOptions.at(option); }    
    std::string GetOutName() const { return m_outName; }

    /**
       \brief Create the vector of options to be read by ChrisLib::DrawPlot
    */
    std::vector<std::string> CreateVectorOptions() const;

    void  LoadFile( std::string fileName );
  
  private : 
    std::vector< std::vector< std::string > > m_rootFilesName;

    std::vector< std::vector< std::string > > m_objName;

    std::vector< std::string > m_legend;

    std::vector< std::vector<std::string> > m_varName;
    std::vector< std::vector<std::string> > m_varYName;
    std::vector< std::vector<std::string> > m_varErrX;
    std::vector< std::vector<std::string> > m_varErrY;
    std::vector< std::string > m_latex;

    std::vector< double > m_varMin;

    std::vector< double > m_varMax;

    std::string m_outName ;
    std::vector< std::string > m_latexOpt;
    std::vector< std::string > m_selectionCut;
  
    std::vector< std::string > m_eventID;

    std::vector< std::vector<std::string> > m_varWeight;
    std::vector< std::string > m_loadFiles;
    std::map<std::string,std::string>  m_mapOptions;
    std::vector<std::vector<double>> m_xBinning;

  };

}
#endif
