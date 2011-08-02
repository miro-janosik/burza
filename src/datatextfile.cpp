#include "datatextfile.h"

using namespace Burza;
using namespace std;

void DataTextfile::SaveSellerToTxtFile( SellerData& seller, ItemDataArray& items )
{
	string oldname = GetSellerFilenameExisting(seller.unique_id);
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
	
	file.Print( "%i", seller.unique_id );
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
			Centrala::Instancia()->Register()->Stav2Char( items[ i ].status ) );
	}
	
	file.Close();
}

SELLER_ID Seller::LoadFromTxtFile( string filename, SellerData& seller, ItemDataArray& items )
{
	items.clear();
	
	CConvFile	file;
	file.OpenFile( filename.c_str(), CConvFile::fm_text, CConvFile::fm_read );
	
	if ( file.IsOpened() == 0 ) {
		throw runtime_error;
	}

	// items cannot have fixed length, because source files are unicode (that means variable char length, either 1 or 2 chars)
	static const int linebuffer_len = 512;
	char linebuffer[ linebuffer_len ];
	char namebuffer[ linebuffer_len ];
	unsigned short strpos;
	ItemData	item;
	
	const char* cp, *linep;
	
	file.gets( linebuffer, linebuffer_len );
	seller.unique_id = atoi( ConvFuncsInstance->StrItem( linebuffer, 0, file_field_delimiters ) );
	seller.shortcode = ConvFuncsInstance->StrItem( linebuffer, 1, file_field_delimiters );
	seller.name = ConvFuncsInstance->StrItem( linebuffer, 2, file_field_delimiters );
	seller.note = ConvFuncsInstance->StrItem( linebuffer, 3, file_field_delimiters );
	
	seller.category = 0;
	seller.last_update = 0;
	
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
		item.status = Centrala::Instancia()->Register()->Char2Stav( ( * ConvFuncsInstance->StrItem( linep, 2, file_field_delimiters ) ) );
		
		item.last_update = 0;
		item.seller_id = id;
		items.push_back(item);
	} ;
	
	file.Close();
	
	return seller.unique_id;
}

void DatatTextfile::LoadFromTxtFile( SELLER_ID id, SellerData& seller, ItemDataArray& items )
{
	std::string		filename;
	SellerData		seller;
	ItemDataArray	items;
			
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
	
	SellerDataIterator it = sellers.find( id );
	if ( it == sellers.end() ) {
		
		throw WrongSellerIdException();
	}

	return *it;
}

ItemDataArray DataTextfile::GetSellerItems(SELLER_ID id)
{
	SellerData	seller;
	ItemDataArray	items;
	
	LoadFromTxtFile( id, seller, items );
	
	return items;
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
	
	throw WrongSellerIdException;
}

string DataTextfile::GetSellerFilenameNew( SellerData& seller )
{
	// File name is specified as "<seller id>-<short code>.txt".
	// Only seller id is unique, as short code could be changed.
	// so I have to look for file starting with seller id.
	
	filename = DataPath;
	filename += ConvFuncs::FormatStdStr( "%i-%s-%s.txt", 
		seller.unique_id, seller.shortcode.c_str(), seller.name.c_str() );
	
	return filename;
}

SELLER_ID DataTextfile::AddSeller(SellerData& seller)
{
	ItemDataArray items;
	
	seller.unique_id = GenerateUniqueSellerId();
	SaveSellerToTxtFile( seller, items );
	
	return seller.unique_id;
}

bool DataTextfile::ModifySellerStart(SELLER_ID id)
{
}
 
void DataTextfile::ModifySellerEnd(SELLER_ID id, SellerData& seller)
{
	ItemsDataArray items = GetSellerItems(id);
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
	
	SellerDataIterator it = sellers.find( id );
	if ( it != sellers.end() ) {
		sellers.erase(it);
	}
}

ITEM_ID DataTextfile::AddItem(ItemData& item)
{
	SellerData seller = GetSeller(item.seller_id);
	ItemsDataArray items = GetSellerItems(id);
	
	item.unique_id = axy;
	items.push_back(item);
	SaveSellerToTxtFile( seller, items );
}
  
void DataTextfile::ModifyItem(ITEM_ID item_id, ItemData& item)
{
	ItemDataIterator it = items.find( item_id );
	if ( it == items.end() ) {
		throw new WrongItemIdException();
	}
	
	*it = item;
	
	SellerData seller = GetSeller(it->seller_id);
	ItemsDataArray items = GetSellerItems(it->seller_id);
	SaveSellerToTxtFile(seller, items);
}

void DataTextfile::RemoveItem(ITEM_ID id)
{
	ItemDataIterator it = items.find( item_id );
	if ( it == items.end() ) {
		throw new WrongItemIdException();
	}
	
	items.erase(it);
	
	SellerData seller = GetSeller(it->seller_id);
	ItemsDataArray items = GetSellerItems(it->seller_id);
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
	
	throw WrongSellerIdException();
}
