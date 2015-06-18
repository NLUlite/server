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



#ifndef __TAGGER_INFO__
#define __TAGGER_INFO__

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
#include "../google-libs/cpp-btree-1.0.1/btree_map.h"

using std::vector;
using std::pair;
using std::make_pair;
using std::map;
using std::string;
using std::cout;
using std::endl;
//using std::forward_list;
using boost::unordered_map;

typedef btree::btree_map<pair<string,string>, string> TMapPStSt;
typedef btree::btree_map<pair<string,string>, double> TMapPStDouble;
typedef btree::btree_map<pair<string,string>, int> TMapPStInt;
typedef btree::btree_map<pair<string, pair<string,string> >, int> TMapPInt;
typedef btree::btree_map<string,vector<string> > TMapStVSt;
typedef btree::btree_map<string,vector<int> > TMapStVInt;
typedef btree::btree_map<string,int > TMapStInt;
typedef btree::btree_map<int,string > TMapIntSt;
typedef btree::btree_map<string,string > TMapStSt;
typedef btree::btree_map<vector<string>,string > TMapVStSt;

class tagger_info
{
	TMapPStSt irregular_conj_;
	TMapPStDouble tag_freqs;
	TMapPStDouble tag_freqs_back;
	TMapPStDouble tag_freqs_prior;
	TMapPInt freqs;
	TMapPInt freqs_back;
	TMapPInt freqs_prior;
	TMapPStSt conjug;
	TMapStVSt conjug_base;
	TMapStInt tot_nums;
	TMapStInt tot_nums_back;
	TMapStInt tot_nums_prior;
	TMapStVSt conj_tag;
     vector<string> months_;
     vector<string> chrono_;
     TMapVStSt substitutions_;
     TMapStSt last_hope_tags_; // if the tagging fails set the tag to these ones.
     TMapStVSt candidate_tags_;
     TMapStSt sure_tags_;
     vector<string> except_;
     TMapStSt unique_words_;

//	unordered_map<vector<string>,vector<int> > q_values_;
	TMapStVInt q_values_;
	TMapIntSt q_names_;
	
public:
     tagger_info();
  
     void load_tag_frequencies(const char *); // bare tag freq
     void load_tag_frequencies_back(const char *); // bare tag freq
     void load_tag_frequencies_prior(const char *); // bare tag freq
     void load_frequencies(const char *);
     void load_frequencies_back(const char *);
     void load_frequencies_prior(const char *);
     void load_conjugations(const char *);
     void load_substitutions(const char *);
     void load_wikidata_names(const char *f);
	void load_wikidata_qs(const char *f);

     void complete_regular_verbs();
     double get_freq(const string &, const string &, const string &);
     double get_freq_back(const string &, const string &, const string &);
     double get_freq_prior(const string &, const string &, const string &);
     string get_conj(const string &, string);
     bool has_base(const string &str);

     bool is_modal_verb    (const string &str);
     bool is_auxiliary_verb(const string &str);
     bool is_candidate_verb(const string &str);
     bool is_candidate_name(const string &str);
     string unique_word(const string &word);

     vector<string> getMonths() { return months_; }
     vector<string> getChronoNames() { return chrono_; }
     TMapVStSt getSubstitutionsMap() { return substitutions_; }
     vector<string> getCandidateTags(const string &str);
     string getSureTag(const string &str) const;
     bool isSureTag(const string &str) const;
     
     double regular_tag(const string &word, const string &tag, string *base);     

     // double get_freq_and_adjust(const string &, string *, const string &);
     
     bool is_valid_general_tag(const string &tag);
     bool is_valid_verb_tag(const string &tag);

     double general_tag(const string &tag, const string &tag_prev);
     double general_tag_back(const string &tag, const string &tag_prev);
     double general_tag_prior(const string &tag, const string &tag_prev);
     
     string conjugate(string base, const string &tag);
     
     string get_base_superlative(string str);

     vector<int> find_wikidata_q(const string &key, const string &tag);
     string get_wikidata_name(int key);
};

#endif // __TAGGER_INFO__
