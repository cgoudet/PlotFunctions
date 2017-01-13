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
     It also removes the painful adressage when configuring or creating TTrees (see examples).
     Adressage is performed by MapBranches::LinkTreeBranches and MapBranches::LinkCSVFile.

     The class accepts 6 types from TTree : int, double, float, unsigned, unsigned long long, long.
     2 types are supported from csv files : double and string.

     MapBranches::GetDouble() allows to read from both double (prioritary) and floats.

     ##Examples :

       ### Reading Tree 

       ```
       MapBranches mapBr;
       TTree *inTree = ...
       mapBr.LinkTreeBranches( inTree ); 
       inTree->GetEntry(0);
       mapBr.GetDouble( "branchName" );//Reading value from branch "branchName" of type double
       ```

       ### Copying Tree

       ```
       MapBranches mapBr;
       TTree *inTree = ...
       TTree *outTree = new TTree( "outTree", "outTree" );
       list<string> branchNames { "branch1" };//List of branches of interest.
       mapBr.LinkTreeBranches( inTree, outTree, branchNames );
       inTree->GetEntry(0);
       mapBr.SetVal( "branch1", 2*mapBr.GetVal( "branch1" ) ); //Modifying value from inTree
       outTree->Fill(); //Filling new value to outTree
       ```

       ### Tree creation

       ```
       MapBranches mapBr;
       TTree *outTree = new TTree( "outTree", "outTree" );
       list<string> branchNames { "branch1" };//List of branches of interest.
       mapBr.LinkTreeBranches( 0, outTree, branchNames );//create  branch "branch1" and link it to the object
       mapBr.SetVal( "branch1", 2 ) ); //Setting value to be written
       outTree->Fill(); //Filling new value to outTree
       ```
       

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
    
    //double GetDouble( std::string name ) const { return m_mapDouble.at( name ); }
    double GetDouble( std::string name ) const;
    double GetFloat( std::string name ) const { return m_mapFloat.at( name ); }
    int GetInt( std::string name ) const { return m_mapInt.at( name ); }
    unsigned long long GetULongLong( std::string name ) const { return m_mapULongLong.at( name ); }
    unsigned GetUnsigned( std::string name ) const { return m_mapUnsigned.at( name ); }
    std::string GetString( std::string name ) const { return m_mapString.at( name ); }
    double GetLongLong( std::string name ) const { return m_mapLongLong.at( name ); }

    /**\brief Link the branche of TTree to the class
       \param inTree TTree to be read. TTree which branches will be linked by the class.
       \param outTree TTree to be written. If outTree is paired with non-null inTree, branches will be created and match with inTree same branches.
       If outTree is paired with 0, branches of double will be created with names from branchesToLink.
       \param branchesToLink If non-empty, only this subset of branches will be linked from inTree.

     */
    void LinkTreeBranches( TTree *inTree, TTree *outTree = 0, std::list<std::string> branchesToLink = std::list<std::string>() );
    void LinkCSVFile(  std::istream &stream, const char delim = ',' );
    bool ReadCSVEntry( std::istream &stream, const char delim = ',' );

    void Print() const;

    void SetVal( std::string label, int val ) { m_mapInt[label]=val; }
    void SetVal( std::string label, std::string val ) { m_mapString[label]=val; }
    void SetVal( std::string label, double val ) { m_mapDouble[label] = val; }
    void SetVal( std::string label, float val ) { m_mapFloat[label] = val; }
    void SetVal( std::string label, unsigned long long val ) { m_mapLongLong[label] = val; }
    void SetVal( std::string label, unsigned int val ) { m_mapUnsigned[label] = val; }

    /**\brief Fill keys with the list of all branch names
     */
    void GetKeys( std::list<std::string> &keys ) const;
    /**\brief Return branch content as string
     */
    std::string GetLabel( const std::string &name ) const;
    /**\brief Reset the object 
     */
    void ClearMaps();
    /** Check the content of the object.
      */
    bool IsLinked() const;

    

  private : 
    enum CSVType{ Int, Double, String, LongLong };
    
    void LinkBranches( TTree *inTree, TTree *outTree, const std::list<std::string> &branchesToLink );
    void LinkOutputFromEmpty( TTree *outTree, const std::list<std::string> &branchesToLink );
	
    std::map< std::string, int > m_mapInt;
    std::map< std::string, double > m_mapDouble;
    std::map< std::string, float > m_mapFloat;
    std::map< std::string, long long > m_mapLongLong;
    std::map< std::string, unsigned long long > m_mapULongLong;
    std::map< std::string, unsigned int > m_mapUnsigned;
    std::map< std::string, std::string> m_mapString;

    std::vector<std::string> m_CSVColsIndex;
    std::vector<CSVType> m_CSVTypes;
  };
}

#endif 
