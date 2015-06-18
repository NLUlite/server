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



#ifndef __MATCH__
#define __MATCH__

#include<iostream>
#include<set>
#include"boost/shared_ptr.hpp"
#include"../drt/drt.hpp"
#include"../drt/DrtPred.hpp"
#include"../drt/DrsPersonae.hpp"
#include"../drt/metric.hpp"
#include"../drt/metric_singleton.hpp"
#include"../knowledge/Knowledge.hpp"

using std::string;
using std::vector;
using std::map;
using std::set;
using boost::tuple;
using boost::make_tuple;
using std::pair;
using std::make_pair;

class MatchElements {
// This class keeps memory of the elements that have previously been matched
// to avoid a double match

	set<string> elements_;
//	set<DrtPred> elements_;
	//vector<DrtPred> elements_;

public:
	bool hasElement(const string &pred) const;
	void insertPrevious(const vector<string> &pred);
	void insertPrevious(const string &pred);
	void clear();
};


class MatchSubstitutions {
     DrtMgu upg_;
     vector<pair<string,string> > substitutions_; // the substitutions in the head of the drs predicates
     int num_matches_;

     string findHeader(const string &str);

public:
     MatchSubstitutions();
     ~MatchSubstitutions();
     void setDrtMgu(const DrtMgu &u) { upg_= u; }
     DrtMgu getDrtMgu() { return upg_; }
     
     void addSubstitution(const string &f, const string &s);
     void addSubstitution(const vector<pair<string,string> > &subs);
     
     vector<pair<string,string> > getSubstitutions() { return substitutions_; }
     void setSubstitutions(const vector<pair<string,string> > &rsub) { substitutions_= rsub; }
     void applySubstitutions(DrtVect *d);
     int getNumberOfMatches() { return num_matches_; }
     
     void increase() { ++num_matches_; }
          
     void operator / (const DrtMgu &u) { upg_/u ; }
};


class MatchType {
     
     bool is_equal_;
     bool is_more_;
     bool is_less_;

     bool is_time_;
     bool is_space_;
          
public:
     MatchType();
     ~MatchType();

     void set_equal() { is_equal_= true ; is_more_= false; is_less_= false; }
     void set_more()  { is_equal_= false; is_more_= true ; is_less_= false; }
     void set_less()  { is_equal_= false; is_more_= false; is_less_= true ; }

     bool is_equal() { return is_equal_; }
     bool is_more()  { return is_more_ ; }
     bool is_less()  { return is_less_ ; }

     void set_time()   { is_time_= true; }
     void unset_time() { is_time_= false; }
     bool is_time()  { return is_time_ ; }


     bool matchComplement(const DrtPred &lhs, const DrtPred &rhs);
     bool matchDate(const DrtPred &lhs, const DrtPred &rhs);          
     bool matchNumber(const DrtPred &lhs, const DrtPred &rhs);

};


class SingleMatchGraph {

     DrtPred verb_;
     vector<DrtPred> subject_;
     vector<DrtPred> object_;
     vector<DrtPred> adverb_;
     DrtPred pointer_;
     
     vector< DrtVect > complements_;
     vector< DrtVect > specifications_;
     // complements and specifications are stored as a vector of predicates

     bool has_verb_;
     bool is_generic_;
     
     
     
public:
     
     SingleMatchGraph() : is_generic_(false) {}
     
     //SingleMatchGraph(const vector<DrtPred> &drs);
     
     //vector<pair<string,vector<string> > > getComplementPairs(const string &str) const;

     vector<DrtPred> getSubject() const {return subject_;}
     vector<DrtPred> getObject() const {return object_;}
     vector<DrtPred> getAdverb() const {return adverb_;}
     DrtPred getPointer() const {return pointer_;}
     DrtPred getVerb() const {return verb_;}
     vector<DrtVect> getComplements(DrtMgu upg = DrtMgu()) const;

     void setSubject(const vector<DrtPred> &s) {subject_= s;}
     void setObject(const vector<DrtPred> &o) {object_= o;}
     void setAdverb(const vector<DrtPred> &a) {adverb_= a;}
     void setVerb(const DrtPred &v) {verb_= v;}
     void setPointer(const DrtPred &p) {pointer_= p;}
     void setComplements(const vector<DrtVect > &c) {complements_= c;}
     void addComplements(const vector<DrtVect > &c) {complements_.insert(complements_.end(),c.begin(),c.end());}

     bool hasVerb() const {return has_verb_;}
     void setVerb(bool hv=true) {has_verb_= hv;}
     
     void setGeneric(bool gen=true) {is_generic_= gen;}
     bool isGeneric() {return is_generic_;}

     DrtVect getDrs() const;
     
     void print(std::ostream &out);
     
     void operator/(const DrtMgu &upg);
};


class Knowledge;

class MatchGraph {
// This class contain the information for matching sentences. It
// is understood that the "drs" in input is just one sentence. 

     vector<DrtPred> verb_list_;
     vector<SingleMatchGraph> sentences_;
         
     DrtPred verb_;
     vector<DrtPred> subject_;
     vector<DrtPred> object_;

     vector< vector<DrtPred> > complements_;
     vector< vector<DrtPred> > specifications_;
     vector<DrtPred> quantifiers_;
     
     // complements and specifications are stored as a vector of predicates

     vector<DrtPred> subs_;
     // "subs_" stores the subordinates predicates like "@SUB_OBJ"

     vector<DrtPred> drs_form_;

     bool has_verb_;

     vector<DrtMgu> upg_forward_, upg_backward_;
          
public:
     MatchGraph(const vector<DrtPred> &drs);
     
     vector<pair<string,vector<string> > > getComplementPairs(const string &str) const;

     vector<DrtPred> getSubject() const {return subject_;}
     vector<DrtPred> getObject() const {return object_;}
     DrtPred getVerb() const {return verb_;}
     vector<vector<DrtPred> > getComplements() const {return complements_;}
     vector<vector<DrtPred> > getSpecifications() const {return specifications_;}
     vector<vector<DrtPred> > getSpecifications(const DrtPred &pred) const;
          
     vector<DrtPred > getQuantifiers() const {return quantifiers_;}
     void addQuantifier(const DrtPred &dp) {quantifiers_.push_back(dp);}
     

     vector<DrtPred> getSubs() const {return subs_;}
     vector<SingleMatchGraph> getSentences() const {return sentences_;}
     void setSentences(const vector<SingleMatchGraph> &s) {sentences_= s;}

     bool hasVerb() const {return has_verb_;}
    
     void additionalSpec(Knowledge *k);
     
     
     void computeDrtMguForward();
     void computeDrtMguBackward();
     DrtMgu getDrtMguBackward();
     
     void print(std::ostream &out);     
     vector<DrtPred> getDrs() const { return drs_form_;}
};

class MatchWrite {
// This is just a rewrite of MatchGraph specifically for the writer

     vector<DrtPred> verb_list_;
     vector<SingleMatchGraph> sentences_;
         
     DrtPred verb_;
     vector<DrtPred> subject_;
     vector<DrtPred> object_;

     vector< vector<DrtPred> > complements_;
     vector< vector<DrtPred> > specifications_;
     // complements and specifications are stored as a vector of predicates

     vector<DrtPred> subs_;
     // "subs_" stores the subordinates predicates like "@SUB_OBJ"

     vector<DrtPred> drs_form_;

     bool has_verb_;

     vector<DrtMgu> upg_forward_, upg_backward_;

public:
     MatchWrite(const vector<DrtPred> &drs);
     
     vector<pair<string,vector<string> > > getComplementPairs(const string &str) const;

     vector<DrtPred> getSubject() const {return subject_;}
     vector<DrtPred> getObject() const {return object_;}
     DrtPred getVerb() const {return verb_;}
     vector<vector<DrtPred> > getComplements() const {return complements_;}
     vector<vector<DrtPred> > getSpecifications() const {return specifications_;}
     vector<vector<DrtPred> > getSpecifications(const DrtPred &pred) const;

     vector<DrtPred> getSubs() const {return subs_;}
     vector<SingleMatchGraph> getSentences() const {return sentences_;}
     void setSentences(const vector<SingleMatchGraph> &s) {sentences_= s;}

     bool hasVerb() const {return has_verb_;}
         
     void print(std::ostream &out);     
     vector<DrtPred> getDrs() const { return drs_form_;}
};

class MatchInfo {
	bool only_names_, match_negative_, inverted_person_;
	int hypernym_height_;

public:
	MatchInfo();
	bool getOnlyNames     ();
	bool getMatchNegative ();
	int  getHypernymHeight();
	bool getInvertedPerson();

	void setOnlyNames     (bool);
	void setMatchNegative (bool);
	void setHypernymHeight(int);
	void setInvertedPerson(bool);
};


class Match {
     metric *d_;

     double phraseMatch(vector<DrtPred> &lhs, vector<DrtPred> &rhs, MatchSubstitutions *msubs, DrtMgu negated_upg= DrtMgu());
     double phraseMatchNames(vector<DrtPred> &lhs, vector<DrtPred> &rhs, MatchSubstitutions *msubs, DrtMgu negated_upg= DrtMgu());
     double matchQuantifiers(DrtPred lp, DrtPred rp, MatchSubstitutions *msubs);
     double adverbMatch(const vector<DrtPred> &ladv, const vector<DrtPred> &radv);

     Knowledge *k_;
     bool match_negative_;
     bool inverted_person_;
     int hyp_height_;
     MatchElements lme_, rme_;

public:
     Match( Knowledge *k );

     double singlePhraseMatch(vector<DrtPred> lhs, vector<DrtPred> &rhs, MatchSubstitutions *msubs, bool only_names= false, bool match_negative= true);
     double singlePhraseMatch(vector<DrtPred> lhs, vector<DrtPred> &rhs, MatchSubstitutions *msubs, MatchInfo mi);
     double singleMatch(const DrtPred &data, const DrtPred &hyp, MatchSubstitutions *msubs, MatchType *mtype);	  
     double singleGraphMatch(const SingleMatchGraph  &lhs, const SingleMatchGraph &hyp, MatchSubstitutions *msubs);
     
     double graphMatch(const MatchGraph &lhs, const MatchGraph &hyp, MatchSubstitutions *msubs);
     double graphMatchNames(const MatchGraph  &lhs, const MatchGraph &hyp, MatchSubstitutions *msubs, DrtMgu negated_upg= DrtMgu());
     double singleGraphMatchNames(const SingleMatchGraph &lhs, const SingleMatchGraph &hyp, MatchSubstitutions *msubs);
     
     vector<string> getAlternativeVerbs(const vector<string> &subj, const string &verb, const vector<string> &obj);
	DrtVect substituteAllWithRule(DrtVect drtvect, const pair<DrtVect,DrtVect> &rule);

};


double get_string_vector_distance(metric *d, const string &head_str, const string &rhs_str, Knowledge *k, int max_sep, string *return_name=0, string *rreturn_name=0);
string get_what();





namespace matching {
     bool is_what(const string &str);
     bool is_who(const string &str);
}



#endif // __MATCH__
