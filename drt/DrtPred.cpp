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



#include"DrtPred.hpp"

const bool debug = false;

template<class T>
static void print_vector(std::vector<T> &vs)
{
	typename vector<T>::iterator tags_iter = vs.begin();
	while (tags_iter < vs.end()) {
		std::cout << (*tags_iter) << " ";
		++tags_iter;
	}
	std::cout << std::endl;
}

template<class T>
static bool shortfind(const vector<T> &vect, const T &element)
{
	if (find(vect.begin(), vect.end(), element) == vect.end())
		return false;
	return true;
}

// DrtMgu class

std::ostream &operator <<(std::ostream &out, const DrtMgu &rhs)
{
	rhs.print(out);
	return out;
}

void DrtMgu::operator /(const DrtMgu &rhs)
{
	DrtMgu::iterator liter = this->begin(), lend = this->end();
	for (; liter != lend; ++liter) {
		DrtMgu::const_iterator riter = rhs.begin(), rend = rhs.end();
		for (; riter != rend; ++riter) {
			if (liter->second == riter->first) {
				liter->second = riter->second;
			}
		}
	}
}

void DrtMgu::print(std::ostream &out) const
{
	DrtMgu::const_iterator liter = this->begin(), lend = this->end();
	for (; liter != lend; ++liter) {
		cout << liter->first << " -> " << liter->second << endl;
	}
}

void DrtMgu::add(const DrtMgu &rhs)
{
	*this / rhs;
	this->insert(this->end(), rhs.begin(), rhs.end());
}

void DrtMgu::addWithoutUnification(const DrtMgu &rhs)
{
	this->insert(this->end(), rhs.begin(), rhs.end());
}

bool DrtMgu::operator <(const DrtMgu &rhs) const
{
	DrtMgu::const_iterator liter = this->begin(), lend = this->end();
	for (; liter != lend; ++liter) {
		DrtMgu::const_iterator riter = rhs.begin(), rend = rhs.end();
		for (; riter != rend; ++riter) {
			if (*liter >= *riter)
				return false;
		}
	}
	return true;
}

void DrtMgu::addReverse(const DrtMgu &rhs)
{
	DrtMgu::const_iterator riter = rhs.begin(), rend = rhs.end();
	for (; riter != rend; ++riter) {
		this->add(riter->second, riter->first);
	}
}

void DrtMgu::uniVal(int num)
{
	///
}

// DrtPred class

void DrtPred::uniVal(int num)
{
	///
}

std::ostream &operator <<(std::ostream &out, const DrtPred &rhs)
{
	rhs.print(out);
	return out;
}

static bool is_quantifier(const string &str)
{
	if (str.size() && str.at(0) == '_')
		return true;
	return false;
}

static bool equal_atoms(const string &lhs, const string &rhs)
{
	vector<string> lstrings, rstrings;
	boost::split(lstrings, lhs, boost::is_any_of("|"));
	boost::split(rstrings, rhs, boost::is_any_of("|"));

	vector<string>::iterator liter, riter, lend, rend;
	liter = lstrings.begin();
	//riter = rstrings.begin();
	lend = lstrings.end();
	rend = rstrings.end();

	for (; liter != lend; ++liter) {
		for (riter = rstrings.begin(); riter != rend; ++riter) {
			if (liter->find("none") != string::npos || riter->find("none") != string::npos)
				continue;
			if (liter->find("subj") != string::npos && riter->find("subj") != string::npos)
				return true; // continue;
			if (liter->find("obj") != string::npos && riter->find("obj") != string::npos)
				return true; // continue;
			if (*liter == *riter)
				return true;
		}
	}

	return false;
}

bool DrtPred::unify(const DrtPred &rhs, DrtMgu *mgu) const
{
	static clock_t all_time = 0;
	clock_t start;
	if (debug)
		start = clock();

	if (debug) {
		cout << "UNIFY::: " << *this << " " << rhs << " " << children_.size() << " " << rhs.children_.size() << endl;
	}

	if (children_.size() != rhs.children_.size())
		return false;
	if (header_ != rhs.header_)
		return false;

	vector<string>::const_iterator liter = children_.begin(), lend = children_.end();
	vector<string>::const_iterator riter = rhs.children_.begin(), rend = rhs.children_.end();

	for (; liter != lend && riter != rend; ++liter, ++riter) {
		if (!equal_atoms(*liter, *riter)) {
			if (is_quantifier(*liter)) {
				mgu->add(*liter, *riter);
			} else if (is_quantifier(*riter)) {
				mgu->add(*riter, *liter);
			} else
				return false;
		}
	}

	if (debug) {
		clock_t end = clock();
		all_time += (end - start);
		cout << "Mtime12::: " << all_time / (double) CLOCKS_PER_SEC << endl;
	}

	return true;
}

bool DrtPred::unify(const DrtPred &rhs)
{
	if (children_.size() != rhs.children_.size())
		return false;

	vector<string>::iterator liter = children_.begin(), lend = children_.end();
	vector<string>::const_iterator riter = rhs.children_.begin(), rend = rhs.children_.end();

	for (; liter != lend && riter != rend; ++liter, ++riter) {
		if (!equal_atoms(*liter, *riter)) {
			if (is_quantifier(*liter)) {
				*liter = *riter;
			} else if (is_quantifier(*liter)) {
				//*riter = *liter;
			} else
				return false;
		}
	}
	return true;
}

void DrtPred::operator /(const DrtMgu &rhs)
{
	vector<string>::iterator citer, cend = children_.end();

	DrtMgu::const_iterator riter = rhs.begin(), rend = rhs.end();
	for (; riter != rend; ++riter) {
		citer = find(children_.begin(), children_.end(), riter->first);
		if (citer != cend)
			*citer = riter->second;
	}
}

bool DrtPred::operator ==(const DrtPred &rhs) const
{
	if (tag_ == rhs.tag_ && header_ == rhs.header_ && children_ == rhs.children_ && name_ == rhs.name_ )
		return true;
	return false;
}

bool DrtPred::operator <(const DrtPred &rhs) const
{
	if (name_ < rhs.name_ && tag_ < rhs.tag_ && header_ < rhs.header_ && children_ < rhs.children_
	    && anaphora_level_ < rhs.anaphora_level_ && is_pivot_ < rhs.is_pivot_ && weigth < rhs.weigth
	    && question_word_ < rhs.question_word_ && is_question_ < rhs.is_question_
	)
		return true;
	return false;
}

void DrtPred::print(std::ostream &out) const
{
	int diff;

	if (this->tag().size())
		out << header_ << "/" << this->tag();
	else
		out << header_;
	if(is_pivot_ || is_anaphora_)
		out << "#";
	if (this->is_pivot())
		out << "[pivot]";
	if (this->is_anaphora())
		out << "[anaphora]";

	out << '(';
	for (int n = 0; n < children_.size(); ++n) {
		if (n != 0)
			out << ',';
		out << children_.at(n);
	}
	out << ')';
}

bool DrtPred::equalAtoms(const PTEl &lhs, const PTEl &rhs) const
{
	if (lhs.uniVal != rhs.uniVal)
		return false;

	vector<string> lstrings, rstrings;
	boost::split(lstrings, lhs.str, boost::is_any_of("|"));
	boost::split(rstrings, rhs.str, boost::is_any_of("|"));

	vector<string>::iterator liter, riter, lend, rend;
	liter = lstrings.begin();
	riter = rstrings.begin();
	lend = lstrings.end();
	rend = rstrings.end();

	for (; liter != lend; ++liter) {
		for (; riter != rend; ++riter) {
			if (liter->find("none") != string::npos || riter->find("none") != string::npos)
				continue;
			if (*liter == *riter)
				return true;
		}
	}

	return false;
}

// Helper functions

double get_single_distance(const string &str)
{
	double single_distance = 0;
	int size = str.size();

	for (int n = 0; n < size; ++n) {
		if (isdigit(str.at(n))) {
			int end = n;
			while (end < size && isdigit(str.at(end)))
				++end;
			single_distance = boost::lexical_cast<int>(str.substr(n, end - n));
			break;
		}
	}
	return single_distance;
}

string get_string_distance(const string &str)
{
	string single_distance;
	int size = str.size();

	for (int n = 0; n < size; ++n) {
		if (isdigit(str.at(n))) {
			int end = n;
			while (end < size)
				++end;
			single_distance = str.substr(n, end - n);
			break;
		}
	}
	return single_distance;
}

bool compare_drt(const DrtPred &drt_sx, const DrtPred &drt_dx)
{
	int dist_sx, dist_dx;
	string str_sx, str_dx;
	string non_verb_str, subj_str, obj_str;

	// Complements marked with '@' go in the end
	string name_sx, name_dx;
	name_sx = extract_header(drt_sx);
	name_dx = extract_header(drt_dx);

	if (name_sx.size() == 0 || name_dx.size() == 0)
		return false;

	if (name_sx.at(0) == '@' && name_dx.at(0) != '@')
		return false;
	else if (name_sx.at(0) != '@' && name_dx.at(0) == '@')
		return true;
	else if (name_dx.at(0) == '@' && name_sx.at(0) == '@') // if both @-names order alphabetically
		return name_sx <= name_dx;

	// The subject goes before the verb and the object after
	if (drt_sx.is_verb() && !drt_dx.is_verb()) {
		subj_str = extract_subject(drt_sx);
		obj_str = extract_object(drt_sx);
		non_verb_str = extract_first_tag(drt_dx);
		if (subj_str == non_verb_str) {
			return false;
		} else if (obj_str == non_verb_str) {
			return true;
		} else {
			return true;
		}
	} else if (!drt_sx.is_verb() && drt_dx.is_verb()) {
		subj_str = extract_subject(drt_dx);
		obj_str = extract_object(drt_dx);
		non_verb_str = extract_first_tag(drt_sx);
		if (obj_str == non_verb_str)
			return false;
		else if (subj_str == non_verb_str)
			return true;
		else
			return false;
	}

	// If two words share the same reference do nothing
	str_sx = extract_first_tag(drt_sx);
	str_dx = extract_first_tag(drt_dx);

	string dist_str_sx = get_string_distance(str_sx);
	string dist_str_dx = get_string_distance(str_dx);
	//cout << dist_str_sx << ", "<< dist_str_dx<< endl;
	if (dist_str_sx < dist_str_dx)
		return true;

	//return dist_sx < dist_dx;
	return false;
}

static void gnomesort(vector<DrtPred>::iterator start, vector<DrtPred>::iterator end)
{
	vector<DrtPred> phrase_old(start, end);
	vector<DrtPred> phrase_new;
	vector<DrtPred>::iterator iter = start;

	int n = 0; // the cycle has to stop
	while (phrase_new != phrase_old && ++n < 20) {
		phrase_old = phrase_new;
		//print_vector(phrase_new);
		iter = start;
		while (boost::next(iter) != end) {
			if (compare_drt(*iter, *boost::next(iter)))
				++iter;
			else {
				DrtPred tmp = *iter;
				*iter = *boost::next(iter);
				++iter;
				*iter = tmp;
			}
		}
		phrase_new = vector<DrtPred>(start, end);
	}
}

void drt_sort(vector<DrtPred> &pre_drt)
{
	gnomesort(pre_drt.begin(), pre_drt.end());
}

vector<DrtPred> get_predicates_from_position(vector<DrtPred> preds, const vector<int> &pos)
{
	vector<DrtPred> ret;
	for (int n = 0; n < pos.size(); ++n) {
		ret.push_back(preds.at(pos.at(n)));
	}
	return ret;
}

vector<string> find_string_object_of_verb(const vector<DrtPred> &preds, int m)
{
	vector<string> cons;
	if (m >= preds.size())
		return cons;

	if (!preds.at(m).is_verb())  // check the verb is a verb
		return cons;
	DrtPred verb_cons = preds.at(m);
	string obj = extract_object(verb_cons);
	vector<DrtPred> connected_obj;

	connected_obj = get_predicates_from_position(preds, find_all_element_with_string(preds, obj));
	for (int n = 0; n < connected_obj.size(); ++n) {
		if (connected_obj.at(n).is_name())
			cons.push_back(extract_header(connected_obj.at(n)));
	}

	return cons;
}

vector<string> find_string_subject_of_verb(const vector<DrtPred> &preds, int m)
{
	vector<string> cons;
	if (m >= preds.size())
		return cons;

	if (!preds.at(m).is_verb())  // check the verb is a verb
		return cons;
	DrtPred verb_cons = preds.at(m);
	string subj = extract_subject(verb_cons);
	vector<DrtPred> connected_subj;

	connected_subj = get_predicates_from_position(preds, find_all_element_with_string(preds, subj));
	for (int n = 0; n < connected_subj.size(); ++n) {
		if (connected_subj.at(n).is_name())
			cons.push_back(extract_header(connected_subj.at(n)));
	}

	return cons;
}

vector<DrtPred> find_object_of_verb(const vector<DrtPred> &preds, int m)
{
	vector<DrtPred> cons;

	if (!preds.at(m).is_verb())  // check the verb is a verb
		return cons;
	DrtPred verb_cons = preds.at(m);
	string obj = extract_object(verb_cons);
	vector<DrtPred> connected_obj;

	connected_obj = get_predicates_from_position(preds, find_all_element_with_string(preds, obj));
	for (int n = 0; n < connected_obj.size(); ++n) {
		if (connected_obj.at(n).is_name())
			cons.push_back(connected_obj.at(n));
		else if (connected_obj.at(n).is_VBN()) // for cases like "to keep trained". Trained is the object of keep (in this drs model)
			cons.push_back(connected_obj.at(n));
	}

	return cons;
}

vector<DrtPred> find_subject_of_verb(const vector<DrtPred> &preds, int m)
{
	vector<DrtPred> cons;

	if (!preds.at(m).is_verb())  // check the verb is a verb
		return cons;
	DrtPred verb_cons = preds.at(m);
	string subj = extract_subject(verb_cons);
	vector<DrtPred> connected_subj;

	connected_subj = get_predicates_from_position(preds, find_all_element_with_string(preds, subj));
	for (int n = 0; n < connected_subj.size(); ++n) {
		if (connected_subj.at(n).is_name() || connected_subj.at(n).tag() == "EX")
			cons.push_back(connected_subj.at(n));
	}

	return cons;
}

int find_int_subject_of_verb(const vector<DrtPred> &preds, int m)
{
	if (!preds.at(m).is_verb())  // check the verb is a verb
		return -1;
	DrtPred verb_cons = preds.at(m);
	string subj = extract_subject(verb_cons);
	vector<DrtPred> connected_subj;

	connected_subj = get_predicates_from_position(preds, find_all_element_with_string(preds, subj));
	for (int n = 0; n < connected_subj.size(); ++n) {
		if ( (connected_subj.at(n).is_name() || connected_subj.at(n).tag() == "EX")
		)
			return n;
	}
	return -1;
}


vector<DrtPred> find_adverb_of_verb(const vector<DrtPred> &preds, int m)
{
	vector<DrtPred> cons;

	if (!preds.at(m).is_verb())  // check the verb is a verb
		return cons;
	DrtPred verb_cons = preds.at(m);
	string ref = extract_first_tag(verb_cons);
	vector<DrtPred> connected_subj;

	connected_subj = get_predicates_from_position(preds, find_all_element_with_string(preds, ref));
	for (int n = 0; n < connected_subj.size(); ++n) {
		if (connected_subj.at(n).is_adverb())
			cons.push_back(connected_subj.at(n));
	}

	return cons;
}

vector<vector<DrtPred> > find_complements_of_verb(const vector<DrtPred> &preds, int m)
{
	vector<vector<DrtPred> > cons;

	if (m < 0 || m >= preds.size())
		return cons;
	if (!preds.at(m).is_verb())  // check the verb is a verb
		return cons;
	DrtPred verb_cons = preds.at(m);
	string verb_ref = extract_first_tag(verb_cons);
	vector<DrtPred> connected_compl;

	connected_compl = get_predicates_from_position(preds, find_all_element_with_string(preds, verb_ref));
	for (int n = 0; n < connected_compl.size(); ++n) {
		string candidate = extract_header(connected_compl.at(n));
		if (candidate.size() == 0)
			continue; // safety check
		string fref = extract_first_tag(connected_compl.at(n));
		string sref = extract_second_tag(connected_compl.at(n));
		if ( //(candidate.at(0) == '@' || candidate == "from" || candidate == "to") /// Do the list of complements!!
		candidate.at(0) == '@' && candidate.find("@SUB") == string::npos
		//&& candidate.find("@PARENT") == string::npos
		// && !ref_is_verb(fref)
				&& !ref_is_verb(sref)) { // it is a complement
			string to_ref = extract_second_tag(connected_compl.at(n));
			DrtVect connected_names_tmp = get_predicates_from_position(preds, find_all_element_with_string(preds, to_ref));
			DrtVect connected_names;
			for (int m = 0; m < connected_names_tmp.size(); ++m) { // a complement can have a dangling complement at the end
				if (!connected_names_tmp.at(m).is_complement())
					connected_names.push_back(connected_names_tmp.at(m));
			}
			connected_names.insert(connected_names.begin(), connected_compl.at(n));
			cons.push_back(connected_names);
		}
	}

	return cons;
}

DrtPred find_pointer_to_verb(const vector<DrtPred> &preds, int m)
{
	DrtPred verb_cons = preds.at(m);
	string verb_ref = extract_first_tag(verb_cons);

	DrtPred head(string("@HEAD(root,") + verb_ref + ")");
	if (!preds.at(m).is_verb())  // check the verb is a verb
		return head;

	for (int n = 0; n < preds.size(); ++n) {
		if (preds.at(n).is_complement()) {
			string fref = extract_second_tag(preds.at(n));
			string sref = extract_second_tag(preds.at(n));
			if (sref == verb_ref && ref_is_verb(fref))
				return preds.at(n);
		}
	}

	return head;
}

vector<pair<pair<string, string>, vector<string> > > find_string_complement_of_verb(const vector<DrtPred> &preds, int m)
// The return vector is a pair of < pair<@COMPLEMENT_TYPE,verb_ref>,reference_string_of_the_objects_pointed_at> and vector<names_of_the_objects_pointed_at>
{
	vector<pair<pair<string, string>, vector<string> > > cons;

	if (!preds.at(m).is_verb())  // check the verb is a verb
		return cons;
	DrtPred verb_cons = preds.at(m);
	string verb_ref = extract_first_tag(verb_cons);
	vector<DrtPred> connected_compl;

	connected_compl = get_predicates_from_position(preds, find_all_element_with_string(preds, verb_ref));
	for (int n = 0; n < connected_compl.size(); ++n) {
		string candidate = extract_header(connected_compl.at(n));
		if (candidate.size() == 0)
			continue; // safety check
		if (candidate.at(0) == '@') { // it is a complement
			string to_ref = extract_second_tag(connected_compl.at(n));
			vector<DrtPred> connected_names = get_predicates_from_position(preds, find_all_element_with_string(preds, to_ref));
			vector<string> connected_names_str;
			for (int j = 0; j < connected_names.size(); ++j) {
				if (connected_names.at(j).is_name()) // You don't want to include things like "@TIME_AT", etc...
					connected_names_str.push_back(extract_header(connected_names.at(j)));
			}
			cons.push_back(make_pair(make_pair(candidate, to_ref), connected_names_str));
		}
	}

	return cons;
}

vector<DrtPred> find_subordinates_of_verb(const vector<DrtPred> &preds, int m)
{
	vector<DrtPred> cons;

	if (!preds.at(m).is_verb())  // check the verb is a verb
		return cons;

	vector<DrtPred> connected_subj, connected_obj;
	string verb_tag;
	verb_tag = extract_first_tag(preds.at(m));

	for (int n = 0; n < preds.size(); ++n) {
		string head = extract_header(preds.at(n));
		string first_tag = extract_first_tag(preds.at(n));
		if (head.size() && head == "@SUBORD" && first_tag == verb_tag) {
			string subord = extract_second_tag(preds.at(n));
			vector<int> positions = find_all_element_with_string(preds, subord);
			for (int j = 0; j < positions.size(); ++j) {
				if (preds.at(positions.at(j)).is_verb())
					cons.push_back(preds.at(positions.at(j)));
			}
		}
	}

	return cons;
}

vector<DrtPred> find_attached_to_verb(const vector<DrtPred> &preds, int m)
// It returns only the verb, subject and object
{
	vector<DrtPred> cons;

	if (!preds.at(m).is_verb())  // check the verb is a verb
		return cons;

	vector<DrtPred> connected_subj, connected_obj;
	DrtPred verb_cons;
	verb_cons = preds.at(m);
	cons.push_back(verb_cons);

	string subj = extract_subject(verb_cons);
	string obj = extract_object(verb_cons);
	connected_subj = get_predicates_from_position(preds, find_all_element_with_string(preds, subj));
	for (int n = 0; n < connected_subj.size(); ++n) {
		string head = extract_header(connected_subj.at(n));
		if (head.size() && head.at(0) != '@')
			cons.push_back(connected_subj.at(n));

	}
	connected_obj = get_predicates_from_position(preds, find_all_element_with_string(preds, obj));
	for (int n = 0; n < connected_obj.size(); ++n) {
		string head = extract_header(connected_obj.at(n));
		if (head.size() && head.at(0) != '@')
			cons.push_back(connected_obj.at(n));
	}

	return cons;
}

vector<DrtPred> find_all_attached_to_verb(vector<DrtPred> preds, int m)
{
	vector<DrtPred> cons;

	if (debug) {
		cout << "ALL_PREDS::: ";
		print_vector(preds);
	}

	if (!preds.at(m).is_verb())  // check the verb is a verb
		return cons;

	//drt_sort(preds);

	vector<DrtPred> connected_subj, connected_obj, connected_verb;
	DrtPred verb_cons;
	verb_cons = preds.at(m);

	// erases items that are connected and point to the same reference, e.g. @PLACE_AT(verb3,verb3)
	string s1, s2, s3;
	for (int n = 0; n < preds.size(); ++n) {
		s1 = s2 = s3 = "";
		if (preds.at(n).is_verb()) {
			s1 = extract_subject(preds.at(n));
			s2 = extract_object(preds.at(n));
			s3 = extract_first_tag(preds.at(n));
		} else {
			s1 = extract_first_tag(preds.at(n));
			s2 = extract_second_tag(preds.at(n));
		}
		if ((preds.at(n).is_complement() || preds.at(n).is_verb())
				&& ((s1 == s2 && s1 != "none") || s2 == s3 || s1 == s3)) {
			preds.erase(preds.begin() + n);
		}
	}

	if (debug) {
		cout << "ALL_PREDS2::: ";
		print_vector(preds);
	}

	string verb = extract_first_tag(verb_cons);

	connected_verb = get_predicates_from_position(preds, find_all_element_with_string(preds, verb));
	vector<DrtVect> items;
	items.push_back(connected_verb);

	if (debug) {
		puts("CONNECTED_VERB::: ");
		print_vector(connected_verb);
	}

	vector<DrtPred> new_terms, new_terms2, new_terms3;
	/// The next cycle erases the @conditions and @conjunctions and
	/// take the terms that are connected to the verb indirectly.
	for (int j = 1; j < 10; ++j) { // maximum 10 steps from the verb
		items.push_back(vector<DrtPred>());
		for (int n = 0; n < items.at(j - 1).size(); ++n) {
			string head_str = extract_header(items.at(j - 1).at(n));
			if (!items.at(j - 1).at(n).is_delete()
					&& (head_str == "@CONDITION" || head_str == "@CONJUNCTION" || head_str == "CONJUNCTION"
							|| head_str == "@COORDINATION" || head_str == "@PAR" || head_str == "@DISJUNCTION")) {
				items.at(j - 1).erase(items.at(j - 1).begin() + n);
				--n;
			} else {
				if (debug) {
					cout << "PRED_ATTACHED::: " << items.at(j - 1).at(n) << endl;
				}
				if (items.at(j - 1).at(n).is_verb()) {
					// if an element is a verb then adds the subj, obj, and specifications of the subj and obj
					string subj = extract_subject(items.at(j - 1).at(n));
					string obj = extract_object(items.at(j - 1).at(n));
					if (debug) {
						cout << "PRED_ATTACHED2::: " << subj << " " << obj << endl;
					}

					new_terms = get_predicates_from_position(preds, find_all_element_with_string(preds, subj));
					new_terms2 = get_predicates_from_position(preds, find_all_element_with_string(preds, obj));
					if (debug) {
						cout << "ALL_PREDS3::: ";
						print_vector(preds);
					}
					if (debug) {
						cout << "PRED_ATTACHED3::: ";
						print_vector(new_terms);
						print_vector(new_terms2);
					}

					for (int m = 0; m < new_terms2.size();) {
						if (find(new_terms.begin(), new_terms.end(), new_terms2.at(m)) != new_terms.end())
							new_terms2.erase(new_terms2.begin() + m);
						else
							++m;
					}
					new_terms.insert(new_terms.end(), new_terms2.begin(), new_terms2.end());
				} else if (items.at(j - 1).at(n).is_name()) {
					// a noun is put into the list
					string first_tag = extract_first_tag(items.at(j - 1).at(n));
					new_terms = get_predicates_from_position(preds, find_all_element_with_string(preds, first_tag));
				} else {
					// an element attached to a complement is added to the list
					string second_tag = extract_second_tag(items.at(j - 1).at(n));
					if (debug) {
						cout << "SECOND_TAG::: " << second_tag << endl;
					}
					new_terms = get_predicates_from_position(preds, find_all_element_with_string(preds, second_tag));
				}
				for (int m = 0; m < new_terms.size();) {
					if (find(items.at(0).begin(), items.at(0).end(), new_terms.at(m)) != items.at(0).end()
							|| find(connected_verb.begin(), connected_verb.end(), new_terms.at(m)) != connected_verb.end()
							|| find(items.at(j).begin(), items.at(j).end(), new_terms.at(m)) != items.at(j).end())
						new_terms.erase(new_terms.begin() + m);
					else
						++m;
				}
				items.at(j).insert(items.at(j).end(), new_terms.begin(), new_terms.end());
			}
		}
		if (items.at(j).size())
			items.at(0).insert(items.at(0).end(), items.at(j).begin(), items.at(j).end());
		else
			break;
	}
	cons.insert(cons.end(), items.at(0).begin(), items.at(0).end());

	return cons;
}

vector<int> find_int_attached_to_verb(const vector<DrtPred> &preds, int m)
{
	vector<int> cons;
	if (m < 0 || m > preds.size())
		return cons;
	if (!preds.at(m).is_verb())  // check the verb is a verb
		return cons;

	vector<int> connected_subj, connected_obj;
	DrtPred verb_cons;
	verb_cons = preds.at(m);
	cons.push_back(m);

	string subj = extract_subject(verb_cons);
	string obj = extract_object(verb_cons);
	//if(subj != condition_from && subj != condition_to) {
	{
		connected_subj = find_all_element_with_string(preds, subj);
		cons.insert(cons.begin(), connected_subj.begin(), connected_subj.end());
	}
	//if(obj != condition_from && obj != condition_to) {
	{
		connected_obj = find_all_element_with_string(preds, obj);
		cons.insert(cons.begin(), connected_obj.begin(), connected_obj.end());
	}

	return cons;
}

vector<int> find_all_joined_element_with_string(const vector<DrtPred> &pre_drt, string str)
// returns all the element with reference "str" and the ones joined to them by "and" or "or"
{
	vector<int> ret_int;
	int m;

	vector<DrtPred>::const_iterator diter;
	vector<string> all_refs;
	all_refs.push_back(str);

	for (int n = 0; n < all_refs.size(); ++n) {
		for (diter = pre_drt.begin(), m = 0; diter != pre_drt.end(); ++diter, ++m) {
			string str = all_refs.at(n);
			string head = extract_header(*diter);
			string fstr = extract_first_tag(*diter);
			string sstr = extract_second_tag(*diter);
			if (fstr != str)
				continue;
			if (diter->is_name()) {
				ret_int.push_back(m);
			} else if (diter->is_complement() && (head == "@AND" || head == "@OR") && !shortfind(all_refs, sstr)) {
				// if the complement is @AND(A,B) adds the B to the list of the refs
				all_refs.push_back(sstr);
			}
		}
	}
	return ret_int;
}

int find_pivot_with_string(const vector<DrtPred> &pre_drt, string str)
{
	vector<int> ints = find_all_element_with_string(pre_drt, str);
	for (int pos = 0; pos < ints.size(); ++pos) {
		if (pre_drt.at(ints.at(pos)).is_pivot())
			return ints.at(pos);
	}

	return -1;
}

vector<int> find_all_element_with_second_string(const vector<DrtPred> &pre_drt, string str)
{
	vector<int> ret_int;
	int m = 0;
	vector<DrtPred>::const_iterator diter = pre_drt.begin();
	for (; diter != pre_drt.end(); ++diter, ++m) {
		string sref = extract_second_tag(*diter);
		if (sref == str)
			ret_int.push_back(m);
	}
	return ret_int;
}

vector<int> find_all_element_with_string(const vector<DrtPred> &pre_drt, string str)
{
	vector<int> ret_int;
	int m = 0;

	vector<DrtPred>::const_iterator diter = pre_drt.begin();

	for (; diter != pre_drt.end(); ++diter, ++m) {
		string vstr = extract_first_tag(*diter);
		if (vstr == str)
			ret_int.push_back(m);
	}
	return ret_int;
}

vector<int> find_all_adverbs_with_string(const vector<DrtPred> &pre_drt, string str)
{
	vector<int> ret_int;
	int m = 0;

	vector<DrtPred>::const_iterator diter = pre_drt.begin();

	for (; diter != pre_drt.end(); ++diter, ++m) {
		string vstr = extract_first_tag(*diter);
		if (vstr == str && diter->is_adverb())
			ret_int.push_back(m);
	}
	return ret_int;
}

vector<int> find_all_element_with_string_no_delete(const vector<DrtPred> &pre_drt, string str)
// exclude elements signed for deletion ("...:DELETE" in the name)
{
	vector<int> ret_int;
	int m = 0;

	vector<DrtPred>::const_iterator diter = pre_drt.begin();

	for (; diter != pre_drt.end(); ++diter, ++m) {
		string vstr = extract_first_tag(*diter);
		string head = extract_header(*diter);
		if (head.find(":DELETE") != string::npos)
			continue;
		if (vstr == str)
			ret_int.push_back(m);
	}
	return ret_int;
}

vector<int> find_all_names_with_string_no_delete(const vector<DrtPred> &pre_drt, string str)
// exclude elements signed for deletion ("...:DELETE" in the name)
{
	vector<int> ret_int;
	int m = 0;

	vector<DrtPred>::const_iterator diter = pre_drt.begin();

	for (; diter != pre_drt.end(); ++diter, ++m) {
		if (!diter->is_name() && !diter->is_WP() && !diter->is_WDT())
			continue;
		string vstr = extract_first_tag(*diter);
		string head = extract_header(*diter);
		if (head.find(":DELETE") != string::npos)
			continue;
		if (vstr == str)
			ret_int.push_back(m);
	}
	return ret_int;
}

vector<int> find_all_element_with_string_everywhere(const vector<DrtPred> &pre_drt, string str)
{
	vector<int> ret_int;
	int m = 0;

	vector<DrtPred>::const_iterator diter = pre_drt.begin();

	for (; diter != pre_drt.end(); ++diter, ++m) {
		string vstr1 = extract_first_tag(*diter);
		string vstr2 = extract_second_tag(*diter);
		string vstr3 = extract_third_tag(*diter);
		string head = extract_header(*diter);
		if (vstr1 == str)
			ret_int.push_back(m);
		else if (vstr2 == str)
			ret_int.push_back(m);
		else if (vstr3 == str)
			ret_int.push_back(m);
	}
	return ret_int;
}

int find_element_with_string(const vector<DrtPred> &pre_drt, string str)
{
	int m = 0;

	vector<DrtPred>::const_iterator diter = pre_drt.begin();

	for (; diter != pre_drt.end(); ++diter, ++m) {
		string vstr = extract_first_tag(*diter);
		string head = extract_header(*diter);
		if (head.find(":DELETE") != string::npos)
			continue;
		if (vstr == str)
			return m;
	}
	if (m == pre_drt.size())
		m = -1;
	return m;
}

int find_name_with_string(const vector<DrtPred> &pre_drt, string str)
{
	int m = 0;

	vector<DrtPred>::const_iterator diter = pre_drt.begin();

	for (; diter != pre_drt.end(); ++diter, ++m) {
		if (!diter->is_name())
			continue;
		string vstr = extract_first_tag(*diter);
		string head = extract_header(*diter);
		if (head.find(":DELETE") != string::npos)
			continue;
		if (vstr == str)
			return m;
	}
	if (m == pre_drt.size())
		m = -1;
	return m;
}

int find_pivot_name_with_string(const vector<DrtPred> &pre_drt, string str)
{
	int m = 0;

	vector<DrtPred>::const_iterator diter = pre_drt.begin();

	for (; diter != pre_drt.end(); ++diter, ++m) {
		if (!diter->is_name() || !diter->is_pivot())
			continue;
		string vstr = extract_first_tag(*diter);
		string head = extract_header(*diter);
		if (head.find(":DELETE") != string::npos)
			continue;
		if (vstr == str)
			return m;
	}
	if (m == pre_drt.size())
		m = -1;
	return m;
}

static bool is_AUX(const DrtPred &pred)
{
	string head = extract_header(pred);

	vector<string> candidates;
	candidates.push_back("AUX");
	candidates.push_back("PASSIVE_AUX");

	if (find(candidates.begin(), candidates.end(), head) != candidates.end())
		return true;
	return false;
}

int find_verb_with_string(const vector<DrtPred> &pre_drt, string str)
{
	int m = 0;

	vector<DrtPred>::const_iterator diter = pre_drt.begin();

	for (; diter != pre_drt.end(); ++diter, ++m) {
		if (diter->is_verb()) {
			string vstr = extract_first_tag(*diter);
			string head = extract_header(*diter);
			if (head.find(":DELETE") != string::npos)
				continue;
			if (vstr == str && !is_AUX(head))
				return m;
		}
	}
	if (m == pre_drt.size())
		m = -1;
	return m;
}

vector<int> find_next_of_string(const vector<DrtPred> &pre_drt, const string &str)
// Finds all the preds that have as a first reference the "str"
{
	vector<int> ret_m;
	int m = 0;

	vector<DrtPred>::const_iterator diter = pre_drt.begin();

	for (; diter != pre_drt.end(); ++diter, ++m) {
		string vstr = extract_first_tag(*diter);
		string head = extract_header(*diter);
		if (head.find(":DELETE") != string::npos)
			continue;
		if (vstr == str)
			ret_m.push_back(m);
	}
	return ret_m;
}

static vector<int> delete_duplicates(vector<int> vect, vector<int>::iterator begin, vector<int>::iterator end)
{
	vector<int>::iterator iter = begin;
	vector<int>::iterator to_delete;
	for (; iter != end; ++iter) {
		to_delete = find(vect.begin(), vect.end(), *iter);
		if (to_delete != vect.end())
			vect.erase(to_delete);
	}

	return vect;
}

int get_position_from_predicate(const std::vector<DrtPred> &drtvect, const DrtPred &pred)
{
	for (int n = 0; n < drtvect.size(); ++n) {
		if (drtvect.at(n) == pred)
			return n;
	}
	return -1;
}

vector<DrtPred> get_elements_next_of(const std::vector<DrtPred> &drtvect, const DrtPred &pred)
{
	int pos = get_position_from_predicate(drtvect, pred);
	vector<int> positions = get_elements_next_of(drtvect, pos);
	return get_predicates_from_position(drtvect, positions);
}

vector<int> get_elements_next_of(const std::vector<DrtPred> &drtvect, int pos)
{
	string rstr;
	vector<int> nexts;
	if (drtvect.at(pos).is_name()) {
		rstr = extract_header(drtvect.at(pos));
		nexts = find_next_of_string(drtvect, rstr);
	}
	if (drtvect.at(pos).is_verb()) {
		rstr = extract_object(drtvect.at(pos));
		nexts = find_next_of_string(drtvect, rstr);
	} else {
		rstr = extract_second_tag(drtvect.at(pos));
		nexts = find_next_of_string(drtvect, rstr);
		nexts.insert(nexts.begin(), pos);
	}
	int m = 0, size_old = -1;
	while (m < nexts.size() && size_old != nexts.size() && nexts.size() != 0 && m < drtvect.size()) {
		if (drtvect.at(nexts.at(m)).is_name() || drtvect.at(nexts.at(m)).is_adjective()) {
			string nex = extract_first_tag(drtvect.at(nexts.at(m)));
			vector<int> nexts_tmp = find_next_of_string(drtvect, nex);
			nexts_tmp = delete_duplicates(nexts_tmp, nexts.begin(), nexts.end());
			nexts.insert(nexts.end(), nexts_tmp.begin(), nexts_tmp.end());
		} else if (drtvect.at(nexts.at(m)).is_verb()) {
			string obj = extract_object(drtvect.at(nexts.at(m)));
			vector<int> nexts_tmp = find_next_of_string(drtvect, obj);
			nexts_tmp = delete_duplicates(nexts_tmp, nexts.begin(), nexts.end());
			nexts.insert(nexts.end(), nexts_tmp.begin(), nexts_tmp.end());
		} else {
			string nex = extract_second_tag(drtvect.at(nexts.at(m)));
			vector<int> nexts_tmp = find_next_of_string(drtvect, nex);
			nexts_tmp = delete_duplicates(nexts_tmp, nexts.begin(), nexts.end());
			nexts.insert(nexts.end(), nexts_tmp.begin(), nexts_tmp.end());
		}
		++m;
	}

	return nexts;
}

void operator /(std::vector<DrtPred> &predVect, const DrtMgu &upg)
{
	std::vector<DrtPred>::iterator predIter;
	std::vector<DrtPred>::iterator endPred = predVect.end();

	for (predIter = predVect.begin(); predIter != endPred; predIter++) {
		(*predIter) / upg;
	}
}

void operator /(std::vector<DrtVect> &predVect, const DrtMgu &upg)
{
	std::vector<DrtVect>::iterator predIter;
	std::vector<DrtVect>::iterator endPred = predVect.end();

	for (predIter = predVect.begin(); predIter != endPred; predIter++) {
		(*predIter) / upg;
	}
}

vector<string> extract_children(const DrtPred &name)
{
	return name.extract_children();
}

DrtPred implant_children(DrtPred &name, const vector<string> &children)
{
	name.implant_children(children);
	return name;
}

string extract_subject(const DrtPred &name)
{
	return name.extract_child(1);
}

string extract_object(const DrtPred &name)
{
	return name.extract_child(2);
}

DrtPred implant_header(DrtPred &pred, const string &header)
{
	return pred.implant_header(header);
}

DrtPred add_header(DrtPred &pred, const string &str)
{
	return pred.implant_header(pred.extract_header() + str);
}

string extract_header(const DrtPred &pred)
{
	return pred.extract_header();
}

string extract_first_tag(const DrtPred &pred)
{
	return pred.extract_child(0);
}
string extract_second_tag(const DrtPred &pred)
{
	return pred.extract_child(1);
}

string extract_third_tag(const DrtPred &pred)
{
	return pred.extract_child(2);
}

DrtPred implant_first(DrtPred &pred, const string &str)
{
	return pred.implant_child(str, 0);
}

DrtPred implant_second(DrtPred &pred, const string &str)
{
	return pred.implant_child(str, 1);
}

DrtPred implant_subject(DrtPred &pred, const string &str)
{
	return pred.implant_child(str, 1);
}

DrtPred implant_subject_safe(DrtPred &pred, const string &name)
{
	string header = pred.extract_header();
	string sref = extract_subject(pred);
	string oref = extract_object(pred);
	if (name == sref || name == oref)
		return pred;
	bool is_passive = (header.find("PASSIVE") != string::npos);
	if (!is_passive)
		return implant_subject(pred, name);
	else
		return implant_object(pred, name);
}

DrtPred implant_object_safe(DrtPred &pred, const string &name)
{
	string header = pred.extract_header();
	string sref = extract_subject(pred);
	string oref = extract_object(pred);
	if (name == sref || name == oref)
		return pred;
	return implant_object(pred, name);
}

DrtPred implant_object(DrtPred &pred, const string &str)
{
	return pred.implant_child(str, 2);
}

DrtPred implant_subject_or_object(DrtPred &pred, const string &name)
{
	string sstr = extract_subject(pred);
	string ostr = extract_object(pred);

	if (!ref_is_name(sstr))
		return implant_subject(pred, name);
	else if (!ref_is_name(ostr))
		return implant_object(pred, name);
	return pred;
}

// DrtPred class
bool DrtPred::isQuantifier() const
{
	string head = header_;
	if (head.find("@QUANTIFIER") != string::npos)
		return true;
	return false;
}

bool DrtPred::is_parenthesis() const
{
	if (tag_.size() && (tag_ == "-LBR-" || tag_ == "-RBR-" || tag_ == "\""))
		return true;
	return false;
}

bool DrtPred::is_verb() const
{
	if (is_modal())
		return true;
	if (tag_.size()
			&& (tag_ == "AUX" || tag_ == "VBP" || tag_ == "VBZ" || tag_ == "VBD" || tag_ == "VB" || tag_ == "VBN"
					|| tag_ == "VBG" || tag_ == "V"))
		return true;
	return false;
}

bool DrtPred::is_plural() const
{
	if (tag_.size() && (tag_ == "NNS" || tag_ == "NNPS"))
		return true;
	if (tag_.size() && tag_ == "PRP" && (this->name() == "they" || this->name() == "them"))
		return true;
	return false;
}

bool DrtPred::is_POS() const
{
	if (tag_.size() && tag_ == "POS")
		return true;
	return false;
}

bool DrtPred::is_VBN() const
{
	if (is_modal())
		return true;
	if (tag_.size() && tag_ == "VBN")
		return true;
	return false;
}

bool DrtPred::is_conj() const
{
	if (tag_.size() && (tag_ == "CC" || tag_ == "-comma-"))
		return true;
	return false;
}

bool DrtPred::is_date() const
{
	string head = header_;
	if (head.size() && head.find("[date]") != string::npos)
		return true;
	return false;
}

bool DrtPred::is_verbatim() const
{
	string head = header_;
	if (head.size() && head.find("[verbatim]") != string::npos)
		return true;
	return false;
}

bool DrtPred::is_place() const
{
	string head = header_;
	if (head.size() && head.find("[place]") != string::npos)
		return true;
	return false;
}

bool DrtPred::is_generic() const
{
	string head = header_;
	if (head.size() && head.find("[*]") != string::npos)
		return true;
	return false;
}

bool DrtPred::is_sentence() const
{
	string head = header_;
	if (head.size() && head.find("[S]") != string::npos)
		return true;
	return false;
}

bool DrtPred::is_what() const
{
	string head = header_;
	if (head.size() && head.find("[what]") != string::npos)
		return true;
	return false;
}

bool DrtPred::is_complement() const
{
	string head = header_;
	if (head.size() && head.at(0) == '@')
		return true;
	return false;
}

bool DrtPred::is_delete() const
{
	string head = header_;
	if (head.size() && head.find(":DELETE") != string::npos)
		return true;
	return false;
}

bool DrtPred::is_name() const
{
	if (tag_.size()
			&& (tag_ == "NN" || tag_ == "NNS" || tag_ == "NNP" || tag_ == "NNPS" || tag_ == "PRP" || tag_ == "JJ" || tag_ == "!WP"
					|| tag_ == "JJS" || tag_ == "CD" || tag_ == "$" || tag_ == "UH"))
		return true;
	return false;
}

bool DrtPred::is_article() const
{
	if (tag_.size() && (tag_ == "DT" || tag_ == "PDT") )
		return true;
	return false;
}

bool DrtPred::is_number() const
{
	if (tag_.size() && tag_ == "CD")
		return true;
	return false;
}

bool DrtPred::is_proper_name() const
{
	if (tag_.size() && (tag_ == "NNP" || tag_ == "NNPS"))
		return true;
	return false;
}

bool DrtPred::is_PRP() const
{
	if (tag_.size() && tag_ == "PRP")
		return true;
	return false;
}

bool DrtPred::is_adverb() const
{
	if (tag_.size() && (tag_ == "RB" || tag_ == "RP" || tag_ == "RBR"))
		return true;
	return false;
}

bool DrtPred::is_comma() const
{
	if (tag_.size() && tag_ == "-comma-")
		return true;
	return false;
}

bool DrtPred::is_adjective() const
{
	if (tag_.size() && (tag_ == "JJ" || tag_ == "JJR" || tag_ == "JJS" //|| tag_== "CD"
	|| tag_ == "PRP$" || tag_ == "VBJ"))
		return true;
	return false;
}

bool DrtPred::is_preposition() const
{
	if (tag_.size() && (tag_ == "IN" || tag_ == "TO" || tag_ == "OF" || tag_ == "POS" || tag_ == "PREP" || tag_ == "\""))
		return true;
	return false;
}

bool DrtPred::is_WDT() const
{
	if (tag_.size() && tag_ == "WDT")
		return true;
	return false;
}

bool DrtPred::is_WP() const
{
	if (tag_.size() && (tag_ == "WP" || tag_ == "!WP"))
		return true;
	return false;
}

bool DrtPred::is_WP_pos() const
{
	if (tag_.size() && tag_ == "WP$")
		return true;
	return false;
}

bool DrtPred::is_WRB() const
{
	if (tag_.size() && tag_ == "WRB")
		return true;
	return false;
}

bool DrtPred::is_modal() const
{
	if (tag_.size() && tag_ == "MD")
		return true;
	return false;
}

DrtPred::DrtPred(const std::string &s, const double w) :
		weigth(w), is_question_(false), is_pivot_(false), header_(""), is_intersection_(false), is_anaphora_(false)
{
	int start, end;
	start = s.find("(");
	end = s.find(")");

	if ((start == string::npos && end != string::npos) || (start != string::npos && end == string::npos))
		throw std::runtime_error("The predicate " + s + " is not well formed!");

	string root_str = "";
	if (start != 0)
		root_str = s.substr(0, start);
	else
		root_str = s;
	if (root_str.find('#') != string::npos) {
		vector<string> strs;
		boost::split(strs, root_str, boost::is_any_of("#"));
		if (strs.size() >= 2) {
			root_str = strs.at(0);
			string options = strs.at(1);
			if (options.find("[pivot]") != string::npos)
				is_pivot_ = true;
			if (options.find("[anaphora]") != string::npos)
				is_anaphora_ = true;
		}
	}
	if (root_str.find('/') != string::npos) {
		vector<string> strs;
		boost::split(strs, root_str, boost::is_any_of("/"));
		if (strs.size() >= 2) {
			root_str = strs.at(0);
			tag_ = strs.at(1);
		}
	}
	name_ = root_str;
	header_ = root_str;

	if (start != end - 1) {
		string all_children = "";
		all_children = s.substr(start + 1, end - start - 1);
		vector<string> strs;
		boost::split(strs, all_children, boost::is_any_of(","));
		for (int n = 0; n < strs.size(); ++n) {
			children_.push_back(strs.at(n));
		}
	}
}

DrtPred::DrtPred(const PredTree &t, const double w) :
		weigth(w), is_question_(false), is_pivot_(false), header_(""), is_intersection_(false), is_anaphora_(false)
{
	Predicate p(t);
	std::stringstream ss;
	ss << p;
	*this = DrtPred(ss.str());
}

DrtPred::DrtPred(const Predicate &p, const double w) :
		weigth(w), is_question_(false), is_pivot_(false), header_(""), is_intersection_(false), is_anaphora_(false)
{
	std::stringstream ss;
	ss << p;
	*this = DrtPred(ss.str());
}

DrtPred::DrtPred(const DrtPred &rhs)
{
	header_ = rhs.header_;
	children_ = rhs.children_;

	name_ = rhs.name_;
	tag_ = rhs.tag_;
	anaphora_level_ = rhs.anaphora_level_;

	is_question_ = rhs.is_question_;
	question_word_ = rhs.question_word_;

	is_pivot_ = rhs.is_pivot_;
	is_intersection_ = rhs.is_intersection_;
	is_anaphora_ = rhs.is_anaphora_;

	weigth = rhs.weigth;
}

string DrtPred::extract_header() const
{
	return header_;
}

string DrtPred::extract_child(int n) const
{
	if (children_.size() == 0)
		return "";
	if (n >= children_.size())
		return children_.at(0);
	return children_.at(n);
}

vector<string> DrtPred::extract_children() const
{
	return children_;
}

DrtPred DrtPred::implant_header(const string &str)
{
	header_ = str;
	return *this;
}

DrtPred DrtPred::implant_child(const string &str, int n)
{
	if (n >= children_.size())
		n = 0;
	children_.at(n) = str;
	return *this;
}

DrtPred DrtPred::implant_children(const vector<string> &str)
{
	children_ = str;
	return *this;
}

bool points_to_subject(const vector<DrtPred> &pre_drt, const DrtPred &pred)
// Returns true if the predicate points to a subject. It is used to
// avoid a percolation to the verb in phrases like "Commissioner for
// EU met the chairman": EU is referred to Commissioner, not to
// "meet".
{
	string ftag = extract_first_tag(pred);
	vector<DrtPred>::const_iterator piter = pre_drt.begin();
	vector<DrtPred>::const_iterator pend = pre_drt.end();
	for (; piter != pend; ++piter) {
		if (piter->is_verb()) {
			string subj = extract_subject(*piter);
			if (subj == ftag)
				return true;
		}
	}
	return false;
}

bool points_to_object(const vector<DrtPred> &pre_drt, const DrtPred &pred)
// Returns true if the predicate points to an object.
{
	string ftag = extract_first_tag(pred);
	vector<DrtPred>::const_iterator piter = pre_drt.begin();
	vector<DrtPred>::const_iterator pend = pre_drt.end();
	for (; piter != pend; ++piter) {
		if (piter->is_verb()) {
			string subj = extract_object(*piter);
			if (subj == ftag)
				return true;
		}
	}
	return false;
}

bool ref_is_name(const string &ftag)
{
	if (ftag.find("name") != string::npos || ftag.find("ref") != string::npos)
		return true;
	return false;
}

bool ref_is_name_no_ref(const string &ftag)
{
	if (ftag.find("name") != string::npos)
		return true;
	return false;
}

bool ref_is_ref(const string &ftag)
{
	if (ftag.find("ref") != string::npos)
		return true;
	return false;
}

bool ref_is_verb(const string &ftag)
{
	if (ftag.find("verb") != string::npos)
		return true;
	return false;
}

DrtVect create_drtvect(const string &s)
{
	string str = s;
	boost::erase_all(str, string(" ")); // strip all the spaces from the string

	// saving the consequence
	int p, size = str.size();
	int depth = 0;
	int p1 = 0, p2;
	// the string
	vector<string> strs;
	for (p = 0; p < size; ++p) {
		if (str.at(p) == '(')
			++depth;
		if (str.at(p) == ')')
			--depth;
		if (depth < 0)
			throw(std::invalid_argument("Badly formed drs."));
		if (str.at(p) == ',' && depth == 0) {
			p2 = p;
			strs.push_back(str.substr(p1, p2 - p1));
			p1 = p2 + 1;
		}
	}
	strs.push_back(str.substr(p1, p - p1));

	vector<string>::iterator si = strs.begin();
	vector<string>::iterator se = strs.end();
	vector<DrtPred> to_return;
	to_return.resize(strs.size());
	vector<DrtPred>::iterator hi = to_return.begin();
	for (; si != se; ++si, ++hi) {
		*hi = DrtPred(*si);
	}

	return to_return;
}

vector<DrtVect> get_linked_drtvect_from_single_drtvect(const DrtVect &drtvect)
// It returns the set of drts that are connected to a specific
// verb. Conjunctions between verbs and @conditions are ignored.
{
	vector<DrtVect> to_return;

	vector<DrtPred>::const_iterator diter = drtvect.begin();
	vector<DrtPred>::const_iterator dend = drtvect.end();

	int n = 0;
	vector<DrtPred> previous_elements;
	for (; diter != dend; ++diter, ++n) {
		if (diter->is_verb() && find(previous_elements.begin(), previous_elements.end(), *diter) == previous_elements.end()) {
			vector<DrtPred> tmp_drtvect = find_all_attached_to_verb(drtvect, n);
			if (tmp_drtvect.size() == 1)  // not interested in lonely verbs
				continue;
			sort(tmp_drtvect.begin(), tmp_drtvect.end());
			tmp_drtvect.erase(std::unique(tmp_drtvect.begin(), tmp_drtvect.end()), tmp_drtvect.end());
			previous_elements.insert(previous_elements.end(), tmp_drtvect.begin(), tmp_drtvect.end());
			to_return.push_back(tmp_drtvect);
		}
	}

	if (debug) {
		std::cout << "CONNECTED:: " << std::endl;
		for (int m = 0; m < to_return.size(); ++m) {
			print_vector(to_return.at(m));
		}
	}

	return to_return;
}

int find_verb_with_object(vector<DrtPred> &pre_drt, string from_str)
{
	for (int n = 0; n < pre_drt.size(); ++n) {
		if (pre_drt.at(n).is_verb()) {
			string obj_str = extract_object(pre_drt.at(n));
			if (from_str == obj_str)
				return n;
		}
	}
	return -1;
}

int find_verb_with_subject(vector<DrtPred> &pre_drt, string from_str)
{
	for (int n = 0; n < pre_drt.size(); ++n) {
		if (pre_drt.at(n).is_verb()) {
			string obj_str = extract_subject(pre_drt.at(n));
			if (from_str == obj_str)
				return n;
		}
	}
	return -1;
}
