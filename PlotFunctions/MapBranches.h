#ifndef MAPBRANCHES_H
#define MAPBRANCHES_H

#include <string>
#include <map>
#include "TTree.h"
#include <vector>
#include <list>


class MapBranches {

 public :
  MapBranches();
  ~MapBranches();

  std::map< std::string, int > &GetMapInt()  { return m_mapInt; }
  std::map< std::string, double > &GetMapDouble()  { return m_mapDouble; }
  std::map< std::string, unsigned long long > &GetMapULongLong()  { return m_mapULongLong; }
  std::map< std::string, long long > &GetMapLongLong()  { return m_mapLongLong; }
  std::map< std::string, unsigned int > &GetMapUnsigned()  { return m_mapUnsigned; }
  
  void LinkTreeBranches( TTree *inTree, TTree *outTree = 0, std::list<std::string> branchesToLink = std::list<std::string>() );
  void Print() const;

  void SetVal( std::string label, int val ) { m_mapInt[label]=val; }
  void SetVal( std::string label, double val ) { m_mapDouble[label] = val; }
  void SetVal( std::string label, unsigned long long val ) { m_mapLongLong[label] = val; }
  void SetVal( std::string label, unsigned int val ) { m_mapUnsigned[label] = val; }

  double GetVal( std::string name ) const;
  void GetKeys( std::list<std::string> &keys );

 private : 
  void ClearMaps();

  std::map< std::string, int > m_mapInt;
  std::map< std::string, double > m_mapDouble;
  std::map< std::string, long long > m_mapLongLong;
  std::map< std::string, unsigned long long > m_mapULongLong;
  std::map< std::string, unsigned int > m_mapUnsigned;

};
#endif 
