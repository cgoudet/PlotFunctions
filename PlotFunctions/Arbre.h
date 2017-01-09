#ifndef ARBRE_H
#define ARBRE_H

#include <list>
#include <map>
#include <string>
#include "TXMLNode.h"
#include <vector>
#include <ostream>

namespace ChrisLib {
  class Arbre 
  {

  public : 
    Arbre();
    
    /**\brief Constructor with constraints on structure.
       \param nodeName Name of the created node
       \param attrConstraints Allowed names for the attribute of the object.
       \param childrenConstraints Allowed nodeNames for the children.
    */
    Arbre( std::string nodeName, std::list<std::string> attrConstraints = std::list<std::string>(), std::list<std::string> childrenConstraints = std::list<std::string>() );

    int AddChild( Arbre &child );

    void Dump( std::string prefix = "");

    std::string GetNodeName() { return m_attributes["nodeName"]; }
    const std::map<std::string, std::string> &GetAttributes() { return m_attributes; }
    const std::list<std::string> &GetAttributesConstraints() { return m_attrConstraints; }
    const std::list<std::string> &GetChildrenConstraints()  { return m_childrenConstraints; }
    const std::list<Arbre> &GetChildren() { return m_children; }
    std::string GetAttribute( std::string attribute ) { return m_attributes.at(attribute); }

    void SetAttribute( std::string key, std::string value );

    /**\brief Write the Arbre into a XML file
       \param outFileName 
       \param docType Name of the dtd file for structure check of the xml.
    */
    void WriteToFile( const std::string &outFileName, const std::string &docType = "" );


    static Arbre ParseXML( std::string inFileName );
    static Arbre CopyNode( TXMLNode * node );

    static int  GetArbresPath( Arbre &arbre , std::vector<Arbre> &outVect, std::vector<std::string> path = std::vector<std::string>(), std::vector<std::map<std::string, std::string>> vectOptions = std::vector<std::map<std::string, std::string>>() );

  private :
  
    void WriteXML( std::ostream &stream, const std::string &prefix = "" );

    std::map<std::string, std::string> m_attributes;
    std::list< Arbre > m_children;

    std::list<std::string> m_childrenConstraints;
    std::list<std::string> m_attrConstraints;
  };
}

#endif
