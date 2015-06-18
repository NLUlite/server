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



#include"Sense.hpp"

const bool debug = false;

template<class T>
static bool shortfind(const vector<T> &vect, const T &element)
{
	if (find(vect.begin(), vect.end(), element) == vect.end())
		return false;
	return true;
}

template<class T>
static bool shortfind(const map<T, int> &mapp, const T &element)
{
	typename map<T, int>::const_iterator miter = mapp.find(element);

	if (miter != mapp.end())
		return true;
	return false;
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

static void print_vector_header_stream(std::stringstream &ff, const std::vector<DrtPred> &vs)
{
	vector<DrtPred>::const_iterator tags_iter = vs.begin();
	while (tags_iter < vs.end()) {
		ff << extract_header(*tags_iter);
		if (boost::next(tags_iter) != vs.end())
			ff << ",";
		++tags_iter;
	}
}

template<class K, class T>
static T get_map_element(boost::unordered_map<K, T> &map_element, const K &key)
{
	T to_return;
	typename boost::unordered_map<K, T>::iterator miter = map_element.find(key);
	if (miter != map_element.end()) {
		to_return = miter->second;
	}
	return to_return;
}

// BEGIN LEVINIZER
static int get_number_from_gloss(string gloss)
{
	vector<string> strs;
	boost::split(strs, gloss, boost::is_any_of(","));
	if (strs.size() == 1)
		return -1;
	return boost::lexical_cast<int>(strs.at(0));
}

static vector<string> get_synonyms_from_gloss(string gloss)
{
	vector<string> to_return;
	vector<string> strs;
	boost::split(strs, gloss, boost::is_any_of(";"));
	if (strs.size() == 1)
		return to_return;
	gloss = strs.at(0);
	boost::split(strs, gloss, boost::is_any_of(","));
	if (strs.size() == 1)
		return to_return;
	gloss = strs.at(1);
	boost::split(strs, gloss, boost::is_any_of(" "));
	return strs;
}

static vector<string> get_verbatims_from_gloss(string gloss)
{
	vector<string> to_return;
	vector<string> strs;
	boost::split(strs, gloss, boost::is_any_of(";"));
	if (strs.size() == 1)
		return to_return;
	for (int n = 1; n < strs.size(); ++n) {
		to_return.push_back(strs.at(n));
	}
	return to_return;
}

static DrtVect get_subj_obj_attached_to_verb(const DrtVect &drtvect, const vector<string> &synonyms)
{
	DrtVect to_return;

	for (int n = 0; n < drtvect.size(); ++n) {
		string head = extract_header(drtvect.at(n));
		if (drtvect.at(n).is_verb() && shortfind(synonyms, head)) {
			string sref = extract_subject(drtvect.at(n));
			string oref = extract_object(drtvect.at(n));
			int possubj = find_pivot_with_string(drtvect, sref);
			int posobj = find_pivot_with_string(drtvect, oref);
			if (possubj != -1)
				to_return.push_back(drtvect.at(possubj));
			if (possubj != -1 || posobj != -1)
				to_return.push_back(drtvect.at(n));
			if (posobj != -1)
				to_return.push_back(drtvect.at(posobj));
		}
	}
	return to_return;
}

static DrtVect get_complements_attached_to_verb(const DrtVect &drtvect, const vector<string> &synonyms)
{
	DrtVect to_return;

	for (int n = 0; n < drtvect.size(); ++n) {
		string head = extract_header(drtvect.at(n));
		if (drtvect.at(n).is_verb() && shortfind(synonyms, head)) {
			vector<DrtVect> complements = find_complements_of_verb(drtvect, n);
			to_return.push_back(drtvect.at(n));
			for (int j = 0; j < complements.size(); ++j) {
				DrtPred first_pred = complements.at(j).at(0);
				string head2 = extract_header(first_pred);
				if (head2 != "@TIME" && head2 != "@MODAL")
					to_return.insert(to_return.end(), complements.at(j).begin(), complements.at(j).end());
			}
		}
	}
	return to_return;
}

static DrtVect get_only_pivots(const DrtVect &drtvect)
{
	DrtVect to_return;
	for (int n = 0; n < drtvect.size(); ++n) {
		if (drtvect.at(n).is_name()) {
			if (drtvect.at(n).is_pivot()) {
				to_return.push_back(drtvect.at(n));
			}
		} else {
			to_return.push_back(drtvect.at(n));
		}
	}
	return to_return;
}

static DrtVect levinize_names(DrtVect drtvect)
{
	metric *d = metric_singleton::get_metric_instance();

	for (int n = 0; n < drtvect.size(); ++n) {
		string head = extract_header(drtvect.at(n));
		string new_head = head;
		if (drtvect.at(n).is_PRP()) {
			drtvect.at(n).setTag("NN");
			new_head = string("[*]");
		} else if (drtvect.at(n).is_WP() || drtvect.at(n).is_WDT()) {
			new_head = "[noun.object]|[noun.person]";
			drtvect.at(n).setTag("NN");
		} else if (drtvect.at(n).is_article()) {
			new_head = "[noun.object]|[noun.person]";
			drtvect.at(n).setTag("NN");
		} else if (drtvect.at(n).is_name() && !drtvect.at(n).is_adjective()) {
			string levin = d->get_levin_noun(head);
			if (levin == "")
				levin = "*";
			new_head = string("[") + levin + "]";
		} else if (drtvect.at(n).is_adjective()) {
			new_head = string("[JJ]");
		}

		extract_header(drtvect.at(n));
		implant_header(drtvect.at(n), new_head);
	}

	return drtvect;
}

static vector<vector<DrtPred> > filter_valid_complements(const vector<vector<DrtPred> > &specs)
{
	vector<vector<DrtPred> > to_return;

	vector<vector<DrtPred> >::const_iterator diter = specs.begin();
	vector<vector<DrtPred> >::const_iterator dend = specs.end();

	for (; diter != dend; ++diter) {
		DrtVect tmp_drt = *diter;
		string head = extract_header(diter->at(0));
		if (head != "@PAR" && head != "@TIME")
			to_return.push_back(*diter);
	}
	return to_return;
}

static vector<DrtPred> only_pivots(const vector<DrtPred> &preds)
{
	vector<DrtPred> to_return;
	for (int n = 0; n < preds.size(); ++n) {
		if (preds.at(n).is_name() && preds.at(n).is_pivot()) {
			to_return.push_back(preds.at(n));
		} else if (!preds.at(n).is_name())
			to_return.push_back(preds.at(n));
	}
	return to_return;
}

static vector<DrtPred> drt_unique(vector<DrtPred> preds)
{
	if (debug) {
		puts("DRT_UNIQUE:::");
		print_vector(preds);
	}

	vector<DrtPred> already_parsed;
	DrtVect to_return;
	for (int n = 0; n < preds.size(); ++n) {
		if (!shortfind(already_parsed, preds.at(n))) {
			to_return.push_back(preds.at(n));
		}
		already_parsed.push_back(preds.at(n));
	}
	return to_return;
}

static bool is_valid_drt(vector<DrtPred> preds)
{
	if (debug) {
		puts("DRT_VALID:::");
		print_vector(preds);
	}

	DrtVect to_return;
	for (int n = 0; n < preds.size(); ++n) {
		string header = extract_header(preds.at(n));
		if (header.find("AUX") != string::npos) {
			return false;
		}
	}
	return true;
}

static vector<DrtVect> get_drtvects_with_no_complements(const DrtVect &drtvect, bool levinize)
// It returns the set of drts that are connected to a specific
// verb. Conjunctions between verbs and @conditions are ignored.
{
	vector<DrtVect> to_return;

	vector<DrtPred>::const_iterator diter = drtvect.begin();
	vector<DrtPred>::const_iterator dend = drtvect.end();

	int n = 0;
	vector<DrtPred> previous_elements;
	//previous_elements.push_back("@DUMMY(dummy)");
	for (; diter != dend; ++diter, ++n) {
		if (diter->is_verb() && find(previous_elements.begin(), previous_elements.end(), *diter) == previous_elements.end()) {
			vector<DrtPred> tmp_drtvect = find_all_attached_to_verb(drtvect, n);
			if (tmp_drtvect.size() == 1)  // not interested in lonely verbs
				continue;
			drt_sort(tmp_drtvect);
			unique(tmp_drtvect.begin(), tmp_drtvect.end());
			MatchWrite mgraph(tmp_drtvect);
			vector<SingleMatchGraph> sentences = mgraph.getSentences();

			vector<SingleMatchGraph>::iterator siter = sentences.begin();
			vector<SingleMatchGraph>::iterator send = sentences.end();

			vector<pair<string, vector<DrtPred> > > sentence_list;
			vector<pair<string, DrtPred> > sentence_preps;
			DrtVect full_drt;
			// pair with the verb reference and the drtvect
			for (; siter != send; ++siter) {
				full_drt.clear();
				DrtPred verb;
				vector<DrtPred> subjects, objects;
				//vector< vector<DrtPred> > complements;
				subjects = only_pivots(siter->getSubject());
				objects = only_pivots(siter->getObject());
				// complements  = filter_valid_complements(siter->getComplements());
				verb = siter->getVerb();
				string verb_name = extract_header(verb);
				full_drt.insert(full_drt.end(), subjects.begin(), subjects.end());
				full_drt.push_back(verb);
				full_drt.insert(full_drt.end(), objects.begin(), objects.end());
				if (levinize)
					full_drt = levinize_names(full_drt);

				full_drt = drt_unique(full_drt);

				if (is_valid_drt(full_drt))
					to_return.push_back(full_drt);
			}
			previous_elements.insert(previous_elements.end(), tmp_drtvect.begin(), tmp_drtvect.end());
		}
	}

	return to_return;
}

static vector<DrtVect> get_drtvects_with_complements(const DrtVect &drtvect, bool levinize)
// It returns the set of drts that are connected to a specific
// verb. Conjunctions between verbs and @conditions are ignored.
{
	vector<DrtVect> to_return;

	vector<DrtPred>::const_iterator diter = drtvect.begin();
	vector<DrtPred>::const_iterator dend = drtvect.end();

	int n = 0;
	vector<DrtPred> previous_elements;
	//previous_elements.push_back("@DUMMY(dummy)");
	for (; diter != dend; ++diter, ++n) {
		if (diter->is_verb() && find(previous_elements.begin(), previous_elements.end(), *diter) == previous_elements.end()) {
			vector<DrtPred> tmp_drtvect = find_all_attached_to_verb(drtvect, n);
			if (tmp_drtvect.size() == 1)  // not interested in lonely verbs
				continue;
			drt_sort(tmp_drtvect);
			unique(tmp_drtvect.begin(), tmp_drtvect.end());
			MatchWrite mgraph(tmp_drtvect);
			vector<SingleMatchGraph> sentences = mgraph.getSentences();

			vector<SingleMatchGraph>::iterator siter = sentences.begin();
			vector<SingleMatchGraph>::iterator send = sentences.end();

			vector<pair<string, vector<DrtPred> > > sentence_list;
			vector<pair<string, DrtPred> > sentence_preps;
			DrtVect full_drt;
			// pair with the verb reference and the drtvect
			for (; siter != send; ++siter) {
				full_drt.clear();
				DrtPred verb;
				vector<DrtPred> subjects, objects;
				vector<vector<DrtPred> > complements;
				subjects = only_pivots(siter->getSubject());
				objects = only_pivots(siter->getObject());
				complements = filter_valid_complements(siter->getComplements());
				verb = siter->getVerb();
				string verb_name = extract_header(verb);
				full_drt.insert(full_drt.end(), subjects.begin(), subjects.end());
				full_drt.push_back(verb);
				full_drt.insert(full_drt.end(), objects.begin(), objects.end());
				for (int j = 0; j < complements.size(); ++j) {
					full_drt.insert(full_drt.end(), complements.at(j).begin(), complements.at(j).end());
				}
				if (levinize)
					full_drt = levinize_names(full_drt);
				full_drt = drt_unique(full_drt);
				to_return.push_back(full_drt);
			}
			previous_elements.insert(previous_elements.end(), tmp_drtvect.begin(), tmp_drtvect.end());
		}
	}

	return to_return;
}

// END LEVINIZER

static string get_composed_lemma(const DrtVect &drtvect, bool levinize)
// the lemma is made by [subject levin], verb, [object levin]
{
	string to_return = "";
	vector<DrtVect> all_drtvect = get_drtvects_with_no_complements(drtvect, levinize);
	if (all_drtvect.size()) {
		stringstream ss;
		print_vector_header_stream(ss, all_drtvect.front());
		to_return = ss.str();
	}

	return to_return;
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

static DrtVect create_asterisk_subjects(DrtVect drtvect)
{
	for(int n=0; n < drtvect.size(); ++n) {
		string fref= extract_first_tag(drtvect.at(n));
		if(is_subject_of_verb(drtvect,fref) ) {
			implant_header(drtvect.at(n),"[*]");
		}
	}
	return drtvect;
}

static vector<string> get_all_composed_lemmas(const DrtVect &drtvect, bool levinize)
// the lemma is made by [subject levin], verb, [object levin]
{
	vector<string> to_return;
	vector<DrtVect> all_drtvect = get_drtvects_with_no_complements(drtvect, levinize);
	for (int n = 0; n < all_drtvect.size(); ++n) {
		stringstream ss;
		print_vector_header_stream(ss, all_drtvect.at(n));
		to_return.push_back(ss.str());
		DrtVect asterisk_subj= create_asterisk_subjects(all_drtvect.at(n));
		if( asterisk_subj != all_drtvect.at(n)) {
			print_vector_header_stream(ss, asterisk_subj);
			to_return.push_back(ss.str());
		}
	}

	return to_return;
}

static string get_composed_lemma_with_complements(const DrtVect &drtvect, bool levinize)
// the lemma is made by [subject levin], verb, [object levin]
{
	string to_return = "";
	vector<DrtVect> all_drtvect = get_drtvects_with_complements(drtvect, levinize);
	if (all_drtvect.size()) {
		stringstream ss;
		print_vector_header_stream(ss, all_drtvect.front());
		to_return = ss.str();
	}

	return to_return;
}

void Sense::loadVerbAgents(const string &s)
{
	std::ifstream file;
	string word;
	int freq;
	int n = 1;
	char line[1000000];
	file.open(s.c_str());
	if (!file.bad()) {
		std::cerr << "Loading sense from \'" << s << "\'." << std::endl;
		while (file.good()) {
			file.getline(line, 1000000);
			vector<string> strs;
			boost::split(strs, line, boost::is_any_of(";"));
			if (strs.size() < 2)
				continue;
			try {
				int lex = boost::lexical_cast<int>(strs.at(0));
				for (int n = 1; n < strs.size(); ++n) {
					DrtVect drtvect = create_drtvect(strs.at(n));
					sense_actors_[lex].push_back(drtvect);
				}
			} catch (std::exception &e) {
			}
		}
	} else
		throw std::length_error(std::string("File") + s + " finished unexpectedly.");
	file.close();
}

void Sense::loadNounsAgents(const string &s)
{
	std::ifstream file;
	string word;
	int freq;
	int n = 1;
	char line[10000];
	file.open(s.c_str());
	if (!file.bad()) {
		std::cerr << "Loading noun sense from \'" << s << "\'." << std::endl;
		while (file.good()) {
			file.getline(line, 10000);
			vector<string> strs;
			boost::split(strs, line, boost::is_any_of(";"));
			if (strs.size() < 2)
				continue;
			try {
				int lex = boost::lexical_cast<int>(strs.at(0));
				for (int n = 1; n < strs.size(); ++n) {
					if (strs.at(n) == "")
						continue;
					DrtVect drtvect = create_drtvect(strs.at(n));
					nouns_actors_[lex].push_back(drtvect);
				}
			} catch (std::exception &e) {
			}
		}
	} else
		throw std::length_error(std::string("File") + s + " finished unexpectedly.");
	file.close();
}

void Sense::loadImpossible(const string &s)
{
	std::ifstream file;
	string word;
	int freq;
	int n = 1;
	char line[10000];
	file.open(s.c_str());
	if (!file.bad()) {
		std::cerr << "Loading impossible sense from \'" << s << "\'." << std::endl;
		while (file.good()) {
			file.getline(line, 10000);
			vector<string> strs;
			boost::split(strs, line, boost::is_any_of(";"));
			if (strs.size() < 2)
				continue;
			try {
				int lex = boost::lexical_cast<int>(strs.at(0));
				for (int n = 1; n < strs.size(); ++n) {
					DrtVect drtvect = create_drtvect(strs.at(n));
					sense_impossible_[lex].push_back(drtvect);
				}
			} catch (std::exception &e) {
			}
		}
	} else
		throw std::length_error(std::string("File") + s + " finished unexpectedly.");
	file.close();
}

void Sense::loadVerbComplements(const string &s)
{
	std::ifstream file;
	string word;
	int freq;
	int n = 1;
	char line[10000];
	file.open(s.c_str());
	if (!file.bad()) {
		std::cerr << "Loading sense from \'" << s << "\'." << std::endl;
		while (file.good()) {
			file.getline(line, 10000);
			vector<string> strs;
			boost::split(strs, line, boost::is_any_of(";"));
			if (strs.size() < 2)
				continue;
			try {
				int lex = boost::lexical_cast<int>(strs.at(0));
				for (int n = 1; n < strs.size(); ++n) {
					DrtVect drtvect = create_drtvect(strs.at(n));
					sense_complements_[lex].push_back(drtvect);
				}
			} catch (std::exception &e) {
			}
		}
	} else
		throw std::length_error(std::string("File") + s + " finished unexpectedly.");
	file.close();
}

void Sense::loadNounsComplements(const string &s)
{
	std::ifstream file;
	string word;
	int freq;
	int n = 1;
	char line[10000];
	file.open(s.c_str());
	if (!file.bad()) {
		std::cerr << "Loading sense from \'" << s << "\'." << std::endl;
		while (file.good()) {
			file.getline(line, 10000);
			vector<string> strs;
			boost::split(strs, line, boost::is_any_of(";"));
			if (strs.size() < 2)
				continue;
			try {
				int lex = boost::lexical_cast<int>(strs.at(0));
				for (int n = 1; n < strs.size(); ++n) {
					DrtVect drtvect = create_drtvect(strs.at(n));
					nouns_complements_[lex].push_back(drtvect);
				}
			} catch (std::exception &e) {
			}
		}
	} else
		throw std::length_error(std::string("File") + s + " finished unexpectedly.");
	file.close();
}

void Sense::loadVerbCombined(const string &s)
{
	std::ifstream file;
	string word;
	int freq;
	int n = 1;
	char line[10000];
	file.open(s.c_str());
	if (!file.bad()) {
		std::cerr << "Loading combined sense from \'" << s << "\'." << std::endl;
		while (file.good()) {
			file.getline(line, 10000);
			vector<string> strs;
			boost::split(strs, line, boost::is_any_of(";"));
			if (strs.size() < 4)
				continue;
			try {
				DrtVect drtvect = create_drtvect(strs.at(1));
				bool levinize = false;
				string lemma = get_composed_lemma(drtvect, levinize);
				double w = boost::lexical_cast<double>(strs.at(2));
				int num = boost::lexical_cast<int>(strs.at(3));
				sense_combined_[lemma].push_back(drtvect);
				std::stringstream ss;
				print_vector_stream(ss, drtvect);
				sense_weight_[ss.str()] = make_pair(w, num);

				if (debug) {
					puts("VERB_COMB:::");
					cout << lemma << " " << ss.str() << endl;
					print_vector(drtvect);
				}

			} catch (std::exception &e) {
			}
		}
	} else
		throw std::length_error(std::string("File") + s + " finished unexpectedly.");
	file.close();
}

void Sense::loadNounsCombined(const string &s)
///
{
	std::ifstream file;
	string word;
	int freq;
	int n = 1;
	char line[10000];
	file.open(s.c_str());
	if (!file.bad()) {
		std::cerr << "Loading combined sense from \'" << s << "\'." << std::endl;
		while (file.good()) {
			file.getline(line, 10000);
			vector<string> strs;
			boost::split(strs, line, boost::is_any_of(";"));
			if (strs.size() < 4)
				continue;
			try {
				DrtVect drtvect = create_drtvect(strs.at(1));
				bool levinize = false;
				string lemma = get_composed_lemma(drtvect, levinize);
				double w = boost::lexical_cast<double>(strs.at(2));
				int num = boost::lexical_cast<int>(strs.at(3));
				nouns_combined_[lemma].push_back(drtvect);
				std::stringstream ss;
				print_vector_stream(ss, drtvect);
				nouns_weight_[ss.str()] = make_pair(w, num);

			} catch (std::exception &e) {
			}
		}
	} else
		throw std::length_error(std::string("File") + s + " finished unexpectedly.");
	file.close();
}

void Sense::loadNegative(const string &s)
{
	std::ifstream file;
	string word;
	int freq;
	int n = 1;
	char line[10000];
	file.open(s.c_str());
	if (!file.bad()) {
		std::cerr << "Loading sense from \'" << s << "\'." << std::endl;
		while (file.good()) {
			file.getline(line, 10000);
			vector<string> strs;
			boost::split(strs, line, boost::is_any_of(";"));
			if (strs.size() < 2)
				continue;
			try {
				int lex = boost::lexical_cast<int>(strs.at(0));
				for (int n = 1; n < strs.size(); ++n) {
					DrtVect drtvect = create_drtvect(strs.at(n));
					sense_negative_[lex].push_back(drtvect);
				}
			} catch (std::exception &e) {
			}
		}
	} else
		throw std::length_error(std::string("File") + s + " finished unexpectedly.");
	file.close();
}

void Sense::loadCombinedNegative(const string &s)
{
	std::ifstream file;
	string word;
	int freq;
	int n = 1;
	char line[10000];
	file.open(s.c_str());
	if (!file.bad()) {
		std::cerr << "Loading sense from \'" << s << "\'." << std::endl;
		while (file.good()) {
			file.getline(line, 10000);
			vector<string> strs;
			boost::split(strs, line, boost::is_any_of(";"));
			if (strs.size() < 4)
				continue;
			try {
				DrtVect drtvect = create_drtvect(strs.at(1));
				// string lemma= strs.at(0);
				bool levinize = false;
				string lemma = get_composed_lemma(drtvect, levinize);
				double w = boost::lexical_cast<double>(strs.at(2));
				int num = boost::lexical_cast<int>(strs.at(3));
				sense_negative_combined_[lemma].push_back(drtvect);
				std::stringstream ss;
				print_vector_stream(ss, drtvect);
				sense_negative_weight_[ss.str()] = make_pair(w, num);
			} catch (std::exception &e) {
			}
		}
	} else
		throw std::length_error(std::string("File ") + s + " finished unexpectedly.");
	file.close();
}

static vector<pair<string, DrtVect> > find_candidate_verbs(const DrtVect &drtvect)
// Find the candidates from the verbs in drtvect
{
	vector<pair<string, DrtVect> > candidate_verbs;
	for (int n = 0; n < drtvect.size(); ++n) {
		if (drtvect.at(n).is_verb()) {
			string header = extract_header(drtvect.at(n));
			DrtVect dvect = find_all_attached_to_verb(drtvect, n);
			header = header.substr(0, header.find(":"));
			candidate_verbs.push_back(make_pair(header, dvect));
			if (debug) {
				puts("CANDIDATE_VERB");
				print_vector(dvect);
			}
		}
	}
	return candidate_verbs;
}

static vector<vector<DrtPred> > find_specifications(const vector<DrtPred> &preds, int m)
{
	vector<vector<DrtPred> > cons;

	if (m < 0 || m >= preds.size())
		return cons;
	if (!preds.at(m).is_name())  // check the verb is a verb
		return cons;
	DrtPred name_cons = preds.at(m);
	string name_ref = extract_first_tag(name_cons);
	vector<DrtPred> connected_compl;

	connected_compl = get_predicates_from_position(preds, find_all_element_with_string(preds, name_ref));
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
				if (!connected_names_tmp.at(m).is_complement())
					connected_names.push_back(connected_names_tmp.at(m));
			}
			connected_names.insert(connected_names.begin(), connected_compl.at(n));
			cons.push_back(connected_names);
		}
	}

	return cons;
}

static DrtVect levinize_names_with_adj(DrtVect drtvect, const string &name)
{
	metric *d = metric_singleton::get_metric_instance();

	for (int n = 0; n < drtvect.size(); ++n) {
		string head = extract_header(drtvect.at(n));
		string new_head = head;
		if (drtvect.at(n).is_pivot() && head == name) {
			// do nothing
		} else if (drtvect.at(n).is_PRP()) {
			drtvect.at(n).setTag("NN");
			new_head = string("[*]");
		} else if (drtvect.at(n).is_WP() || drtvect.at(n).is_WDT()) {
			new_head = "[noun.object]|[noun.person]";
			drtvect.at(n).setTag("NN");
		} else if (drtvect.at(n).is_article()) {
			new_head = "[noun.object]|[noun.person]";
			drtvect.at(n).setTag("NN");
		} else if (drtvect.at(n).is_adjective()) {
			new_head = "[JJ]";
			//drtvect.at(n).setTag("JJ");
		} else if (drtvect.at(n).is_name()) {
			string levin = d->get_levin_noun(head);
			if (levin == "")
				levin = "*";
			new_head = string("[") + levin + "]";
		}
		implant_header(drtvect.at(n), new_head);
		drtvect.at(n).name() = new_head;
	}

	return drtvect;
}

static vector<DrtVect> get_nouns_from_drtvect(const DrtVect &drtvect, string ref)
// It returns the set of nouns with lemma=name, with adjectives and specifications
{
	vector<DrtVect> to_return;

	vector<DrtPred>::const_iterator diter = drtvect.begin();
	vector<DrtPred>::const_iterator dend = drtvect.end();

	int n = 0;
	for (; diter != dend; ++diter, ++n) {
		if (diter->is_name() && diter->is_pivot()) {
			string fref = extract_first_tag(*diter);
			string name = extract_header(*diter);
			if (fref != ref)
				continue;
			DrtVect name_drt = get_predicates_from_position(drtvect, find_all_names_with_string_no_delete(drtvect, fref));
			vector<DrtVect> specs = find_specifications(drtvect, n);
			for (int m = 0; m < specs.size(); ++m) {
				name_drt.insert(name_drt.end(), specs.at(m).begin(), specs.at(m).end());
			}
			name_drt = levinize_names_with_adj(name_drt, name);
			to_return.push_back(name_drt);
		}
	}

	return to_return;
}

static vector<pair<string, DrtVect> > find_candidate_nouns(const DrtVect &drtvect)
// returns all the names with their adjectives and specifications
{
	vector<pair<string, DrtVect> > candidate_verbs;
	for (int n = 0; n < drtvect.size(); ++n) {
		if (drtvect.at(n).is_name() && drtvect.at(n).is_pivot()) {
			string fref = extract_first_tag(drtvect.at(n));
			string header = extract_header(drtvect.at(n));
			vector<DrtVect> vdrt = get_nouns_from_drtvect(drtvect, fref);
			for (int j = 0; j < vdrt.size(); ++j) {
				candidate_verbs.push_back(make_pair(header, vdrt.at(j)));
				stringstream ss;
				if (debug) {
					print_vector_stream(ss, vdrt.at(j));
					std::cerr << ss.str() << endl;
				}
			}
		}
	}
	return candidate_verbs;
}

static vector<int> find_candidate_lexnames(const vector<string> &cverbs)
// Find the candidates from the verbs in drtvect
{
	vector<int> lexs;
	metric *d = metric_singleton::get_metric_instance();
	for (int n = 0; n < cverbs.size(); ++n) {
		vector<int> lex_tmp = d->verb_lexnames(cverbs.at(n));
		if (debug) {
			cout << "FIND_CANDIDATE_LEX:::" << cverbs.at(n) << endl;
			print_vector(lex_tmp);
		}
		lexs.insert(lexs.end(), lex_tmp.begin(), lex_tmp.end());
	}
	return lexs;
}

static vector<int> find_candidate_noun_lexnames(const string &name)
// Find the candidates from the noun
{
	vector<int> lexs;
	metric *d = metric_singleton::get_metric_instance();
	vector<int> lex_tmp = d->noun_lexnames(name);
	lexs.insert(lexs.end(), lex_tmp.begin(), lex_tmp.end());
	return lexs;
}

vector<DrtVect> Sense::getCandidateDrtVect(int lexname)
{
	vector<DrtVect> to_return;
	MapIntDrt::iterator miter = sense_actors_.find(lexname);
	if (miter != sense_actors_.end()) {
		to_return = miter->second;
	}
	miter = sense_complements_.find(lexname);
	if (miter != sense_complements_.end()) {
		to_return.insert(to_return.end(), miter->second.begin(), miter->second.end());
	}
	return to_return;
}

vector<DrtVect> Sense::getCandidateNounsDrtVect(int lexname)
{
	vector<DrtVect> to_return;
	MapIntDrt::iterator miter = nouns_actors_.find(lexname);
	if (miter != nouns_actors_.end()) {
		to_return = miter->second;
	}
	miter = nouns_complements_.find(lexname);
	if (miter != nouns_complements_.end()) {
		to_return.insert(to_return.end(), miter->second.begin(), miter->second.end());
	}

	return to_return;
}

vector<DrtVect> Sense::getCandidateImpossibleDrtVect(int lexname)
{
	vector<DrtVect> to_return;
	MapIntDrt::iterator miter = sense_impossible_.find(lexname);
	if (miter != sense_impossible_.end()) {
		to_return = miter->second;
	}
	return to_return;
}

vector<DrtVect> Sense::getCandidateCombined(const DrtVect &drtvect, const string &lemma)
{
	DrtVect extracted_drt;
	bool has_drt = false;
	for (int n = 0; n < drtvect.size(); ++n) {
		if (drtvect.at(n).is_verb()) {
			string header = extract_header(drtvect.at(n));
			header = header.substr(0, header.find(":"));

			if (debug) {
				cout << "LEMMA_COMB:::" << header << ", " << lemma << endl;
			}

			if (header == lemma) {
				extracted_drt = find_all_attached_to_verb(drtvect, n);
				if (debug) {
					puts("TRUE:::");
					print_vector(extracted_drt);
				}
				has_drt = true;
				break;
			}
		}
	}

	if (debug) {
		cout << "LEMMA_COMBINED::: " << lemma << endl;
	}

	vector<DrtVect> to_return;
	if (!has_drt)
		return to_return;

	bool levinize = true;
	vector<string> keys = get_all_composed_lemmas(extracted_drt, levinize);
	for (int n = 0; n < keys.size(); ++n) {
		if (debug) {
			cout << "LEMMA_COMBINED2::: " << keys.at(n) << endl;
		}
		MapStrDrt::iterator miter = sense_combined_.find(keys.at(n));
		if (miter != sense_combined_.end()) {
			to_return.insert(to_return.end(), miter->second.begin(), miter->second.end());
		}
	}
	return to_return;
}

vector<DrtVect> Sense::getCandidateCombinedNegative(const DrtVect &drtvect, const string &lemma)
{
	DrtVect extracted_drt;
	bool has_drt = false;
	for (int n = 0; n < drtvect.size(); ++n) {
		if (drtvect.at(n).is_verb()) {
			string header = extract_header(drtvect.at(n));
			header = header.substr(0, header.find(":"));
			if (header == lemma) {
				extracted_drt = find_all_attached_to_verb(drtvect, n);
				has_drt = true;
				break;
			}
		}
	}

	if (debug) {
		cout << "LEMMA_COMBINED_NEGATIVE::: " << lemma << endl;
	}
	vector<DrtVect> to_return;
	if (!has_drt)
		return to_return;

	bool levinize = true;
	vector<string> keys = get_all_composed_lemmas(extracted_drt, levinize);
	for (int n = 0; n < keys.size(); ++n) {
		if (debug) {
			cout << "LEMMA_NEGATIVE_COMBINED2::: " << keys.at(n) << endl;
		}
		MapStrDrt::iterator miter = sense_negative_combined_.find(keys.at(n));
		if (miter != sense_negative_combined_.end()) {
			to_return.insert(to_return.end(), miter->second.begin(), miter->second.end());
		}
	}
	return to_return;
}

vector<DrtVect> Sense::getCandidateNegative(int lexname)
{
	vector<DrtVect> to_return;
	MapIntDrt::iterator miter = sense_negative_.find(lexname);

	if (debug) {
		cout << "CANDIDATE_NEGATIVE::: " << lexname << endl;
	}

	if (miter != sense_negative_.end()) {
		to_return = miter->second;
	}
	return to_return;
}

static bool is_valid_reference(const string &ref)
{
	if (ref.find("name") != string::npos || ref.find("ref") != string::npos || ref.find("verb") != string::npos
			|| ref.find("obj") != string::npos || ref.find("subj") != string::npos || ref.find("prev") != string::npos
			|| ref.find("next") != string::npos || ref.find("from") != string::npos || ref.find("to") != string::npos
			|| ref.find("none") != string::npos)
		return true;

	return false;
}

static vector<DrtPred> name_unifiers(vector<DrtPred> preds)
{
	for (int n = 0; n < preds.size(); ++n) {
		string head_str = extract_header(preds.at(n));
		vector<string> children = preds.at(n).extract_children();
		for (int m = 0; m < children.size(); ++m) {
			string str_tmp = children.at(m);
			if (is_valid_reference(str_tmp)) {
				str_tmp.insert(0, "_");
				children.at(m) = str_tmp;
			}
		}
		preds.at(n).implant_children(children);
	}
	return preds;
}

static DrtVect clean_drtvect(const DrtVect &drtvect)
{
	DrtVect to_return;
	for (int n = 0; n < drtvect.size(); ++n) {
		string header = extract_header(drtvect.at(n));
		if (!drtvect.at(n).is_delete()) {
			header = header.substr(0, header.find(":"));
			DrtPred tmp_pred(drtvect.at(n));
			implant_header(tmp_pred, header);
			to_return.push_back(tmp_pred);
		}
	}

	return to_return;
}

static double get_multiplier(const DrtVect &drtvect)
{
	double to_return = 1;
	metric *d = metric_singleton::get_metric_instance();

	for (int n = 0; n < drtvect.size(); ++n) {
		string header = extract_header(drtvect.at(n));
		string tag = drtvect.at(n).tag();
		string fref = extract_first_tag(drtvect.at(n));
		string sref = extract_second_tag(drtvect.at(n));
		if (header == "@OR" || header == "@AND") {
			int m1 = find_pivot_name_with_string(drtvect, fref);
			int m2 = find_pivot_name_with_string(drtvect, sref);
			if (m1 == -1 || m2 == -1)
				continue;
			string levin1 = d->get_levin_noun(extract_header(drtvect.at(m1)));
			string levin2 = d->get_levin_noun(extract_header(drtvect.at(m2)));

			if (debug) {
				cout << "MULT2::: " << drtvect.at(m1) << ", " << levin1 << ", " << drtvect.at(m2) << ", " << levin2 << endl;

			}

			if (levin1 == levin2)
				to_return += 1;
		}
	}

	return to_return;
}

double Sense::rateVerbs(DrtVect drtvect)
{
	drtvect = clean_drtvect(drtvect);
	vector<pair<string, DrtVect> > candidate_verbs = find_candidate_verbs(drtvect);
	bool levinize = true;
	vector<string> candidate_lemmas = get_all_composed_lemmas(drtvect, levinize);
	if (debug)
		cout << candidate_verbs.size() << endl;
	// Do the matching with the appropriate candidates
	Knowledge k;
	MatchInfo mi;
	mi.setHypernymHeight(1);
	Match match(&k);
	double w = 0;
	for (int m = 0; m < candidate_verbs.size(); ++m) {
		vector<string> cverbs;
		cverbs.push_back(candidate_verbs.at(m).first);
		vector<int> lexnames = find_candidate_lexnames(cverbs);
		DrtVect dvect = candidate_verbs.at(m).second;
		double multiplier = get_multiplier(dvect);

		if (debug) {
			cout << "MULTIPLIER::: " << multiplier << endl;
		}

		for (int n = 0; n < lexnames.size(); ++n) {
			vector<DrtVect> cdrts = this->getCandidateImpossibleDrtVect(lexnames.at(n));
			for (int j = 0; j < cdrts.size(); ++j) {
				if (debug)
					print_vector(cdrts.at(j));
				if (debug)
					puts("HERE_IMPOSSIBLE");
				MatchSubstitutions msubs;
				DrtVect question(cdrts.at(j));
				question = name_unifiers(question);
				double tmp_w2 = match.singlePhraseMatch(dvect, question, &msubs, mi);
				if (tmp_w2 != 0) {
					if(debug) {
						cout << "IMPOSSIBLE_W::: " << tmp_w2 << endl;
					}
					return -2;
				}
			}
		}

		bool continue_loop = true;
		double tmp_w = 0;
		for (int n = 0; n < lexnames.size() && continue_loop; ++n) {
			vector<DrtVect> cdrts = this->getCandidateDrtVect(lexnames.at(n));
			if (debug) {
				cout << "LX2::: " << lexnames.size() << "," << cdrts.size() << endl;
			}
			for (int j = 0; j < cdrts.size(); ++j) {
				if (debug)
					print_vector(cdrts.at(j));
				if (debug)
					puts("HERE_DECLARED");
				MatchSubstitutions msubs;
				DrtVect question(cdrts.at(j));
				question = name_unifiers(question);

				double tmp_w2 = match.singlePhraseMatch(dvect, question, &msubs, mi);
				if (tmp_w2 != 0) {
					tmp_w = 1;
					continue_loop = false;
					break;
				}
			}
		}

		// Loops over the negated sentences
		continue_loop = true;
		for (int n = 0; n < lexnames.size() && continue_loop; ++n) {
			vector<DrtVect> cdrts = this->getCandidateDrtVect(lexnames.at(n));
			cdrts = this->getCandidateNegative(lexnames.at(n));
			for (int j = 0; j < cdrts.size(); ++j) {
				if (debug)
					print_vector(cdrts.at(j));
				if (debug)
					puts("HERE_NEGATED:::");
				MatchSubstitutions msubs;
				DrtVect question(cdrts.at(j));
				question = name_unifiers(question);
				double tmp_w2 = match.singlePhraseMatch(dvect, question, &msubs, mi);
				if (tmp_w2 != 0) {
					tmp_w = 0;
					continue_loop = false;
					break;
				}
			}
		}
		if (tmp_w != 0) {
			w += 1; // if there is no specific sense for this rule, but the verb was in Wordnet, add 1 to the sense
			vector<DrtVect> cdrts2;
			cdrts2 = this->getCandidateCombined(drtvect, cverbs.front());
			if (debug) {
				cout << "LX3::: " << cverbs.front() << "," << cdrts2.size() << endl;
			}
			for (int j = 0; j < cdrts2.size(); ++j) {
				MatchSubstitutions msubs;
				DrtVect question(cdrts2.at(j));
				std::stringstream ss;
				print_vector_stream(ss, question);

				if (debug) {
					cout << "QUESTION:::" << ss.str() << endl;
				}

				pair<double, int> drt_weight = get_map_element(sense_weight_, ss.str());
				question = name_unifiers(question);
				double wtmp = match.singlePhraseMatch(dvect, question, &msubs, mi) * drt_weight.first;
				w += wtmp * multiplier;
			}
			cdrts2 = this->getCandidateCombinedNegative(drtvect, cverbs.front());
			for (int j = 0; j < cdrts2.size(); ++j) {
				MatchSubstitutions msubs;
				DrtVect question(cdrts2.at(j));
				std::stringstream ss;
				print_vector_stream(ss, question);
				pair<double, int> drt_weight = get_map_element(sense_negative_weight_, ss.str());
				question = name_unifiers(question);
				double wtmp = match.singlePhraseMatch(dvect, question, &msubs, mi) * drt_weight.first;
				w -= wtmp; // * multiplier;
			}

		}
	}
	return w;
}

double Sense::rateNouns(DrtVect drtvect)
{
	double w = 0;
	vector<pair<string, DrtVect> > candidate_nouns = find_candidate_nouns(drtvect);

	if (debug) {
		std::cerr << "CNOUNS::: " << candidate_nouns.size() << endl;
	}

	Knowledge k;
	Match match(&k);
	for (int m = 0; m < candidate_nouns.size(); ++m) {
		string name = candidate_nouns.at(m).first;
		vector<int> lexnames = find_candidate_noun_lexnames(name);
		DrtVect drtvect_tmp = candidate_nouns.at(m).second;
		for (int n = 0; n < lexnames.size(); ++n) {
			MatchSubstitutions msubs;
			vector<DrtVect> cdrts = this->getCandidateNounsDrtVect(lexnames.at(n));

			for (int j = 0; j < cdrts.size(); ++j) {
				DrtVect question(cdrts.at(j));
				std::stringstream ss;
				print_vector_stream(ss, question);
				if (debug) {
					std::cerr << "CNOUNS::: " << cdrts.size() << " " << lexnames.at(n) << " " << candidate_nouns.size()
							<< endl;
					std::cerr << "CNOUNS::: " << ss.str() << endl;
				}
				question = name_unifiers(question);
				double wtmp = match.singlePhraseMatch(drtvect_tmp, question, &msubs, false, false);
				if(debug) {
					std::cerr << "WTMP:::" << wtmp << endl;
				}
				if(wtmp != 0) {
					//w += wtmp;
					w +=1;
				}
			}
		}
	}

	return w;
}

double Sense::rate(DrtVect drtvect)
{
	double wverbs = 0, wnouns = 0, w = 0;
	wverbs = this->rateVerbs(drtvect);
	wnouns = this->rateNouns(drtvect);

	if (wverbs < 0 || wnouns < 0)
		return -1;

	w = wverbs + wnouns;

	return w;

}

bool Sense::increaseWeight(const vector<DrtVect> &all_drts, double delta, int increase_num)
{
	for (int n = 0; n < all_drts.size(); ++n) {
		DrtVect drtvect = all_drts.at(n);
		std::stringstream ss;
		print_vector_stream(ss, drtvect);
		MapDrtDouble::iterator miter = sense_weight_.find(ss.str());
		if (miter != sense_weight_.end()) {
			double w = miter->second.first;
			int num = miter->second.second;
			if (w + delta > 1)
				return false;
			miter->second = make_pair(w + delta, num + increase_num);
		} else {
			double w = 0;
			int num = 0;
			sense_weight_[ss.str()] = make_pair(w + delta, num + increase_num);
			vector<pair<string, DrtVect> > candidate_pairs = find_candidate_verbs(drtvect);
			if (candidate_pairs.size() == 0)
				continue;
			string lemma = candidate_pairs.at(0).first;
			sense_combined_[lemma].push_back(drtvect);
		}
	}

	return true;
}

bool Sense::decreaseWeight(const vector<DrtVect> &all_drts, double delta, int increase_num)
{
	for (int n = 0; n < all_drts.size(); ++n) {
		DrtVect drtvect = all_drts.at(n);
		std::stringstream ss;
		print_vector_stream(ss, drtvect);
		MapDrtDouble::iterator miter = sense_negative_weight_.find(ss.str());
		if (miter != sense_negative_weight_.end()) {
			double w = miter->second.first;
			int num = miter->second.second;
			if (w + delta > 1)
				return false;
			miter->second = make_pair(w + delta, num + increase_num);
		} else {
			double w = 0;
			int num = 0;
			sense_negative_weight_[ss.str()] = make_pair(w + delta, num + increase_num);
			vector<pair<string, DrtVect> > candidate_pairs = find_candidate_verbs(drtvect);
			if (candidate_pairs.size() == 0)
				continue;
			string lemma = candidate_pairs.at(0).first;
			sense_negative_combined_[lemma].push_back(drtvect);
		}
	}

	return true;
}

vector<tuple<string, string, double, int> > Sense::getCombinedLines()
{
	vector<tuple<string, string, double, int> > to_return;

	MapStrDrt::iterator miter = sense_combined_.begin(), mend = sense_combined_.end();

	for (; miter != mend; ++miter) {

		tuple<string, string, double, int> item;
		vector<DrtVect> drtvects = miter->second;

		if (debug) {
			puts("LINES:::");
		}

		for (int n = 0; n < drtvects.size(); ++n) {

			item.get<0>() = miter->first;
			std::stringstream ss;
			print_vector_stream(ss, drtvects.at(n));
			pair<double, int> drt_weight = get_map_element(sense_weight_, ss.str());
			item.get<1>() = ss.str();
			item.get<2>() = drt_weight.first;
			item.get<3>() = drt_weight.second;

			to_return.push_back(item);
		}
	}

	return to_return;
}

vector<tuple<string, string, double, int> > Sense::getCombinedNegativeLines()
{
	vector<tuple<string, string, double, int> > to_return;

	MapStrDrt::iterator miter = sense_negative_combined_.begin(), mend = sense_negative_combined_.end();

	for (; miter != mend; ++miter) {

		tuple<string, string, double, int> item;
		vector<DrtVect> drtvects = miter->second;

		for (int n = 0; n < drtvects.size(); ++n) {

			item.get<0>() = miter->first;
			std::stringstream ss;
			print_vector_stream(ss, drtvects.at(n));
			pair<double, int> drt_weight = get_map_element(sense_negative_weight_, ss.str());
			item.get<1>() = ss.str();
			item.get<2>() = drt_weight.first;
			item.get<3>() = drt_weight.second;

			to_return.push_back(item);
		}
	}

	return to_return;
}

Sense::Sense(const Sense &rhs)
{
	sense_actors_ = rhs.sense_actors_;
	sense_complements_ = rhs.sense_complements_;
	sense_negative_ = rhs.sense_negative_;
	sense_combined_ = rhs.sense_combined_;
	sense_weight_ = rhs.sense_weight_;
	sense_negative_combined_ = rhs.sense_negative_combined_;
	sense_negative_weight_ = rhs.sense_negative_weight_;
}

