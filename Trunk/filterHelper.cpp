
#include "FilterHelper.h"

FilterHelper::FilterHelper(string configFileName)
{
	char tmpHost[256];
	ifile.open(configFileName, std::ios::in);
	while(ifile.good()) {
		ifile.getline(tmpHost, 256);
		string t(tmpHost);
		// remove space before characters
		t.erase(t.begin(), find_if(t.begin(), t.end(), std::not1(std::ptr_fun(::isspace))));
		// remove space after characters
		t.erase(find_if(t.rbegin(), t.rend(), std::not1(std::ptr_fun(::isspace))).base(), t.end());
   		if(t.length() != 0)
			filterList.emplace_back(t);
	}
	
	cout << "Block List:" << endl;
	for(auto t : filterList) {
		cout << "[" << t << "]" << endl;
	}
}
	
bool FilterHelper::isHostValid(string hostName)
{
	hostName.erase(hostName.begin(), find_if(hostName.begin(), hostName.end(), std::not1(std::ptr_fun(::isspace))));
	hostName.erase(find_if(hostName.rbegin(), hostName.rend(), std::not1(std::ptr_fun(::isspace))).base(), hostName.end());

	vector<string>::iterator it;
	for(it = filterList.begin(); it != filterList.end(); it++) {
		regex e(*it);
		smatch sm;
		regex_match(hostName, sm, e);
//		cout << "[" << sm.size() << "]" << endl;
		if(sm.size() != 0) {
			return false;
		}
	}
	return true;
}

