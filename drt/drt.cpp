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



#include"drt.hpp"
#include"../match/Match.hpp"

const bool debug = false;

template<class T>
static bool shortfind(const vector<T> &vect, const T &element)
{
	if (find(vect.begin(), vect.end(), element) == vect.end())
		return false;
	return true;
}

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

bool Priors::hasPrior(const string &str)
{
	if (shortfind(priors_, str))
		return true;

	if (str.find("_") != string::npos) {
		vector<string> strs;
		boost::split(strs, str, boost::is_any_of("_"));
		for (int n = 0; n < strs.size(); ++n)
			if (shortfind(priors_, strs.at(n)))
				return true;
	}
	return false;
}

void Priors::addNoun(const string &str)
{
	priors_.push_back(str);
}

vector<string> Priors::getString()
{
	return priors_;
}

void QuestionList::add(DrtPred d)
{
	qlist_.push_back(d);
}

void QuestionList::add(const vector<DrtPred> &drtvect)
{
	vector<DrtPred>::const_iterator diter = drtvect.begin();
	vector<DrtPred>::const_iterator dend = drtvect.end();

	for (; diter != dend; ++diter) {
		if (debug) {
			cout << "QQUESTION0::: " << extract_header(*diter) << endl;
		}
		if (diter->is_question()) {
			DrtPred tmp_pred(*diter);
			implant_header(tmp_pred, diter->get_question_word());
			tmp_pred.name() = diter->get_question_word();
			if (debug) {
				cout << "QQUESTION::: " << extract_header(*diter) << endl;
				cout << "QQUESTION2::: " << diter->get_question_word() << endl;
			}
			this->add(tmp_pred);
		} else if (diter->is_name()) {
			string header = extract_header(*diter);
			priors_.addNoun(header);
			if (header.find("_") != string::npos) {
				vector<string> strs;
				boost::split(strs, header, boost::is_any_of("_"));
				for (int n = 0; n < strs.size(); ++n)
					priors_.addNoun(strs.at(n));
			}

			if (debug) {
				cout << "QPRIOR::: " << extract_header(*diter) << endl;
			}
		}
	}
}

void QuestionList::operator /(const DrtMgu &upg)
{
	vector<DrtPred>::iterator qiter = qlist_.begin(), qend = qlist_.end();

	for (; qiter != qend; ++qiter) {
		*qiter / upg;
	}
}

void QuestionList::applySubstitutions(MatchSubstitutions &msubs)
{
	msubs.applySubstitutions(&qlist_);
}

void drt::apply_phrase_number(int num)
{
	int size = drt_.size();
	vector<DrtPred> ret_drt(drt_);

	for (int n = 0; n < size; ++n) {
		vector<string> children = ret_drt.at(n).extract_children();
		for (int m = 0; m < children.size(); ++m) {
			string str = children.at(m);
			if (str.size() && isdigit(str.at(str.size() - 1))) {
				try {
					str += string("_") + boost::lexical_cast<string>(num);
					children.at(m) = str;
				} catch (std::exception &e) {
					///
				}
			}
		}
		ret_drt.at(n).implant_children(children);
	}
	drt_ = ret_drt;
}

vector<pair<pair<string, string>, string> > drt::get_references() const
{
	int size = drt_.size();
	vector<pair<pair<string, string>, string> > ret_references;

	for (int n = 0; n < size; ++n) {
		string head_str = extract_header(drt_.at(n));
		string anaphora_level = drt_.at(n).anaphoraLevel();
		string str = extract_first_tag(drt_.at(n));
		if (str.find("ref") == 0) {
			ret_references.push_back(make_pair(make_pair(head_str, anaphora_level), str));
		}
	}
	return ret_references;
}

vector<DrtPred> drt::get_references_with_preds() const
{
	int size = drt_.size();
	vector<DrtPred> ret_references;

	for (int n = 0; n < size; ++n) {
		string head_str = extract_header(drt_.at(n));
		string anaphora_level = drt_.at(n).anaphoraLevel();
		string str = extract_first_tag(drt_.at(n));
		if (str.find("ref") == 0 || drt_.at(n).is_proper_name()) {
			ret_references.push_back(drt_.at(n));
		}
	}
	return ret_references;
}

vector<DrtPred> drt::predicates_with_references() const
{
	vector<DrtPred> predicates(drt_);
	for (int r = 0; r < references_.size(); ++r) {
		string from_str = references_.at(r).second;
		string to_str = references_.at(r).first;

		if (debug) {
			cout << "PREDS_WITH_REFS::: " << from_str << " " << to_str << endl;
		}

		for (int n = 0; n < predicates.size(); ++n) {
			vector<string> children = predicates.at(n).extract_children();
			bool is_anaphora = false;
			bool make_substitution = true;
			// if a substitution gives two identical children, the substitution is not made
			for (int m = 0; m < children.size(); ++m) {
				if (debug) {
					cout << "PREDS_WITH_REFS2::: " << children.at(m) << endl;
				}
				if (children.at(m) == to_str) {
					make_substitution = false;
					break;
				}
				if (children.at(m) == from_str) {
					children.at(m) = to_str;
					is_anaphora= true;
				}
			}
			if (make_substitution) {
				predicates.at(n).implant_children(children);
			}
		}
	}
	return predicates;
}

bool drt::has_donkey() const
{
	vector<DrtPred> predicates(drt_);

	for (int n = 0; n < predicates.size(); ++n) {
		string ptstring = extract_header(predicates.at(n));
		if (ptstring == "@OWN" || ptstring == "@OWNED_BY") {
			string str = extract_first_tag(predicates.at(n));
			if (str.find("|") != string::npos)
				return true;
		}
	}

	return false;
}

vector<string> drt::get_donkey() const
{
	vector<DrtPred> predicates(drt_);
	vector<string> donkeys;

	for (int n = 0; n < predicates.size(); ++n) {
		string ptstring = extract_header(predicates.at(n));
		if (ptstring == "@OWN") {
			string str = extract_first_tag(predicates.at(n));
			if (str.find("|") != string::npos) {
				donkeys.push_back(str);
			}
		}
		if (ptstring == "@OWNED_BY") {
			string str = extract_second_tag(predicates.at(n));
			if (str.find("|") != string::npos) {
				donkeys.push_back(str);
			}
		}
	}

	return donkeys;
}

vector<pair<string, string> > drt::get_donkey_instantiation()
{
	vector<DrtPred> predicates(this->predicates_with_references());
	vector<pair<string, string> > donkeys_ref;

	for (int n = 0; n < predicates.size(); ++n) {
		string ptstring = extract_header(predicates.at(n));
		if (ptstring == "@OWN") {
			string str = extract_first_tag(predicates.at(n));
			if (str.find("|") != string::npos) {
				vector<string> strs;
				boost::split(strs, str, boost::is_any_of("|"));
				if (strs.size() == 2) {
					//string str1= strs.at(0);
					string str2 = strs.at(1);
					if (str2.size()) // the reference "man|" is also valid /// CORRECT THIS!!
						donkeys_ref.push_back(make_pair(str2, str));
				}
			}
		}
		if (ptstring == "@OWNED_BY") {
			string str = extract_second_tag(predicates.at(n));
			if (str.find("|") != string::npos) {
				vector<string> strs;
				boost::split(strs, str, boost::is_any_of("|"));
				if (strs.size() == 2) {
					//string str1= strs.at(0);
					string str2 = strs.at(1);
					if (str2.size()) // the reference "man|" is also valid /// CORRECT THIS!!
						donkeys_ref.push_back(make_pair(str2, str));
				}
			}
		}
	}

	return donkeys_ref;
}

static string extrapolate_levin_description(const vector<DrtPred> &preds)
{
	string description;
	metric *d = metric_singleton::get_metric_instance();

	vector<DrtPred>::const_iterator piter = preds.begin();
	vector<DrtPred>::const_iterator pend = preds.end();
	string lev_tmp;

	for (; piter != pend; ++piter) {
		string head_str = extract_header(*piter);
		if (piter->is_verb()) {
			lev_tmp = d->get_levin_verb(head_str);
			if (lev_tmp.size()) {
				description += lev_tmp;
				description += "-";
			}
		} else {
			lev_tmp = d->get_levin_noun(head_str);
			if (lev_tmp.size()) {
				description += lev_tmp;
				description += "-";
			}
		}
	}
	if (description.size() > 1)
		description.erase(description.size() - 1);

	return description;
}

void drt::find_levin()
{
	vector<DrtPred> predicates(drt_);

	vector<vector<DrtPred> > preds_to_levinize;

	for (int n = 0; n < predicates.size(); ++n) {
		if (predicates.at(n).is_verb()) {
			vector<DrtPred> tmp_lev = find_attached_to_verb(predicates, n);
			drt_sort(tmp_lev);
			preds_to_levinize.push_back(tmp_lev);
		}
	}

	vector<vector<DrtPred> >::iterator lev_iter = preds_to_levinize.begin();
	vector<vector<DrtPred> >::iterator lev_end = preds_to_levinize.end();

	levin_descriptions_.clear();

	for (; lev_iter != lev_end; ++lev_iter) {
		levin_descriptions_.push_back(extrapolate_levin_description(*lev_iter));
	}
}

void drt::cleanDrs()
// Clean the Drs for use in the solver
{
	vector<DrtPred> data(drt_);

	vector<DrtPred>::iterator diter = data.begin();
	vector<DrtPred>::iterator dend = data.end();

	for (; diter != dend; ++diter) {
		string name = extract_header(*diter);
		if (name.at(0) == '@') {
			int pos = name.find("[spec]");
			if (pos != string::npos) {
				string tmp_name = name.substr(0, pos);
				implant_header(*diter, tmp_name);
			}
		}
	}
	drt_ = data;
}

void drt::setQuestionList(const QuestionList &ql)
{
	qlist_ = ql;
}
