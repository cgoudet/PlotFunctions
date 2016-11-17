/**	

\mainpage



\author Christophe Goudet goudetchristophe@gmail.com
\date 11/16/2016

# PlotFunctions

PlotFunctions is a library designed in the ROOT framework to :
- Provide a user friendly general procedure to plot data from TTree or CSV files.
- A generalist object plotting algorithm : ChrisLib::DrawPlot.
- Provide a class to ease the process of TTree linking, reading and copying : ChrisLib::MapBranches
- Provide a tree class which can be used to create and or parse xml files : ChrisLib::Arbre
- A set of C++ functions : SideFunctions.h SideFunctionsTpp.h
- Basic foncteurs to be used in STL algorithms : Foncteurs.h
- A set of python functions : SideFunction.py
- A robust minimization algorithm in RooFit framework : ChrisLib::FitData
- A framework to scan a likelihood.

## Plot framework
The interface of the plot framework is limited to a single configuration file in the boost::program_options format.
The user can enter information about the source files, the desired input and the plotting options in a sinngle file.

To run the program, call :
```
PlotDist <filename1> <filename2>
```
The framework can read histograms, TTree and TMatrixD from a TFile and read CSV files.
From a branch of a TTree or a column of a CSV file, the framework can create hitograms, TProfile, TGraphErros as weel as event level comparison.
CSV output of the values as weel as ROOT files containing them are possible.
The full documentation is available at ChrisLib::InputCompare


## Likelihood scan framework