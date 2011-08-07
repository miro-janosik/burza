#ifndef _dataaccess_h_
#define _dataaccess_h_

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
	int	CATEGORY_ID
SELLERS
	primary key int SELLER_ID
	string SHORT_CODE
	string NAME
	string COMMENT
	datetime MODIFIED
	bool	CHANGING
	string CHANGED_BY
	int	CATEGORY_ID
LOGS
	primary key int LOG_ID
	int ITEM_ID
	int SELLER_ID
	string message

SELLER_CATEGORY
	primary key int ID
	double FEE_PER_SALE
ITEM_CATEGORY
	primary key int ID
	double FEE_PER_ITEM

xx SETTINGS
	primary key string key
	string value
SETTINGS
KEY	VALUE
*/

namespace Burza {

typedef int SELLER_ID;
typedef int ITEM_ID;
typedef int SELLER_CATEGORY_ID;
typedef int ITEM_CATEGORY_ID;

static const SELLER_ID SELLER_ID_NONE = -1;
static const ITEM_ID ITEM_ID_NONE = -1;
static const SELLER_CATEGORY_ID SELLER_CATEGORY_ID_NONE = -1;
static const ITEM_CATEGORY_ID ITEM_CATEGORY_ID_NONE = -1;

struct SellerData {
	std::string				name;
	std::string				shortcode;
	std::string				note;
	SELLER_CATEGORY_ID		category;

	ConvFuncs::s_datetime	last_update;
	// ID generated through Register::GenerateSellerUniqueCode()
	SELLER_ID				id;

public:
	SellerData();
	SellerData(string _name, string _shortcode, string _note, int _category);
} ;

struct ItemData {
	std::string				name;
	int						number;
	double					price;
	ITEM_CATEGORY_ID		category;
	int						status;

	ITEM_ID					id;
	ConvFuncs::s_datetime	last_update;
	SELLER_ID				seller_id;
	
public:
	ItemData();
	ItemData(string _name, int _number, double _price, int _category, int _status);
} ;

struct SellerCategory {
	SELLER_CATEGORY_ID	id;
	char				letter;
	double				seller_fee;
};

struct ItemCategory {
	ITEM_CATEGORY_ID	id;
	char				letter;
	double				fee_per_item;
};

typedef vector<SellerData> SellerDataArray;
typedef SellerDataArray::iterator SellerDataIterator;

typedef vector<ItemData> ItemDataArray;
typedef ItemDataArray::iterator ItemDataIterator;

typedef vector<SellerCategory> SellerCategoryArray;
typedef SellerCategoryArray::iterator SellerCategoryIterator;

typedef vector<ItemCategory> ItemCategoryArray;
typedef ItemCategoryArray::iterator ItemCategoryIterator;

/* Abstract class providing interface functions to access the BURZA data.
  Reason of the interface is that data can be stored either locally in data files
  or on a Db server.
*/
class DataAccessInterface {
  public:  
  //! Data functions
	
  //! Get list of sellers data.
  virtual SellerDataArray GetSellers( void ) = 0;
  
  //! Get single seller data.
  //! Throws WrongSellerIdException if no such seller is present.
  virtual SellerData& GetSeller(SELLER_ID id) = 0;
  
  //! Get single seller data.
  //! Throws WrongSellerIdException if no such seller is present.
  virtual SellerData& GetSellerByCode(string& code) = 0;
  
  //! Get seller's items.
  //! Throws WrongSellerIdException if no such seller is present.
  virtual ItemDataArray GetSellerItems(SELLER_ID id) = 0;

  //! Get seller's item. Throws WrongItemNumberException if item not present.
  //! Throws WrongSellerIdException if no such seller is present.
  virtual ItemData GetSellerItem(SELLER_ID id, int item_number) = 0;
  
  //! Adds new seller, returns ID.
  virtual SELLER_ID AddSeller(SellerData& seller);

  //! Call when you want to modify seller. Returns false if already being modified.
  //! In that case wait or cancel modifications.
  //! Throws WrongSellerIdException if no such seller is present.
  virtual bool ModifySellerStart(SELLER_ID id);
  
  //! Call when you want to finish seller modifications.
  //! Throws WrongSellerIdException if no such seller is present.
  virtual void ModifySellerEnd(SELLER_ID id, SellerData& seller);

  //! Removes seller.
  //! Throws WrongSellerIdException if no such seller is present.
  virtual void RemoveSeller(SELLER_ID id);

  //! Adds new item.
  virtual ITEM_ID AddItem(ItemData& item, SELLER_ID id);
  
  //! Changes item.
  virtual void ModifyItem(ITEM_ID item_id, ItemData& item);

  //! Remove item.
  //! Throws WrongItemIdException if no such item is present.
  virtual void RemoveItem(ITEM_ID id);

  //! Get item.
  //! Throws WrongItemIdException if no such item is present.
  virtual ItemData GetItem(ITEM_ID id) = 0;

  //! Get multiple items
  //! Throws WrongItemIdException if no such item is present.
  virtual ItemDataArray& GetItems(vector<ITEM_ID> ids);
  
  // ==== categories ====
  ItemCategory GetItemCategory(ITEM_CATEGORY_ID id);
  ItemCategory GetItemCategoryByLetter(char letter);

  ItemCategoryArray GetItemsCategories(vector<ITEM_CATEGORY_ID>& ids);
  ItemCategoryArray GetItemsCategoriesByLetter(vector<char>& letters);

  SellerCategory GetSellerCategory(SELLER_CATEGORY_ID id);
  SellerCategory GetSellerCategoryByLetter(char letter);

  SellerCategoryArray GetSellersCategories(vector<SELLER_CATEGORY_ID>& ids);
  SellerCategoryArray GetSellersCategoriesByLetter(vector<char>& letters);

  // ==== modified time ====
  
  //! Returns true if seller was modified since last local change
  //! Throws WrongSellerIdException if no such seller is present.
  virtual bool IsSellerModified(SELLER_ID id);
  
  //! Returns seller data if seller was modified on server since last local change
  //! Returns null otherwise
  //! Throws WrongSellerIdException if no such seller is present.
  virtual SellerData* GetSellerIfModified(SELLER_ID id);

  //! Returns true if item was modified on server since last local change
  //! Throws WrongItemIdException if no such item is present.
  virtual bool IsItemModified(ITEM_ID id);

  //! Returns item data if item was modified on server since last local change
  //! Returns null otherwise
  //! Throws WrongItemIdException if no such item is present.
  virtual ItemData* GetItemIfModified(ITEM_ID id);

  // ==== network identification and connection ====
  
  //! set name of local machine or program, to be seen on different machines in form
  //! of "Seller XYZ is being modified by NAME since DATE"
  virtual void SetLocalName(string& name);

  //! Connect to server
  virtual bool ConnectDb(string& serverName, string& login, string& password);
  
public:
  // ==== helper functions ====
	
  //! looks for item in the item array. Returns NULL if such item not found in the array
  static ItemData* FindItemByNumber( ItemDataArray& list, int number );

  //! looks for a seller in the array. Returns index of the seller in the array, or -1 if not found
  static int FindSellerIndexByCode( SellerDataArray& list, string code );
  
  //! returns unique non-used seller ID
  virtual SELLER_ID GetNewSellerId( void ) = 0;

  //! returns unique non-used item ID
  virtual ITEM_ID GetNewItemId( void ) = 0;
  
  //! Calls AddItem on multiple items. Sets item id's
  virtual void AddItems( ItemDataArray& items, SELLER_ID id );
  
  //! returns array of exactly the same size as array in parameter.
  //! returned array contains category for items in the same order as items in parameter.
  virtual ItemCategoryArray GetItemsCategories( ItemDataArray& items );
  
  //! returns string representing 'item status'
  static const char* ItemStatus2String( ItemStatus status );
  
  //! converts status to short character and back.
  static char ItemStatus2Char( ItemStatus status );
  static ItemStatus Char2ItemStatus( char c );
};

//! Base class that should be used as base for classes that read and write data.
class DataStorage : public DataAccessInterface 
{
  public:

	//! Get seller's items.
	ItemDataArray GetSellerItems(SELLER_ID id);

	//! set name of local machine or program, to be seen on different machines in form
	//! of "Seller XYZ is being modified by NAME since DATE"
	void SetLocalName(string& name)
	{
		agent_name = name;
	}
	
  protected:
	//! All loaded items, items of all sellers.
	ItemDataArray	items;
	
	//! All loaded sellers
	SellerDataArray	sellers;
	
	//! name of local program/process/computer
	string		agent_name;
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

class WrongSellerException : public DataStorageException {
    virtual const char* what() const throw() {
        return "Wrong Seller";
    }
};

class WrongItemException : public DataStorageException {
    virtual const char* what() const throw() {
        return "Wrong Item";
    }
};

class WrongSellerCategoryException : public DataStorageException {
    virtual const char* what() const throw() {
        return "Wrong Item category";
    }
};

class WrongItemCategoryException : public DataStorageException {
    virtual const char* what() const throw() {
        return "Wrong Item category";
    }
};

class ServerException : public DataStorageException {
    virtual const char* what() const throw() {
        return "Server or network problems";
    }
};

class NotImplementedException : public DataStorageException {
    virtual const char* what() const throw() {
        return "Functionality not implemented";
    }
};

enum ItemStatus {
	not_in_stock_yet = 0,
	on_stock,
	sold,
	returned,
	given_as_gift,
	lost,
	undefined,
	ItemStatusCount
} ;

}; // namespace Burza

#endif