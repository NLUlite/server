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



#ifndef __PARSER_INFO__
#define __PARSER_INFO__

#include<iostream>
#include<fstream>
#include<stdexcept>
#include<vector>
#include<map>
#include<algorithm>
#include<string>
#include<cmath>
#include<boost/algorithm/string.hpp>
#include<boost/unordered_map.hpp>
#include"../infero/infero.hpp"
//#include "../google-libs/cpp-btree-1.0.1/btree_map.h"


using std::vector;
using std::map;
//using google::sparse_hash_map;
using boost::unordered_map;
using std::string;

#define AP 54059 /* a prime */
#define BP 76963 /* another prime */

class hash_str{
public:

     long operator() (const vector<string> &vs) const {
          string str = "";
          for (int n = 0; n < vs.size(); ++n) {
               str += vs.at(n) + " ";
          }

          unsigned h = 31 /* also prime */;
          for (int n = 0; n < str.size(); ++n) {
               h = (h * AP) ^ (str.at(n) * BP);
          }
          return h;
     }
};

class ClauseContainer {
     double w_;
     string clause_str_;
     double cons_info_, hyp_info_;
     
public:
     ClauseContainer(const string &);
     void setWeigth(double);
     void setConsequenceInfo(double);
     void setHypothesisInfo(double);
     
     Clause getClause();
};

//typedef unordered_map<vector<string>, vector<ClauseContainer>, hash_str > clauses_map;
typedef unordered_map<vector<string>, vector<ClauseContainer> > clauses_map;

class parser_info {
     clauses_map matching_feet;
     clauses_map matching_bulk;
     clauses_map matching_roots;
     vector<Clause> matching_corrections;

public:
     parser_info() {}
     ~parser_info() {}

     void load_feet_clauses(const char *f);
     void load_bulk_clauses(const char *f);
     void load_roots_clauses(const char *f);
     void load_matching_clauses(const char *f);

     clauses_map& get_feet_clauses_map()  { return matching_feet;  }
     clauses_map& get_bulk_clauses_map()  { return matching_bulk;  }
     clauses_map& get_roots_clauses_map() { return matching_roots; }
     vector<Clause> get_correction_clauses() { return matching_corrections; }
};

#endif // __PARSER_INFO__
