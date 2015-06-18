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



#ifndef __PARSER__
#define __PARSER__

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
#include"tagger_info.hpp"
#include"tagger.hpp"
#include"parser_info.hpp"
//#include "../google-libs/cpp-btree-1.0.1/btree_map.h"

using std::vector;
using std::map;

//using google::sparse_hash_map;
using boost::unordered_map;
using std::string;

class parser {
     bool is_question_;
     tagger_info *info;
     parser_info *pinfo_;
     vector<FuzzyPred> tagged_, tagged_simple_;
     vector<int> noprn_, nosbar_;
     
     FuzzyPred parser_tree;
     vector<pair<FuzzyPred, double> > parsed_;
     
     vector<pair<vector<FuzzyPred>, Memory > > apply_censor(vector<pair<vector<FuzzyPred>, Memory > > data_double);
     
     vector<pair<vector<FuzzyPred>, Memory > > parse_all_subs(vector<FuzzyPred> data);
     vector<pair<vector<FuzzyPred>, Memory > > parse(vector<FuzzyPred> data);
     bool is_sbar_trigger(const FuzzyPred &trigger_pred, const vector<FuzzyPred> &preds, vector<FuzzyPred>::iterator ipos, int pos);
     bool is_closing_nested(vector<FuzzyPred>::iterator ipos, vector<FuzzyPred>::iterator iter_begin, vector<FuzzyPred> &preds, const FuzzyPred &open_item);
     vector<pair<string,vector<FuzzyPred> > > extract_nested(vector<FuzzyPred> *data, int *num_prn);
     string find_lemma_of_verb(const string &name);

public:
     parser(tagger_info *ti, parser_info *pinfo) : info(ti), pinfo_(pinfo) {}
     ~parser();

     FuzzyPred get_parser_tree() {return parser_tree; }
     void load_feet_clauses(const char *f);
     void load_bulk_clauses(const char *f);
     void load_roots_clauses(const char *f);

     void setTags(const vector<FuzzyPred> &tags);
     void setForbiddenPRN(const vector<int> &noprn);
     void setForbiddenSBAR(const vector<int> &noprn);

     //void load_markov_tags(const char *f);
     void do_parse();
     vector<pair<FuzzyPred, double> > get_parsed() {return parsed_;}
     void clear();
};

#endif // __PARSER__
