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

    void Dump( std::string prefix = "") const;

    std::string GetNodeName() const { return m_attributes.at("nodeName"); }
    std::string GetText() const { return m_attributes.at("text"); }
    const std::map<std::string, std::string> &GetAttributes() const { return m_attributes; }
    const std::list<std::string> &GetAttributesConstraints() const { return m_attrConstraints; }
    const std::list<std::string> &GetChildrenConstraints() const { return m_childrenConstraints; }
    const std::list<Arbre> &GetChildren() const { return m_children; }
    std::string GetAttribute( std::string attribute ) const { return m_attributes.at(attribute); }

    bool IsAttribute( std::string attribute ) const { return (m_attributes.find(attribute)!=m_attributes.end()); }
    void SetAttribute( std::string key, std::string value );

    /**\brief Write the Arbre into a XML file
       \param outFileName 
       \param docType Name of the dtd file for structure check of the xml.
    */
    void WriteToFile( const std::string &outFileName, const std::string &docType = "" ) const;


    static Arbre ParseXML( std::string inFileName );
    static Arbre CopyNode( TXMLNode * node );

    static int  GetArbresPath( Arbre &arbre , std::vector<Arbre> &outVect, std::vector<std::string> path = std::vector<std::string>(), std::vector<std::map<std::string, std::string>> vectOptions = std::vector<std::map<std::string, std::string>>() );

  private :
  
    void WriteXML( std::ostream &stream, const std::string &prefix = "" ) const;

    std::map<std::string, std::string> m_attributes;
    std::list< Arbre > m_children;

    std::list<std::string> m_childrenConstraints;
    std::list<std::string> m_attrConstraints;
  };
}

#endif
