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
#include <stdexcept>

using std::string;
using std::map;
using std::cout;
using std::endl;
using std::list;
using std::runtime_error;
using std::invalid_argument;
using std::range_error;

using namespace ChrisLib;

ChrisLib::MapBranches::MapBranches() { 
}

ChrisLib::MapBranches::~MapBranches(){}

//=================================================
void ChrisLib::MapBranches::LinkTreeBranches( TTree *inTree, TTree *outTree, list< string > branchesToLink ) {
  ClearMaps();

  if ( !inTree ) throw invalid_argument( "MapBranches::LInkTreeBranches : Null input TTree." );
  if ( branchesToLink.size() ) inTree->SetBranchStatus( "*", 0);

  TObjArray *branches = inTree->GetListOfBranches();
  TClass *expectedClass;
  EDataType expectedType;
  string name;
  for ( unsigned int iBranch = 0; iBranch < (unsigned int) branches->GetEntries(); iBranch++ ) {
    
    ( (TBranch*) (*branches)[iBranch])->GetExpectedType( expectedClass, expectedType );
    name=(*branches)[iBranch]->GetName();

    if ( branchesToLink.size() && find( branchesToLink.begin(), branchesToLink.end(), name ) == branchesToLink.end() ) continue;
    inTree->SetBranchStatus( name.c_str(), 1 );

    if ( !expectedClass ) {
      //documentation at https://root.cern.ch/doc/master/TDataType_8h.html#add4d321bb9cc51030786d53d76b8b0bd
      switch ( expectedType ) { 
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
	throw range_error( "MapBranches::LinkTreeBranches : type  " + std::to_string(expectedType) + " not implemented for branch " + name );
      }

    }
  }
  return ;
}

//==============================================
void ChrisLib::MapBranches::ClearMaps() {
  m_mapInt.clear();
  m_mapDouble.clear();
  m_mapLongLong.clear();
  m_mapULongLong.clear();
  m_mapUnsigned.clear();
}

//============================================
double ChrisLib::MapBranches::GetVal( string name ) const {

  auto itInt = m_mapInt.find( name );
  if ( itInt != m_mapInt.end() ) return static_cast<double>(itInt->second);

  auto itDouble = m_mapDouble.find( name );
  if ( itDouble != m_mapDouble.end() ) return itDouble->second;

  auto itULongLong = m_mapULongLong.find( name );
  if ( itULongLong != m_mapULongLong.end() ) return static_cast<double>(itULongLong->second);

  auto itLongLong = m_mapLongLong.find( name );
  if ( itLongLong != m_mapLongLong.end() ) return static_cast<double>(itLongLong->second);

  auto itUnsigned = m_mapUnsigned.find( name );
  if ( itUnsigned != m_mapUnsigned.end() ) return static_cast<double>(itUnsigned->second);

  throw runtime_error( "MapBranches::GetVal : No branche named " + name );
}

//=============================================
void ChrisLib::MapBranches::Print() const {

  for ( auto it = m_mapInt.begin(); it!= m_mapInt.end(); ++it  ) cout << it->first << " " << it->second << endl;
  for ( auto it = m_mapDouble.begin(); it!= m_mapDouble.end(); ++it  ) cout << it->first << " " << it->second << endl;
  for ( auto it = m_mapULongLong.begin(); it!= m_mapULongLong.end(); ++it  ) cout << it->first << " " << it->second << endl;
  for ( auto it = m_mapLongLong.begin(); it!= m_mapLongLong.end(); ++it  ) cout << it->first << " " << it->second << endl;
  for ( auto it = m_mapUnsigned.begin(); it!= m_mapUnsigned.end(); ++it  ) cout << it->first << " " << it->second << endl;
					
}

//============================================
void ChrisLib::MapBranches::GetKeys( list<string> &keys ) {
  keys.clear();
  for ( auto it = m_mapInt.begin(); it!= m_mapInt.end(); ++it  ) keys.push_back( it->first );
  for ( auto it = m_mapDouble.begin(); it!= m_mapDouble.end(); ++it  ) keys.push_back( it->first );
  for ( auto it = m_mapULongLong.begin(); it!= m_mapULongLong.end(); ++it  ) keys.push_back( it->first );
  for ( auto it = m_mapLongLong.begin(); it!= m_mapLongLong.end(); ++it  ) keys.push_back( it->first );
  for ( auto it = m_mapUnsigned.begin(); it!= m_mapUnsigned.end(); ++it  ) keys.push_back( it->first );
}
