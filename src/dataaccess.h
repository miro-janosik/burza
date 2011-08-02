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

typedef int SELLER_ID;
typedef int ITEM_ID;

class SellerData {
public:
	std::string		name;
	std::string		shortcode;
	std::string		note;
	char			category;

	ConvFuncs::s_datetime	last_update;
	// ID generated through Register::GenerateSellerUniqueCode()
	SELLER_ID		unique_id;

public:
	SellerData(string _name, string _shortcode, string _note, char _category);
} ;

class ItemData {
public:
	std::string		name;
	int				number;
	double			price;
	char			category;
	int				status;

	ITEM_ID			unique_id;
	ConvFuncs::s_datetime	last_update;
	SELLER_ID		seller_id;
	
public:
	ItemData(string _name, int _number, double _price, char _category, int _status, SELLER_ID _seller_id);
} ;

typedef vector<SellerData> SellerDataArray;
typedef SellerDataArray::iterator SellerDataIterator;

typedef vector<ItemData> ItemDataArray;
typedef ItemDataArray::iterator ItemDataIterator;

/* Abstract class providing interface functions to access the BURZA data.
  Reason of the interface is that data can be stored either locally in data files
  or on a Db server.
*/
class DataAccessInterface {
  public:  
  //! Get list of sellers data.
  virtual SellerDataArray GetSellers( void ) = 0;
  
  //! Get single seller data.
  virtual SellerData& GetSeller(SELLER_ID id) = 0;
  
  //! Get single seller data.
  virtual SellerData& GetSellerByCode(string& code) = 0;
  
  //! Get seller's items.
  virtual ItemDataArray GetSellerItems(SELLER_ID id) = 0;

  //! Get seller's item.
  virtual ItemData GetSellerItem(SELLER_ID id, int item_number) = 0;
  
  //! Adds new seller, returns ID.
  virtual SELLER_ID AddSeller(SellerData& seller);

  //! Call when you want to modify seller. Returns false if already being modified.
  //! In that case wait or cancel modifications.
  virtual bool ModifySellerStart(SELLER_ID id);
  
  //! Call when you want to finish seller modifications.
  virtual void ModifySellerEnd(SELLER_ID id, SellerData& seller);

  //! Removes seller.
  virtual void RemoveSeller(SELLER_ID id);

  //! Adds new item.
  virtual ITEM_ID AddItem(ItemData& item);
  
  //! Changes item.
  virtual void ModifyItem(ITEM_ID item_id, ItemData& item);

  //! Remove item.
  virtual void RemoveItem(ITEM_ID id);

  //! Get item.
  virtual ItemData GetItem(ITEM_ID id) = 0;
  
  //! Returns true if seller was modified since last local change
  virtual bool IsSellerModified(SELLER_ID id);
  
  //! Returns seller data if seller was modified on server since last local change
  //! Returns null otherwise
  virtual SellerData* GetSellerIfModified(SELLER_ID id);

  //! Returns true if item was modified on server since last local change
  virtual bool IsItemModified(ITEM_ID id);

  //! Returns item data if item was modified on server since last local change
  //! Returns null otherwise
  virtual ItemData* GetItemIfModified(ITEM_ID id);

  //! set name of local machine or program, to be seen on different machines in form
  //! of "Seller XYZ is being modified by NAME since DATE"
  virtual void SetLocalName(string& name);

  //! Connect to server
  virtual bool ConnectDb(string& serverName, string& login, string& password);
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

}; // namespace Burza

#endif