#ifndef MAPBRANCHES_H
#define MAPBRANCHES_H

#include <string>
#include <map>
#include "TTree.h"
#include <vector>
#include <list>
//#include <fstream>
#include <istream>

namespace ChrisLib {
  
  class MapBranches {
    
  public :
    MapBranches();
    ~MapBranches();
    
    const std::map< std::string, int > &GetMapInt() const { return m_mapInt; }
    std::map< std::string, double > &GetMapDouble()  { return m_mapDouble; }
    const std::map< std::string, unsigned long long > &GetMapULongLong() const { return m_mapULongLong; }
    const std::map< std::string, long long > &GetMapLongLong() const { return m_mapLongLong; }
    const std::map< std::string, unsigned int > &GetMapUnsigned() const { return m_mapUnsigned; }

    void LinkTreeBranches( TTree *inTree, TTree *outTree = 0, std::list<std::string> branchesToLink = std::list<std::string>() );
    void LinkCSVFile(  std::istream &stream, const char delim = ',' );
    void ReadCSVEntry( std::istream &stream, const char delim = ',' );

    void Print() const;

    void SetVal( std::string label, int val ) { m_mapInt[label]=val; }
    void SetVal( std::string label, double val ) { m_mapDouble[label] = val; }
    void SetVal( std::string label, unsigned long long val ) { m_mapLongLong[label] = val; }
    void SetVal( std::string label, unsigned int val ) { m_mapUnsigned[label] = val; }

    const void* GetVal( std::string name ) const;
    void GetKeys( std::list<std::string> &keys );
    void ClearMaps();

    enum CSVType{ Int, Double, String };
    

  private : 

    std::map< std::string, int > m_mapInt;
    std::map< std::string, double > m_mapDouble;
    std::map< std::string, long long > m_mapLongLong;
    std::map< std::string, unsigned long long > m_mapULongLong;
    std::map< std::string, unsigned int > m_mapUnsigned;
    std::map< std::string, std::string> m_mapString;

    std::vector<std::string> m_CSVColsIndex;
    std::vector<CSVType> m_CSVTypes;
  };
}

#endif 
