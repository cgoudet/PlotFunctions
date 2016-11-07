#ifndef FONCTERURS_H
#define FONCTERURS_H
#include <string>
#include <iostream>

namespace ChrisLib {
  class ReplaceString {
  public :
  ReplaceString( std::string toRemove ) : m_toRemove(toRemove),m_toReplace("") {}
  ReplaceString( std::string toRemove, std::string toReplace ) : m_toRemove(toRemove),m_toReplace(toReplace) {}
    std::string operator()( std::string name ) { 
      size_t start_pos = 0;
      while((start_pos = name.find(m_toRemove, start_pos)) != std::string::npos) {
        name.replace(start_pos, m_toRemove.length(), m_toReplace);
        start_pos += m_toReplace.length(); // Handles case where 'to' is a substring of 'from'
      }
      return name;
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
