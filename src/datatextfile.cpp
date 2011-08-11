#include <stdexcept>
#include <string.h>
#include <algorithm>
#include "datatextfile.h"
#include "translate.h"
#include "burza.h"

using namespace Burza;
using namespace std;

char DataTextfile::file_field_delimiters[] = "|*\r\n";

DataTextfile::DataTextfile( string datapath )
{
	DataPath = datapath;
	
	ItemCategory	cat;
	
	cat.id = 1;
	cat.letter = 'a';
	cat.fee_per_item = 1.0;
	m_ItemCategories.push_back(cat);

	cat.id = 2;
	cat.letter = 'b';
	cat.fee_per_item = 4.0;
	m_ItemCategories.push_back(cat);
}	

void DataTextfile::SaveSellerToTxtFile( SellerData& seller, ItemDataArray& items )
{
	string oldname = GetSellerFilenameExisting(seller.id);
	string newname = GetSellerFilenameNew(seller);

	// compare new filename with previous
	// if different, erase previous file.
	if (( oldname != "" ) && ( oldname != newname )) {
		ConvFuncs::DoDeleteFile( oldname.c_str() );
	}
	
	CConvFile	file;
	file.OpenFile( newname.c_str(), CConvFile::fm_text, CConvFile::fm_write );
	if ( ! file.OpenedWriteable() ) {
		
		string s1 = _( "Could not open output file '%s'" );
		string s2 = ConvFuncs::FormatStdStr( s1.c_str(), newname.c_str() );
		
		throw runtime_error( s2 );
	}
	
	ReplaceCharactersInString( seller.name, file_field_delimiters, '_' );
	ReplaceCharactersInString( seller.note, file_field_delimiters, '_' );
	
	file.Print( "%i", seller.id );
	file.Write( "*" );
	file.Write( seller.shortcode.c_str() );
	file.Write( "*" );
	file.Write( seller.name.c_str() );
	file.Write( "*" );
	file.Write( seller.note.c_str() );
	file.Write( "\n" );
	
	string s;
	
	for( unsigned int i=0; i<items.size(); i++ ) {
		
		ReplaceCharactersInString( items[ i ].name, file_field_delimiters, '_' );
		
		file.Print( "%i*%s*%0.1f",
			items[ i ].number, items[ i ].name.c_str(), items[ i ].price );
		file.Print( "*%c*%c\n",
			items[ i ].category,
			ItemStatus2Char( items[ i ].status ) );
	}
	
	file.Close();
}

SELLER_ID DataTextfile::LoadFromTxtFile( string filename, SellerData& seller, ItemDataArray& items )
{
	items.clear();
	
	CConvFile	file;
	file.OpenFile( filename.c_str(), CConvFile::fm_text, CConvFile::fm_read );
	
	if ( file.IsOpened() == 0 ) {
		throw runtime_error("DataTextfile::LoadFromTxtFile() File not opened.");
	}

	// items cannot have fixed length, because source files are unicode (that means variable char length, either 1 or 2 chars)
	static const int linebuffer_len = 512;
	char linebuffer[ linebuffer_len ];
	char namebuffer[ linebuffer_len ];
	unsigned short strpos;
	ItemData	item;
	
	const char* cp, *linep;
	
	file.gets( linebuffer, linebuffer_len );
	seller.id = atoi( ConvFuncsInstance->StrItem( linebuffer, 0, file_field_delimiters ) );
	seller.shortcode = ConvFuncsInstance->StrItem( linebuffer, 1, file_field_delimiters );
	seller.name = ConvFuncsInstance->StrItem( linebuffer, 2, file_field_delimiters );
	seller.note = ConvFuncsInstance->StrItem( linebuffer, 3, file_field_delimiters );
	
	while( 1 ) {
		if ( file.Eof() )
			break;
		if ( file.gets( linebuffer, linebuffer_len ) == 0 )
			break;
		
		linep = linebuffer;
		cp = ConvFuncsInstance->StrItem( linebuffer, 0, file_field_delimiters, &strpos );
		// skip empty lines
		if ( cp == NULL )
			continue;
		item.number = atoi( cp );
		
		// item name
		linep += strpos;
		cp = ConvFuncsInstance->StrItem( linep, 0, file_field_delimiters, &strpos );
		item.name = cp;
		
		linep += strpos;
		
		item.price = atof( ConvFuncsInstance->StrItem( linep, 0, file_field_delimiters ) );
		item.category = * ConvFuncsInstance->StrItem( linep, 1, file_field_delimiters );
		item.status = Char2ItemStatus( ( * ConvFuncsInstance->StrItem( linep, 2, file_field_delimiters ) ) );
		
		item.last_update = GetCurrentTime();
		item.seller_id = GetNewSellerId();
		items.push_back(item);
	} ;
	
	file.Close();
	
	return seller.id;
}

void DataTextfile::LoadFromTxtFile( SELLER_ID id, SellerData& seller, ItemDataArray& items )
{
	std::string		filename;
			
	filename = DataPath;
	filename += GetSellerFilenameExisting(id);
	
	try {
	  LoadFromTxtFile(filename, seller, items);
	}
	catch(...)
	{
		throw;
	}
}

SellerDataArray DataTextfile::GetSellers( void )
{
	std::string		predajca_pat, filename;
	SellerData		seller;
	ItemDataArray	seller_items;
	SELLER_ID		id;

	sellers.clear();
	
	predajca_pat = DataPath;
	predajca_pat += "*.txt";
	
	bool found;
	found = ConvFuncsInstance->Find_FirstFile( predajca_pat.c_str() );
	while( found ) {
		found = ConvFuncsInstance->Find_NextFile();
		
		filename = DataPath;
		filename += ConvFuncsInstance->Find_CurrentFileName();
		
		try {
		  items.clear();
		  
		  id = LoadFromTxtFile( filename, seller, seller_items );
		}
		catch(...)
		{
			continue;
		}
		
		sellers.push_back(seller);
	}
	
	return sellers;
}

SellerData& DataTextfile::GetSeller(SELLER_ID id)
{
	if (sellers.empty()) {
		GetSellers();
	}
	
	for( int i=0; i < sellers.size(); ++i )
		if ( sellers[ i ].id == id )
			return sellers[ i ];
	
	throw WrongSellerException();
}

ItemDataArray DataTextfile::GetSellerItems(SELLER_ID id)
{
	SellerData	seller;
	ItemDataArray	items;
	
	LoadFromTxtFile( id, seller, items );
	
	return items;
}

ItemData DataTextfile::GetSellerItem(SELLER_ID id, int item_number)
{
	SellerData	seller;
	ItemDataArray	items;
	
	LoadFromTxtFile( id, seller, items );
	
	ItemData* item = FindItemByNumber( items, item_number );
	if ( item == NULL ) {
		throw new WrongItemException();
	}
	
	return *item;
}

ItemData DataTextfile::GetItem(ITEM_ID id)
{
	throw NotImplementedException();
}

string DataTextfile::GetSellerFilenameExisting( SELLER_ID id )
{
	string predajca_pat;
	
	// find previous filename
	predajca_pat = DataPath;
	predajca_pat += ConvFuncs::FormatStdStr( "%i-*.txt", id );

	bool previous_found;
	previous_found = ConvFuncsInstance->Find_FirstFile( predajca_pat.c_str() );
	if ( previous_found ) {
		return ConvFuncsInstance->Find_CurrentFileName();
	}
	
	throw WrongSellerException();
}

string DataTextfile::GetSellerFilenameNew( SellerData& seller )
{
	// File name is specified as "<seller id>-<short code>.txt".
	// Only seller id is unique, as short code could be changed.
	// so I have to look for file starting with seller id.
	
	string filename = DataPath;
	filename += ConvFuncs::FormatStdStr( "%i-%s-%s.txt", 
		seller.id, seller.shortcode.c_str(), seller.name.c_str() );
	
	return filename;
}

SELLER_ID DataTextfile::AddSeller(SellerData& seller)
{
	ItemDataArray items;
	
	seller.id = GetNewSellerId();
	SaveSellerToTxtFile( seller, items );
	
	return seller.id;
}

bool DataTextfile::ModifySellerStart(SELLER_ID id)
{
}
 
void DataTextfile::ModifySellerEnd(SELLER_ID id, SellerData& seller)
{
	ItemDataArray items = GetSellerItems(id);
	SaveSellerToTxtFile( seller, items );
}

void DataTextfile::RemoveSeller(SELLER_ID id)
{
	string filename = GetSellerFilenameExisting(id);

	// compare new filename with previous
	// if different, erase previous file.
	if ( filename != "" ) {
		string complete = DataPath;
		complete += filename;
		ConvFuncs::DoDeleteFile( complete.c_str() );
	}
	
	for( int i=0; i < sellers.size(); ++i ) {
		if ( sellers[ i ].id == id ) {
			sellers.begin() + i;
			break;
		}
	}
}

ITEM_ID DataTextfile::AddItem(ItemData& item, SELLER_ID sellerId)
{
	SellerData seller = GetSeller(sellerId);
	ItemDataArray items = GetSellerItems(sellerId);
	
	item.id = GetNewItemId();
	items.push_back(item);
	SaveSellerToTxtFile( seller, items );
}

void DataTextfile::ModifyItem(ITEM_ID item_id, ItemData& item)
{
	SellerData seller = GetSeller(item.seller_id);
	ItemDataArray items = GetSellerItems(item.seller_id);

	int idx = GetItemIndex( item_id );
	if ( idx == -1 ) {
		throw new WrongItemException();
	}
	
	items[ idx ] = item;
	
	SaveSellerToTxtFile(seller, items);
}

void DataTextfile::RemoveItem(ITEM_ID id)
{
	SELLER_ID seller_id = GetItem( id ).seller_id;
	int idx = GetItemIndex( id );
	if ( idx == -1 ) {
		throw new WrongItemException();
	}
	
	items.erase( items.begin() + idx );
	
	SellerData seller = GetSeller(seller_id);
	ItemDataArray items = GetSellerItems(seller_id);
	SaveSellerToTxtFile(seller, items);
}

bool DataTextfile::IsSellerModified(SELLER_ID id)
{
	return false;
}

SellerData* DataTextfile::GetSellerIfModified(SELLER_ID id)
{
	return NULL;
}

bool DataTextfile::IsItemModified(ITEM_ID id)
{
	return false;
}

ItemData* DataTextfile::GetItemIfModified(ITEM_ID id)
{
	return NULL;
}

SellerData& DataTextfile::GetSellerByCode(string& code)
{
	if (sellers.empty()) {
		GetSellers();
	}
	
	SellerDataIterator it = sellers.begin(), ite = sellers.end();
	for( ; it != ite; ++it ) {
		if (it->shortcode == code) {
			return *it;
		}
	}
	
	throw WrongSellerException();
}

SELLER_ID DataTextfile::GetNewSellerId( void )
{
	if (sellers.empty()) {
		GetSellers();
	}
	
	SELLER_ID biggestId = 0;
	
	SellerDataIterator it = sellers.begin(), ite = sellers.end();
	for( ; it != ite; ++it ) {
		if (it->id > biggestId) {
			biggestId = it->id;
		}
	}
	
	return biggestId + 1;
}

ITEM_ID DataTextfile::GetNewItemId( void )
{
	if (sellers.empty()) {
		GetSellers();
	}
	
	ITEM_ID biggestId = 0;
	
	SellerDataIterator it = sellers.begin(), ite = sellers.end();
	for( ; it != ite; ++it ) {

		ItemDataArray items = GetSellerItems(it->id);
		ItemDataIterator itemit = items.begin(), itemite = items.end();
	  
		for( ; itemit != itemite; ++itemit ) {
			if (itemit->id > biggestId) {
				biggestId = itemit->id;
			}
		}
	}
	
	return biggestId + 1;
}

ItemCategoryArray DataTextfile::GetItemsCategories( ItemDataArray& items )
{
	ItemCategory	cat;
	ItemCategoryArray	res;
	
	for( unsigned int i=0; i<items.size(); ++i ) {
		
		cat = GetItemCategory( items[ i ].category );
		res.push_back(cat);
	}
	
	return res;
}

ItemCategory DataTextfile::GetItemCategory(ITEM_CATEGORY_ID id)
{
	for( int i=0; i < m_ItemCategories.size(); ++i ) {
		if ( m_ItemCategories[ i ].id == id ) {
			return m_ItemCategories[ i ];
		}
	}
	
	throw WrongItemCategoryException();
}

void DataTextfile::ReplaceCharactersInString( std::string& str, const char* from, char to )
{
	unsigned int len = strlen( from );
	for( unsigned int i=0; i<len; i++ ) {
		std::replace( str.begin(), str.end(), from[ i ], to );
	}
}

// ItemCategory DataTextfile::GetItemCategoryByLetter(char letter)
// ItemCategoryArray DataTextfile::GetItemsCategories(vector<ITEM_CATEGORY_ID>& ids);
// ItemCategoryArray DataTextfile::GetItemsCategoriesByLetter(vector<char>& letters);

SellerCategory DataTextfile::GetSellerCategory(SELLER_CATEGORY_ID id)
{
	for( int i=0; i < m_SellerCategories.size(); ++i ) {
		if ( m_SellerCategories[ i ].id == id ) {
			return m_SellerCategories[ i ];
		}
	}
	
	throw WrongSellerCategoryException();
}

// SellerCategory DataTextfile::GetSellerCategoryByLetter(char letter);
// SellerCategoryArray DataTextfile::GetSellersCategories(vector<SELLER_CATEGORY_ID>& ids);
//SellerCategoryArray DataTextfile::GetSellersCategoriesByLetter(vector<char>& letters);

int DataTextfile::GetItemIndex( ITEM_ID id )
{
	for( int i=0; i<items.size(); ++i )
		if ( items[ i ].id == id )
			return i;
	
	return -1;
}

int DataTextfile::GetSellerIndex( SELLER_ID id )
{
	for( int i=0; i<sellers.size(); ++i )
		if ( sellers[ i ].id == id )
			return i;
	
	return -1;
}
