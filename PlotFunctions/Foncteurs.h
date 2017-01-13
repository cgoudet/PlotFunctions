#ifndef FONCTERURS_H
#define FONCTERURS_H
#include <string>
#include <iostream>
#include <exception>

namespace ChrisLib {
  class ReplaceString {
  public :
  ReplaceString( std::string toRemove ) : m_toRemove(toRemove),m_toReplace("") {
      if ( toRemove == "" ) throw std::invalid_argument( "ReplaceString::ReplaceString : Forbidden ot remove empty string" );
    }
    
  ReplaceString( std::string toRemove, std::string toReplace ) : ReplaceString(toRemove) {
      m_toReplace = toReplace;
    }
    
    std::string operator()( std::string name ) { 

      std::string newString;
      newString.reserve( name.length() );  // avoids a few memory allocations
      
      std::string::size_type lastPos = 0;
      std::string::size_type findPos;
      
      while( std::string::npos != ( findPos = name.find( m_toRemove, lastPos )))
	{
	  newString.append( name, lastPos, findPos - lastPos );
	  newString += m_toReplace;
	  lastPos = findPos + m_toRemove.length();
	}

      // Care for the rest after last occurrence
      newString += name.substr( lastPos );
      
      return newString;
    }

  private : 
    std::string m_toRemove;
    std::string m_toReplace;
  };

  //#########################################"
  class Prefix {
  public:
  Prefix( const std::string prefix ) : m_prefix(prefix) {}
    std::string operator()( const std::string &s ) { return m_prefix+s; }

  private : 
    std::string m_prefix;
  };
}

#endif
