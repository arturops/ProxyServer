/**
 * This file is a wraper of HttpHeader got from User's browser.
 */

#ifndef _HTTP_HDR_H_
#define _HTTP_HDR_H_

#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>

using std::vector;
using std::string;
using std::cout;
using std::endl;
using std::istringstream;
using std::search;

typedef enum HttpHdrType {
	GET, POST, CONNECT, HEAD, PUT, DELETE, OPTIONS, TRACE, UNKNOWN
}HttpHdrType;

class HttpHdr {
public:
	vector<char> hdr;
	HttpHdr() {};
	HttpHdr(vector<char> &vec) : hdr(vec) {};
	
	HttpHdrType getHdrType();
	bool isHdrComplete();
	void insertContent(vector<char>::iterator begin, vector<char>::iterator end);
	string getRequestHost();
	string getRequestFile();
	
	void removeHostPrefix();
	void printHdrHex();
	void printHdrASC(int prefix);
};



#endif

