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
#include <sstream>
#include <iterator>

using std::ostream_iterator;
using std::string;
using std::map;
using std::cout;
using std::endl;
using std::list;
using std::runtime_error;
using std::invalid_argument;
using std::range_error;
using std::istream;
using std::stringstream;
using std::to_string;
using namespace ChrisLib;

ChrisLib::MapBranches::MapBranches() { 
}

ChrisLib::MapBranches::~MapBranches(){}
//===================================================
void ChrisLib::MapBranches::LinkOutputFromEmpty( TTree *outTree, const list<string> &branchesToLink ) {
  if ( branchesToLink.empty() ) throw invalid_argument( "MapBranches::LinkOutputFromEmpty : empty list." );
  if ( outTree->GetEntries() ) throw invalid_argument( "MapBranches::LinkOutputFromEmpty : TTree must be empty." );
  if ( outTree->GetListOfBranches()->GetEntries() ) throw invalid_argument( "MapBranches::LinkOutputFromEmpty : TTree must have no branch" );
  for_each( branchesToLink.begin(), branchesToLink.end(), [this, outTree]( const string &s ) { outTree->Branch( s.c_str(), &this->m_mapDouble[s] ); } );
}
//=================================================
void ChrisLib::MapBranches::LinkTreeBranches( TTree *inTree, TTree *outTree, list< string > branchesToLink ) {
  ClearMaps();
  if ( !inTree && !outTree ) throw invalid_argument( "MapBranches::LinkTreeBranches : Null inputs TTree." );
  else if ( !inTree ) LinkOutputFromEmpty( outTree, branchesToLink );
  else LinkBranches( inTree, outTree, branchesToLink );
}
//=================================================
void ChrisLib::MapBranches::LinkBranches( TTree *inTree, TTree *outTree, const list< string > &branchesToLink ) {
  if ( !branchesToLink.empty() ) inTree->SetBranchStatus( "*", 0);

  TObjArray *branches = inTree->GetListOfBranches();
  for ( unsigned int iBranch = 0; iBranch < static_cast<unsigned int>(branches->GetEntries()); iBranch++ ) {

    TClass *expectedClass;
    EDataType expectedType;
    
    ( (TBranch*) (*branches)[iBranch])->GetExpectedType( expectedClass, expectedType );
    string name=(*branches)[iBranch]->GetName();
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
      case 5 : {//float
	m_mapFloat[name] = 0;
	inTree->SetBranchAddress( name.c_str(), &m_mapFloat[name] );
	if ( outTree ) {
	  if ( !outTree->FindBranch( name.c_str()) ) outTree->Branch( name.c_str(), &m_mapFloat[name] );
	  else outTree->SetBranchAddress( name.c_str(), &m_mapFloat[name] );
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
  m_mapFloat.clear();
  m_mapDouble.clear();
  m_mapLongLong.clear();
  m_mapULongLong.clear();
  m_mapUnsigned.clear();
  m_mapString.clear();
}

//=============================================
void ChrisLib::MapBranches::Print() const {

  for ( auto it = m_mapInt.begin(); it!= m_mapInt.end(); ++it  ) cout << "int " << it->first << " " << it->second << endl;
  for ( auto it = m_mapFloat.begin(); it!= m_mapFloat.end(); ++it  ) cout << "float " << it->first << " " << it->second << endl;
  for ( auto it = m_mapDouble.begin(); it!= m_mapDouble.end(); ++it  ) cout << "double "<< it->first << " " << it->second << endl;
  for ( auto it = m_mapULongLong.begin(); it!= m_mapULongLong.end(); ++it  ) cout << "ULongLong " << it->first << " " << it->second << endl;
  for ( auto it = m_mapLongLong.begin(); it!= m_mapLongLong.end(); ++it  ) cout << "LongLong " << it->first << " " << it->second << endl;
  for ( auto it = m_mapUnsigned.begin(); it!= m_mapUnsigned.end(); ++it  ) cout << "Unsigned " << it->first << " " << it->second << endl;
  for ( auto it = m_mapString.begin(); it!= m_mapString.end(); ++it  ) cout << "string " << it->first << " " << it->second << endl;
}

//============================================
void ChrisLib::MapBranches::GetKeys( list<string> &keys ) const {
  keys.clear();
  for ( auto it = m_mapInt.begin(); it!= m_mapInt.end(); ++it  ) keys.push_back( it->first );
  for ( auto it = m_mapFloat.begin(); it!= m_mapFloat.end(); ++it  ) keys.push_back( it->first );
  for ( auto it = m_mapDouble.begin(); it!= m_mapDouble.end(); ++it  ) keys.push_back( it->first );
  for ( auto it = m_mapULongLong.begin(); it!= m_mapULongLong.end(); ++it  ) keys.push_back( it->first );
  for ( auto it = m_mapLongLong.begin(); it!= m_mapLongLong.end(); ++it  ) keys.push_back( it->first );
  for ( auto it = m_mapUnsigned.begin(); it!= m_mapUnsigned.end(); ++it  ) keys.push_back( it->first );
  for ( auto it = m_mapString.begin(); it!= m_mapString.end(); ++it  ) keys.push_back( it->first );
}

//============================================
void ChrisLib::MapBranches::LinkCSVFile( istream &stream, const char delim ) {
  //Check the first two lines to setup the class and put back the reader at the firs data line

  stream.seekg( 0, stream.beg );

  unsigned int nCols = 0;
  char line[500];
  bool isFirstLineTitle=true;
  //  int iValue=0;
  double dValue=0;
  string sValue;  
  unsigned nStringL2=0;
  for ( unsigned iLine=0; iLine<2; ++iLine ) {

    stream.getline( line, 500 );
    stringstream firstLine(line);

    if ( !iLine ) {
      sValue = line;
      nCols = std::count( sValue.begin(), sValue.end(), delim )+1;
      m_CSVColsIndex = vector<string>(nCols);
      m_CSVTypes = vector<CSVType>(nCols, CSVType::String );
    }

    for ( unsigned iCol=0; iCol<nCols; ++iCol ) {
      // if ( firstLine >> iValue ) {
      // 	if ( !iLine ) isFirstLineTitle=false;
      // 	m_CSVTypes[iCol] = CSVType::Int;
      // 	cout << iValue << endl;
      // 	firstLine.getline(line, 500, delim );
      // 	continue;
      // }
      if ( firstLine >> dValue ) {
	if (!iLine) isFirstLineTitle=false;
	m_CSVTypes[iCol] = CSVType::Double;
	firstLine.getline(line, 500, delim );
	continue;
      }
      
      if ( !firstLine.good() ) firstLine.clear();

      firstLine.getline(line, 500, delim );
      if ( !iLine ) m_CSVColsIndex[iCol] = line;
      else ++nStringL2;
    }
    // cout << "back1 :  " << m_CSVColsIndex.back() << endl;
    // if ( m_CSVColsIndex.back()=="" ) {
    //   cout << "eof : " << firstLine.eof() << endl;
    //   cout << "failbit : " << !firstLine.good() << endl;
    //   firstLine.getline( line, 500 );
    //   cout << "line : " << line << endl;
    //   m_CSVColsIndex.back() = line;
    //   cout << "back : " << m_CSVColsIndex.back() << endl;
    // }
  }

  if ( nStringL2 == nCols ) isFirstLineTitle=false;


  for ( unsigned iCol=0; iCol<nCols; ++iCol ) {
    //    cout << m_CSVColsIndex[iCol] << " " << m_CSVTypes[iCol] << endl;
    if ( !isFirstLineTitle ) m_CSVColsIndex[iCol] = to_string(iCol);
    if ( m_CSVTypes[iCol] == CSVType::Int ) m_mapInt[m_CSVColsIndex[iCol]]=0;
    else if ( m_CSVTypes[iCol] == CSVType::Double ) m_mapDouble[m_CSVColsIndex[iCol]]=0;
    else if ( m_CSVTypes[iCol] == CSVType::String ) m_mapString[m_CSVColsIndex[iCol]]="";
  }
  
  stream.seekg( 0, stream.beg );
  stream.getline(line, 500 );

  // cout << "keys: "<< endl;
  // list<string> keys;
  // GetKeys( keys );
}


//============================================================
bool ChrisLib::MapBranches::ReadCSVEntry( istream &stream, const char delim ) {

  //  int iValue=0;
  double dValue=0;

  char line[500];
  stream.getline( line, 500 );
  if ( stream.eof() ) return false;
  stringstream firstLine(line);
  unsigned nCols = m_CSVColsIndex.size();
  if ( !nCols ) throw runtime_error( "MapBranches::ReadCSVEntry : No column have been linked." );

  for ( unsigned iCol=0; iCol<nCols; ++iCol ) {
    //    cout << iCol << " " << m_CSVColsIndex[iCol] << endl;
    // if ( m_CSVTypes[iCol]==CSVType::Int ) {
    //   firstLine >> iValue;
    //   if ( !firstLine.good() ) throw runtime_error( "MapBranches::ReadCSVEntry : Can not read integer in column " + m_CSVColsIndex[iCol]);
    //   m_mapInt.at(m_CSVColsIndex[iCol]) = iValue;
    // }
    if ( m_CSVTypes[iCol]==CSVType::Double ) {
      firstLine >> dValue;
      //      if ( firstLine.eof() ) return;
      if ( firstLine.fail() ) throw runtime_error( "MapBranches::ReadCSVEntry : Can not read double in column " + m_CSVColsIndex[iCol]);
      m_mapDouble.at(m_CSVColsIndex[iCol]) = dValue;
      //      cout << "double : " << m_CSVColsIndex[iCol] << " " << dValue << endl;
      firstLine.getline(line, 500, delim );
    }
    else if ( m_CSVTypes[iCol]==CSVType::String ) {
      firstLine.getline(line, 500, delim );
      if ( firstLine.eof() ) return false;
      if ( firstLine.fail() ) throw runtime_error( "MapBranches::ReadCSVEntry : Can not read string in column " + m_CSVColsIndex[iCol]);
      m_mapString.at(m_CSVColsIndex[iCol]) = line;      
      //      cout << "string : " << m_CSVColsIndex[iCol] << " " << line << endl;
    }
  }
  return true;
}
//========================================
string ChrisLib::MapBranches::GetLabel( const string &name ) const {

  auto itInt = m_mapInt.find( name );
  if ( itInt != m_mapInt.end() ) return to_string(itInt->second);

  auto itFloat = m_mapFloat.find( name );
  if ( itFloat != m_mapFloat.end() ) return to_string(itFloat->second);

  auto itDouble = m_mapDouble.find( name );
  if ( itDouble != m_mapDouble.end() ) return to_string(itDouble->second);
  
  auto itULongLong = m_mapULongLong.find( name );
  if ( itULongLong != m_mapULongLong.end() ) return to_string(itULongLong->second);

  auto itLongLong = m_mapLongLong.find( name );
  if ( itLongLong != m_mapLongLong.end() ) return to_string(itLongLong->second);

  auto itUnsigned = m_mapUnsigned.find( name );
  if ( itUnsigned != m_mapUnsigned.end() ) return to_string(itUnsigned->second);

  return GetString( name );

}
//=======================================================
bool ChrisLib::MapBranches::IsLinked() const {

  return ( m_mapInt.size() ||
	   m_mapFloat.size() ||
	   m_mapDouble.size() ||
	   m_mapULongLong.size() ||
	   m_mapLongLong.size() ||
	   m_mapUnsigned.size() ||
	   m_mapString.size()
	   );

}


//========================================================
double ChrisLib::MapBranches::GetDouble( string name ) const {
  
  auto itDouble = m_mapDouble.find( name );
  if ( itDouble != m_mapDouble.end() ) return itDouble->second;

  auto itFloat = m_mapFloat.find( name );
  if ( itFloat != m_mapFloat.end() ) return itFloat->second;
  
  throw range_error( "MapBranches::GetDouble : " + name + " is neither a double or a float." );
}

//===================================================
