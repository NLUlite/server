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



#include"Match.hpp"

const bool debug = false;

template<class T>
static bool shortfind(const vector<T> &vect, const T &element)
{
	if (find(vect.begin(), vect.end(), element) == vect.end())
		return false;
	return true;
}

template<class T>
static bool shortfind(const vector<T> &vect, const vector<T> &elements)
{
	for (int n = 0; n < elements.size(); ++n) {
		T element = elements.at(n);
		if (find(vect.begin(), vect.end(), element) == vect.end())
			return false;
	}
	return true;
}

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

template<class T>
static void print_vector(std::set<T> &vs)
{
	if (vs.size()) {
		typename set<T>::iterator tags_iter = vs.begin();
		while (tags_iter != vs.end()) {
			std::cout << (*tags_iter) << " ";
			++tags_iter;
		}
		std::cout << std::endl;
	}
}


template<class T>
static void print_vector(std::vector<T> &vs, std::ostream &out)
{
	if (vs.size()) {
		typename vector<T>::iterator tags_iter = vs.begin();
		while (tags_iter != vs.end()) {
			out << (*tags_iter) << " ";
			++tags_iter;
		}
		out << std::endl;
	}
}

static bool has_object(const DrtPred &pred)
{
	string obj = extract_object(pred);
	if (obj.find("ref") != string::npos || obj.find("name") != string::npos)
		return true;
	return false;
}

static DrtVect unify_elements(DrtVect to_return, DrtMgu mgu)
{
	for(int n=0; n < to_return.size(); ++n) {
		if(!to_return.at(n).is_conj() && to_return.at(n).is_complement())
			continue;
		to_return.at(n)/mgu;
	}
	return to_return;
}


static bool is_transitive(const DrtPred &pred)
{
	string str = extract_header(pred);

	vector<string> candidates; // intransitive verbs
	candidates.push_back("smile");
	candidates.push_back("sleep");
	candidates.push_back("sit");
	candidates.push_back("compete");
	candidates.push_back("arrive");
	candidates.push_back("pay_attention");
	candidates.push_back("tumble");
	candidates.push_back("occur");
	candidates.push_back("correspond");
	candidates.push_back("give_way");
	candidates.push_back("belong");
	candidates.push_back("die");
	candidates.push_back("lie");
	candidates.push_back("think");
	candidates.push_back("travel");
	candidates.push_back("tell");
	candidates.push_back("exist");

	if (find(candidates.begin(), candidates.end(), str) != candidates.end())
		return false;
	return true;
}

static bool is_broken(const DrtPred &pred)
{
	string head = extract_header(pred);

	if (head.size() && head.at(0) == '@') {
		string ftag = extract_first_tag(pred);
		if (ftag.find("broken") != string::npos)
			return true;
	}
	return false;
}

static bool is_broken(const string &str)
{
	if (str.find("broken") != string::npos)
		return true;
	return false;
}

string get_what()
// list of meanings for "what/WP"
{
	return "!person|mission|plant|snake|family|car|place|politician|chancellor|time|mountain|support|moon|planet|name|life|animal|hero|country|player|league|team|profession|playmaker|goalscorer|officer|medal|price|thing|love|event|happening|material|metal|road|place|company|junction|terminus|vehicle|[NNP]|thing";
}


bool MatchElements::hasElement(const string &pred) const
{
	if(debug) {
		cout << "HAS_ELEMENT:::" << pred << " ";
		set<string> to_print(elements_);
		print_vector(to_print);
	}

	set<string>::const_iterator miter= elements_.find(pred);
	if(miter != elements_.end() )
		return true;
	return false;
}

void MatchElements::insertPrevious(const vector<string> &preds)
{
	//elements_.insert(elements_.end(),preds.begin(),preds.end() );
	elements_.insert(preds.begin(),preds.end() );
}

void MatchElements::insertPrevious(const string &pred)
{
	//elements_.insert(elements_.end(),preds.begin(),preds.end() );
	elements_.insert(pred);
}


void MatchElements::clear()
{
	elements_.clear();
}


class Date {
	string date_str_;

	int day_;
	int month_;
	int year_;

public:
	Date(const string &str);
	bool operator<(const Date &rhs);
};

bool Date::operator<(const Date &rhs)
{
	if (year_ < rhs.year_)
		return true;
	if (year_ > rhs.year_)
		return false;
	if (month_ < rhs.month_)
		return true;
	if (month_ > rhs.month_)
		return false;
	if (day_ < rhs.day_)
		return true;
	if (day_ > rhs.day_)
		return false;
	return false;
}

Date::Date(const string &str)
{
	day_ = 0;
	month_ = 0;
	year_ = 0;

	vector<string> months;
	months.push_back("january");
	months.push_back("february");
	months.push_back("march");
	months.push_back("april");
	months.push_back("may");
	months.push_back("june");
	months.push_back("july");
	months.push_back("august");
	months.push_back("september");
	months.push_back("october");
	months.push_back("november");
	months.push_back("december");

	string datestr = "[date]_";
	int datepos = str.find(datestr);
	date_str_ = str.substr(datepos + datestr.size(), str.size());

	vector<string> strings;
	boost::split(strings, date_str_, boost::is_any_of("_"));

	if (strings.size() == 0)
		return;

	if (strings.size() == 1) {
		vector<string>::iterator miter = find(months.begin(), months.end(), strings.at(0));
		if (miter == months.end()) {
			year_ = boost::lexical_cast<int>(strings.at(0));
		} else {
			month_ = std::distance(months.begin(), miter);
		}
	}

	if (strings.size() == 2) {
		vector<string>::iterator miter = find(months.begin(), months.end(), strings.at(0));
		if (miter != months.end()) {
			month_ = std::distance(months.begin(), miter);
			year_ = boost::lexical_cast<int>(strings.at(1));
		} else {
			miter = find(months.begin(), months.end(), strings.at(1));
			if (miter == months.end())
				return;
			month_ = std::distance(months.begin(), miter);
			day_ = boost::lexical_cast<int>(strings.at(0));
		}
	}

	if (strings.size() == 3) {
		vector<string>::iterator miter = find(months.begin(), months.end(), strings.at(1));
		if (miter == months.end())
			return;
		day_   = boost::lexical_cast<int>(strings.at(0));
		month_ = std::distance(months.begin(), miter);
		year_  = boost::lexical_cast<int>(strings.at(2));
	}
}

MatchType::MatchType()
{
	is_equal_= false; is_more_= false; is_less_= false;
	is_time_= false; is_space_= false;
}

MatchType::~MatchType()
{
	;
}


bool MatchType::matchDate(const DrtPred &lhs, const DrtPred &rhs)
{
	string lhead = extract_header(lhs);
	string rhead = extract_header(rhs);

	if (debug) {
		cout << "MML:: " << lhead << endl;
		cout << "MMR:: " << rhead << endl;
	}

	try {

		Date ldate(lhead), rdate(rhead);

		if (debug) {
			cout << this->is_more() << " " << this->is_less() << " " << (ldate < rdate) << " " << (rdate < ldate) << endl;
		}

		if (this->is_more() && ldate < rdate)
			return true;
		if (this->is_less() && rdate < ldate)
			return true;

		if (lhead == rhead)
			return true;

	} catch (std::exception &e) {
		if (debug)
			puts("etc:::");
	}
	return false;
}

bool MatchType::matchNumber(const DrtPred &lhs, const DrtPred &rhs)
{
	string lhead = extract_header(lhs);
	string rhead = extract_header(rhs);

	try {

		double lnumber = boost::lexical_cast<double>(lhead);
		double rnumber = boost::lexical_cast<double>(rhead);
		if (this->is_more() && lnumber < rnumber)
			return true;
		if (this->is_less() && rnumber < lnumber)
			return true;

		if (lhead == rhead)
			return true;

	} catch (std::exception &e) {
		if (debug)
			puts("etc2:::");
	}
	return false;
}

bool MatchType::matchComplement(const DrtPred &lhs, const DrtPred &rhs)
// <left|right>
{
	string lhead = extract_header(lhs);
	string rhead = extract_header(rhs);

	if (lhead == "@TIME_AT" && rhead == "@AFTER") {
		this->set_more();
		this->set_time();
		return true;
	}
	if (lhead == "@TIME_AT" && rhead == "@BEFORE") {
		this->set_less();
		this->set_time();
		return true;
	}
	if (rhead == "@TIME_AT" && lhead == "@AFTER") {
		this->set_more();
		this->set_time();
		return true;
	}
	if (rhead == "@TIME_AT" && lhead == "@BEFORE") {
		this->set_less();
		this->set_time();
		return true;

	}
	if (rhead == "@MORE_THAN" && lhead == "@MORE_THAN") {
		this->set_more();
		return true;
	}
	if (rhead == "@LESS_THAN" && lhead == "@LESS_THAN") {
		this->set_less();
		return true;
	}
	if (rhead == "@QUANTITY" && lhead == "@MORE_THAN") {
		this->set_more();
		return true;
	}
	if (rhead == "@QUANTITY" && lhead == "@LESS_THAN") {
		this->set_less();
		return true;
	}
	if (lhead == rhead) {
		if (rhead == "@TIME_AT" || rhead == "@AFTER" || rhead == "@BEFORE")
			this->set_time();
		return true;
	}

	return false;
}

MatchSubstitutions::MatchSubstitutions()
: num_matches_(0)
{
	; //
}

MatchSubstitutions::~MatchSubstitutions()
{
	; //
}

string MatchSubstitutions::findHeader(const string &str)
{
	for (int n = 0; n < substitutions_.size(); ++n) {
		if (substitutions_.at(n).first == str)
			return substitutions_.at(n).second;
	}
	return "";
}

void MatchSubstitutions::applySubstitutions(DrtVect *d)
{
	DrtVect::iterator diter = d->begin(), dend = d->end();
	for (; diter != dend; ++diter) {
		string header = extract_header(*diter);
		string new_header = this->findHeader(header);
		if (new_header != "") {
			implant_header(*diter, new_header);
			diter->setName(new_header);
		}
	}
}


void MatchSubstitutions::addSubstitution(const string &first, const string &second)
{
	substitutions_.push_back(make_pair(first, second));
}

void MatchSubstitutions::addSubstitution(const vector<pair<string, string> > &subs)
{
	substitutions_.insert(substitutions_.end(), subs.begin(), subs.end());
}

vector<DrtVect> SingleMatchGraph::getComplements(DrtMgu upg) const
// the complements that are broken and already assigned as a specifications are not returned
{
	vector<DrtVect> to_return;

	for (int n = 0; n < complements_.size(); ++n) {
		DrtPred tmp_compl;
		if (complements_.at(n).size())
			tmp_compl = complements_.at(n).at(0);
		if (is_broken(tmp_compl)) {
			tmp_compl / upg;
			if (is_broken(tmp_compl))
				to_return.push_back(complements_.at(n));
		} else
			to_return.push_back(complements_.at(n));
	}

	return to_return;
	return complements_;
}

void SingleMatchGraph::operator/(const DrtMgu &upg)
{
	subject_ / upg;
	object_ / upg;
	verb_ / upg;
	adverb_ / upg;
	complements_ / upg;
}
void SingleMatchGraph::print(std::ostream &out)
{
	out << endl;

	if (subject_.size()) {
		out << "subject: ";
		print_vector(subject_, out);
		if (has_verb_) {
			out << "verb: " << verb_ << endl;
		}
	}

	if (object_.size()) {
		out << "object: ";
		print_vector(object_, out);
	}

	if (complements_.size()) {
		for (int n = 0; n < complements_.size(); ++n) {
			out << "complement: ";
			print_vector(complements_.at(n), out);
		}
	}

	out << endl;
}

DrtVect SingleMatchGraph::getDrs() const
{
	DrtVect to_return;

	if (subject_.size()) {
		to_return.insert(to_return.end(), subject_.begin(), subject_.end());
	}
	if (has_verb_) {
		to_return.push_back(verb_);
	}
	if (object_.size()) {
		to_return.insert(to_return.end(), object_.begin(), object_.end());
	}
	if (complements_.size()) {
		for (int n = 0; n < complements_.size(); ++n) {
			to_return.insert(to_return.end(), complements_.at(n).begin(), complements_.at(n).end());
		}
	}
	return to_return;
}

void MatchGraph::print(std::ostream &out)
{
	out << endl;

	vector<SingleMatchGraph>::iterator siter = sentences_.begin();
	vector<SingleMatchGraph>::iterator send = sentences_.end();

	for (; siter != send; ++siter) {
		siter->print(out);
	}

	if (specifications_.size()) {
		for (int n = 0; n < specifications_.size(); ++n) {
			out << "specification: ";
			print_vector(specifications_.at(n), out);
		}
	}

	out << "subs: ";
	print_vector(subs_, out);

	out << endl;
}

static vector<DrtPred> get_specification_from_pred(const vector<DrtPred> &preds, const DrtPred &pred)
{
	vector<DrtPred> to_return = get_elements_next_of(preds, pred);
	if(debug) {
		cout << "NEXTOF:: ";
		print_vector(to_return);
	}

	return to_return;
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

static bool is_conjunction_spec(const DrtVect &drtvect)
{
	DrtPred first = drtvect.at(0);
	string head = extract_header(first);
	if (head == "@AND" || head == "@OR")
		return true;
	return false;
}

static bool points_to_verb(const DrtPred &pred)
{
	string fref = extract_first_tag(pred);
	string head = extract_header(pred);
	if (fref.find("verb") != string::npos)
		return true;
	return false;
}

static bool second_tag_is_verb(const DrtPred &pred)
{
	string sref = extract_second_tag(pred);
	string head = extract_header(pred);
	if (sref.find("verb") != string::npos)
		return true;
	return false;
}


static pair<string, string> extract_conjunction_ref(const DrtVect &drtvect)
{
	DrtPred first = drtvect.at(0);
	string fref = extract_first_tag(first);
	string sref = extract_second_tag(first);

	return make_pair(fref, sref);
}

static pair<DrtMgu, DrtMgu> generate_upgs_from_conjunction_ref(const pair<string, string> &cref)
// the original conjunction is @AND(A,B). this creates an upg that contains A->A|B, B->B|A, and the roll-back DrtMgu
{
	string f1, s1, f2, s2;

	DrtMgu upg_forward, upg_backward;

	f1 = cref.first;
	s1 = cref.first + "|" + cref.second;
	upg_forward.add(f1, s1);
	upg_backward.add(s1, f1);
	f2 = cref.second;
	s2 = cref.second + "|" + cref.first;
	upg_forward.add(f2, s2);
	upg_backward.add(s2, f2);

	return make_pair(upg_forward, upg_backward);
}

void MatchGraph::computeDrtMguForward()
// applies the DrtMgu for dealing with conjunctions
{
	vector<DrtMgu>::iterator uiter, uend;
	uiter = upg_forward_.begin();
	uend = upg_forward_.end();

	for (; uiter != uend; ++uiter) {
		specifications_ / *uiter;
		complements_ / *uiter;
		vector<SingleMatchGraph> sentences = this->getSentences();
		for (int n = 0; n < sentences.size(); ++n) {
			sentences.at(n) / (*uiter);
		}
		this->setSentences(sentences);
	}
}

void MatchGraph::computeDrtMguBackward()
// applies the DrtMgu for dealing with conjunctions
{
	vector<DrtMgu>::iterator uiter, uend;
	uiter = upg_backward_.begin();
	uend = upg_backward_.end();

	for (; uiter != uend; ++uiter) {
		specifications_ / *uiter;
		complements_ / *uiter;
		vector<SingleMatchGraph> sentences = this->getSentences();
		for (int n = 0; n < sentences.size(); ++n) {
			sentences.at(n) / (*uiter);
		}
		this->setSentences(sentences);
	}
}

static vector<DrtPred> find_subject_of_verb_with_conjunctions(const vector<DrtPred> &preds, int m)
{
	vector<DrtPred> cons;

	if (!preds.at(m).is_verb())  // check the verb is a verb
		return cons;
	DrtPred verb_cons = preds.at(m);
	string obj = extract_subject(verb_cons);
	vector<DrtPred> connected_subj;

	connected_subj = get_predicates_from_position(preds, find_all_element_with_string(preds, obj));
	for (int n = 0; n < connected_subj.size(); ++n) {
		string head = extract_header(connected_subj.at(n));
		string child_ref = extract_first_tag(connected_subj.at(n));
		if (connected_subj.at(n).is_name() || (connected_subj.at(n).is_article() && child_ref.find("name") != string::npos)) // it is a DT that has been associated with another name ("this" or "that")
			cons.push_back(connected_subj.at(n));
		else if (connected_subj.at(n).is_conj() && (head == "@AND" || head == "@OR")) {
			string sref = extract_second_tag(connected_subj.at(n));
			vector<DrtPred> connected_conj = get_predicates_from_position(preds, find_all_element_with_string(preds, sref));
			for (int m = 0; m < connected_conj.size(); ++m) {
				if (!shortfind(connected_subj, connected_conj.at(m)))
					connected_subj.push_back(connected_conj.at(m));
			}
		}
	}

	return cons;
}

static vector<DrtPred> find_object_of_verb_with_conjunctions(const vector<DrtPred> &preds, int m)
{
	vector<DrtPred> cons;

	if (!preds.at(m).is_verb())  // check the verb is a verb
		return cons;
	DrtPred verb_cons = preds.at(m);
	string obj = extract_object(verb_cons);
	vector<DrtPred> connected_subj;

	connected_subj = get_predicates_from_position(preds, find_all_element_with_string(preds, obj));
	for (int n = 0; n < connected_subj.size(); ++n) {
		string head = extract_header(connected_subj.at(n));
		string child_ref = extract_first_tag(connected_subj.at(n));
		if (connected_subj.at(n).is_name() || (connected_subj.at(n).is_article() && child_ref.find("name") != string::npos)) // it is a DT that has been associated with another name ("this" or "that")
			cons.push_back(connected_subj.at(n));
		else if (connected_subj.at(n).is_conj() && (head == "@AND" || head == "@OR")) {
			string sref = extract_second_tag(connected_subj.at(n));
			vector<DrtPred> connected_conj = get_predicates_from_position(preds, find_all_element_with_string(preds, sref));
			for (int m = 0; m < connected_conj.size(); ++m) {
				if (!shortfind(connected_subj, connected_conj.at(m)))
					connected_subj.push_back(connected_conj.at(m));
			}
		}
	}

	return cons;
}

static vector<vector<DrtPred> > find_complements_of_verb_with_conjunctions(const vector<DrtPred> &preds, int m)
{
	vector<vector<DrtPred> > cons;

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
		if (candidate.at(0) == '@' && candidate.find("@SUB") == string::npos && !ref_is_verb(sref)) { // it is a complement
			string to_ref = extract_second_tag(connected_compl.at(n));
			DrtVect connected_names_tmp = get_predicates_from_position(preds, find_all_element_with_string(preds, to_ref));
			DrtVect connected_names;
			for (int m = 0; m < connected_names_tmp.size(); ++m) { // a complement can have a dangling complement at the end
				string head = extract_header(connected_names_tmp.at(m));
				string child_ref = extract_first_tag(connected_names_tmp.at(m));
				if (connected_names_tmp.at(m).is_name()
						|| (connected_names_tmp.at(m).is_article() && child_ref.find("name") != string::npos)) // it is a DT that has been associated with another name ("this" or "that")
					connected_names.push_back(connected_names_tmp.at(m));
				else if (connected_names_tmp.at(m).is_conj() && (head == "@AND" || head == "@OR")) {
					string sref2 = extract_second_tag(connected_names_tmp.at(m));
					vector<DrtPred> connected_conj;
					connected_conj = get_predicates_from_position(preds, find_all_joined_element_with_string(preds, sref2));
					for (int m2 = 0; m2 < connected_conj.size(); ++m2) {
						string head2 = extract_header(connected_conj.at(m2));
						if (head2 == "@AND" || head2 == "@OR")
							continue;
						if (!shortfind(connected_names, connected_conj.at(m2)))
							connected_names.push_back(connected_conj.at(m2));
					}
				}
			}
			connected_names.insert(connected_names.begin(), connected_compl.at(n));
			cons.push_back(connected_names);
		}
	}

	return cons;
}

DrtMgu MatchGraph::getDrtMguBackward()
{
	DrtMgu to_return;

	if (upg_backward_.size() == 0)
		return to_return;

	to_return = upg_backward_.at(0);
	for (int n = 1; n < upg_backward_.size(); ++n) {
		to_return.add(upg_backward_.at(n));
	}

	return to_return;
}

static bool pivot_compare(const DrtPred &lhs, const DrtPred &rhs)
{
	if (lhs.is_pivot() && !rhs.is_pivot())
		return true;
	if (!lhs.is_pivot() && rhs.is_pivot())
		return false;

	return false;
}

static bool string_compare(const DrtPred &lhs, const DrtPred &rhs)
{
	if (extract_header(lhs) < extract_header(rhs) )
		return true;
	return false;
}


MatchGraph::MatchGraph(const vector<DrtPred> &drs) :
		drs_form_(drs)
{
	std::sort(drs_form_.begin(), drs_form_.end(), pivot_compare);
	//std::sort(drs_form_.begin(), drs_form_.end(), string_compare);

	vector<DrtPred>::iterator diter = drs_form_.begin();
	vector<DrtPred>::iterator dend = drs_form_.end();

	int m = 0;
	bool verb_trigger = true; // process only the first verb;

	vector<DrtPred> candidate_names;
	vector<DrtVect> temp_complements;

	vector<string> subs_str;
	subs_str.push_back("@SUB-OBJ");
	subs_str.push_back("@SUBORD");

	for (; diter != dend; ++diter, ++m) {
		// extract the specifications
		string fref, sref;
		if (diter->is_complement()) {
			if (diter->isQuantifier()) {
				this->addQuantifier(*diter);
			} else {
				fref = extract_first_tag(*diter);
				sref = extract_second_tag(*diter);
			}
		}
		if (diter->is_name()) {
			candidate_names.push_back(*diter);
		}
		if (diter->is_article()) {
			string child_ref = extract_first_tag(*diter);
			if (child_ref.find("name") != string::npos) // it is a DT that has been associated with another name ("this" or "that")
				candidate_names.push_back(*diter);
		}
		if (( has_specification(*diter)
			 && !diter->isQuantifier() )
		//|| is_broken(*diter) // this is necessary to capture the broken complements
		) {
			string ref = extract_first_tag(*diter);
			int pos_name = find_element_with_string(drs_form_, ref);
			string name = drs_form_.at(pos_name).name();
			DrtVect drtvect = get_specification_from_pred(drs_form_, *diter); /// It only takes one specification!!!
			if (is_conjunction_spec(drtvect)) {
				pair<string, string> cref = extract_conjunction_ref(drtvect);
				pair<DrtMgu, DrtMgu> upg_pair = generate_upgs_from_conjunction_ref(cref);

				upg_forward_.push_back(upg_pair.first);
				upg_backward_.push_back(upg_pair.second);

				drs_form_ / upg_pair.first;
				//drs_form_ = unify_elements(drs_form_, upg_pair.first);
			}
			specifications_.push_back(drtvect);
		}
		if (//find(subs_str.begin(), subs_str.end(), extract_header(*diter)) != subs_str.end()
				//||
				(diter->is_complement() && ref_is_verb(fref) && ref_is_verb(sref) )
		) {
			subs_.push_back(*diter);
		}
		if ( //points_to_verb(*diter) &&
				is_broken(*diter)
		// this is necessary to capture the broken complements
		) {
			string ref = extract_first_tag(*diter);
			int pos_name = find_element_with_string(drs_form_, ref);
			string name = drs_form_.at(pos_name).name();
			DrtVect drtvect = get_specification_from_pred(drs_form_, *diter); /// It only takes one specification!!!
			temp_complements.push_back(drtvect);
		}
		// extract information from verbs: subject, object, complements
		if (diter->is_verb()) {
			SingleMatchGraph smg;
			smg.setSubject(find_subject_of_verb_with_conjunctions(drs_form_, m));
			smg.setObject(find_object_of_verb_with_conjunctions(drs_form_, m));
			smg.setVerb(*diter);
			smg.setVerb(true);
			smg.setAdverb(find_adverb_of_verb(drs_form_, m));
			smg.setComplements(find_complements_of_verb_with_conjunctions(drs_form_, m));

			vector<DrtVect> vcompl = find_complements_of_verb_with_conjunctions(drs_form_, m);

			if (diter->is_generic())
				smg.setGeneric();

			verb_list_.push_back(*diter);
			sentences_.push_back(smg);

			verb_trigger = false;
			has_verb_ = true;
		}
	}
	if (verb_trigger) {
		// there was no verb.
		/// This is never used as a match, since names are compared in graphMatchNames
		string first_ref = "";
		vector<DrtPred> subjects;
		for (int n = 0; n < candidate_names.size(); ++n) {
			string ref_tmp = extract_first_tag(candidate_names.at(n));
			if (first_ref == "") {
				first_ref = ref_tmp;
				subjects.push_back(candidate_names.at(n));
			} else if (ref_tmp == first_ref) {
				subjects.push_back(candidate_names.at(n));
			}
		}
		SingleMatchGraph smg;
		smg.setSubject(subjects);
		smg.setObject(subjects);
		smg.setVerb(false);
		sentences_.push_back(smg);
		has_verb_ = false;
	}
	if (sentences_.size() && temp_complements.size()) {
		sentences_.front().addComplements(temp_complements);
	}
}

MatchWrite::MatchWrite(const vector<DrtPred> &drs) :
		drs_form_(drs)
{
	vector<DrtPred>::iterator diter = drs_form_.begin();
	vector<DrtPred>::iterator dend = drs_form_.end();

	int m = 0;
	bool verb_trigger = true; // process only the first verb;

	vector<DrtPred> candidate_names;

	vector<string> subs_str;
	subs_str.push_back("@SUB-OBJ");
	subs_str.push_back("@SUBORD");
	//subs_str.push_back("@SUB-ALLOCUTION");

	for (; diter != dend; ++diter, ++m) {
		// extract the specifications
		//string head= diter->pred().begin()->str;
		string fref, sref;
		if (diter->is_complement()) {
			fref = extract_first_tag(*diter);
			sref = extract_second_tag(*diter);
		}
		if (diter->is_name()) {
			candidate_names.push_back(*diter);
		}
		if (diter->is_article()) {
			string child_ref = extract_first_tag(*diter);
			if (child_ref.find("name") != string::npos) // it is a DT that has been associated with another name ("this" or "that")
				candidate_names.push_back(*diter);
		}
		if (has_specification(*diter)) {
			string ref = extract_first_tag(*diter);
			int pos_name = find_element_with_string(drs_form_, ref);
			string name = drs_form_.at(pos_name).name();
			DrtVect drtvect = get_specification_from_pred(drs_form_, *diter); /// It only takes one specification!!!
			specifications_.push_back(drtvect);
		}
		if ( //find(subs_str.begin(), subs_str.end(), extract_header(*diter)) != subs_str.end()
				//||
				(diter->is_complement() && ref_is_verb(fref) && ref_is_verb(sref))) {
			subs_.push_back(*diter);
		}
		// extract information from verbs: subject, object, complements
		if (diter->is_verb()) {
			SingleMatchGraph smg;
			smg.setSubject(find_subject_of_verb(drs_form_, m));
			smg.setObject(find_object_of_verb(drs_form_, m));
			smg.setVerb(*diter);
			smg.setVerb(true);
			smg.setAdverb(find_adverb_of_verb(drs_form_, m));
			smg.setComplements(find_complements_of_verb(drs_form_, m));

			if (diter->is_generic())
				smg.setGeneric();

			verb_list_.push_back(*diter);
			sentences_.push_back(smg);

			verb_trigger = false;
			has_verb_ = true;
		}
	}
	if (verb_trigger) {
		// there was no verb.
		/// This is never used as a match, since names are compared in graphMatchNames
		string first_ref = "";
		//puts("CANDIDATES::::");
		//print_vector(candidate_names);
		vector<DrtPred> subjects;
		for (int n = 0; n < candidate_names.size(); ++n) {
			string ref_tmp = extract_first_tag(candidate_names.at(n));
			if (first_ref == "") {
				first_ref = ref_tmp;
				subjects.push_back(candidate_names.at(n));
			} else if (ref_tmp == first_ref) {
				subjects.push_back(candidate_names.at(n));
			}
		}
		SingleMatchGraph smg;
		smg.setSubject(subjects);
		smg.setObject(subjects);
		smg.setVerb(false);
		sentences_.push_back(smg);
		//puts("SUBJ::::");
		//print_vector(subjects);
		//object_= subject_;  // If there is no verb the NP is stored both in subject_ and object_
		has_verb_ = false;
	}
}

vector<vector<DrtPred> > MatchWrite::getSpecifications(const DrtPred &pred) const
{
	vector<vector<DrtPred> > to_return;

	string ref = extract_first_tag(pred);

	vector<vector<DrtPred> >::const_iterator siter = specifications_.begin();
	vector<vector<DrtPred> >::const_iterator send = specifications_.end();
	for (; siter != send; ++siter) {
		string spec_ref = extract_first_tag(siter->at(0));
		if (spec_ref == ref) {
			to_return.push_back(*siter);
		}
	}

	return to_return;
}

static vector<vector<DrtPred> > filter_valid_specifications(const vector<vector<DrtPred> > &specs)
{
	vector<vector<DrtPred> > to_return;

	vector<vector<DrtPred> >::const_iterator diter = specs.begin();
	vector<vector<DrtPred> >::const_iterator dend = specs.end();

	for (; diter != dend; ++diter) {
		string head = extract_header(diter->at(0));
		if (head != "@OWN")
			to_return.push_back(*diter);
	}
	return to_return;
}

static vector<DrtVect> get_additional_spec(const vector<DrtPred> &names, Knowledge *k)
{
	vector<DrtVect> to_return;

	for (int n = 0; n < names.size(); ++n) {
		string ref = extract_first_tag(names.at(n));
		vector<DrtVect> tmp = filter_valid_specifications(k->getSpecificationsFromRef(ref));
		to_return.insert(to_return.end(), tmp.begin(), tmp.end());
	}

	return to_return;
}

void MatchGraph::additionalSpec(Knowledge *k)
{
	// add additional specification to the specifications_ list.
	// For example, if cat(name1) has specification @PLACE_AT(name1,name2) floor(name2)
	// in the Knowledge but not in this phrase, the specification is added here.
	vector<DrtVect> additional_specs_subj = get_additional_spec(subject_, k);
	vector<DrtVect> additional_specs_obj = get_additional_spec(object_, k);
	specifications_.insert(specifications_.end(), additional_specs_subj.begin(), additional_specs_subj.end());
	specifications_.insert(specifications_.end(), additional_specs_obj.begin(), additional_specs_obj.end());
}

vector<vector<DrtPred> > MatchGraph::getSpecifications(const DrtPred &pred) const
{
	vector<vector<DrtPred> > to_return;

	string ref = extract_first_tag(pred);

	vector<vector<DrtPred> >::const_iterator siter = specifications_.begin();
	vector<vector<DrtPred> >::const_iterator send = specifications_.end();
	for (; siter != send; ++siter) {
		string spec_ref = extract_first_tag(siter->at(0));
		if (spec_ref == ref) {
			to_return.push_back(*siter);
		}
	}

	return to_return;
}

static bool contained_NNP(const string &lhs, const string &rhs)
{
	vector<string> lhs_strs;
	boost::split(lhs_strs, lhs, boost::is_any_of("_"));
	vector<string> rhs_strs;
	boost::split(rhs_strs, rhs, boost::is_any_of("_"));

	for (int n = 0; n < lhs_strs.size(); ++n) {
		if (find(rhs_strs.begin(), rhs_strs.end(), lhs_strs.at(n)) == rhs_strs.end())
			return false;
	}
	return true;
}

static inline string get_date(const vector<DrtPred> &drtvect)
{
	string date = "";

	for (int n = 0; n < drtvect.size(); ++n) {
		string head = extract_header(drtvect.at(n));
		if (head == "@TIME_AT") {
			for (int m = n; m < drtvect.size(); ++m) {
				if (drtvect.at(m).is_date()) {
					date = extract_header(drtvect.at(m));
					return date;
				}
			}
		}
	}

	return date;
}

static string retrieve_date(Knowledge *k, const DrtPred &pred)
{
	string head = extract_header(pred);
	string date = "";

	vector<string> refs = k->getRefFromName(head);
	vector<string>::iterator riter = refs.begin(), rend = refs.end();
	for (; riter != rend; ++riter) {
		vector<vector<DrtPred> > specifications = k->getSpecificationsFromRef(*riter);
		for (int n = 0; n < specifications.size(); ++n) {
			date = get_date(specifications.at(n));
			if (date != "")
				return date;
		}
	}
	return date;
}

Match::Match(Knowledge *k)
{
	hyp_height_= 6; // standard height of the search for hypernyms
	d_ = metric_singleton::get_metric_instance();
	k_ = k;
	inverted_person_= false;
}

static inline double max(double a, double b)
{
	return a > b ? a : b;
}

static inline double min(double a, double b)
{
	return a < b ? a : b;
}

static bool is_forbidden_synonym_verb(const string &orig_str, const string name)
{
	vector<string> to_return;
	to_return.push_back(orig_str);
	bool orig_is_a_who = matching::is_who(orig_str);

	if(debug) {
		cout << "FORB::: " << orig_str << " " << name << endl;
	}

	if( orig_str == "live"
		&& (name == "travel" || name == "move" || name == "go")
	)
		return true;

	if( orig_str == "die"
		&& (name == "go")
	)
		return true;
	if( orig_str == "take"
			&& (name == "shoot")
	)
		return true;
	if( orig_str == "shoot"
			&& (name == "take")
	)
		return true;

	return false;
}


double get_string_vector_distance(metric *d, const vector<string> &head_strs, const vector<string> &ref_strs, Knowledge *k, int max_sep, string *return_name, string *rreturn_name)
{
	double distance = 0;
	vector<string>::const_iterator riter = ref_strs.begin();
	vector<string>::const_iterator rend = ref_strs.end();
	vector<string>::const_iterator liter = head_strs.begin();
	vector<string>::const_iterator lend = head_strs.end();

	bool invert = false;

	for (; liter != lend; ++liter) {
		for (riter = ref_strs.begin(); riter != rend; ++riter) {
			if (debug)
				cout << "MATCH30::: " << *liter << ", " << ", " << *riter << endl;
			if(return_name != 0)
				*return_name = *liter;
			if(rreturn_name != 0)
				*rreturn_name = *riter;
			if (*liter == "[*]" || *riter == "[*]" || *liter == "[any]")
				return 1;
			if (*liter == *riter)
				return 1 + 1; // same words must be considered more relevant
			if (riter->find("[verb") != string::npos) {
				// the rhs is a levin => find the levin of the lhs and return true or false
				string lhs_levin = d->get_levin_verb(*liter);
				lhs_levin = string("[") + lhs_levin + "]";
				if (lhs_levin == *riter)
					return 1;
			}
			if (riter->find("[noun") != string::npos) {
				// the rhs is a levin => find the levin of the lhs and return true or false
				string lhs_levin = d->get_levin_noun(*liter);
				lhs_levin = string("[") + lhs_levin + "]";
				if (lhs_levin == *riter)
					return 1;
			}

			string lstr = *liter, rstr = *riter;

			// The symbol "=" in front of a word requires exact match
			if (rstr.size() && lstr.size() && rstr.at(0) == '=' && lstr.at(0) != '=') {
				rstr = rstr.substr(1, rstr.size());
				if (lstr == rstr)
					return 1;
				else
					return 0;
			}
			if (rstr.size() && lstr.size() && rstr.at(0) != '=' && lstr.at(0) == '=') {
				lstr = lstr.substr(1, lstr.size());
				if (lstr == rstr)
					return 1;
				else
					return 0;
			}

			if (rstr.size() && lstr.size() && rstr.at(0) == '!' && lstr.at(0) != '!') {
				rstr = rstr.substr(1, rstr.size());
				invert = true;
			}
			if (rstr.size() && lstr.size() && rstr.at(0) != '!' && lstr.at(0) == '!') {
				lstr = lstr.substr(1, lstr.size());
				invert = true;
			}
			if (rstr.size() && lstr.size() && lstr.at(0) == '!' && lstr.at(0) == '!') {
				lstr = lstr.substr(1, lstr.size());
				rstr = rstr.substr(1, rstr.size());
			}
			if (!invert) {
				distance = max(distance, d->hypernym_dist(lstr, rstr, max_sep));
			} else {
				distance = max(distance, 1 - d->hypernym_dist(lstr, rstr, max_sep));
				invert = false;
			}
			if (debug)
				cout << "MATCH3::: " << *liter << ", " << ", " << *riter << ", " << rstr << ": " << distance << " - "
						<< max_sep << endl;
			if (distance > 0.4) {
				return distance; // profiling suggests there is no point in trying everything
			}
		}
	}

	/// You are doing the cycle twice!!!! Correct this!!!
	vector<boost::shared_ptr<Predicate> > hyper_trees = k->getHypernym();

	if (distance == 0
		&& head_strs.size() && ref_strs.size()
	) {
		string head_str = head_strs.at(0);
		string rhs_str = ref_strs.at(0);

		//puts("HYPER:::");
		//print_vector(hyper_trees);

		distance = max(distance, d->hypernym_dist_from_trees(head_str, rhs_str, hyper_trees, max_sep));
		if(return_name != 0)
			*return_name = head_str;
		if(return_name != 0)
			*rreturn_name = rhs_str;
		if (debug)
			cout << "MATCH33::: " << head_str << ", " << ", " << rhs_str << ": " << distance << " - "
			<< max_sep << endl;
	}
	/// You are doing the cycle twice!!!! Correct this!!!

	return distance;
}


double get_string_vector_distance(metric *d, const string &head_str, const string &rhs_str, Knowledge *k, int max_sep, string *return_name, string *rreturn_name)
{
     vector<string> head_strs;
     boost::split(head_strs, head_str, boost::is_any_of("|"));

     vector<string> ref_strs;
     boost::split(ref_strs, rhs_str, boost::is_any_of("|"));

	return get_string_vector_distance(d, head_strs, ref_strs, k, max_sep, return_name, rreturn_name);
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
		to_return.push_back("group");
		to_return.push_back("man");
		to_return.push_back("relative");
		to_return.push_back("human_being");
		to_return.push_back("corporation");
		to_return.push_back("organization");
		to_return.push_back("politician");
		to_return.push_back("leader");
		to_return.push_back("team");
		to_return.push_back("other");
		to_return.push_back("thing");
		to_return.push_back("[noun.person]");
	}
	if (str == "material") {
		to_return.push_back("metal");
	}
	if (str == "place") {
		to_return.push_back("district");
		to_return.push_back("city");
		to_return.push_back("town");
		to_return.push_back("state");
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
		to_return.push_back("domestic_animal");
	}
	//cout << "PERSON:::" << str << endl;
	return to_return;
}

static vector<string> get_reference_string(string ref_str)
{
	vector<string> ret_str;
	ret_str.push_back(ref_str);
	if (ref_str == "he" || ref_str == "him" || ref_str == "himself") {
		ret_str.push_back("man");
		ret_str.push_back("person");
		ret_str.push_back("male_person");
	} else if (ref_str == "she" || ref_str == "her" || ref_str == "herself") {
		ret_str.push_back("woman");
		ret_str.push_back("person");
		ret_str.push_back("female_person");
	} else if (ref_str == "someone") {
		ret_str.push_back("man");
		ret_str.push_back("woman");
		ret_str.push_back("person");
		ret_str.push_back("female_person");
	} else if (ref_str == "we" || ref_str == "us") {
		ret_str.push_back("man");
		ret_str.push_back("woman");
		ret_str.push_back("person");
		ret_str.push_back("female_person");
	} else if (ref_str == "they" || ref_str == "them" || ref_str == "themselves") {
		ret_str.push_back("man");
		ret_str.push_back("woman");
		ret_str.push_back("person");
		ret_str.push_back("female_person");
	} else if (ref_str == "it" || ref_str == "itself") {
		ret_str.push_back("thing");
		ret_str.push_back("material");
		ret_str.push_back("building");
		ret_str.push_back("domestic_animal");
		ret_str.push_back("beast");
		ret_str.push_back("bird");
		ret_str.push_back("socialism");
		ret_str.push_back("capitalism");
		ret_str.push_back("political_movement");
		ret_str.push_back("step");
		ret_str.push_back("reptile");
		ret_str.push_back("snake");
	} else
		ret_str.push_back(ref_str);
	return ret_str;
}

static vector<string> get_clean_names(const vector<DrtPred> &preds, MatchElements *me)
{
	vector<string> to_return;
	for(int n=0; n < preds.size(); ++n) {
		string header= extract_header(preds.at(n) );
		if(debug) {
			cout << "CLEANING::: " << header << " ";
			DrtVect to_print(preds);
			print_vector(to_print);
		}

		if(me->hasElement(header))
			continue;
		to_return.push_back(header);
	}
	return to_return;
}

double Match::singleMatch(const DrtPred &data, const DrtPred &hyp, MatchSubstitutions *msubs, MatchType *mtype)
/// Data and Hyp are inverted!!
{
	double ret_weigth = 0, sep;

	metric *d = metric_singleton::get_metric_instance();

	DrtPred tmp_pred = hyp;

	DrtMgu retDrtMgu = msubs->getDrtMgu();

	string head_data = extract_header(data);
	string head_hyp = extract_header(hyp);


	if (debug) {
		cout << "PROPER3:: " << data << ", " << hyp << endl;
		cout << "PROPER3:: " << head_data << ", " << head_hyp << endl;
		cout << "PROPER3:: " << data.tag() << ", " << hyp.tag() << endl;
		cout << "PROPER3:: " << data.name() << ", " << hyp.name() << endl;
		cout << "PROPER3:: " << data.is_question() << ", " << hyp.is_question() << endl;
	}
	//string what_str = get_what();
	string what_str = "[what]";
	if (data.is_verbatim() && !hyp.is_verbatim() && head_hyp.find(what_str) == string::npos
			&& head_hyp.find("[*]") == string::npos) {
		return 0;
	}
	if (!data.is_verbatim() && hyp.is_verbatim() && head_data.find(what_str) == string::npos
			&& head_data.find("[*]") == string::npos) {
		return 0;
	}

	if (data.is_verbatim() && hyp.is_verbatim()
			&& (head_data.find("[verbatim]_[*]") != string::npos || head_hyp.find("[verbatim]_[*]") != string::npos)) {
		return 1;
	}

	if (hyp.tag() == "!WP" && data.is_question() ) {
		return 0;
	}
	if (data.tag() == "!WP" && hyp.is_question() ) {
		return 0;
	}
	if (debug) {
		cout << "PROPER4:: " << data << ", " << hyp << endl;
	}
	if (head_data.find("[JJ]") != string::npos && hyp.is_adjective()) {
		return 1;
	}
	if (head_data.find("[NNP]") != string::npos && hyp.is_proper_name()) {
		return 1;
	}
	if (head_data.find("[number]") != string::npos && hyp.is_number()) {
		return 1;
	}

	// proper names must be separately. A proper name can be a place, hence the comparison as normal names
	if (data.is_proper_name() && hyp.is_proper_name()) {
		if (contained_NNP(head_data, head_hyp) || contained_NNP(head_hyp, head_data)
		// "noam_chomsky" and "chomsky" are the same proper noun
				) {
			implant_header(tmp_pred, extract_header(data));
			if (data.unify(tmp_pred, &retDrtMgu)) {
				msubs->setDrtMgu(retDrtMgu);
				return 1;
			}
		}
	}

	if (debug) {
		cout << "PROPER5:: " << data << ", " << hyp << endl;
	}

	// A proper name can contain a common name ("E5_Freeway" contains "freeway")
	/// clearly this includes the previous check
	if (data.is_name() && !data.is_proper_name() && hyp.is_proper_name()) {
		if (head_data.find("[NNP]") != string::npos)
			return 1;
		if (contained_NNP(head_data, head_hyp) || contained_NNP(head_hyp, head_data)
		// "noam_chomsky" and "chomsky" are the same proper noun
				) {
			implant_header(tmp_pred, extract_header(data));
			if (data.unify(tmp_pred, &retDrtMgu)) {
				msubs->setDrtMgu(retDrtMgu);
				return 1;
			}
		}
	}
	if (debug) {
		cout << "PROPER6:: " << data << ", " << hyp << endl;
	}

	if (data.is_date() && extract_header(hyp) == "time") {
		return 1;
	}

	if (hyp.is_date() && extract_header(data) == "time") {
		return 1;
	}


	// A proper name can contain a common name ("E5_Freeway" contains "freeway")
	/// clearly this includes the previous check
	/// NO! -> it confuses "mexico_city" and "city"
//	if (data.is_name() && hyp.is_name()) {
//		vector<string> strs;
//		string new_head_data = head_data;
//		string new_head_hyp  = head_hyp;
//		if (new_head_data.find("_") != string::npos && new_head_hyp.find("_") == string::npos) {
//			boost::split(strs, new_head_data, boost::is_any_of("_"));
//			new_head_data = strs.back();
//		}
//		if (new_head_hyp.find("_") != string::npos && new_head_data.find("_") == string::npos) {
//			boost::split(strs, new_head_hyp, boost::is_any_of("_"));
//			new_head_hyp = strs.back();
//		}
//		if (new_head_data == new_head_hyp) {
//
//			implant_header(tmp_pred, extract_header(data));
//			if (data.unify(tmp_pred, &retDrtMgu)) {
//				lme_.insertPrevious(extract_header(hyp) );
//				msubs->setDrtMgu(retDrtMgu);
//				return 1;
//			}
//		}
//	}

	// An adjective can pertain to a name (without being a hyponym of the name)
	if (hyp.is_name() // hyp.is_adjective() <= sometimes an adjective is tagged as NN
	&& data.is_name()) {
		if (d->pertains_to_name(head_hyp, head_data, hyp_height_) > 0.5) {
			DrtPred tmp_pred2 = hyp;
			implant_header(tmp_pred2, extract_header(data));
			if (data.unify(tmp_pred2, &retDrtMgu)) {
				msubs->setDrtMgu(retDrtMgu);
				return 1;
			}
		}
	}

	////  DO IT IN SOME OTHER WAY (this way is too slow)
	if (mtype->is_time() // There should be a comparison between dates
	&& (!data.is_date() || !hyp.is_date()) // But there are no dates
			) {
		// Therefore try to retrieve the dates from the KB
		//puts("RETRIEVING_DATE:::");
		string data_date, hyp_date;
		if (!data.is_date()) {
			data_date = retrieve_date(k_, data);
			//if (data_date == "")
			//     return 0;
		} else
			data_date = extract_header(data);
		if (!hyp.is_date()) {
			hyp_date = retrieve_date(k_, hyp);
			//if (hyp_date == "")
			//     return 0;
		} else
			hyp_date = extract_header(hyp);
		if (mtype->matchDate(data_date, hyp_date)) {
			mtype->set_equal(); // reset the mtype
			mtype->unset_time();
			implant_header(tmp_pred, extract_header(data));
			if (data.unify(tmp_pred, &retDrtMgu)) {
				msubs->setDrtMgu(retDrtMgu);
				return 1;
			}
		}
	}

	if (extract_header(data) == extract_header(tmp_pred) && !data.is_complement()) {
		if (data.unify(hyp, &retDrtMgu)) {
			double sep = 1.0;
			if(hyp.is_verb() && data.is_verb() && !has_object(hyp) && !has_object(data) && is_transitive(hyp) && is_transitive(data) )
				sep += 1.0;
			msubs->setDrtMgu(retDrtMgu);
			return sep;
		}
	} else if (data.is_date() && hyp.is_date()) {
		if (mtype->matchDate(data, hyp)) {
			mtype->set_equal(); // reset the mtype
			mtype->unset_time();
			implant_header(tmp_pred, extract_header(data));
			if (data.unify(tmp_pred, &retDrtMgu)) {
				msubs->setDrtMgu(retDrtMgu);
				return 1;
			}
		}
	} else if (data.is_number() && hyp.is_number()) {
		if (mtype->matchNumber(data, hyp)) {
			mtype->set_equal(); // reset the mtype
			implant_header(tmp_pred, extract_header(data));
			if (data.unify(tmp_pred, &retDrtMgu)) {
				msubs->setDrtMgu(retDrtMgu);
				return 1;
			}
		}
	} else if (data.is_verb() && hyp.is_verb()) {
		string levin_data = d_->get_levin_verb(head_data);
		string levin_hyp = d_->get_levin_verb(head_hyp);
		if(debug) {
			cout << "MOBJ01::: " << data << " " << hyp << endl;
			cout << "MOBJ02::: " << has_object(data) << " " << has_object(hyp) << endl;
			cout << "MOBJ03::: " << is_transitive(data) << " " << is_transitive(hyp) << endl;
		}
		if ((head_data == "do"
				&& head_hyp != "be"
				//&& levin_hyp != "verb.stative"
				) //|| (head_hyp == "do"&& levin_data != "verb.stative")
		||
		(head_data == "happen"
				&& head_hyp != "be"
				//&& levin_hyp != "verb.stative"
				) //|| (head_hyp == "happen"&& levin_data != "verb.stative")
		||
		(head_data == "occurr"
				&& head_hyp != "be"
		)
				|| head_data.find("[*]") != string::npos || head_hyp.find("[*]") != string::npos) {
			implant_header(tmp_pred, extract_header(data));
			if (data.unify(tmp_pred, &retDrtMgu)) {
				msubs->setDrtMgu(retDrtMgu);
				return 1; /// "do" and "happen" often appear in questions as generic verbs
			}
		} else {
			if ((head_data == "be" && head_hyp != "be") || (head_data != "be" && head_hyp == "be")
					|| (head_data == "have" && head_hyp != "have") || (head_data != "have" && head_hyp == "have"))
				return 0;
			/// it is too dangerous to compare
			/// directly verbs to auxiliaries. YOU
			/// MUST CORRECT THIS, by instantiating
			/// the meaning of the auxiliaries.
			vector<string> all_candidates_hyp, all_candidates_data;
			all_candidates_hyp = get_similar_verbs(head_hyp);
			all_candidates_data = get_similar_verbs(head_data);
			for (int n = 0; n < all_candidates_hyp.size(); ++n) {
				for (int m = 0; m < all_candidates_data.size(); ++m) {
					if(is_forbidden_synonym_verb(all_candidates_data.at(m),all_candidates_hyp.at(n)))
						continue;
					sep = d_->verb_hypernym_dist(all_candidates_hyp.at(n), all_candidates_data.at(m), hyp_height_);
					implant_header(tmp_pred, extract_header(data));
					if (sep >= 0.4 && data.unify(tmp_pred, &retDrtMgu)) {
						if(debug) {
							cout << "MOBJ1::: " << data << " " << hyp << endl;
							cout << "MOBJ2::: " << has_object(data) << " " << has_object(hyp) << endl;
							cout << "MOBJ3::: " << is_transitive(data) << " " << is_transitive(hyp) << endl;
						}

						if(!has_object(hyp) && !has_object(data) && is_transitive(hyp) && is_transitive(data) )
							sep += 1.0;
						msubs->setDrtMgu(retDrtMgu);
						return sep; /// "do" and "happen" often appear in questions as generic verbs
					}
				}
			}
		}
	} else if (data.is_complement() && hyp.is_complement()) {
		if (mtype->matchComplement(data, hyp)) {
			if (debug) {
				cout << "MATCHCOMPL::: " << data << " " << hyp << endl;
			}
			implant_header(tmp_pred, extract_header(data));
			if (data.unify(tmp_pred, &retDrtMgu)) {
				msubs->setDrtMgu(retDrtMgu);
				return 1;
			} else
				return 0;
		}
		double sep = get_string_vector_distance(d, head_hyp, head_data, k_, hyp_height_);
		if (sep > 0.4) {
			if (head_data.find("|") != string::npos) {
				// if the upper complement is a conjunction "@PLACE_AT|@MOTION_TO",
				// then keep memory of the complement in the data (that must be one of the two)
				//cout << "SUBP3@@@::: " << head_hyp << ", " << head_data << endl;
				msubs->addSubstitution(head_data, head_hyp);
			}
			implant_header(tmp_pred, extract_header(data));
			if (data.unify(tmp_pred, &retDrtMgu)) {
				msubs->setDrtMgu(retDrtMgu);
				return 1;
			}
		} else
			return 0;

	} else if (!data.is_verb() && !hyp.is_verb()) {
		string rhs_str = head_data; // the question is always rhs
		string lhs_str = head_hyp;

		if(debug) {
			cout << "RHS_STR::: " << rhs_str << endl;
		}

		string lhs_ref = extract_first_tag(hyp);
		string rhs_ref = extract_first_tag(data);

		// all the names of the reference are added to lhs_str
		vector<string> lhs_names;
		vector<string> rhs_names;

		if(head_data == "[what]") {
			//head_data = get_what();
			rhs_names = d->get_what();
		}
		if(inverted_person_ && head_hyp == "[what]") { // "[what]" cannot be a hyponym except for inverted matches
			//head_hyp = get_what();
			lhs_names = d->get_what();
		}

		vector<string> lhs_strs;

		if(rhs_str.size() && rhs_str.find("|") != string::npos) {
			vector<string> rhs_strs;
			boost::split(rhs_strs, rhs_str, boost::is_any_of("|"));
			if(rhs_strs.size() > 1)
				rhs_names.insert(rhs_names.end(), rhs_strs.begin(), rhs_strs.end());
			else
				rhs_names.push_back(rhs_str);
		} else
			rhs_names.push_back(rhs_str);
		lhs_names.push_back(lhs_str);

		vector<DrtPred> lhs_preds = k_->getPredsFromRef(lhs_ref);

		if(debug) {
			cout << "LPREDS:::" << endl;
			print_vector(lhs_preds);
		}

		vector<string> new_lhs_names = get_clean_names(lhs_preds, &lme_ );
		lhs_names.insert(lhs_names.end(), new_lhs_names.begin(), new_lhs_names.end());

		if(debug) {
			cout << "LPREDS:::" << endl;
			print_vector(lhs_preds);
		}

		if (debug) {
			cout << "LNAMES:::";
			print_vector(lhs_names);
		}

		if (debug) {
			cout << "RNAMES:::" << endl;
			print_vector(rhs_names);
		}

		if (data.is_PRP()) {
			vector<string> tmp_names = get_reference_string(rhs_str);
			rhs_names.insert(rhs_names.end(), tmp_names.begin(), tmp_names.end());
		}

		vector<string> similar_rhs_names = get_similar_names(rhs_str);
		rhs_names.insert(rhs_names.end(), similar_rhs_names.begin(), similar_rhs_names.end());

		string selected_name, rselected_name;
		sep = get_string_vector_distance(d, lhs_names, rhs_names, k_, hyp_height_, &selected_name, &rselected_name);
		if(debug) {
			cout << "RSTR:::" << rhs_str << endl;
			cout << "LSTR:::" << lhs_str << endl;
			cout << "SELECTED_NAME:::" << selected_name << endl;
			cout << "W:::" << sep << endl;
		}
		if(inverted_person_ && sep < 0.2)
			sep = get_string_vector_distance(d, rhs_names, lhs_names, k_, hyp_height_, &selected_name, &rselected_name);
		lme_.insertPrevious(selected_name);

		if (sep >= 0.2) {
			implant_header(tmp_pred, extract_header(data));
			if (data.unify(tmp_pred, &retDrtMgu)) {
				msubs->setDrtMgu(retDrtMgu);
				ret_weigth += sep;
			} else
				return 0;

			if(debug) {
				cout << "NAME_BAR::: " << head_data << " " << head_hyp << endl;
			}

			if (head_data.find("|") != string::npos) {
				if(debug)
					cout << "ADD_SUB::: " << head_data << " " << rselected_name << endl;
				if(rselected_name != "")
					msubs->addSubstitution(head_data, rselected_name);
				else
					msubs->addSubstitution(head_data, selected_name); // Something has to be added
			}
          } else if (inverted_person_ &&
          		 data.is_proper_name() && !hyp.is_proper_name() && hyp.is_name()) {
               implant_header(tmp_pred, extract_header(hyp));
               // check that the Proper Name refers to a person
               bool is_people_name= d->gender_proper_name(head_data) != "";
               if(debug) {
               	cout << "PEOPLE_INV::: " << is_people_name << " " << tmp_pred << " " << data << endl;
               }
               DrtPred tmp_data= data;
               if(is_people_name) {
               	implant_header(tmp_data, extract_header(hyp));
               }
               if (head_hyp == "person" && is_people_name && tmp_data.unify(tmp_pred, &retDrtMgu)) {
                    msubs->setDrtMgu(retDrtMgu);
                    return 1; /// in this way a proper name always refer to a person
               }
		} else if (hyp.is_proper_name() && !data.is_proper_name() && data.is_name()) {
			implant_header(tmp_pred, extract_header(data));
			bool is_people_name = true;
			for(int lpos=0; lpos < lhs_names.size(); ++lpos ) {
				is_people_name = d->gender_proper_name(lhs_names.at(lpos)) != "";
				if(is_people_name)
					break;
				if(debug) {
					cout << "PROPER_NAME::: " << is_people_name << ", " << head_hyp << endl;
				}
			}
			if ((head_data.find("person") != string::npos || head_data.find("[noun.person]") != string::npos) && is_people_name
					&& data.unify(tmp_pred, &retDrtMgu)) {
				msubs->setDrtMgu(retDrtMgu);
				return 1; /// in this way a proper name always refer to a person
			}
		} else
			return 0;
	}
	msubs->setDrtMgu(retDrtMgu);
	return ret_weigth;
}

double Match::phraseMatchNames(vector<DrtPred> &lpreds, vector<DrtPred> &rpreds, MatchSubstitutions *msubs, DrtMgu negated_upg)
// The question is in the rhs.
{
	int lsize= lpreds.size();
	int rsize= rpreds.size();
	if(rsize > lsize)
		return 0;

	double w;
	int safe = 0, safe_max = 2;
	MatchSubstitutions msubs2;
	clock_t start1;
	if (debug)
		start1 = clock();

	for (; safe < safe_max; ++safe) {
		w = 0;
		double wtmp = 0;
		MatchType mtype;
		int matched = 0;
		msubs2 = *msubs;
		vector<int> already_parsed;
		for (int n = 0; n < rpreds.size(); ++n) {
			DrtPred rpred = rpreds.at(n);
			rpred / msubs2.getDrtMgu();
			int m_start, m_end;
			m_start = 0;
			m_end   = lpreds.size();

			clock_t start0;
			if (debug)
				start0 = clock();
			for (int m = m_start; m < m_end; ++m) {
				MatchSubstitutions msubs_tmp;
				if(shortfind(already_parsed,m))
					continue;
				DrtPred lpred = lpreds.at(m);
				if( (lpred.is_name() && rpred.is_name() )
						|| (lpred.is_complement() && rpred.is_complement() )
						|| (lpred.is_verb() && rpred.is_verb() )
				) {
					clock_t start;
					if (debug)
						start = clock();
					double wtmp = singleMatch(rpred, lpred, &msubs_tmp, &mtype);
					if (debug) {
						clock_t end = clock();
						int all_time = (end - start);
						cout << "Mtime_single_match::: " << all_time / (double) CLOCKS_PER_SEC << endl;
					}
					DrtMgu tmp_upg = msubs_tmp.getDrtMgu();
					if (debug) {
						cout << "MATCHING::::" << lpred << " " << rpred << endl;
						cout << "MATCHING::::" << wtmp << endl;
						cout << "MATCHING::::" << negated_upg << endl;
						cout << "MATCHING::::" << tmp_upg << endl;
					}
					if (wtmp != 0 && !(tmp_upg.size() && shortfind(negated_upg, tmp_upg))) {
						if (debug) {
							cout << "MATCHING_FINAL4::::" << w << endl;
						}
						w += wtmp;
						DrtMgu orig_upg = msubs2.getDrtMgu();
						orig_upg.add(tmp_upg);
						msubs2.setDrtMgu(orig_upg);
						msubs2.setSubstitutions(msubs_tmp.getSubstitutions());
						++matched;
						already_parsed.push_back(m);
						break;
					}
				}
			}
			if (debug) {
				clock_t end0 = clock();
				int all_time = (end0 - start0);
				cout << "Mtime_all_match::: " << all_time / (double) CLOCKS_PER_SEC << endl;
			}
		}
		if (matched < rpreds.size()) {
			negated_upg.addWithoutUnification(msubs2.getDrtMgu());
		} else
			break;
	}
	if (debug) {
		clock_t end1 = clock();
		int all_time = (end1 - start1);
		cout << "Mtime_all_match1::: " << all_time / (double) CLOCKS_PER_SEC << endl;
	}
	if (safe == safe_max) {
		if (debug) {
			cout << "MATCHING_FINAL2::::" << w << endl;
		}
		w = 0;
	} else {
		*msubs = msubs2;
		if (debug) {
			cout << "MATCHING_FINAL::::" << msubs->getDrtMgu() << endl;
		}
	}

	return w;
}

double Match::phraseMatch(vector<DrtPred> &answers, vector<DrtPred> &hyp, MatchSubstitutions *msubs, DrtMgu negated_upg)
{
	double ret_weigth = 0;
	int hyp_hooks = hyp.size();
	int hooks;
	hooks = 0;
	vector<DrtPred>::iterator answerIter;
	vector<DrtPred>::iterator endAnswer = answers.end();
	vector<DrtPred>::iterator hypIter;
	vector<DrtPred>::iterator endHyp = hyp.end();
	MatchSubstitutions msubs2 = *msubs;
	DrtMgu upgTmp(msubs->getDrtMgu()); // The initial unifiers list is kept from the value of retDrtMgu
	DrtPred hypoTmp;
	answerIter = answers.begin();
	hypIter = hyp.begin();
	MatchType mtype;
	for (; hypIter != endHyp; ++hypIter) {
		for (; answerIter != endAnswer; ++answerIter) {
			hypoTmp = (*hypIter);
			hypoTmp / upgTmp;
			MatchSubstitutions msubs_tmp;
			double tmp_w = singleMatch(hypoTmp, *answerIter, &msubs_tmp, &mtype);

			string head_data2 = extract_header(hypoTmp);
			string head_hyp2 = extract_header(*answerIter);

			DrtMgu tmp_u = msubs_tmp.getDrtMgu();
			if (debug) {
				cout << "MATCHING::::" << hypoTmp << ", " << *answerIter << "= " << tmp_w << endl;
				cout << "MATCHING::::" << hypoTmp.tag() << ", " << answerIter->tag() << "= " << tmp_w << endl;
				cout << "MATCHING::::" << negated_upg << endl;
				cout << "MATCHING::::" << tmp_u << endl;
			}

			if (tmp_w > 0 && !(tmp_u.size() && shortfind(negated_upg, tmp_u))) {
				upgTmp.insert(upgTmp.end(), tmp_u.begin(), tmp_u.end());
				msubs2.addSubstitution(msubs_tmp.getSubstitutions());
				ret_weigth += tmp_w;
				++answerIter;
				++hooks;
				break;
			}
		}
	}

	if (debug) {
		cout << "HOOKS::: " << hooks << endl;
		cout << "HOOKS::: " << hyp_hooks << endl;
		cout << "HOOKS2::: " << endl;
		print_vector(hyp);
	}

	if (hooks == 0 || hooks != hyp_hooks)
		return 0;

	msubs->setDrtMgu(upgTmp);
	msubs->addSubstitution(msubs2.getSubstitutions());
	return ret_weigth / hooks;
}

bool contain_not(const vector<DrtPred> &adv)
{
	for (int n = 0; n < adv.size(); ++n) {
		string head = extract_header(adv.at(n));
		if (adv.at(n).is_adverb() && head == "not")
			return true;
	}
	return false;
}

double Match::singleGraphMatch(const SingleMatchGraph &lhs, const SingleMatchGraph &rhs, MatchSubstitutions *msubs)
// the question is in the rhs
{
	double wsubj = 0, wobj = 0, wverb = 0, wspec = 0, wcompl = 0;
	double w = 0;
	// retDrtMgu is updated in every "phraseMatch".

	// compare subjects
	vector<DrtPred> lsubj = lhs.getSubject();
	vector<DrtPred> rsubj = rhs.getSubject();

	if(debug) {
		puts("AAAAA");
		print_vector(lsubj);
		print_vector(rsubj);
		puts("AAAAA2");
	}
	if (lsubj.size() != 0 && rsubj.size() != 0) {
		wsubj = this->phraseMatch(lsubj, rsubj, msubs);
		if (debug) {
			cout << "SUBJECT_W:::" << wsubj << endl;
			cout << msubs->getDrtMgu() << endl;
		}
		if (wsubj == 0)
			return 0;
		else {
			w += wsubj;
			msubs->increase();
		}
	} else if (rsubj.size() != 0 && lsubj.size() == 0)
		return 0;

	// compare verbs
	vector<DrtPred> lverb;
	vector<DrtPred> rverb;

	lverb.push_back(lhs.getVerb());
	rverb.push_back(rhs.getVerb());

	wverb = this->phraseMatch(lverb, rverb, msubs);

	if (wverb == 0)
		return 0;
	else {
		w += wverb;
		msubs->increase();
	}

	if (debug)
		puts("OBJECT_BEFORE:::");

	// compare objects
	vector<DrtPred> lobj = lhs.getObject();
	vector<DrtPred> robj = rhs.getObject();

	if(debug) {
		puts("AAAAA3");
		print_vector(lobj);
		print_vector(robj);
		puts("AAAAA4");
	}
	if (lobj.size() == 0 && robj.size() != 0)
		return 0;
	if (lobj.size() != 0 && robj.size() != 0) {
		wobj = this->phraseMatch(lobj, robj, msubs);
		if (debug) {
			cout << "OBJECT_W:::" << wobj << endl;
		}
		if (wobj == 0)
			return 0;
		else {
			w += wobj;
			msubs->increase();
		}
	} else if (robj.size() != 0 && lobj.size() == 0)
		return 0;

	if (debug)
		puts("OBJECT_DONE:::");

	double wtmp;
	// compare complements
	vector<vector<DrtPred> > lcompl = lhs.getComplements(msubs->getDrtMgu());
	vector<vector<DrtPred> > rcompl = rhs.getComplements(msubs->getDrtMgu());

	if(debug) {
		puts("AAAAA5");
		for(int n=0; n < lcompl.size(); ++n) {
			DrtVect drtvect = lcompl.at(n);
			print_vector(drtvect);
		}
		for(int n=0; n < rcompl.size(); ++n) {
			DrtVect drtvect = rcompl.at(n);
			print_vector(drtvect);
		}
		puts("AAAAA6");
	}

	if (rcompl.size() != 0 && lcompl.size() == 0)
		return 0;

	// The question is not matched if the candidate
	// answer does not have the same complements

	if (debug) {
		cout << "MSIZE0::: " << rcompl.size() << " " << lcompl.size() << endl;
	}

	wcompl = 0;
	int m, matched = 0;
	for (m = 0; m < rcompl.size(); ++m) {
		MatchSubstitutions msubs_tmp = *msubs;
		double wComplTmp = 0;
		int tmp_matched = 0;
		for (int n = 0; n < lcompl.size(); ++n) {
			if(debug) {
				cout << "COMPLEMENTS::: " << endl;
				print_vector(rcompl.at(m));
				print_vector(lcompl.at(n));
			}
			wtmp = this->phraseMatch(lcompl.at(n), rcompl.at(m), &msubs_tmp);
			if (wtmp == 0) {
				wComplTmp = 0;
				msubs_tmp = *msubs;
				continue;
			} else {
				wComplTmp += wtmp;
				*msubs = msubs_tmp;
				++tmp_matched;
				break;
			}
		}
		if (tmp_matched != 0) {
			wcompl = wComplTmp;
			*msubs = msubs_tmp;
			matched += tmp_matched;
		}
	}
	if (debug) {
		cout << "MSIZE::: " << wcompl << " " << matched << " " << rcompl.size() << endl;
	}
	if ((rcompl.size() && wcompl == 0) || matched != rcompl.size())
		return 0;
	w += wcompl; /// The complements are counted twice !!

	// compare adverbs
	vector<DrtPred> ladv = lhs.getAdverb();
	vector<DrtPred> radv = rhs.getAdverb();
	double wadv = 0;
	try {
		wadv = this->adverbMatch(ladv, radv);
		w += wadv;
	} catch (std::runtime_error &e) {
		if (debug)
			cout << "ADV:::" << endl;
		return 0;
	}

	if(debug)
		puts("RETURN:::");
	return w;
}

double Match::adverbMatch(const vector<DrtPred> &ladv, const vector<DrtPred> &radv)
// two adverbs with the same pertainym (pertadjective?) match
{
	double w = 0;

	metric *d = metric_singleton::get_metric_instance();

	for (int rn = 0; rn < radv.size(); ++rn) {
		vector<string> rpertainyms = d->get_adv_pertainyms(extract_header(radv.at(rn)));
		if (debug) {
			puts("PERTAINYMS::: ");
			print_vector(rpertainyms);
		}
		if (rpertainyms.size() == 0)
			continue;

		double wtmp = 0;
		for (int ln = 0; ln < ladv.size(); ++ln) {
			vector<string> lpertainyms = d->get_adv_pertainyms(extract_header(ladv.at(ln)));
			for (int n = 0; n < lpertainyms.size(); ++n) {
				if (shortfind(rpertainyms, lpertainyms.at(n)))
					wtmp += 1;
			}
		}
		if (wtmp == 0)
			throw std::runtime_error("");
		w += wtmp;
	}

	return w;
}

double Match::matchQuantifiers(DrtPred lp, DrtPred rp, MatchSubstitutions *msubs)
{
	map<string, vector<string> > map_quant;
	map_quant["all"].push_back("all");
	map_quant["most"].push_back("all");
	map_quant["most"].push_back("most");
	map_quant["many"].push_back("all");
	map_quant["many"].push_back("many");
	map_quant["many"].push_back("most");
	map_quant["some"].push_back("all");
	map_quant["some"].push_back("many");
	map_quant["some"].push_back("some");
	map_quant["some"].push_back("most");

	string lstr = extract_second_tag(lp);
	string rstr = extract_second_tag(rp);

	if (debug)
		cout << "QUANTIFIER:::" << lstr << " " << rstr << endl;

	map<string, vector<string> >::iterator miter = map_quant.find(rstr);
	if (miter != map_quant.end()) {
		vector<string> l_candidates = miter->second;
		if (debug)
			print_vector(l_candidates);
		if (shortfind(l_candidates, lstr)) {
			implant_second(lp, rstr);
			DrtMgu retDrtMgu = msubs->getDrtMgu();
			if (rp.unify(lp, &retDrtMgu)) {
				msubs->setDrtMgu(retDrtMgu);
				return 1;
			}
		}
	}

	return 0;
}

static vector<DrtPred> normalize_quantifiers(const vector<DrtPred> &lquant, const vector<DrtPred> &rquant)
// if a quantifier is present on the right but not on the left then add a dummy "some" on the left side
{
	vector<DrtPred> to_return(lquant);

	for (int n = 0; n < rquant.size(); ++n) {
		bool r_is_found = false;
		string rfstr = extract_first_tag(rquant.at(n));
		string rsstr = extract_second_tag(rquant.at(n));
		for (int m = 0; m < lquant.size(); ++m) {
			string lfstr = extract_first_tag(lquant.at(m));
			string lsstr = extract_second_tag(lquant.at(m));
			if (lfstr == rfstr)
				r_is_found = true;
		}
		if (!r_is_found) {
			DrtPred tmp_pred(string("@QUANTIFIER(") + rfstr + ",some)");
			to_return.push_back(tmp_pred);
		}
	}

	return to_return;
}

static DrtMgu get_upg_to_negate(MatchSubstitutions msub)
{
	DrtMgu to_return;

	DrtMgu upg = msub.getDrtMgu();
	DrtMgu::iterator it = upg.begin(), iend = upg.end();
	for (; it != iend; ++it) {
		if (ref_is_verb(it->first))
			to_return.add(it->first, it->second);
	}

	return to_return;
}

static bool verb_has_broken(const DrtPred &pred)
{
	if (debug) {
		puts("HAS_BROKEN:::");
		cout << pred << endl;
	}

	string subj = extract_subject(pred);
	string obj = extract_object(pred);
	if (is_broken(subj) || is_broken(obj))
		return true;
	return false;
}

double Match::graphMatch(const MatchGraph &lhs, const MatchGraph &rhs, MatchSubstitutions *msubs)
// the question is in the rhs
{
	vector<SingleMatchGraph> lhs_sent = lhs.getSentences();
	vector<SingleMatchGraph> rhs_sent = rhs.getSentences();
	vector<SingleMatchGraph>::iterator siter1 = lhs_sent.begin();
	vector<SingleMatchGraph>::iterator send1 = lhs_sent.end();
	vector<SingleMatchGraph>::iterator siter2 = rhs_sent.begin();
	vector<SingleMatchGraph>::iterator send2 = rhs_sent.end();

	double w = 0, wsentence = 0, wspec = 0, wsubs = 0, wtmp;

	// compare subordinates description (the sub_ variable in SMG)
	vector<DrtPred> lsubs = lhs.getSubs();
	vector<DrtPred> rsubs = rhs.getSubs();

	DrtMgu negated_upg;
	int safe = 0;
	MatchSubstitutions original_msub = *msubs;
	int max_safe = 2;
	for (; safe < max_safe; ++safe) {
		*msubs = original_msub;
		if(debug) {
			puts("MATCHING_SUBS:::");
			print_vector(lsubs);
			print_vector(rsubs);
		}
		if(debug) {
			cout << "RESULT_M0:: " << lsubs.size() << " " << rsubs.size() << endl;
		}
		if (lsubs.size() != 0 && rsubs.size() != 0) {
			wsubs = this->phraseMatchNames(lsubs, rsubs, msubs, negated_upg);
			if(debug) {
				cout << "RESULT_M:: " << wsubs << endl;
			}
			if (wsubs != 0)
				w += wsubs;
			else
				return 0;
		} else if (rsubs.size() != 0 && lsubs.size() == 0)
			return 0;

		// compare the content of the subordinates
		siter1 = lhs_sent.begin();
		siter2 = rhs_sent.begin();

		int matched = 0;
		int pos1 = 0, pos2 = 0;
		for (; siter2 != send2; ++siter2, ++pos2) {
			MatchSubstitutions msubs_tmp = *msubs;
			double wSentTmp = 0;
			int tmp_matched = 0;
			siter1 = lhs_sent.begin();
			pos1 = 0;
			for (; siter1 != send1; ++siter1, ++pos1) {
				wtmp = this->singleGraphMatch(*siter1, *siter2, &msubs_tmp);

				// Check for negative RB
				vector<DrtPred> adv1 = siter1->getAdverb();
				vector<DrtPred> adv2 = siter2->getAdverb();
				bool neg1 = contain_not(adv1);
				bool neg2 = contain_not(adv2);

				if( //match_negative_ &&
						((neg1 && !neg2) || (neg2 && !neg1)) ){
					wtmp = 0; // only sentences that both contain or lack a negation can be matched
					if (debug)
						cout << "NEG::: " << neg1 << " " << neg2 << endl;

				}

				if(debug) {
					cout << "SGM_Result::: " << wtmp << endl;
				}
				if (wtmp == 0) {
					wSentTmp = 0;
					msubs_tmp = *msubs;
					continue;
				} else {
					wSentTmp += wtmp;
					*msubs = msubs_tmp;
					++tmp_matched;
					break;
				}
			}
			if (tmp_matched != 0) {
				wsentence = wSentTmp;
				*msubs = msubs_tmp;
				matched += tmp_matched;
			}
		}
		if (debug) {
			cout << "NEG_UPG2:::" << negated_upg << endl;
		}
		if (debug) {
			cout << "MSIZE2:::" << rhs_sent.size() << " " << wsentence << " " << matched << endl;
		}
		if (!((rhs_sent.size() && wsentence == 0) || matched != rhs_sent.size())) {

			w += wsentence; /// The sentences are counted twice !!
			break; // break the backtracking
		}
		negated_upg.add(get_upg_to_negate(*msubs));
		if (debug) {
			cout << "NEG_UPG:::" << negated_upg << endl;
		}
	}
	if(debug) {
		cout << "SAFE::: " << safe << endl;
	}
	if (safe == max_safe) // the backtracking did not reach a solution
		return 0;

	// compare quantifiers
	vector<DrtPred> lquant = lhs.getQuantifiers();
	vector<DrtPred> rquant = rhs.getQuantifiers();
	rquant / msubs->getDrtMgu();
	lquant = normalize_quantifiers(lquant, rquant);
	int matched = 0;
	double wquant = 0;
	for (int m = 0; m < rquant.size(); ++m) {
		MatchSubstitutions msubs_tmp = *msubs;
		double wQuantTmp = 0;
		int tmp_matched = 0;
		for (int n = 0; n < lquant.size(); ++n) {
			wtmp = this->matchQuantifiers(lquant.at(n), rquant.at(m), &msubs_tmp);
			if (wtmp == 0) {
				wQuantTmp = 0;
				msubs_tmp = *msubs;
				continue;
			} else {
				wQuantTmp += wtmp;
				*msubs = msubs_tmp;
				++tmp_matched;
				break;
			}
		}
		if (tmp_matched != 0) {
			wquant = wQuantTmp;
			*msubs = msubs_tmp;
			matched += tmp_matched;
		}
	}
	if ((rquant.size() && wquant == 0) || matched != rquant.size())
		return 0;
	w += wquant; /// The specifications are counted twice !!

	if (debug) {
		puts("COMPARE_SPECS:::");
	}

	// compare specifications
	vector<vector<DrtPred> > lspec = lhs.getSpecifications();
	vector<vector<DrtPred> > rspec = rhs.getSpecifications();
	matched = 0;
	for (int m = 0; m < rspec.size(); ++m) {
		MatchSubstitutions msubs_tmp = *msubs;
		double wSpecTmp = 0;
		int tmp_matched = 0;
		for (int n = 0; n < lspec.size(); ++n) {
			wtmp = this->phraseMatch(lspec.at(n), rspec.at(m), &msubs_tmp);
			if (wtmp == 0) {
				wSpecTmp = 0;
				msubs_tmp = *msubs;

				continue;
			} else {
				wSpecTmp += wtmp;
				*msubs = msubs_tmp;
				++tmp_matched;
				break;
			}
		}
		if (tmp_matched != 0) {
			wspec = wSpecTmp;
			*msubs = msubs_tmp;
			matched += tmp_matched;
		}
	}
	if ((rspec.size() && wspec == 0) || matched != rspec.size())
		return 0;
	w += wspec; /// The specifications are counted twice !!

	if (debug) {
		puts("COMPARE_SPECS2:::");
	}

	// last check for broken sentences
	siter2 = rhs_sent.begin();
	if ((siter2->getSubject().size() == 0 || siter2->getObject().size() == 0) && verb_has_broken(siter2->getVerb())) {
		DrtVect complete_rhs = rhs.getDrs();

		if (debug) {
			puts("COMPLETE_RHS");
			print_vector(complete_rhs);
		}

		siter1 = lhs_sent.begin();
		vector<DrtPred> lsubj = siter1->getSubject();
		vector<DrtPred> lobj = siter1->getObject();
		DrtVect incomplete_lhs;  // only contains the subjects and object
		incomplete_lhs.insert(incomplete_lhs.end(), lsubj.begin(), lsubj.end());
		incomplete_lhs.insert(incomplete_lhs.end(), lobj.begin(), lobj.end());

		double w2 = 0;
		MatchSubstitutions msubs_tmp = *msubs;
		w2 = this->graphMatchNames(incomplete_lhs, complete_rhs, &msubs_tmp);

		if (debug) {
			puts("COMPLETE_RHS2:::");
			cout << w2 << endl;
		}

		if (w2 == 0)
			return 0;
		*msubs = msubs_tmp;
	}

	return w;
}

double Match::graphMatchNames(const MatchGraph &lhs, const MatchGraph &rhs, MatchSubstitutions *msubs, DrtMgu negated_upg)
// The question is in the rhs.
//
// This function compares two drs where at least one of the two has no verb. Only subjects, objects and specifications matters.
/// You shouls also consider complements!!!
{
	double wsubj = 0, wobj = 0, wverb = 0, wspec = 0, wcompl = 0;
	double w = 0, wtmp = 0;

	vector<DrtPred> lpreds = lhs.getDrs();
	vector<DrtPred> rpreds = rhs.getDrs();

	MatchType mtype;
	int matched = 0;

	for (int m = 0; m < rpreds.size(); ++m) {
		DrtPred rpred = rpreds.at(m);
		MatchSubstitutions msubs_tmp;
		for (int n = 0; n < lpreds.size(); ++n) {
			DrtPred lpred = lpreds.at(n);
			double wtmp = singleMatch(lpred, rpred, &msubs_tmp, &mtype);
			if (wtmp != 0) {
				w += wtmp;
				DrtMgu orig_upg = msubs->getDrtMgu();
				DrtMgu tmp_upg = msubs_tmp.getDrtMgu();
				orig_upg.add(tmp_upg);
				msubs->setDrtMgu(orig_upg);
				msubs->setSubstitutions(msubs_tmp.getSubstitutions());
				++matched;
				break;
			}
		}
	}
	if (matched < rpreds.size())
		w = 0;

	return w;
}

double Match::singleGraphMatchNames(const SingleMatchGraph &lhs, const SingleMatchGraph &rhs, MatchSubstitutions *msubs)
// The question is in the rhs.
//
// This function compares two drs where at least one of the two has no verb. Only subjects, objects and specifications matters.
/// You shouls also consider complements!!!
{
	double wsubj = 0, wobj = 0, wverb = 0, wspec = 0, wcompl = 0;
	double w = 0, wtmp = 0;

	vector<DrtPred> lpreds = lhs.getDrs();
	vector<DrtPred> rpreds = rhs.getDrs();

	MatchType mtype;
	int matched = 0;

	for (int m = 0; m < rpreds.size(); ++m) {
		DrtPred rpred = rpreds.at(m);
		MatchSubstitutions msubs_tmp;
		for (int n = 0; n < lpreds.size(); ++n) {
			DrtPred lpred = lpreds.at(n);
			double wtmp = singleMatch(lpred, rpred, &msubs_tmp, &mtype);
			if (wtmp != 0) {
				w += wtmp;
				DrtMgu orig_upg = msubs->getDrtMgu();
				DrtMgu tmp_upg = msubs_tmp.getDrtMgu();
				orig_upg.add(tmp_upg);
				msubs->setDrtMgu(orig_upg);
				msubs->setSubstitutions(msubs_tmp.getSubstitutions());
				++matched;
				break;
			}
		}
	}
	if (matched < rpreds.size())
		w = 0;

	return w;
}

static void switch_subj_obj(DrtPred &pred)
{
	string fstr = extract_subject(pred);
	string sstr = extract_object(pred);
	string str_tmp;
	str_tmp = fstr;
	fstr = sstr;
	sstr = str_tmp;
	implant_subject(pred, fstr);
	implant_object(pred, sstr);
}

static bool has_TO_BE(const vector<DrtPred> &question)
// if it finds a "to be" verb switch subject with object
{
	vector<DrtPred> to_return(question);
	for (int n = 0; n < to_return.size(); ++n) {
		if (to_return.at(n).is_verb() && extract_header(to_return.at(n)) == "be") {
			return true;
		}
	}

	return false;
}

static vector<DrtPred> invert_TO_BE(const vector<DrtPred> &question)
// if it finds a "to be" verb switch subject with object
{
	vector<DrtPred> to_return(question);
	for (int n = 0; n < to_return.size(); ++n) {
		if (to_return.at(n).is_verb() && extract_header(to_return.at(n)) == "be") {
			switch_subj_obj(to_return.at(n));
		}
	}

	return to_return;
}

static void drs_unique(vector<DrtPred> &preds)
{
	vector<DrtPred> already_present;

	for (int n = 0; n < preds.size(); ++n) {
		string header = extract_header(preds.at(n));
		if (find(already_present.begin(), already_present.end(), preds.at(n)) == already_present.end()) {
			already_present.push_back(preds.at(n));
		} else if(header != "[what]"){
			preds.erase(preds.begin() + n);
			--n;
		}
	}
}

static int count_relevant_preds(const DrtVect &hyp)
{
	int size = 0;

	for (int n = 0; n < hyp.size(); ++n) {
		if (!hyp.at(n).is_complement())
			++size;
	}

	return size;
}

static bool has_subject(const DrtPred &pred)
{
	string subj = extract_subject(pred);
	if (subj.find("ref") != string::npos || subj.find("name") != string::npos)
		return true;
	return false;
}

static bool drtvect_has_subject(const DrtVect &d)
{
	for (int n = 0; n < d.size(); ++n) {
		if (d.at(n).is_verb() && has_subject(d.at(n)))
			return true;
	}
	return false;
}


static bool drtvect_has_object(const DrtVect &d)
{
	for (int n = 0; n < d.size(); ++n) {
		if (d.at(n).is_verb() && has_object(d.at(n)))
			return true;
	}
	return false;
}

double Match::singlePhraseMatch(vector<DrtPred> answers, vector<DrtPred> &hyp, MatchSubstitutions *msubs, MatchInfo mi)
{
	bool only_names= mi.getOnlyNames();
	bool match_negative= mi.getMatchNegative();
	inverted_person_= mi.getInvertedPerson();
	hyp_height_ = mi.getHypernymHeight();

	double w = this->singlePhraseMatch(answers, hyp, msubs, only_names, match_negative);
	return w;
}


static DrtVect promote_elements(DrtVect drtpred)
//noun1/NN(A) @GENITIVE(A,B) noun2(B) -> noun2(A) noun2/NN(A) @GENITIVE(A,B) noun2(B)
/// This function creates an infinite loop!
{
	DrtVect to_return(drtpred);

	int size = drtpred.size();

	for(int n=0; n < size; ++n) {
		string header = extract_header(drtpred.at(n) );
		if(header == "@GENITIVE") {
			string fref = extract_first_tag(drtpred.at(n));
			string sref = extract_second_tag(drtpred.at(n));
			vector<int> poz = find_all_element_with_string(drtpred,sref);
			if(poz.size() == 0)
				continue;
			for(int j=0; j < poz.size(); ++j) {
				int m= poz.at(j);
				string fref2 = extract_first_tag(drtpred.at(m));
				if(fref2.find("presupp_genitive") != -1)
					continue;
				DrtPred new_pred= drtpred.at(m);
				implant_first(new_pred,fref);
				to_return.push_back(new_pred);
			}
		}
	}

	return to_return;
}

static DrtVect multiply_what(DrtVect to_return)
//[what] -> 2x [what]
{
	for(int n=0; n < to_return.size(); ++n) {
		string header = extract_header(to_return.at(n) );
		if(header == "[what]") {
			DrtPred new_pred= to_return.at(n);
			to_return.insert(to_return.begin()+n,new_pred);
			++n;
		}
	}

	return to_return;
}

static bool has_uninstantiated(const DrtVect &drtvect)
{
	for(int n=0; n < drtvect.size(); ++n) {
		vector<string> children = extract_children(drtvect.at(n) );
		for(int m=0; m < children.size(); ++m) {
			if(children.at(m).size()
					&& children.at(m).at(0) == '_'
					&& children.at(m).find("_subj") != -1
					&& children.at(m).find("_obj") != -1
					&& children.at(m).find("_none") != -1
			) {
				return true;
			}
		}
	}

	return false;
}

double Match::singlePhraseMatch(vector<DrtPred> answers, vector<DrtPred> &hyp, MatchSubstitutions *msubs, bool only_names, bool match_negative)
{
	static clock_t all_time;
	match_negative_= match_negative;

	answers = promote_elements(answers);

	if(debug) {
		cout << "AFTER_PROMOTION:::" << endl;
		print_vector(answers);
	}

	answers = multiply_what(answers);

	drs_unique(answers);
	drs_unique(hyp);

	MatchGraph lhs(answers), rhs(hyp);

	if (debug) {
		puts("BBBBBBB");
		print_vector(answers);
		print_vector(hyp);
		puts("BBBBBBB2");
	}
	lhs.computeDrtMguForward();
	rhs.computeDrtMguForward();

	int size = count_relevant_preds(hyp);
	if (lhs.hasVerb() && rhs.hasVerb() && !only_names) {
		clock_t start;
		if (debug)
			start = clock();

		if (debug) {
			puts("HAS_VERB:::");
		}
		double w1 = 0, w2 = 0;
		w1 = this->graphMatch(lhs, rhs, msubs);
		w1 /= size;
		lme_.clear();
		rme_.clear();
		if (w1) {
			(*msubs) / rhs.getDrtMguBackward();
			(*msubs) / lhs.getDrtMguBackward();
			if (debug) {
				cout << "W1:: " << w1 << ", " << msubs->getDrtMgu() << endl;
			}
			if (debug) {
				clock_t end = clock();
				all_time += (end - start);
				cout << "Mtime13::: " << all_time / (double) CLOCKS_PER_SEC << endl;
			}
			return w1;
		}

		if (!has_TO_BE(hyp))
			return 0;

		DrtMgu new_upg;
		msubs->setDrtMgu(new_upg);
		vector<DrtPred> inverted_question = invert_TO_BE(hyp);
		MatchGraph lhs2(answers), rhs2(inverted_question);
		if (debug) {
			puts("BBBBBBB3");
			print_vector(answers);
			print_vector(inverted_question);
			puts("BBBBBBB4");
		}
		w2 = this->graphMatch(lhs2, rhs2, msubs);
		w2 /= size;
		lme_.clear();
		rme_.clear();
		if (w1 || w2) {
			(*msubs) / rhs.getDrtMguBackward();
			(*msubs) / lhs.getDrtMguBackward();
			if (debug) {
				clock_t end = clock();
				all_time += (end - start);
				cout << "Mtime14::: " << all_time / (double) CLOCKS_PER_SEC << endl;
			}
			return max(w1, w2);
		}

		// other checks to be added here
	} else {
		// One of the two drs has no verb. Different matching algorithm:
		double w;

		if (debug) {
			puts("MATCHING_NAMES:::");
		}

		DrtVect lpreds = lhs.getDrs();
		DrtVect rpreds = rhs.getDrs();
		bool neg1 = contain_not(lpreds);
		bool neg2 = contain_not(rpreds);
		if( //match_negative_ &&
				((neg1 && !neg2) || (neg2 && !neg1)) ){
			return 0; // only sentences that both contain or lack a negation can be matched
		}
		w = this->phraseMatchNames(lpreds, rpreds, msubs);
		(*msubs) / rhs.getDrtMguBackward();
		(*msubs) / lhs.getDrtMguBackward();

		rpreds/msubs->getDrtMgu();

		lme_.clear();
		rme_.clear();

		if(debug) {
			cout << "RPREDS_FINAL::: ";
			print_vector(rpreds);
		}

		if(has_uninstantiated(rpreds)) {
			return 0;
		}
		return w;
	}

	return 0;
}

static DrtVect delete_elements(const DrtVect &drtvect, const DrtVect &rhs)
{
	DrtVect to_return;
	for(int n = 0 ; n < drtvect.size(); ++n) {
		if(!shortfind(rhs,drtvect.at(n)) )
		   to_return.push_back(drtvect.at(n) );
	}
		
	return to_return;
}

static DrtVect clean_after_colon(DrtVect drtvect)
{
	for(int n=0; n < drtvect.size(); ++n) {
		string header = extract_header(drtvect.at(n));
		header = header.substr(0,header.find(':'));
		implant_header(drtvect.at(n),header);
	}

	return drtvect;
}

DrtVect Match::substituteAllWithRule(DrtVect drtvect, const pair<DrtVect,DrtVect> &rule)
{
	DrtVect to_return;

	DrtVect hyp= rule.second;

	if (debug) {
		puts("BBBBBBB");
		print_vector(drtvect);
		print_vector(hyp);
		puts("BBBBBBB2");
	}


	MatchSubstitutions msubs;
	DrtMgu negated_upg;
	DrtVect clean_drtvect = clean_after_colon(drtvect);
	double w= this->singlePhraseMatch(clean_drtvect,hyp,&msubs);
	if(w < 0.1) {
		if(debug) {
			cout << "W_PRE_INV::: " << w << endl;
		}
		vector<DrtPred> inverted = invert_TO_BE(clean_drtvect);
		w= this->phraseMatchNames(inverted,hyp,&msubs,negated_upg);
		if(w < 0.1) {
			return drtvect;
		}
	}	
	DrtMgu mgu = msubs.getDrtMgu();
	hyp/mgu;    
	to_return= delete_elements(drtvect,hyp);
	DrtVect cons= rule.first;
	cons/mgu;
	to_return.insert(to_return.begin(),cons.begin(),cons.end() );

	return to_return;
}


vector<string> Match::getAlternativeVerbs(const vector<string> &subj, const string &verb, const vector<string> &obj)
{
	vector<string> to_return;
	metric *d = metric_singleton::get_metric_instance();

	if (subj.size()) {
		string subj_str = subj.at(0);
		if (!d->has_synset(subj_str)) { // if there is no such name it is a person's name /// BAD SOLUTION!!!
			subj_str = "person";
		}
		double event_dist = d->jico_dist(subj_str, "event", hyp_height_);
		double happening_dist = d->jico_dist(subj_str, "happening", hyp_height_);
		double person_dist = d->jico_dist(subj_str, "person", hyp_height_);
		double man_dist = d->jico_dist(subj_str, "man", hyp_height_);
		double woman_dist = d->jico_dist(subj_str, "woman", hyp_height_);
		double object_dist = d->jico_dist(subj_str, "object", hyp_height_);
		double animal_dist = d->jico_dist(subj_str, "animal", hyp_height_);
		double corporation_dist = d->jico_dist(subj_str, "corporation", hyp_height_);

		if (person_dist > 0.8 || man_dist > 0.8 || woman_dist > 0.8 || object_dist > 0.8 || animal_dist > 0.8
				|| corporation_dist > 0.8) {
			to_return.push_back("do");
		}
		if (person_dist > 0.8 || man_dist > 0.8 || woman_dist > 0.8 || object_dist > 0.8 || animal_dist > 0.8
				|| corporation_dist > 0.8 || event_dist > 0.8 || happening_dist > 0.8) {
			to_return.push_back("happen");
		}
	}

	return to_return;
}

namespace matching {
bool is_what(const string &str)
{
	Knowledge k;
	metric *d = metric_singleton::get_metric_instance();
	if (get_string_vector_distance(d, str, get_what(), &k, 5) > 0.1)
		return true;
	return false;
}
bool is_who(const string &str)
{
	Knowledge k;
	metric *d = metric_singleton::get_metric_instance();
	if (get_string_vector_distance(d, str, "person|[noun.person]|man|team|professional|politician|president|emperor", &k, 5) > 0.1)
		return true;
	return false;
}
}


MatchInfo::MatchInfo()
:	only_names_(false), match_negative_(true), hypernym_height_(6), inverted_person_(false)
{
	;
}
bool MatchInfo::getOnlyNames()
{
	return only_names_;
}
bool MatchInfo::getMatchNegative()
{
	return match_negative_;
}
bool MatchInfo::getInvertedPerson()
{
	return inverted_person_;
}
int MatchInfo::getHypernymHeight()
{
	return hypernym_height_;
}

void MatchInfo::setOnlyNames(bool value)
{
	only_names_= value;
}
void MatchInfo::setMatchNegative (bool value)
{
	match_negative_= value;
}
void MatchInfo::setInvertedPerson(bool value)
{
	inverted_person_= value;
}
void MatchInfo::setHypernymHeight(int value)
{
	hypernym_height_= value;
}


