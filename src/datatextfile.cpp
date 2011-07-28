#include "datatextfile.h"

using namespace Burza;
using namespace std;

void DataTextfile::SaveSellerToTxtFile( SellerData& seller, ItemDataArray& items )
{
	string oldname = GetSellerFilenameExisting(seller);
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

void Seller::LoadFromTxtFile( SELLER_ID id, SellerData& seller, ItemDataArray& items )
{
	items.clear();
	
	string filename = GetSellerFilenameExisting( id );
	
	CConvFile	file;
	file.OpenFile( filename.c_str(), CConvFile::fm_text, CConvFile::fm_read );
	
	if ( file.IsOpened() == 0 ) {
		throw;
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
}

22:31 < mirex> Sho_: how do you do it in 4.2?
22:32 < Sho_> mirex: there's a page in the config dialog to add new qthelp 
              files, so now you just need one for the c++ stl
22:33 < Sho_> http://www.steveire.com/libstdc++-v3.qch might do
22:34 < Sho_> or 
https://docs.google.com/leaf?id=0B6w3G_xx1dDnMjYyMjQ1ODctNGZmMy00MzRhLWJlOGEtMDdhOTc0N2EzOTgx&sort=name&layout=list&num=50

SellerDataArray DataTextfile::GetSellers( void )
{
	std::string		predajca_dir, predajca_pat, predajca_fname;
			
	predajca_dir = Centrala::Instancia()->Nastavenia()->m_data_dir_path;
	
	predajca_pat = predajca_dir;
	predajca_pat += "*.txt";
	
	bool found;
	found = ConvFuncsInstance->Find_FirstFile( predajca_pat.c_str() );
	while( found ) {
		
		found = ConvFuncsInstance->Find_NextFile();
	}
	
}

SellerData& DataTextfile::GetSeller(SELLER_ID id)
{
  
}
  
ItemDataArray DataTextfile::GetSellerItems(SELLER_ID id)

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
	
	return "";
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
	ItemDataArray	items = GetSellerItems( seller.unique_id );
  
	SaveSellerToTxtFile( seller, items );
}

  //! Call when you want to modify seller. Returns false if already being modified.
  //! In that case wait or cancel modifications.
  bool DataTextfile::ModifySellerStart(SELLER_ID id);
  
  //! Call when you want to finish seller modifications.
  void ModifySellerEnd(SELLER_ID id, SellerData& seller);

  //! Removes seller.
  void RemoveSeller(SELLER_ID id);

  //! Adds new item.
  ITEM_ID AddItem(ItemData& item);
  
  //! Changes item.
  void ModifyItem(ITEM_ID item_id, ItemData& item);

  //! Remove item.
  void RemoveItem(ITEM_ID id);

  //! Returns true if seller was modified since last local change
  bool IsSellerModified(SELLER_ID id);

  //! Returns seller data if seller was modified on server since last local change
  //! Returns null otherwise
  SellerData* GetSellerIfModified(SELLER_ID id);

  //! Returns true if item was modified on server since last local change
  bool IsItemModified(ITEM_ID id);

  //! Returns item data if item was modified on server since last local change
  //! Returns null otherwise
  ItemData* GetItemIfModified(ITEM_ID id);

  //! set name of local machine or program, to be seen on different machines in form
  //! of "Seller XYZ is being modified by NAME since DATE"
  void SetLocalName(string& name);

  //! Connect to server
  bool ConnectDb(string& serverName, string& login, string& password);
