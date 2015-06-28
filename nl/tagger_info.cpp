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



#include"tagger_info.hpp"

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

template<class K, class T>
static T get_map_element(btree::btree_map<K, T> &map_element, K &key)
{
	T to_return;
	typename unordered_map<K, T>::iterator miter = map_element.find(key);
	if (miter != map_element.end()) {
		to_return = miter->second;
	}
	return to_return;
}

template<class K, class T>
static T get_map_element(btree::btree_map<K, T> &map_element, const K &key)
{
	T to_return;
	//typename unordered_map<K, T>::iterator miter = map_element.find(key);
	typename btree::btree_map<K, T>::iterator miter = map_element.find(key);
	if (miter != map_element.end()) {
		to_return = miter->second;
	}
	return to_return;
}

template<class T>
static bool shortfind(const vector<T> &vect, const T &element)
{
	if (find(vect.begin(), vect.end(), element) == vect.end())
		return false;
	return true;
}


static bool is_verb(const string &tag)
{
	if (tag == "VBP" || tag == "VBZ" || tag == "VBD" || tag == "VB" || tag == "VBN" || tag == "VBG")
		return true;
	return false;
}

static bool is_name(const string &tag)
{
	if (tag == "NN" || tag == "NNS" || tag == "NNP" || tag == "NNPS" || tag == "PRP" || tag == "JJ" || tag == "JJS")
		return true;
	return false;
}

static double guess_word(const string &word, const string &tag)
{
	int size = word.size();
	// if(word != "\'s" && word != "as" && word != "his" && word != "its" && word != "this" && word.at(size-1) == 's'
	// 	&& (tag == "NNS" || tag == "NNPS" || tag == "VBZ") )
	// 	  return 1./3;
	// if(size > 4 && word.at(size-1) == 'g' && word.at(size-2) == 'n' && word.at(size-3) == 'i' && tag == "VBG") // gerundive
	// 	  return 1;
	// if(size > 3 && word.at(size-1) == 'y' && word.at(size-2) == 'l' && tag == "RB") // it is an adverb
	// 	  return 1;
	// if(word != "family" && size > 3 && word.at(size-1) == 'y' && word.at(size-2) == 'l' && tag == "NN") // it is a name
	// 	  return 1;
	// if(size > 3 && word.at(size-1) == 'd' && word.at(size-2) == 'e' && ( tag == "VBD" || tag == "VBN" ) ) // it is a past tense
	//  	  return 1./2;

	// if(tag == "NN" || tag == "NNP") /// you should differently check for proper names
	// 	  return 1;

	return 0; // default
}

tagger_info::tagger_info()
// initializes the private data structures
{
	//complete_regular_verbs();

	// name of the months
	months_.push_back("january");
	months_.push_back("february");
	months_.push_back("march");
	months_.push_back("april");
	months_.push_back("may");
	months_.push_back("june");
	months_.push_back("july");
	months_.push_back("august");
	months_.push_back("september");
	months_.push_back("october");
	months_.push_back("november");
	months_.push_back("december");

	// names indicating time
	chrono_.push_back("today");
	chrono_.push_back("tomorrow");
	chrono_.push_back("yesterday");
	chrono_.push_back("last_week");

	// sure tags
	sure_tags_["a"] = "DT";
	sure_tags_["to"] = "TO";
	sure_tags_["the"] = "DT";
	sure_tags_["for"] = "IN";
	sure_tags_["in"] = "IN";
	sure_tags_["of"] = "IN";
	sure_tags_["if"] = "IN";
	sure_tags_["into"] = "IN";
	sure_tags_["by"] = "IN";
	sure_tags_["after"] = "IN";
	sure_tags_["about"] = "IN";
	sure_tags_["at-what-time"] = "WRB";
	sure_tags_["across"] = "IN";
	sure_tags_["whether"] = "IN";
	sure_tags_["during"] = "IN";
	sure_tags_["from"] = "IN";
	sure_tags_["who"] = "WP";
	sure_tags_["whom"] = "WP";
	sure_tags_["he"] = "PRP";
	sure_tags_["it"] = "PRP";
	sure_tags_["his"] = "PRP$";
	sure_tags_["she"] = "PRP";
	sure_tags_["you"] = "PRP";
	sure_tags_["they"] = "PRP";
	sure_tags_["their"] = "PRP$";
	sure_tags_["theirs"] = "PRP$";
	sure_tags_["thus"] = "RB";
	sure_tags_["and"] = "CC";
	sure_tags_["or"] = "CC";
	sure_tags_["'s"] = "POS";
	sure_tags_["its"] = "PRP$";
	sure_tags_["as"] = "IN";
	sure_tags_["among"] = "IN";
	sure_tags_["means"] = "NN"; //irregular singular
	sure_tags_["politics"] = "NNS"; //only plural
	sure_tags_["perhaps"] = "RB";
	sure_tags_["themselves"] = "PRP";
	sure_tags_["born"] = "VBN";
	sure_tags_["how"] = "WRB";
	sure_tags_["where"] = "WRB";
	sure_tags_["while"] = "IN";
	sure_tags_["how-far"] = "WRB";
	sure_tags_["how-much"] = "WRB";
	sure_tags_["how-many"] = "WRB";
	sure_tags_["how-many-times"] = "WRB";
	sure_tags_["how-big"] = "WRB";
	sure_tags_["how-old"] = "WRB";
	sure_tags_["should"] = "MD";
	sure_tags_["more-than"] = "IN";
	sure_tags_["less-than"] = "IN";
	sure_tags_["up-to"] = "IN";
	sure_tags_["of_course"] = "RB";
	sure_tags_["i"] = "PRP";
	sure_tags_["people"] = "NNS";
	sure_tags_["at"] = "IN";
	sure_tags_["also"] = "RB";
	sure_tags_["whose"] = "WP$";
	sure_tags_["according-to"] = "IN";
	sure_tags_["%"] = "NN";
	sure_tags_["uk"] = "NN";
	sure_tags_["already-made"] = "RB";
	sure_tags_["other-than"] = "IN";
	sure_tags_["together"] = "RB";
	sure_tags_["at_least"] = "RB";
	sure_tags_["began"] = "VBD";
	sure_tags_["why"] = "WRB";
	sure_tags_["this"] = "DT";
	sure_tags_["that"] = "DT";
	sure_tags_["these"] = "DT";
	sure_tags_["those"] = "DT";
	sure_tags_["than"] = "IN";
	sure_tags_["to"] = "TO";
	sure_tags_["sometimes"] = "RB";
	sure_tags_["no-more"] = "RB";
	sure_tags_["no-longer"] = "RB";
	sure_tags_["such_as"] = "IN";
	sure_tags_["newly_formed"] = "JJ";
	sure_tags_["we"] = "PRP";
	sure_tags_["could"] = "MD";
	sure_tags_["would"] = "MD";
	sure_tags_["a_lot"] = "RB";
	sure_tags_["cannot"] = "MD";
	sure_tags_["in_and_out"] = "IN";
	sure_tags_["instead_of"] = "IN";
	sure_tags_["all_the_way"] = "RB";
	sure_tags_["all_the_way_around"] = "RB";

	// exceptions to regular tags
	except_.push_back("mars");
	except_.push_back("ios");
	except_.push_back("as");
	except_.push_back("us");
	except_.push_back("is");
	except_.push_back("need");
	except_.push_back("hiss");
	except_.push_back("breed");
	except_.push_back("red");
	except_.push_back("does");
	except_.push_back("has");
	except_.push_back("was");
	except_.push_back("focus");
	except_.push_back("miss");
	except_.push_back("oppress");
	except_.push_back("pass");
	except_.push_back("possess");
	except_.push_back("progress");
	except_.push_back("stress");
	except_.push_back("address");
	except_.push_back("cross");
	except_.push_back("dismiss");
	except_.push_back("encompass");
	except_.push_back("express");
	except_.push_back("tennis");
	except_.push_back("ed");

	except_.push_back("its");
	except_.push_back("ring");
	except_.push_back("interesting");
	except_.push_back("bring");
	except_.push_back("evening");
	except_.push_back("sing");
	except_.push_back("string");
	except_.push_back("sing");
	except_.push_back("spring");
	except_.push_back("wing");
	except_.push_back("timing");
	except_.push_back("swing");
	except_.push_back("ping");
	except_.push_back("king");
	except_.push_back("right-wing");
	except_.push_back("during");
}

double tagger_info::general_tag(const string &tag, const string &tag_prev)
// context_free conditional probability for the previous neighbor
{
	TMapPStDouble::iterator gen_iter;
	gen_iter = tag_freqs.find(make_pair(tag, tag_prev));
	if (gen_iter != tag_freqs.end())
		return gen_iter->second;
	return 0;
}

double tagger_info::general_tag_back(const string &tag, const string &tag_next)
// context_free conditional probability for the nn previous neighbor
{
	TMapPStDouble::iterator gen_iter;
	gen_iter = tag_freqs_back.find(make_pair(tag, tag_next));
	if (gen_iter != tag_freqs_back.end())
		return gen_iter->second;
	return 0;
}

double tagger_info::general_tag_prior(const string &tag, const string &tag_next)
// context_free conditional probability for the next neighbor
{
	TMapPStDouble::iterator gen_iter;
	gen_iter = tag_freqs_prior.find(make_pair(tag, tag_next));
	if (gen_iter != tag_freqs_prior.end())
		return gen_iter->second;
	return 0;
}

bool tagger_info::has_base(const string &word)
// returns true if word is present as unconjugated
{
	TMapStVSt::iterator conj_iter;
	conj_iter = conjug_base.find(word);
	if (conj_iter != conjug_base.end())
		return true;
	return false;
}

bool tagger_info::is_valid_general_tag(const string &tag)
{
	vector<string> valid_ones;
	valid_ones.push_back("NN");
	valid_ones.push_back("NNP");
	valid_ones.push_back("NNS");
	valid_ones.push_back("NNPS");
	valid_ones.push_back("JJ");

	if (find(valid_ones.begin(), valid_ones.end(), tag) != valid_ones.end())
		return true;
	return false;
}

bool tagger_info::is_valid_verb_tag(const string &tag)
{
	vector<string> valid_ones;
	valid_ones.push_back("VB");
	//valid_ones.push_back("VBG");
	valid_ones.push_back("VBZ");
	valid_ones.push_back("VBP");
	valid_ones.push_back("VBD");
	valid_ones.push_back("VBN");

	if (find(valid_ones.begin(), valid_ones.end(), tag) != valid_ones.end())
		return true;
	return false;
}

double tagger_info::get_freq(const string &word, const string &tag, const string &prev_tag)
// return the conditional probability of a word with tag "tag" being preceded by "prev_tag"
{
	string base;
	TMapPStSt::iterator base_iter;
	TMapStVSt::iterator conj_iter;
	conj_iter = conj_tag.find(word);
	if (conj_iter != conj_tag.end()
			&& find(conj_iter->second.begin(), conj_iter->second.end(), tag) == conj_iter->second.end()) {
		return 0;
	}
	conj_iter = conjug_base.find(word);
	if (conj_iter != conjug_base.end()
			&& find(conj_iter->second.begin(), conj_iter->second.end(), tag) != conj_iter->second.end()) {
		return 0;
	}
	base_iter = conjug.find(std::make_pair(word, tag));
	if (base_iter != conjug.end()) {
		base = base_iter->second;
		TMapPInt::iterator freq_iter_num;
		TMapStInt::iterator freq_iter_den;
		freq_iter_num = freqs.find(std::make_pair(base, std::make_pair(tag, prev_tag)));
		freq_iter_den = tot_nums.find(base);
		double freq = 0;
		if (freq_iter_num != freqs_prior.end() && freq_iter_den != tot_nums_prior.end()) {
			freq = freq_iter_num->second / static_cast<double>(freq_iter_den->second)
					* exp(-1 / sqrt(static_cast<double>(freq_iter_den->second)));
		}
		return freq;
	} else
		base = word;

	TMapPInt::iterator freq_iter_num;
	TMapStInt::iterator freq_iter_den;
	freq_iter_num = freqs.find(std::make_pair(base, std::make_pair(tag, prev_tag)));
	freq_iter_den = tot_nums.find(base);

	double to_ret = 0;
	string base_tmp;
	to_ret = regular_tag(word, tag, &base_tmp);
	if (base_tmp == "[wrong_tag]")
		return 0;
	if (to_ret != 0 && tag == "NNS" && !is_candidate_name(base_tmp)) {
		return 0; // avoid verbs like "derive" to be understood as NNS
	}
	if (to_ret != 0 && tag == "VBZ" && !is_candidate_verb(base_tmp)) {
		if (debug)
			puts("NOVERB");
		return 0; // avoid verbs like "academic" to be understood as VBZ
	}
	if (freq_iter_den == tot_nums.end()) {
		if (to_ret != 0 && base_iter == conjug.end() // "has" is in the conjug file as VBZ, must not be adressed as regular
				) {
			freq_iter_num = freqs.find(std::make_pair(base_tmp, std::make_pair(tag, prev_tag)));
			freq_iter_den = tot_nums.find(base_tmp);
			if (freq_iter_num == freqs.end()) {
				return 0; /// NO GUESSWORK BETWEEN VBZ AND NNS!!!
			} else {
				double freq = freq_iter_num->second / static_cast<double>(freq_iter_den->second);
				return freq;
			}
		} else if (is_valid_general_tag(tag)) {
			to_ret = general_tag(tag, prev_tag);
			if (to_ret != 0)
				return to_ret;
		}
	} else if (to_ret != 0 && base_iter == conjug.end() // "has" is in the conjug file as VBZ, must not be adressed as regular
			) {
		freq_iter_num = freqs.find(std::make_pair(base_tmp, std::make_pair(tag, prev_tag)));
		freq_iter_den = tot_nums.find(base_tmp);
		if (freq_iter_num == freqs.end()) {
			if ((!is_candidate_verb(word) && is_valid_general_tag(tag))
					|| (!is_candidate_name(word) && is_valid_verb_tag(tag))) {
				if (last_hope_tags_.find(word) != last_hope_tags_.end() && get_map_element(last_hope_tags_, word) == tag)
					return 0;
				to_ret = general_tag(tag, prev_tag);
				//cout << "PGEN::::" << base_tmp << ", " << to_ret << ", " << tag << endl;
			} else
				to_ret = 0;
			return to_ret;
		} else {
			double freq = freq_iter_num->second / static_cast<double>(freq_iter_den->second);
			return freq;
		}
	}
	if (freq_iter_num == freqs.end() && !is_candidate_name(word) && !is_candidate_verb(word)
			) {
		if (last_hope_tags_.find(word) != last_hope_tags_.end() && to_ret == 0) {
			if (get_map_element(last_hope_tags_, word) == tag) {
				return 1;
			}
		}
		return 0;
	}

	if (freq_iter_den == tot_nums.end() || freq_iter_num == freqs.end())
		return 0;

	if (base_iter == conjug.end()
			&& (tag == "VBZ" || tag == "VBD" || tag == "VBN" || tag == "VBG" || tag == "NNS" || tag == "NNPS")) // These are the only tags that modify the word. The only ones incorrect at this point
		return 0;

	double freq = freq_iter_num->second / static_cast<double>(freq_iter_den->second)
			* exp(-1 / sqrt(static_cast<double>(freq_iter_den->second)));
	return freq;
}


double tagger_info::get_freq_back(const string &word, const string &tag, const string &next_tag)
// return the conditional probability of a word with tag "tag" being preceded by the second nearest "next_tag"
{
	string base;
	TMapPStSt::iterator base_iter;
	TMapStVSt::iterator conj_iter;
	conj_iter = conj_tag.find(word);
	if (conj_iter != conj_tag.end()
			&& find(conj_iter->second.begin(), conj_iter->second.end(), tag) == conj_iter->second.end()) {
		// The word is in the conjugation file but is never tagged
		// as "tag" return 0
		return 0;
	}
	conj_iter = conjug_base.find(word);
	if (conj_iter != conjug_base.end()
			&& find(conj_iter->second.begin(), conj_iter->second.end(), tag) != conj_iter->second.end()) {
		// The word is in the conjugation file but is declined in another way than "word"
		return 0;
	}
	base_iter = conjug.find(std::make_pair(word, tag));
	if (base_iter != conjug.end()) {
		base = base_iter->second;
		TMapPInt::iterator freq_iter_num;
		TMapStInt::iterator freq_iter_den;
		freq_iter_num = freqs_back.find(std::make_pair(base, std::make_pair(tag, next_tag)));
		freq_iter_den = tot_nums_back.find(base);
		double freq = 0;
		if (freq_iter_num != freqs_prior.end() && freq_iter_den != tot_nums_prior.end()) {
			freq = freq_iter_num->second / static_cast<double>(freq_iter_den->second)
					* exp(-1 / sqrt(static_cast<double>(freq_iter_den->second)));
		}
		return freq;
	} else
		base = word;

	TMapPInt::iterator freq_iter_num;
	TMapStInt::iterator freq_iter_den;
	freq_iter_num = freqs_back.find(std::make_pair(base, std::make_pair(tag, next_tag)));
	freq_iter_den = tot_nums_back.find(base);

	double to_ret;
	string base_tmp;
	to_ret = regular_tag(word, tag, &base_tmp);
	if (base_tmp == "[wrong_tag]")
		return 0;
	if (to_ret != 0 && tag == "NNS" && !is_candidate_name(word))
		return 0; // avoid verbs like "derive" to be understood as NNS
	if (to_ret != 0 && tag == "VBZ" && !is_candidate_verb(base_tmp)) {
		return 0; // avoid verbs like "academic" to be understood as VBZ
	}
	if (freq_iter_den == tot_nums_back.end()) {
		if (to_ret != 0 && base_iter == conjug.end()) {
			freq_iter_num = freqs_back.find(std::make_pair(base_tmp, std::make_pair(tag, next_tag)));
			freq_iter_den = tot_nums_back.find(base_tmp);
			if (freq_iter_num == freqs_back.end()) {
				return 0; /// NO GUESSWORK BETWEEN VBZ AND NNS!!!
			} else {
				double freq = freq_iter_num->second / static_cast<double>(freq_iter_den->second)
						* exp(-1 / sqrt(static_cast<double>(freq_iter_den->second)));
				return freq;
			}
		} else if (is_valid_general_tag(tag)) {
			return general_tag_back(tag, next_tag);
		}
	} else if (to_ret != 0 && base_iter == conjug.end() // "has" is in the conjug file as VBZ, must not be adressed as regular
			) {
		freq_iter_num = freqs.find(std::make_pair(base_tmp, std::make_pair(tag, next_tag)));
		freq_iter_den = tot_nums.find(base_tmp);
		if (freq_iter_num == freqs.end()) {
			if ((!is_candidate_verb(word) && is_valid_general_tag(next_tag))
					|| (!is_candidate_name(word) && is_valid_verb_tag(next_tag))) {
				if (last_hope_tags_.find(word) != last_hope_tags_.end() && get_map_element(last_hope_tags_, word) == tag)
					return 0;
				to_ret = general_tag_back(tag, next_tag);
			} else
				to_ret = 0;
			return to_ret;
		} else {
			double freq = freq_iter_num->second / static_cast<double>(freq_iter_den->second); // * exp(-1 / sqrt(static_cast<double> (freq_iter_den->second)));
			return freq;
		}
	}
	if (freq_iter_num == freqs_back.end() || freq_iter_den == tot_nums_back.end()) {
		return 0;
	}

	if (base_iter == conjug.end()
			&& (tag == "VBZ" || tag == "VBD" || tag == "VBN" || tag == "VBG" || tag == "NNS" || tag == "NNPS")) // These are the only tags that modify the word. The only ones non admissable at this point
		return 0;

	double freq = freq_iter_num->second / static_cast<double>(freq_iter_den->second)
			* exp(-1 / sqrt(static_cast<double>(freq_iter_den->second)));
	return freq;
}

double tagger_info::get_freq_prior(const string &word, const string &tag, const string &next_tag)
// return the conditional probability of a word with tag "tag" being followed by "next_tag"
{
	string base;
	TMapPStSt::iterator base_iter;
	TMapStVSt::iterator conj_iter;
	conj_iter = conj_tag.find(word);
	if (conj_iter != conj_tag.end()
			&& find(conj_iter->second.begin(), conj_iter->second.end(), tag) == conj_iter->second.end()) {
		// The word is in the conjugation file but is never tagged
		// as "tag" return 0
		return 0;
	}
	conj_iter = conjug_base.find(word);
	if (conj_iter != conjug_base.end()
			&& find(conj_iter->second.begin(), conj_iter->second.end(), tag) != conj_iter->second.end()) {
		// The word is in the conjugation file but is declined in another way than "word"
		return 0;
	}
	base_iter = conjug.find(std::make_pair(word, tag));
	if (base_iter != conjug.end()) {
		base = base_iter->second;
		TMapPInt::iterator freq_iter_num;
		TMapStInt::iterator freq_iter_den;
		freq_iter_num = freqs_prior.find(std::make_pair(base, std::make_pair(tag, next_tag)));
		freq_iter_den = tot_nums_prior.find(base);
		double freq = 0;
		if (freq_iter_num != freqs_prior.end() && freq_iter_den != tot_nums_prior.end()) {
			freq = freq_iter_num->second / static_cast<double>(freq_iter_den->second)
					* exp(-1 / sqrt(static_cast<double>(freq_iter_den->second)));
		}
		return freq;
	} else
		base = word;

	TMapPInt::iterator freq_iter_num;
	TMapStInt::iterator freq_iter_den;
	freq_iter_num = freqs_prior.find(std::make_pair(base, std::make_pair(tag, next_tag)));
	freq_iter_den = tot_nums_prior.find(base);

	double to_ret;
	string base_tmp;
	to_ret = regular_tag(word, tag, &base_tmp);
	if (base_tmp == "[wrong_tag]")
		return 0;
	if (to_ret != 0 && tag == "NNS" && !is_candidate_name(word))
		return 0; // avoid verbs like "derive" to be understood as NNS
	if (to_ret != 0 && tag == "VBZ" && !is_candidate_verb(base_tmp)) {
		return 0; // avoid verbs like "academic" to be understood as VBZ
	}
	if (freq_iter_den == tot_nums_prior.end()) {
		if (to_ret != 0 && base_iter == conjug.end()) {
			freq_iter_num = freqs_prior.find(std::make_pair(base_tmp, std::make_pair(tag, next_tag)));
			freq_iter_den = tot_nums_prior.find(base_tmp);
			if (freq_iter_num == freqs_prior.end()) {
				return 0; /// NO GUESSWORK BETWEEN VBZ AND NNS!!!
			} else {
				double freq = freq_iter_num->second / static_cast<double>(freq_iter_den->second)
						* exp(-1 / sqrt(static_cast<double>(freq_iter_den->second)));
				return freq;
			}
		} else if (is_valid_general_tag(tag)) {
			return general_tag_prior(tag, next_tag);
		}
	} else if (to_ret != 0 && base_iter == conjug.end() // "has" is in the conjug file as VBZ, must not be adressed as regular
			) {
		freq_iter_num = freqs.find(std::make_pair(base_tmp, std::make_pair(tag, next_tag)));
		freq_iter_den = tot_nums.find(base_tmp);
		if (freq_iter_num == freqs.end()) {
			if ((!is_candidate_verb(word) && is_valid_general_tag(next_tag))
					|| (!is_candidate_name(word) && is_valid_verb_tag(next_tag))) {
				if (last_hope_tags_.find(word) != last_hope_tags_.end() && get_map_element(last_hope_tags_, word) == tag)
					return 0;
				to_ret = general_tag_prior(tag, next_tag);
			} else
				to_ret = 0;
			return to_ret;
		} else {
			double freq = freq_iter_num->second / static_cast<double>(freq_iter_den->second);
			return freq;
		}
	}
	if (freq_iter_den == tot_nums_prior.end() || freq_iter_num == freqs_prior.end())
		return 0;

	if (base_iter == conjug.end()
			&& (tag == "VBZ" || tag == "VBD" || tag == "VBN" || tag == "VBG" || tag == "NNS" || tag == "NNPS")) // These are the only tags that modify the word. The only ones non admissable at this point
		return 0;
	double freq = freq_iter_num->second / static_cast<double>(freq_iter_den->second)
			* exp(-1 / sqrt(static_cast<double>(freq_iter_den->second)));
	return freq;
}

string tagger_info::get_conj(const string &word, string tag)
// return the base form for "word" tagged as "tag"
{
	if (tag == "NNPS")
		tag = "NNS";

	// Some combination just don't exist (although they are in grammatically possible)
	map<pair<string, string>, string> exceptions;
	exceptions[make_pair("says", "NNS")] = 1;
	exceptions[make_pair("ends", "NNS")] = 1;
	map<pair<string, string>, string>::iterator miter = exceptions.find(make_pair(word, tag));
	if (miter != exceptions.end())
		return "";

	string base = "";

	TMapPStSt::iterator base_iter;
	TMapStVSt::iterator conj_iter;
	conj_iter = conj_tag.find(word);
	if (conj_iter == conj_tag.end())
		goto regular;
	if (conj_iter != conj_tag.end()
			&& find(conj_iter->second.begin(), conj_iter->second.end(), tag) == conj_iter->second.end()) {
		// The word is in the conjugation file but is never tagged
		// as "tag"
		return base;
	}
	conj_iter = conjug_base.find(word);
	if (conj_iter != conjug_base.end()
			&& find(conj_iter->second.begin(), conj_iter->second.end(), tag) != conj_iter->second.end()) {
		// The word is in the conjugation file but is declined in another way than "word"
		return base;
	}
	base_iter = conjug.find(std::make_pair(word, tag));
	if (base_iter != conjug.end()) {
		base = base_iter->second;
		return base;
	} else {
		base = word;
	}

	regular: /// one in a million
	string base_tmp = "";
	double w = this->regular_tag(word, tag, &base_tmp); // Try the regular tag anyway
	if (w != 0 && base_tmp != "")
		base = base_tmp;
	return base;
}


void tagger_info::load_wikidata_names(const char *f)
{
	std::ifstream file;
	char article[1024];
	vector<string> data;

	file.open(f);
	if (!file.bad()) {
		std::cerr << "Loading WikiData names from \'" << f << "\'." << std::endl;
		while (file.good()) {
			file.getline(article, 1024);
			if(article[0] == '#')
				continue;
			boost::split(data, article, boost::is_any_of(" "));

			if (data.size() >= 2) {
				string q_str= data.at(0);
				q_str = q_str.substr(1,q_str.size() );
				int q_id= boost::lexical_cast<int>(q_str);
				vector<string> key(data.begin()+1,data.end() );

				// lowers all the letters
				string name = "";
				for (int n = 0; n < key.size(); ++n) {
					string wordstr = key.at(n);
					for (int i = 0; i < wordstr.size(); ++i) {
						char tmp_char = std::tolower(wordstr.at(i));
						wordstr.at(i) = tmp_char;
					}
					key.at(n) = wordstr;
					name += wordstr + " ";
				}
				q_names_[q_id] = name;
			}
		}
	} else
		throw std::length_error(std::string("File") + f + " finished unexpectedly.");

}

void tagger_info::load_wikidata_qs(const char *f)
{
	std::ifstream file;
	char article[1024];
	vector<string> data;

	file.open(f);
	if (!file.bad()) {
		std::cerr << "Loading WikiData items from \'" << f << "\'." << std::endl;
		while (file.good()) {
			file.getline(article, 1024);
			if(article[0] == '#')
				continue;
			boost::split(data, article, boost::is_any_of(" "));

			if (data.size() >= 2) {
				string q_str= data.at(0);
				q_str = q_str.substr(1,q_str.size() );
				int q_id= boost::lexical_cast<int>(q_str);
				vector<string> key(data.begin()+1,data.end() );

				// lowers all the letters
				string name = "";
				for (int n = 0; n < key.size(); ++n) {
					string wordstr = key.at(n);
					for (int i = 0; i < wordstr.size(); ++i) {
						char tmp_char = std::tolower(wordstr.at(i));
						wordstr.at(i) = tmp_char;
					}
					key.at(n) = wordstr;
					name += wordstr;
					if(n != key.size()-1)
						name += "_";
				}
				q_values_[name].push_back(q_id);
			}
		}
	} else
		throw std::length_error(std::string("File") + f + " finished unexpectedly.");
}


void tagger_info::load_tag_frequencies(const char *f)
{
	std::ifstream file;
	char article[1024];
	vector<string> data;
	int value;
	TMapPStInt tag_freqs_tmp;
	TMapStInt tot_freqs_tmp;

	file.open(f);
	if (!file.bad()) {
		std::cerr << "Loading tagging frequencies from \'" << f << "\'." << std::endl;
		while (file.good()) {
			file.getline(article, 1024);
			boost::split(data, article, boost::is_any_of("\t"));
			if (data.size() == 3) {
				value = boost::lexical_cast<int>(data.at(2));
				tag_freqs_tmp[std::make_pair(data.at(0), data.at(1))] += value;
				tot_freqs_tmp[data.at(0)] += value;
				//std::cout << data.at(0) << " " << data.at(1) << " "<< data.at(2) << std::endl;
			}
		}
	} else
		throw std::length_error(std::string("File") + f + " finished unexpectedly.");
	file.close();

	TMapPStInt::iterator tagiter = tag_freqs_tmp.begin();
	for (; tagiter != tag_freqs_tmp.end(); ++tagiter) {
		double tot_tmp = get_map_element(tot_freqs_tmp, tagiter->first.first);
		tag_freqs[make_pair(tagiter->first.first, tagiter->first.second)] = tagiter->second / tot_tmp * exp(-1 / sqrt(tot_tmp));
	}
}

void tagger_info::load_substitutions(const char *f)
{
	std::ifstream file;
	char article[1024];
	vector<string> data;

	file.open(f);
	if (!file.bad()) {
		std::cerr << "Loading substitutions from \'" << f << "\'." << std::endl;
		while (file.good()) {
			file.getline(article, 1024);
			boost::split(data, article, boost::is_any_of("\t"));
			if (data.size() == 2) {
				vector<string> trigger_vect;
				boost::split(trigger_vect, data.at(0), boost::is_any_of("_"));
				substitutions_[trigger_vect] = data.at(1);
			}
		}
	} else
		throw std::length_error(std::string("File") + f + " finished unexpectedly.");

	file.close();
}

void tagger_info::load_tag_frequencies_back(const char *f)
{
	std::ifstream file;
	char article[1024];
	vector<string> data;
	int value;
	TMapPStInt tag_freqs_tmp;
	TMapStInt tot_freqs_tmp;

	file.open(f);
	if (!file.bad()) {
		std::cerr << "Loading tagging frequencies from \'" << f << "\'." << std::endl;
		while (file.good()) {
			file.getline(article, 1024);
			boost::split(data, article, boost::is_any_of("\t"));
			if (data.size() == 3) {
				value = boost::lexical_cast<int>(data.at(2));
				tag_freqs_tmp[std::make_pair(data.at(0), data.at(1))] += value;
				tot_freqs_tmp[data.at(0)] += value;
				//std::cout << data.at(0) << " " << data.at(1) << " "<< data.at(2) << std::endl;
			}
		}
	} else
		throw std::length_error(std::string("File") + f + " finished unexpectedly.");
	file.close();

	TMapPStInt::iterator tagiter = tag_freqs_tmp.begin();
	for (; tagiter != tag_freqs_tmp.end(); ++tagiter) {
		double tot_tmp = get_map_element(tot_freqs_tmp, tagiter->first.first);
		tag_freqs_back[make_pair(tagiter->first.first, tagiter->first.second)] = tagiter->second / tot_tmp
				* exp(-1 / sqrt(tot_tmp));
	}
}

void tagger_info::load_tag_frequencies_prior(const char *f)
{
	std::ifstream file;
	char article[1024];
	vector<string> data;
	int value;
	TMapPStInt tag_freqs_tmp;
	TMapStInt tot_freqs_tmp;

	file.open(f);
	if (!file.bad()) {
		std::cerr << "Loading tagging frequencies from \'" << f << "\'." << std::endl;
		while (file.good()) {
			file.getline(article, 1024);
			boost::split(data, article, boost::is_any_of("\t"));
			if (data.size() == 3) {
				value = boost::lexical_cast<int>(data.at(2));
				tag_freqs_tmp[std::make_pair(data.at(0), data.at(1))] += value;
				tot_freqs_tmp[data.at(0)] += value;
			}
		}
	} else
		throw std::length_error(std::string("File") + f + " finished unexpectedly.");
	file.close();

	TMapPStInt::iterator tagiter = tag_freqs_tmp.begin();
	for (; tagiter != tag_freqs_tmp.end(); ++tagiter) {
		double tot_tmp = get_map_element(tot_freqs_tmp, tagiter->first.first);
		tag_freqs_prior[make_pair(tagiter->first.first, tagiter->first.second)] = tagiter->second / tot_tmp
				* exp(-1 / sqrt(tot_tmp));
	}
}

void tagger_info::load_frequencies(const char *f)
{
	std::ifstream file;
	char article[1024];
	vector<string> data;
	int value;
	string old_name = "";
	TMapStInt local_tag_num;

	file.open(f);
	if (!file.bad()) {
		std::cerr << "Loading tagging frequencies from \'" << f << "\'." << std::endl;
		while (file.good()) {
			file.getline(article, 1024);
			boost::split(data, article, boost::is_any_of("\t"));
			if (data.size() == 4) {
				value = boost::lexical_cast<int>(data.at(3));
				freqs[std::make_pair(data.at(0), std::make_pair(data.at(1), data.at(2)))] = value;
				tot_nums[data.at(0)] += value;
				if (old_name != data.at(0)) {
					TMapStInt::iterator liter = local_tag_num.begin(), lend = local_tag_num.end();
					pair<string, int> chosen_tag(make_pair("", 0));
					for (; liter != lend; ++liter) {
						if (chosen_tag.second < liter->second)
							chosen_tag = make_pair(liter->first, liter->second);
					}
					last_hope_tags_[old_name] = chosen_tag.first;
					local_tag_num = TMapStInt();
					old_name = data.at(0);
				}
				++local_tag_num[data.at(1)];
			}
		}
	} else
		throw std::length_error(std::string("File") + f + " finished unexpectedly.");
	file.close();
}

void tagger_info::load_frequencies_back(const char *f)
{
	std::ifstream file;
	char article[1024];
	vector<string> data;
	int value;

	file.open(f);
	if (!file.bad()) {
		std::cerr << "Loading tagging frequencies from \'" << f << "\'." << std::endl;
		while (file.good()) {
			file.getline(article, 1024);
			boost::split(data, article, boost::is_any_of("\t"));
			if (data.size() == 4) {
				value = boost::lexical_cast<int>(data.at(3));
				freqs_back[std::make_pair(data.at(0), std::make_pair(data.at(1), data.at(2)))] = value;
				tot_nums_back[data.at(0)] += value;
			}
		}
	} else
		throw std::length_error(std::string("File") + f + " finished unexpectedly.");
	file.close();
}

void tagger_info::load_frequencies_prior(const char *f)
{
	std::ifstream file;
	char article[1024];
	vector<string> data;
	int value;

	file.open(f);
	if (!file.bad()) {
		std::cerr << "Loading tagging frequencies from \'" << f << "\'." << std::endl;
		while (file.good()) {
			file.getline(article, 1024);
			boost::split(data, article, boost::is_any_of("\t"));
			if (data.size() == 4) {
				value = boost::lexical_cast<int>(data.at(3));
				freqs_prior[std::make_pair(data.at(0), std::make_pair(data.at(1), data.at(2)))] = value;
				tot_nums_prior[data.at(0)] += value;
			}
		}
	} else
		throw std::length_error(std::string("File") + f + " finished unexpectedly.");
	file.close();
}

void tagger_info::load_conjugations(const char *f)
{
	std::ifstream file;
	char article[1024];
	vector<string> data;
	int value;

	file.open(f);
	if (!file.bad()) {
		std::cerr << "Loading plurals and tenses from \'" << f << "\'." << std::endl;
		while (file.good()) {
			file.getline(article, 1024);
			boost::split(data, article, boost::is_any_of("\t"));
			if (data.size() >= 3) {
				conjug[std::make_pair(data.at(0), data.at(2))] = data.at(1);
				conjug_base[data.at(1)].push_back(data.at(2));
				conj_tag[data.at(0)].push_back(data.at(2));
				irregular_conj_[make_pair(data.at(1), data.at(2))] = data.at(0);
			}
		}
	} else
		throw std::length_error(std::string("File") + f + " finished unexpectedly.");
}

void tagger_info::complete_regular_verbs()
{
	// Create all the regular conjugations
	TMapPInt::iterator freqs_iter = freqs.begin();
	TMapPInt::iterator freqs_end = freqs.end();
	vector<char> vowels;
	vector<string> existing_tags;
	vowels.push_back('a');
	vowels.push_back('e');
	vowels.push_back('i');
	vowels.push_back('o');
	vowels.push_back('u');
	vowels.push_back('y');

	string final_string;
	for (; freqs_iter != freqs_end; ++freqs_iter) {
		string base_str = freqs_iter->first.first;
		string tag_str = freqs_iter->first.second.first;
		;
		if (is_verb(tag_str)) {
			TMapStVSt::iterator base_iter = conjug_base.find(base_str);
			if (base_iter != conjug_base.end())
				existing_tags = base_iter->second;
			else
				existing_tags = vector<string>();
			if (find(existing_tags.begin(), existing_tags.end(), "VBG") == existing_tags.end()) { // if the gerundive has not been declared
				final_string = base_str + "ing";
				conjug_base[base_str].push_back("VBG");
				conjug[std::make_pair(final_string, "VBG")] = base_str;
				conj_tag[final_string].push_back("VBG");
			}
			if (find(existing_tags.begin(), existing_tags.end(), "VBD") == existing_tags.end()) { // if the simple past has not been declared
				if (base_str.size() > 2 && base_str.at(base_str.size() - 1) == 'y'
						&& find(vowels.begin(), vowels.end(), base_str.at(base_str.size() - 2)) != vowels.end()) {
					base_str.erase(base_str.size() - 1);
					final_string = base_str + "ied";
				} else
					final_string = base_str + "ed";
				conjug_base[base_str].push_back("VBD");
				conjug[std::make_pair(final_string, "VBD")] = base_str;
				conj_tag[final_string].push_back("VBD");
			}
			if (find(existing_tags.begin(), existing_tags.end(), "VBN") == existing_tags.end()) { // if the past participle has not been declared
				if (base_str.size() > 2 && base_str.at(base_str.size() - 1) == 'y'
						&& find(vowels.begin(), vowels.end(), base_str.at(base_str.size() - 2)) != vowels.end()) {
					base_str.erase(base_str.size() - 1);
					final_string = base_str + "ied";
				} else
					final_string = base_str + "ed";
				conjug_base[base_str].push_back("VBN");
				conjug[std::make_pair(final_string, "VBN")] = base_str;
				conj_tag[final_string].push_back("VBN");
			}
			if (find(existing_tags.begin(), existing_tags.end(), "VBZ") == existing_tags.end()) { // if the past participle has not been declared
				if (base_str.size() > 2 && base_str.at(base_str.size() - 1) == 'y'
						&& find(vowels.begin(), vowels.end(), base_str.at(base_str.size() - 2)) != vowels.end()) {
					base_str.erase(base_str.size() - 1);
					final_string = base_str + "ies";
				} else
					final_string = base_str + "s";
				conjug_base[base_str].push_back("VBZ");
				conjug[std::make_pair(final_string, "VBZ")] = base_str;
				conj_tag[final_string].push_back("VBZ");
			}
		}
		if (is_name(tag_str)) {
			TMapStVSt::iterator base_iter = conjug_base.find(base_str);
			if (base_iter != conjug_base.end())
				existing_tags = base_iter->second;
			else
				existing_tags = vector<string>();
			if (find(existing_tags.begin(), existing_tags.end(), "NNS") == existing_tags.end()) { // if the plural has not been declared
				if (base_str.size() > 2 && base_str.at(base_str.size() - 1) == 'y'
						&& find(vowels.begin(), vowels.end(), base_str.at(base_str.size() - 2)) != vowels.end()) {
					base_str.erase(base_str.size() - 1);
					final_string = base_str + "ies";
				} else
					final_string = base_str + "s";
				conjug_base[base_str].push_back("NNS");
				conjug[std::make_pair(final_string, "NNS")] = base_str;
				conj_tag[final_string].push_back("NNS");
			}
			if (find(existing_tags.begin(), existing_tags.end(), "NNPS") == existing_tags.end()) { // if the plural has not been declared
				if (base_str.size() > 2 && base_str.at(base_str.size() - 1) == 'y'
						&& find(vowels.begin(), vowels.end(), base_str.at(base_str.size() - 2)) != vowels.end()) {
					base_str.erase(base_str.size() - 1);
					final_string = base_str + "ies";
				} else
					final_string = base_str + "s";
				conjug_base[base_str].push_back("NNPS");
				conjug[std::make_pair(final_string, "NNPS")] = base_str;
				conj_tag[final_string].push_back("NNPS");
			}
		}
	}
}

static inline bool has_ending(const string &word, const string &ending)
{
	if (word.length() >= ending.length())
		if (word.compare(word.length() - ending.length(), ending.length(), ending) == 0)
			return true;
	return false;
}

static inline bool has_ending(const string &word, const vector<string> &endings)
{
	for (int n = 0; n < endings.size(); ++n) {
		if (word.length() >= endings.at(n).length())
			if (word.compare(word.length() - endings.at(n).length(), endings.at(n).length(), endings.at(n)) == 0)
				return true;
	}
	return false;
}

double tagger_info::regular_tag(const string &word, const string &tag, string *base)
{
	if (word.size() < 2) {
		if (tag == "NN")
			return 1;
		else
			return 0; // single letters are always names
	}

	if (find(except_.begin(), except_.end(), word) != except_.end())
		return 0;

	// Create all the regular conjugations
	vector<char> vowels;
	vowels.push_back('a');
	vowels.push_back('e');
	vowels.push_back('i');
	vowels.push_back('o');
	vowels.push_back('u');
	vowels.push_back('y');

	string final_string;

	int num_candidates = 0;
	vector<string> candidate_tag;
	vector<string> bases;

	// if it is a VBG candidate
	if (has_ending(word, "ing")) {
		int size = word.size();
		string base = word.substr(0, size - 3);
		string base2 = base + "e";
		if (tot_nums.find(base) != tot_nums.end()
		//&& this->get_conj(base,"VB") != ""
				) {
			++num_candidates;
			candidate_tag.push_back("VBG");
			bases.push_back(base);
		}
		if (tot_nums.find(base2) != tot_nums.end()
		//&& this->get_conj(base,"VB") != ""
				) {
			++num_candidates;
			candidate_tag.push_back("VBG");
			bases.push_back(base2);
		}
	}

	// if it is a VBN candidate
	if (has_ending(word, "ed")) {
		int size = word.size();
		string base = word.substr(0, size - 2);
		string base2 = base;
		string base3 = base;
		base3 += "e";
		size = base2.size() - 1;
		if (base2.size() && base2.at(size) == 'i' && size > 2)
			base2.at(size) = 'y';
		if (tot_nums.find(base) != tot_nums.end()) {
			++num_candidates;
			candidate_tag.push_back("VBN");
			bases.push_back(base);
		} else if (tot_nums.find(base2) != tot_nums.end()) {
			++num_candidates;
			candidate_tag.push_back("VBN");
			bases.push_back(base2);
		} else if (tot_nums.find(base3) != tot_nums.end()) {
			++num_candidates;
			candidate_tag.push_back("VBN");
			bases.push_back(base3);
		}
	}

	// if it is a VBD candidate
	if (has_ending(word, "ed")) {
		int size = word.size();
		string base = word.substr(0, size - 2);
		string base2 = base;
		string base3 = base;
		base3 += "e";
		size = base2.size() - 1;
		if (base2.at(size) == 'i' && size > 2)
			base2.at(size) = 'y';
		if (tot_nums.find(base) != tot_nums.end()) {
			++num_candidates;
			candidate_tag.push_back("VBD");
			bases.push_back(base);
		} else if (tot_nums.find(base2) != tot_nums.end()) {
			++num_candidates;
			candidate_tag.push_back("VBD");
			bases.push_back(base2);
		} else if (tot_nums.find(base3) != tot_nums.end()) {
			++num_candidates;
			candidate_tag.push_back("VBD");
			bases.push_back(base3);
		}
	}

	// if it is a VBZ candidate
	if (has_ending(word, "s")) {
		int size = word.size();
		string base = word.substr(0, size - 1);
		string base2 = base;
		size = base2.size() - 1;
		if (base2.at(size) == 'i' && size > 2 && find(vowels.begin(), vowels.end(), base2.at(size - 1)) == vowels.end())
			base2.at(size) = 'y';
		if (tot_nums.find(base) != tot_nums.end()) {
			++num_candidates;
			candidate_tag.push_back("VBZ");
			bases.push_back(base);
		} else if (tot_nums.find(base2) != tot_nums.end()) {
			++num_candidates;
			candidate_tag.push_back("VBZ");
			bases.push_back(base2);
		}
	}
	// if it is a NNS candidate
	if (has_ending(word, "s")) {
		int size = word.size();
		string base = word.substr(0, size - 1);
		string base2 = base;
		size = base2.size() - 1;
		if (base2.at(size) == 'i' && size > 2 && find(vowels.begin(), vowels.end(), base2.at(size - 1)) == vowels.end())
			base2.at(size) = 'y';
		if (tot_nums.find(base) != tot_nums.end()) {
			++num_candidates;
			candidate_tag.push_back("NNS");
			bases.push_back(base);
			candidate_tag.push_back("NNPS");
			bases.push_back(base);
		} else if (tot_nums.find(base2) != tot_nums.end()) {
			++num_candidates;
			candidate_tag.push_back("NNS");
			bases.push_back(base2);
			candidate_tag.push_back("NNPS");
			bases.push_back(base2);
		}
	}

	vector<string>::iterator citer = find(candidate_tag.begin(), candidate_tag.end(), tag);
	if (citer != candidate_tag.end()) {
		int d = distance(candidate_tag.begin(), citer);
		*base = bases.at(d);
		return 1. / num_candidates;
	}
	if (num_candidates > 0) {
		*base = "[wrong_tag]";
		return 0;
	}
	if ((tag == "NN" || tag == "NNP") && this->is_candidate_name(word)) {
		*base = word;
		return 1;
	}
	if ((tag == "VB" || tag == "VBP") && this->is_candidate_verb(word)) {
		*base = word;
		return 1;
	}
	return 0;
}

bool tagger_info::is_auxiliary_verb(const string &word)
{
	vector<string> tags;
	tags.push_back("VBZ");
	tags.push_back("VBP");
	tags.push_back("VBD");
	tags.push_back("VBN");

	for (int n = 0; n < tags.size(); ++n) {
		string base = this->get_conj(word, tags.at(n));
		if (base == "")
			base = word;
		if (base == "have" || base == "be" || base == "do") {
			return true;
		}
	}

	return false;
}

bool tagger_info::is_modal_verb(const string &word)
{
	string base;
	base = word;
	// Modals are also auxiliaries
	if (base == "can" || base == "could" || base == "may" || base == "might" || base == "shall" || base == "should"
			|| base == "will" || base == "would") {
		return true;
	}

	return false;
}
bool tagger_info::is_candidate_verb(const string &word)
{
	vector<string> exceptions;
	exceptions.push_back("still");

	metric *d = metric_singleton::get_metric_instance();
	if (!shortfind(exceptions,word) && d->has_verb(word)) {
		return true;
	}

	// This is how you can tell if a word can be a verb
	if (freqs.find(std::make_pair(word, std::make_pair("VB", "TO"))) != freqs.end()) {
		return true;
	}
	return false;
}

bool tagger_info::is_candidate_name(const string &word)
{
	// This is how you can tell if a word can be a verb
	if (freqs.find(std::make_pair(word, std::make_pair("NN", "DT"))) != freqs.end())
		return true;
	if (freqs.find(std::make_pair(word, std::make_pair("NNS", "DT"))) != freqs.end())
		return true;
	if (freqs.find(std::make_pair(word, std::make_pair("NNP", "DT"))) != freqs.end())
		return true;
	if (freqs.find(std::make_pair(word, std::make_pair("NNPS", "DT"))) != freqs.end())
		return true;
	return false;
}

string tagger_info::unique_word(const string &word)
{
	TMapStSt::iterator miter = unique_words_.find(word);
	if (miter != unique_words_.end()) {
		return miter->second;
	}
	return "";
}
vector<string> tagger_info::getCandidateTags(const string &str)
{
	vector<string> to_return;
	TMapStVSt::iterator citer = candidate_tags_.find(str);

	if (citer != candidate_tags_.end()) {
		to_return = citer->second;
	}

	return to_return;
}

string tagger_info::getSureTag(const string &str) const
{
	string to_return;
	TMapStSt::const_iterator citer = sure_tags_.find(str);

	if (citer != sure_tags_.end()) {
		to_return = citer->second;
	} else
		to_return = "";

	return to_return;
}

bool tagger_info::isSureTag(const string &str) const
{
	string to_return;
	TMapStSt::const_iterator citer = sure_tags_.find(str);
	if (citer != sure_tags_.end())
		return true;
	return false;
}

string tagger_info::conjugate(string base, const string &tag)
// conjugate base according to tag
{
	if (base.size() == 0)
		return "";

	string conj = base;

	// search for an irregular conjugation
	TMapPStSt::iterator irr_iter = irregular_conj_.find(make_pair(base, tag));
	if (irr_iter != irregular_conj_.end()) {
		conj = irr_iter->second;
		return conj;
	}
	map<pair<string, string>, string> strange_elements;
	strange_elements[make_pair("find", "VBD")] = "found";
	strange_elements[make_pair("find", "VBN")] = "found";
	strange_elements[make_pair("do", "VBP")] = "do";
	strange_elements[make_pair("do", "VB")] = "do";
	strange_elements[make_pair("have", "VB")] = "have";
	strange_elements[make_pair("have", "VBP")] = "have";
	strange_elements[make_pair("have", "VBZ")] = "has";
	strange_elements[make_pair("be", "VB")] = "be";
	strange_elements[make_pair("be", "VBZ")] = "is";
	strange_elements[make_pair("be", "VBP")] = "am";
	strange_elements[make_pair("be", "VBG")] = "being";
	strange_elements[make_pair("see", "VBG")] = "seeing";
	strange_elements[make_pair("born", "VBN")] = "born";
	strange_elements[make_pair("people", "NNS")] = "people";
	strange_elements[make_pair("specie", "NN")] = "species";
	strange_elements[make_pair("specie", "NNP")] = "species";
	strange_elements[make_pair("specie", "NNS")] = "species";
	strange_elements[make_pair("specie", "NNPS")] = "species";
	strange_elements[make_pair("star", "NNS")] = "stars";
	strange_elements[make_pair("star", "NNPS")] = "stars";
	map<pair<string, string>, string>::iterator miter = strange_elements.find(make_pair(base, tag));
	if (miter != strange_elements.end()) {
		conj = miter->second;
		return conj;
	}

	// Create all the regular conjugations
	vector<string> vowels;
	vowels.push_back("a");
	vowels.push_back("e");
	vowels.push_back("i");
	vowels.push_back("o");
	vowels.push_back("u");
	vowels.push_back("y");

	bool vow_trigger = false;
	bool prior_vow_trigger = false;
	string base2 = base;
	//puts("LAST:::");
	char last_char = base.at(base.size() - 1);
	//puts("LAST2:::");

	if (base == "don" && tag != "NN" && tag != "NNP" && tag != "NNS" && tag != "NNPS")
		base = "do";

	if (base2 == "don" && tag != "NN" && tag != "NNP" && tag != "NNS" && tag != "NNPS")
		base2 = "do";

	metric *d = metric_singleton::get_metric_instance();
	if (has_ending(base, vowels) && base.size() > 1) {
		base2 = base.substr(0, base.size() - 1);
		vow_trigger = true;
	}
	if (has_ending(base2, vowels))
		prior_vow_trigger = true;

	if (tag == "VBG") {
		if (vow_trigger)
			conj = base2 + "ing";
		else
			conj = base + "ing";
	}
	if (tag == "VBD") {
		if (vow_trigger)
			conj = base2 + "ed";
		else
			conj = base + "ed";
	}
	if (tag == "VBN") {
		if (vow_trigger)
			conj = base2 + "ed";
		else
			conj = base + "ed";
	}
	if (tag == "VBZ") {
		conj = base + "s";
	}
	if (tag == "NNS" || tag == "NNPS") {
		if (d->has_noun(base + 'e')) {
			base += "e";
		}
		if (d->has_noun(base2 + 'e')) {
			base2 += "e";
		}
		if (last_char == 's')
			conj = base;
		else if (last_char == 'y' && prior_vow_trigger) {
			conj = base + "s";
		} else if (last_char == 'y' && !prior_vow_trigger) {
			conj = base2 + "ies";
		} else if (vow_trigger && prior_vow_trigger) {
			conj = base2 + "s";
		} else {
			conj = base + "s";
		}
	}
	return conj;
}

string tagger_info::get_base_superlative(string str)
{
	TMapStSt irregulars;
	irregulars["better"] = "good";
	irregulars["worse"] = "bad";
	irregulars["greater"] = "big";

	TMapStSt::iterator miter = irregulars.find(str);
	if (miter != irregulars.end()) {
		return miter->second;
	}

	// Create all the regular conjugations
	vector<string> vowels;
	vowels.push_back("a");
	vowels.push_back("e");
	vowels.push_back("i");
	vowels.push_back("o");
	vowels.push_back("u");
	vowels.push_back("y");

	string base = str.substr(0, str.size() - 2);
	string to_return = base;
	if (has_ending(base, vowels)) {
		string base2 = base.substr(0, base.size() - 1);
		if (!has_ending(base2, vowels)) {
			to_return = base2 + "y";
		}
	}
	metric *d = metric_singleton::get_metric_instance();
	if (d->is_adjective(to_return + 'e')) {
		to_return = to_return + 'e';
	}

	return to_return;
}



vector<int> tagger_info::find_wikidata_q(const string &key, const string &tag)
{
	vector<int> to_return;


	TMapStVInt::iterator miter = q_values_.find(key);
	if (miter != q_values_.end()) {
		to_return = miter->second;
	}	

	if(key.find("the_") == 0) {
		string new_key = key.substr(4,key.size());
		TMapStVInt::iterator miter2 = q_values_.find(new_key);
		if (miter2 != q_values_.end()) {
			to_return.insert(to_return.begin(),miter2->second.begin(),miter2->second.end());
		}
	}
	if(tag == "NNS" || tag == "NNPS") {
		string new_key = this->get_conj(key,tag);
		TMapStVInt::iterator miter2 = q_values_.find(new_key);
		if (miter2 != q_values_.end()) {
			to_return.insert(to_return.begin(),miter2->second.begin(),miter2->second.end());
		}
	}

	return to_return;
}


string tagger_info::get_wikidata_name(int key)
{
	string to_return;

	TMapIntSt::iterator miter = q_names_.find(key);
	if (miter != q_names_.end()) {
		return miter->second;
	}
	return to_return;
}

