// -----
// Program: Burza
// Autor: Miro Janosik
// Vytvorene pre MC Srdiecko Trencin
// -----
#ifndef BURZATABLETABLE_H
#define BURZATABLETABLE_H

#include<string>
#include <vector>
//for exceptions on linux
#include<stdexcept>

using namespace std;

namespace BurzaTable {
	
class TableCell {
private:
	string	m_text;
	
public:
	TableCell() {
		m_text = "";
	} ;
		
	// sets text
	string& operator=( const char* text ) { m_text = text; return m_text; }
	// sets text
	string& operator=( const string& s ) { m_text = s; return m_text; }
	// sets text
	void  SetText( const string& text ) { m_text = text; }
	// gets text
	const string& GetText( void ) { return m_text; }
	// compares to text
	bool operator==( const char* text ) { return ( m_text == text ); }
	//bool operator==( const string& text ) { return ( m_text == text ); }
	bool operator!=( const char* text ) { return ( m_text != text ); }
	//bool operator!=( const string& text ) { return ( m_text != text ); }
	
	operator const string&() { return m_text; }
	//operator const char*() { return m_text.c_str(); }
	
	void Clear( void ) {
		m_text = "";
	}
	
} ;


template< typename T > class Table {
private:
	std::vector< T >	m_cells;
	// unsigned int index = m_rows * row + column;
	unsigned int m_rows, m_columns, m_cells_count;
	
public:
	Table()
	{
		m_rows = 0;
		m_columns = 0;
		m_cells_count = 0;
	}
	
	Table( unsigned int rows, unsigned int columns )
	{
		m_rows = 0;
		m_columns = 0;
		m_cells_count = 0;
		
		Resize( rows, columns );
	}
	
	unsigned int Rows( void ) { return m_rows; }
	unsigned int Columns( void ) { return m_columns; }
	
	//! changes size of the table. Cells will keep their values
	//! new cells will be empty
	void Resize( unsigned int rows, unsigned int columns )
	{
		if(( rows == m_rows ) && ( columns == m_columns ))
			return;
		
		std::vector< T >	new_cells;
		
		new_cells.resize( rows * columns );
		
		// copy what can be copied from old data
		unsigned int minr, minc;
		minr = rows;
		if ( minr > m_rows )
			minr = m_rows;
		
		minc = columns;
		if ( minc > m_columns )
			minc = m_columns;
		
		for( unsigned int r = 0; r < minr; r++ )
			for( unsigned int c = 0; c < minc; c++ ) {
				new_cells[ ( r * rows ) + c ] = m_cells[ ( r * m_rows ) + c ];
			}
		
		m_cells = new_cells;
		m_columns = columns;
		m_rows = rows;
		m_cells_count = m_columns * m_rows;
	}
	
	void EmptyCells( void )
	{
		for( unsigned int c = 0; c < m_cells_count; c++ ) {
				m_cells[ c ].Clear();
			}
	}
	
	TableCell& Cell( const unsigned int row, const unsigned int column ) 
		throw( std::out_of_range )
	{
		if (( row >= m_rows ) ||
			( column >= m_columns ))
			throw std::out_of_range( "Cell() out of range" );
		
		unsigned int i = m_rows * row + column;
		return m_cells[ i ];
	}
	
	TableCell& CellByIndex( const unsigned int index ) 
		throw( std::out_of_range )
	{
		if ( index >= m_cells_count )
			throw std::out_of_range( "CellByIndex() out of range" );
		
		return m_cells[ index ];
	}
	
} ;

typedef Table< TableCell > TableOfCells;

class TableColumnSpec {
public:
	TableColumnSpec()
	{
		m_name = "";
		m_width = 100;
		m_items_bold = false;
		m_items_italic = false;
		m_items_font_size = 14;
		m_merge_header_cell_to_right = false;		
	}
	
	string		m_name;
	unsigned int	m_width;
	
	bool		m_items_bold;
	bool		m_items_italic;
	unsigned int m_items_font_size;
	bool		m_merge_header_cell_to_right;
} ;

class TableColumnSpecs {
public:
	vector< TableColumnSpec > m_columns;
	
	unsigned int Columns( void ) { return m_columns.size(); }
	
	void AddColumn( string name, unsigned int width )
	{
		TableColumnSpec	c;
		c.m_name = name;
		c.m_width = width;
		m_columns.push_back( c );
	}
} ;


};


#endif
