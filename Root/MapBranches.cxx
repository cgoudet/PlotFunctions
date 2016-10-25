#include "PlotFunctions/SideFunctions.h"
#include "PlotFunctions/SideFunctionsTpp.h"
#include "PlotFunctions/MapBranches.h"

#include "TObjArray.h"
#include "TClass.h"
#include "TKey.h"
#include "TFile.h"
#include "TTree.h"

#include <iostream>
#include <string>
#include <map>
using std::string;
using std::map;
using std::cout;
using std::endl;

MapBranches::MapBranches() { 
}

MapBranches::~MapBranches(){}

void MapBranches::LinkTreeBranches( TTree *inTree, TTree *outTree, vector< string > branchesToLink ) {
  ClearMaps();

  TObjArray *branches = inTree->GetListOfBranches();
  TClass *expectedClass;
  EDataType expectedType;
  string name;
  for ( unsigned int iBranch = 0; iBranch < (unsigned int) branches->GetEntries(); iBranch++ ) {
    
    ( (TBranch*) (*branches)[iBranch])->GetExpectedType( expectedClass, expectedType );
    name=(*branches)[iBranch]->GetName();
    if ( branchesToLink.size() && SearchVectorBin( name, branchesToLink ) == branchesToLink.size() ) continue;

    if ( !expectedClass ) {
      switch ( expectedType ) { //documentation at https://root.cern.ch/doc/master/TDataType_8h.html#add4d321bb9cc51030786d53d76b8b0bd
      case 3 : {//int
	m_mapInt[name] = 0;
	inTree->SetBranchAddress( name.c_str(), &m_mapInt[name] );
	if ( outTree ) {
	  if ( !outTree->FindBranch( name.c_str()) ) outTree->Branch( name.c_str(), &m_mapInt[name] );
	  else outTree->SetBranchAddress( name.c_str(), &m_mapInt[name] );
	}
	break;}
      case 8 : {//double
	m_mapDouble[name] = 0;
	inTree->SetBranchAddress( name.c_str(), &m_mapDouble[name] );
	if ( outTree ) {
	  if ( !outTree->FindBranch( name.c_str()) ) outTree->Branch( name.c_str(), &m_mapDouble[name] );
	  else outTree->SetBranchAddress( name.c_str(), &m_mapDouble[name] );
	}
	break;}
      case 13 :
	m_mapUnsigned[ name ] = 0;
	inTree->SetBranchAddress( name.c_str(), &m_mapUnsigned[name] );
	if ( outTree ) {
	  if ( !outTree->FindBranch( name.c_str()) ) outTree->Branch( name.c_str(), &m_mapUnsigned[name] );
	  else outTree->SetBranchAddress( name.c_str(), &m_mapUnsigned[name] );
	}
	break;
      case 16 :
	m_mapLongLong[ name ] = 0;
	inTree->SetBranchAddress( name.c_str(), &m_mapLongLong[name] );
	if ( outTree ) {
	  if ( !outTree->FindBranch( name.c_str()) ) outTree->Branch( name.c_str(), &m_mapLongLong[name] );
	  else outTree->SetBranchAddress( name.c_str(), &m_mapLongLong[name] );
	}
	break;
      case 17 :
	m_mapULongLong[ name ] = 0;
	inTree->SetBranchAddress( name.c_str(), &m_mapULongLong[name] );
	if ( outTree ) {
	  if ( !outTree->FindBranch( name.c_str()) ) outTree->Branch( name.c_str(), &m_mapULongLong[name] );
	  else outTree->SetBranchAddress( name.c_str(), &m_mapULongLong[name] );
	}
	break;
      default :
	cout << "LinkTree branches not planned for type : " << expectedType << endl;
	cout << "branchName : " << name << endl;
	exit(0);
      }

    }
  }
  return ;
}

//==============================================
void MapBranches::ClearMaps() {
  m_mapInt.clear();
  m_mapDouble.clear();
  m_mapLongLong.clear();
  m_mapULongLong.clear();
  m_mapUnsigned.clear();
}

//============================================
double MapBranches::GetVal( string name ) {
  for ( auto vMap : m_mapInt ) if ( vMap.first == name ) return (double) vMap.second;
  for ( auto vMap : m_mapDouble ) if ( vMap.first == name ) return (double) vMap.second;
  for ( auto vMap : m_mapULongLong ) if ( vMap.first == name ) return (double) vMap.second;
  for ( auto vMap : m_mapLongLong ) if ( vMap.first == name ) return (double) vMap.second;
  for ( auto vMap : m_mapUnsigned ) if ( vMap.first == name ) return (double) vMap.second;

  return 0;
}
