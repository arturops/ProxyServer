
#include "HttpPkt.h"


void HttpPkt::insertContent(vector<char>::iterator begin, vector<char>::iterator end)
{
	pkt.insert(pkt.end(), begin, end);
}

//content
bool HttpPkt::isPktComplete()
{
	vector<char> headEnd = {0x0d, 0x0a, 0x0d, 0x0a};
	vector<char> lineEnd = {0x0d, 0x0a};
	string contentLengthStr = "Content-Length:"; //using Content-Length field to find the size of the HTTP 1.1 response
//	string transferEncoding = "Transfer-Encoding: chunked";
	
	auto headerEnd = search(pkt.begin(), pkt.end(), headEnd.begin(), headEnd.end());
	if(headerEnd == pkt.end()) {	// HTTP header not complete
		return false;
	}
	else {	// header complete
		headerEnd += 4;	// move after "\r\n\r\n"
		if(transferCodingType == TransferCoding::TC_UNKNOWN) {
			//look for the word Content-Length
			auto generalCodingBeg = search(pkt.begin(), headerEnd, contentLengthStr.begin(), contentLengthStr.end());
			if(generalCodingBeg != headerEnd) {
				transferCodingType = TransferCoding::GENERAL;
				auto generalCodingEnd = search(generalCodingBeg, headerEnd, lineEnd.begin(), lineEnd.end());
				string newstr(generalCodingBeg, generalCodingEnd), tmp1;
				istringstream istrstm(newstr);
				istrstm >> tmp1 >> contentLength;
				cout << "\tTransfer in Gerenal coding, length: " << contentLength << ". Total: " << contentLength + (headerEnd - pkt.begin()) << endl;
			}
			else {
				transferCodingType = TransferCoding::CHUNKED;
				cout << "\tTransfer in Chunked coding."<< endl;
			}
		}
		
		if(transferCodingType == TransferCoding::GENERAL) {
			if(contentLength + (headerEnd - pkt.begin()) > pkt.size()) {
				return false;
			}
			return true;
		}
		else if(transferCodingType == TransferCoding::CHUNKED) {
			int chunkSize, dataSize = 0;
			auto chunkBeg = headerEnd;
			auto sizeBeg = headerEnd, sizeEnd = headerEnd;	// chunk block size begin & chunk block size end;
			do {
				sizeEnd = search(chunkBeg, pkt.end(), lineEnd.begin(), lineEnd.end());
				if(sizeEnd != pkt.end()) {
					sizeEnd += 2;//2 because Content-Length: bytes \r\n
					string str(sizeBeg, sizeEnd);
					istringstream istrstm(str);
					istrstm.setf(std::ios::hex, std::ios::basefield);	// size express in ascii as hex.
					//in hex to be able to add it to the sizeEnd iterator()
					istrstm >> chunkSize;
					chunkBeg = sizeEnd + chunkSize + 2;
					if(chunkBeg >= pkt.end() && chunkSize != 0) {
						// chunk block not end
						return false;
					}
					else {
						// current chunk block has ended.
						sizeBeg = chunkBeg;
						dataSize += chunkSize;
						contentLength = (contentLength < dataSize) ? dataSize : contentLength;
					}
				}
				else {
					return false;
				}
			} while(chunkSize != 0);
			return true;
		}
	}
}

void HttpPkt::printPkt()
{
	printf("\n\n*****************************\n\n");
	for(int i = 0; i < pkt.size(); i++) {
		if(i % 16 == 0)
			printf("\n");
		printf("%02x ", (unsigned char)pkt[i]);
	}
	printf("\n\n*****************************\n\n");
}

//method to get the header
HttpHdr* HttpPkt::getHdr()
{
	vector<char> headEnd = {0x0d, 0x0a, 0x0d, 0x0a};
	auto headerEnd = search(pkt.begin(), pkt.end(), headEnd.begin(), headEnd.end());
	if(headerEnd == pkt.end()) {	// HTTP header not complete
		return nullptr;
	}
	vector<char> vec(pkt.begin(), headerEnd + 4);
	return new HttpHdr(vec);
}

//get coding type
TransferCoding HttpPkt::getCodingType()
{
	return transferCodingType;
}
