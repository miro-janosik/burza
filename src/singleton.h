#ifndef _singleton_h_
#define _singleton_h_

template<class T> class Singleton {
protected:
	static T	*instance;
public:
	static T& Instance( void )
	{
		if ( instance == NULL )
			instance = new T();

		return *instance;
	}

	static void DeleteInstance( void )
	{
		if ( instance == NULL )
			return;

		delete instance;
		instance = NULL;
	}
};

#endif