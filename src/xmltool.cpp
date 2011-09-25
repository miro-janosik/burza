//
//		Tery project header file
//	Tery project is a 2d game engine
//	For more info see http://tery.mypage.sk
//	Do not use or distribute without permission please.

#include "xmltool.h"

#include "myassert.h"

#include "irrXML.h"

#include "format.h"
#include "log.h"
#include "convfile.h"
#include "convfncs.h"

XmlWrapperTool::CXmlTreeSingleNode	XmlWrapperTool::CXmlTreeSingleNode::m_null_child( "!null child!", "!null child!" );

void XmlWrapperTool::CXmlTreeSingleNode::SetValueInt( int n )
{ format fmt; fmt.format_string( "%i", n ); m_value = fmt.str(); }

void XmlWrapperTool::CXmlTreeSingleNode::SetValueFloat( float n )
{ format fmt; fmt.format_string( "%0.1f", n ); m_value = fmt.str(); }

void XmlWrapperTool::CXmlWrapperTools::ReadTreeFromIrrXml( irr::io::IrrXMLReader* reader, XmlWrapperTool::CXmlTreeSingleNode& target_root )
{
	LogS( InfoMinor, "CXmlWrapperTools::ReadTreeFromIrrXml() reading xml into '%s' root.\n",
		  target_root.GetName().c_str() );
	
	irr::io::EXML_NODE			node_type;
	const char			*node_name, *ccp, empty_str[] = "";
	std::string			text;
	int					attributes_count, i; //, j;
	// int					typei;
	CXmlTreeSingleNode		*curnode;
	irr::io::IrrXMLReader		*xml = reader;
	
	typedef	std::vector< std::string >	t_string_array;
	// t_string_array				inside;
	// t_string_array::iterator	inside_it;
	bool					invalid_object_variable = false;
	
	
	curnode = &target_root;
	
	while( xml->read() )
	{
		node_type = xml->getNodeType();
		node_name = xml->getNodeName();
		if ( node_name == NULL )
			node_name = empty_str;
		
		if ( node_type == irr::io::EXN_ELEMENT ) {
			
			curnode = curnode->AddChild( node_name, "" );
			
			// to go into other elements if empty
			if ( xml->isEmptyElement() )
				node_type = irr::io::EXN_ELEMENT_END;
			
			attributes_count = xml->getAttributeCount();
		
			for( i=0; i<attributes_count; i++ ) {
				curnode->AddAttribute( xml->getAttributeName( i ), xml->getAttributeValue( i ) );
			}
		}
		
		if ( node_type == irr::io::EXN_TEXT ) {
			
			// fill the data only if data are not empty and filled only with tabs/enters/spaces
			ccp = xml->getNodeData();
			if ( ccp != NULL ) {
				text = ccp;

				/*
				std::string::size_type pos;
				pos = text.find_first_not_of( " \t\r\n" );
				if ( pos != std::string::npos ) {
					curnode->SetValue( ccp );
				}
				*/

				curnode->SetValue( ccp );
			}
		}
		
		// move to parent nodes if end of node, make it something like breakable if
		if ( node_type == irr::io::EXN_ELEMENT_END ) {
			// inside.begin() should contain name of current node.
			/*
			if ( inside.size() == 0 ) {
				LogS( InfoError, "CXmlWrapperTools::ReadTreeFromIrrXml() Error while parsing, expecting endtag '%s' but already in root node.\n", node_name );
				break;
			}
			
			if ( * inside.begin() != node_name ) {
				LogS( InfoError, "CXmlWrapperTools::ReadTreeFromIrrXml() Error while parsing, expecting endtag '%s' but found '%s' instead.\n", node_name, inside.begin()->c_str() );
				break;
			}
			inside.erase( inside.begin() );
			*/

			if ( curnode == & target_root ) {
				LogS( InfoError, "CXmlWrapperTools::ReadTreeFromIrrXml() Error while parsing, expecting endtag '%s' but already in root node.\n", node_name );
				break;
			}
			
			if ( curnode->GetName() != node_name ) {
				LogS( InfoError, "CXmlWrapperTools::ReadTreeFromIrrXml() Error while parsing, expecting endtag '%s' but found '%s' instead.\n", node_name, curnode->GetNameCstr() );
				break;
			}

			curnode = curnode->GetParent();
			
		} ;
	} // end while
	
	LogS( InfoMinor, "CXmlWrapperTools::ReadTreeFromIrrXml() file read.\n" );
	
}


int XmlWrapperTool::CXmlWrapperTools::WriteSimpleXmlFile( XmlWrapperTool::CXmlTreeSingleNode& root, CConvFile& file )
{
	if ( ! file.IsOpened() ) {
		LogS( InfoError, "CXmlWrapperTools::WriteSimpleXmlFile() target file not opened.\n" );
		return 1;
	}

	file.Print( "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n" );

	/*
	 on every loop:
	 - get index of current item. 
	 - Get that item from current parent item.
	 - Print that item into file with its attributes
	 - if item has children then store current item index and change parent to current item, change index to 0


	*/
	
	CXmlTreeSingleNode	*walk, *curnode, *parentnode;
	// list of item indexes in recursion
	std::list< unsigned int >	position;
	unsigned int	curidx, i;
	bool			node_has_sub_children;
	// CXmlTreeSingleNode::t_children::iterator	chit, chite;
	// number of spaces in front of every line
	std::string		tablevel = "";
		
	// default parent is root. Find first sub object and set it as current index
	// and push that index into [position]
	parentnode = &root;
	
	for( i=0; i<parentnode->GetNumOfChildren(); i++ ) {

		walk = parentnode->GetChildByIndex( i );
		if ( walk->GetType() == tot_object )
			break;

	}
	if ( i >= parentnode->GetNumOfChildren() ) {
		// root node has no subchildren object items. Return.
		return 0;
	}

	// set index of first item.
	position.push_back( i );

	while( ! position.empty() ) {
		
		curidx = * position.begin();
		position.erase( position.begin() );
		curnode = parentnode->GetChildByIndex( curidx );

		// Write object tag.
		// afterwards there are these possibilities:
		// 1) move to children node - dive in
		// 2) moving to another kin node
		// 3) moving to parent node - move out
		
		// write header of object
		file.Print( "%s<%s", tablevel.c_str(), curnode->GetNameCstr() );

		// sum up all 'attributes', write them. Also see if there are any sub-children.
		node_has_sub_children = false;

		/*
		for( i=0; i<curnode->GetNumOfChildren(); i++ ) {

			walk = curnode->GetChildByIndex( i );

			if ( walk->GetType() == tot_object )
				node_has_sub_children = true;

			if ( walk->GetType() == tot_attribute ) {
				file.Print( " %s=\"%s\"", walk->GetNameCstr(), walk->GetValueCstr() );
			}
		}
		*/
		for( i=0; i<curnode->GetNumOfAttributes(); i++ ) {

			walk = curnode->GetAttributeByIndex( i );

			file.Print( " %s=\"%s\"", walk->GetNameCstr(), walk->GetValueCstr() );
		}
		node_has_sub_children = ( curnode->GetNumOfChildren() > 0 );


		// node has subnodes ? write data, end tag and dive into first child.
		// finished current node ? write data, end tag and move out.
		if ( node_has_sub_children == false ) {

			if ( curnode->GetValue().empty() ) {
				file.Print( " />\r\n" );
			} else {
				file.Print( ">" );
				// data can be quite long. Print() is length-limited, Write is not.
				file.Write( curnode->GetValueCstr() );
				file.Print( "</%s>\r\n", curnode->GetNameCstr() );
			}

		} else {

			file.Print( ">\r\n" );

			if ( ! curnode->GetValue().empty() ) {
				file.Print( walk->GetValueCstr() );
			}
		}

		// dive in to child object node - push back current node index, 
		// set parent to current node. Also increase number of spaces in [tablevel]
		if ( node_has_sub_children ) {

			position.push_front( curidx );
			parentnode = curnode;
			curidx = -1;
			tablevel += "  ";
		}

		// move to next child object of current parent.
		/*
		int numchildren = parentnode->GetNumOfChildren();
		do {

			curidx++;
			if ( curidx >= numchildren ) {
				curnode = & CXmlTreeSingleNode::m_null_child;
				break;
			}
			curnode = parentnode->GetChildByIndex( curidx );

		} while ( curnode->GetType() != tot_object );
		*/
		curidx++;
		curnode = parentnode->GetChildByIndex( curidx );

		// no more children - move out of current parent to upper level.
		// write end tag, decrease number of [tablevel] spaces and
		// in parent node move to next object
		while ( curnode == & CXmlTreeSingleNode::m_null_child ) {

			// if I iterate out of the whole tree, after all of children of root node
			// have been processed that means I'm at the end of tree.
			if ( parentnode == &root )
				break;

			assert( tablevel.size() >= 2 );
			tablevel.erase( tablevel.begin(), tablevel.begin() + 2 );
			file.Print( "%s</%s>\r\n", tablevel.c_str(), parentnode->GetNameCstr() );

			parentnode = parentnode->GetParent();

			// move to next child object
			curidx = * position.begin();
			position.erase( position.begin() );
			/*
			int numchildren = parentnode->GetNumOfChildren();
			do {

				curidx++;
				if ( curidx >= numchildren ) {
					curnode == & CXmlTreeSingleNode::m_null_child;
					break;
				}
				curnode = parentnode->GetChildByIndex( curidx );

			} while ( curnode->GetType() != tot_object );
			*/
			curidx++;
			curnode = parentnode->GetChildByIndex( curidx );
			
		}

		if (( parentnode == &root ) && ( curnode == & CXmlTreeSingleNode::m_null_child )) {
			break;
		}

		// more children - I should move to next child in next step, keep parent.
		position.push_front( curidx );
		
	} ;
	
	return 0;
}

bool XmlWrapperTool::CXmlWrapperTools::DoTreeNodeChildrenHaveChildren( XmlWrapperTool::CXmlTreeSingleNode& node )
{
	CXmlTreeSingleNode::t_children::iterator	it, ite;
	
	ite = node.m_children.end();
	for( it=node.m_children.begin(); it!=ite; it++ )
		if ( (*it).GetNumOfChildren() != 0 )
			return true;
	
	return false;
}

XmlWrapperTool::CXmlTreeSingleNode* XmlWrapperTool::CXmlWrapperTools::FindSubChild( 
		const XmlWrapperTool::CXmlTreeSingleNode& root, const char* name, const XmlWrapperTool::CXmlTreeSingleNode* last_found )
{
	assert( false );
	/*
	CXmlTreeSingleNode::t_children::iterator	it, ite;
	CXmlTreeSingleNode		*res, *ch;
	int					idx;
	
	if ( last_found == NULL )
	ch = &root;
	else
	ch = last_found;
	
	while( 1 ) {
		
		// browse all children, look through their names.
		
	ch = 
	
	ite = node.m_children.end();
	for( it=node.m_children.begin(); it!=ite; it++ ) {
		
	res = root.NameEqualNoCase( name );
		
	if ( res ) {
	if ( last_found == NULL )
	return res;
	else
	if ( last_found == res ) {
	last_found = NULL;
	return res;
}
				
}
		
	res = FindSubChild( *it, name, last_found );
	if ( res )
	return res;
	*/
	return NULL;
}

int XmlWrapperTool::CXmlWrapperTools::ReadTreeFromDiskFile( const char* filename, XmlWrapperTool::CXmlTreeSingleNode& target_root )
{
	/*
	nefunguje.
	bool res;
	ConvFuncs::SingletonInstanceCreate();
	res = ConvFuncsInstance->FindFirstFile( filename );
	ConvFuncs::SingletonInstanceDelete();
	
	if ( res == false ) {
		LogS( InfoError, "CXmlWrapperTools::ReadTreeFromDiskFile() '%s' : not found.\n", filename );
		return 1;
	}
	*/
	
	irr::io::IrrXMLReader* xml = irr::io::createIrrXMLReader( filename );
	if ( xml == NULL ) {
		LogS( InfoError, "CXmlWrapperTools::ReadTreeFromDiskFile() error.\n" );
		return 1;
	}
	
	ReadTreeFromIrrXml( xml, target_root );
	
	delete xml;
	return 0;
}

int XmlWrapperTool::CXmlWrapperTools::ReadTreeFromDiskFile( CConvFile& file, XmlWrapperTool::CXmlTreeSingleNode& target_root )
{
	IrrXmlReaderConvFile	cfreader;

	cfreader.m_pfile = &file;

	irr::io::IrrXMLReader* xml = irr::io::createIrrXMLReader( &cfreader );
	if ( xml == NULL ) {
		LogS( InfoError, "CXmlWrapperTools::ReadTreeFromDiskFile() error.\n" );
		return 1;
	}
	
	ReadTreeFromIrrXml( xml, target_root );
	
	delete xml;
	return 0;
}


int XmlWrapperTool::IrrXmlReaderConvFile::read(void* buffer, int sizeToRead)
{
	return m_pfile->Read( sizeToRead, buffer );
}

int XmlWrapperTool::IrrXmlReaderConvFile::getSize()
{
	return m_pfile->FileSize();
}
