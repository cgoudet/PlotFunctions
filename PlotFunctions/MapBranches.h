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

  /*!\brief Wrapper to ease the use of TTree and CSV files

     The MapBranches class allows for a common ways of reading entries of a TTree or streams.
     By default the csv format is implied but delimiter can be changed.
     It also removes the painful adressage when configuring TTrees.

     The class accepts 5 types from TTree : int, double, unsigned, unsigned long long, long.
     2 types are supported from csv files : double and string.

     ### Reading TTree
     
     - Create a list of the branches that are to be read. 
     An empty list implies that all branches are to be read.
     
     - Call ChrisLib::MapBranches::LinkTreeBranches the perform the adressage.
     
     - Call TTree::GetEntry
     
     - To read a variable, one must know the type of the looked for variable.
     One getter is implemented for each accepted type, for example ChrisLib::MapBranches::GetDouble ( branchName ).
     This function returns a void pointer so the user must explicitely cast the output to the desired type.
     The complete line would look : \n
     value = *static_cast<const double *>( mapBranch.GetVal( branchName ) )

     ### Copying TTRee
     The class provide the possibility to create additional branches to a TTree, directly linked to the branche with the same name in the first TTree.
     To do so, just put the second TTree as the second argument of ChrisLib::MapBranches::LinkTreeBranches.
     When calling tree2->Fill(), the values of the current event of tree1 will be copied.

     One can change values of the current event by calling ChrisLib::MapBranches::SetVal ( branchName, value ).
     value must have the same type as the branch. No implicit conversion is performed.

     ### Reading stream
     - Call ChrisLib::MapBranches::LinkCSVFile with your input stream (for example, your fstream object ).
     The header and types are automatically set and the buffer is moved to the begining of data.
     - Call ChrisLib::MapBranches::ReadCSVEntry to read the next entry
     - Call ChrisLib::MapBranches::GetVal to read the value. See 'Reading TTree' for more details.
     
     Carefull : End of file is not reach when reading the last line for the first time. 
     One must then test it after calling ReadCSVEntry to not read last line twice.
   */
  class MapBranches {
    
  public :
    MapBranches();
    ~MapBranches();
    
    double GetDouble( std::string name ) const { return m_mapDouble.at( name ); }
    int GetInt( std::string name ) const { return m_mapInt.at( name ); }
    unsigned long long GetULongLong( std::string name ) const { return m_mapULongLong.at( name ); }
    unsigned GetUnsigned( std::string name ) const { return m_mapUnsigned.at( name ); }
    std::string GetString( std::string name ) const { return m_mapString.at( name ); }
    double GetLongLong( std::string name ) const { return m_mapLongLong.at( name ); }

    void LinkTreeBranches( TTree *inTree, TTree *outTree = 0, std::list<std::string> branchesToLink = std::list<std::string>() );
    void LinkCSVFile(  std::istream &stream, const char delim = ',' );
    void ReadCSVEntry( std::istream &stream, const char delim = ',' );

    void Print() const;

    void SetVal( std::string label, int val ) { m_mapInt[label]=val; }
    void SetVal( std::string label, double val ) { m_mapDouble[label] = val; }
    void SetVal( std::string label, unsigned long long val ) { m_mapLongLong[label] = val; }
    void SetVal( std::string label, unsigned int val ) { m_mapUnsigned[label] = val; }

    void GetKeys( std::list<std::string> &keys ) const;
    std::string GetLabel( const std::string &name ) const;
    void ClearMaps();
    bool IsLinked() const;
    enum CSVType{ Int, Double, String, LongLong };
    

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
