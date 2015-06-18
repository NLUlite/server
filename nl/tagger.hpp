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



#ifndef __TAGGER__
#define __TAGGER__

#include<iostream>
#include<fstream>
#include<stdexcept>
#include<vector>
#include<algorithm>
#include<iterator>
#include<string>
#include<cmath>
#include<map>
#include<boost/algorithm/string.hpp>
#include"../infero/infero.hpp"
#include"tagger_info.hpp"
#include"../drt/metric_singleton.hpp"
#include"NumeralCast.hpp"
#include"Filter.hpp"
#include"../context/Context.hpp"
#include"../wisdom/WisdomInfo.hpp"

using std::vector;
using std::string;

extern "C" {
     void init_ran(int iseed);	// Initialize     
     double dran();		// Double [0, 1)
}

class tagger
{
     string phrase;
     vector<string> words;
     vector<string> words_orig;
     vector<string> word_tags;
     vector<FuzzyPred> tagged_preds;
     vector<FuzzyPred> tagged_preds_simpl;
     tagger_info *info;
     vector<string> tags;
     vector<string> punctuation_tags;

     Context *context_;
     
     bool is_question_;

     WisdomInfo wi_;

     void fill_tags();
     void init_random_tags();
     void thermalize();
     vector< vector<string> > measure();
     void step(vector<bool> &mask);

     vector<string> post_process_tags( vector<string> tagged);
     vector<string> post_process_original(vector<string> tagged);
     vector<string> guess_missing_tags(vector<string> tagged);
     vector<FuzzyPred> convert_proper_names(vector<FuzzyPred>);
     bool isHiddenQuestion(const vector<string>&, const vector<string>&);
     std::pair<vector<FuzzyPred>,vector<FuzzyPred> > join_names(vector<FuzzyPred> tagged, vector<FuzzyPred> tagged_simple);
     vector<string> substitute_wikidata_qs(vector<string> tagged);

     
public:
     tagger(tagger_info *ti, WisdomInfo wi= WisdomInfo() );
  
     void set_phrase(string &str);
     void do_tag();
     vector<FuzzyPred> get_tagged_simple() { return tagged_preds_simpl; }
     vector<FuzzyPred> get_tagged() { return tagged_preds; }
     tagger_info* get_info() {return info;}
     
     void setContext(Context *c) {context_= c;}
     void setWisdomInfo(WisdomInfo wi) {wi_= wi;}
};

#endif // __TAGGER__
