
#include "HttpHdr.h"

//	GET, POST, CONNECT, HEAD, PUT, DELETE, OPTIONS, TRACE, UNKNOWN

HttpHdrType HttpHdr::getHdrType()
{
	vector<char> line = {0x0d, 0x0a};//CRLF end of a header line of HTTP CRLF = "\r\n" = 0x0D,0x0A

	//look for end of the header
	auto it = search(hdr.begin(), hdr.end(), line.begin(), line.end());
	if(it == hdr.end()) {
		return HttpHdrType::UNKNOWN;
	}

	//create str with header contain
	string str(hdr.begin(), it), type;
	
	//split header -- into strings
	istringstream istrstm(str);
	istrstm >> type;//passes first string to type
	
	if(type == "GET")
		return HttpHdrType::GET;
	else if(type == "POST")
		return HttpHdrType::POST;
	else if(type == "PUT")
		return HttpHdrType::PUT;
	else if(type == "CONNECT")
		return HttpHdrType::CONNECT;
	else if(type == "HEAD")
		return HttpHdrType::HEAD;
	else if(type == "DELETE")
		return HttpHdrType::DELETE;
	else if(type == "TRACE")
		return HttpHdrType::TRACE;
	else if(type == "OPTIONS")
		return HttpHdrType::OPTIONS;
	else 
		return HttpHdrType::UNKNOWN;
}

bool HttpHdr::isHdrComplete()
{
	//look for doble endline of header to check if it is complete
	vector<char> headEnd = {0x0d, 0x0a, 0x0d, 0x0a};
	auto headerEnd = search(hdr.begin(), hdr.end(), headEnd.begin(), headEnd.end());
	if(headerEnd == hdr.end()) {	// HTTP header not complete
		return false;
	}
	return true;
}

void HttpHdr::insertContent(vector<char>::iterator begin, vector<char>::iterator end)
{
	//insert data at the end of hdr from a vector
	hdr.insert(hdr.end(), begin, end);
}

string HttpHdr::getRequestHost()
{
	string hostLine, url, hostname;
	
	//elements to use for the search
	vector<char> line = {0x0d, 0x0a};
	vector<char> hoststr = {'H', 'o', 's', 't', ':'};
	
	//search for word host and end of line
	auto hostBeg = search(hdr.begin(), hdr.end(), hoststr.begin(), hoststr.end());
	auto hostEnd = search(hostBeg, hdr.end(), line.begin(), line.end());
	
	//creates string of the Host line
	hostLine = string(hostBeg, hostEnd);
	istringstream istrstm(hostLine);
	istrstm >> url;//throws "Host:"
	istrstm >> hostname;//throws hostname
	
	return hostname;
}

string HttpHdr::getRequestFile()
{
	vector<char> line = {0x0d, 0x0a};
	string filename;
	auto firstLine = search(hdr.begin(), hdr.end(), line.begin(), line.end());
	string newstr(hdr.begin(), firstLine);
	istringstream istrstm(newstr);
	istrstm >> filename;//throw HTTP type
	istrstm >> filename;//throw object/file requested
	return filename;
}


void HttpHdr::removeHostPrefix()
{
	vector<char> httpBeg = {'h', 't', 't', 'p'};
	vector<char> splash = {'/'};//comment later

	auto itBeg = search(hdr.begin(), hdr.end(), httpBeg.begin(), httpBeg.end());
	auto itEnd = itBeg + 7;// 7 because of h,t,t,p,:,/,/ --> 7
	while(*itEnd != '/') itEnd++; //in case of https://
	hdr.erase(itBeg, itEnd);
}

void HttpHdr::printHdrHex()
{
	printf("\n\n*****************************\n\n");
	for(int i = 0; i < hdr.size(); i++) {
		if(i % 16 == 0)
			printf("\n");
		printf("%02x ", (unsigned char)hdr[i]);
	}
	printf("\n\n*****************************\n\n");
}

void HttpHdr::printHdrASC(int prefix)
{
	printf("\n(%d)*****************************\n", prefix);
	for(int i = 0; i < hdr.size(); i++) {
		printf("%c", (unsigned char)hdr[i]); //unsigned char 0-255 print ASCII
	}
	printf("\n(%d)*****************************\n", prefix);
}
