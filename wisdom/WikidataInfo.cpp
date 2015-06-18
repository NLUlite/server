/*
The MIT License (MIT)

Copyright (c) 2015 Alberto Cetoli (alberto.cetoli@nlulite.com)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

*/



#include"WikidataInfo.hpp"


void WikidataInfo::loadRules(const string &str)
{
	std::ifstream file;
	char article[10000];

	file.open(str.c_str() );
	if (!file.bad()) {
		std::cerr << "Loading wikidata rules from \'" << str << "\'." << std::endl;
		while (file.good()) {
			vector<string> data;
			file.getline(article, 10000);
			//boost::split(data, article, boost::is_any_of(":-"));
			string line(article);
			if(line.size() && line.at(0) == '#')
				continue;
			int pos = line.find(":-");
			if(pos == string::npos)
				continue;
			data.push_back(line.substr(0,pos));
			data.push_back(line.substr(pos+2,line.size() ));
			if (data.size() == 2) {
				rules_.push_back(make_pair(create_drtvect(data.at(0)),create_drtvect(data.at(1))) );
			}
		}
	} else
		throw std::length_error(std::string("File") + str + " finished unexpectedly.");
	file.close();
}

void WikidataInfo::loadForbidden(const string &str)
{
	std::ifstream file;
	char article[10000];
	vector<string> data;

	file.open(str.c_str());
	if (!file.bad()) {
		std::cerr << "Loading wikidata forbidden from \'" << str << "\'." << std::endl;
		while (file.good()) {
			file.getline(article, 10000);
			boost::split(data, article, boost::is_any_of(";"));
			if (data.size() == 2) {
				vector<string> tmpv;
				tmpv.push_back(data.at(1));
				forbidden_[data.at(0)] = tmpv;
			}
		}
	} else
		throw std::length_error(std::string("File") + str + " finished unexpectedly.");
	file.close();
}

void WikidataInfo::loadTypes(const string &str)
{
	std::ifstream file;
	char article[10000];
	vector<string> data;

	file.open(str.c_str());
	if (!file.bad()) {
		std::cerr << "Loading wikidata types from \'" << str << "\'." << std::endl;
		while (file.good()) {
			file.getline(article, 10000);
			boost::split(data, article, boost::is_any_of(","));
			if (data.size() == 2) {
				types_[data.at(0)] = data.at(1);
			}
		}
	} else
		throw std::length_error(std::string("File") + str + " finished unexpectedly.");
	file.close();
}


