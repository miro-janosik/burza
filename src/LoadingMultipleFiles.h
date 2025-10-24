#ifndef LOADINGMULTIPLEFILES_H
#define LOADINGMULTIPLEFILES_H

#include <set>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <chrono>
#include <functional>
#include <ostream>

#include <gtkmm.h>

#include "RepeatNotifierWorker.h"

using namespace std;
using namespace std::placeholders;

namespace Burza 
{

class LoadSellerWorkerExchangeObject
{
public:
	string status;
	double progressPercent;
	bool working;
	bool finished;
};

class LoadSellerWorker
{
public:
	typedef function<void(string)> AddToStatusFunc;
	typedef function<void(AddToStatusFunc)> Work;
	
	void Start(Work work)
	{
		{
			lock_guard<mutex> lock(dataLock);
			data.working = true;
			data.finished = false;
			this->work = work;
			
			// Start a new worker thread.
			workerThread = new std::thread(std::bind(&LoadSellerWorker::DoWork, this));
		}
	}
	
	void Stop()
	{
		lock_guard<mutex> lock(dataLock);
		data.working = false;
		data.finished = true;
	}
	
	LoadSellerWorkerExchangeObject GetData()
	{
		lock_guard<mutex> lock(dataLock);
		return data;
	}
	
	LoadSellerWorker()
	{
		data.status = "";
		data.progressPercent = 0.0;
		data.working = false;
		data.finished = false;
	}
	
	~LoadSellerWorker()
	{
		Stop();
	}
	
private:
	std::thread* workerThread = nullptr;
	Work work = nullptr;
	
	mutable std::mutex dataLock;
	LoadSellerWorkerExchangeObject	data;
	
	void DoWork( void )
	{
		{
			lock_guard<mutex> lock(dataLock);
			data.working = true;
			data.finished = false;
		}
		
		work(std::bind(&LoadSellerWorker::AddToStatus, this, _1));
		
		{
			lock_guard<mutex> lock(dataLock);
			data.working = false;
			data.finished = true;
		}
	}
	
	void AddToStatus( string addition )
	{
		lock_guard<mutex> lock(dataLock);
		data.status += addition;
		data.status += "\n";
	}
};

class LoadingMultipleFiles
{
public:
	LoadingMultipleFiles();
	void ConnectButtons( void );
	
	// displays 'folder select' dialog, and fills [sourcePath] if folder was selected.
	bool AskForFolder( Gtk::Window& parentWindow );
	
	void Finished(const Glib::RefPtr<Gio::AsyncResult>& result, const Glib::RefPtr<Gtk::FileDialog>& dialog);

	void RunImport( Gtk::Window& parentWindow );
	
public:
	
	// Glib::RefPtr<Gnome::Glade::Xml> xmlWindow; // xmlWndLoadFileProgress
	Gtk::Window*	window;

	Gtk::TextView		*editLoadingText;

	Gtk::Button		*buttonInterrupt;
	Gtk::Button		*buttonOk;
	
private:
	
	// path selected by user
	string		sourcePath;
	
	const string folderConversion = "conversion";
	const string folderFinished = "finished";
	const string folderError = "error";

	// this dispatcher will be called from worker thread and received in GUI thread
	Glib::Dispatcher loadingRefreshNotificationDispatcher;

	RepeatNotifierWorker	repeatNotifier;
	
	LoadSellerWorker	loadSellersWorker;
	
	vector<string> folderNames { folderConversion, folderFinished, folderError };
	
	// this notification will be called frequently to refresh 'progress of importing'
	void RefreshingNotificationAction( void );

	// creates folders that are needed in a folder where import is done
	void CreateOutputFolders( void );
	
	// displays the loading window
	void DisplayWindow( Gtk::Window& parentWindow );
	
	void LoadMultipleFiles( function<void(int, string)> addToStatus );

	void FinishedClickedOk();
	
	// logs text
	void ImportMultiSellersLog(const char* text);
	void ImportMultiSellersLog(std::string text);

	
	void SimpleNotification( int perc, string text );
};

}

#endif // LOADINGMULTIPLEFILES_H
