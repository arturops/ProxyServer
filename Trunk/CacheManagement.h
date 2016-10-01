/**
 * This file store all the filenames in cache directory. 
 * It only store the file store by current process. (If the proxy started twice, file cached first time would not be recorded)
 */
 
#ifndef _CACHE_MANAGEMENT_H_
#define _CACHE_MANAGEMENT_H_

#include <algorithm>
#include <iostream>
#include <list>
#include <vector>
#include <fstream>
#include <string>
#include <regex>

using std::list;
using std::vector;
using std::string;
using std::cout;
using std::endl;
using std::istringstream;
using std::ifstream;
using std::ofstream;
using std::fstream;
using std::search;
	
class CacheManagement {
public:
	static CacheManagement* createCacheManagement();	//static function used to ensure the CacheManagement can only be created once.

	void insertIntoCachedFiles(string filename);		//insert filename into cachedFiles list.
	vector<string>* searchForURL(string keyword);		//search the keyword in cachedFiles url
	vector<string>* searchForContent(string keyword);	//search the keyword in cachedFiles content
	
	void showFiles();	// use in debug, will show all the file in cachedFiles list.
private:
	CacheManagement();
	
	pthread_mutex_t mutex;
	static bool existed;
	list<string> cachedFiles;
};

#endif

