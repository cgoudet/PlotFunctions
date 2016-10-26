#ifndef MAPBRANCHES_H
#define MAPBRANCHES_H

#include <string>
#include <map>
#include "TTree.h"
#include <vector>

using std::vector;
using std::string;
using std::map;

class MapBranches {

 public :
  MapBranches();
  ~MapBranches();

  const map< string, int > &GetMapInt() const { return m_mapInt; }
  const map< string, double > &GetMapDouble() const { return m_mapDouble; }
  const map< string, unsigned long long > &GetMapULongLong() const { return m_mapULongLong; }
  const map< string, long long > &GetMapLongLong() const { return m_mapLongLong; }
  const map< string, unsigned int > &GetMapUnsigned() const { return m_mapUnsigned; }

  void LinkTreeBranches( TTree *inTree, TTree *outTree = 0, vector<string> branchesToLink = vector<string>() );
  void Print() const;

  void SetVal( string label, int val ) { m_mapInt[label]=val; }
  void SetVal( string label, double val ) { m_mapDouble[label] = val; }
  void SetVal( string label, unsigned long long val ) { m_mapLongLong[label] = val; }
  void SetVal( string label, unsigned int val ) { m_mapUnsigned[label] = val; }

  double GetVal( string name );


 private : 
  void ClearMaps();

  map< string, int > m_mapInt;
  map< string, double > m_mapDouble;
  map< string, long long > m_mapLongLong;
  map< string, unsigned long long > m_mapULongLong;
  map< string, unsigned int > m_mapUnsigned;

};
#endif 
