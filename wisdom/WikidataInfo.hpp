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



#ifndef __WD_INFO__
#define __WD_INFO__

#include<iostream>
#include<fstream>
#include<stdexcept>
#include<vector>
#include<algorithm>
#include<string>
#include<map>
//#include<forward_list>
#include<boost/unordered_map.hpp>
#include<cmath>
#include<boost/algorithm/string.hpp>
#include<boost/lexical_cast.hpp>
#include"../drt/metric_singleton.hpp"
#include"../drt/DrtPred.hpp"
//#include "../google-libs/cpp-btree-1.0.1/btree_map.h"

using std::vector;
using std::pair;
using std::make_pair;
using std::map;
using std::string;
using std::cout;
using std::endl;
//using std::forward_list;
using boost::unordered_map;


class WikidataInfo {
	vector<pair<DrtVect,DrtVect> > rules_;
	map<string,vector<string> >   forbidden_;
	map<string,string>   types_;

public:
	void loadRules(const string &str);
	void loadForbidden(const string &str);
	void loadTypes(const string &str);
	vector<pair<DrtVect,DrtVect> >  getRules()     {return rules_;    }
	map<string,vector<string> >  getForbidden() {return forbidden_;}
	map<string,string>  getTypes()     {return types_;    }	
};


#endif // __WD_INFO__
