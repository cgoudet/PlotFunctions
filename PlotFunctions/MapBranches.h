#ifndef MAPBRANCHES_H
#define MAPBRANCHES_H

#include <string>
#include <map>
#include "TTree.h"

using std::string;
using std::map;

class MapBranches {

 public :
  MapBranches();
  ~MapBranches();

  map< string, int > &GetMapInt() { return m_mapInt; }
  map< string, double > &GetMapDouble() { return m_mapDouble; }
  map< string, unsigned long long > &GetMapLongLong() { return m_mapLongLong; }

  void LinkTreeBranches( TTree *inTree, TTree *outTree = 0 );
  void SetVal( string label, int val ) { m_mapInt[label]=val; }
  void SetVal( string label, double val ) { m_mapDouble[label] = val; }
  void SetVal( string label, unsigned long long val ) { m_mapLongLong[label] = val; }


 private : 
  map< string, int > m_mapInt;
  map< string, double > m_mapDouble;
  map< string, unsigned long long > m_mapLongLong;



};
#endif 
