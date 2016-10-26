#ifndef FONCTERURS_H
#define FONCTERURS_H
#include <string>

class ReplaceString {
 public :
 ReplaceString( std::string toRemove ) : m_toRemove(toRemove),m_toReplace("") {}
 ReplaceString( std::string toRemove, std::string toReplace ) : m_toRemove(toRemove),m_toReplace(toReplace) {}
  std::string operator()( std::string const &name ) { 
    std::string outStr = name;
    std::size_t pos = outStr.find(m_toRemove);
    while( pos != std::string::npos ) {
      outStr.replace( pos, pos+m_toRemove.size(), m_toReplace );
      pos = outStr.find(m_toRemove);
    }
    return outStr;
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


#endif
