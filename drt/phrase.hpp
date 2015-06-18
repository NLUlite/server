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



#ifndef __PHRASE__
#define __PHRASE__

#include<iostream>
#include<algorithm>
#include<vector>
#include<cmath>
#include<map>
#include<string>
#include<fstream>
#include<boost/tuple/tuple.hpp>
#include<boost/utility.hpp>
#include<boost/algorithm/string.hpp>
#include<boost/lexical_cast.hpp>
#include"../infero/infero.hpp"
#include"metric.hpp"
#include"constituents.hpp"
#include"drt_builder.hpp"
#include"drt.hpp"
#include"phrase_info.hpp"

using std::string;
using std::vector;
using std::map;
using boost::tuple;
using boost::make_tuple;
using std::pair;
using std::make_pair;

class composition;

class phrase {
     PredTree parsed;
     vector<string> tags_;
     vector<string> names_;
     PhraseInfo *phrase_info_;
     string base_text_;
     DrtPred error_;
     vector<int> heights_;
     vector<int> prn_depths_;
     DrtVect drt_form_, orig_drt_form_;
     double likeliness_;
     vector<pair<pair<int,int>, constituents> > connections_;
     composition composition_;
     vector<constituents> constit_;

     QuestionList qlist_;

     bool has_question_;
     bool has_condition_;

     // void drt_add_verbs(const vector<int> &pos);
     // void drt_find_predicates();
     // void drt_add_predicates(const vector<PredTree::iterator> &verb_iter);
     vector<FuzzyPred> get_drt_form(const vector<pair<int,int> > &connections);

public:
     phrase(const FuzzyPred &pred, PhraseInfo *pi);

     void compute_names(const FuzzyPred &pred);
     void restore_names(const FuzzyPred &pred);
     void compute_prn_depths(const FuzzyPred &pred);
     void compute_tags(const FuzzyPred &pred);
     void compute_constit(const composition &comp);
     vector<string> get_tags() const {return tags_;}
     vector<string> get_names() const {return names_;}
     vector<FuzzyPred> get_tag_preds() const;
     string get_text() const {return base_text_;}
     vector<pair<pair<int,int>,constituents> > get_connections() const {return connections_;}
     // void compute_heights(const FuzzyPred &pred);
     DrtVect get_drt() const { return drt_form_; }
     DrtVect get_orig_drt() const { return orig_drt_form_; }
     composition get_composition() {return composition_;}
     vector<constituents> get_constit() {return constit_;}
     int get_num_elements() const;
     double get_likeliness() const {return likeliness_;}
     void set_likeliness(double l) {likeliness_= l;}

     void set_question() {has_question_=true;}
     bool has_question() const  {return has_question_;}
     void set_condition() {has_condition_=true;}
     bool has_condition() const  {return has_condition_;}

     void setQuestionList(const QuestionList &ql) { qlist_= ql; }
     QuestionList getQuestionList() { return qlist_; }

     FuzzyPred getParsed() const {return FuzzyPred(parsed);}
     
     PhraseInfo* getInfo();

     DrtPred getError() const;

     vector<int> get_prn_depths() const;

     bool operator == (const phrase &rhs) const;
};


#endif // __PHRASE__
