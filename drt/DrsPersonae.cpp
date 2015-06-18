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



#include"DrsPersonae.hpp"

const bool debug = false;

template<class T>
static void print_vector(const std::vector<T> &vs)
{
	if (vs.size()) {
		typename vector<T>::const_iterator tags_iter = vs.begin();
		while (tags_iter != vs.end()) {
			std::cout << (*tags_iter) << " ";
			++tags_iter;
		}
		std::cout << std::endl;
	}
}

static bool compare_actions(const SPAction &lhs, const SPAction &rhs)
{
	return lhs.get() > rhs.get();
}


template<class T>
static T insert_in_map(T &map1, const T &map2)
{
	typedef T map_type;
	typename map_type::const_iterator miter2= map2.begin();
	typename map_type::const_iterator mend2= map2.end();
	for(; miter2 != mend2; ++miter2) {
		typename map_type::iterator miter= map1.find(miter2->first);
		if(miter != map1.end()) {
			vector<SPAction> new_vect= miter->second;
			new_vect.insert(new_vect.end(),miter2->second.begin(),miter2->second.end());
			sort(new_vect.begin(), new_vect.end());
			new_vect.erase(std::unique(new_vect.begin(), new_vect.end()), new_vect.end());
			miter->second = new_vect;
		}
		else
			map1[miter2->first] = miter2->second;
	}

	return map1;
}


template<class T>
static bool shortfind(const vector<T> &vect, const T &element)
{
	if (find(vect.begin(), vect.end(), element) == vect.end())
		return false;
	return true;
}

static bool shortfind(const MapStBool &mapp, const string &element)
{
	MapStBool::const_iterator miter = mapp.find(element);
	if (miter != mapp.end())
		return true;
	return false;
}

static bool shortfind(const MapStBool &mapp, const SPAction &a, const string &element)
{
	MapStBool::const_iterator miter = mapp.find(element);
	if (miter != mapp.end()) {
		vector<SPAction> action_vect = miter->second;
		if (debug) {
			cout << "IN_MAP:: ";
			print_vector(action_vect);
		}
		if (shortfind(action_vect, a))
			return true;
	}
	return false;
}

std::ostream &operator <<(std::ostream &out, const SPAction &a)
{
	out << a->getRef() << endl;
	return out;
}

static void print_vector_stream(std::stringstream &ff, const std::vector<DrtPred> &vs)
{
	vector<DrtPred>::const_iterator tags_iter = vs.begin();
	while (tags_iter < vs.end()) {
		tags_iter->print(ff);
		if (boost::next(tags_iter) != vs.end())
			ff << ",";
		++tags_iter;
	}
}

template<class M, class V>
static void insert_vector_map(M &m, V &v)
{
	for (int n = 0; n < v.size(); ++n) {
		m[v.at(n)] = true;
	}
}

std::size_t RefHasher::operator ()(const string &str) const
{

	vector<string> strs;
	boost::split(strs, str, boost::is_any_of("_"));

	int size = strs.size();
	if (size < 3) {
		boost::hash<string> str_hash;
		return str_hash(str);
	}

	try {
		int num1 = boost::lexical_cast<int>(get_single_distance(strs.at(size - 3)));
		int num2 = boost::lexical_cast<int>(strs.at(size - 2));
		int num3 = boost::lexical_cast<int>(strs.at(size - 1));
		int num0 = 0;

		if (str.find("name") != string::npos) {
			num0 = 1;
		}
		if (str.find("ref") != string::npos) {
			num0 = 2;
		}
		if (str.find("verb") != string::npos) {
			num0 = 3;
		}
		if (str.find("presupp") != string::npos) {
			num0 += 5;
		}
		std::size_t to_return = num0 + num1 * 1e1 + num2 * 1e4 + num3 * 1e8;
		return to_return;
	} catch (std::exception &e) {
		boost::hash<string> str_hash;
		return str_hash(str);
	}
	return 1;
}

bool Action::operator ==(const Action &rhs)
{
	if (link_ == rhs.link_ && text_ == rhs.text_ && ref_ == rhs.link_ && verb_ == rhs.verb_ // && object_ == rhs.object_
	&& complements_ == rhs.complements_)
		return true;

	return false;
}

vector<DrtPred> create_dummy_preds_from(const vector<string> &heads, const string &ref)
{
	vector<string>::const_iterator hiter = heads.begin();
	vector<string>::const_iterator hend = heads.end();
	vector<DrtPred> to_return;

	for (; hiter != hend; ++hiter) {
		to_return.push_back(*hiter + "(" + ref + ")");
	}
	return to_return;
}

static string substitute_string(string str, const string& orig, const string& replace)
{
	int pos = 0;
	while ((pos = str.find(orig, pos)) != std::string::npos) {
		str.replace(pos, orig.size(), replace);
		pos += replace.size();
	}
	return str;
}

static string drs_substitutions(string text)
{
	text = substitute_string(text, "@MOTION_TO", "@0|");
	text = substitute_string(text, "@MOTION_FROM", "@1|");
	text = substitute_string(text, "@PLACE_AT", "@2|");
	text = substitute_string(text, "@TIME_TO", "@3|");
	text = substitute_string(text, "@TIME_FROM", "@4|");
	text = substitute_string(text, "@TIME_AT", "@5|");
	text = substitute_string(text, "@TIME_DURATION", "@6|");
	text = substitute_string(text, "@DATIVE", "@7|");
	text = substitute_string(text, "@CAUSED_BY", "@8|");
	text = substitute_string(text, "@FOR", "@9|");
	text = substitute_string(text, "@TOPIC", "@A|");
	text = substitute_string(text, "@AND", "@B|");
	text = substitute_string(text, "@OR", "@C|");
	text = substitute_string(text, "@WITH", "@D|");
	text = substitute_string(text, "@EXCLUDING", "@E|");
	text = substitute_string(text, "@AFTER", "@F|");
	text = substitute_string(text, "@BEFORE", "@G|");
	text = substitute_string(text, "@MOTION_THROUGH", "@H|");
	text = substitute_string(text, "@MOTION_AGAINST", "@I|");
	text = substitute_string(text, "@TIME_THROUGH", "@J|");
	text = substitute_string(text, "@QUANTITY", "@K|");
	text = substitute_string(text, "@QUANTIFIER", "@L|");
	text = substitute_string(text, "@ALLOCUTION", "@M|");
	text = substitute_string(text, "@COMPARED", "@N|");
	text = substitute_string(text, "@COMPARED_TO", "@O|");
	text = substitute_string(text, "@OWNED_BY", "@P|");
	text = substitute_string(text, "@CLOCK_AT", "@Q|");
	text = substitute_string(text, "@OWN", "@R|");
	text = substitute_string(text, "@AGE", "@S|");
	text = substitute_string(text, "@MORE", "@T|");
	text = substitute_string(text, "@LESS", "@U|");
	text = substitute_string(text, "@TIMES", "@V|");
	text = substitute_string(text, "@MORE_THAN", "@X|");
	text = substitute_string(text, "@LESS_THAN", "@Y|");
	text = substitute_string(text, "@SIZE", "@Z|");
	text = substitute_string(text, "@ACCORDING_TO", "@01|");
	text = substitute_string(text, "@SUBORD", "@02|");
	text = substitute_string(text, "@TIME", "@03|");
	text = substitute_string(text, "@MODAL", "@04|");
	text = substitute_string(text, "@QUANTITY", "@05|");
	text = substitute_string(text, "@GENITIVE", "@06|");

	////
	//text = "";
	////

	return text;
}

static string drs_substitutions_backwards(string text)
{
	text = substitute_string(text, "@0|", "@MOTION_TO");
	text = substitute_string(text, "@1|", "@MOTION_FROM");
	text = substitute_string(text, "@2|", "@PLACE_AT");
	text = substitute_string(text, "@3|", "@TIME_TO");
	text = substitute_string(text, "@4|", "@TIME_FROM");
	text = substitute_string(text, "@5|", "@TIME_AT");
	text = substitute_string(text, "@6|", "@TIME_DURATION");
	text = substitute_string(text, "@7|", "@DATIVE");
	text = substitute_string(text, "@8|", "@CAUSED_BY");
	text = substitute_string(text, "@9|", "@FOR");
	text = substitute_string(text, "@A|", "@TOPIC");
	text = substitute_string(text, "@B|", "@AND");
	text = substitute_string(text, "@C|", "@OR");
	text = substitute_string(text, "@D|", "@WITH");
	text = substitute_string(text, "@E|", "@EXCLUDING");
	text = substitute_string(text, "@F|", "@AFTER");
	text = substitute_string(text, "@G|", "@BEFORE");
	text = substitute_string(text, "@H|", "@MOTION_THROUGH");
	text = substitute_string(text, "@I|", "@MOTION_AGAINST");
	text = substitute_string(text, "@J|", "@TIME_THROUGH");
	text = substitute_string(text, "@K|", "@QUANTITY");
	text = substitute_string(text, "@L|", "@QUANTIFIER");
	text = substitute_string(text, "@M|", "@ALLOCUTION");
	text = substitute_string(text, "@N|", "@COMPARED");
	text = substitute_string(text, "@O|", "@COMPARED_TO");
	text = substitute_string(text, "@P|", "@OWNED_BY");
	text = substitute_string(text, "@Q|", "@CLOCK_AT");
	text = substitute_string(text, "@R|", "@OWN");
	text = substitute_string(text, "@S|", "@AGE");
	text = substitute_string(text, "@T|", "@MORE");
	text = substitute_string(text, "@U|", "@LESS");
	text = substitute_string(text, "@V|", "@TIMES");
	text = substitute_string(text, "@X|", "@MORE_THAN");
	text = substitute_string(text, "@Y|", "@LESS_THAN");
	text = substitute_string(text, "@Z|", "@SIZE");
	text = substitute_string(text, "@01|", "@ACCORDING_TO");
	text = substitute_string(text, "@02|", "@SUBORD");
	text = substitute_string(text, "@03|", "@TIME");
	text = substitute_string(text, "@04|", "@MODAL");
	text = substitute_string(text, "@05|", "@QUANTITY");
	text = substitute_string(text, "@06|", "@GENITIVE");
	return text;
}

Action::Action(const string &str, const string &link, const string &text, const vector<DrtPred> &drs) :
		ref_(str), text_(text), link_(link), is_subordinate_(false)
{
	std::stringstream ss;
	print_vector_stream(ss, drs);
	drs_form_ = drs_substitutions(ss.str());
}

vector<DrtPred> Action::getDrs() const
{
	vector<DrtPred> to_return(create_drtvect(drs_form_));
	CMap::const_iterator miter = complements_.begin();
	CMap::const_iterator mend = complements_.end();
	for (; miter != mend; ++miter) {
		vector<string> complement_lines = miter->second;
		for (int n = 0; n < complement_lines.size(); ++n) {
			DrtVect compl_vect(create_drtvect(drs_substitutions_backwards(complement_lines.at(n))));
			to_return.insert(to_return.end(), compl_vect.begin(), compl_vect.end());
		}
	}
	return to_return;
}

vector<DrtVect> Action::getSpecificComplement(const string &name) const
{
	vector<DrtVect> to_return;
	CMap::const_iterator miter = complements_.find(name);
	if (miter != complements_.end()) {
		vector<string> complement_lines = miter->second;
		for (int n = 0; n < complement_lines.size(); ++n) {
			DrtVect compl_vect(create_drtvect(drs_substitutions_backwards(complement_lines.at(n))));
			to_return.push_back(compl_vect);
		}
	}
	return to_return;
}

vector<vector<DrtPred> > Action::getComplements() const
{
	vector<vector<DrtPred> > to_return;

	vector<string> all_complements = get_all_complement_strings();
	all_complements.push_back("@TIME");

	vector<string>::iterator aiter = all_complements.begin();
	vector<string>::iterator aend = all_complements.end();

	int n = 0;
	for (; aiter != aend; ++aiter, ++n) {
		vector<DrtVect> complement_lines = getSpecificComplement(*aiter);
		for (int n = 0; n < complement_lines.size(); ++n) {
			to_return.insert(to_return.end(), complement_lines.begin(), complement_lines.end());
		}
	}
	return to_return;
}

void Action::addComplement(const DrtVect &dvect)
{
	string ctype = "";
	for (int n = 0; n < dvect.size(); ++n) {
		if (dvect.at(n).is_complement()) {
			ctype = extract_header(dvect.at(n));
			break;
		}
	}
	if (ctype == "")
		return;

	std::stringstream ss;
	print_vector_stream(ss, dvect);
	complements_[ctype].push_back(drs_substitutions(ss.str()));
}

static vector<string> find_headers_with_reference(const DrtVect &drtvect, const string &ref)
{
	vector<string> to_return;

	for (int n = 0; n < drtvect.size(); ++n) {
		string fref = extract_first_tag(drtvect.at(n));
		if (fref == ref)
			to_return.push_back(extract_header(drtvect.at(n)));
	}
	return to_return;
}

Persona::Persona()
{
	num_actions_ = 0;
}

Persona::~Persona()
{
}

vector<string> Persona::getNames() const
{
	vector<string> to_return;

	for (int n = 0; n < pred_names_.size(); ++n) {
		string header = extract_header(pred_names_.at(n));
		to_return.push_back(header);
	}
	return to_return;
}

static vector<DrtPred> find_preds_with_reference(const DrtVect &drtvect, const string &ref)
{
	vector<DrtPred> to_return;

	for (int n = 0; n < drtvect.size(); ++n) {
		string fref = extract_first_tag(drtvect.at(n));
		if (fref == ref)
			to_return.push_back(drtvect.at(n));
	}
	return to_return;
}

vector<DrtPred> Persona::getPreds() const
{
	return pred_names_;
}

bool Persona::hasSpecification()
{
	if (specifications_.size())
		return true;
	return false;
}

vector<vector<DrtPred> > Persona::getSpecifications()
{
	return specifications_;
}

void Persona::addPersona(const Persona &rhs)
{
	if (reference_ != "" && reference_ != rhs.reference_)
		throw(std::runtime_error("Trying to join two personas with different references!"));

	if (reference_ == "")
		reference_ = rhs.reference_;

	if (ref_to_actions_.size()) {
		ref_to_actions_.insert(rhs.ref_to_actions_.begin(), rhs.ref_to_actions_.end());
	} else
		ref_to_actions_ = rhs.ref_to_actions_;
	if (verb_to_actions_.size()) {
		verb_to_actions_.insert(rhs.verb_to_actions_.begin(), rhs.verb_to_actions_.end());
	} else
		verb_to_actions_ = rhs.verb_to_actions_;

	names_.insert(names_.end(), rhs.names_.begin(), rhs.names_.end());
	pred_names_.insert(pred_names_.end(), rhs.pred_names_.begin(), rhs.pred_names_.end());
	actions_.insert(actions_.end(), rhs.actions_.begin(), rhs.actions_.end());
	specifications_.insert(specifications_.end(), rhs.specifications_.begin(), rhs.specifications_.end());

	/// Add the specifications!!!!
}

Action Persona::getActionFromVerbName(const string &head)
// Searches for an action that finds the verb with name "head"
{
	MapStActionPtr::iterator miter = verb_to_actions_.find(head);
	if (miter != verb_to_actions_.end())
		return Action(*miter->second);
	// If the verb name is not found
	throw(std::runtime_error("No action found with verb " + head));
}

Action Persona::getActionFromVerbRef(const string &ref)
// Searches for an action that finds the verb with reference "ref"
{
	MapStActionPtr::iterator miter = ref_to_actions_.find(ref);
	if (miter != ref_to_actions_.end())
		return Action(*miter->second);
	// If the verb name is not found
	throw(std::runtime_error("No action found with reference " + ref));
}

boost::shared_ptr<Action> Persona::addAction(const Action &a)
{
	//if(actions_.size() < max_action_size) {
	boost::shared_ptr<Action> pa(new Action(a));
	if (num_actions_ < max_action_size) {
		//boost::shared_ptr<Action> pa(new Action);

		actions_.push_back(pa);
		//actions_.push_front(pa);
		++num_actions_;
		//actions_[num_actions_] = pa;

		string verb_name = a.getVerb();
		string verb_ref = a.getRef();

		if (debug) {
			cout << "LINK2::: " << a.getLink() << endl;
			cout << "ADDING_ACTION::: " << verb_name << " " << verb_ref << endl;
		}
		verb_to_actions_[verb_name] = pa;
		ref_to_actions_[verb_ref] = pa;
	}
	return pa;
}

static bool has_specification(const DrtPred &pred)
{
	string head = extract_header(pred);

	if (head.size() && head.at(0) == '@') {
		string ftag = extract_first_tag(pred);
		if (ftag.find("name") != string::npos || ftag.find("ref") != string::npos)
			return true;
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


static vector<DrtPred> filter_first_specification(const vector<DrtPred> &preds)
{
	vector<DrtPred> to_return;
	bool has_verb = false;
	for (int n = 0; n < preds.size(); ++n) {
		if(preds.at(n).is_verb() )
			has_verb = true;
		if (n != 0 && preds.at(n).is_complement() && !has_verb)
			break;
		to_return.push_back(preds.at(n));
	}

	return to_return;
}

static vector<DrtPred> get_specification_from_pred(const vector<DrtPred> &preds, const DrtPred &pred)
{
	vector<DrtPred> to_return = get_elements_next_of(preds, pred);
	to_return = filter_first_specification(to_return);

	if (debug) {
		cout << "SPECIFICATIONS::: ";
		print_vector(to_return);
	}

	return to_return;
}

static bool is_subordinate_of_verb(const DrtVect &pre_drt, const string &subord_ref, DrtPred *candidate_subord_pred)
{
	vector<DrtPred>::const_iterator piter = pre_drt.begin();
	vector<DrtPred>::const_iterator pend = pre_drt.end();

	if(debug) {
		cout << "IS_SUBORD::: " << subord_ref << endl;
		print_vector(pre_drt);
	}

	for (; piter != pend; ++piter) {
		//if(piter->pred().begin()->str == "@SUBORD") {
		string fref = extract_first_tag(*piter);
		string sref = extract_second_tag(*piter);
		if (piter->is_complement()
				//&& ref_is_verb(fref)
				&& ref_is_verb(sref)) {
			string second_tag = extract_second_tag(*piter);
			if (second_tag == subord_ref) {

				if(debug) {
					cout << "SUBORD_CANDIDATES::: " << *piter << endl;
				}

				*candidate_subord_pred = *piter;
				return true;
			}
		}
	}

	return false;
}

static vector<DrtPred> substitute_ref_safe(vector<DrtPred> &pre_drt, const string &from_str, const string &to_str)
// returns the same pre_drt if there is a bad substitution, i.e., if there is an element which would point to itself
{
	vector<DrtPred> predicates(pre_drt);

	for (int n = 0; n < predicates.size(); ++n) {
		vector<string> children = predicates.at(n).extract_children();
		for (int m = 0; m < children.size(); ++m) {
			if (children.at(m) == to_str) {
				predicates = pre_drt;
				break;
			}
			if (children.at(m) == from_str) {
				children.at(m) = to_str;
			}
		}
		predicates.at(n).implant_children(children);
	}
	return predicates;
}

static vector<DrtVect> filter_valid_complements(vector<DrtVect> cvect)
{
	vector<string> invalid;
	invalid.push_back("@TIME");
	invalid.push_back("@MODAL");
	invalid.push_back("@CONJUNCTION");

	for (int n = 0; n < cvect.size(); ++n) {
		DrtVect complement = cvect.at(n);
		for (int m = 0; m < complement.size(); ++m) {
			string head = extract_header(complement.at(m));
			if (find(invalid.begin(), invalid.end(), head) != invalid.end()) {
				cvect.erase(cvect.begin() + n);
				n = 0;
				break;
			}
		}
	}

	return cvect;
}

DrsPersonae::DrsPersonae()
{
}

DrsPersonae::DrsPersonae(const vector<DrtVect> p, const vector<string> &all_texts, const string &link) :
		preds_(p), texts_(all_texts), link_(link)
{
	for (int n = 0; n < p.size(); ++n) {
		codes_.push_back(CodePred());
	}
}

DrsPersonae::DrsPersonae(const vector<drt> &drt_list, const string &link) :
		link_(link)
{
	for (int n = 0; n < drt_list.size(); ++n) {
		preds_.push_back(drt_list.at(n).predicates_with_references());
		texts_.push_back(drt_list.at(n).getText());
		codes_.push_back(drt_list.at(n).getCode());
	}
}

static int find_prep_with_target(vector<DrtPred> &pre_drt, string from_str)
{
	for (int n = 0; n < pre_drt.size(); ++n) {
		PredTree tmp_verb = extract_header(pre_drt.at(n));
		string sref = extract_second_tag(pre_drt.at(n));
		if (pre_drt.at(n).is_complement() && !pre_drt.at(n).is_parenthesis() // the percolation MUST be inside a PRN
				&& sref == from_str) {
			return n;
		}
	}
	return -1;
}

static boost::tuple<string, string, string> get_reference_percolated_to_verb(vector<DrtPred> &pre_drt, int n)
/// Implement a recursive algorithm !!!
{
	if (debug) {
		cout << "PERCOLATING::: ";
		print_vector(pre_drt);
	}

	boost::tuple<string, string, string> to_return(
			boost::make_tuple(extract_first_tag(pre_drt.at(n)), extract_subject(pre_drt.at(n)),
					extract_object(pre_drt.at(n))));
	string from_str = extract_first_tag(pre_drt.at(n));
	int m = n;
	// see if this this the end of a complement
	int safe = 0, safe_max = 3;
	while (m != -1 && safe++ < safe_max) {
		m = find_prep_with_target(pre_drt, from_str);
		if (m != -1)
			from_str = extract_first_tag(pre_drt.at(m)); // from_str becomes the first element of the preposition
		else
			break;
	}

	m = find_verb_with_subject(pre_drt, from_str);
	if (m == -1) {
		m = find_verb_with_object(pre_drt, from_str);
	}

	if (debug) {
		cout << "PERCOLATING2::: ";
		cout << m << endl;
	}

	if (m != -1) { // There might not be such a verb
		string verb_str = extract_first_tag(pre_drt.at(m));
		if (ref_is_verb(verb_str)) {
			int m2 = find_verb_with_string(pre_drt, verb_str);
			string subj_str = "none";
			string obj_str = "none";
			if (m2 != -1) {
				subj_str = extract_subject(pre_drt.at(m2));
				obj_str = extract_object(pre_drt.at(m2));
			}
			to_return = boost::make_tuple(verb_str, subj_str, obj_str);
		} else {
			m = find_verb_with_object(pre_drt, verb_str);
			if (m == -1) // if there is no verb, try to find a preposition
				m = find_prep_with_target(pre_drt, verb_str);
			if (m != -1) { // There might not be such a verb
				string verb_str = extract_first_tag(pre_drt.at(m));
				if (ref_is_verb(verb_str)) {
					int m2 = find_verb_with_string(pre_drt, verb_str);
					string subj_str = "none";
					string obj_str = "none";
					if (m2 != -1) {
						subj_str = extract_subject(pre_drt.at(m2));
						obj_str = extract_object(pre_drt.at(m2));
					}
					to_return = boost::make_tuple(verb_str, subj_str, obj_str);
				} else {
					m = find_verb_with_object(pre_drt, verb_str);
					if (m == -1) // if there is no verb, try to find a preposition
						m = find_prep_with_target(pre_drt, verb_str);
					if (m != -1) { // There might not be such a verb
						string verb_str = extract_first_tag(pre_drt.at(m));
						if (ref_is_verb(verb_str)) {
							int m2 = find_verb_with_string(pre_drt, verb_str);
							string subj_str = "none";
							string obj_str = "none";
							if (m2 != -1) {
								subj_str = extract_subject(pre_drt.at(m2));
								obj_str = extract_object(pre_drt.at(m2));
							}
							to_return = boost::make_tuple(verb_str, subj_str, obj_str);
						} else {
							m = find_verb_with_object(pre_drt, verb_str);
							if (m == -1) // if there is no verb, try to find a preposition
								m = find_prep_with_target(pre_drt, verb_str);
							if (m != -1) { // There might not be such a verb
								string verb_str = extract_first_tag(pre_drt.at(m));
								int m2 = find_verb_with_string(pre_drt, verb_str);
								string subj_str = "none";
								string obj_str = "none";
								if (m2 != -1) {
									subj_str = extract_subject(pre_drt.at(m2));
									obj_str = extract_object(pre_drt.at(m2));
								}
								to_return = boost::make_tuple(verb_str, subj_str, obj_str);
							}
						}
					}
				}
			}
		}
	}
	return to_return;
}

bool is_valid_subject_ref(const string &str)
{
	if (str.find("none") != string::npos || str.find("subj") != string::npos || str.find("obj") != string::npos)
		return false;
	return true;
}

static vector<string> extract_all_subj_refs(const DrtVect &drtvect, const DrtPred &pred)
{
	vector<string> to_return;
	string ref = extract_subject(pred);
	to_return.push_back(ref);
	for (int n = 0; n < drtvect.size(); ++n) {
		string fref = extract_first_tag(drtvect.at(n));
		string sref = extract_second_tag(drtvect.at(n));
		string header = extract_header(drtvect.at(n));
		if ((header == "@AND" || header == "@OR") && shortfind(to_return, fref))
			to_return.push_back(sref);
	}
	return to_return;
}
static vector<string> extract_all_obj_refs(const DrtVect &drtvect, const DrtPred &pred)
{
	vector<string> to_return;
	string ref = extract_object(pred);
	to_return.push_back(ref);
	for (int n = 0; n < drtvect.size(); ++n) {
		string fref = extract_first_tag(drtvect.at(n));
		string sref = extract_second_tag(drtvect.at(n));
		string header = extract_header(drtvect.at(n));
		if ((header == "@AND" || header == "@OR") && shortfind(to_return, fref))
			to_return.push_back(sref);
	}
	return to_return;
}

SPAction DrsPersonae::addAction(const DrtVect &drtpreds, const DrtPred &pred, int m, const string &text, const CodePred &code,
		string ref, string oref, SPAction pa_main)
{

	string verb_ref = extract_first_tag(pred);

	// presuppositions with a verb "be" do not join subj and obj
	bool is_presupp = false;
	if (verb_ref.find("presupp") != string::npos  // this is to avoid "new car" -> "car is new" -> "new car" again
		&& verb_ref.find("material") == string::npos
			)
		is_presupp = true;
	if (verb_ref.find("[data]") != string::npos)
		is_presupp = true;

	MapStPers::iterator pers_iter = personae_.find(ref);

	DrtPred candidate_subord_prep;
	bool is_subord_trigger;
	is_subord_trigger = is_subordinate_of_verb(drtpreds, verb_ref, &candidate_subord_prep);

	// add the verb name to the verb references
	//verb_names_[verb_ref].push_back(extract_header(pred));
	verb_preds_[verb_ref].push_back(pred);

	if (pers_iter == personae_.end()) {
		/// If the persona does not exist then the obj reference is used
		pers_iter = personae_.find(oref);
		if (pers_iter != personae_.end()) {
			///-R references_.push_back(oref);
			ref = oref; //// FOR RULES!!!
		} else {
			///-R references_.push_back(verb_ref);
			personae_[verb_ref].setReference(verb_ref);
			pers_iter = personae_.find(verb_ref);
			ref = verb_ref; //// FOR RULES!!!
		}
	}

	// The verb is added here if the persona already exists

	vector<DrtPred> tmp_preds = find_attached_to_verb(drtpreds, m);

	/// the type should be a reference to the type of phase
	/// but now it is just the reference to the verb "verb_ref"
	Action tmp_action(verb_ref, link_, text, tmp_preds);
	tmp_action.setSubordinate(is_subord_trigger);
	///

	if (debug) {
		cout << "TMP_ACTION::: " << verb_ref << " " << ref << " " << link_ << " " << is_subord_trigger << endl;
		print_vector(tmp_preds);
	}

	string verb_name = extract_header(pred);
	tmp_action.addVerb(verb_name);
	tmp_action.setCode(code);

	vector<string> object = find_string_object_of_verb(drtpreds, m);
	vector<DrtPred> pred_object = find_object_of_verb(drtpreds, m);
	//tmp_action.addObject(object);
	vector<DrtVect> complements_lines = find_complements_of_verb(drtpreds, m);

	// Add the object of "the verb "to be" as another
	// instantiation of the subject (reference = "ref")
	if (verb_name == "be" && !is_presupp) {
		for (int k = 0; k < pred_object.size(); ++k) {
			string obj_name = extract_header(pred_object.at(k));
			if (obj_name.find("[*]") != string::npos)
				continue;
			MapStPers::iterator pers_iter = personae_.find(ref);
			if (pers_iter != personae_.end()) {
				//pers_iter->second.addName(obj_name);
				pers_iter->second.addPred(pred_object.at(k));
				if (debug) {
					cout << "BE_SUBJ::: " << ref << " " << object.at(k) << " " << pred_object.at(k) << endl;
				}
			}
		}
	}

	vector<string> subject = find_string_subject_of_verb(drtpreds, m);
	vector<DrtPred> pred_subject = find_subject_of_verb(drtpreds, m);
	// Add the subject of "the verb "to be" as another
	// instantiation of the object (reference = "ref")
	if (verb_name == "be" && !is_presupp) {
		for (int k = 0; k < pred_subject.size(); ++k) {
			string subj_name = extract_header(pred_subject.at(k));
			if (subj_name.find("[*]") != string::npos)
				continue;
			MapStPers::iterator pers_iter = personae_.find(oref);
			if (pers_iter != personae_.end()) {
				//pers_iter->second.addName(subj_name);
				pers_iter->second.addPred(pred_subject.at(k));
				if (debug) {
					cout << "BE_OBJ::: " << oref << " " << subject.at(k) << endl;
				}
			}
		}
	}

	// Finds and add to the main action the subordinates, like in: he is happy (to live)

	if (is_subord_trigger) {
		boost::tuple<DrtPred, string, string> subord_tuple;
		subord_tuple.get<0>() = candidate_subord_prep; // The type of subordinate
		subord_tuple.get<1>() = ref; // the persona reference
		subord_tuple.get<2>() = verb_ref; // The subordinate reference
		string parent_verb_ref = extract_first_tag(candidate_subord_prep);
		subord_verbs_[parent_verb_ref].push_back(subord_tuple);
	}

	for (int n = 0; n < complements_lines.size(); ++n) {
		tmp_action.addComplement(complements_lines.at(n));
	}
	if (verb_name == "be" && !is_presupp) {
		// Adds the complements to the verb as a specifications to the subject
		vector<DrtVect> complements_drtvect = find_complements_of_verb(drtpreds, m);
		complements_drtvect = filter_valid_complements(complements_drtvect);

		MapStPers::iterator pers_iter = personae_.find(ref);
		if (pers_iter != personae_.end()) {
			for (int n = 0; n < complements_drtvect.size(); ++n) {
				DrtVect drtvect = complements_drtvect.at(n);
				drtvect = substitute_ref_safe(drtvect, verb_ref, ref);
				personae_[ref].addSpecification(drtvect);
			}
			for (int n = 0; n < complements_drtvect.size(); ++n) {
				DrtVect drtvect = complements_drtvect.at(n);
				drtvect = substitute_ref_safe(drtvect, verb_ref, oref);
				personae_[oref].addSpecification(drtvect);
			}
		}
	}
	SPAction pa;
	pa = pers_iter->second.addAction(tmp_action);

	if (!shortfind(subj_refs_, pa, ref)) {
		if (debug) {
			cout << "Personae_SUBJ_REF:::" << ref << " " << pa->getRef() << endl;
		}
		subj_refs_[ref].push_back(pa);
	}
	if (!shortfind(obj_refs_, pa, oref)) {
		obj_refs_[oref].push_back(pa);
		if (debug) {
			cout << "Personae_OBJ_REF:::" << oref << " " << pa->getRef() << endl;
		}
	}
	if (extract_header(pred) == "be") {
		if (debug) {
			cout << "PERS_BE:::" << pa->getRef() << endl;
			DrtVect tmp_drtvect = drtpreds;
			print_vector(tmp_drtvect);
		}
		// invert subject and object for the verb "to be" (consistently with Match )
		if (!shortfind(obj_refs_, pa, ref)) {
			obj_refs_[ref].push_back(pa);
			if (debug) {
				vector<SPAction> spvect = obj_refs_[ref];
				print_vector(spvect);
				cout << "Personae_INVERTED_OBJ_REF:::" << ref << " " << pa->getRef() << endl;
			}
		}
		if (!shortfind(subj_refs_, pa, oref)) {
			subj_refs_[oref].push_back(pa);
			if (debug) {
				cout << "Personae_INVERTED_SUBJ_REF:::" << oref << " " << pa->getRef() << endl;
			}
		}
	}

	for(int n=0; n < drtpreds.size(); ++n) {
		if(!drtpreds.at(n).is_name() )
			continue;
		string fref = extract_first_tag(drtpreds.at(n) );
		if(fref == ref || fref == oref)
			continue;
		name_refs_[fref].push_back(pa);
		if (debug) {
			cout << "Personae_NAME_REF:::" << fref << " " << pa->getRef() << endl;
		}
	}
	return pa;
}

void DrsPersonae::compute()
{
	for (int num = 0; num < preds_.size(); ++num) {
		DrtVect drtpreds(preds_.at(num));
		string text(texts_.at(num));
		CodePred code = codes_.at(num);

		vector<DrtPred>::iterator diter = drtpreds.begin();
		vector<DrtPred>::iterator dend = drtpreds.end();

		metric *d = metric_singleton::get_metric_instance();

		vector<DrtPred> previous_elements;
		int m = 0;
		diter = drtpreds.begin();

		// extract the references from names
		for (; diter != dend; ++diter, ++m) {
			if (diter->is_name()) {
				if (debug) {
					cout << "NAME_DRS::: " << *diter << endl;
				}
				string ref = extract_first_tag(*diter);
				if (ref.size() == 0)
					continue; // just a safety check
				//if (ref.at(0) == '_')
				//     continue; // quantifiers cannot be references

				MapStPers::iterator pers_iter = personae_.find(ref);

				names_to_refs_[diter->name()].push_back(ref);

				if (pers_iter == personae_.end()) {
					if (debug) {
						cout << "ADDING_PRED11" << *diter << endl;
					}
					///-R references_.push_back(ref);
					personae_[ref].setReference(ref);
					personae_[ref].addPred(*diter);
					if (debug) {
						cout << "ADDING_PRED21" << *diter << endl;
					}
				} else {
					if (debug) {
						cout << "ADDING_PRED12" << *diter << endl;
					}
					pers_iter->second.addPred(*diter);
					if (debug) {
						cout << "ADDING_PRED22" << *diter << endl;
					}
				}
			}
			if (has_specification(*diter)) {
				string ref = extract_first_tag(*diter);
				int pos_name = find_element_with_string(drtpreds, ref);
				string name = drtpreds.at(pos_name).name();
				MapStPers::iterator pers_iter = personae_.find(ref);
				if (pers_iter != personae_.end()) {
					vector<DrtPred> drtvect = get_specification_from_pred(drtpreds, *diter);
					personae_[ref].addSpecification(drtvect);
					if (debug) {
						cout << "ADDING_SPEC:::" << endl;
						print_vector(drtvect);
					}
				}
			}
			// save the adverbs as well
			if (diter->is_adverb()) {
				string ref = extract_first_tag(*diter);
				if (ref.size() == 0)
					continue; // just a safety check
				// if (ref.at(0) == '_')
				//      continue; // quantifiers cannot be references
				adverbs_[ref].push_back(*diter);
			}
		}

		// process the verb after the personae are found
		diter = drtpreds.begin();
		m = 0;

		//  1) Find the main sentence (for cycle to search for the one that is not a sub) and add it
		//  2) Save the pa of the main sentence
		//  3) Add the subordinates (for cycle excluding the main sentence)

		// insert the main sentence
		SPAction pa_main;
		for (; diter != dend; ++diter, ++m) {
			if (diter->is_verb()
			) {
				string verb_ref = extract_first_tag(*diter);
				DrtPred candidate_subord_prep;
				bool is_subord_trigger;
				is_subord_trigger = is_subordinate_of_verb(drtpreds, verb_ref, &candidate_subord_prep);
				if(is_subord_trigger) // This is a subordinate!
					continue;

				if(debug) {
					puts("MAIN_SENTENCE2::: ");
				}

				vector<string> all_subj_refs = extract_all_subj_refs(drtpreds, *diter);
				vector<string> all_obj_refs = extract_all_obj_refs(drtpreds, *diter);

				//boost::shared_ptr<Action> action_pointer;

				for (int subj_num = 0; subj_num < all_subj_refs.size(); ++subj_num) {
					for (int obj_num = 0; obj_num < all_obj_refs.size(); ++obj_num) {
						string ref = all_subj_refs.at(subj_num);
						string oref = all_obj_refs.at(obj_num);
						//if(subj_num == 0 && obj_num == 0)
						pa_main = this->addAction(drtpreds, *diter, m, text, code, ref, oref, pa_main);

					}
				}
			}
		}

		diter = drtpreds.begin();
		m=0;
		// Insert the subordinate
		for (; diter != dend; ++diter, ++m) {
			if (diter->is_verb()
			//&& find(previous_elements.begin(), previous_elements.end(), *diter) == previous_elements.end()
			) {
				string verb_ref = extract_first_tag(*diter);
				DrtPred candidate_subord_prep;
				bool is_subord_trigger;
				is_subord_trigger = is_subordinate_of_verb(drtpreds, verb_ref, &candidate_subord_prep);
				if(!is_subord_trigger) // This is not a subordinate!
					continue;

				if(debug) {
					puts("SUBORD_SENTENCE2::: ");
				}

				vector<string> all_subj_refs = extract_all_subj_refs(drtpreds, *diter);
				vector<string> all_obj_refs = extract_all_obj_refs(drtpreds, *diter);

				//boost::shared_ptr<Action> action_pointer;

				for (int subj_num = 0; subj_num < all_subj_refs.size(); ++subj_num) {
					for (int obj_num = 0; obj_num < all_obj_refs.size(); ++obj_num) {
						string ref = all_subj_refs.at(subj_num);
						string oref = all_obj_refs.at(obj_num);
						//if(subj_num == 0 && obj_num == 0)
						this->addAction(drtpreds, *diter, m, text, code, ref, oref, pa_main);

					}
				}
			}
		}
	}

	preds_.clear(); /// no longer needed
	codes_.clear();
	texts_.clear();
}

vector<string> DrsPersonae::mapRefToActionRef(const string &ref) const
{
	vector<string> to_return;
	PMapStVSt::const_iterator miter = persona_pointer_.find(ref);
	if (miter != persona_pointer_.end()) {
		to_return = miter->second;
	}
	return to_return;
}

Persona DrsPersonae::getPersona(const string &ref) const
{
	MapStPers::const_iterator pers_iter = personae_.find(ref);
	if (pers_iter != personae_.end())
		return pers_iter->second;
	else
		throw std::runtime_error("No such reference: " + ref + ".");
}

void DrsPersonae::print(std::ostream &out)
{
	MapStPers::iterator pers_iter = personae_.begin();
	for (; pers_iter != personae_.end(); ++pers_iter) {
		vector<string> names = pers_iter->second.getNames();
		vector<string>::iterator niter = names.begin();
		vector<string>::iterator nend = names.end();
		for (; niter != nend; ++niter) {
			cout << "PPRINT:: " << *niter << "(" << pers_iter->first << ")" << endl;
		}
		Persona tmp_persona = this->getPersona(pers_iter->first);
		std::vector<boost::shared_ptr<Action> > actions = tmp_persona.getActions();
		std::vector<boost::shared_ptr<Action> >::iterator aiter = actions.begin();
		std::vector<boost::shared_ptr<Action> >::iterator aend = actions.end();
		for (; aiter != aend; ++aiter) {
			vector<DrtPred> drs = (*aiter)->getDrs();
			puts("PERSSS::::");
			print_vector(drs);
		}
	}
}

void DrsPersonae::addPersonae(const DrsPersonae &dp)
{
	codes_.insert(codes_.end(), dp.codes_.begin(), dp.codes_.end());
	preds_.insert(preds_.end(), dp.preds_.begin(), dp.preds_.end());

	if (personae_.size()) {
		MapStPers::const_iterator piter = dp.personae_.begin();
		MapStPers::const_iterator pend = dp.personae_.end();
		for (; piter != pend; ++piter) {
			Persona tmp_persona = piter->second;
			string ref = tmp_persona.getReference();

			if (debug) {
				cout << "ADDING_REF::: " << ref << endl;
			}

			MapStPers::iterator this_piter = personae_.find(ref);
			if (this_piter != personae_.end())
				this_piter->second.addPersona(tmp_persona);
			else
				personae_[ref] = tmp_persona;
		}
		// personae_.insert(dp.personae_.begin(), dp.personae_.end() );
	} else
		personae_ = dp.personae_;

	if (debug) {
		cout << "ADDING_REF2::: " << endl;
	}

	// add the verb names in dp
	if (verb_names_.size()) {
		verb_names_.insert(dp.verb_names_.begin(), dp.verb_names_.end());
		//insert_in_map(verb_names_, dp.verb_names_ );
	} else
		verb_names_ = dp.verb_names_;

	if (debug) {
		cout << "ADDING_REF3::: " << endl;
	}
	// add the verb preds in dp
	if (verb_preds_.size()) {
		verb_preds_.insert(dp.verb_preds_.begin(), dp.verb_preds_.end());
		//insert_in_map(verb_preds_, dp.verb_preds_ );
	} else
		verb_preds_ = dp.verb_preds_;

	if (debug) {
		cout << "ADDING_REF4::: " << endl;
	}

	if (adverbs_.size()) {
		adverbs_.insert(dp.adverbs_.begin(), dp.adverbs_.end());
		//insert_in_map(adverbs_, dp.adverbs_ );
	} else
		adverbs_ = dp.adverbs_;
	if (debug) {
		cout << "ADDING_REF5::: " << endl;
	}
	// add the names_to_refs_ in dp
	if (names_to_refs_.size()) {
		names_to_refs_.insert(dp.names_to_refs_.begin(), dp.names_to_refs_.end());
		//insert_in_map(names_to_refs_, dp.names_to_refs_ );
	} else
		names_to_refs_ = dp.names_to_refs_;
	if (debug) {
		cout << "ADDING_REF6::: " << endl;
	}
	if(debug) {
		cout << "ADDING_SUBJ_REFS::: ";
		cout << subj_refs_.size() << " " << dp.subj_refs_.size() << endl;
	}
	if (subj_refs_.size()) {
		//subj_refs_.insert(dp.subj_refs_.begin(), dp.subj_refs_.end());
		insert_in_map(subj_refs_, dp.subj_refs_ );
		if(debug) {
			cout << "ADDING_SUBJ_REFS2::: ";
			cout << subj_refs_.size() << endl;
		}
	} else
		subj_refs_ = dp.subj_refs_;
	if (debug) {
		cout << "ADDING_REF7::: " << endl;
	}
	if(debug) {
		cout << "ADDING_OBJ_REFS::: ";
		cout << obj_refs_.size() << " " << dp.obj_refs_.size() << endl;
	}

	if (obj_refs_.size()) {
		//obj_refs_.insert(dp.obj_refs_.begin(), dp.obj_refs_.end());
		insert_in_map(obj_refs_, dp.obj_refs_ );
		if(debug) {
			cout << "ADDING_OBJ_REFS2::: ";
			cout << obj_refs_.size() << endl;
		}
	} else
		obj_refs_ = dp.obj_refs_;
	if (name_refs_.size()) {
		//name_refs_.insert(dp.name_refs_.begin(), dp.name_refs_.end());
		insert_in_map(name_refs_, dp.name_refs_ );
	} else
		name_refs_ = dp.name_refs_;
	if (debug) {
		cout << "ADDING_REF8::: " << endl;
	}
	// add the persona_pointer_ in dp
	if (persona_pointer_.size()) {
		persona_pointer_.insert(dp.persona_pointer_.begin(), dp.persona_pointer_.end());
	} else
		persona_pointer_ = dp.persona_pointer_;
	if (debug) {
		cout << "ADDING_REF9::: " << endl;
	}
	// add the subordinates in dp
	if (subord_verbs_.size()) {
		subord_verbs_.insert(dp.subord_verbs_.begin(), dp.subord_verbs_.end());
	} else
		subord_verbs_ = dp.subord_verbs_;
	if (debug) {
		cout << "ADDING_REF10::: " << endl;
	}
	references_.insert(references_.end(), dp.references_.begin(), dp.references_.end());
	if (debug) {
		cout << "ADDING_REF11::: " << endl;
	}
}

void DrsPersonae::addReferences(DrsPersonae dp)
// Only add the references and the connected personae. It does not save the actions
{
	MapStPers::iterator piter = dp.personae_.begin();
	MapStPers::iterator pend = dp.personae_.end();
	for (; piter != pend; ++piter)
		piter->second.clearActions();

	if (personae_.size()) {
		MapStPers::const_iterator piter = dp.personae_.begin();
		MapStPers::const_iterator pend = dp.personae_.end();
		for (; piter != pend; ++piter) {
			Persona tmp_persona = piter->second;
			string ref = tmp_persona.getReference();

			MapStPers::iterator this_piter = personae_.find(ref);
			if (this_piter != personae_.end())
				this_piter->second.addPersona(tmp_persona);
			else
				personae_[ref] = tmp_persona;
		}
	} else
		personae_ = dp.personae_;

	// add the verb names in dp
	if (verb_names_.size()) {
		verb_names_.insert(dp.verb_names_.begin(), dp.verb_names_.end());
		//insert_in_map(verb_names_, dp.verb_names_ );
	} else
		verb_names_ = dp.verb_names_;

	// add the verb preds in dp
	if (verb_preds_.size()) {
		verb_preds_.insert(dp.verb_preds_.begin(), dp.verb_preds_.end());
		//insert_in_map(verb_preds_, dp.verb_preds_ );
	} else
		verb_preds_ = dp.verb_preds_;

	if (adverbs_.size()) {
		adverbs_.insert(dp.adverbs_.begin(), dp.adverbs_.end());
		//insert_in_map(adverbs_, dp.adverbs_ );
	} else
		adverbs_ = dp.adverbs_;

	// add the names_to_refs_ in dp
	if (names_to_refs_.size()) {
		names_to_refs_.insert(dp.names_to_refs_.begin(), dp.names_to_refs_.end());
		//insert_in_map(names_to_refs_, dp.names_to_refs_ );
	} else
		names_to_refs_ = dp.names_to_refs_;

	if (subj_refs_.size()) {
		//subj_refs_.insert(dp.subj_refs_.begin(), dp.subj_refs_.end());
		insert_in_map(subj_refs_, dp.subj_refs_ );
	} else
		subj_refs_ = dp.subj_refs_;

	if (obj_refs_.size()) {
		//obj_refs_.insert(dp.obj_refs_.begin(), dp.obj_refs_.end());
		insert_in_map(obj_refs_, dp.obj_refs_ );
	} else
		obj_refs_ = dp.obj_refs_;
	if (name_refs_.size()) {
		//name_refs_.insert(dp.name_refs_.begin(), dp.name_refs_.end());
		insert_in_map(name_refs_, dp.name_refs_ );
	} else
		name_refs_ = dp.name_refs_;

	// add the persona_pointer_ in dp
	if (persona_pointer_.size()) {
		persona_pointer_.insert(dp.persona_pointer_.begin(), dp.persona_pointer_.end());
	} else
		persona_pointer_ = dp.persona_pointer_;

	// add the subordinates in dp
	if (subord_verbs_.size()) {
		subord_verbs_.insert(dp.subord_verbs_.begin(), dp.subord_verbs_.end());
	} else
		subord_verbs_ = dp.subord_verbs_;

	references_.insert(references_.end(), dp.references_.begin(), dp.references_.end());
}

void Persona::sort()
{
	std::sort(names_.begin(), names_.end());
	names_.erase(std::unique(names_.begin(), names_.end()), names_.end());

	std::sort(pred_names_.begin(), pred_names_.end());
	pred_names_.erase(std::unique(pred_names_.begin(), pred_names_.end()), pred_names_.end());

	std::sort(actions_.begin(), actions_.end(), compare_actions);
	actions_.erase(std::unique(actions_.begin(), actions_.end()), actions_.end());

	std::sort(specifications_.begin(), specifications_.end());
	specifications_.erase(std::unique(specifications_.begin(), specifications_.end()), specifications_.end());
}

void DrsPersonae::sort()
{
	std::sort(references_.begin(), references_.end());
	references_.erase(std::unique(references_.begin(), references_.end()), references_.end());

	if (personae_.size()) {
		MapStPers::iterator piter = personae_.begin();
		MapStPers::iterator pend = personae_.end();
		for (; piter != pend; ++piter) {
			piter->second.sort();
		}
	}
}

vector<pair<DrtPred, Action> > DrsPersonae::getSubordinates(const string &verb_ref) const
{
	vector<pair<DrtPred, Action> > to_return;
	MapStTuple::const_iterator sub_iter = subord_verbs_.find(verb_ref);

	if (debug) {
		puts("get_SUB::::");
		cout << verb_ref << endl;
	}
	if (sub_iter != subord_verbs_.end()) {
		if (debug) {
			puts("get_SUB2::::");
		}
		vector<boost::tuple<DrtPred, string, string> > sub_tuple = sub_iter->second;
		for (int m = 0; m < sub_tuple.size(); ++m) {
			DrtPred type_pred = sub_tuple.at(m).get<0>();
			string persona_ref = sub_tuple.at(m).get<1>();
			string verb_ref = sub_tuple.at(m).get<2>();
			if (debug) {
				cout << "get_SUB2.5::: " << type_pred << " " << persona_ref << " " << verb_ref << endl;
			}
			try {
				Persona p = this->getPersona(persona_ref);
				Action act = p.getActionFromVerbRef(verb_ref);
				to_return.push_back(make_pair(type_pred, act));
				if (debug) {
					cout << "get_SUB3::: " << type_pred << endl;
				}
			} catch (std::runtime_error &e) {
				if (debug) {
					cout << e.what() << endl;
				}
			}
		}
	}

	return to_return;
}

vector<string> DrsPersonae::getVerbNames(const string &ref) const
{
	MapStVDPred::const_iterator verbs_iter = verb_preds_.find(ref);
	vector<DrtPred> tmp_vpred;
	if (verbs_iter != verb_preds_.end()) {
		tmp_vpred = verbs_iter->second;
	}
	vector<string> to_return;
	for (int n = 0; n < tmp_vpred.size(); ++n)
		to_return.push_back(extract_header(tmp_vpred.at(n)));
	return to_return;
}

vector<DrtPred> DrsPersonae::getVerbPreds(const string &ref) const
{
	MapStVDPred::const_iterator verbs_iter = verb_preds_.find(ref);
	if (verbs_iter != verb_preds_.end()) {
		return verbs_iter->second;
	}
	return vector<DrtPred>();
}

vector<string> DrsPersonae::getRefFromName(const string &name) const
{
	PMapStVSt::const_iterator name_iter = names_to_refs_.find(name);
	if (name_iter != names_to_refs_.end()) {
		return name_iter->second;
	}
	return vector<string>();
}

vector<DrtPred> DrsPersonae::getAdverbs(const string &ref) const
{
	vector<DrtPred> to_return;
	MapStVDPred::const_iterator miter = adverbs_.find(ref);
	if (miter != adverbs_.end())
		to_return = miter->second;

	return to_return;
}

void DrsPersonae::clear()
{
	MapStPers::iterator piter = this->personae_.begin();
	MapStPers::iterator pend = this->personae_.end();
	for (; piter != pend; ++piter)
		piter->second.clearActions();
	personae_.clear();
	verb_names_.clear();
	verb_preds_.clear();
	adverbs_.clear();
	subord_verbs_.clear();
}

bool DrsPersonae::refIsSubj(const string &str) const
{
	MapStBool::const_iterator miter = subj_refs_.find(str);
	if (miter != subj_refs_.end()) {
		return true;
	}
	return false;
}

bool DrsPersonae::refIsObj(const string &str) const
{
	MapStBool::const_iterator miter = obj_refs_.find(str);
	if (miter != obj_refs_.end()) {
		return true;
	}
	return false;
}



vector<SPAction> DrsPersonae::getSubjActions(const string &str) const
{
	MapStBool::const_iterator miter = subj_refs_.find(str);
	if (miter != subj_refs_.end()) {
		vector<SPAction> vspa= miter->second;
		std::sort(vspa.begin(), vspa.end(), compare_actions);
		vspa.erase(std::unique(vspa.begin(), vspa.end()), vspa.end());
		return vspa;
	}
	return vector<SPAction>();
}

vector<SPAction> DrsPersonae::getObjActions(const string &str) const
{
	MapStBool::const_iterator miter = obj_refs_.find(str);
	if (miter != obj_refs_.end()) {
		vector<SPAction> vspa= miter->second;
		std::sort(vspa.begin(), vspa.end(), compare_actions);
		vspa.erase(std::unique(vspa.begin(), vspa.end()), vspa.end());
		return vspa;
	}
	return vector<SPAction>();
}

vector<SPAction> DrsPersonae::getNameActions(const string &str) const
{
	MapStBool::const_iterator miter = name_refs_.find(str);
	if (miter != name_refs_.end()) {
		vector<SPAction> vspa= miter->second;
		std::sort(vspa.begin(), vspa.end(), compare_actions);
		vspa.erase(std::unique(vspa.begin(), vspa.end()), vspa.end());
		return vspa;
	}
	return vector<SPAction>();
}

void DrsPersonae::clearUseless()
{
	subj_refs_.clear();
	obj_refs_.clear();
	name_refs_.clear();
	references_.clear();
	persona_pointer_.clear();


	MapStPers::iterator piter = this->personae_.begin();
	MapStPers::iterator pend = this->personae_.end();
	for (; piter != pend; ++piter)
		piter->second.clearActions();
}
