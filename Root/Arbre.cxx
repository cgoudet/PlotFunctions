#include "PlotFunctions/Arbre.h"
#include "PlotFunctions/SideFunctions.h"
using std::sort;
using std::map;
#include <iostream>
using std::cout;
using std::endl;
#include "TList.h"
#include "TIterator.h"
#include "TXMLAttr.h"
#include "TXMLDocument.h"
#include "TDOMParser.h"
using std::vector;

//===================================
ChrisLib::Arbre::Arbre() {
  m_attributes["nodeName"] = "Arbre";
}
//===================================
ChrisLib::Arbre::Arbre( string nodeName, list<string> attrConstraints, list<string> childrenConstraints) {
  m_attributes["nodeName"] = nodeName;
  m_attrConstraints = attrConstraints;
  m_childrenConstraints = childrenConstraints;

  m_attrConstraints.sort();
  m_childrenConstraints.sort();
}
//===================================
int ChrisLib::Arbre::AddChild( Arbre &child ) {

  //Check if the child node name is autorized in this level
  bool isAutorized = false;
  for ( auto vList : m_childrenConstraints ) {
    if ( child.GetNodeName() == vList ) {
      isAutorized=true;
      break;
    }
    else if ( child.GetNodeName() > vList ) break;
  }
  if ( !m_childrenConstraints.size() ) isAutorized = true;
  if ( !isAutorized ) {
    cout << "Child type is not accepted." << endl;
    return 1;
  }
  //Add the child as the last of the other childs having the same name
  //Otherwise, put it according to alphabetical order of node name
  unsigned int initSize = m_children.size();
  for ( list<Arbre>::iterator it = m_children.begin(); it!=m_children.end(); ++it ) {
    if ( child.GetNodeName() > it->GetNodeName() ) {
      m_children.insert( it, child );
      break;
    }
  }
  if ( initSize ==  m_children.size() ) m_children.push_back( child );
  return 0;
}

//===================================
Arbre ChrisLib::Arbre::ParseXML( string inFileName ) {
  //  cout << "ParseXML( " << inFileName << " ) " << endl;  

  TDOMParser xmlparser;
  //Check if the xml file is ok                                                                                                                                                                      
  xmlparser.ParseFile( inFileName.c_str() );
  TXMLDocument* xmldoc = xmlparser.GetXMLDocument();
  TXMLNode * rootNode = xmldoc->GetRootNode();
  Arbre outArbre = CopyNode( rootNode );
  return outArbre;
}
//===================================
void ChrisLib::Arbre::Dump( string prefix ) {

  cout << prefix << m_attributes["nodeName"] << endl;
  for ( auto vKey : m_attributes ) {
    if ( vKey.first == "nodeName" ) continue;
    cout << prefix << "  " << vKey.first << " : " << vKey.second << endl;
  }
  for ( auto vChild : m_children ) vChild.Dump( prefix + "\t" );

}
//===================================
void ChrisLib::Arbre::SetAttribute( string key, string value ) {
  bool isAutorized = false;
  for ( auto vAttr : m_attrConstraints ) {
    if ( vAttr == key ) {
      isAutorized = true;
      m_attributes[key] = value;
      break;
    }
    else if ( vAttr > key ) break;
  }

    if ( !m_attrConstraints.size() ) isAutorized = true;
    if ( key == "nodeName" ) isAutorized = false;
    if ( !isAutorized ) { cout << key << " is not a valid attribute" << endl; return; }

    m_attributes[key] = value;
}
//===================================
Arbre ChrisLib::Arbre::CopyNode( TXMLNode * node ) {
  
  Arbre outArbre( node->GetNodeName() );
  TList *attr = node->GetAttributes();
  TIterator *it = 0;
  if(attr!=0) {
    it = attr->MakeIterator();
    for ( auto attr = (TXMLAttr*) it->Next(); attr!=0; attr=(TXMLAttr*)it->Next() ) {
      outArbre.SetAttribute( attr->GetName(),attr->GetValue() );
    }
    delete it; it=0;
  }

  const char* dum = node->GetText();
  if ( dum ) {
    string dumStr(dum);
    if ( dumStr != "" ) outArbre.SetAttribute( "text", dumStr );
    //    cout << node->GetNodeName() << " " << dumStr << endl;
  }

//cout << node->GetText() << endl;
  // map<string,string> dumMap = outArbre.GetAttributes();
  // PrintMapKeys( dumMap );

  TXMLNode *childNode = node->GetChildren();
  while ( childNode!=0 ) {
    Arbre childArbre = CopyNode( childNode );
    outArbre.AddChild( childArbre );
    childNode = childNode->GetNextNode();
  }
  return outArbre;

}
//===================================
int  ChrisLib::Arbre::GetArbresPath( Arbre &arbre, vector<Arbre> &outVect, vector<string> path, vector<map<string, string>> vectOptions ) {
  if ( vectOptions.size() && vectOptions.size() != path.size() ) { cout << "options and path do not have same size" << endl; exit(0); }
  // cout << "=====" << endl;
  // cout << "nodeName : " << path.back() << " " << arbre.GetNodeName() << endl;
  if ( string(arbre.GetNodeName()) > path.back() ) return 1;
  else if ( string(arbre.GetNodeName()) < path.back() ) return 2;

  if ( vectOptions.size() ) {
    map<string, string> mapAttr = arbre.GetAttributes();
    for ( auto vKey : vectOptions.back() ) {
      //      cout << "key : " << vKey.first << " " << vKey.second << " " << mapAttr[vKey.first] << endl;
      if ( mapAttr[vKey.first] != vKey.second ) return 2;
    }
    vectOptions.pop_back();
  }
  
  path.pop_back();
  if ( !path.size() ) {
    //    cout << "keep it! " << endl;
    outVect.push_back( arbre );
    //    arbre.Dump();
    return 0;
  }

  list<Arbre> children = arbre.GetChildren();
  for ( auto child : children ) {
    GetArbresPath( child, outVect, path, vectOptions );
  }

  return 0;
}
//===================================
