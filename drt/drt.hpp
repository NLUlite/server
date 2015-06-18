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



#ifndef __DRT__
#define __DRT__

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
#include"../fopl/fopl.hpp"
#include"metric.hpp"
#include"DrtPred.hpp"
#include"metric_singleton.hpp"
#include"constituents.hpp"
#include"parser_singleton.hpp"
#include"../engine/Engine.hpp"

using std::string;
using std::vector;
using std::map;
using boost::tuple;
using boost::make_tuple;
using std::pair;
using std::make_pair;
using std::cout;
using std::endl;

class phrase;

class Priors {

	vector<string> priors_;

public:
	bool hasPrior(const string &str);
	void addNoun(const string &str);
	vector<string> getString();
};

class MatchSubstitutions;

class QuestionList {
     // Questions are linked to words. For example, in "what did you
     // eat?" the word "what" should be the result of the question

     vector< DrtPred > qlist_;
     // vector of predicates that must be answered (what, who, ...)

     Priors priors_;
     // words that cannot be written as an answer (they are already present in the question)

public:

     void add(DrtPred d);
     void add(const vector<DrtPred> &drtvect);     
     vector<DrtPred> get() const { return qlist_; }

     void operator / (const DrtMgu &upg);
     bool operator == (const QuestionList &rhs) const {return qlist_==rhs.qlist_; }

     void applySubstitutions(MatchSubstitutions &msubs);

     Priors getPriors() const {return priors_;}
};


class drt {
     string link_;
     string text_;

     vector<DrtPred> drt_;
     vector<pair<string,string> > references_;
     vector<string> levin_descriptions_;

     bool has_question_;
     bool has_condition_;
     bool has_code_;

     QuestionList qlist_;
     
     CodePred code_;
     
public:
     drt() {drt_=vector<DrtPred>(); has_question_=false; has_condition_=false; has_code_= false;}
     drt(const vector<DrtPred> &d) : drt_(d) {has_question_=false; has_condition_=false; has_code_= false;}
     void set_question() {has_question_=true;}
     void set_condition() {has_condition_=true;}
     bool has_question() const  {return has_question_;}
     bool has_condition() const {return has_condition_;}
     bool has_donkey() const;
     vector<string> get_donkey() const;
     
     void find_levin();
     vector<string> get_levin() {return levin_descriptions_;}

     void apply_phrase_number(int num);
     vector<pair<pair<string,string>,string> > get_references() const;
     vector<DrtPred> get_references_with_preds() const;
     vector<pair<string,string> > get_donkey_instantiation();

     void set_references(const vector<pair<string,string> > &r) {references_=r;}
     void add_references(const vector<pair<string,string> > &r) {references_.insert(references_.end(),r.begin(),r.end());}
     
     void setLink(const string &s) { link_= s;}
     string getLink() const { return link_;}

     void setText(const string &s) { text_= s;}
     string getText() const { return text_;}

     void cleanDrs();
     void clear() {;} /// clean all the elements

     vector<DrtPred> predicates() const {return drt_;}
     void setPredicates(const vector<DrtPred> &d) {drt_= d;}
     vector<DrtPred> predicates_with_references() const;

     void setQuestionList(const QuestionList &ql);
     QuestionList getQuestionList() { return qlist_; }

     void setCode(const CodePred &cp) {code_=cp; has_code_= true;}
     CodePred getCode() const {return code_;}
     bool hasCode() {return has_code_;}
};

#endif // __DRT__
