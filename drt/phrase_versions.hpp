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



#ifndef __PHRASE_VERSIONS__
#define __PHRASE_VERSIONS__

#include<iostream>
#include<algorithm>
#include<vector>
#include<map>
#include<string>
#include<fstream>
#include<boost/tuple/tuple.hpp>
#include<boost/utility.hpp>
#include<boost/algorithm/string.hpp>
#include<boost/lexical_cast.hpp>
#include<boost/tokenizer.hpp>
#include"../infero/infero.hpp"
#include"../wisdom/WisdomInfo.hpp"
#include"../nl/parser_include.hpp"
#include"../context/Context.hpp"
#include"../engine/Engine.hpp"
#include"Triggers.hpp"
#include"parser_singleton.hpp"
#include"metric_singleton.hpp"
#include"metric.hpp"
#include"phrase.hpp"
#include"phrase_info.hpp"
#include"DrtPred.hpp"

using std::string;
using std::vector;
using std::map;
using boost::tuple;
using boost::make_tuple;
using std::pair;
using std::make_pair;
using std::cout;
using std::endl;


class phrase_versions {     
     Context *context_;
     
     vector<pair<FuzzyPred, double> > parsed_versions_;
     vector<pair<phrase, double> > phrase_versions_;
     
     string text_;          
     PhraseInfo *phrase_info_;
     WisdomInfo wi_;

public:     
     phrase_versions();
     phrase_versions(const string &str, PhraseInfo *pi, Context *c=0, WisdomInfo wi= WisdomInfo() );
     vector<DrtPred> get_most_likely_drt() const;
     phrase get_most_likely_phrase() const;
     string get_text() const {return text_;}
     vector<pair<phrase, double> > get_phrases_with_weight() {return phrase_versions_;}
     
     void setContext(Context *c) {context_ = c;}
     int num_phrases() {return parsed_versions_.size();}
};

#endif // __PHRASE_VERSIONS__
