/**
 * This file is a wraper of HttpPacket got from HTTP Server.
 */

#ifndef _HTTP_PKT_H_
#define _HTTP_PKT_H_

#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>

#include "HttpHdr.h"

using std::vector;
using std::string;
using std::cout;
using std::endl;
using std::istringstream;
using std::search;

typedef enum TransferCoding {
	GENERAL, CHUNKED, TC_UNKNOWN
}TransferCoding;

class HttpPkt {
	int contentLength = 0;
	TransferCoding transferCodingType = TransferCoding::TC_UNKNOWN;
public:
	vector<char> pkt;
	
	
	HttpPkt(vector<char> &vec) : pkt(vec) {}
	HttpPkt() {}
	
	void insertContent(vector<char>::iterator begin, vector<char>::iterator end);
	void printPkt();
	bool isPktComplete();
	
	HttpHdr* getHdr();
	TransferCoding getCodingType();
};



#endif
