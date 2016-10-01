/**
 * This file is a wraper of two processes. 
 *  1. send the HttpHdr got from user to real server
 *  2. receive the packet get from real server and send them to the user's browser.
 *  
 *  * It only handle GET request.
 */
 
#ifndef _HTTP_GETFORWARDER_H_
#define _HTTP_GETFORWARDER_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <pthread.h>

#include <vector>
#include <list>
#include <string>
#include <algorithm>
#include <iostream>
#include <sstream>

#include "CacheStorageHelper.h"
#include "HttpHdr.h"
#include "HttpPkt.h"

using std::vector;
using std::string;
using std::cout;
using std::endl;
using std::istringstream;
using std::search;

#define MAXDATASIZE 1024

class HttpGetForwarder {
public:
	explicit HttpGetForwarder(int sockfd, HttpHdr* httpGetRequest, CacheStorageHelper &helper);
	~HttpGetForwarder(){}

	int errorHappens = 0;
	void sendToUrl(HttpPkt &pkt);
	void sendToClient(char* data, int size);
	static bool sendDataToClient(int clientfd, char* data, int size);
private:
	const int MAX_URL_LEN = 256;
	CacheStorageHelper cacheHelper;
	
	int clientSockfd;	// socket fd used to send data back to browser;
	int serverSockfd;	// socket fd of URL, used to forward HTTP GET request;
	struct sockaddr_in serv_addr;

	string hostname;
	HttpHdrType requestType;
	
	char* sendToURLBuffer;
	int sendToURLBufferSize;
	vector<char> sendToClientStr;
	
};

#endif
