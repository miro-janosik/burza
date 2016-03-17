#ifndef _RepeatNotifierWorker_h_
#define _RepeatNotifierWorker_h_

#include <thread>
#include <mutex>
#include <chrono>
#include <functional>

class RepeatNotifierWorker
{
public:
	typedef std::function<void(void)> RepeatedAction;

	void Start(chrono::milliseconds delay, RepeatedAction repeatedAction)
	{
		Stop();
		
		chrono::milliseconds stopDelay(10);
		while (1)
		{
			{
				lock_guard<mutex> lock(dataLock);
				if (active == false) {
					workerThread = nullptr;
					break;
				}
			}
			
			std::this_thread::sleep_for(stopDelay);
		}
		
		{
			lock_guard<mutex> lock(dataLock);
			this->delay = delay;
			this->repeatedAction = repeatedAction;
			active = true;
		}
		
		// Start a new worker thread.
		workerThread = new std::thread(std::bind(&RepeatNotifierWorker::DoWork, this));
	}
	
	void Stop()
	{
		lock_guard<mutex> lock(dataLock);
		active = false;
	}
	
	~RepeatNotifierWorker()
	{
		Stop();
	}
	
private:
	mutable std::mutex dataLock;
	std::thread* workerThread = nullptr;
	bool active;
	chrono::milliseconds delay;
	RepeatedAction repeatedAction;
	
	void DoWork( void )
	{
		while (1)
		{
			std::this_thread::sleep_for(delay);
			
			// read variable while locked
			{
				lock_guard<mutex> lock(dataLock);
				if (!active)
					break;
			}
			
			repeatedAction();
		}
	}
};

/*
class WorkerBase
{
public:
	typedef std::function<void(void)> Action;

	void Start(chrono::milliseconds repeatDelay, Action finishAction)
	{
		this->delay = repeatDelay;
		this->repeatedAction = repeatedAction;
		this->finishAction = finishAction;
		active = true;
		
		// Start a new worker thread.
		workerThread = new std::thread(std::bind(&RepeatNotifierWorker::DoWork, this));
		
		// lock_guard<mutex> lock(dataLock);
	}
	
	void Stop()
	{
		lock_guard<mutex> lock(dataLock);
		active = false;
	}
	
	~RepeatNotifierWorker()
	{
		Stop();
	}
	
private:
	mutable std::mutex dataLock;
	std::thread* workerThread = nullptr;
	bool active;
	chrono::milliseconds delay;
	Action repeatedAction, finishAction;
	
	void DoWork( void )
	{
		while (1)
		{
			std::this_thread::sleep_for(delay);
			
			// read variable while locked
			{
				lock_guard<mutex> lock(dataLock);
				if (!active)
					break;
			}
			
			repeatedAction();
		}
	}
};
*/

#endif _RepeatNotifierWorker_h_
