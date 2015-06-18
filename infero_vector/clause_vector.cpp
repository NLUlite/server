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



#include"clause_vector.hpp"

using std::string;
using std::cout;
using std::endl;

std::ostream & operator <<(std::ostream &out, const clause_vector& c) {
	c.print_stream(out);
	return out;
}

clause_vector::clause_vector(const string &s) :
		has_code_(false), has_match_code_(false) {
	string str = s;
	boost::erase_all(str, string(" ")); // strip all the spaces from the string
	vector<string> strs, strings;
	boost::split(strings, str, boost::is_any_of(":"));
	if (strings.size() != 2)
		throw(std::invalid_argument("Badly formed clause: " + s));
	strings.at(1).erase(0, 1); // erase the '-'

	// saving the consequence
	str = strings.at(0);
	strs.clear();
	int p, size = str.size();
	int depth = 0;
	int p1 = 0, p2;
	for (p = 0; p < size; ++p) {
		if (str.at(p) == '(')
			++depth;
		if (str.at(p) == ')')
			--depth;
		if (depth < 0)
			throw(std::invalid_argument("Badly formed clause."));
		if (str.at(p) == ',' && depth == 0) {
			p2 = p;
			strs.push_back(str.substr(p1, p2 - p1));
			p1 = p2 + 1;
		}
	}
	strs.push_back(str.substr(p1, p - p1));

	vector<string>::iterator si = strs.begin();
	vector<string>::iterator se = strs.end();
	consequence.resize(strs.size());
	vector<DrtPred>::iterator hi = consequence.begin();
	for (; si != se; ++si, ++hi) {
		//      std::cout << (*si) << std::endl;
		*hi = DrtPred(*si);
	}

	// saving the hypothesis
	str = strings.at(1);
	vector<string> hyps;
	boost::split(hyps, str, boost::is_any_of("&"));
	vector<string>::iterator hypsiter = hyps.begin();
	vector<string>::iterator hypsend = hyps.end();
	for (; hypsiter != hypsend; ++hypsiter) {
		strs.clear();
		size = hypsiter->size();
		depth = 0;
		p1 = 0;
		for (p = 0; p < size; ++p) {
			if (hypsiter->at(p) == '(')
				++depth;
			if (hypsiter->at(p) == ')')
				--depth;
			if (depth < 0)
				throw(std::invalid_argument("Badly formed clause."));
			if (hypsiter->at(p) == ',' && depth == 0) {
				p2 = p;
				strs.push_back(hypsiter->substr(p1, p2 - p1));
				p1 = p2 + 1;
			}
		}
		strs.push_back(hypsiter->substr(p1, p - p1));

		si = strs.begin();
		se = strs.end();
		vector<DrtPred> hyp;
		hyp.resize(strs.size());
		hi = hyp.begin();
		for (; si != se; ++si, ++hi) {
			//      std::cout << (*si) << std::endl;
			*hi = DrtPred(*si);
		}
		hypothesis.push_back(hyp);
	}

	setWeigth(1);
}

void clause_vector::operator/(const DrtMgu &rhs) {
	upg.clear();
	upg = rhs;
	consequence / upg;
	vector<vector<DrtPred> >::iterator hi = hypothesis.begin();
	for (; hi != hypothesis.end(); ++hi) {
		*hi / upg;
	}
}

void clause_vector::setDrtMgu(const DrtMgu &rhs) {
	upg.clear();
	upg = rhs;
}

void clause_vector::uniVal(const int &value) {

	vector<DrtPred>::iterator si = consequence.begin();
	vector<DrtPred>::iterator se = consequence.end();
	for (; si != se; ++si)
		si->uniVal(value);

	vector<vector<DrtPred> >::iterator hi = hypothesis.begin();
	for (; hi != hypothesis.end(); ++hi) {
		si = hi->begin();
		se = hi->end();
		for (; si != se; ++si)
			si->uniVal(value);
	}
}

void clause_vector::print() const {
	vector<DrtPred>::const_iterator consIter;
	vector<DrtPred>::const_iterator endCons = consequence.end();
	for (consIter = consequence.begin(); consIter != endCons; ++consIter) {
		std::cout << *consIter;
		if (boost::next(consIter) != endCons)
			std::cout << ", ";
	}
	std::cout << " :- ";
	vector<vector<DrtPred> >::const_iterator hi = hypothesis.begin();
	for (; hi != hypothesis.end(); ++hi) {
		vector<DrtPred>::const_iterator hypIter;
		vector<DrtPred>::const_iterator endHyp = hi->end();
		for (hypIter = hi->begin(); hypIter != endHyp; ++hypIter) {
			std::cout << *hypIter;
			if (boost::next(hypIter) != endHyp)
				std::cout << ", ";
		}
		if (boost::next(hi) != hypothesis.end())
			std::cout << " & ";
	}
	std::cout << "." << std::endl;
}

void clause_vector::print_stream(std::ostream &out) const {

	vector<DrtPred>::const_iterator consIter;
	vector<DrtPred>::const_iterator endCons = consequence.end();
	for (consIter = consequence.begin(); consIter != endCons; ++consIter) {
		//out << *consIter;
		consIter->print(out);
		if (boost::next(consIter) != endCons)
			out << ", ";
	}
	out << " :- ";
	vector<vector<DrtPred> >::const_iterator hi = hypothesis.begin();
	for (; hi != hypothesis.end(); ++hi) {
		vector<DrtPred>::const_iterator hypIter;
		vector<DrtPred>::const_iterator endHyp = hi->end();
		for (hypIter = hi->begin(); hypIter != endHyp; ++hypIter) {
			out << *hypIter;
			if (boost::next(hypIter) != endHyp)
				out << ", ";
		}
		if (boost::next(hi) != hypothesis.end())
			out << " & ";
	}
}

void set_clauses_unival(vector<clause_vector> *cv, int n) {
	vector<clause_vector>::iterator cviter = cv->begin();
	vector<clause_vector>::iterator cvend = cv->end();

	while (cviter != cvend) {
		cviter->uniVal(n);
		++cviter;
	}
}

void set_clauses_unival(vector<vector<clause_vector> > *cv, int value) {
	for (int n = 0; n < cv->size(); ++n) {
		vector<clause_vector>::iterator cviter = cv->at(n).begin();
		vector<clause_vector>::iterator cvend = cv->at(n).end();

		while (cviter != cvend) {
			cviter->uniVal(value);
			++cviter;
		}
	}
}

static string extrapolate_levin_description(const vector<DrtPred> &preds) {
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
	if (description.size())
		description.erase(description.size() - 1);

	return description;
}

void clause_vector::find_levin() {
	levin_descriptions_hyp_.clear();
	levin_descriptions_cons_.clear();

	levin_descriptions_hyp_.push_back(
			extrapolate_levin_description(hypothesis.at(0)));
	levin_descriptions_cons_.push_back(
			extrapolate_levin_description(consequence));
}

void operator /(std::vector<clause_vector> &predVect, const DrtMgu &upg) {
	std::vector<clause_vector>::iterator predIter;
	std::vector<clause_vector>::iterator endPred = predVect.end();

	for (predIter = predVect.begin(); predIter != endPred; predIter++) {
		(*predIter) / upg;
	}

}

void operator /(std::vector<std::vector<clause_vector> > &predVect,
		const DrtMgu &upg) {
	std::vector<std::vector<clause_vector> >::iterator predIter;
	std::vector<std::vector<clause_vector> >::iterator endPred = predVect.end();

	for (predIter = predVect.begin(); predIter != endPred; predIter++) {
		(*predIter) / upg;
	}

}
