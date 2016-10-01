
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

#include "FilterHelper.h"
#include "trmColor.h"
#include "HttpPkt.h"
#include "HttpGetForwarder.h"
#include "CacheManagement.h"
#include "CacheSearchHelper.h"
#include "CacheStorageHelper.h"

#define BUFF_SIZE	1024

using namespace std;

#pragma pack(1)

int portnumber;

struct _sock_vec{
	int sock;
	vector<char>* vec;
	pthread_t pid;
};

struct _sock_hdr{
	int sock;
	HttpHdr* hdr;
};

vector<char> hdrEnd = {0x0d, 0x0a, 0x0d, 0x0a};

// variable of clients
//list<_sock_vec> clients;
_sock_vec clients[FD_SETSIZE];
int maxfd, nready;
fd_set rset, allset;
int sockCnt = 0;
CacheManagement *cacheMem;

char recvbuf[BUFF_SIZE]; //1024

const int sin_size = sizeof(struct sockaddr_in);

/** Set up a IPV4 TCP Listen socket and return the file descruptor of the socket.
 *
 */
int setUpV4TCPListenSocket(int portnum)
{
	int listenfd;
	struct sockaddr_in chiaddr, servaddr;

	// establish socket
	listenfd = socket(AF_INET,SOCK_STREAM,0);
	if(listenfd == -1) {
		printf("Server Error, socket established error %s\n", (char*)strerror(errno));
		exit(1);
	}
	// set socket param
	int opt = SO_REUSEADDR;
	setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	bzero(&servaddr,sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(portnum);

	// bind socket to ip:port
	if (::bind(listenfd, (struct sockaddr *)&servaddr, sizeof(struct sockaddr)) == -1) {
		fprintf(stderr, "Server Error, bind error:%s\n", strerror(errno));
		exit(1);
	}
	
	if(listen(listenfd,SOMAXCONN) == -1) {
		printf("listen error: %s\n", strerror(errno));
		exit(1);
	}
	return listenfd;
}


void* doRequest(void* ptr)
{
	_sock_hdr *s = (_sock_hdr*)ptr;
	string host = s->hdr->getRequestHost();
	string file = s->hdr->getRequestFile();
	
	CacheStorageHelper helper(host + file);
	cout << "(" << s->sock << ")" << "***** In New Thread ***** " << host << ":" << file << endl;
	
	if(host == "127.0.0.1" || host == "localhost") {
		if(file == "/search.html") {
			ifstream ifs("www/search.html");//to read from file
			ifs.seekg(0, std::ios_base::end);//move to end of file (stream)
			int len = ifs.tellg();//get the position, since it is the end is SIZE OF FILE
			cout << len << endl;
			ifs.seekg(0, std::ios_base::beg);//return to beginning
			char *dat = new char[len];
			ifs.read(dat, len);//read data from file search.html
			
			HttpGetForwarder::sendDataToClient(s->sock, dat, len);//no HttpGetForwarder object so class::
		}
		//if search is not at the end of the string (npos) you go to search (query_string) 
		else if(file.find("search.html") != string::npos) {
			CacheSearchHelper searchHelper(file);//to identify keyword and type
			searchHelper.doSearch(cacheMem);//fills in retContent
			
			int len = searchHelper.retContent.size();//number of elements resulting from search
			cout << len << endl;
			char *dat = new char[len];
			copy(searchHelper.retContent.begin(), searchHelper.retContent.end(), dat);//copy retcontent to dat

			HttpGetForwarder::sendDataToClient(s->sock, dat, len);//send dat to client (result of search)
		}
		else {	// page not found
			ifstream ifs("www/404.html");
			ifs.seekg(0, std::ios_base::end);
			int len = ifs.tellg();
			cout << len << endl;
			ifs.seekg(0, std::ios_base::beg);
			char *dat = new char[len];
			ifs.read(dat, len);
			
			HttpGetForwarder::sendDataToClient(s->sock, dat, len);
		}
	}
	//if no localhost or 127.0.0.1 found
	else {
		HttpGetForwarder* handler = new HttpGetForwarder(s->sock, s->hdr, helper);
	
		char* data;
		int size = 0;
		HttpPkt pkt;
		
		//CacheStorageHelper helper
		if(helper.isFilePathValid() && helper.isFileExisted()) {	// file path valid and found in cache file
			cout << "(" << s->sock << ")" << "find in cached data." << endl;
			size = helper.getFileLength();
			if(size != -1) {
				data = new char[size];
				bool ret = helper.readFileContent(data, size);
				if(!ret) {
					cout << "error in read cached data" << endl;
				}
			}
			else {
				cout << "size == -1" << endl;
			}
			cout << "(" << s->sock << ")" << "have read " << size << " Bytes from cache and will send them to client..." << endl;
		}
		//else if fileExisted == false
		else if(helper.isFilePathValid()) {	// file path valid but not found in cache file
			cout << "(" << s->sock << ")" << "sending messages to real server " << host << endl;
			cacheMem->insertIntoCachedFiles(host + file);
			handler->sendToUrl(pkt);
			size = pkt.pkt.size();
			if(!(handler->errorHappens)) {
				data = new char[size];
			    copy(pkt.pkt.begin(), pkt.pkt.end(), data);
			}
			if(false == helper.writeIntoFile(data, size)) {
				cout << "error in cache data" << endl;
//				getchar();
			}
			else
				cout << "(" << s->sock << ")" << "have recvived " << size << " Bytes from real server and will send them to client..." << endl;
		}
		else {	// file path not valid, just get from URL and send to client, will not cache this file
			cout << "(" << s->sock << ")" << "sending messages to real server " << host << endl;
			handler->sendToUrl(pkt);
			size = pkt.pkt.size();
			if(!(handler->errorHappens)) {
				data = new char[size];
			    copy(pkt.pkt.begin(), pkt.pkt.end(), data);
			}		
		}

		//after pathValid and fileExisted for caching
		handler->sendToClient(data, size);
		
		if(handler->errorHappens) {
			switch(handler->errorHappens) {
				case 1:
					printf(RED "(%d) error in resolve Host Name.\n" NONE, s->sock);
					break;
				case 2:
					printf(RED "(%d) error in Create Socket when connect to real server.\n" NONE, s->sock);
					break;
				case 3:
					printf(RED "(%d) error in Connect to real server.\n" NONE, s->sock);
					break;
				case 4:
					printf(RED "(%d) error in Send data to real server.\n" NONE, s->sock);
					break;
				case 5:
					printf(RED "(%d) error in Recvive from real server.\n" NONE, s->sock);
					break;
				case 6:
					printf(RED "(%d) error in Send data to client.\n" NONE, s->sock);
					break;
			}
		}
		cout << "(" << s->sock << ")" << "***** After  Thread ***** " << host << file << endl;
		delete handler;
	}
}

int main(int argc, char *argv[])
{
	int i;
    // get port number
    if (argc != 2) {
        fprintf(stderr, "http proxy Usage:%s portnumber\n", argv[0]);
        exit(1);
    }
    if ((portnumber = atoi(argv[1])) < 0) {
        fprintf(stderr, "invalid portnumber\n");
        exit(1);
    }
    
	int listenfd = setUpV4TCPListenSocket(portnumber);
	int connfd;

	// initialize select
	for(i = 0; i < FD_SETSIZE; i++) {
		clients[i].sock = -1;
	}
	maxfd = listenfd;
	FD_ZERO(&allset);
	FD_SET(listenfd, &allset);

	// every request to "host name inside cfgFile" would be ignore.
	//string cfgFile("./filterURL.config");
	string cfgFile("filterURL.txt");
	FilterHelper filter(cfgFile);
	
	// allocate CacheManagement obj, used to manage all the cached file.
    cacheMem = CacheManagement::createCacheManagement();

	cout << "+++ HTTP Proxy have started +++" << endl;
	while(1) {
		cacheMem->showFiles();	// show all the files recorded in the cacheManagement.

		struct sockaddr_in addr;
		int sock_len;
		rset = allset;
		nready = select(maxfd + 1, &rset, NULL, NULL, NULL);

		cout << "Current Connection Cnt :" << sockCnt << "/" << FD_SETSIZE << endl;
		
		if(FD_ISSET(listenfd, &rset)) {
			// get a connection from new client.
			if((connfd = accept(listenfd, (struct sockaddr *)&addr, (socklen_t *) & sock_len)) == -1) {
				printf("accept error :%s\n", strerror(errno));
				continue;
			}
			
			for(i = 0; i < FD_SETSIZE; i++) {
				if(clients[i].sock < 0) {
					clients[i].sock = connfd;
					clients[i].vec = new vector<char>();
					cout << "------- here vec " << clients[i].vec << endl;
					sockCnt++;
					if(i == FD_SETSIZE)
						printf("too many clients\n");
					FD_SET(connfd, &allset);
					if(connfd > maxfd)
						maxfd = connfd;
					nready--;
					if(nready <= 0)
						break;
				}
			}
		}
cout << " new client allocated succesfully " << endl;
		for(i = 0; i < FD_SETSIZE; i++) {
			int sockfd = clients[i].sock;
			if(sockfd == -1)//essential
				continue;
			if(FD_ISSET(sockfd, &rset)) {
				int n;
					cout << "------- here2 vec " << clients[i].vec << endl;
				vector<char> *vec = clients[i].vec;
					cout << "------- here3 vec " << clients[i].vec << endl;
				pthread_t pid = clients[i].pid;


				//512 bytes tested , 1024 to be tested
				if((n = recv(sockfd, recvbuf, BUFF_SIZE, 0)) == 0) {
					//connection closed from client, set the status to inactive
					printf(" ==== recv from sockfd %d, recved len = %d ==== Client close this connection \n", sockfd, n);

					// when closing a sock, we need to delete vec in _sock_vec, remove _sock_vec from clients, clear sockfd from allset then close sock.
					sockCnt--;
					delete vec;
					clients[i].sock = -1;
//					pthread_cancel(pid);
					FD_CLR(sockfd, &allset);
					close(sockfd);
				}
				else if(n < 0) {
					printf("recv error : %s\n", strerror(errno));
				}
				else {
					// process msg;
					printf(" ==== recv from sockfd %d, recved len = %d  ==== \n", sockfd, n);

					vec->insert(vec->end(), recvbuf, recvbuf + n);
					auto it1 = search(vec->begin(), vec->end(), hdrEnd.begin(), hdrEnd.end());

					if(it1 != vec->end()) {	// found hdr end
						it1 += 4;
						vector<char> tmp_vec(vec->begin(), it1);
						vec->erase(vec->begin(), it1);

						HttpHdr *hdr = new HttpHdr(tmp_vec);
						if(!filter.isHostValid(hdr->getRequestHost())) {	// host name are in filter list
							cout << "host " << hdr->getRequestHost() << " are found in filter list." << endl;
							
							sockCnt--;
							delete hdr;
							delete vec;
							clients[i].sock = -1;
							FD_CLR(sockfd, &allset);
							close(sockfd);
						}
						else {	// host name are not in filter list.
							hdr->printHdrASC(sockfd);
							if(hdr->getHdrType() == HttpHdrType::GET) {
								hdr->removeHostPrefix();

								_sock_hdr tmp;
								tmp.sock = sockfd;
								tmp.hdr = hdr;
								
								if(pthread_create(&pid, NULL, doRequest, &tmp)) {
									printf("create thread error: %s\n");
								}
							}
							else {
								// output the Http Hdr content on screen
								cout << hdr->getHdrType() << endl;
								cout << "not GET request, will ignore this request. Press Enter." << endl;
								
								sockCnt--;
								delete hdr;
								delete vec;
								clients[i].sock = -1;
								FD_CLR(sockfd, &allset);
								close(sockfd);
							}
						}
					}
				}
				if(--nready <= 0)
					break;
			}
		}
	}

	close(listenfd);
	return 0;
}
