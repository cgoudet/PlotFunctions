#include "PlotFunctions/MapBranches.h"
#include <iostream>
#include "TObjArray.h"
#include "TClass.h"
#include "TKey.h"
#include "TFile.h"
#include <string>
#include <map>
#include "TTree.h"
#include "PlotFunctions/SideFunctions.h"

using std::string;
using std::map;
using std::cout;
using std::endl;

MapBranches::MapBranches() { 
}

MapBranches::~MapBranches(){}

void MapBranches::LinkTreeBranches( TTree *inTree, TTree *outTree, vector< string > branchesToLink ) {

  TObjArray *branches = inTree->GetListOfBranches();
  TClass *expectedClass;
  EDataType expectedType;
  string name;
  for ( unsigned int iBranch = 0; iBranch < (unsigned int) branches->GetEntries(); iBranch++ ) {
    
    ( (TBranch*) (*branches)[iBranch])->GetExpectedType( expectedClass, expectedType );
    name=(*branches)[iBranch]->GetName();
    if ( SearchVectorBin( name, branchesToLink ) == branchesToLink.size() ) continue;

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
      default :
	cout << "bootstrap not planned for type : " << expectedType << endl;
      }
    }
  }
  return ;
}
