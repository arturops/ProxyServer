/**
 * This file is a helper of search engine. 
 * The Helper support two kinds of search method.
 *  1. search in URL
 *  2. search in Content
 * It would search with the help of CacheManagement and return corresponding http packet to client.
 */
 
#ifndef _CACHE_SEARCH_HELPER_H_
#define _CACHE_SEARCH_HELPER_H_

#include <cstring>
#include <algorithm>
#include <iostream>
#include <list>
#include <vector>
#include <fstream>
#include <string>
#include <regex>

#include "CacheManagement.h"

using std::list;
using std::vector;
using std::string;
using std::to_string;
using std::cout;
using std::endl;
using std::istringstream;
using std::ifstream;
using std::ofstream;
using std::fstream;
using std::find_if;

static char httpHdr[] = "HTTP/1.1 200 OK\r\nServer: ArtOz\r\nContent-Type: text/html; charset=UTF-8\r\nTransfer-Encoding: chunked\r\n\r\n";

class CacheSearchHelper {
public:
	CacheSearchHelper(string searchStr);
	void doSearch(CacheManagement* cacheMem);
	
	vector<char> retContent;	// return http packet are store in vector
private:
	string keyword;
	string searchType;
};

#endif
