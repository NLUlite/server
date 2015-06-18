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



#ifndef __TAGGER_AUX__
#define __TAGGER_AUX__

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

bool aux_is_transitive(const string &str);
bool aux_is_verbatim(const string &word);
bool aux_is_adjective(const string &tag);
bool aux_is_conj(const string &tag);
bool aux_is_article(const string &tag);
bool aux_is_preposition(const string &tag);
bool aux_is_adverb(const string &tag);
bool aux_is_CD(const string &tag);
bool aux_is_PRP(const string &tag);
bool aux_is_verb(const string &tag);
bool aux_is_noun(const string &tag);
bool aux_is_proper_noun(const string &tag);
bool aux_is_plural(const string &tag);
bool aux_is_subject_PRP(const string &str);
bool aux_is_someone(const string &tag);
bool aux_is_something(const string &tag);
bool aux_is_date(const string &str);
bool aux_is_place(const string &str);
bool aux_verb_supports_indirect_obj(const string &name);
bool has_number(string s);
bool aux_is_cardinal_word(const string &word);

void aux_post_process_original(vector<string> words, vector<string> tagged, tagger_info *info, bool is_question_);


#endif // __TAGGER_AUX__
