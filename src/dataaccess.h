#include <vector>
#include "convfncs.h"

using namespace std;

/*
TABLES:
ITEMS
	primary key int ITEM_ID
	string NAME
	int SELLER_ID
	float PRICE
	datetime MODIFIED
	bool	CHANGING
	string CHANGED_BY
SELLERS
	primary key int SELLER_ID
	string SHORT_CODE
	string NAME
	string COMMENT
	datetime MODIFIED
	bool	CHANGING
	string CHANGED_BY
LOGS
	primary key int LOG_ID
	int ITEM_ID
	int SELLER_ID
	string message

xx SETTINGS
	primary key string key
	string value
SETTINGS
KEY	VALUE
*/

namespace Burza {

class SellerData {
public:
	std::string		name;
	std::string		shortcode;
	std::string		note;
	char			category;

	ConvFuncs::s_datetime	last_update;
	// ID generated through Register::GenerateSellerUniqueCode()
	unsigned int	unique_id;
} ;

class ItemData {
public:
	std::string		name;
	int				number;
	double			price;
	char			category;
	int				status;

	ConvFuncs::s_datetime	last_update;
	SELLER_ID		seller_id;
} ;

typedef int SELLER_ID;
typedef int ITEM_ID;

typedef vector<SellerData> SellerDataArray;
typedef SellerDataArray::iterator SellerDataIterator;

typedef vector<ItemData> ItemDataArray;
typedef ItemDataArray::iterator ItemDataIterator;

/* Abstract class providing interface functions to access the BURZA data.
  Reason of the interface is that data can be stored either locally in data files
  or on a Db server.
*/
class DataAccessInterface {
  
  //! Get list of sellers data.
  SellerDataArray GetSellers( void ) = 0;
  
  //! Get single seller data.
  SellerData& GetSeller(SELLER_ID id) = 0;
  
  //! Get seller's items.
  ItemDataArray GetSellerItems(SELLER_ID id) = 0;

  //! Adds new seller, returns ID.
  SELLER_ID AddSeller(SellerData& seller);

  //! Call when you want to modify seller. Returns false if already being modified.
  //! In that case wait or cancel modifications.
  bool ModifySellerStart(SELLER_ID id);
  
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

  //! Returns items of specified seller
  ItemDataArray GetSellerItems(SELLER_ID id);
  
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
};

//! Base class that should be used as base for classes that read and write data.
class DataStorage : public DataAccessInterface 
{
  public:

	//! Get seller's items.
	ItemDataArray GetSellerItems(SELLER_ID id);
	
  protected:
	ItemDataArray	items;
	
	SellerDataArray	sellers;
};

class DataStorageException : public std::exception {
    virtual const char* what() const throw() {
        return "DataStorageException";
    }
};

class NotConnectedException : public DataStorageException {
    virtual const char* what() const throw() {
        return "Not Connected to Data";
    }
};

class WrongSellerIdException : public DataStorageException {
    virtual const char* what() const throw() {
        return "Wrong Seller Id";
    }
};

class WrongItemIdException : public DataStorageException {
    virtual const char* what() const throw() {
        return "Wrong Item Id";
    }
};

Program: jeden button - Nastavenia/setup:

	SERVER CONNECTION
		SERVER_NAME/IP
		LOGIN
		PWD
	Registration key
		key (correct/incorrect)
		registered to
		host id
		
ak pri startupe nieje server, tak msg a pouzi lokal db

