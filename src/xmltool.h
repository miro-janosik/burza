//
//		Tery project header file
//	Tery project is a 2d game engine
//	For more info see http://tery.mypage.sk
//	Do not use or distribute without permission please.

#ifndef _xmltool_h_
#define _xmltool_h_

#include <string>
#include <list>
#include <vector>

#include "irrXML.h"
#include "convfile.h"

#ifdef _UNIX
	#ifndef stricmp
		#define stricmp(a, b) strcasecmp((a),(b))
	#endif
#endif

/*
namespace irr {
	namespace io {
		template class IrrXMLReader;
	}
}
*/

namespace XmlWrapperTool {

enum e_tree_node_type {
	tot_object,		// can have children
	tot_attribute,	// can not have children
} ;

class CXmlTreeSingleNode;
class CXmlWrapperTools;

//! Class used as data storage in tree form. Object and its children are forming a tree full of data
//! Class/Tree data are also used as wrapper for XML reader/writer data.
//! Currently we're using irrXML.
//! See also helper class CXmlWrapperTools with functions for handling and converting the tree data.
class CXmlTreeSingleNode {
friend class CXmlWrapperTools;
public:
	CXmlTreeSingleNode() { Reset(); }
	CXmlTreeSingleNode( const char* name, const char* value ) { Reset(); SetName( name ); SetValue( value ); }

	e_tree_node_type GetType( void ) 
	{ return m_type; }
	
	//! -- name --
	void SetName( const char* n )
	{ m_name = n; }
	std::string GetName( void )
	{ return m_name; }
	const char* GetNameCstr( void )
	{ return m_name.c_str(); }
	bool NameEqual( const char* n )
	{ return ( m_name == n ); }
	bool NameEqualNoCase( const char* n )
	{ return ( stricmp( m_name.c_str(), n ) == 0 ); }
	
	//! value
	void SetValue( const char* n )
	{ m_value = n; }
	void SetValueInt( int n );
	void SetValueFloat( float n );
	
	const std::string& GetValue( void )
	{ return m_value; }
	const char* GetValueCstr( void )
	{ return m_value.c_str(); }
	bool ValueEqual( const char* n )
	{ return ( m_value == n ); }
	bool ValueEqualNoCase( const char* n )
	{ return ( stricmp( m_value.c_str(), n ) == 0 ); }
	int GetValueInt( void )
	{ return atoi( m_value.c_str() ); }
	float GetValueFloat( void )
	{ return (float) atof( m_value.c_str() ); }
	
	//! -- children --
	//! returns pointer to new child, never returns NULL
	//! pointer may be invalidated after another AddChild or other modification of children.
	CXmlTreeSingleNode* AddChild( const char* name, const char* value = "" )
	{ 
		if ( this == & CXmlTreeSingleNode::m_null_child ) return & CXmlTreeSingleNode::m_null_child;
		CXmlTreeSingleNode o; o.m_type = tot_object; o.m_name = name; o.m_value = value; 
		o.m_parent = this; m_children.push_back( o ); return &( m_children.back() ); 
	}

	CXmlTreeSingleNode* AddAttribute( const char* name, const char* value = "" ) 
	{ 
		if ( this == & CXmlTreeSingleNode::m_null_child ) return & CXmlTreeSingleNode::m_null_child;
		CXmlTreeSingleNode o; o.m_type = tot_attribute; o.m_name = name; o.m_value = value; 
		o.m_parent = this; m_children_attributes.push_back( o ); return &( m_children_attributes.back() ); 
	}
	
	//! Copies source node with its children into current node.
	void AddChildCopyNode( CXmlTreeSingleNode& node );
	
	//! Moves source node with its children from its parent node into current node. It is removed from its previous parent node.
	void AddChildMoveNode( CXmlTreeSingleNode& node );
	
	inline unsigned int GetNumOfChildren( void ) const
		{ return m_children.size(); }
	
	inline unsigned int GetNumOfAttributes( void ) const
		{ return m_children_attributes.size(); }
	
	void ClearChildren( void )
		{ m_children.clear(); }
	
	void ClearAttributes( void )
		{ m_children_attributes.clear(); }
	
	//! returns pointer to [m_null_child] if index is out of bounds
	CXmlTreeSingleNode* GetChildByIndex( unsigned int idx )
	{
		if ( m_children.size() <= idx )
			return & CXmlTreeSingleNode::m_null_child;
		
		for( it=m_children.begin(); idx > 0; idx-- )
			it++;
		
		return & ( *it );
	}

	//! compares name case insensitive
	//! idx: searches for n-th occurence of node with this name. idx = 0 means first occurence, idx = 1 means second 
	//! returns pointer to [m_null_child] if no such child exists
	CXmlTreeSingleNode* GetChild( const char* name, unsigned int idx = 0 )
	{ 	for( it=m_children.begin(); it!=m_children.end(); it++ )
			if ( it->NameEqualNoCase( name ) ) {
				if ( idx == 0 )
					return &(*it);
				idx--;
			}

		return & CXmlTreeSingleNode::m_null_child;
	}
	
	//! compares name case sensitive
	//! returns pointer to [m_null_child] if no such child exists
	CXmlTreeSingleNode* GetChildCaseSensitive( const char* name, unsigned int idx = 0 )
	{ 	for( it=m_children.begin(); it!=m_children.end(); it++ )
			if ( it->NameEqual( name ) ) {
				if ( idx == 0 )
					return &(*it);
				idx--;
			}

		return & CXmlTreeSingleNode::m_null_child;
	}
	
	//! looks for child of specific name and returns pointer to it. If it does not exist it is created.
	//! compares name case insensitive
	//! returns pointer to [m_null_child] if called on [m_null_child]
	CXmlTreeSingleNode* GetChildOrAdd( const char* name )
	{
		for( it=m_children.begin(); it!=m_children.end(); it++ )
			if ( it->NameEqualNoCase( name ) )
				return &(*it);
		
		return AddChild( name );
	}

	//! returns pointer to [m_null_child] if index is out of bounds
	CXmlTreeSingleNode* GetAttributeByIndex( unsigned int idx )
	{
		if ( m_children_attributes.size() <= idx )
			return & CXmlTreeSingleNode::m_null_child;
		
		for( it=m_children_attributes.begin(); idx > 0; idx-- )
			it++;
		
		return & ( *it );
	}

	//! compares name case insensitive
	//! idx: searches for n-th occurence of node with this name. idx = 0 means first occurence, idx = 1 means second 
	//! returns pointer to [m_null_child] if no such child exists
	CXmlTreeSingleNode* GetAttribute( const char* name, unsigned int idx = 0 )
	{ 	for( it=m_children_attributes.begin(); it!=m_children_attributes.end(); it++ )
			if ( it->NameEqualNoCase( name ) ) {
				if ( idx == 0 )
					return &(*it);
				idx--;
			}

		return & CXmlTreeSingleNode::m_null_child;
	}
	
	//! compares name case sensitive
	//! returns pointer to [m_null_child] if no such child exists
	CXmlTreeSingleNode* GetAttributeCaseSensitive( const char* name, unsigned int idx = 0 )
	{ 	for( it=m_children_attributes.begin(); it!=m_children_attributes.end(); it++ )
			if ( it->NameEqual( name ) ) {
				if ( idx == 0 )
					return &(*it);
				idx--;
			}

		return & CXmlTreeSingleNode::m_null_child;
	}
	
	// looks for a child with specific name, and with specific attribute containing specific value
	//! returns pointer to [m_null_child] if no such child exists
	CXmlTreeSingleNode* GetChildWithAttributeValue( const char* child_name, 
		const char* attribute_name, const char* attribute_value )
	{ 	for( it=m_children.begin(); it!=m_children.end(); it++ )
			if ( it->NameEqualNoCase( child_name ) ) {
				if ( it->GetAttribute( attribute_name )->ValueEqualNoCase( attribute_value ) )
					return &(*it);
			}

		return & CXmlTreeSingleNode::m_null_child;
	}

	//! may return [m_null_child] if node has no parent
	CXmlTreeSingleNode* GetParent( void )
		{ return m_parent; }
	
	//! null result
	//! variable used as failure result of some functions which are looking for node
	//! child has name and value "!null child!", it has no parent and no children.
	static CXmlTreeSingleNode	m_null_child;
	
	// returns pointer to null result.
	static CXmlTreeSingleNode* Null( void ) { return & CXmlTreeSingleNode::m_null_child; }

	// returns true if node is not equal to null result.
	// that means, if you call node = node.GetChild( "a" ); then if call was successfull node.IsValid() will return true.
	bool IsValid( void ) { return ( this != Null() ); }

	// === function overloads for use with std::string parameters instead of const char* ===

	CXmlTreeSingleNode( const std::string& name, const std::string& value ) { CXmlTreeSingleNode( name.c_str(), value.c_str() ); }
	void SetName( const std::string& n ) { SetName( n.c_str() ); }
	bool NameEqual( const std::string& n ) { return NameEqual( n.c_str() ); }
	bool NameEqualNoCase( const std::string& n ) { return NameEqualNoCase( n.c_str() ); }
	void SetValue( const std::string& n ) { SetValue( n.c_str() ); }
	bool ValueEqual( const std::string& n ) { return ValueEqual( n.c_str() ); }
	bool ValueEqualNoCase( const std::string& n ) { return ValueEqualNoCase( n.c_str() ); }
	CXmlTreeSingleNode* AddChild( const std::string& name, const std::string& value = "" ) { return AddChild( name.c_str(), value.c_str() ); }
	CXmlTreeSingleNode* AddAttribute( const std::string& name, const std::string& value = "" ) { return AddAttribute( name.c_str(), value.c_str() ); }
	CXmlTreeSingleNode* GetChild( const std::string& name, unsigned int idx = 0 ) { return GetChild( name.c_str(), idx ); }
	CXmlTreeSingleNode* GetChildCaseSensitive( const std::string& name, unsigned int idx = 0 ) { return GetChildCaseSensitive( name.c_str(), idx ); }
	CXmlTreeSingleNode* GetAttribute( const std::string& name, unsigned int idx = 0 ) { return GetAttribute( name.c_str(), idx ); }
	CXmlTreeSingleNode* GetAttributeCaseSensitive( const std::string& name, unsigned int idx = 0 ) { return GetAttributeCaseSensitive( name.c_str(), idx ); }
	CXmlTreeSingleNode* GetChildWithAttributeValue( const std::string& child_name, 
		const std::string& attribute_name, const std::string& attribute_value ) { return GetChildWithAttributeValue( child_name.c_str(), attribute_name.c_str(), attribute_value.c_str() ); }

private: 
	e_tree_node_type	m_type;
	std::string			m_name;
	std::string			m_value;
	
	typedef std::list< CXmlTreeSingleNode > t_children;
	t_children			m_children;
	t_children			m_children_attributes;
	t_children::iterator	it;
	
	//! null if there is no parent
	CXmlTreeSingleNode	*m_parent;
	
	void Reset( void )
		{ m_type = tot_object; m_name = ""; m_value = ""; m_children.clear(); m_children_attributes.clear(); m_parent = &CXmlTreeSingleNode::m_null_child; }
	
	//! -- type --
	void SetType( e_tree_node_type t ) 
	{ m_type = t; }
	
} ; // CXmlTreeSingleNode


class IrrXmlReaderConvFile : public irr::io::IFileReadCallBack
{
public:
	//! Reads an amount of bytes from the file.
	/** \param buffer: Pointer to buffer where to read bytes will be written to.
	\param sizeToRead: Amount of bytes to read from the file.
	\return Returns how much bytes were read. */
	int read(void* buffer, int sizeToRead);

	//! Returns size of file in bytes
	int getSize();

	CConvFile	*m_pfile;
};



//! see comment for CXmlTreeSingleNode
class CXmlWrapperTools {
public:
	
	//! reads data from file through IrrXML reader
	//! returns: 0 if successfull, 1 on error
	static int ReadTreeFromDiskFile( const char* filename, CXmlTreeSingleNode& target_root );
	static int ReadTreeFromDiskFile( CConvFile& file, CXmlTreeSingleNode& target_root );
	
	//! reads data from source irr reader into target root object
	static void ReadTreeFromIrrXml( irr::io::IrrXMLReader* reader, CXmlTreeSingleNode& target_root );
	
	//! writes xml data into target text file.
	static int WriteSimpleXmlFile( CXmlTreeSingleNode& root, CConvFile& file );
	
	//! returns true if any child of parameter node has at least one child
	static bool DoTreeNodeChildrenHaveChildren( CXmlTreeSingleNode& node );
	
	//! looks for child or subchild of a node with specific name. If [last_found] 
	//! is supplied, then its looking for subchild AFTER this last-found item
	static CXmlTreeSingleNode* FindSubChild( const CXmlTreeSingleNode& root, 
		const char* name, const CXmlTreeSingleNode* last_found=NULL );
	
} ;

//! base for wrapper class, providing no functionality
class CXmlWrapperBase {
public:
	
	//! reads data from file
	virtual int ReadTreeFromDiskFile( const char* filename, CXmlTreeSingleNode& target_root ) { return 1; }
	virtual int ReadTreeFromDiskFile( CConvFile& file, CXmlTreeSingleNode& target_root ) { return 1; }
	
	//! writes xml data into target text file.
	virtual int WriteTreeToDiskFile( const char* filename, CXmlTreeSingleNode& root ) { return 1; }
	virtual int WriteTreeToDiskFile( CConvFile& file, CXmlTreeSingleNode& root ) { return 1; }
	
} ;

//! IrrXml wrapper, providing only reading functionality
class CXmlWrapperIrrXml {
public:
	
	//! reads data from file
	virtual int ReadTreeFromDiskFile( const char* filename, CXmlTreeSingleNode& target_root ) { return 1; }
	virtual int ReadTreeFromDiskFile( CConvFile& file, CXmlTreeSingleNode& target_root ) { return 1; }
	
	//! writes xml data into target text file.
	virtual int WriteTreeToDiskFile( const char* filename, CXmlTreeSingleNode& root ) { return 1; }
	virtual int WriteTreeToDiskFile( CConvFile& file, CXmlTreeSingleNode& root ) { return 1; }
	
} ;


//! simple wrapper, providing only simple writing xml functionality
class CXmlWrapperSimple {
public:
	
	//! reads data from file
	virtual int ReadTreeFromDiskFile( const char* filename, CXmlTreeSingleNode& target_root ) { return 1; }
	virtual int ReadTreeFromDiskFile( CConvFile& file, CXmlTreeSingleNode& target_root ) { return 1; }
	
	//! writes xml data into target text file.
	virtual int WriteTreeToDiskFile( const char* filename, CXmlTreeSingleNode& root ) { return 1; }
	virtual int WriteTreeToDiskFile( CConvFile& file, CXmlTreeSingleNode& root ) { return 1; }
	
} ;

} ; // XmlWrapperTool


#endif

