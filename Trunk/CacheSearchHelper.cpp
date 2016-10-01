
#include "CacheSearchHelper.h"

CacheSearchHelper::CacheSearchHelper(string searchStr)
{
	// extract keyword & search Type from search Str.
	//QUERY_STRING ?keyword=....&type=....
	const char* path = searchStr.c_str();
	string key("keyword");
	int it0 = searchStr.find(key), it1;
	while(path[it0] != '=')//to find input keyword in the search.html
		it0++;
	it0++;
	it1 = it0;
	while(path[it1] != '&')
		it1++;
	keyword = string(&path[it0], &path[it1]);//create a string that contains the search keyword
	it0 = it1;
	while(path[it0] != '=') //now look for type
		it0++;
	it0++;
	it1 = it0;
	while(path[it1])
		it1++;
	searchType = string(&path[it0], &path[it1]);//create a string that contains the search type
//	cout << keyword << "," << searchType << endl;
}

static char htmlBeg[] = "<html>\n<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\" />\n<title>Search Result</title>\n<body>\n<h2>Search Results (";
static char htmlEnd[] = "</body>\n</html>";
static char h2end[] = ")</h2>";
static char urlStart[] = "<p><a href=\"";
static char urlEnd[] = "</a></p>";

void CacheSearchHelper::doSearch(CacheManagement* cacheMem)
{
	retContent = vector<char>(httpHdr, httpHdr + sizeof(httpHdr) - 1);//fill the vector with httpHdr[], -1 because string starts in 0 and sizeof gives +1 to the vector size
	vector<string> *vec;
	vector<char> pkt;
	
	pkt.insert(pkt.end(), htmlBeg, htmlBeg + sizeof(htmlBeg) - 1);//-1 because of sizeof
	
	// search by URL or by Content.
	if(searchType == "url") {
		vec = cacheMem->searchForURL(keyword);
	}
	else if(searchType == "content") {
		vec = cacheMem->searchForContent(keyword);
	}
	else {	// unknown search type
		return;
	}
	// as return file list are store in vector, we decode and create the http packet to reply.
	int len = vec->size();
	string strLen = to_string(len);
	//pkt is a vector<char>
	pkt.insert(pkt.end(), strLen.begin(), strLen.end());//to show how many results were found -- Result(#)
	pkt.insert(pkt.end(), h2end, h2end + sizeof(h2end) - 1);//close header html
	//we have the results of search in vec, so we create links now
	for(auto it = vec->begin(); it != vec->end(); it++) {
		pkt.insert(pkt.end(), urlStart, urlStart + sizeof(urlStart) - 1);//start of link
		string address(*it);//create address string 
		string url("http://" + address);//do the link
		
		pkt.insert(pkt.end(), url.begin(), url.end());//insert in the pkt (now we have header and links)
		pkt.push_back('\"');
		pkt.push_back('>');//close html for the hyperlink
		pkt.insert(pkt.end(), address.begin(), address.end());//give the name of the link
		pkt.insert(pkt.end(), urlEnd, urlEnd + sizeof(urlEnd) - 1);
	}
	pkt.insert(pkt.end(), htmlEnd, htmlEnd + sizeof(htmlEnd) - 1);//end html and complete the packet
	
	//to create chunks
	char chunkSize[16], endChunk[] = "\r\n0\r\n\r\n";
	sprintf(chunkSize, "%x\r\n", (unsigned int)pkt.size()); //every html result is a chunk
	
	//building HTTP RESPONSE TO SEARCH ENGINE
	//reContent created with HTTP hdr in constructor so add the rest here
	retContent.insert(retContent.end(), chunkSize, chunkSize + strlen(chunkSize));//telling number of chunks
	retContent.insert(retContent.end(), pkt.begin(), pkt.end());//putting the search result html code into retContent
	retContent.insert(retContent.end(), endChunk, endChunk + sizeof(endChunk) - 1);//putting a chunk size 0 at the end so TCP knows no more chunks come


	// after create http packet, all bytes are store in retContent. 
	// we could use that to reply to user.
	
	/*
	cout << retContent.size() << "%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%" << endl;
	for(auto s : retContent)
		cout << s;
	cout << endl;
	cout << "%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%" << endl;
	*/
	return;
}

