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



#include"Knowledge.hpp"
#include<ctime>

const bool debug = false;
const bool measure_time = false;
const bool show_candidates = false;
const bool commercial_version = true;
boost::mutex io_mutex_kcounter;

// this is the right configuration for a Wisdom of about 80 MB (about 8 MD of text)

const int max_rules_candidates = 150;     // max candidates to be matched
const double timeout_prediction = 2.0; // if there are 2 seconds to timeout, do not start searching for references
const int max_specs = 3; // the max number of specifications for each name
const int max_items = 50; // the max number of items when building a drs from an action
const int max_subordinates = 3; // the max number of subordinates for each sentence

template<class T>
static void print_vector(std::vector<T> &vs)
{
	if (vs.size()) {
		typename vector<T>::iterator tags_iter = vs.begin();
		while (tags_iter != vs.end()) {
			std::cout << (*tags_iter) << " ";
			++tags_iter;
		}
		std::cout << std::endl;
	}
}

static void print_vector_stream(std::stringstream &ff, const std::vector<DrtPred> &vs)
{
	vector<DrtPred>::const_iterator tags_iter = vs.begin();
	while (tags_iter < vs.end()) {
		tags_iter->print(ff);
		if (boost::next(tags_iter) != vs.end())
			ff << ", ";
		++tags_iter;
	}
}

template<class T>
static void print_vector(std::stringstream &ss, const std::vector<T> &vs)
{
	typename vector<T>::const_iterator tags_iter = vs.begin();
	while (tags_iter < vs.end()) {
		ss << (*tags_iter) << " ";
		++tags_iter;
	}
}

template<class T>
static bool shortfind(const vector<T> &vect, const T &element)
{
	if (find(vect.begin(), vect.end(), element) != vect.end())
		return true;
	return false;
}

template<class T>
static bool shortfind(const map<T,bool> &m, const T &element)
{
	typename map<T,bool>::const_iterator miter = m.find(element);
	if (miter != m.end())
		return true;
	return false;
}

template<typename T, typename OutIter>
void stl_intersect(const T& set1, const T& set2, OutIter out)
{
	std::set_intersection(set1.begin(), set1.end(), set2.begin(), set2.end(), out);
}

template<class T>
T sort_intersect(T& set1, T& set2)
{
	T vout;
	std::sort(set1.begin(), set1.end());
	std::sort(set2.begin(), set2.end());
	std::set_intersection(set1.begin(), set1.end(), set2.begin(), set2.end(), std::back_inserter(vout));
	return vout;
}

static double get_weight_map_element(MapRNDouble &map_element, const pair<SPAction,string> &key)
{
	double to_return=0;
	MapRNDouble::iterator miter = map_element.find(key);

	if (miter != map_element.end()) {
		to_return = miter->second;
	}
	if(to_return == 0)
		to_return = 0.01; ///
	return to_return;
}

static double get_rules_weight_map_element(RulesMapRNDouble &map_element, const pair<string,string> &key)
{
	double to_return=0;
	RulesMapRNDouble::iterator miter = map_element.find(key);

	if (miter != map_element.end()) {
		to_return = miter->second;
	}
	if(to_return == 0)
		to_return = 0.01; ///
	return to_return;
}




CandidatesStruct::CandidatesStruct(WisdomInfo wi)
{
	lacks_subj_= false;
	lacks_obj_ = false;
	lacks_verb_= true;
}



void CandidatesStruct::addVerbRefs(const vector<SPAction> &refs)
{
	verb_refs_.insert(verb_refs_.end(), refs.begin(), refs.end() );
}

void CandidatesStruct::addSubjRefs(const vector<SPAction> &refs)
{
	subj_refs_.insert(subj_refs_.end(), refs.begin(), refs.end() );
}

void CandidatesStruct::addObjRefs(const vector<SPAction> &refs)
{
	obj_refs_.insert(obj_refs_.end(), refs.begin(), refs.end() );
}

void CandidatesStruct::addNameRefs(const vector<SPAction> &refs)
{
	name_refs_.insert(name_refs_.end(), refs.begin(), refs.end() );
}

void CandidatesStruct::addComplRefs(const vector<SPAction> &refs)
{
	compl_refs_.insert(compl_refs_.end(), refs.begin(), refs.end() );
}


void CandidatesStruct::addNewSubjRefs(const vector<SPAction> &refs, bool is_new)
{
	if(is_new)
		all_subj_refs_.push_back(refs);
	else {
		if(all_subj_refs_.size() == 0)
			all_subj_refs_.push_back(refs);
		else
			all_subj_refs_.back().insert(all_subj_refs_.back().end(), refs.begin(), refs.end() );
	}
}

void CandidatesStruct::addNewObjRefs(const vector<SPAction> &refs, bool is_new)
{
	if(is_new)
		all_obj_refs_.push_back(refs);
	else {
		if(all_obj_refs_.size() == 0)
			all_obj_refs_.push_back(refs);
		else
			all_obj_refs_.back().insert(all_obj_refs_.back().end(), refs.begin(), refs.end() );
	}
}

void CandidatesStruct::addNewNameRefs(const vector<SPAction> &refs, bool is_new)
{
	if(is_new)
		all_name_refs_.push_back(refs);
	else {
		if(all_name_refs_.size() == 0)
			all_name_refs_.push_back(refs);
		else
			all_name_refs_.back().insert(all_name_refs_.back().end(), refs.begin(), refs.end() );
	}
}

void CandidatesStruct::addNewComplRefs(const vector<SPAction> &refs, bool is_new)
{
	if(is_new)
		all_compl_refs_.push_back(refs);
	else {
		if(all_compl_refs_.size() == 0)
			all_compl_refs_.push_back(refs);
		else
			all_compl_refs_.back().insert(all_compl_refs_.back().end(), refs.begin(), refs.end() );
	}
}

void CandidatesStruct::computeIntersection()
{
	vector<SPAction> inter_tmp;

	for(int n=0; n < all_subj_refs_.size(); ++n) {
		if (all_subj_refs_.at(n).size()) {
			std::sort(all_subj_refs_.at(n).begin(), all_subj_refs_.at(n).end());
			all_subj_refs_.at(n).erase(std::unique(all_subj_refs_.at(n).begin(), all_subj_refs_.at(n).end()), all_subj_refs_.at(n).end());
		}
	}
	for(int n=0; n < all_obj_refs_.size(); ++n) {
		if (all_obj_refs_.at(n).size()) {
			std::sort(all_obj_refs_.at(n).begin(), all_obj_refs_.at(n).end());
			all_obj_refs_.at(n).erase(std::unique(all_obj_refs_.at(n).begin(), all_obj_refs_.at(n).end()), all_obj_refs_.at(n).end());
		}
	}
	for(int n=0; n < all_name_refs_.size(); ++n) {
		if (all_name_refs_.at(n).size()) {
			std::sort(all_name_refs_.at(n).begin(), all_name_refs_.at(n).end());
			all_name_refs_.at(n).erase(std::unique(all_name_refs_.at(n).begin(), all_name_refs_.at(n).end()), all_name_refs_.at(n).end());
		}
	}
	for(int n=0; n < all_compl_refs_.size(); ++n) {
		if (all_compl_refs_.at(n).size()) {
			std::sort(all_compl_refs_.at(n).begin(), all_compl_refs_.at(n).end());
			all_compl_refs_.at(n).erase(std::unique(all_compl_refs_.at(n).begin(), all_compl_refs_.at(n).end()), all_compl_refs_.at(n).end());
		}
	}

	if(all_subj_refs_.size())
		subj_refs_ = all_subj_refs_.at(0);
	for(int n=1; n < all_subj_refs_.size(); ++n) {
		inter_tmp= all_subj_refs_.at(n);
		subj_refs_ = sort_intersect(subj_refs_,inter_tmp);
	}
	if(all_obj_refs_.size())
		obj_refs_ = all_obj_refs_.at(0);
	for(int n=1; n < all_obj_refs_.size(); ++n) {
		inter_tmp= all_obj_refs_.at(n);
		obj_refs_ = sort_intersect(obj_refs_,inter_tmp);
	}
	if(all_name_refs_.size())
		name_refs_ = all_name_refs_.at(0);
	// For broken questions the names must include subjects and objects.
	// The intersection between name_refs_ make the approximation fail
	if(wi_.getAccuracyLevel() <= 2) {
		for(int n=1; n < all_name_refs_.size(); ++n) {
			inter_tmp= all_name_refs_.at(n);
			name_refs_ = sort_intersect(name_refs_,inter_tmp);
		}
	}
	if(all_compl_refs_.size())
		compl_refs_ = all_compl_refs_.at(0);
	for(int n=1; n < all_compl_refs_.size(); ++n) {
		inter_tmp= all_compl_refs_.at(n);
		compl_refs_ = sort_intersect(compl_refs_,inter_tmp);
	}
	this->sort();
}

void CandidatesStruct::setLacksSubj(bool lacks)
{
	lacks_subj_= lacks;
}

void CandidatesStruct::setLacksObj (bool lacks)
{
	lacks_obj_= lacks;
}

void CandidatesStruct::setLacksVerb(bool lacks)
{
	lacks_verb_= lacks;
}

void CandidatesStruct::sort()
{
	if (subj_refs_.size()) {
		std::sort(subj_refs_.begin(), subj_refs_.end());
		subj_refs_.erase(std::unique(subj_refs_.begin(), subj_refs_.end()), subj_refs_.end());
	}
	if (obj_refs_.size()) {
		std::sort(obj_refs_.begin(), obj_refs_.end());
		obj_refs_.erase(std::unique(obj_refs_.begin(), obj_refs_.end()), obj_refs_.end());
	}
	if (verb_refs_.size()) {
		std::sort(verb_refs_.begin(), verb_refs_.end());
		verb_refs_.erase(std::unique(verb_refs_.begin(), verb_refs_.end()), verb_refs_.end());
	}
	if (name_refs_.size()) {
		std::sort(name_refs_.begin(), name_refs_.end());
		name_refs_.erase(std::unique(name_refs_.begin(), name_refs_.end()), name_refs_.end());
	}
	if (compl_refs_.size()) {
		std::sort(compl_refs_.begin(), compl_refs_.end());
		compl_refs_.erase(std::unique(compl_refs_.begin(), compl_refs_.end()), compl_refs_.end());
	}
}



RulesCandidatesStruct::RulesCandidatesStruct()
{
	lacks_subj_= false;
	lacks_obj_ = false;
	lacks_verb_= true;
}



void RulesCandidatesStruct::addVerbRefs(const vector<string> &refs)
{
	verb_refs_.insert(verb_refs_.end(), refs.begin(), refs.end() );
}

void RulesCandidatesStruct::addSubjRefs(const vector<string> &refs)
{
	subj_refs_.insert(subj_refs_.end(), refs.begin(), refs.end() );
}

void RulesCandidatesStruct::addObjRefs(const vector<string> &refs)
{
	obj_refs_.insert(obj_refs_.end(), refs.begin(), refs.end() );
}

void RulesCandidatesStruct::addNameRefs(const vector<string> &refs)
{
	name_refs_.insert(name_refs_.end(), refs.begin(), refs.end() );
}

void RulesCandidatesStruct::addComplRefs(const vector<string> &refs)
{
	compl_refs_.insert(compl_refs_.end(), refs.begin(), refs.end() );
}

void RulesCandidatesStruct::setLacksSubj(bool lacks)
{
	lacks_subj_= lacks;
}

void RulesCandidatesStruct::setLacksObj (bool lacks)
{
	lacks_obj_= lacks;
}

void RulesCandidatesStruct::setLacksVerb(bool lacks)
{
	lacks_verb_= lacks;
}

void RulesCandidatesStruct::sort()
{
	if (subj_refs_.size()) {
		std::sort(subj_refs_.begin(), subj_refs_.end());
		subj_refs_.erase(std::unique(subj_refs_.begin(), subj_refs_.end()), subj_refs_.end());
	}
	if (obj_refs_.size()) {
		std::sort(obj_refs_.begin(), obj_refs_.end());
		obj_refs_.erase(std::unique(obj_refs_.begin(), obj_refs_.end()), obj_refs_.end());
	}
	if (verb_refs_.size()) {
		std::sort(verb_refs_.begin(), verb_refs_.end());
		verb_refs_.erase(std::unique(verb_refs_.begin(), verb_refs_.end()), verb_refs_.end());
	}
	if (name_refs_.size()) {
		std::sort(name_refs_.begin(), name_refs_.end());
		name_refs_.erase(std::unique(name_refs_.begin(), name_refs_.end()), name_refs_.end());
	}
	if (compl_refs_.size()) {
		std::sort(compl_refs_.begin(), compl_refs_.end());
		compl_refs_.erase(std::unique(compl_refs_.begin(), compl_refs_.end()), compl_refs_.end());
	}
}



static bool is_broken(const string &str)
{
	if (str.find("broken") != string::npos)
		return true;
	return false;
}

static vector<string> get_similar_verbs(const string &str)
{
	vector<string> to_return;
	to_return.push_back(str);
	if (str == "say") {
		to_return.push_back("report");
		to_return.push_back("speak");
	}
	if (str == "travel") {
		to_return.push_back("go");
	}
	if (str == "go") {
		to_return.push_back("travel");
	}
	return to_return;
}

static vector<string> get_similar_names(const string &str)
{
	vector<string> to_return;
	to_return.push_back(str);
	if (str == "man") {
		to_return.push_back("person");
		to_return.push_back("relative");
		to_return.push_back("human_being");
	}
	if (str == "person") {
		to_return.push_back("profession");
		to_return.push_back("professional");
		to_return.push_back("educator");
		to_return.push_back("team");
		to_return.push_back("man");
		to_return.push_back("relative");
		to_return.push_back("human_being");
		to_return.push_back("corporation");
		to_return.push_back("organization");
		to_return.push_back("politician");
		to_return.push_back("chancellor");
		to_return.push_back("team");
		to_return.push_back("other");
		to_return.push_back("[noun.person]");
	}
	if (str == "material") {
		to_return.push_back("metal");
	}
	if (str == "place") {
		to_return.push_back("district");
	}
	if (str == "team") {
		to_return.push_back("people");
		to_return.push_back("person");
	}
	if (str == "animal") {
		to_return.push_back("mammal");
		to_return.push_back("reptile");
		to_return.push_back("marsupial");
		to_return.push_back("dinosaur");
		to_return.push_back("beast");
	}
	if (str == "snake") {
		to_return.push_back("reptile");
		to_return.push_back("animal");
		to_return.push_back("beast");
	}
	if (str == "chicken") {
		to_return.push_back("hen");
	}
	metric *d = metric_singleton::get_metric_instance();
	bool is_people_name = d->gender_proper_name(str) != "";
	if(is_people_name) {
		to_return.push_back("person");
	}

	return to_return;
}

static vector<string> get_similar_names_for_rules(const string &str)
{
	vector<string> to_return;
	to_return.push_back(str);
	if (str == "man") {
		to_return.push_back("person");
		to_return.push_back("relative");
		to_return.push_back("human_being");
	}
	if (str == "person") {
		to_return.push_back("profession");
		to_return.push_back("professional");
		to_return.push_back("educator");
		to_return.push_back("team");
		to_return.push_back("man");
		to_return.push_back("relative");
		to_return.push_back("human_being");
		to_return.push_back("corporation");
		to_return.push_back("organization");
		to_return.push_back("politician");
		to_return.push_back("chancellor");
		to_return.push_back("team");
		to_return.push_back("other");
		to_return.push_back("[noun.person]");
	}
	if (str == "material") {
		to_return.push_back("metal");
	}
	if (str == "place") {
		to_return.push_back("district");
	}
	if (str == "team") {
		to_return.push_back("people");
		to_return.push_back("person");
	}
	if (str == "animal") {
		to_return.push_back("mammal");
		to_return.push_back("reptile");
		to_return.push_back("marsupial");
		to_return.push_back("dinosaur");
		to_return.push_back("beast");
	}
	if (str == "snake") {
		to_return.push_back("reptile");
		to_return.push_back("animal");
		to_return.push_back("beast");
	}
	metric *d = metric_singleton::get_metric_instance();
	bool is_people_name = d->gender_proper_name(str) != "";
	if(is_people_name) {
		to_return.push_back("person");
	}
	string name_levin = d->get_levin_noun(str);
	if (name_levin.size()) {
		to_return.push_back(string("[") + name_levin + "]");
		if(name_levin == "noun.animal") {
			to_return.push_back("animal");
			to_return.push_back("[what]");
		}
		if(name_levin == "noun.state") {
			to_return.push_back("state");
			to_return.push_back("[what]");
		}
	}

	if (matching::is_what(str)) {
		to_return.push_back("[what]");
		to_return.push_back("thing");
	}
	if (matching::is_who(str))
		to_return.push_back("person");

	return to_return;
}


static vector<string> get_similar_names_for_data(const string &str)
{
	vector<string> to_return;
	to_return.push_back(str);
	if (str == "man") {
		to_return.push_back("person");
		to_return.push_back("relative");
		to_return.push_back("human_being");
	}
	if (str == "person") {
		to_return.push_back("profession");
		to_return.push_back("professional");
		to_return.push_back("educator");
		to_return.push_back("team");
		to_return.push_back("man");
		to_return.push_back("relative");
		to_return.push_back("human_being");
		to_return.push_back("corporation");
		to_return.push_back("organization");
		to_return.push_back("politician");
		to_return.push_back("chancellor");
		to_return.push_back("team");
		to_return.push_back("other");
		to_return.push_back("[noun.person]");
	}
	if (str == "material") {
		to_return.push_back("metal");
	}
	if (str == "place") {
		to_return.push_back("district");
	}
	if (str == "team") {
		to_return.push_back("people");
		to_return.push_back("person");
	}
	if (str == "animal") {
		to_return.push_back("mammal");
		to_return.push_back("reptile");
		to_return.push_back("marsupial");
		to_return.push_back("dinosaur");
		to_return.push_back("beast");
	}
	if (str == "snake") {
		to_return.push_back("reptile");
		to_return.push_back("animal");
		to_return.push_back("beast");
	}

	return to_return;
}


bool KnowledgeAnswer::operator==(const KnowledgeAnswer &rhs) const
{
	if (preds_ == rhs.preds_ && link_ == rhs.link_ && text_ == rhs.text_ && weigth_ == rhs.weigth_ && qlist_ == rhs.qlist_)
		return true;
	return false;
}

void KnowledgeAnswer::operator/(const DrtMgu &additional_mgu)
{
	upg_ / additional_mgu;
	preds_ / additional_mgu;
}

void KnowledgeAnswer::use_mgu_prior(const DrtMgu &additional_mgu)
{
	qlist_ / additional_mgu;
	qlist_ / upg_;
}

void Knowledge::addPersonae(DrsPersonae &p)
{
	//personae_.addPersonae(p);
	this->processPersonae(p); // personae is not const because it p.sort() is called
}

void Knowledge::addRules(Rules &r)
{
	//rules_.addRules(r);
	this->processRules(r);
}

static vector<string> get_candidate_names_for_adjective(const string &name_str)
{
	vector<string> to_return;
	metric *d = metric_singleton::get_metric_instance();
	vector<string> strs;
	boost::split(strs, name_str, boost::is_any_of("|"));
	for (int n = 0; n < strs.size(); ++n) {
		vector<string> strs2; // cpp11 // = {name_str};
		strs2.push_back(name_str);
		to_return.insert(to_return.end(), strs2.begin(), strs2.end());
	}
	vector<string> strs2 = get_similar_names(name_str);
	to_return.insert(to_return.end(), strs2.begin(), strs2.end());
	if (d->pertains_to_name(name_str, "material", 6) > 0.5 || d->hypernym_dist(name_str, "material", 8) > 0.5) {
		to_return.push_back("material");
	}
	if (d->hypernym_dist(name_str, "metal", 8) > 0.5) {
		to_return.push_back("metal");
		to_return.push_back("material");
	}
	if (d->pertains_to_name(name_str, "color", 6) > 0.5 || d->hypernym_dist(name_str, "color", 8) > 0.5) {
		to_return.push_back("color");
		to_return.push_back("colour");
	}
	to_return.push_back(name_str);
	to_return.push_back("[*]");

	return to_return;
}

static vector<string> get_candidate_names_for_complements(const string &name_str)
{
	vector<string> to_return;
	metric *d = metric_singleton::get_metric_instance();
	vector<string> strs;
	boost::split(strs, name_str, boost::is_any_of("|"));
	vector<string> strs2;
	to_return.insert(to_return.end(), strs2.begin(), strs2.end());
	to_return.push_back(name_str);
	if (name_str == "@QUANTITY") {
		to_return.push_back("@MORE_THAN");
		to_return.push_back("@LESS_THAN");
	}
	if (name_str == "@MORE_THAN") {
		to_return.push_back("@QUANTITY");
		to_return.push_back("@LESS_THAN");
	}
	if (name_str == "@LESS_THAN") {
		to_return.push_back("@QUANTITY");
		to_return.push_back("@MORE_THAN");
	}
	if (name_str == "@TIME_AT") {
		to_return.push_back("@AFTER");
		to_return.push_back("@BEFORE");
	}
	if (name_str == "@AFTER") {
		to_return.push_back("@TIME_AT");
		to_return.push_back("@BEFORE");
	}
	if (name_str == "@BEFORE") {
		to_return.push_back("@TIME_AT");
		to_return.push_back("@AFTER");
	}

	return to_return;
}

static vector<string> get_candidate_names_for_noun(const string &name_str)
{
	vector<string> to_return;
	metric *d = metric_singleton::get_metric_instance();
	vector<string> strs;
	boost::split(strs, name_str, boost::is_any_of("|"));
	for (int n = 0; n < strs.size(); ++n) {
		string name_str_n= strs.at(n);
		vector<string> strs2; // cpp11 // = {name_str_n};
		strs2.push_back(name_str_n);
		to_return.insert(to_return.end(), strs2.begin(), strs2.end());

		string name_levin = d->get_levin_noun(name_str_n);
		if (name_levin.size()) {
			to_return.push_back(string("[") + name_levin + "]");
			if(name_levin == "noun.animal")
				to_return.push_back("animal");
			if(name_levin == "noun.state")
				to_return.push_back("state");
		} else {
			to_return.push_back("[what]");
			to_return.push_back("person");
		}
		bool is_people_name = d->gender_proper_name(name_str_n) != "";
		if(is_people_name) {
			to_return.push_back("person");
		}
	}
	vector<string> strs2 = get_similar_names(name_str);
	to_return.insert(to_return.end(), strs2.begin(), strs2.end());
	to_return.push_back(name_str);
	to_return.push_back("[*]");

	if (matching::is_what(name_str)) {
		to_return.push_back("[what]");
		to_return.push_back("thing");
	}
	if (matching::is_who(name_str))
		to_return.push_back("person");

	return to_return;
}

static vector<string> get_candidate_names_for_verb(const string &name_str)
{
	vector<string> to_return;
	metric *d = metric_singleton::get_metric_instance();
	vector<string> strs;
	boost::split(strs, name_str, boost::is_any_of("|"));
	for (int n = 0; n < strs.size(); ++n) {
		string name_str_n= strs.at(n);
		vector<string> strs2; // cpp11 // = {name_str_n};
		strs2.push_back(name_str_n);
		to_return.insert(to_return.end(), strs2.begin(), strs2.end());

		string name_levin = d->get_levin_verb(name_str_n);
		if (name_levin.size()) {
			to_return.push_back(string("[") + name_levin + "]");
		}
	}
	vector<string> strs2 = get_similar_verbs(name_str);
	to_return.insert(to_return.end(), strs2.begin(), strs2.end());
	to_return.push_back(name_str);
	to_return.push_back("[*]");

	std::sort(to_return.begin(), to_return.end());
	to_return.erase(std::unique(to_return.begin(), to_return.end()), to_return.end());

	return to_return;
}

template<class M, class V>
void insert_vector_into_map(M &m, V &v, const string &s)
{
	for (int n = 0; n < v.size(); ++n) {
		m[s].push_back(v.at(n));
	}
}

template<class M, class V>
void insert_rules_vector_into_map(M &m, V &v, const string &s)
{
	for (int n = 0; n < v.size(); ++n) {
		m[s].push_back(v.at(n));
	}
}


template<class M, class A>
void insert_action_into_map(M &m, A &a, const string &s)
{
	m[s].push_back(a);
}


void insert_vsp(MapStVSt &m, const string &key, SPAction pa)
{
	MapStVSt::iterator miter = m.find(key);
	if(miter != m.end()) {
		vector<SPAction> vpa = miter->second;
		if(!shortfind(vpa,pa))
			miter->second.push_back(pa);
	} else {
		m[key].push_back(pa);
	}
}

void Knowledge::insertNameIntoMaps(const DrsPersonae &p, vector<string> &all_refs, const string &name_levin, const string &orig_name)
{
	metric *d = metric_singleton::get_metric_instance();
	for (int n = 0; n < all_refs.size(); ++n) {
		string ref = all_refs.at(n);
		bool ref_is_subj = p.refIsSubj(ref);
		bool ref_is_obj = p.refIsObj(ref);
		vector<SPAction> subj_actions= p.getSubjActions(ref);
		vector<SPAction>  obj_actions= p.getObjActions(ref);


		for(int na=0; na < subj_actions.size(); ++na) {
			SPAction pa= subj_actions.at(na);
			subj_name_refs_[name_levin].push_back(pa);
		}
		for(int na=0; na < obj_actions.size(); ++na) {
			SPAction pa= obj_actions.at(na);
			obj_name_refs_[name_levin].push_back(pa);
		}
		//if(subj_actions.size() == 0 && obj_actions.size() == 0) {
		{
			vector<SPAction> pas= p.getNameActions(ref);
			for (int m = 0; m < pas.size(); ++m) {
				//insert_vsp(name_refs_,name_levin,pas.at(m));
				name_refs_[name_levin].push_back(pas.at(m));
				//name_weigths_[make_pair(pas.at(m),name_levin)] = w;
			}
		}
	}
}


void Knowledge::processPersonae(DrsPersonae &p)
{
	p.sort();

	MapStPers::iterator riter = p.begin();
	MapStPers::iterator rend  = p.end();

	metric *d = metric_singleton::get_metric_instance();

	for (; riter != rend; ++riter) {
		try {
			Persona tmp_persona = riter->second;
			string reference = tmp_persona.getReference();

			vector<DrtPred> pers_names = tmp_persona.getPreds();
			for (int m = 0; m < pers_names.size(); ++m) {
				string name_str = extract_header(pers_names.at(m));


				vector<string> all_names = get_candidate_names_for_noun(name_str);
				vector<string> all_refs;
				all_refs.push_back(reference);

				for (int n = 0; n < all_names.size(); ++n) {
					string name_levin = all_names.at(n);
					insertNameIntoMaps(p, all_refs, name_levin, name_str);
				}
				all_names = get_candidate_names_for_adjective(name_str);
				for (int n = 0; n < all_names.size(); ++n) {
					string name_levin = all_names.at(n);
					//insert_vector_into_map(name_refs_,all_refs,name_levin);
					insertNameIntoMaps(p, all_refs, name_levin, name_str);
				}

				if (pers_names.at(m).is_proper_name() || pers_names.at(m).is_PRP() || pers_names.at(m).is_article()) {
					vector<string> alt_nouns;
					alt_nouns.push_back("[what]");
					alt_nouns.push_back("thing");
					alt_nouns.push_back("person");
					alt_nouns.push_back("man");
					alt_nouns.push_back("something");
					alt_nouns.push_back("someone");

					for (int j = 0; j < alt_nouns.size(); ++j) {
						insertNameIntoMaps(p, all_refs, alt_nouns.at(j), name_str);
					}
				}
				if (pers_names.at(m).is_number() ) {
					vector<string> alt_nouns;
					alt_nouns.push_back("[number]");

					for (int j = 0; j < alt_nouns.size(); ++j) {
						insertNameIntoMaps(p, all_refs, alt_nouns.at(j), name_str);
					}
				}
			}
			// store the names of the specifications
			vector<DrtVect> complements = tmp_persona.getSpecifications();
			for (int k = 0; k < complements.size(); ++k) {
				if (complements.at(0).size() == 0)
					continue;
				DrtPred compl0 = complements.at(k).at(0);
				string header = extract_header(compl0);
				string fref= extract_first_tag(compl0);
				string sref= extract_second_tag(compl0);
				vector<string> all_names = get_candidate_names_for_complements(header);
				for (int m = 0; m < all_names.size(); ++m) {
					string compl_levin = all_names.at(m);
					vector<SPAction> all_refs;
					all_refs = p.getNameActions(sref);
					insert_vector_into_map(complement_refs_, all_refs, compl_levin);
					all_refs = p.getSubjActions(sref);
					insert_vector_into_map(complement_refs_, all_refs, compl_levin);
					all_refs = p.getObjActions(sref);
					insert_vector_into_map(complement_refs_, all_refs, compl_levin);
					all_refs = p.getNameActions(fref);
					insert_vector_into_map(complement_refs_, all_refs, compl_levin);
					all_refs = p.getSubjActions(fref);
					insert_vector_into_map(complement_refs_, all_refs, compl_levin);
					all_refs = p.getObjActions(fref);
					insert_vector_into_map(complement_refs_, all_refs, compl_levin);
				}
			}

			std::vector<boost::shared_ptr<Action> > actions = tmp_persona.getActions();
			std::vector<boost::shared_ptr<Action> >::iterator aiter= actions.begin();
			std::vector<boost::shared_ptr<Action> >::iterator aend= actions.end();
			for (; aiter != aend; ++aiter) {
				string action_ref= (*aiter)->getRef();

				//vector<string> all_refs = p.mapRefToActionRef(reference);
				//all_refs.push_back(reference); // necessary because the object of an action can be the subject of anoter action

				// stores the names of the complements that introduce subordinates
				vector<pair<DrtPred, Action> > subordinate_pairs = p.getSubordinates(action_ref);
				for (int m = 0; m < subordinate_pairs.size(); ++m) {
					string header = extract_header(subordinate_pairs.at(m).first);
					insert_action_into_map(complement_refs_, *aiter, header);
					string fref = extract_first_tag (subordinate_pairs.at(m).first);
					string sref = extract_second_tag(subordinate_pairs.at(m).first);

					// a subordinate points to the head sentence
					/// subord_pairs_[sref] = fref;
				}

				// store the name of the verb
				string verb_str = (*aiter)->getVerb();
				vector<string> all_names = get_candidate_names_for_verb(verb_str);
				for (int m = 0; m < all_names.size(); ++m) {
					string verb_levin = all_names.at(m);
					verb_refs_[verb_levin].push_back(*aiter); // store the name of the Persona
					//// "Happen" and "do" are generic verbs. All the
					//// actions are stored under these verbs as well. It
					//// is not an elegant solution.
					vector<string> alt_verbs;
					alt_verbs.push_back("do");
					alt_verbs.push_back("happen");
					alt_verbs.push_back("occur");

					for (int j = 0; j < alt_verbs.size(); ++j) {
						//for (int j2 = 0; j2 < all_refs.size(); ++j2) {
						verb_refs_[alt_verbs.at(j)].push_back(*aiter);
					}
				}
				// store the names of the complements
				vector<DrtVect> complements = (*aiter)->getComplements();
				for (int k = 0; k < complements.size(); ++k) {
					if (complements.at(0).size() == 0)
						continue;
					DrtPred compl0 = complements.at(k).at(0);
					string header = extract_header(compl0);
					vector<string> all_names = get_candidate_names_for_complements(header);
					for (int m = 0; m < all_names.size(); ++m) {
						string compl_levin = all_names.at(m);
						//for (int j2 = 0; j2 < all_refs.size(); ++j2) {
						complement_refs_[compl_levin].push_back(*aiter);
						//}
					}
				}
			}
		} catch (std::runtime_error &e) {
			///
		}
	}
	//this->clearNameRefs();
	p.clearUseless();
	personae_.addPersonae(p);
}


void Knowledge::insertRulesNameIntoMaps(const Rules &r, vector<string> &all_refs, const string &name_levin, const string &orig_name)
{
	metric *d = metric_singleton::get_metric_instance();
	double w = d->distance(orig_name,name_levin);

	for (int n = 0; n < all_refs.size(); ++n) {
		string ref = all_refs.at(n);
		bool ref_is_subj = r.refIsSubj(ref);
		bool ref_is_obj = r.refIsObj(ref);
		if (ref_is_subj) {
			rules_subj_name_refs_[name_levin].push_back(ref);
			rules_subj_weigths_[make_pair(ref,name_levin)] = w;
		}
		if (ref_is_obj) {
			vector<string> new_refs = r.mapRefToActionRef(ref);
			new_refs.push_back(ref); // the subject of an action can be the object of another action
			for (int m = 0; m < new_refs.size(); ++m) {
				rules_obj_name_refs_[name_levin].push_back(new_refs.at(m));
				rules_obj_weigths_[make_pair(new_refs.at(m),name_levin)] = w;
			}
		}
		if (!ref_is_subj && !ref_is_obj) {
			vector<string> new_refs = r.mapRefToActionRef(ref);
			for (int m = 0; m < new_refs.size(); ++m) {
				rules_name_refs_[name_levin].push_back(new_refs.at(m));
				rules_name_weigths_[make_pair(new_refs.at(m),name_levin)] = w;
			}
		}
	}
}

void Knowledge::processRules(Rules &r)
{
	r.sort();
	vector<string> refs;
	refs = r.getReferences();
	vector<string>::iterator riter = refs.begin();
	vector<string>::iterator rend = refs.end();
	metric *d = metric_singleton::get_metric_instance();

	int ntest = 0;
	for (; riter != rend; ++riter, ++ntest) {
		try {
			ConditionalPersona tmp_persona = r.getConditionalPersona(*riter);
			vector<DrtPred> pers_names = tmp_persona.getPreds();
			for (int m = 0; m < pers_names.size(); ++m) {
				string name_str = extract_header(pers_names.at(m));
				vector<string> all_names = get_candidate_names_for_noun(name_str);
				vector<string> all_refs;
				all_refs.push_back(*riter);

				for (int n = 0; n < all_names.size(); ++n) {
					string name_levin = all_names.at(n);
					insertRulesNameIntoMaps(r, all_refs, name_levin, name_str);
				}
				all_names = get_candidate_names_for_adjective(name_str);
				for (int n = 0; n < all_names.size(); ++n) {
					string name_levin = all_names.at(n);
					insertRulesNameIntoMaps(r, all_refs, name_levin, name_str);
				}

				if (pers_names.at(m).is_proper_name() || pers_names.at(m).is_PRP() || pers_names.at(m).is_article()) {
					vector<string> alt_nouns;
					alt_nouns.push_back("[what]");
					alt_nouns.push_back("thing");
					alt_nouns.push_back("person");
					alt_nouns.push_back("man");
					alt_nouns.push_back("something");
					alt_nouns.push_back("someone");

					for (int j = 0; j < alt_nouns.size(); ++j) {
						insertRulesNameIntoMaps(r, all_refs, alt_nouns.at(j), name_str);
					}
				}
			}
			// store the names of the specifications
			vector<DrtVect> complements = tmp_persona.getSpecifications();
			for (int k = 0; k < complements.size(); ++k) {
				if (complements.at(0).size() == 0)
					continue;
				DrtPred compl0 = complements.at(k).at(0);
				string header = extract_header(compl0);
				vector<string> all_names = get_candidate_names_for_complements(header);
				for (int m = 0; m < all_names.size(); ++m) {
					string compl_levin = all_names.at(m);
					vector<string> all_refs = r.mapRefToActionRef(*riter);
					all_refs.push_back(*riter);
					insert_rules_vector_into_map(rules_complement_refs_, all_refs, compl_levin);
				}
			}
			vector<boost::shared_ptr<ConditionalAction> > actions = tmp_persona.getActions();
			for (int n = 0; n < actions.size(); ++n) {
				// store the name of the verb
				string verb_str = actions.at(n)->getVerb();
				vector<string> all_names = get_candidate_names_for_verb(verb_str);
				vector<string> all_refs = r.mapRefToActionRef(*riter);
				all_refs.push_back(*riter); // necessary because the object of an action can be the subject of anoter action
				for (int m = 0; m < all_names.size(); ++m) {
					string verb_levin = all_names.at(m);
					rules_verb_refs_[verb_levin].push_back(*riter); // store the name of the Persona
					double w = d->distance(verb_str,verb_levin);
					rules_verb_weigths_[make_pair(*riter,verb_levin)] = w;
					
					//vector<string> object = actions.at(n)->getObject();

					//// "Happen" and "do" are generic verbs. All the
					//// actions are stored under these verbs as well. It
					//// is not an elegant solution.
					vector<string> alt_verbs;
					alt_verbs.push_back("do");
					alt_verbs.push_back("happen");
					alt_verbs.push_back("occur");

					for (int j = 0; j < alt_verbs.size(); ++j) {
						for (int j2 = 0; j2 < all_refs.size(); ++j2) {
							rules_verb_refs_[alt_verbs.at(j)].push_back(all_refs.at(j2));
							double w = d->distance(verb_str,verb_levin);
							rules_verb_weigths_[make_pair(all_refs.at(j2),verb_levin)] = w;
						}
					}
				}
				// store the names of the complements
				vector<DrtVect> complements = actions.at(n)->getComplements();
				for (int k = 0; k < complements.size(); ++k) {
					if (complements.at(0).size() == 0)
						continue;
					DrtPred compl0 = complements.at(k).at(0);
					string header = extract_header(compl0);
					vector<string> all_names = get_candidate_names_for_complements(header);
					for (int m = 0; m < all_names.size(); ++m) {
						string compl_levin = all_names.at(m);
						for (int j2 = 0; j2 < all_refs.size(); ++j2) {
							rules_complement_refs_[compl_levin].push_back(all_refs.at(j2));
						}
					}
				}
			}
		} catch (std::runtime_error &e) {
			///
		}		
	}

	r.clearUseless();
	rules_.addRules(r);

}

Knowledge::Knowledge()
	: engine_(this)
{
	fixed_time_   = wi_.getFixedTime();
	max_refs_     = wi_.getMaxRefs();
	max_candidates_refs_ = wi_.getMaxCandidatesRefs();
	max_candidates_      = wi_.getMaxCandidates();
}

static bool compare_actions(const SPAction &lhs, const SPAction &rhs)
{
	return lhs.get() > rhs.get();
}


void Knowledge::clearNameRefs()
{
	MapStVSt::iterator miter = subj_name_refs_.begin();
	MapStVSt::iterator mend  = subj_name_refs_.end();

	MapStVSt smap,omap,nmap;

	for(; miter != mend; ++miter) {
		vector<SPAction> vspa = miter->second;
		std::sort(vspa.begin(), vspa.end(), compare_actions);
		vspa.erase(std::unique(vspa.begin(), vspa.end()), vspa.end());
		//miter->second = vspa;
		smap[miter->first] = vspa;
	}
	subj_name_refs_.clear();
	subj_name_refs_ = smap;


	miter = obj_name_refs_.begin();
	mend  = obj_name_refs_.end();

	for(; miter != mend; ++miter) {
		vector<SPAction> vspa = miter->second;
		std::sort(vspa.begin(), vspa.end(), compare_actions);
		vspa.erase(std::unique(vspa.begin(), vspa.end()), vspa.end());
		//miter->second = vspa;
		omap[miter->first] = vspa;
	}
	obj_name_refs_.clear();
	obj_name_refs_ = omap;

	miter = name_refs_.begin();
	mend  = name_refs_.end();

	for(; miter != mend; ++miter) {
		vector<SPAction> vspa = miter->second;
		std::sort(vspa.begin(), vspa.end(), compare_actions);
		vspa.erase(std::unique(vspa.begin(), vspa.end()), vspa.end());
		//miter->second = vspa;
		nmap[miter->first] = vspa;
	}
	name_refs_.clear();
	name_refs_ = nmap;

//	subj_name_refs_.rehash(subj_name_refs_.size());
//	obj_name_refs_.rehash(obj_name_refs_.size());
//	name_refs_.rehash(name_refs_.size());
}

Knowledge::Knowledge(DrsPersonae &p, Rules &r) :
		//personae_(p), rules_(r),
		engine_(this)
{
	fixed_time_ = wi_.getFixedTime();

	this->processPersonae(p);
	this->processRules(r);
}

vector<Candidate> Knowledge::getCandidate(const string &ref, double remaining_time = 30)
{
	clock_t start_time = clock();

	vector<Candidate> candidates;
	try {
		Persona tmp_persona = personae_.getPersona(ref);
		std::vector<boost::shared_ptr<Action> > actions = tmp_persona.getActions();
		std::vector<boost::shared_ptr<Action> >::iterator aiter = actions.begin();
		std::vector<boost::shared_ptr<Action> >::iterator aend = actions.end();
		for (; aiter != aend; ++aiter) {
			clock_t current_time = clock();
			double elapsed = (current_time - start_time) / (double) CLOCKS_PER_SEC;

			DrtVect drs = (*aiter)->getDrs();

			if(elapsed > remaining_time)
				break;

			clock_t start;
			if (debug || measure_time)
				start = clock();

			drs = this->getFullDrs(drs);




			string link = (*aiter)->getLink();
			string text = (*aiter)->getText();
			CodePred code = (*aiter)->getCode();

			// unique
			DrtVect candidate_drs;
			for (int n = 0; n < drs.size(); ++n) {
				if (!shortfind(candidate_drs, drs.at(n)))
					candidate_drs.push_back(drs.at(n));
			}

			Candidate tmp_candidate(boost::make_tuple(candidate_drs, link, text, code));

			if (!shortfind(candidates, tmp_candidate))
				candidates.push_back(tmp_candidate);
		}
	} catch (std::runtime_error &e) {
		///
	}

	return candidates;
}


vector<Candidate> Knowledge::getCandidate(SPAction aiter, double remaining_time = 30)
{
	clock_t start_time = clock();

	vector<Candidate> candidates;
	try {
		clock_t current_time = clock();
		double elapsed = (current_time - start_time) / (double) CLOCKS_PER_SEC;

		vector<DrtPred> drs = aiter->getDrs();

		if(elapsed > remaining_time)
			return candidates;

		clock_t start;
		if (debug || measure_time)
			start = clock();

		drs = this->getFullDrs(drs);



		string link = aiter->getLink();
		string text = aiter->getText();
		CodePred code = aiter->getCode();

		// unique
		DrtVect candidate_drs;
		for (int n = 0; n < drs.size(); ++n) {
			if (!shortfind(candidate_drs, drs.at(n)))
				candidate_drs.push_back(drs.at(n));
		}

		Candidate tmp_candidate(boost::make_tuple(candidate_drs, link, text, code));

		if (!shortfind(candidates, tmp_candidate))
			candidates.push_back(tmp_candidate);

	} catch (std::runtime_error &e) {
		///
	}

	return candidates;
}



vector<RulesCandidate> Knowledge::getRulesCandidate(const string &ref, double remaining_time = 30)
{
	clock_t start_time = clock();

	vector<RulesCandidate> candidates;
	try {
		ConditionalPersona tmp_persona = rules_.getConditionalPersona(ref);
		vector<boost::shared_ptr<ConditionalAction> > actions = tmp_persona.getActions();
		vector<boost::shared_ptr<ConditionalAction> >::iterator aiter = actions.begin();
		vector<boost::shared_ptr<ConditionalAction> >::iterator aend = actions.end();
		for (; aiter != aend; ++aiter) {
			clock_t current_time = clock();
			double elapsed = (current_time - start_time) / (double) CLOCKS_PER_SEC;

			vector<DrtPred> cons = (*aiter)->getCons();
			clause_vector clause = (*aiter)->getClause();

			if(elapsed > remaining_time)
				break;

			string link = (*aiter)->getLink();
			string text = (*aiter)->getText();
			
			RulesCandidate tmp_candidate(boost::make_tuple(cons, clause, link, text));
			if (!shortfind(candidates, tmp_candidate))
				candidates.push_back(tmp_candidate);
		}
	} catch (std::runtime_error &e) {
		///
	}

	return candidates;
}


DrtVect Knowledge::getFullDrs(DrtVect drs)
{

	vector<DrtPred> drs_tmp(drs);
	string verb_ref = "";
	for (int ncycle = 0; ncycle < 3; ++ncycle) { // a specification can have specifications as well
		for (int m = 0; m < drs_tmp.size(); ++m) {
			if (drs.at(m).is_name()) {
				string ref_action = extract_first_tag(drs_tmp.at(m));
				vector<vector<DrtPred> > specs = this->getSpecifications(ref_action);
				for (int n = 0; drs.size() < max_items && n < specs.size() && n < max_specs; ++n) {
					vector<DrtPred> single_spec = specs.at(n);
					if (single_spec.size() && !shortfind(drs, single_spec.at(0)))
						drs.insert(drs.end(), single_spec.begin(), single_spec.end());
				}
			}
			if (drs.at(m).is_verb()) {
				string ref_action = extract_subject(drs_tmp.at(m));
				string vref_action = extract_first_tag(drs_tmp.at(m));
				vector<vector<DrtPred> > complements = this->getComplements(ref_action, vref_action);
				for (int n = 0; drs.size() < max_items && n < complements.size(); ++n) {
					vector<DrtPred> single_compl = complements.at(n);
					if (single_compl.size() && !shortfind(drs, single_compl.at(0)))
						drs.insert(drs.end(), single_compl.begin(), single_compl.end());
				}
			}
			if (verb_ref == "" && drs.at(m).is_verb())
				verb_ref = extract_first_tag(drs.at(m));
		}
		drs_tmp = drs;
	}

	//add the subordinates to the action
	vector<string> verb_refs;
	verb_refs.push_back(verb_ref);
	int ncycle = 0;
	vector<string> already_parsed;
	for (int n = 0; n < verb_refs.size() && ncycle < 5; ++n, ++ncycle) { // subordinates can be nested
		string vref = verb_refs.at(n);
		if (shortfind(already_parsed, vref))
			continue;
		vector<pair<DrtPred, Action> > subordinates = personae_.getSubordinates(vref);
		for (int m = 0; drs.size() < max_items && m < subordinates.size() && m < max_subordinates; ++m) {

			DrtPred type_pred = subordinates.at(m).first;
			DrtVect subord = subordinates.at(m).second.getDrs();
			drs.push_back(type_pred);
			drs.insert(drs.end(), subord.begin(), subord.end());
			string fref = subordinates.at(m).second.getRef();
			verb_refs.push_back(fref);
		}
		// add the adverbs to the action
		vector<DrtPred> adverbs = personae_.getAdverbs(vref);
		drs.insert(drs.end(), adverbs.begin(), adverbs.end());
		already_parsed.push_back(vref);
	}

	// Specifications of the subordinates
	for (int ncycle = 0; ncycle < 3; ++ncycle) { // a specification can have specifications as well
		for (int m = 0; m < drs_tmp.size(); ++m) {
			if (drs.at(m).is_name()) {
				string ref_action = extract_first_tag(drs_tmp.at(m));
				vector<vector<DrtPred> > specs = this->getSpecifications(ref_action);
				for (int n = 0; drs.size() < max_items && n < specs.size() && n < max_specs; ++n) {
					vector<DrtPred> single_spec = specs.at(n);
					if (single_spec.size() && !shortfind(drs, single_spec.at(0)))
						drs.insert(drs.end(), single_spec.begin(), single_spec.end());
				}
			}
			if (drs.at(m).is_verb()) {
				string ref_action = extract_subject(drs_tmp.at(m));
				string vref_action = extract_first_tag(drs_tmp.at(m));
				vector<vector<DrtPred> > complements = this->getComplements(ref_action, vref_action);
				for (int n = 0; drs.size() < max_items && n < complements.size(); ++n) {
					vector<DrtPred> single_compl = complements.at(n);
					if (single_compl.size() && !shortfind(drs, single_compl.at(0)))
						drs.insert(drs.end(), single_compl.begin(), single_compl.end());
				}
			}
		}
		drs_tmp = drs;
	}

	return drs;
}

DrtVect Knowledge::getFullDrsForWriting(DrtVect drs)
{
	vector<DrtPred> drs_tmp(drs);
	string verb_ref = "";
	for (int ncycle = 0; ncycle < 3; ++ncycle) { // a specification can have specifications as well
		for (int m = 0; m < drs_tmp.size(); ++m) {
			if (drs.at(m).is_name() && !drs.at(m).is_anaphora() ) {
				string ref_action = extract_first_tag(drs_tmp.at(m));
				vector<vector<DrtPred> > specs = this->getSpecifications(ref_action);
				for (int n = 0; n < specs.size(); ++n) {
					vector<DrtPred> single_spec = specs.at(n);
					if (single_spec.size() && !shortfind(drs, single_spec.at(0)))
						drs.insert(drs.end(), single_spec.begin(), single_spec.end());
				}
			}
			if (drs.at(m).is_verb()) {
				string ref_action = extract_subject(drs_tmp.at(m));
				string vref_action = extract_first_tag(drs_tmp.at(m));
				vector<vector<DrtPred> > complements = this->getComplements(ref_action, vref_action);
				for (int n = 0; n < complements.size(); ++n) {
					vector<DrtPred> single_compl = complements.at(n);
					if (single_compl.size() && !shortfind(drs, single_compl.at(0)))
						drs.insert(drs.end(), single_compl.begin(), single_compl.end());
				}
			}
			if (verb_ref == "" && drs.at(m).is_verb())
				verb_ref = extract_first_tag(drs.at(m));
		}
		drs_tmp = drs;
	}

	//add the subordinates to the action
	vector<string> verb_refs;
	verb_refs.push_back(verb_ref);
	int ncycle = 0;
	vector<string> already_parsed;
	for (int n = 0; n < verb_refs.size() && ncycle < 5; ++n, ++ncycle) { // subordinates can be nested
		string vref = verb_refs.at(n);
		if (shortfind(already_parsed, vref))
			continue;
		vector<pair<DrtPred, Action> > subordinates = personae_.getSubordinates(vref);
		for (int m = 0; m < subordinates.size(); ++m) {
			DrtPred type_pred = subordinates.at(m).first;
			vector<DrtPred> subord = subordinates.at(m).second.getDrs();
			drs.push_back(type_pred);
			drs.insert(drs.end(), subord.begin(), subord.end());
			string fref = subordinates.at(m).second.getRef();
			verb_refs.push_back(fref);
		}
		// add the adverbs to the action
		vector<DrtPred> adverbs = personae_.getAdverbs(vref);
		drs.insert(drs.end(), adverbs.begin(), adverbs.end());
		already_parsed.push_back(vref);
	}

	// Specifications of the subordinates
	for (int ncycle = 0; ncycle < 3; ++ncycle) { // a specification can have specifications as well
		for (int m = 0; m < drs_tmp.size(); ++m) {
			if (drs.at(m).is_name() && !drs.at(m).is_anaphora() ) {
				string ref_action = extract_first_tag(drs_tmp.at(m));
				vector<vector<DrtPred> > specs = this->getSpecifications(ref_action);
				for (int n = 0; n < specs.size(); ++n) {
					vector<DrtPred> single_spec = specs.at(n);
					if (single_spec.size() && !shortfind(drs, single_spec.at(0)))
						drs.insert(drs.end(), single_spec.begin(), single_spec.end());
				}
			}
			if (drs.at(m).is_verb()) {
				string ref_action = extract_subject(drs_tmp.at(m));
				string vref_action = extract_first_tag(drs_tmp.at(m));
				vector<vector<DrtPred> > complements = this->getComplements(ref_action, vref_action);
				for (int n = 0; n < complements.size(); ++n) {
					vector<DrtPred> single_compl = complements.at(n);
					if (single_compl.size() && !shortfind(drs, single_compl.at(0)))
						drs.insert(drs.end(), single_compl.begin(), single_compl.end());
				}
			}
		}
		drs_tmp = drs;
	}

	return drs;
}


static bool has_only_broken(const DrtVect &drtvect)
{
	for (int n = 0; n < drtvect.size(); ++n) {
		string fref = extract_first_tag(drtvect.at(n));
		if (fref.find("broken") == string::npos)
			return false;
	}
	return true;
}

static bool has_some_broken(const DrtVect &drtvect)
{
	for (int n = 0; n < drtvect.size(); ++n) {
		string fref = extract_first_tag(drtvect.at(n));
		if (fref.find("broken") != string::npos)
			return true;
	}
	return false;
}

static inline int max(int a, int b)
{
	return a < b ? b : a;
}
static inline int min(int a, int b)
{
	return a >= b ? b : a;
}

static void launch_matching_thread(KResult *results, Match *match, DrtVect &question, const Candidate &candidate)
{
	MatchSubstitutions msubs;
	DrtVect tmp_pred(candidate.get<0>());
	double w;
	if (has_some_broken(question)) {
		bool match_names = true;

		w = match->singlePhraseMatch(tmp_pred, question, &msubs, match_names);
	} else if(!has_only_broken(question)) {
		bool match_names = true;
		w = match->singlePhraseMatch(tmp_pred, question, &msubs, match_names);
	} else {
		bool match_names = false;
		w = match->singlePhraseMatch(tmp_pred, question, &msubs, match_names);
	}
	if (w != 0) {
		*results = boost::make_tuple(msubs, w, candidate.get<1>(), candidate.get<2>(), candidate.get<3>());
	} else {
		*results = boost::make_tuple(msubs, -1, candidate.get<1>(), candidate.get<2>(), candidate.get<3>());
	}
}

static vector<KResult> clean_results(const vector<KResult> &results)
{
	vector<KResult> to_return;
	for (int n = 0; n < results.size(); ++n) {
		if (results.at(n).get<1>() > 0) {
			to_return.push_back(results.at(n));
		}
	}
	return to_return;
}

static vector<KRuleResult> clean_rule_results(const vector<KRuleResult> &results)
{
	vector<KRuleResult> to_return;
	for (int n = 0; n < results.size(); ++n) {
		if (results.at(n).get<2>() > 0) {
			to_return.push_back(results.at(n));
		}
	}
	return to_return;
}


class KThreadCounter {
	int num_, max_num_;
	clock_t time_start_;
	double max_time_;

public:
	KThreadCounter(int max_num,
			double max_time // number of seconds after which the counter returns -1
			) :
			max_num_(max_num), num_(0), max_time_(max_time)
	{
		time_start_= clock();
	}
	int getCurrentNumber();
	double getRemainingTime();
};

double KThreadCounter::getRemainingTime()
{
	clock_t current_time = clock();
	double elapsed = (current_time - time_start_) / (double) CLOCKS_PER_SEC;
	return max_time_-elapsed;
}

int KThreadCounter::getCurrentNumber()
{
	boost::mutex::scoped_lock lock(io_mutex_kcounter); // this method is used concurrently
	clock_t current_time = clock();
	double elapsed = (current_time - time_start_) / (double) CLOCKS_PER_SEC;

	if(elapsed > max_time_)
		return -1;
	if (num_ < max_num_) {
		++num_;
		return num_ - 1;
	}
	return -1;
}

class ParserKThread {

	Knowledge *k_;
	vector<KResult> *kresults_;
	DrtVect question_;
	vector<Candidate> *candidates_;
	KThreadCounter *counter_;

	void launchMatchingThread(KResult *results, Match *match, DrtVect &question, Candidate candidate);

public:
	ParserKThread(vector<KResult> *kresults, Knowledge *k, DrtVect &question, vector<Candidate> *candidates, KThreadCounter *counter) :
		kresults_(kresults), k_(k), question_(question), candidates_(candidates), counter_(counter)
	{
	}

	void operator()();
};

void ParserKThread::operator()()
{
	int num = 0;
	while (num < candidates_->size() && num < kresults_->size()) {
		num = counter_->getCurrentNumber();

		if (num == -1)
			break;

		KResult *out = &kresults_->at(num);
		Candidate in = candidates_->at(num);
		Match match(k_);
		this->launchMatchingThread(out, &match, question_, in);
	}
}

static pair<DrtVect, DrtMgu> break_off_specifications(DrtVect drtvect)
{
	DrtMgu mgu;
	for (int n = 0; n < drtvect.size(); ++n) {
		string fref = extract_first_tag(drtvect.at(n));
		if (drtvect.at(n).is_complement() && ref_is_name(fref)) {
			string old_ref = fref;
			string new_ref = string("_[broken_off]_") + boost::lexical_cast<string>(n) + "_" + fref;
			implant_first(drtvect.at(n), new_ref);
			mgu.add(old_ref, new_ref);
		}
	}
	return make_pair(drtvect, mgu);
}

static pair<DrtVect, DrtMgu> break_off_complements(DrtVect drtvect)
{
	DrtMgu mgu;
	for (int n = 0; n < drtvect.size(); ++n) {
		string fref = extract_first_tag(drtvect.at(n));
		if (drtvect.at(n).is_complement() && ref_is_verb(fref)) {
			string old_ref = fref;
			string new_ref = string("_[broken_off]_") + boost::lexical_cast<string>(n) + "_" + fref;
			implant_first(drtvect.at(n), new_ref);
			mgu.add(old_ref, new_ref);
		}
	}
	return make_pair(drtvect, mgu);
}


void ParserKThread::launchMatchingThread(KResult *results, Match *match, DrtVect &question, const Candidate candidate)
{
	MatchSubstitutions msubs;
	DrtVect tmp_pred(candidate.get<0>());
	double w;

	clock_t start;

	if (has_some_broken(question)) {// && !has_only_broken(question) ) {
		bool match_names = true;
		w = match->singlePhraseMatch(tmp_pred, question, &msubs, match_names);
	} else {
		bool match_names = false;
		w = match->singlePhraseMatch(tmp_pred, question, &msubs, match_names);
	}
	if (w != 0) {
		*results = boost::make_tuple(msubs, w, candidate.get<1>(), candidate.get<2>(), candidate.get<3>());
	} else {
		*results = boost::make_tuple(msubs, -1, candidate.get<1>(), candidate.get<2>(), candidate.get<3>());
	}
}

class CandidatesKThread {
	Knowledge *k_;
	vector<vector<Candidate> > *results_;
	vector<SPAction> *all_refs_;
	KThreadCounter *counter_;

public:
	CandidatesKThread(vector<vector<Candidate> > *results, Knowledge *k, vector<SPAction> *all_refs, KThreadCounter *counter) :
		results_(results), k_(k), all_refs_(all_refs), counter_(counter)
	{
	}

	void operator()();
};

void CandidatesKThread::operator()()
{
	int num = 0;
	while (num < all_refs_->size() && num < results_->size()) {
		num = counter_->getCurrentNumber();
		if (num == -1)
			break;
		vector<Candidate> *out = &results_->at(num);
		SPAction in = all_refs_->at(num);
		*out = k_->getCandidate(in, counter_->getRemainingTime() );
	}
}

vector<KResult> Knowledge::findMatch(vector<SPAction> &all_refs, vector<DrtPred> &question)
{
	vector<KResult> results;

	clock_t start;

	int max_threads = 1;
	Parameters *par = parameters_singleton::instance();
	if(commercial_version)
		max_threads = par->getNumThreads();
	int num_results= min(max_candidates_refs_,all_refs.size() );
	int num_threads = min(num_results, max_threads);

	double max_time = min(fixed_time_, wi_.getTimeout()) ; // + threads_time/num_threads; // The max time for the operation to finish

	vector<vector<Candidate> > tmp_results0(num_results);
	KThreadCounter counter0(num_results,max_time);
	vector<CandidatesKThread> pt0_vect(num_threads, CandidatesKThread(&tmp_results0, this, &all_refs, &counter0) );
	boost::thread_group g0;
	for (int t = 0; t < num_threads; ++t) {
		g0.create_thread( pt0_vect.at(t) );
	}
	g0.join_all();

	vector<Candidate> all_candidates;
	int n = 0;
	for (int m=0; m < tmp_results0.size() && n < max_candidates_; ++m) {
		vector<Candidate> candidates = tmp_results0.at(m);
		n += candidates.size();
		all_candidates.insert(all_candidates.end(), candidates.begin(), candidates.end());
	}

	max_time = min(fixed_time_, wi_.getTimeout()) ;
	num_threads = min(all_candidates.size(), max_threads);
	boost::thread_group g;
	vector<KResult> tmp_results(all_candidates.size());
	KThreadCounter counter(all_candidates.size(),max_time);
	vector<ParserKThread> pt_vect(num_threads, ParserKThread(&tmp_results, this, question, &all_candidates, &counter) );
	for (int t = 0; t < num_threads; ++t) {
		g.create_thread(pt_vect.at(t));
	}
	g.join_all();
	tmp_results = clean_results(tmp_results);
	results.insert(results.end(), tmp_results.begin(), tmp_results.end());


	return results;
}

vector<KRuleResult> Knowledge::find_rules_match(vector<boost::tuple<vector<DrtPred>, clause_vector, string, string> > &data,
		vector<DrtPred> &question)
{
	vector<KRuleResult> results;

	vector<boost::tuple<vector<DrtPred>, clause_vector, string, string> >::iterator diter = data.begin();
	vector<boost::tuple<vector<DrtPred>, clause_vector, string, string> >::iterator dend = data.end();
	Match match(this);
	int n = 0;
	for (; diter != dend && n < 400; ++diter, ++n) {
		MatchSubstitutions msubs;
		vector<DrtPred> tmp_pred(diter->get<0>());
		double w = match.singlePhraseMatch(tmp_pred, question, &msubs);
		if (w != 0) {
			results.push_back(boost::make_tuple(diter->get<1>(), msubs, w, diter->get<2>(), diter->get<3>()));
		}
	}
	return results;
}

static bool operator ==(const Candidate &lhs, const Candidate &rhs)
{
	DrtVect lvect = lhs.get<0>();
	DrtVect rvect = rhs.get<0>();

	if (lhs.get<0>() == rhs.get<0>() && lhs.get<1>() == rhs.get<1>() && lhs.get<2>() == rhs.get<2>())
		return true;
	return false;
}

static bool has_subject(const DrtPred &pred)
{
	string subj = extract_subject(pred);
	if (subj.find("subj") == string::npos)
		return true;
	return false;
}

template<typename T>
void candidate_unique(vector<T> &c)
{
	vector<T> already_present;

	for (int n = 0; n < c.size(); ++n) {
		if (std::find(already_present.begin(), already_present.end(), c.at(n)) == already_present.end()) {
			already_present.push_back(c.at(n));
		} else {
			c.erase(c.begin() + n);
			--n;
		}
	}
}

static bool is_subject_of_verb(const DrtVect &dvect, const string &ref)
{
	for (int n = 0; n < dvect.size(); ++n) {
		if (dvect.at(n).is_verb()) {
			string sref = extract_subject(dvect.at(n));
			if (ref == sref)
				return true;
		}
		if (dvect.at(n).is_complement()) {
			string header = extract_header(dvect.at(n));
			if(header == "@AND" || header == "@OR") {
				string fref = extract_first_tag(dvect.at(n));
				string sref = extract_second_tag(dvect.at(n));
				if(ref == sref) {
					if(is_subject_of_verb(dvect,fref) )
						return true;
				}
			}
		}
	}

	return false;
}

static bool is_object_of_verb(const DrtVect &dvect, const string &ref)
{
	for (int n = 0; n < dvect.size(); ++n) {
		if (dvect.at(n).is_verb()) {
			string oref = extract_object(dvect.at(n));
			if (ref == oref)
				return true;
		}
		if (dvect.at(n).is_complement()) {
			string header = extract_header(dvect.at(n));
			if(header == "@AND" || header == "@OR") {
				string fref = extract_first_tag(dvect.at(n));
				string sref = extract_second_tag(dvect.at(n));
				if(ref == sref) {
					if(is_object_of_verb(dvect,fref) )
						return true;
				}
			}
		}
	}

	return false;
}


static bool is_subject_of_first_verb(const DrtVect &dvect, const string &ref)
{
	for (int n = 0; n < dvect.size(); ++n) {
		if (dvect.at(n).is_verb()) {
			string sref = extract_subject(dvect.at(n));
			if (ref == sref)
				return true;
			else
				return false;
		}
	}

	return false;
}

static bool is_object_of_first_verb(const DrtVect &dvect, const string &ref)
{
	for (int n = 0; n < dvect.size(); ++n) {
		if (dvect.at(n).is_verb()) {
			string oref = extract_object(dvect.at(n));
			if (ref == oref)
				return true;
			else
				return false;
		}
	}

	return false;
}


static bool verb_has_subject(const DrtVect &dvect, const DrtPred &pred)
{
	if (pred.is_verb()) {
		string sref = extract_subject(pred);
		int m = find_element_with_string(dvect, sref);
		if (m != -1)
			return true;
		return false;
	}

	return false;
}

static bool verb_has_object(const DrtVect &dvect, const DrtPred &pred)
{
	if (pred.is_verb()) {
		string oref = extract_object(pred);
		int m = find_element_with_string(dvect, oref);
		if (m != -1)
			return true;
		return false;
	}

	return false;
}

static bool has_subject(const DrtVect &dvect)
{
	for (int n = 0; n < dvect.size(); ++n) {
		if (dvect.at(n).is_verb()) {
			string sref = extract_subject(dvect.at(n));
			int m = find_element_with_string(dvect, sref);
			if (m != -1)
				return true;
			return false;
		}
	}

	return false;
}

static vector<Candidate> candidate_intersection(const vector<Candidate> &c1, const vector<Candidate> &c2, bool has_what,
		bool lacks_subj)
{
	vector<Candidate> to_return;

	if (c1.size() && !c2.size())
		return c1;
	if (c2.size() && !c1.size())
		return c2;

	for (int n = 0; n < c2.size(); ++n) {
		DrtVect tmpvect = c2.at(n).get<0>();
	}

	for (int n = 0; n < c1.size(); ++n) {
		DrtVect tmpvect = c1.at(n).get<0>();
		if (shortfind(c2, c1.at(n))) {
			to_return.push_back(c1.at(n));
		}
	}

	// phrases with no subject are added anyway
	metric *d = metric_singleton::get_metric_instance();
	for (int n = 0; n < c1.size(); ++n) {
		if (!has_subject(c1.at(n).get<0>()) || has_what || lacks_subj)
			to_return.push_back(c1.at(n)); // add the candidates selected looking at the verbs.
		else {
			DrtVect drtvect = c1.at(n).get<0>();
			for (int m = 0; m < drtvect.size(); ++m) {
				if (drtvect.at(m).is_verb()) {
					vector<DrtPred> subjs = find_subject_of_verb(drtvect, m);
					int j;
					for (j = 0; j < subjs.size(); ++j) {
						string head = extract_header(subjs.at(j));
						string levin = d->get_levin_noun(head);
						if (levin == "") { // subjects with no levin count as no subjects
							to_return.push_back(c1.at(n));
							break;
						}
					}
					if (j != subjs.size()) // a levin-less subj has been found
						break;
				}
			}
		}
	}

	return to_return;
}

static vector<string> erase_impossible_synonyms(const string &orig_str, const vector<string> &names)
{
	vector<string> to_return;
	to_return.push_back(orig_str);
	metric *d = metric_singleton::get_metric_instance();
	bool orig_is_a_who = matching::is_who(orig_str);
	for(int n=0; n < names.size(); ++n) {
		string name= names.at(n);
		bool name_is_a_plant= d->hypernym_dist(name,"plant", 5) > 0.4;
		bool name_is_a_body_part= d->hypernym_dist(name,"organ", 5) > 0.4;
		name_is_a_body_part= name_is_a_body_part  | ( d->hypernym_dist(name,"body_part", 5) > 0.4 );
		// george_bush -/> bush; einstein -/> brain
		if( name != orig_str
			&& !(orig_is_a_who
				&& (name_is_a_plant || name_is_a_body_part)
			)
			&& orig_str.find(name) == string::npos // george_washington -/> washington
		) {
			to_return.push_back(name);
		}
	}

	return to_return;
}

static vector<string> erase_impossible_synonyms_verb(const string &orig_str, const vector<string> &names)
{
	vector<string> to_return;
	to_return.push_back(orig_str);
	bool orig_is_a_who = matching::is_who(orig_str);
	for(int n=0; n < names.size(); ++n) {
		string name= names.at(n);

		if( orig_str == "live"
			&& (name == "travel" || name == "move" || name == "go")
		)
			continue;

		if( orig_str == "die"
			&& (name == "go")
		)
			continue;
		if( orig_str == "take"
			&& (name == "shoot")
		)
			continue;
		if( orig_str == "shoot"
			&& (name == "take")
		)
			continue;

		to_return.push_back(name);

	}

	return to_return;
}


void Knowledge::find_all_candidates(DrtVect &question, CandidatesStruct *cstruct)
{
	vector<DrtPred>::const_iterator qiter = question.begin();
	vector<DrtPred>::const_iterator qend = question.end();
	vector<boost::shared_ptr<Action> > prev_action_pointers_verb, prev_action_pointers_noun;
	//vector<Action*> prev_action_pointers_verb, prev_action_pointers_noun;
	metric *d = metric_singleton::get_metric_instance();

	bool has_prior_verb = false;
	bool is_new;

	// Finds the candidates for the match with the answers
	for (; qiter != qend; ++qiter) { // find the upgs for the answer
		is_new= true;
		if(measure_time || debug) {
			cout << "FTIMEOUT::: " << wi_.getTimeout() << endl;
		}
		if(wi_.getTimeout() - timeout_prediction < 0)
			break;
		if (qiter->is_verb()
			&& !has_prior_verb
				) {
			has_prior_verb = true; // only the first verb must be considered
			vector<string> strings;
			MapStVSt::iterator miter;
			string qstr = extract_header(*qiter);

			if (!verb_has_subject(question, *qiter))
				cstruct->setLacksSubj(true);

			if (!verb_has_object(question, *qiter))
				cstruct->setLacksObj(true);

			vector<string> alt_verbs;
			alt_verbs.push_back("do");
			alt_verbs.push_back("happen");
			alt_verbs.push_back("occur");

			strings.push_back(qstr);
			vector<string> hyponyms;
			for(int ns=0; ns < strings.size(); ++ns) {
				string ns_string= strings.at(ns);
				vector<string> hyponyms_tmp = d->get_hyponyms_of_verb(ns_string, 2);
				hyponyms.insert(hyponyms.end(),hyponyms_tmp.begin(),hyponyms_tmp.end() );
			}
			strings.insert(strings.end(),hyponyms.begin(),hyponyms.end() );
			strings = erase_impossible_synonyms_verb(qstr, strings);
			for (int n = 0; n < strings.size(); ++n) {
				string verb_str = strings.at(n);

				string verb_levin = verb_str;
				if (find(alt_verbs.begin(), alt_verbs.end(), verb_str) != alt_verbs.end()) {
					miter = verb_refs_.find(verb_str); // search for the string name only for "generic verbs"
				} else
					miter = verb_refs_.find(verb_levin);
				if (miter != verb_refs_.end()) {
					vector<SPAction> ref_str = miter->second;
					if(max_refs_ < ref_str.size())
						ref_str.resize(max_refs_);
					cstruct->setLacksVerb(false);
					cstruct->addVerbRefs(ref_str);
				}
			}
		}
		if (qiter->is_name() && !qiter->is_number()) {
			vector<string> strings;
			string qstr = extract_header(*qiter);
			string fref = extract_first_tag(*qiter);
			boost::split(strings, qstr, boost::is_any_of("|"));
			strings.push_back("[any]");
			vector<string> similar_names = get_similar_names_for_data(qstr);
			strings.insert(strings.end(),similar_names.begin(),similar_names.end() );
			vector<string> synonyms;

			if(wi_.getUseSynonyms() ) { // finding synomyms is faster than finding hyponyms
				synonyms = d->get_synonyms_of_noun(qstr);
				synonyms = erase_impossible_synonyms(qstr, synonyms);
				strings.insert(strings.end(),synonyms.begin(),synonyms.end() );
			} else if(wi_.getUseHyponyms() ){
				vector<string> hyponyms;
				for(int ns=0; ns < strings.size(); ++ns) {
					string ns_string= strings.at(ns);
					vector<string> hyponyms_tmp = d->get_hyponyms_of_noun(ns_string, wi_.getNumHyponyms() );
					hyponyms.insert(hyponyms.end(),hyponyms_tmp.begin(),hyponyms_tmp.end() );
				}
				strings.insert(strings.end(),hyponyms.begin(),hyponyms.end() );
			}

			// The following adds too many results
			if(wi_.getUsePertaynims()) {
				vector<string> pertainyms;
				for(int ns=0; ns < strings.size(); ++ns) {
					string ns_string= strings.at(ns);
					vector<string> pertainyms_tmp = d->get_reverse_pertainyms(ns_string);
					pertainyms.insert(pertainyms.end(),pertainyms_tmp.begin(),pertainyms_tmp.end() );
				}
				strings.insert(strings.end(),pertainyms.begin(),pertainyms.end() );
			}
			for (int n = 0; n < strings.size(); ++n) {
				MapStVSt::iterator miter;
				string name_str = strings.at(n);
				string name_levin = name_str;
				vector<SPAction> ref_str;
				if (is_subject_of_first_verb(question, fref) && !is_broken(fref)) {
					miter = subj_name_refs_.find(name_levin);
					if (miter != subj_name_refs_.end()) {
						vector<SPAction> ref_str2= miter->second;
						ref_str.insert(ref_str.end(), ref_str2.begin(), ref_str2.end());
					}
				} else if (is_object_of_first_verb(question, fref) && !is_broken(fref)) {
					miter = obj_name_refs_.find(name_levin);
					if (miter != obj_name_refs_.end()) {
						vector<SPAction> ref_str2= miter->second;
						ref_str.insert(ref_str.end(), ref_str2.begin(), ref_str2.end());
					}
				} else if (!is_broken(fref)) {
					miter = name_refs_.find(name_levin);
					if (miter != name_refs_.end()) {
						vector<SPAction> ref_str2= miter->second;
						ref_str.insert(ref_str.end(), ref_str2.begin(), ref_str2.end());
					}
				}
				if (is_broken(fref)) {
					miter = subj_name_refs_.find(name_levin);
					if (miter != subj_name_refs_.end()) {
						vector<SPAction> ref_str2= miter->second;
						ref_str.insert(ref_str.end(), ref_str2.begin(), ref_str2.end());
					}
					miter = obj_name_refs_.find(name_levin);
					if (miter != obj_name_refs_.end()) {
						vector<SPAction>  ref_str2= miter->second;
						ref_str.insert(ref_str.end(), ref_str2.begin(), ref_str2.end());
					}
					miter = name_refs_.find(name_levin);
					if (miter != name_refs_.end()) {
						vector<SPAction> ref_str2= miter->second;
						ref_str.insert(ref_str.end(), ref_str2.begin(), ref_str2.end());
					}
				}

				if (ref_str.size()) {
					if(max_refs_ < ref_str.size())
						ref_str.resize(max_refs_);
					if (is_subject_of_first_verb(question, fref)) {
						cstruct->addNewSubjRefs(ref_str, is_new);
					} else if (is_object_of_first_verb(question, fref))
						cstruct->addNewObjRefs(ref_str,is_new);
					else {
						if(debug)
							cout << "NAME_REFS2::: " << name_levin << endl;
						cstruct->addNewNameRefs(ref_str,is_new);
					}
				}
				is_new = false;
			}
		}
		if (qiter->is_complement()) {
			vector<string> strings;
			string qstr = extract_header(*qiter);
			bool is_WRB = qiter->tag() == "WRB";// WRB in questions do not need a name to intersect with

			///
			if (qstr.find("@QUANTITY") != -1)
				is_WRB = false;
			if (qstr.find("@SUBORD") != -1)
				is_WRB = false;
			///

			boost::split(strings, qstr, boost::is_any_of("|"));
			for (int n = 0; n < strings.size(); ++n) {
				MapStVSt::iterator miter;
				string name_str = strings.at(n);
				string name_levin = name_str;
				miter = complement_refs_.find(name_levin);
				if (miter != complement_refs_.end()) {
					vector<SPAction> ref_str = miter->second;
					if(max_refs_ < ref_str.size())
						ref_str.resize(max_refs_);
					cstruct->addNewComplRefs(ref_str, is_new);
					if(is_WRB) {
						cstruct->addNewNameRefs(ref_str, is_new);
					}
				}
				is_new = false;
			}
		}
	}

	//cstruct->sort(); // done in ::computeIntersection()

	// the return values are stored in the two vector<Candidate>
}



static vector<SPAction> refs_intersection(CandidatesStruct &cstruct)
{
	cstruct.computeIntersection();

	vector<SPAction> verb_refs  = cstruct.getVerbRefs();
	vector<SPAction> subj_refs  = cstruct.getSubjRefs();
	vector<SPAction> obj_refs   = cstruct.getObjRefs();
	vector<SPAction> name_refs  = cstruct.getNameRefs();
	vector<SPAction> compl_refs = cstruct.getComplRefs();
	bool lacks_verb    = cstruct.getLacksVerb();
	bool lacks_subject = cstruct.getLacksSubj();
	bool lacks_object  = cstruct.getLacksObj();

	vector<SPAction> all_refs1, all_refs2, all_refs3;

	if(debug || show_candidates)
		cout << "INTERS_LACK::: " << lacks_subject << " " << lacks_verb << " " << lacks_object << endl;

	if (!lacks_subject && !lacks_verb) { // the question has a subject and a verb -> intersect the subject and verb in the knowledge
		all_refs1 = sort_intersect(verb_refs, subj_refs);
		if (!lacks_object) {
			all_refs1 = sort_intersect(all_refs1, obj_refs);
		}
	} else if (!lacks_object && !lacks_verb) { // the question has an object and a verb -> intersect the object and verb in the knowledge
		all_refs1 = sort_intersect(verb_refs, obj_refs);
	}
	if( !lacks_subject && !lacks_verb && all_refs1.size() == 0)
		return vector<SPAction>();
	if( lacks_subject && !lacks_object && !lacks_verb && all_refs1.size() == 0)
		return vector<SPAction>();
	if ((lacks_subject || lacks_object) && !lacks_verb && all_refs1.size() == 0) // if the question has a verb and no subj or obj, use the verb refs
		all_refs1 = verb_refs;
	if (lacks_verb && verb_refs.size() == 0) // if there is no verb in the question then all_refs1 only contains references to names
		all_refs1 = name_refs;
	if( !lacks_verb && verb_refs.size() == 0) // if the question has a verb that is nowhere in the data then return a void vector<string>.
		return vector<SPAction>();
	// the question has complement -> intersect the complements with the all_refs1
	if (compl_refs.size()) {
		all_refs2 = sort_intersect(all_refs1, compl_refs);
	}

	if (all_refs2.size() == 0)  // there is no intersection with the complement
		all_refs3 = name_refs;
	else {
		// the complements must have names
		all_refs3 = sort_intersect(all_refs2, name_refs);
	}

	if (all_refs3.size() == 0) // if the complements don't have names, use all_refs1
		if(all_refs2.size() == 0)
			all_refs3 = all_refs1;
		else
			all_refs3 = all_refs2;

	return all_refs3;
}

static vector<string> rules_refs_intersection(const RulesCandidatesStruct &cstruct)
// only intersects verbs with complements
{
	vector<string> verb_refs  = cstruct.getVerbRefs();
	vector<string> compl_refs = cstruct.getComplRefs();
	bool lacks_verb    = cstruct.getLacksVerb();
	if(lacks_verb)
		return compl_refs;
	vector<string> all_refs1, all_refs2, all_refs3;
	all_refs1= verb_refs;
	if (compl_refs.size()) {
		all_refs2 = sort_intersect(all_refs1, compl_refs);
	}
	if (all_refs2.size() == 0) { // there is no intersection with the complement
		all_refs3 = all_refs1;
	} else {
		all_refs3 = all_refs2;
	}
	return all_refs3;
}


static int find_verb_with_subject(const DrtVect &pre_drt, string from_str)
{
	for (int n = 0; n < pre_drt.size(); ++n) {
		string subj = extract_subject(pre_drt.at(n));
		if (pre_drt.at(n).is_verb() && subj == from_str) {
			return n;
		}
	}
	return -1;
}

static int find_verb_with_object(const DrtVect &pre_drt, string from_str)
{
	for (int n = 0; n < pre_drt.size(); ++n) {
		string obj = extract_object(pre_drt.at(n));
		if (pre_drt.at(n).is_verb() && obj == from_str) {
			return n;
		}
	}
	return -1;
}


typedef boost::tuple<string,string,string,vector<string> > QElements; // headers of subject, object, verb, names

QElements get_question_elements(const DrtVect &question)
{
	QElements to_return;

	string subj, obj, verb;
	vector<string> names;
	for(int n=0; n < question.size(); ++n) {
		if(question.at(n).is_verb() ) {
			verb = extract_header (question.at(n));
			string sref = extract_subject(question.at(n));
			string oref = extract_object (question.at(n));
			int ms = find_element_with_string(question,sref);
			int mo = find_element_with_string(question,oref);
			if(ms != -1) {
				subj = extract_header(question.at(ms));
			}
			if(mo != -1) {
				obj = extract_header(question.at(mo));
			}
			break;
		}
	}
	for(int n=0; n < question.size(); ++n) {
		if(question.at(n).is_name() ) {
			string header= extract_header(question.at(n));
			if(header != subj && header != obj) {
				names.push_back(header);
			}
		}
	}

	to_return.get<0>()= subj;
	to_return.get<1>()=  obj;
	to_return.get<2>()= verb;
	to_return.get<3>()= names;

	return to_return;
}

vector<pair<SPAction,double> > Knowledge::getRefWeights(const DrtVect &question, vector<SPAction> refs)
{
	vector<pair<SPAction,double> > to_return;

	string subj, obj, verb;
	vector<string> names;
	QElements question_elements = get_question_elements(question);

	subj = question_elements.get<0>();
	obj  = question_elements.get<1>();
	verb = question_elements.get<2>();
	names= question_elements.get<3>();

	for (int n=0; n < refs.size(); ++n) {
		double w = 1;
		double ws = get_weight_map_element(subj_weigths_,make_pair(refs.at(n),subj));
		double wo = get_weight_map_element( obj_weigths_,make_pair(refs.at(n), obj));
		double wv = get_weight_map_element(verb_weigths_,make_pair(refs.at(n),verb));
		double wn = 1;
		for (int m = 0; m < names.size(); ++m) {
			wn *= get_weight_map_element(name_weigths_,make_pair(refs.at(n),names.at(m)) );
		}
		w = ws * wo * wv * wn;
		to_return.push_back( make_pair(refs.at(n),w) );
	}

	return to_return;
}

static bool compare_refs(const pair<SPAction,double> &lhs, const pair<SPAction,double> &rhs)
{
	return lhs.second > rhs.second;
}

static bool compare_rules_refs(const pair<string,double> &lhs, const pair<string,double> &rhs)
{
	return lhs.second > rhs.second;
}



static vector<SPAction> sort_refs(vector<pair<SPAction,double> > &refs_weights)
{
	vector<SPAction> to_return;

	std::sort(refs_weights.begin(),refs_weights.end(),compare_refs);

	for(int n=0; n < refs_weights.size(); ++n)
		to_return.push_back(refs_weights.at(n).first);

	return to_return;
}

static vector<string> sort_rules_refs(vector<pair<string,double> > &refs_weights)
{
	vector<string> to_return;

	std::sort(refs_weights.begin(),refs_weights.end(),compare_rules_refs);

	for(int n=0; n < refs_weights.size(); ++n)
		to_return.push_back(refs_weights.at(n).first);

	return to_return;
}



vector<pair<string,double> > Knowledge::getRulesRefWeights(const DrtVect &question, vector<string> refs)
{
	vector<pair<string,double> > to_return;

	string subj, obj, verb;
	vector<string> names;
	QElements question_elements = get_question_elements(question);

	subj = question_elements.get<0>();
	obj  = question_elements.get<1>();
	verb = question_elements.get<2>();
	names= question_elements.get<3>();

	for (int n=0; n < refs.size(); ++n) {
		double w = 1;
		double ws = get_rules_weight_map_element(rules_subj_weigths_,make_pair(refs.at(n),subj));
		double wo = get_rules_weight_map_element(rules_obj_weigths_ ,make_pair(refs.at(n), obj));
		double wv = get_rules_weight_map_element(rules_verb_weigths_,make_pair(refs.at(n),verb));
		double wn = 1;
		for (int m = 0; m < names.size(); ++m) {
			wn *= get_rules_weight_map_element(rules_name_weigths_,make_pair(refs.at(n),names.at(m)) );
		}
		w = ws * wo * wv * wn;
		to_return.push_back( make_pair(refs.at(n),w) );
	}

	return to_return;
}


vector<KnowledgeAnswer> Knowledge::getAnswers(drt &question_drt)
{
	vector<DrtPred> question = question_drt.predicates_with_references();
	QuestionList qlist = question_drt.getQuestionList();

	vector<DrtPred> ql = qlist.get();
	vector<Candidate> candidates, candidates_from_verbs, candidates_from_names;
	vector<SPAction> verb_refs, subj_refs, obj_refs, name_refs, compl_refs;

	//bool lacks_verb = true, lacks_subj = false, lacks_obj = false;

	clock_t start;
	if (debug || measure_time)
		start = clock();

	CandidatesStruct cstruct(wi_);
	cstruct.setLacksVerb(true);

	//find_all_candidates(question, verb_refs, subj_refs, obj_refs, name_refs, compl_refs, lacks_verb, lacks_subj, lacks_obj);
	this->find_all_candidates(question, &cstruct);
	if (debug || measure_time)
		start = clock();

	vector<SPAction> all_refs = refs_intersection(cstruct);



	// Find the DrtMgus by matching the candidates with the answers
	if (debug || measure_time)
		start = clock();
	vector<KResult> results;
	if(wi_.getTimeout() > 0)
		results = this->findMatch(all_refs, question);
	// Unifies with the question

	vector<KnowledgeAnswer> answers;
	for (int n = 0; n < results.size(); ++n) {
		bool to_add = true;

		vector<DrtPred> tmp_drs(question);
		MatchSubstitutions msubs = results.at(n).get<0>();
		DrtMgu upg_tmp = msubs.getDrtMgu();
		tmp_drs / upg_tmp;
		msubs.applySubstitutions(&tmp_drs);
		double w = results.at(n).get<1>();
		string link = results.at(n).get<2>();
		string text = results.at(n).get<3>();
		KnowledgeAnswer single_answer;

		// If the drs has a code to execute, add the answer only if the code returns true
		CodePred code = results.at(n).get<4>();
		if (!(code == CodePred())) {
			to_add = false;
			this->addTemporary(tmp_drs);
			code / upg_tmp;
			engine_.setKnowledge(this);
			CodePred result = engine_.run(code);
			if (result == Engine::pt_true)
				to_add = true;
			this->clearAllTemporary();
		}

		if (to_add) {
			single_answer.setPreds(tmp_drs);
			single_answer.setWeigth(w);
			single_answer.setLink(link);
			single_answer.setText(text);
			single_answer.setDrtMgu(upg_tmp);

			// Update the Question list
			QuestionList tmp_qlist(qlist);
			tmp_qlist / upg_tmp;
			tmp_qlist.applySubstitutions(msubs);

			single_answer.setQuestionList(tmp_qlist);

			answers.push_back(single_answer);
		}
	}

	return answers;
}

vector<KnowledgeAnswer> Knowledge::getAnswers(DrtVect &question)
{
	vector<Candidate> candidates, candidates_from_verbs, candidates_from_names;
	vector<SPAction> verb_refs, subj_refs, obj_refs, name_refs, compl_refs;

	CandidatesStruct cstruct(wi_);
	cstruct.setLacksVerb(true);

	this->find_all_candidates(question, &cstruct);

	vector<SPAction> all_refs = refs_intersection(cstruct);

	// Find the DrtMgus by matching the candidates with the answers
	vector<KResult> results;
	if(wi_.getTimeout() > 0)
		results = this->findMatch(all_refs, question);

	// Unifies with the question
	vector<KnowledgeAnswer> answers;
	for (int n = 0; n < results.size(); ++n) {
		bool to_add = true;

		vector<DrtPred> tmp_drs(question);
		MatchSubstitutions msubs = results.at(n).get<0>();
		DrtMgu upg_tmp = msubs.getDrtMgu();
		tmp_drs / upg_tmp;
		msubs.applySubstitutions(&tmp_drs);
		double w = results.at(n).get<1>();
		string link = results.at(n).get<2>();
		string text = results.at(n).get<3>();
		KnowledgeAnswer single_answer;

		CodePred code = results.at(n).get<4>();
		if (!(code == CodePred())) {
			to_add = false;
			this->addTemporary(tmp_drs);
			code / upg_tmp;
			engine_.setKnowledge(this);
			CodePred result = engine_.run(code);
			if (result == Engine::pt_true)
				to_add = true;
			this->clearAllTemporary();
		}

		if (to_add) {
			single_answer.setPreds(tmp_drs);
			single_answer.setWeigth(w);
			single_answer.setLink(link);
			single_answer.setText(text);
			single_answer.setDrtMgu(upg_tmp);

			answers.push_back(single_answer);
		}
	}

	return answers;
}

static bool operator ==(const RulesCandidate &lhs, const RulesCandidate &rhs)
{
	if (lhs.get<0>() == rhs.get<0>() && lhs.get<1>() == rhs.get<1>() && lhs.get<2>() == rhs.get<2>()
			&& lhs.get<3>() == rhs.get<3>())
		return true;

	return false;
}

vector<RulesCandidate> candidate_intersection_rules(const vector<RulesCandidate> &c1, const vector<RulesCandidate> &c2)
{
	vector<RulesCandidate> to_return;

	if (c1.size() && !c2.size())
		return c1;
	if (c2.size() && !c1.size())
		return c2;

	for (int n = 0; n < c1.size(); ++n) {
		if (shortfind(c2, c1.at(n)))
			to_return.push_back(c1.at(n));
	}

	// phrases with no subject are added anyway
	metric *d = metric_singleton::get_metric_instance();
	for (int n = 0; n < c1.size(); ++n) {
		if (!has_subject(c1.at(n).get<0>()))
			to_return.push_back(c1.at(n)); // add the candidates selected looking at the verbs.
		else {
			DrtVect drtvect = c1.at(n).get<0>();
			for (int m = 0; m < drtvect.size(); ++m) {
				if (drtvect.at(m).is_verb()) {
					vector<DrtPred> subjs = find_subject_of_verb(drtvect, m);
					int j;
					for (j = 0; j < subjs.size(); ++j) {
						string head = extract_header(subjs.at(j));
						string levin = d->get_levin_noun(head);
						if (levin == "") { // subjects with no levin count as no subjects
							to_return.push_back(c1.at(n));
							break;
						}
					}
					if (j != subjs.size()) // a levin-less subj has been found
						break;
				}
			}
		}
	}

	return to_return;
}

static string get_verb_name_to_search(Knowledge *k, const DrtPred &pred)
{
	string to_return;

	string head = extract_header(pred);
	string fref = extract_first_tag(pred);
	if (head == "[*]" || head == "[any]") {
		vector<string> all_names = k->getVerbNames(fref);
		if (all_names.size())
			to_return = all_names.at(0);
		else
			to_return = head;
	} else
		to_return = head;

	return to_return;
}

static string get_noun_name_to_search(Knowledge *k, const DrtPred &pred)
{
	string to_return;

	string head = extract_header(pred);
	string fref = extract_first_tag(pred);
	if (head == "[*]" || head == "[any]") {
		vector<string> all_names = k->getNounNames(fref);
		if (all_names.size())
			to_return = all_names.at(0);
		else
			to_return = head;
	} else
		to_return = head;

	return to_return;
}

vector<RulesCandidate> Knowledge::getGenericVerbRules()
{
	vector<RulesCandidate> candidates_from_verbs;
	vector<boost::shared_ptr<ConditionalAction> > prev_action_pointers_verb;
	vector<string> prev_refs_verb;
	RulesMapStVSt::iterator miter;

	string verb_levin = "[*]";
	miter = rules_verb_refs_.find(verb_levin);
	if (miter != rules_verb_refs_.end() ) {
		vector<string> ref_str = miter->second;
		vector<string>::iterator ref_iter = ref_str.begin();
		vector<string>::iterator ref_end = ref_str.end();
		for (; ref_iter != ref_end; ++ref_iter) {
			if (shortfind(prev_refs_verb, *ref_iter))
				continue;
			prev_refs_verb.push_back(*ref_iter);
			try {
				ConditionalPersona tmp_persona = rules_.getConditionalPersona(*ref_iter);
				vector<boost::shared_ptr<ConditionalAction> > actions = tmp_persona.getActions();
				vector<boost::shared_ptr<ConditionalAction> >::iterator aiter = actions.begin();
				vector<boost::shared_ptr<ConditionalAction> >::iterator aend = actions.end();
				for (; aiter != aend; ++aiter) {
					if (shortfind(prev_action_pointers_verb, *aiter))
						continue;
					prev_action_pointers_verb.push_back(*aiter);
					vector<DrtPred> cons = (*aiter)->getCons();
					clause_vector clause = (*aiter)->getClause();
					string link = (*aiter)->getLink();
					string text = (*aiter)->getText();
					RulesCandidate tmp_candidate(boost::make_tuple(cons, clause, link, text));
					if (!shortfind(candidates_from_verbs, tmp_candidate))
						candidates_from_verbs.push_back(tmp_candidate);
				}
			} catch (std::exception &e) {
				///
			}
		}
	}

	return candidates_from_verbs;
}

void Knowledge::find_all_rules_candidates(DrtVect &question, RulesCandidatesStruct *cstruct)
{
	vector<DrtPred>::const_iterator qiter = question.begin();
	vector<DrtPred>::const_iterator qend = question.end();
	vector<boost::shared_ptr<Action> > prev_action_pointers_verb, prev_action_pointers_noun;
	metric *d = metric_singleton::get_metric_instance();

	bool has_prior_verb = false;

	/// Temporary measure: subjects and objects and noun are not considered, because of the nature of rules search

	// Finds the candidates for the match with the answers
	for (; qiter != qend; ++qiter) { // find the upgs for the answer
		if(measure_time || debug) {
			cout << "FTIMEOUT::: " << wi_.getTimeout() << endl;
		}
		if(wi_.getTimeout() - timeout_prediction < 0)
			break;
		if (qiter->is_verb()
			&& !has_prior_verb
		) {
			has_prior_verb = true; // only the first verb must be considered
			vector<string> strings;
			RulesMapStVSt::iterator miter;
			string qstr = extract_header(*qiter);

			if (!verb_has_subject(question, *qiter))
				cstruct->setLacksSubj(true);

			if (!verb_has_object(question, *qiter))
				cstruct->setLacksObj(true);

			vector<string> alt_verbs;
			//// Generic verbs cannot apply for rules (computationally expensive)
			alt_verbs.push_back("do");
			alt_verbs.push_back("happen");
			alt_verbs.push_back("occur");
			////
			strings.push_back(qstr);
			vector<string> hyponyms;
			for(int ns=0; ns < strings.size(); ++ns) {
				string ns_string= strings.at(ns);
				vector<string> hyponyms_tmp = d->get_hyponyms_of_verb(ns_string, 2);
				hyponyms.insert(hyponyms.end(),hyponyms_tmp.begin(),hyponyms_tmp.end() );
			}
			strings.insert(strings.end(),hyponyms.begin(),hyponyms.end() );
			for (int n = 0; n < strings.size(); ++n) {
				string verb_str = strings.at(n);
				string verb_levin = verb_str;
				if (find(alt_verbs.begin(), alt_verbs.end(), verb_str) != alt_verbs.end()) {
					miter = rules_verb_refs_.find(verb_str); // search for the string name only for "generic verbs"
				} else
					miter = rules_verb_refs_.find(verb_levin);
				if (miter != rules_verb_refs_.end()) {
					vector<string> ref_str = miter->second;
					cstruct->setLacksVerb(false);
					cstruct->addVerbRefs(ref_str);
				}
			}
		}
		if (qiter->is_complement()) {
			bool is_WRB = qiter->tag() == "WRB";// WRB in questions do not need a name to intersect with
			vector<string> strings;
			string qstr = extract_header(*qiter);
			boost::split(strings, qstr, boost::is_any_of("|"));
			for (int n = 0; n < strings.size(); ++n) {
				RulesMapStVSt::iterator miter;
				string name_str = strings.at(n);
				string name_levin = name_str;
				miter = rules_complement_refs_.find(name_levin);
				if (miter != rules_complement_refs_.end()) {
					vector<string> ref_str = miter->second;
					cstruct->addComplRefs(ref_str);
					if(is_WRB) {
						cstruct->addNameRefs(ref_str);
					}
				}
			}
		}
	}

	cstruct->sort();

	// the return values are stored in the two vector<Candidate>
}

class RulesCandidatesKThread {
	Knowledge *k_;
	vector<vector<RulesCandidate> > *results_;
	vector<string> *all_refs_;
	KThreadCounter *counter_;

public:
	RulesCandidatesKThread(vector<vector<RulesCandidate> > *results, Knowledge *k, vector<string> *all_refs, KThreadCounter *counter) :
		results_(results), k_(k), all_refs_(all_refs), counter_(counter)
	{
	}

	void operator()();
};

void RulesCandidatesKThread::operator()()
{
	int num = 0;
	while (num < all_refs_->size() && num < results_->size()) {
		num = counter_->getCurrentNumber();
		if (num == -1)
			break;
		vector<RulesCandidate> *out = &results_->at(num);
		string in = all_refs_->at(num);
		*out = k_->getRulesCandidate(in, counter_->getRemainingTime() );
	}
}


class RulesParserKThread {

	Knowledge *k_;
	vector<KRuleResult> *kresults_;
	DrtVect question_;
	vector<RulesCandidate> *candidates_;
	KThreadCounter *counter_;

	void launchMatchingThread(KRuleResult *results, Match *match, DrtVect &question, RulesCandidate candidate);

public:
	RulesParserKThread(vector<KRuleResult> *kresults, Knowledge *k, DrtVect &question, vector<RulesCandidate> *candidates, KThreadCounter *counter) :
		kresults_(kresults), k_(k), question_(question), candidates_(candidates), counter_(counter)
	{
	}

	void operator()();
};

void RulesParserKThread::launchMatchingThread(KRuleResult *results, Match *match, DrtVect &question, const RulesCandidate candidate)
{
	MatchInfo mi;
	mi.setInvertedPerson(true);

	MatchSubstitutions msubs;
	DrtVect tmp_pred(candidate.get<0>());
	double w;
	w = match->singlePhraseMatch(tmp_pred, question, &msubs, mi);
	if (w != 0) {
		*results = boost::make_tuple(candidate.get<1>(), msubs,  w, candidate.get<2>(), candidate.get<3>() );
	} else {
		*results = boost::make_tuple(candidate.get<1>(), msubs, -1, candidate.get<2>(), candidate.get<3>() );
	}
}


void RulesParserKThread::operator()()
{
	int num = 0;
	while (num < candidates_->size() && num < kresults_->size()) {
		num = counter_->getCurrentNumber();

		if (num == -1)
			break;

		KRuleResult *out = &kresults_->at(num);
		RulesCandidate in = candidates_->at(num);
		Match match(k_);
		this->launchMatchingThread(out, &match, question_, in);
	}
}


vector<KRuleResult> Knowledge::findRulesMatch(vector<string> &all_refs, vector<DrtPred> &question)
{
	vector<KRuleResult> results;

	int max_threads = 1;
	Parameters *par = parameters_singleton::instance();
	if(commercial_version)
		max_threads = par->getNumThreads();
	int num_results= min(max_candidates_refs_,all_refs.size() );
	int num_threads = min(num_results, max_threads);

	double max_time = min(fixed_time_, wi_.getTimeout()) ; // + threads_time/num_threads; // The max time for the operation to finish

	vector<vector<RulesCandidate> > tmp_results0(num_results);
	KThreadCounter counter0(num_results,max_time);
	vector<RulesCandidatesKThread> pt0_vect(num_threads, RulesCandidatesKThread(&tmp_results0, this, &all_refs, &counter0) );
	boost::thread_group g0;
	for (int t = 0; t < num_threads; ++t) {
		g0.create_thread( pt0_vect.at(t) );
	}
	g0.join_all();

	vector<RulesCandidate> all_candidates;
	int n = 0;
	for (int m=0; m < tmp_results0.size() && n < max_rules_candidates; ++m) {
		vector<RulesCandidate> candidates = tmp_results0.at(m);
		n += candidates.size();
		all_candidates.insert(all_candidates.end(), candidates.begin(), candidates.end());
	}


	// Try to match the candidates
	//Match match(this);

	max_time = min(fixed_time_, wi_.getTimeout()) ;
	num_threads = min(all_candidates.size(), max_threads);
	boost::thread_group g;
	vector<KRuleResult> tmp_results(all_candidates.size());
	KThreadCounter counter(all_candidates.size(),max_time);
	vector<RulesParserKThread> pt_vect(num_threads, RulesParserKThread(&tmp_results, this, question, &all_candidates, &counter) );
	for (int t = 0; t < num_threads; ++t) {
		g.create_thread(pt_vect.at(t));
	}
	g.join_all();
	tmp_results = clean_rule_results(tmp_results);
	results.insert(results.end(), tmp_results.begin(), tmp_results.end());

	return results;
}


vector<KnowledgeAnswer> Knowledge::getRules(vector<DrtPred> &question)
{
	vector<RulesCandidate> candidates, candidates_from_names, candidates_from_verbs;
	vector<string> verb_refs, subj_refs, obj_refs, name_refs, compl_refs;

	RulesCandidatesStruct cstruct;
	cstruct.setLacksVerb(true);

	clock_t start;
	if(debug || measure_time)
		start = clock();

	this->find_all_rules_candidates(question, &cstruct);

	if(debug || measure_time)
		start = clock();

	vector<string> all_refs = rules_refs_intersection(cstruct);



	// Find the DrtMgus by matching the candidates with the answers
	if(debug || measure_time)
		start = clock();

	vector<KRuleResult> results = this->findRulesMatch(all_refs, question);


	// Unifies with the question

	//vector< pair<pair<vector<DrtPred>,string>, double > > answers;
	vector<KnowledgeAnswer> answers;
	for (int n = 0; n < results.size(); ++n) {
		vector<DrtPred> tmp_drs(question);
		MatchSubstitutions msubs = results.at(n).get<1>();
		DrtMgu upg_tmp = msubs.getDrtMgu();
		tmp_drs / upg_tmp;
		double w = results.at(n).get<2>();
		string link = results.at(n).get<3>();
		string text = results.at(n).get<4>();
		KnowledgeAnswer single_answer;

		clause_vector tmp_clause(results.at(n).get<0>());

		single_answer.setPreds(tmp_drs);
		single_answer.setClause(tmp_clause);
		single_answer.setWeigth(w);
		single_answer.setLink(link);
		single_answer.setText(text);

		answers.push_back(single_answer);
	}

	return answers;
}

vector<vector<DrtPred> > Knowledge::getSpecifications(const string &ref)
{
	vector<vector<DrtPred> > to_return;

	try {
		if (personae_.getPersona(ref).hasSpecification()) {
			to_return = personae_.getPersona(ref).getSpecifications();
		}
	} catch (std::exception &e) {
		///
	}

	return to_return;
}

vector<vector<DrtPred> > Knowledge::getComplements(const string &ref, const string &vref)
{
	vector<vector<DrtPred> > to_return;

	try {
		Action action = personae_.getPersona(ref).getActionFromVerbRef(vref);
		to_return = action.getComplements();
	} catch (std::exception &e) {
	}

	return to_return;
}

void Knowledge::addHypernym(const Predicate &h)
{
	boost::shared_ptr<Predicate> sp(new Predicate(h));
	hyper_trees_.push_back(sp);
}

void Knowledge::addHypernym(const vector<Predicate> &h)
{
	for (int n = 0; n < h.size(); ++n) {
		this->addHypernym(h.at(n));
	}
}

void Knowledge::addTemporary(const DrtVect &drtvect)
{
	string text = "current question";
	vector<DrtVect> all_drs;
	vector<string> all_text;
	all_drs.push_back(drtvect);
	all_text.push_back(text);
	DrsPersonae p(all_drs, all_text, "");
	p.compute();
	temp_personae_.addPersonae(p);
}

void Knowledge::addTemporary(const vector<DrtVect> &drtvect)
{
	for (int n = 0; n < drtvect.size(); ++n) {
		this->addTemporary(drtvect.at(n));
	}
}

void Knowledge::clearAllTemporary()
{
	temp_personae_.clear();
}

vector<string> Knowledge::getVerbNames(const string &ref_str)
{
	vector<string> to_return, names, tnames, cnames;

	try {
		names = personae_.getVerbNames(ref_str);
	} catch (std::exception &e) {
	}
	try {
		tnames = temp_personae_.getVerbNames(ref_str);
	} catch (std::exception &e) {
	}
	try {
		cnames = rules_.getVerbNames(ref_str);
	} catch (std::exception &e) {
	}
	to_return.insert(to_return.end(), names.begin(), names.end());
	to_return.insert(to_return.end(), tnames.begin(), tnames.end());
	to_return.insert(to_return.end(), cnames.begin(), cnames.end());

	return to_return;
}

vector<string> Knowledge::getNounNames(const string &ref_str)
{
	vector<string> to_return, names, tnames, cnames;

	try {
		names = personae_.getPersona(ref_str).getNames();
	} catch (std::exception &e) {
	}
	try {
		tnames = temp_personae_.getPersona(ref_str).getNames();
	} catch (std::exception &e) {
	}
	try {
		cnames = rules_.getConditionalPersona(ref_str).getNames();
	} catch (std::exception &e) {
	}
	to_return.insert(to_return.end(), names.begin(), names.end());
	to_return.insert(to_return.end(), tnames.begin(), tnames.end());
	to_return.insert(to_return.end(), cnames.begin(), cnames.end());

	return to_return;
}

vector<string> Knowledge::getVerbNamesFromRef(const string &ref) const
{
	vector<string> to_return;
	try { // try to see if the reference is in the data
		to_return = personae_.getVerbNames(ref);
	} catch (std::runtime_error &exc) {
	}
	return to_return;
}

vector<DrtPred> Knowledge::getVerbPredsFromRef(const string &ref) const
{
	vector<DrtPred> to_return;
	try { // try to see if the reference is in the data
		to_return = personae_.getVerbPreds(ref);
	} catch (std::runtime_error &exc) {
	}

	return to_return;
}

vector<DrtPred> Knowledge::getAdverbsFromRef(const string &ref) const
{
	vector<DrtPred> to_return;
	try { // try to see if the reference is in the data
		to_return = personae_.getAdverbs(ref);
	} catch (std::runtime_error &exc) {
	}

	return to_return;
}

vector<pair<DrtPred, Action> > Knowledge::getSubordinatesFromRef(const string &ref) const
{
	vector<pair<DrtPred, Action> > to_return;
	try { // try to see if the reference is in the data
		to_return = personae_.getSubordinates(ref);
	} catch (std::runtime_error &exc) {
	}

	return to_return;
}

Action Knowledge::getActionFromVerbRef(const string &ref, const string &vref) const
{
	Action to_return;
	try { // try to see if the reference is in the data
		to_return = personae_.getPersona(ref).getActionFromVerbRef(vref);
	} catch (std::runtime_error &exc) {
	}

	try { // try to get the action from the verb ref
		to_return = personae_.getPersona(vref).getActionFromVerbRef(vref);
	} catch (std::runtime_error &exc) {
	}

	return to_return;
}

vector<string> Knowledge::getRefFromName(const string &name) const
{
	vector<string> to_return;
	to_return = personae_.getRefFromName(name);
	return to_return;
}

ConditionalAction Knowledge::getConditionalActionFromVerbRef(const string &ref, const string &vref) const
{
	ConditionalAction to_return;
	try { // try to see if the reference is in the data
		to_return = rules_.getConditionalPersona(ref).getActionFromVerbRef(vref);
	} catch (std::runtime_error &exc) {
	}

	try { // try to get the action from the verb ref
		to_return = rules_.getConditionalPersona(vref).getActionFromVerbRef(vref);
	} catch (std::runtime_error &exc) {
	}

	return to_return;
}

vector<DrtPred> Knowledge::getPredsFromRef(const string &ref) const
{
	vector<DrtPred> to_return;
	try { // try to see if the reference is in the data
		to_return = personae_.getPersona(ref).getPreds();
	} catch (std::runtime_error &exc) {
	}
	try { // try to see if the reference is in the data
		vector<DrtPred> pred_rulenames = rules_.getConditionalPersona(ref).getPreds();
		to_return.insert(to_return.end(), pred_rulenames.begin(), pred_rulenames.end());
	} catch (std::runtime_error &exc) {
	}

	return to_return;
}

vector<string> Knowledge::getNamesFromRef(const string &ref) const
{
	vector<string> to_return;
	try { // try to see if the reference is in the data
		to_return = personae_.getPersona(ref).getNames();
	} catch (std::runtime_error &exc) {
	}
	try { // try to see if the reference is in the data
		vector<string> pred_rulenames = rules_.getConditionalPersona(ref).getNames();
		to_return.insert(to_return.end(), pred_rulenames.begin(), pred_rulenames.end());
	} catch (std::runtime_error &exc) {
	}

	return to_return;
}

vector<string> Knowledge::getTempNamesFromRef(const string &ref) const
{
	vector<string> to_return;
	try { // try to see if the reference is in the data
		to_return = temp_personae_.getPersona(ref).getNames();
	} catch (std::runtime_error &exc) {
	}

	return to_return;
}


vector<DrtVect> Knowledge::getSpecificationsFromRef(const string &ref) const
{
	vector<DrtVect> to_return;
	try { // try to see if the reference is in the data
		to_return = personae_.getPersona(ref).getSpecifications();
	} catch (std::runtime_error &exc) {
	}

	return to_return;
}

void operator /(vector<KnowledgeAnswer> &kav, const DrtMgu &mgu)
{
	for (int n = 0; n < kav.size(); ++n) {
		kav.at(n) / mgu;
	}
}

void use_mgu_prior(vector<KnowledgeAnswer> &kav, const DrtMgu &mgu)
// implant the mgu before the one alredy in use
{
	for (int n = 0; n < kav.size(); ++n) {
		kav.at(n).use_mgu_prior(mgu);
	}
}

static bool is_valid_drs(const DrtVect &drtvect)
{
	for (int n = 0; n < drtvect.size(); ++n) {
		if (drtvect.at(n).is_verb()) {
			string fref = extract_first_tag(drtvect.at(n));
			if (fref.find("presupp") != string::npos)
				return false; // A presupposition is not part of the original text
			if (fref.find("[data]") != string::npos)
				return false; // Internal drs cannot be written out
		}
	}
	return true;
}

static vector<DrtPred> filter_first_specification(const vector<DrtPred> &preds)
{
	vector<DrtPred> to_return;
	for (int n = 0; n < preds.size(); ++n) {
		if (n != 0 && preds.at(n).is_complement())
			break;
		to_return.push_back(preds.at(n));
	}

	return to_return;
}

static vector<DrtPred> get_specification_from_pred(const vector<DrtPred> &preds, const DrtPred &pred)
{
	vector<DrtPred> to_return = get_elements_next_of(preds, pred);
	to_return = filter_first_specification(to_return);

	return to_return;
}


static DrtVect prepare_drs_for_writing(DrtVect drtvect)
{
	DrtVect to_return;

	vector<DrtPred> subj_compl, obj_compl;
	for (int n = 0; n < drtvect.size(); ++n) {
		string header = extract_header(drtvect.at(n));
		if (drtvect.at(n).is_verb() && header == "be") {
			vector<DrtVect> be_compl = find_complements_of_verb(drtvect, n);
			string subj_ref = extract_subject(drtvect.at(n));
			string obj_ref = extract_object(drtvect.at(n));
			for (int m = 0; m < be_compl.size(); ++m) {
				DrtVect compl_tmp = be_compl.at(m);
				if(compl_tmp.size() == 0)
					continue;
				implant_first(compl_tmp.at(0), subj_ref);
				subj_compl.push_back(compl_tmp.at(0));
				implant_first(compl_tmp.at(0), obj_ref);
				obj_compl.push_back(compl_tmp.at(0));
			}
		}
	}
	for (int n = 0; n < drtvect.size(); ++n) {
		if (drtvect.at(n).is_complement()
			&& ( shortfind(subj_compl, drtvect.at(n)) || shortfind(obj_compl, drtvect.at(n)) )
		) {
			add_header(drtvect.at(n),":DELETE");
		}
	}

	for (int n = 0; n < drtvect.size(); ++n) {
		string header = extract_header(drtvect.at(n));
		if (header.find(":DELETE") == string::npos
			&& !shortfind(to_return,drtvect.at(n))
		)
			to_return.push_back(drtvect.at(n));
	}

	return to_return;
}

vector<drt> Knowledge::getDrtList()
{
	vector<drt> to_return;

	vector<string> refs = personae_.getReferences();
	vector<string>::iterator riter = refs.begin();
	vector<string>::iterator rend = refs.end();
	map<string,bool> already_written;
	map< string, bool > already_processed;
	for (; riter != rend; ++riter) {
		try {
			if(shortfind(already_written,*riter) )
				continue;
			std::vector<boost::shared_ptr<Action> > actions = personae_.getPersona(*riter).getActions();
			std::vector<boost::shared_ptr<Action> >::iterator aiter = actions.begin();
			std::vector<boost::shared_ptr<Action> >::iterator aend = actions.end();
			for (; aiter != aend; ++aiter) {
				if( (*aiter)->isSubordinate() )
					continue;
				DrtVect drs = (*aiter)->getDrs();
				if (!is_valid_drs(drs))
					continue;
				std::stringstream ss;
				print_vector_stream(ss, drs);
				string drs_str= ss.str();
				if(shortfind(already_processed,drs_str))
					continue;

				already_processed[ drs_str ] = true;
				drs = this->getFullDrsForWriting(drs);
				drs = prepare_drs_for_writing(drs);
				drt tmp_drt(drs);
				tmp_drt.setText((*aiter)->getText());
				tmp_drt.setLink((*aiter)->getLink());
				
				to_return.push_back(tmp_drt);
			}
			already_written[ *riter ] = true;
		} catch (std::exception &e) {
			//std::cerr << e.what() << endl;
		}
	}

	return to_return;
}



vector<Candidate> Knowledge::getAnswerCandidates(drt &question_drt)
{
	vector<DrtPred> question = question_drt.predicates_with_references();
	QuestionList qlist = question_drt.getQuestionList();

	vector<DrtPred> ql = qlist.get();
	vector<Candidate> candidates, candidates_from_verbs, candidates_from_names;
	vector<SPAction> verb_refs, subj_refs, obj_refs, name_refs, compl_refs;
	clock_t start;
	if (debug || measure_time)
		start = clock();

	CandidatesStruct cstruct(wi_);
	cstruct.setLacksVerb(true);

	this->find_all_candidates(question, &cstruct);


	if (debug || measure_time)
		start = clock();

	vector<SPAction> all_refs = refs_intersection(cstruct);


	int max_threads = 1;
	Parameters *par = parameters_singleton::instance();
	if(commercial_version)
		max_threads = par->getNumThreads();
	int num_results= min(max_candidates_refs_,all_refs.size() );
	int num_threads = min(num_results, max_threads);
	double max_time = min(fixed_time_, wi_.getTimeout()) ; //+ threads_time/num_threads; // the max_time for the operation to finish

	if (debug || measure_time)
		start = clock();

	vector<vector<Candidate> > tmp_results0(num_results);
	KThreadCounter counter0(num_results,max_time);
	vector<CandidatesKThread> pt0_vect(num_threads, CandidatesKThread(&tmp_results0, this, &all_refs, &counter0) );
	boost::thread_group g0;
	for (int t = 0; t < num_threads; ++t) {
		g0.create_thread( pt0_vect.at(t) );
	}
	g0.join_all();


	vector<Candidate> all_candidates;
	int n = 0;
	for (int m=0; m < tmp_results0.size() && n < max_candidates_; ++m) {
		vector<Candidate> candidates = tmp_results0.at(m);
		n += candidates.size();
		all_candidates.insert(all_candidates.end(), candidates.begin(), candidates.end());
	}

	if (debug || measure_time)
		start = clock();



	return all_candidates;
}

vector<KnowledgeAnswer> Knowledge::getAnswers(drt &question_drt, vector<Candidate> &all_candidates)
{
	// Matches the candidates

	int max_threads = 1;
	Parameters *par = parameters_singleton::instance();
	if(commercial_version)
		max_threads = par->getNumThreads();

	clock_t start;
	if (debug || measure_time)
		start = clock();

	vector<KResult> results;
	int num_threads = min(all_candidates.size(), max_threads);
	double max_time = min(fixed_time_, wi_.getTimeout()) ; // + threads_time/num_threads; // Max time for the operation to finish
	boost::thread_group g;
	vector<KResult> tmp_results(all_candidates.size());
	KThreadCounter counter(all_candidates.size(),max_time );
	DrtVect question = question_drt.predicates_with_references();
	vector<ParserKThread> pt_vect(num_threads, ParserKThread(&tmp_results, this, question, &all_candidates, &counter) );
	for (int t = 0; t < num_threads; ++t) {
		g.create_thread(pt_vect.at(t));
	}
	g.join_all();
	tmp_results = clean_results(tmp_results);
	results.insert(results.end(), tmp_results.begin(), tmp_results.end());


	if (debug || measure_time)
		start = clock();


	// Unifies with the question
	QuestionList qlist = question_drt.getQuestionList();
	vector<KnowledgeAnswer> answers;
	for (int n = 0; n < results.size(); ++n) {
		bool to_add = true;

		vector<DrtPred> tmp_drs(question);
		MatchSubstitutions msubs = results.at(n).get<0>();
		DrtMgu upg_tmp = msubs.getDrtMgu();
		tmp_drs / upg_tmp;
		msubs.applySubstitutions(&tmp_drs);
		double w = results.at(n).get<1>();
		string link = results.at(n).get<2>();
		string text = results.at(n).get<3>();
		KnowledgeAnswer single_answer;

		// If the drs has a code to execute, add the answer only if the code returns true
		if (debug || measure_time)
			start = clock();

		CodePred code = results.at(n).get<4>();
		if (!(code == CodePred())) {
			to_add = false;
			this->addTemporary(tmp_drs);
			code / upg_tmp;
			engine_.setKnowledge(this);


			if (debug || measure_time)
				start = clock();
			CodePred result = engine_.run(code);


			if (result == Engine::pt_true)
				to_add = true;
			this->clearAllTemporary();
		}


		if (to_add) {
			single_answer.setPreds(tmp_drs);
			single_answer.setWeigth(w);
			single_answer.setLink(link);
			single_answer.setText(text);
			single_answer.setDrtMgu(upg_tmp);

			// Update the Question list
			QuestionList tmp_qlist(qlist);
			tmp_qlist / upg_tmp;
			tmp_qlist.applySubstitutions(msubs);

			single_answer.setQuestionList(tmp_qlist);

			answers.push_back(single_answer);
		}
	}



	return answers;

}

void Knowledge::setWisdomInfo(const WisdomInfo &wi)
{
	wi_= wi;
	fixed_time_ = wi_.getFixedTime();
	max_refs_     = wi_.getMaxRefs();
	max_candidates_refs_ = wi_.getMaxCandidatesRefs();
	max_candidates_      = wi_.getMaxCandidates();
}

