
#include "HttpGetForwarder.h"

HttpGetForwarder::HttpGetForwarder(int sockfd, HttpHdr* httpGetRequest, CacheStorageHelper &helper) : hostname(httpGetRequest->getRequestHost()), requestType(httpGetRequest->getHdrType()), cacheHelper(helper)
{
	clientSockfd = sockfd;
	sendToURLBufferSize = httpGetRequest->hdr.size();
	sendToURLBuffer = new char[sendToURLBufferSize];
	copy(httpGetRequest->hdr.begin(), httpGetRequest->hdr.end(), sendToURLBuffer);

    cout << "(" << clientSockfd << ")" << "after constructur" << endl;
}

//connect to a server (proxy is client)
//passed pkt by reference (changes are preserved)
void HttpGetForwarder::sendToUrl(HttpPkt &pkt)
{
    struct hostent *host;//to obtain host address
	
    //use name of host to get struct hostent
    if((host=gethostbyname(hostname.c_str())) == NULL) {
        printf("(%d)gethostbyname error: %s\n", clientSockfd, strerror(errno));
        errorHappens = 1;
        return;
    }
	
    //open socket
    if((serverSockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        printf("(%d)socket error: %s\n", clientSockfd, strerror(errno));
        errorHappens = 2;
        return;
    }
    serv_addr.sin_family=AF_INET;
    serv_addr.sin_port=htons(80);//80 port number for HTTP
    serv_addr.sin_addr = *((struct in_addr *)host->h_addr);
    bzero(&(serv_addr.sin_zero),8);

    //use inet_ntoa(address network format) -> to address IPv4 format
    cout << "(" << clientSockfd << ")" << "connecting to " << inet_ntoa(serv_addr.sin_addr) << endl;
    if(connect(serverSockfd, (struct sockaddr *)&serv_addr, sizeof(struct sockaddr)) == -1) {
        printf("(%d)connect error: %s\n", clientSockfd, strerror(errno));
        errorHappens = 3;
        return;
    }
	printf("(%d)has connected to real server\n", clientSockfd);

	//send the request to server	
	int sendsize, cnt = 0;
	//sendToURLBufferSize is size of the header
	while(cnt < sendToURLBufferSize) {
	    if((sendsize = send(serverSockfd, sendToURLBuffer, sendToURLBufferSize, 0)) == -1) {
	        printf("(%d)send error: %s\n", clientSockfd, strerror(errno));
	        errorHappens = 4;
	        return;
	    }
	    cnt += sendsize;
	    printf("(%d)have send %d [%d] (%d)\n", clientSockfd, cnt, sendToURLBufferSize, sendsize);
	}
	
	//receiving data from server
	//moved from sendToClient after baseline
	char buf[MAXDATASIZE];
	int recvbytes;
	do {
	    if((recvbytes = recv(serverSockfd, buf, MAXDATASIZE, 0)) == -1) {
	        printf("(%d)recv err: %s\n", clientSockfd, strerror(errno));
	        errorHappens = 5;
	        return;
	    }
	    if(recvbytes != 0) {
		    vector<char> tmp(buf, buf + recvbytes);//creating a vector with the buffer data
		    //object pkt received is being modified here	
		    pkt.insertContent(tmp.begin(), tmp.end());
		    cout << "(" << clientSockfd << ")" << "get " << pkt.pkt.size() << " from " << hostname << endl;
		}
		else {
			cout << "(" << clientSockfd << ")" << "server closes the connection. Press Enter" << endl;
//			getchar();
//			pkt.printPkt();
			break;
		}
	} while(!pkt.isPktComplete());
}

void HttpGetForwarder::sendToClient(char* data, int size)
{
    /*
    printf("\n\n=========================\n\n");
	int ccnt = 0;
    for(int i = 0; i < size; i++) {
		printf("%02x ", (unsigned char)data[i]);
		if(ccnt == 15) {
			printf("\n");
			fflush(stdout);
		}
		ccnt = (ccnt + 1) % 16;
    }
    printf("\n\n=========================\n\n");
    */

	if(!errorHappens) {
		bool re = HttpGetForwarder::sendDataToClient(clientSockfd, data, size);
		if(!re)
			errorHappens = 6;
	}
}

bool HttpGetForwarder::sendDataToClient(int clientfd, char* data, int size)
{
    int sendcnt, cnt = 0;
	while(cnt < size) {
	    if((sendcnt = send(clientfd, data + cnt, size - cnt, 0)) == -1){ 
		//data+cnt to send the next chunk of the file (cnt works as offset on the file)
		//size-cnt to do not resend data over and over
	    	printf("(%d)send to browser error : %s \n", clientfd, strerror(errno));
	    	return false;
	    }
	    cnt += sendcnt;
	}
	cout << "(" << clientfd << ")" << "have send " << cnt << " to client" << endl;
	return true;
}
