
#include "CacheStorageHelper.h"


bool CacheStorageHelper::isFilePathValid()
{
	string dbslash("//");//we use  "//" to prevent access to root folder of computer (file://..)
	size_t pos = filePath.find(dbslash);//if no "//" the we end up at end of string
	if(pos == string::npos) {
		return true;//no "//" so it is valid: cache/host/file<--- WE END UP HERE (npos)
	}
	else {
		return false;//dont let access file://..
	}
}

int CacheStorageHelper::getFileLength()
{
	if(cachedFile->good()) {
		cachedFile->seekg(0, std::ios_base::end);
		int len = cachedFile->tellg();
		cachedFile->seekg(0, std::ios_base::beg);
		return len;
	}
	return -1;
}

bool CacheStorageHelper::isFileExisted()
{
	return fileExisted;
}

bool CacheStorageHelper::readFileContent(char *buffer, int bufferSize)
{
	//fstream* cachedFile
	cachedFile->read(buffer, bufferSize);
	if(*cachedFile)
		return true;
	else
		return false;
}

bool CacheStorageHelper::createDirectory()
{
	int pos = filePath.rfind('/');//to leave aside the file
	string dir(filePath, 0, pos);//only consider cache/host
	string mkdirStr = "mkdir -p " + dir;//-p to create intermediate directories if needed (first time cache/ and then host/
	system(mkdirStr.c_str());//outputting to terminal
	cout << "++++++++++ I was creating directory " << dir << endl;
	return true;
}

bool CacheStorageHelper::writeIntoFile(char *data, int dataSize)
{
	if(cachedFile->good()) {
		cachedFile->write(data, dataSize);
		return true;
	}
	return false;
}
