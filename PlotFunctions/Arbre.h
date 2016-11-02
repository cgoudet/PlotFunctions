#ifndef ARBRE_H
#define ARBRE_H

#include <list>
using std::list;
#include <map>
using std::map;
#include <string>
using std::string;
#include "TXMLNode.h"
#include <vector>
using std::vector;

class Arbre 
{

 public : 
  Arbre();
  Arbre( string nodeName, list<string> attrConstraints = list<string>(), list<string> childrenConstraints = list<string>() );

  int AddChild( Arbre &child );

  void Dump( string prefix = "");

  string GetNodeName() { return m_attributes["nodeName"]; }
  map<string, string> GetAttributes() { return m_attributes; }
  list<string> GetAttributesConstraints() { return m_attrConstraints; }
  list<string> GetChildrenConstraints()  { return m_childrenConstraints; }
  list<Arbre> GetChildren() { return m_children; }
  string GetAttribute( string attribute ) { return m_attributes[attribute]; }

  void SetAttribute( string key, string value );



  static Arbre ParseXML( string inFileName );
  static Arbre CopyNode( TXMLNode * node );

  static int  GetArbresPath( Arbre &arbre , vector<Arbre> &outVect, vector<string> path = vector<string>(), vector<map<string, string>> vectOptions = vector<map<string, string>>() );

 private :
  
  

  map<string, string> m_attributes;
  list< Arbre > m_children;

  list<string> m_childrenConstraints;
  list<string> m_attrConstraints;
};

#endif
