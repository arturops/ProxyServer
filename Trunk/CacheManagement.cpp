
#include "CacheManagement.h"

//since we have threads accessing to this class we need to lock and unlock the functions 
//so that only one thread can access at a time

bool CacheManagement::existed = false;

//allocate cachemanagement pointer
CacheManagement* CacheManagement::createCacheManagement()
{
	CacheManagement* ret = NULL;
	if(!CacheManagement::existed) {
		ret = new CacheManagement();
		CacheManagement::existed = true;//just to make sure we dont allocate another cachemanagement 
	}
	return ret;
}


void CacheManagement::insertIntoCachedFiles(string filename)
{
	pthread_mutex_lock(&mutex);
	cachedFiles.insert(cachedFiles.begin(), filename);//insert filename on list could have used push_back
	pthread_mutex_unlock(&mutex);
}


vector<string>* CacheManagement::searchForURL(string keyword)
{
	pthread_mutex_lock(&mutex);
	vector<string> *vec = new vector<string>();
	//perform again a based-range for loop to find a match
	for(string path : cachedFiles) {
		if(path.find(keyword) != string::npos) {
			vec->push_back(path);//store the string that has match in vec
		}
	}
	pthread_mutex_unlock(&mutex);
	return vec;
}

vector<string>* CacheManagement::searchForContent(string keyword)
{
	pthread_mutex_lock(&mutex);
	vector<string> *vec = new vector<string>();
	for(string path : cachedFiles) {
		ifstream *ifs;//to open a file stream for reading
		if(path.find('/') == path.rfind('/')) {
			ifs = new ifstream("cache/" + path + "root");
		}
		else {
			ifs = new ifstream("cache/" + path);
		}
		//try opening stream
		if(ifs->good()) {
			ifs->seekg(0, std::ios_base::end);
			int len = ifs->tellg();//get size of file
			ifs->seekg(0, std::ios_base::beg);
			char *data = new char[len];
			ifs->read(data, len);//read all bytes in file
			
			auto it = search(data, data + len, keyword.begin(), keyword.end());//search the keyword in the whole data (file)
			if(it != data + len) {
				//if we find a match within the file, put the path in vector<string>
				vec->push_back(path);
			}
			delete []data;
		}
		delete ifs;
	}
	pthread_mutex_unlock(&mutex);
	return vec;
}

CacheManagement::CacheManagement()
{
	pthread_mutex_init(&mutex, NULL);
	cachedFiles.erase(cachedFiles.begin(), cachedFiles.end());//clean list if there any
}

void CacheManagement::showFiles()
{
	cout << "#################################" << endl;
	for(string t : cachedFiles)//range-based for loop to iterate all strings in list<string> cachedFiles
		cout << t << endl;
	cout << "#################################" << endl;
}
