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



#ifndef __KNOWLEDGE__
#define __KNOWLEDGE__

#include<iostream>
#include<algorithm>
#include<vector>
#include<cmath>
#include<map>
#include<string>
#include<fstream>
#include<stdexcept>
#include<functional>
#include<boost/tuple/tuple.hpp>
#include<boost/utility.hpp>
#include<boost/algorithm/string.hpp>
#include<boost/lexical_cast.hpp>
#include"../match/Match.hpp"
#include"../drt/metric.hpp"
#include"../drt/metric_singleton.hpp"
#include"../drt/drt.hpp"
#include"../drt/Rules.hpp"
#include"../drt/DrtPred.hpp"
#include"../drt/DrsPersonae.hpp"
#include"../infero_vector/path_memory.hpp"
#include"../engine/Engine.hpp"
#include"../wisdom/WisdomInfo.hpp"
#include"../aux/parameters_singleton.hpp"
//#include<boost/unordered_map.hpp>
//#include<google/sparse_hash_map>
#include "../google-libs/cpp-btree-1.0.1/btree_map.h"

using boost::unordered_map;

using std::string;
using std::vector;
using std::map;
//using boost::tuple;
//using boost::make_tuple;
using std::pair;
using std::make_pair;

class path_memory;

typedef boost::tuple<vector<DrtPred>, clause_vector, string, string> RulesCandidate;

class KnowledgeAnswer {
     // This class is to communicate the answer to "Wisdom"
     vector<DrtPred> preds_;
     clause_vector clause_;
     string link_;
     string text_;
     double weigth_;
     DrtMgu upg_;
     path_memory mem_;
     QuestionList qlist_;

public:

     vector<DrtPred> getPreds() const { return preds_; }
     clause_vector getClause() const { return clause_; }
     string getLink() const { return link_; }
     string getText() const { return text_; }
     double getWeigth() const { return weigth_; }
     DrtMgu getDrtMgu() const { return upg_; }
     path_memory getMemory() const { return mem_; }

     void setPreds(const vector<DrtPred> &p) { preds_= p; }
     void setClause(const clause_vector &cv) { clause_= cv; }
     void setLink(const string &str) { link_= str; }
     void setText(const string &str) { text_= str; }
     void setWeigth(double w) { weigth_= w; }
     void setDrtMgu(const DrtMgu &u) { upg_= u; }
     void setMemory(const path_memory &m) { mem_= m; }

     void setQuestionList(const QuestionList &ql) { qlist_= ql; }
     QuestionList getQuestionList() const { return qlist_; }

     bool operator==(const KnowledgeAnswer &rhs) const;
     void operator/(const DrtMgu &mgu);
     void use_mgu_prior(const DrtMgu &mgu);
};

class AnswerContainer {
     vector<KnowledgeAnswer> answers_;

public:
     AnswerContainer(const KnowledgeAnswer &ka) { answers_.push_back(ka); }
     AnswerContainer(const AnswerContainer &ac) { answers_= ac.answers_;  }

     vector<KnowledgeAnswer> getAnswers() const { return answers_; }
     void add(const KnowledgeAnswer &ka) { answers_.push_back(ka); }
     void add(const AnswerContainer &ka) { answers_.insert(answers_.end(), ka.answers_.begin(), ka.answers_.end()); }
     KnowledgeAnswer at(int pos) const {return answers_.at(pos); }
     KnowledgeAnswer& at(int pos) {return answers_.at(pos); }
     int size() const { return answers_.size(); }
};

class RulesCandidatesStruct {
	vector<string> verb_refs_,
	subj_refs_,
	obj_refs_,
	name_refs_, // any noun other than subject and object
	compl_refs_;

	bool lacks_subj_, // the question has no subject
		lacks_obj_,  // the question has no object
		lacks_verb_; // the question has no verb

public:
	RulesCandidatesStruct();

	void addVerbRefs (const vector<string> &refs);
	void addSubjRefs (const vector<string> &refs);
	void addObjRefs  (const vector<string> &refs);
	void addNameRefs (const vector<string> &refs);
	void addComplRefs(const vector<string> &refs);

	void setLacksSubj(bool lacks);
	void setLacksObj (bool lacks);
	void setLacksVerb(bool lacks);

	vector<string> getVerbRefs () const { return verb_refs_;}
	vector<string> getSubjRefs () const { return subj_refs_;}
	vector<string> getObjRefs  () const { return  obj_refs_;}
	vector<string> getNameRefs () const { return name_refs_;}
	vector<string> getComplRefs() const { return compl_refs_;}


	bool getLacksSubj() const { return  lacks_subj_;}
	bool getLacksObj () const { return  lacks_obj_;}
	bool getLacksVerb() const { return  lacks_verb_;}

	void sort();
};


class CandidatesStruct {
	vector<SPAction> verb_refs_,
	subj_refs_,
	obj_refs_,
	name_refs_, // any noun other than subject and object
	compl_refs_;

	WisdomInfo wi_;

	vector< vector<SPAction> >
		all_subj_refs_,
		all_obj_refs_,
		all_name_refs_,
		all_compl_refs_;

	bool lacks_subj_, // the question has no subject
		lacks_obj_,  // the question has no object
		lacks_verb_; // the question has no verb

public:
	CandidatesStruct(WisdomInfo wi);

	void addNewSubjRefs (const vector<SPAction> &refs, bool is_new);
	void addNewObjRefs  (const vector<SPAction> &refs, bool is_new);
	void addNewNameRefs (const vector<SPAction> &refs, bool is_new);
	void addNewComplRefs(const vector<SPAction> &refs, bool is_new);

	void addVerbRefs (const vector<SPAction> &refs);
	void addSubjRefs (const vector<SPAction> &refs);
	void addObjRefs  (const vector<SPAction> &refs);
	void addNameRefs (const vector<SPAction> &refs);
	void addComplRefs(const vector<SPAction> &refs);

	void setLacksSubj(bool lacks);
	void setLacksObj (bool lacks);
	void setLacksVerb(bool lacks);

	vector<SPAction> getVerbRefs () const { return verb_refs_;}
	vector<SPAction> getSubjRefs () const { return subj_refs_;}
	vector<SPAction> getObjRefs  () const { return  obj_refs_;}
	vector<SPAction> getNameRefs () const { return name_refs_;}
	vector<SPAction> getComplRefs() const { return compl_refs_;}

	bool getLacksSubj() const { return  lacks_subj_;}
	bool getLacksObj () const { return  lacks_obj_;}
	bool getLacksVerb() const { return  lacks_verb_;}

	void computeIntersection();

	void sort();
};

//typedef boost::shared_ptr<Action> PAction;

class MatchSubstitutions;

typedef boost::tuple<MatchSubstitutions,double,string,string,CodePred> KResult; 
typedef boost::tuple<clause_vector, MatchSubstitutions, double, string, string> KRuleResult;
typedef boost::tuple<vector<DrtPred>, string, string, CodePred> Candidate;

//typedef unordered_map<string, vector<string> > MapStVSt;

typedef pair<SPAction,string> ActionName;
typedef unordered_map<ActionName, double > MapRNDouble;
//typedef btree::btree_map<ActionName, double > MapRNDouble;
typedef pair<string,string> RefAndName;
typedef unordered_map<RefAndName, double > RulesMapRNDouble;



typedef unordered_map<string, string > MapStSt;
//typedef unordered_map<string, vector<SPAction> > MapStVSt;
typedef btree::btree_map<string, vector<SPAction> > MapStVSt;
//typedef unordered_map<string, vector<string> > MapStVSt;
//typedef google::sparse_hash_map<string, vector<SPAction>, std::tr1::hash<string> > MapStVSt;
typedef unordered_map<string, vector<string> > RulesMapStVSt;


class Knowledge {
	WisdomInfo wi_;

     DrsPersonae temp_personae_;
     DrsPersonae personae_;
     Rules rules_;
     Engine engine_;

     MapStVSt name_refs_, obj_name_refs_, subj_name_refs_;
     MapStVSt verb_refs_;
     MapStVSt complement_refs_;
     MapStVSt object_refs_;
     //MapStSt  subord_pairs_;

     RulesMapStVSt rules_name_refs_, rules_obj_name_refs_, rules_subj_name_refs_;
     RulesMapStVSt rules_verb_refs_;
     RulesMapStVSt rules_complement_refs_;
     RulesMapStVSt rules_object_refs_;

     MapRNDouble verb_weigths_;
     MapRNDouble subj_weigths_;
     MapRNDouble obj_weigths_;
     MapRNDouble name_weigths_;
     MapRNDouble compl_weigths_;
     
     RulesMapRNDouble rules_verb_weigths_;
     RulesMapRNDouble rules_subj_weigths_;
     RulesMapRNDouble rules_obj_weigths_;
     RulesMapRNDouble rules_name_weigths_;
     RulesMapRNDouble rules_compl_weigths_;

     double fixed_time_;       // fixed time after which the search for candidates stops (in sec)
     int max_refs_;            // max refs a name can have when pointing to an action
     int max_candidates_refs_; // max refs to be processed for matching
     int max_candidates_;      // max candidates to be matched


     vector<boost::shared_ptr<Predicate> > hyper_trees_; // additional hypernym trees
     
     vector<KResult> findMatch(vector<SPAction> &all_refs, vector<DrtPred> &question);
     vector<KRuleResult> findRulesMatch(vector<string> &all_refs, vector<DrtPred> &question);
     vector<KRuleResult> find_rules_match(vector<boost::tuple<vector<DrtPred>, clause_vector, string, string> > &data, vector<DrtPred> &question);
     void find_all_candidates(DrtVect &question, CandidatesStruct *cstruct);
     void find_all_rules_candidates(DrtVect &question, RulesCandidatesStruct *cstruct);

     void processPersonae(DrsPersonae &p);
     void processRules(Rules &r);
     DrtVect getFullDrs(DrtVect drs);       
     DrtVect getFullDrsForWriting(DrtVect drs);
     vector<Candidate> getCandidate(const string &ref, double remaining_time);
     vector<Candidate> getCandidate(SPAction aiter, double remaining_time);
     vector<RulesCandidate> getRulesCandidate(const string &ref, double remaining_time);
     vector<RulesCandidate> getGenericVerbRules();


     vector<pair<SPAction,double> > getRefWeights(const DrtVect &question, vector<SPAction> refs);
     vector<pair<string,double> > getRulesRefWeights(const DrtVect &drtvect, vector<string> refs);


     friend class CandidatesKThread;
     friend class RulesCandidatesKThread;

public:
     Knowledge();
     Knowledge(DrsPersonae &p, Rules &r);
     //vector< pair<pair<vector<DrtPred>,string>, double > > getAnswers(vector<DrtPred> &question);

     vector<Candidate> getAnswerCandidates(drt &question);

     vector<KnowledgeAnswer> getAnswers(drt &question);
     vector<KnowledgeAnswer> getAnswers(drt &question_drt, vector<Candidate> &all_candidates);
     vector<KnowledgeAnswer> getAnswers(vector<DrtPred> &question);
     vector<KnowledgeAnswer> getRules(vector<DrtPred> &question);

     void addPersonae(DrsPersonae &p);
     void addRules(Rules &r);
     vector<vector<DrtPred> > getSpecifications(const string &ref);
     vector<vector<DrtPred> > getComplements(const string &ref, const string &vref);
     DrsPersonae getPersonae() const {return personae_;}     
     Rules getRulesPersonae() const {return rules_;}     
     DrsPersonae getTempPersonae() const {return temp_personae_;}     
     
     void addHypernym(const Predicate &h);
     void addHypernym(const vector<Predicate> &h);
     vector<boost::shared_ptr<Predicate> > getHypernym() {return hyper_trees_;}

     void addTemporary(const DrtVect&);
     void addTemporary(const vector<DrtVect> &drtvect);
     void clearAllTemporary();
          
     vector<string> getVerbNames(const string &ref_str);
     vector<string> getNounNames(const string &ref_str);
     
     vector<DrtPred> getPredsFromRef(const string &ref) const;
     vector<DrtVect> getSpecificationsFromRef(const string &ref) const;
     vector<string> getVerbNamesFromRef(const string &ref) const;
     vector<DrtPred> getVerbPredsFromRef(const string &ref) const;
     vector<DrtPred> getAdverbsFromRef(const string &ref) const;
     vector<pair<DrtPred, Action> > getSubordinatesFromRef(const string &ref) const;
     Action getActionFromVerbRef(const string &ref, const string &vref) const;
     ConditionalAction getConditionalActionFromVerbRef(const string &ref, const string &vref) const;
     vector<string> getRefFromName(const string &name) const;
     vector<string> getNamesFromRef(const string &ref) const;
     vector<string> getTempNamesFromRef(const string &ref) const;

     void insertNameIntoMaps(const DrsPersonae &p, vector<string> &all_refs, const string &name_levin, const string &orig_name);
     void insertRulesNameIntoMaps(const Rules &p, vector<string> &all_refs, const string &name_levin, const string &orig_name);

     vector<drt> getDrtList();

     void setWisdomInfo(const WisdomInfo &wi);

     void clearNameRefs();
};


void operator / (vector<KnowledgeAnswer> &kav, const DrtMgu &mgu);
void use_mgu_prior(vector<KnowledgeAnswer> &kav, const DrtMgu &mgu);

#endif // __KNOWLEDGE__
