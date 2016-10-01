/**
 * This file is a helper of cache storage function. 
 *  1. create directory and cache data if the path is valid.
 *  2. if the request file are already in cache, then return the cached data instead of get from server.
 */
 
#ifndef _CACHE_STORAGE_HELPER_H_
#define _CACHE_STORAGE_HELPER_H_

#include <algorithm>
#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <regex>

using std::vector;
using std::string;
using std::cout;
using std::endl;
using std::istringstream;
using std::ifstream;
using std::ofstream;
using std::fstream;

#define CACHE_ROOT_DIR0 "cache/"
#define CACHE_ROOT_DIR1 "cache"

class CacheStorageHelper {
public:
	explicit CacheStorageHelper(string cacheFileName) {
		if(cacheFileName.c_str()[0] == '/') {
			filePath = string(CACHE_ROOT_DIR1) + cacheFileName;
		}
		else {
			filePath = string(CACHE_ROOT_DIR0) + cacheFileName;
		}
	
			
		createDirectory();//create a unique directory to save the cache file
		//now separate the file from filepath
		int pos = filePath.rfind('/');
		string fileName(filePath, pos + 1);//avoid slash
		string empty("");
		if(fileName == empty) {
			filePath += "root";//just in case (no name on file)
		}

		fstream tmpFileStream(filePath, fstream::in);
		if(tmpFileStream.fail()) {	// file not existed now;
			fileExisted = false;
			string touch("touch " + filePath);
			system(touch.c_str());
			cachedFile = new fstream(filePath, fstream::out);//same as doing ofstream to write on the file
			if(cachedFile->fail()) {
				cout << "fail when create file" << endl;
			}
		}
		else {	// file existed
			fileExisted = true;
			cachedFile = new fstream(filePath, fstream::out | fstream::in);//::out and ::in are default, but just to remember that the file will be open for reading and writing
			if(cachedFile->fail()) {
				cout << "fail when create file" << endl;
			}
		}
		tmpFileStream.close();
	}
	~CacheStorageHelper() {
		cachedFile->close();
	}
	
	bool isFilePathValid();
	bool isFileExisted();
	int getFileLength();	// get size of file in byte
	
	string& getFilePath() {return filePath;}
	bool createDirectory();

	bool readFileContent(char *buffer, int bufferSize);
	bool writeIntoFile(char *data, int dataSize);
	
private:
	fstream* cachedFile;
	bool fileExisted;
	string filePath;
};


#endif
