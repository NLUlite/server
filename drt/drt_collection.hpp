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



#ifndef __DRT_COLL__
#define __DRT_COLL__

#include<iostream>
#include<algorithm>
#include<vector>
#include<cmath>
#include<map>
#include<string>
#include<fstream>
#include<boost/thread.hpp>
#include<boost/thread/mutex.hpp>
#include<boost/tuple/tuple.hpp>
#include<boost/utility.hpp>
#include<boost/algorithm/string.hpp>
#include<boost/lexical_cast.hpp>
#include"../infero/infero.hpp"
#include"../infero_vector/infero_vector.hpp"
#include"../match/Match.hpp"
#include"../match/Anaphora.hpp"
#include"../pragmatics/Presupposition.hpp"
#include"../context/Context.hpp"
#include"../nl/Filter.hpp"
#include"../aux/parameters_singleton.hpp"
#include"metric.hpp"
#include"metric_singleton.hpp"
#include"constituents.hpp"
#include"phrase.hpp"
#include"phrase_versions.hpp"
#include"drt.hpp"
#include"drs_anaphora_levels.hpp"
#include"DrtPred.hpp"
#include"DrsPersonae.hpp"
#include"complements.hpp"
#include"Rules.hpp"
#include"phrase_info.hpp"

using std::string;
using std::vector;
using std::map;
using boost::tuple;
using boost::make_tuple;
using std::pair;
using std::make_pair;

class CandidateQuestion {
	vector<drt> candidates_;
	vector<DrtPred> conjunctions_;

public:
	void add(const drt &d);
	void add(const DrtPred &p);

	void add(const vector<drt> &d);
	void add(const vector<DrtPred> &p);

	drt getFirstCandidate() const;
	vector<drt> getCandidates() const;
	vector<drt> getAllButFirstCandidates() const;

	vector<DrtPred> getConjunctions() const;
};

typedef vector<CandidateQuestion> QuestionVersions;

class drt_collection {
     string link_;
     int global_num_;

     vector<phrase_versions> phrase_collection_;
     vector<drt> drt_collection_;
     pair<vector<drt>,vector<DrtPred> > questions_;
     vector< QuestionVersions > candidate_questions_;
     vector<vector<pair<string,string> > > phrase_references_;
     
     DrsPersonae personae_;
     Rules rules_;
     
     Context context_;
     
     PhraseInfo *pi_;
     WisdomInfo wi_;

     void init(string &text);
     vector<drt> compute_data();
     pair<vector<drt>,vector<DrtPred> > compute_questions();
     vector< QuestionVersions > compute_candidate_questions();


public:
     drt_collection(string text, int global_num, Context c, PhraseInfo *pi, WisdomInfo wi= WisdomInfo() );
     drt_collection(string text, int global_num, Context c, PhraseInfo *pi, const string &link, WisdomInfo wi= WisdomInfo() );
     ~drt_collection();
     vector<phrase_versions> get_phrases() const {return phrase_collection_;}
     void add_phrase(const phrase_versions &pv);

     vector<drt> get_collection() const {return drt_collection_;}
     void set_collection(const vector<drt> &d) { drt_collection_= d;}

     void setLink(const string &str);
     string getLink() {return link_;}

     void connect_allocution_references();

     vector<drt> extract_data();
     vector<clause_vector> extract_clauses();
     pair<vector<drt>,vector<DrtPred> > extract_questions();
     vector< QuestionVersions > extract_candidate_questions();
     DrsPersonae getPersonae() {return personae_;}
     Rules getRules() {return rules_;}          
     
     Context getContext() {return context_;}

     void setWisdomInfo(WisdomInfo wi);
};

#endif // __DRT_COLL__
