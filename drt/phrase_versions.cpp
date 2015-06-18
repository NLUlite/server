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



#include"phrase_versions.hpp"

const bool debug = false;

template<class T>
static bool shortfind(const vector<T> &vect, const T &element)
{

	if (find(vect.begin(), vect.end(), element) == vect.end()) {
		return false;
	}
	return true;
}

static bool shortfind_phrase(vector<pair<phrase, double> > &vect, pair<phrase, double> &element)
{

	if (find(vect.begin(), vect.end(), element) == vect.end())
		return false;

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

static bool shortfind_tagged(const vector<vector<FuzzyPred> > &vect, const vector<FuzzyPred> &element)
// only headers are compared
{
	for (int n = 0; n < vect.size(); ++n) {
		vector<FuzzyPred> fpreds = vect.at(n);
		if (fpreds.size() != element.size())
			continue;
		int m = 0;
		for (; m < fpreds.size(); ++m) {
			string lhs_header = extract_header(fpreds.at(m));
			string rhs_header = extract_header(element.at(m));
			if (lhs_header != rhs_header)
				break;
		}
		if (m == fpreds.size())
			return true;
	}

	return false;
}

static int count_verbs(const DrtVect &drt)
{
	int num_verbs = 0;
	for (int n = 0; n < drt.size(); ++n) {
		if (drt.at(n).is_verb())
			++num_verbs;
	}
	return num_verbs;
}

static vector<DrtPred> get_not_connected(const DrtVect &orig_drt, const vector<DrtVect> &connected)
{
	DrtVect not_connected, connected_list;

	for (int m = 0; m < connected.size(); ++m) {
		connected_list.insert(connected_list.end(), connected.at(m).begin(), connected.at(m).end());
	}
	for (int n = 0; n < orig_drt.size(); ++n) {
		if (!shortfind(connected_list, orig_drt.at(n)) && !orig_drt.at(n).is_complement()) {
			not_connected.push_back(orig_drt.at(n));
		}
	}

	if (debug) {
		std::cout << "UNCONNECTED:: " << std::endl;
		print_vector(not_connected);
	}

	return not_connected;
}

static bool compare_phrases(const pair<phrase, double> &lhs, const pair<phrase, double> &rhs)
{
	double left, right;
	left = lhs.first.get_likeliness();
	right = rhs.first.get_likeliness();

	// The sentence with more connected elements should win
	DrtVect ldrt = lhs.first.get_drt();
	DrtVect rdrt = rhs.first.get_drt();

	vector<DrtVect> lconn = get_linked_drtvect_from_single_drtvect(ldrt);
	vector<DrtVect> rconn = get_linked_drtvect_from_single_drtvect(rdrt);

	vector<DrtPred> lnc = get_not_connected(ldrt, lconn);
	vector<DrtPred> rnc = get_not_connected(rdrt, rconn);
	int lsize = ldrt.size() - lnc.size();
	int rsize = rdrt.size() - rnc.size();
	if (rsize > 0) {
		left  += log(lsize);
	}
	if (rsize > 0) {
		right += log(rsize);
	}
	if (left == right) {
		int num_verb_left = count_verbs(ldrt);
		int num_verb_right = count_verbs(rdrt);
		left  += log(num_verb_left);
		right += log(num_verb_right);
	}

	return left > right;
}

static vector<FuzzyPred> clear_tags(vector<FuzzyPred> to_return)
{
	for (int n = 0; n < to_return.size(); ++n) {
		string tag = to_return.at(n).pred().begin()->str;
		string word = to_return.at(n).pred().begin()->str;
		if (tag == "VBZ" && word == "life")
			to_return.at(n).pred().begin()->str = "live";
	}

	return to_return;
}

phrase_versions::phrase_versions()
{
}

static inline int min(int a, int b)
{
	return a < b ? a : b;
}

phrase_versions::phrase_versions(const string &str, PhraseInfo *pi, Context *c, WisdomInfo wi)
{
	context_ = c;
	text_ = str; // save the text
	phrase_info_ = pi;
	wi_ = wi;

	metric *d = metric_singleton::get_metric_instance();
	tagger_info *info(parser_singleton::get_tagger_info_instance());
	tagger tagger_(info);
	parser_info *pinfo = parser_singleton::get_parser_info_instance();
	parser parser_(info, pinfo);
	string tmp_str(str);  /// make const string in set_phrase
	tagger_.setWisdomInfo(wi_);
	tagger_.set_phrase(tmp_str);
	tagger_.setContext(context_);
	tagger_.do_tag();

	// Cycle until all the triggers are satisfied
	vector<FuzzyPred> tagged = tagger_.get_tagged();
	parser_.clear();
	parser_.setTags(tagged);
	parser_.do_parse();
	parsed_versions_ = parser_.get_parsed();

	if (parsed_versions_.size() == 0) /// If the parser fails then the phrase should not be parsed
		throw(std::runtime_error(string("Incomprehensible: " + text_ + ".")));

	vector<pair<FuzzyPred, double> >::iterator piter = parsed_versions_.begin();
	vector<pair<FuzzyPred, double> >::iterator pend = parsed_versions_.end();

	for (; piter != pend; ++piter) {
		phrase_versions_.push_back(make_pair(phrase(piter->first, phrase_info_), piter->second));
	}
	phrase first_phrase = phrase_versions_.at(0).first;

	Triggers triggers;
	//triggers.process(first_phrase.get_orig_drt(),tagged,first_phrase.getParsed() );
	int n = 0;
	int max_cycles = min(tagged.size(), 7);

	vector<vector<FuzzyPred> > already_parsed;
	already_parsed.push_back(tagged);
	vector<FuzzyPred> already_new_tree;
	vector<vector<int> > already_noPRN, already_noSBAR;
	vector<pair<phrase, double> > phrase_versions_searchlist = phrase_versions_;
	already_new_tree.push_back(parsed_versions_.at(0).first);
	phrase_versions_.clear();

	while (phrase_versions_searchlist.size() && n < max_cycles) {
		if (debug)
			std::cout << "PHRASE_ITERATION0" << n << endl;

		pair<phrase, double> first_phrase_element = phrase_versions_searchlist.front();
		if (shortfind_phrase(phrase_versions_, first_phrase_element)) {

			phrase_versions_searchlist.erase(phrase_versions_searchlist.begin());
			++n;
			continue; // no point in processing something already processed
		}
		first_phrase = first_phrase_element.first;
		tagged = first_phrase.get_tag_preds();
		triggers.process(first_phrase.get_orig_drt(), tagged, first_phrase.getParsed(), first_phrase.getError() );
		vector<vector<FuzzyPred> > new_tags = triggers.getCorrectedTags();
		vector<int> noPRN = triggers.getForbiddenPRN();
		vector<int> noSBAR = triggers.getForbiddenSBAR();
		FuzzyPred new_tree = triggers.getParsedTree();
		if (debug)
			std::cout << "PHRASE_ITERATION01" << n << endl;

		for (int m = 0; m < new_tags.size(); ++m) {
			if (debug)
				std::cout << "PHRASE_ITERATION02" << n << endl;
			vector<FuzzyPred> cleared_tags = clear_tags(new_tags.at(m));
			if (debug)
				std::cout << "PHRASE_ITERATION03" << n << endl;
			if (shortfind_tagged(already_parsed, cleared_tags))
				continue;
			if (debug)
				std::cout << "PHRASE_ITERATION04" << n << endl;

			if (debug) {
				std::cerr << "TAGS:::" << endl;
			}
			parser_.clear(); // clear the previous parser constraints
			parser_.setTags(cleared_tags);
			parser_.do_parse();
			vector<pair<FuzzyPred, double> > parsed_versions2 = parser_.get_parsed();
			already_parsed.push_back(cleared_tags);
			if (parsed_versions2.size() == 0 || shortfind(already_new_tree, parsed_versions2.at(0).first)) {
				//++n;
				break;
			}
			try {
				already_new_tree.push_back(parsed_versions2.at(0).first);
				phrase another_phrase(parsed_versions2.at(0).first, phrase_info_);
				phrase_versions_searchlist.push_back(make_pair(another_phrase, parsed_versions2.at(0).second));
			} catch (...) {
				///
			}
		}
		if (debug)
			std::cout << "PHRASE_ITERATION05" << n << endl;
		if (noPRN.size() && !shortfind(already_noPRN, noPRN)) {
			if (debug) {
				std::cerr << "NO_PRN:::" << endl;
			}
			parser_.clear(); // clear the previous parser constraints
			parser_.setForbiddenPRN(noPRN);
			parser_.setTags(tagged);
			parser_.do_parse();
			vector<pair<FuzzyPred, double> > parsed_versions2 = parser_.get_parsed();
			already_noPRN.push_back(noPRN);
			if (parsed_versions2.size() && !shortfind(already_new_tree, parsed_versions2.front().first)) {
				already_new_tree.push_back(parsed_versions2.front().first);
				try {

					phrase another_phrase(parsed_versions2.front().first, phrase_info_);
					phrase_versions_searchlist.push_back(make_pair(another_phrase, parsed_versions2.front().second));
				} catch (...) {
					///
				}
			}
		}
		if (noSBAR.size() && !shortfind(already_noSBAR, noSBAR)) {
			if (debug) {
				std::cerr << "NO_SBAR:::" << endl;
			}
			parser_.clear(); // clear the previous parser constraints
			parser_.setForbiddenSBAR(noSBAR);
			parser_.setTags(tagged);
			parser_.do_parse();
			vector<pair<FuzzyPred, double> > parsed_versions2 = parser_.get_parsed();
			already_noSBAR.push_back(noSBAR);
			if (parsed_versions2.size() != 0 && !shortfind(already_new_tree, parsed_versions2.at(0).first)) {
				already_new_tree.push_back(parsed_versions2.at(0).first);
				try {
					phrase another_phrase(parsed_versions2.at(0).first, phrase_info_);
					phrase_versions_searchlist.push_back(make_pair(another_phrase, parsed_versions2.at(0).second));
				} catch (...) {
					///
				}
			}
		}
		if (!(new_tree == FuzzyPred()) && !shortfind(already_new_tree, new_tree)) {
			if (debug) {
				std::cerr << "NEW_TREE:::" << endl;
			}
			already_new_tree.push_back(new_tree);
			try {
				phrase another_phrase(new_tree, phrase_info_);
				phrase_versions_searchlist.push_back(make_pair(another_phrase, 1));
			} catch (...) {
				///
			}
		}
		++n;
		if (debug)
			std::cout << "PHRASE_ITERATION" << n << " " << phrase_versions_searchlist.size() << endl;
		phrase_versions_.push_back(
				make_pair(phrase_versions_searchlist.front().first, phrase_versions_searchlist.front().second));
		sort(phrase_versions_searchlist.begin(), phrase_versions_searchlist.end(), compare_phrases);
	}
	parser_.clear(); // clear the previous parser constraints
	sort(phrase_versions_.begin(), phrase_versions_.end(), compare_phrases);
	if (phrase_versions_.size() == 0) {
		phrase tmp_phrase(FuzzyPred("ROOT(S(N(@incomprehensible)))"), phrase_info_);
		phrase_versions_.push_back(make_pair(tmp_phrase, 0));
	}
}

vector<DrtPred> phrase_versions::get_most_likely_drt() const
{
	if (phrase_versions_.size() > 0)
		return phrase_versions_.at(0).first.get_drt();
	return vector<DrtPred>(1, DrtPred("@incomprehensible(ref0)"));
}
phrase phrase_versions::get_most_likely_phrase() const
{
	if (phrase_versions_.size() > 0)
		return phrase_versions_.at(0).first;
	return phrase(FuzzyPred("ROOT(S(N(@incomprehensible)))"), phrase_info_);
}
