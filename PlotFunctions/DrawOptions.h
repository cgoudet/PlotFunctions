#ifndef DRAWOPTIONS_H
#define DRAWOPTIONS_H

#include <string>
#include <vector>
#include <list>
#include <map>

namespace ChrisLib {
  
/**\brief Class containing drawing options

     ### Options Description
     
     - doRatio=number : Create a bin by bin ratio pad of plotted histograms. 
     By default, comparison is performed with respect to the first plotted histogram (see drawStyle for other mode).
     Two modes of comparison are accepted : doRatio=1 performs the comparison (y2-y1)/y1 and doRatio=2 performs the per bin difference.


 */
class DrawOptions {
 public :
  DrawOptions();

  bool GetDoChi2() const { return m_bools.at("doChi2"); }
  bool GetGrid() const { return m_bools.at("grid"); }
  bool GetLogY() const { return m_bools.at("logy"); }
  bool GetOffset() const { return m_bools.at("offset"); }
  bool GetOrderX() const { return m_bools.at("orderX"); }
  
  int GetDoRatio() const { return m_ints.at("doRatio"); }
  int GetDrawStyle() const { return m_ints.at("drawStyle"); }
  int GetShiftColor() const { return m_ints.at("shiftColor"); }

  double GetExtendUp() const { return m_doubles.at("extendUp"); }
  double GetNormalize() const { return m_doubles.at("normalize"); }
  double GetScale() const { return m_doubles.at("scale"); }
  double GetLine() const { return m_doubles.at("line"); }
  double GetClean() const { return m_doubles.at("clean"); }
  
  std::string GetXTitle() const { return m_strings.at("xTitle"); }
  std::string GetYTitle() const { return m_strings.at("yTitle"); }
  std::string GetExtension() const { return m_strings.at("extension"); }

  const std::vector<double> &GetLegendCoord() const { return m_legendCoord; }
  const std::vector<double> &GetRangeUserX() const { return m_rangeUserX; }
  const std::vector<double> &GetRangeUserY() const { return m_rangeUserY; }

  const std::vector<std::string> &GetLegends() const { return m_legends; }
  const std::vector<std::string> &GetLatex() const { return m_latex; }
  
  const std::vector<std::vector<double>> &GetLatexPos() const { return m_latexPos; }
  
  void FillOptions( const std::vector<std::string> &options );
  void AddOption( const std::string &option );
  
 private :

  std::map<std::string,bool> m_bools;
  std::map<std::string,int> m_ints;
  std::map<std::string,double> m_doubles;
  std::map<std::string,std::string> m_strings;

  std::vector<double> m_legendCoord;
  std::vector<double> m_rangeUserX;
  std::vector<double> m_rangeUserY;
  
  std::vector<std::string> m_legends;
  std::vector<std::string> m_latex;

  std::vector<std::vector<double>> m_latexPos;
    
};

}
#endif
