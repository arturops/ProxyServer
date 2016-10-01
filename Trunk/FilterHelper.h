
#ifndef _FILTER_HELPER_H_
#define _FILTER_HELPER_H_

#include <algorithm>
#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <regex>

using std::vector;
using std::string;
using std::cout;
using std::endl;
using std::istringstream;
using std::ifstream;
using std::find_if;
using std::regex;
using std::regex_match;
using std::smatch;

class FilterHelper {
	ifstream ifile;
	vector<string> filterList;
public:
	FilterHelper(string configFileName);
	
	bool isHostValid(string hostName);
};



#endif

