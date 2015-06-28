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



#include"tagger.hpp"
#include"tagger_aux.hpp"

using std::map;
using std::pair;
using std::vector;
using std::make_pair;

const bool debug = false;
const bool activate_context = false;
// boost::mutex io_mutex_tagger;

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

tagger::tagger(tagger_info *ti, WisdomInfo wi) :
		info(ti), context_(0), wi_(wi)
{
	fill_tags();
}

void tagger::fill_tags()
{
	tags.push_back("CC");
	tags.push_back("CD");
	tags.push_back("DT");
	tags.push_back("EX");
	tags.push_back("FW");
	tags.push_back("IN");
	tags.push_back("JJ");
	tags.push_back("JJR");
	tags.push_back("JJS");
	tags.push_back("LS");
	tags.push_back("MD");
	tags.push_back("NN");
	tags.push_back("NNS");
	tags.push_back("NNP");
	tags.push_back("NNPS");
	tags.push_back("PDT");
	tags.push_back("POS");
	tags.push_back("PRP");
	tags.push_back("PRP$");
	tags.push_back("RB");
	tags.push_back("RBR");
	tags.push_back("RBS");
	tags.push_back("RP");
	tags.push_back("TO");
	tags.push_back("UH");
	tags.push_back("VB");
	tags.push_back("VBD");
	tags.push_back("VBG");
	tags.push_back("VBN");
	tags.push_back("VBP");
	tags.push_back("VBZ");
	tags.push_back("WDT");
	tags.push_back("WP");
	tags.push_back("WP$");
	tags.push_back("WRB");

	// Puntuation tags
	punctuation_tags.push_back("#");
	punctuation_tags.push_back("$");
	punctuation_tags.push_back("\"");
	punctuation_tags.push_back("/");
	punctuation_tags.push_back("''");
	punctuation_tags.push_back("``");
	punctuation_tags.push_back(")");
	punctuation_tags.push_back("(");
	punctuation_tags.push_back("-comma-");
	punctuation_tags.push_back("-period-");
	//punctuation_tags.push_back("-QM-"); // question mark (my addition)
	punctuation_tags.push_back("-colon-");
}

static bool is_to_delete(string s)
{
	if (s.find(" ") != string::npos || s.size() == 0)
		return true;
	if (static_cast<int>(s.at(0)) < 32 || static_cast<int>(s.at(0)) > 128)
		return true;
	return false;
}

static vector<string> divide_in_words(string &text)
{
	string non_words = "!@$€£%^&*(){}:;\"\'.,<>~`\\|?="; // the symbols '[]_/#' are missing
	string capital = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	string small = "abcdefghijklmnopqrstuvwxyz";
	string numbers = "1234567890";
	int size = text.size();
	int incr;

	for (incr = 0; incr < size; ++incr) {
		if (text.at(incr) == ','
				&& incr > 0
				&& numbers.find(text.at(incr - 1)) != string::npos
				&& incr > 0
				&& numbers.find(text.at(incr + 1)) != string::npos) { // 5,000
			text.erase(incr, 1);
			size = text.size();
			continue;
		} else if ((text.at(incr) == '.' || text.at(incr) == ':') && incr > 0 && numbers.find(text.at(incr - 1)) != string::npos
				&& incr < size - 1 && numbers.find(text.at(incr + 1)) != string::npos) { // 52.231 or 22:00
			continue;
		} else if (non_words.find(text.at(incr)) != string::npos) {
			if (incr < size - 2 && text.at(incr + 1) == '_') // this is necessary for the [verbatim] case.)
				continue;
			string test_str = text.substr(0, incr);
			bool is_verbatim = false;
			for (int n = test_str.size() - 1; n >= 0; --n) {
				if (test_str.at(n) == '_') {
					is_verbatim = true;
					break;
				}
				if (test_str.at(n) == ' ')
					break;
			} // this is necessary for the [verbatim] case, as in: he said "he didn't drive"
			if (is_verbatim)
				continue;

			if (text.at(incr) != '\'' || numbers.find(text.at(incr + 1)) != string::npos) // takes in consideration '' and '80s
				text.insert(incr + 1, " ");
			text.insert(incr, " ");
			size = text.size();
			++incr;
		}
	}

	vector<string> text_vect;
	boost::split(text_vect, text, boost::is_any_of(" \t\n\r"));
	vector<string>::iterator where = text_vect.begin();
	while ((where = find_if(where, text_vect.end(), is_to_delete)) != text_vect.end()) {
		where = text_vect.erase(where);
	}
	return text_vect;
}

static vector<string> replace_commas(const vector<string> &str)
{
	vector<string> ret_str(str);
	vector<string>::iterator i = ret_str.begin();

	for (; i != ret_str.end(); ++i) {
		if (*i == ",")
			*i = string("-comma-");
		if (*i == "-")
			*i = string("-comma-");
		if (*i == ".")
			*i = string("-period-");
		//if (*i== "?") *i= string("-QM-");
		if (*i == ":")
			*i = string("-colon-");
	}

	return ret_str;
}

static vector<string> make_substitutions(vector<string> words, TMapVStSt subst)
{
	vector<string>::iterator tags_iter_begin;
	vector<string>::iterator tags_iter_end;
	tags_iter_begin = words.begin();

	for (; tags_iter_begin != words.end(); ++tags_iter_begin) {
		tags_iter_end = words.end();
		for (; tags_iter_end != tags_iter_begin; --tags_iter_end) {
			vector<string> tmp_vect(tags_iter_begin, tags_iter_end);

			/// You should to_lower all the elements of tmp_vect
			for (int n = 0; n < tmp_vect.size(); ++n) {
				string wordstr = tmp_vect.at(n);
				for (int i = 0; i < wordstr.size(); ++i) {
					char tmp_char = std::tolower(wordstr.at(i));
					wordstr.at(i) = tmp_char;
				}
				tmp_vect.at(n) = wordstr;
			}
			if (subst.find(tmp_vect) != subst.end()) {
				string subst_with = subst.find(tmp_vect)->second;
				words.erase(tags_iter_begin, tags_iter_begin + tmp_vect.size());
				words.insert(tags_iter_begin, subst_with);
				tags_iter_end = words.end();
			}
		}
	}

	return words;
}

static vector<string> change_special_words(vector<string> words)
// re-employ -> employ
{
	metric *d = metric_singleton::get_metric_instance();

	vector<string>::iterator words_iter;
	words_iter = words.begin();

	for (; words_iter != words.end(); ++words_iter) {
		string word = *words_iter;
		if(words.size() && word.find("re-") == 0) {
			string lemma= word.substr(3,word.size());
			if(!d->has_noun(word) && !d->has_verb(word) && ( d->has_verb(lemma) || d->has_noun(lemma) ) ) {
				*words_iter = lemma;
			}
		}
	}

	return words;
}

static string sign_apostrophes(string str)
// Change apostrophes into quotation marks (to be processed by sign_verbatim later)
{
	while (true) {
		int start = str.find("\'");
		if (start != string::npos && start >= 0 && start < str.size() - 1 && str.at(start - 1) == ' '
				&& str.at(start + 1) != ' ') {
			int end = str.find("\'", start + 1);
			if (end != string::npos && str.at(end - 1) != ' ') {
				str.at(start) = '\"';
				str.at(end) = '\"';
			} else
				break;
		} else
			break;
	}

	while (true) {
		int start = str.find("`");
		if (start != string::npos && start >= 0 && start < str.size() - 1 && str.at(start - 1) == ' '
				&& str.at(start + 1) != ' ') {
			int end = str.find("`", start + 1);
			if (end != string::npos && str.at(end - 1) != ' ') {
				str.at(start) = '\"';
				str.at(end) = '\"';
			} else
				break;
		} else
			break;
	}

	return str;
}

static string sign_traces(string str)
{
	while (true) {
		int start = str.find("-");
		if (start != string::npos && start > 0 && start < str.size() - 1 && str.at(start - 1) == ' '
				&& str.at(start + 1) == ' ') {
			int end = str.find("-", start + 1);
			if (end != string::npos && str.at(end - 1) == ' ' && str.at(end + 1) == ' ') {
				str.at(start) = '(';
				str.at(end) = ')';
			} else
				break;
		} else
			break;
	}

	return str;
}

static string substitute_commas(string str)
{
	while (true) {
		int start = str.find(",");
		if (start != string::npos && start < str.size()) {
			str.replace(start, 1, "-comma-");
		}
		if (start == string::npos)
			break;
	}
	// change the ( and ) so that )_ cannot be present. It would cause recursion in unifier
	while (true) {
		int start = str.find("(");
		if (start != string::npos && start < str.size()) {
			str.replace(start, 1, "-LBR-");
		}
		if (start == string::npos)
			break;
	}
	while (true) {
		int start = str.find(")");
		if (start != string::npos && start < str.size()) {
			str.replace(start, 1, "-RBR-");
		}
		if (start == string::npos)
			break;
	}

	return str;
}

static string sign_verbatim_quotes(string str)
{
	while (true) {
		int start = str.find("\"");
		if (start == string::npos)
			start = str.find("`");
		if (start != string::npos) {
			int tmp_start = str.find("`");
			if (tmp_start != string::npos && tmp_start < start) {
				start = tmp_start;
			}
		}
		if (start != string::npos) {
			int end = str.find("\"", start + 1);
			if (end != string::npos) {
				string verbatim = str.substr(start + 1, end - start - 1);
				for (int n = 0; n < verbatim.size(); ++n)
					if (verbatim.at(n) == ' ')
						verbatim.at(n) = '_';
				verbatim = "[verbatim]_" + substitute_commas(verbatim);
				if (end > 0 && end < str.size() - 1
					&& (str.at(end + 1) == ',' || str.at(end + 1) == '.' || str.at(end + 1) == ':' || str.at(end + 1) == '?' || str.at(end + 1) == '!')
				) {
					str.insert(end + 1, " ");
				}
				str.replace(start, end - start + 1, verbatim);

			} else {
				str += "\"";
				end = str.size() - 1;
				string verbatim = str.substr(start + 1, end - start - 1);
				for (int n = 0; n < verbatim.size(); ++n)
					if (verbatim.at(n) == ' ')
						verbatim.at(n) = '_';
				verbatim = "[verbatim]_" + substitute_commas(verbatim);
				str.replace(start, end - start + 1, verbatim);
				break;
			}
		} else
			break;
	}

	return str;
}

void tagger::set_phrase(string &txt)
{
	is_question_ = false;

	int i;
	phrase = "";

	Filter filter(txt);
	txt = filter.str();

	NumeralCast ncast(txt);
	string s = ncast.result();

	s = sign_traces(s);
	s = sign_verbatim_quotes(s);
	s = sign_apostrophes(s);  // This must go before sign_verbatim()
	s = sign_verbatim_quotes(s);

	words_orig = replace_commas(divide_in_words(s));
	words_orig = make_substitutions(words_orig, info->getSubstitutionsMap());
	for (i = 0; i < s.size(); ++i) {
		char tmp_char = std::tolower(s.at(i));
		phrase.push_back(tmp_char);
	}
	vector<string> tmp_str = divide_in_words(phrase);
	words = replace_commas(tmp_str);
	words = make_substitutions(words, info->getSubstitutionsMap());
	words = change_special_words(words);

	if (words.size() > 1 && words.at(words.size() - 1) == "?") {
		is_question_ = true;
	}

}

void tagger::init_random_tags()
{
	int size_tags = tags.size();
	int size_words = words.size();

	//init_ran(time(0));

	int n, tag_num;
	word_tags.at(0) = ""; /// Change this, it can be any punctuation
	for (n = 1; n < size_words + 2; ++n) {
		tag_num = dran() * size_tags;
		word_tags.at(n) = tags.at(tag_num);
	}
}

static bool is_number(string s)
{
	for (int n = 0; n < s.size(); ++n)
		if (isdigit(s.at(n))) {
			if (n > 0 && !isdigit(s.at(n - 1)))
				return false;
			if (n == 0)
				return true;
		}
	return false;
}


static bool has_hashtag(const string &str)
{
	if (str.find("#") != string::npos)
		return true;
	return false;
}

void tagger::step(vector<bool> &mask)
{
	vector<string> new_tags, new_tags2;
	int size_tags = tags.size();
	int size_words = words.size();
	double r, prob_before, prob_after, prob_after2;
	int i, tag_num;
	string candidate_tag, candidate_tag2;
	bool aux = false, who_trigger = false, if_trigger = false;
	metric *d = metric_singleton::get_metric_instance();

	vector<string> chrono = info->getChronoNames();

	for (i = 0; i < size_words; ++i) {
		if (word_tags.at(i + 1) == "POS" && words.at(i) == "\'s"
				&& (word_tags.at(i) == "WP" || word_tags.at(i) == "WRB" || word_tags.at(i) == "WDT" || word_tags.at(i) == "DT"
						|| word_tags.at(i) == "EX" || words.at(i - 1) == "that")
		) {
			word_tags.at(i + 1) = "VBZ";
			words.at(i) = "is";
			mask.at(i) = false;
		}
		if (words.at(i) == "\'ve") {
			word_tags.at(i + 1) = "VBP";
			words.at(i) = "have";
			mask.at(i) = false;
		}
		if (words.at(i) == "\'d") {
			word_tags.at(i + 1) = "MD";
			words.at(i) = "would";
			mask.at(i) = false;
		}

		if (!mask.at(i))
			continue;

		string sure_tag = info->getSureTag(words.at(i));
		if (info->is_auxiliary_verb(words.at(i)) || info->is_modal_verb(words.at(i)))
			aux = true;
		if (words.at(i) == "who" || words.at(i) == "what") {
			who_trigger = true;
		}
		if (has_hashtag(words.at(i))) {
			string new_tag = "NN";
			string candidate_noun= words.at(i);
			candidate_noun= candidate_noun.substr(0,candidate_noun.find("#") );
			if(info->get_conj(candidate_noun,"NNS") != "")
				new_tag = "NNS";
			word_tags.at(i + 1) = new_tag;
			mask.at(i) = false;
			continue;
		}
		if (words.at(i) == "if")
			if_trigger = true;
		if (word_tags.at(i + 1) == "CC" && word_tags.at(1) == "WP")
			who_trigger = true;
		if (word_tags.at(i + 1) == "IN" || word_tags.at(i + 1) == "TO")
			aux = false;
		if (word_tags.at(i + 1) == "MD" && words.at(i) == "\'ll") {
			words.at(i) = "will";
			mask.at(i) = false;
		}
		if (i > 0 && words.at(i) == "\'t") {
			if (!aux_is_verb(word_tags.at(i)))
				word_tags.at(i) = "VBP";
			word_tags.at(i + 1) = "RB";
			words.at(i) = "not";
			mask.at(i) = false;
		}
		if (mask.at(i) && context_ && !info->is_candidate_name(words.at(i))
				&& info->get_freq(words.at(i), "RB", word_tags.at(i)) == 0
				&& info->get_freq(words.at(i), "IN", word_tags.at(i)) == 0
				&& info->get_freq(words.at(i), "TO", word_tags.at(i)) == 0
				&& info->get_freq(words.at(i), "PRP$", word_tags.at(i)) == 0
				&& info->get_freq(words.at(i), "CC", word_tags.at(i)) == 0) {
			if (activate_context) {
				double w = context_->evaluateProperName(words.at(i)); // the same word is used before as NNP
				string tag = word_tags.at(i + 1);
				if (w > 0.5 //&& (tag == "NN" || tag == "JJ")
						) {
					word_tags.at(i + 1) = "NNP";
					mask.at(i) = false;
					continue;
				}
			}
		}
		if (mask.at(i) && d->gender_proper_name(words.at(i)) != "" && words.at(i) != "may" && words.at(i) != "april"
				&& words.at(i) != "will" && words.at(i) != "said"
						) {
			word_tags.at(i + 1) = "NNP";
			mask.at(i) = false;
			continue;
		}
		if (word_tags.at(i + 1) == "POS" && words.at(i) == "\'") {
			words.at(i) = "\'s";
		}

		if (words.at(i).find("[date]") != string::npos) {
			word_tags.at(i + 1) = "CD";
			mask.at(i) = false;
		} else if (words.at(i).find("[verbatim]") != string::npos) {
			word_tags.at(i + 1) = "NNP";
			mask.at(i) = false;
		}
		//else if( i < words_orig.size() && is_number(words_orig.at(i) ) )
		else if (is_number(words.at(i))) {
			word_tags.at(i + 1) = "CD";
			mask.at(i) = false;
		} else if (find(punctuation_tags.begin(), punctuation_tags.end(), words.at(i)) != punctuation_tags.end()) {
			word_tags.at(i + 1) = words.at(i);
			mask.at(i) = false;
		} else if (words.at(i) == "?") {
			word_tags.at(i + 1) = "-period-";
			mask.at(i) = false;
		} else if (sure_tag != "" && mask.at(i)) {
			word_tags.at(i + 1) = sure_tag;
			mask.at(i) = false;
		} else if ((i != 0) && mask.at(i) && i < words_orig.size() && isupper(words_orig.at(i).at(0))
				&& info->get_freq(words.at(i), "RB", word_tags.at(i)) == 0
				&& info->get_freq(words.at(i), "IN", word_tags.at(i)) == 0
				&& info->get_freq(words.at(i), "TO", word_tags.at(i)) == 0
				&& info->get_freq(words.at(i), "PRP$", word_tags.at(i)) == 0
				&& info->get_freq(words.at(i), "CC", word_tags.at(i)) == 0) {
			if (debug)
				cout << "MM::: " << words.at(i) << endl;
			int size = words.at(i).size();
			string word = words.at(i);
			if (word.at(size - 1) != 's')
				word_tags.at(i + 1) = "NNP";
			else
				word_tags.at(i + 1) = "NNPS";
			mask.at(i) = false;
		} else if (find(chrono.begin(), chrono.end(), words.at(i)) != chrono.end()) {
			word_tags.at(i + 1) = "NN";
			mask.at(i) = false;
		}
		else if (mask.at(i) && is_question_ && i == 0 && info->is_auxiliary_verb(words.at(i))
				&& info->get_conj(words.at(i), "VBZ") == "" && info->get_conj(words.at(i), "VBD") == "") {
			word_tags.at(i + 1) = "VBP"; // An unconjugated auxiliary as a first word in a question is VBP
			mask.at(i) = false;
		} else if (mask.at(i)) {
			prob_before = info->get_freq(words.at(i), word_tags.at(i + 1), word_tags.at(i));
			prob_before += info->get_freq_back(words.at(i), word_tags.at(i + 1), word_tags.at(i + 2));
			if (i > 0)
				prob_before += info->get_freq_prior(words.at(i), word_tags.at(i + 1), word_tags.at(i - 1));

			double prob_after0 = 0;
			int safe = 0;
			while (prob_after0 == 0 && ++safe < 50) {
				tag_num = dran() * tags.size();
				new_tags = word_tags;
				candidate_tag = tags.at(tag_num);
				if (candidate_tag == word_tags.at(i + 1))
					continue; // the new tag must be new
				new_tags.at(i + 1) = candidate_tag;
				prob_after = info->get_freq(words.at(i), new_tags.at(i + 1), word_tags.at(i));
				prob_after0 = prob_after;
			}
			prob_after += info->get_freq_back(words.at(i), new_tags.at(i + 1), word_tags.at(i + 2));
			if (i > 0)
				prob_after += info->get_freq_prior(words.at(i), new_tags.at(i + 1), word_tags.at(i - 1));
			double prob_after_new = 0, prob_after_old = -1;
			vector<string> new_tags_new;
			for (int n = 0; (prob_before == 0 && prob_after0 == 0) && n < tags.size() /// this number is  arbitrary
			; ++n) {
				// avoids that the tag is stuck in a tag with prob 0
				//tag_num = ndran() * tags.size();

				tag_num = n;
				new_tags_new = word_tags;
				candidate_tag = tags.at(tag_num);

				new_tags_new.at(i + 1) = candidate_tag;
				prob_after_new = info->get_freq(words.at(i), new_tags_new.at(i + 1), word_tags.at(i));
				if (prob_after_new == 0)
					continue;
				prob_after_new += info->get_freq_back(words.at(i), new_tags_new.at(i + 1), word_tags.at(i + 2));
				if (i > 0)
					prob_after_new += info->get_freq_prior(words.at(i), new_tags_new.at(i + 1), word_tags.at(i - 1));
				if (prob_after_new > prob_after_old) {
					prob_after_old = prob_after_new;
					new_tags = new_tags_new;
				}
			}
			if (prob_after_old > prob_after)
				prob_after = prob_after_old;

			if (is_question_
					&& (aux && candidate_tag.find("NN") != string::npos && word_tags.at(i) != "DT"
							&& word_tags.at(i) != "PRP$" && word_tags.at(i) != "CD" && info->is_candidate_verb(words.at(i))
							&& !info->is_candidate_verb(words.at(i + 1)))) {
				// This part is due to the fact that in questions
				// like "where does the dog go?", "go" is tagged
				// as a name. The tagging frequencies that I have
				// are not adequate for questions. Here I try to
				// correct the mistakes by converting in VB the NN
				// that would be more likely as VBZ (as "go" in
				// the previous example).
				candidate_tag2 = "VBZ";
				new_tags2 = new_tags;
				new_tags2.at(i + 1) = candidate_tag2;
				prob_after2 = info->get_freq(words.at(i), new_tags2.at(i + 1), word_tags.at(i));
				prob_after2 += info->get_freq_back(words.at(i), new_tags2.at(i + 1), word_tags.at(i + 2));
				if (i > 0)
					prob_after2 += info->get_freq_prior(words.at(i), new_tags2.at(i + 1), word_tags.at(i - 1));
				if (prob_after2 < prob_after) {
					aux = false;
					prob_after = prob_after2;
					new_tags2.at(i + 1) = "VB";
					new_tags = new_tags2;
					prob_after = 1000;
					//print_vector(new_tags);
				}
			}

			if (((is_question_ && who_trigger) || if_trigger)
					&& (word_tags.at(i + 1) == "NNS" || word_tags.at(i + 1) == "NNPS")) {
				// "WP(who) NNS(shops?)" ->  "WP(who) VBZ(shops?)"
				string word = words.at(i);
				string tag = word_tags.at(i + 1);
				string base = "";
				double to_ret = info->regular_tag(word, "VBZ", &base);
				if (base != "" && info->is_candidate_verb(base)) {
					// if there is a VBZ for this word then change from NNS to VBZ
					new_tags.at(i + 1) = "VBZ";
					prob_after = 1000;
					who_trigger = false;
					if_trigger = false;
					mask.at(i) = false;
				}
			}

			if (is_question_ && word_tags.at(i) == "VBZ" // "does that ring a bell?"
			&& words.at(i) == "that") {
				new_tags.at(i + 1) = "DT";
				prob_after = 1000;
				mask.at(i) = false;
			}
			if (is_question_    // "can you please elaborate..."
			&& word_tags.at(i) == "PRP" && i > 0 && word_tags.at(i - 1) == "MD" && words.at(i) == "please"
					&& (word_tags.at(i + 2) == "VB" || info->is_candidate_verb(words.at(i + 1)))) {
				aux = false;
				new_tags.at(i + 1) = "RB";
				new_tags.at(i + 2) = "VB";
				prob_after = 1000;
				mask.at(i) = false;
			}
			if (is_question_    // "How do I..."
			&& (word_tags.at(i) == "WRB" || word_tags.at(i) == "WP")
					&& (word_tags.at(i + 1) == "VB" || word_tags.at(i + 1) == "VBG") && word_tags.at(i + 2) == "PRP"
					&& info->is_candidate_verb(words.at(i))) {
				new_tags.at(i + 1) = "VBP";
				prob_after = 1000;
				mask.at(i) = false;
			}

			if (    //is_question_
				   // "How to VB..."
			word_tags.at(i) == "TO" && i > 1 && word_tags.at(i - 1) == "WRB" && words.at(i - 2) == "how"
					&& info->is_candidate_verb(words.at(i))) {
				new_tags.at(i + 1) = "VB";
				prob_after = 1000;
				mask.at(i) = false;
			}

			if ( // "what do..."
			word_tags.at(i + 1) == "VB" && words.at(i) == "do"
					&& (word_tags.at(i) == "WP" || word_tags.at(i) == "WDT" || word_tags.at(i) == "WRB")) {
				new_tags.at(i + 1) = "VBP";
				mask.at(i) = false;
				prob_after = 1000;
			}

			if ( // "with what are traces found..."
			word_tags.at(i + 1) == "VBZ" && word_tags.at(i) == "VBP" && info->get_conj(words.at(i), "NNS") != "") {
				word_tags.at(i + 1) = "NNS";
				mask.at(i) = false;
				continue;
				//prob_after = 10000;
			}

			//r= dran();
			if (debug)
				cout << "FINAL::: " << prob_after << " <> " << prob_before << endl;
			if (prob_before == 0
			//|| r < log(prob_after) -log(prob_before)
					|| prob_after > prob_before) {
				//if(r < exp(100*prob_after/prob_before) ) {
//		    string tmp_tag= new_tags.at(i+1);
//		    string base= info->get_conj(words.at(i),tmp_tag);
//		    if(base != "") {
//			 vector<string> candidate_tags= info->getCandidateTags(base); // each word has its own candidate tags			 			 
//			 if(find(candidate_tags.begin(), candidate_tags.end(), tmp_tag) == candidate_tags.end()) 
//			      continue;
//		    }
				word_tags = new_tags;
			}
		}
	}
}

void tagger::thermalize()
{
	int incr;
	int size_words = words.size();
	vector<bool> mask(size_words, true);
	for (incr = 0; incr < 1500; ++incr) {
		step(mask);
	}
}

static inline int min(int a, int b)
{
	if (a < b)
		return a;
	return b;
}

vector<vector<string> > tagger::measure()
{
	vector<vector<string> > measured_tags;
	vector<string> old_tags;
	int incr, incr2;

	int size_words = words.size();
	vector<bool> mask(size_words, true);
	int size = min(size_words / 2 + 7, 10);
	//int size= 150;
	if (is_question_)
		size = min(size_words / 2 + 7, 10);
	//if(is_question_) size= 160;
	for (incr = 0; incr < size; ++incr) {
		step(mask);
//          cout << incr << " ";
		old_tags = word_tags;

	}
	measured_tags.push_back(word_tags);
	return measured_tags;
}

static void print_vector_string(const vector<string> &vs)
{
	vector<string>::const_iterator tags_iter = vs.begin();
	while (tags_iter < vs.end()) {
		std::cout << (*tags_iter) << " ";
		++tags_iter;
	}
	std::cout << std::endl;
}

static bool compare(const pair<vector<string>, double> &lhs, const pair<vector<string>, double> &rhs)
{
	return lhs.second > rhs.second;
}

static string simplify_name(const string &str)
{
	if (str == "VBZ" || str == "VBP" || str == "VBD")
		return string("V");
	if (str == "NN" || str == "NNS" || str == "NNP" || str == "NNPS")
		return string("N");
	if (str == "JJ" || str == "JJS" || str == "VBJ")
		return string("J");
	return str;
}

static vector<string> simplify_tags(const vector<string> &words, const vector<string> &tagged)
{
	vector<string> ret_vect;
	vector<string>::const_iterator words_iter = words.begin();
	vector<string>::const_iterator tagged_iter = tagged.begin() + 1;

	//map<string,string> special_words;
	//special_words["[date]_sometimes"] = "CD-DATE";
	//special_words["more-than"] = "IN-COMPARATIVE";

	ret_vect.push_back(simplify_name(tagged.front()));

	while (boost::next(tagged_iter) != tagged.end()) {
		string word = *words_iter;
		string tag = *tagged_iter;
		//map<string,string>::iterator miter = special_words.find(word);
		//if(miter != special_words.end() ) {
		if (aux_is_date(word)) {
			ret_vect.push_back("CD-DATE");
		}
		//else if(word == "more-than") {
		//     ret_vect.push_back("IN-COMPARATIVE");
		//}
		else
			ret_vect.push_back(simplify_name(*tagged_iter));
		++words_iter;
		++tagged_iter;
	}
	ret_vect.push_back(simplify_name(tagged.back()));

	return ret_vect;
}


vector<FuzzyPred> tagger::convert_proper_names(vector<FuzzyPred> to_return)
// some names should not be NNP in this framework 
// countries : NNP -> NN
// nationalities : NNP -> JJ
{
	vector<FuzzyPred>::iterator diter = to_return.begin(), dend = to_return.end();
	string word, tag;
	metric *d = metric_singleton::get_metric_instance();

	bool NN_trigger = false;

	while (diter != dend) {
		word = diter->pred().begin().firstChild()->str;
		tag = diter->pred().begin()->str;

		if (aux_is_proper_noun(tag) && d->hypernym_dist(word, "country") > 0.4)
			diter->pred().begin()->str = "NN";
		else if (aux_is_proper_noun(tag) && d->pertains_to_name(word, "country") > 0.4)
			diter->pred().begin()->str = "JJ";
		else if (tag== "NNPS" && word.at(word.size()-1) == 's'
				&& d->hypernym_dist(word, "country") > 0.1
		)
			diter->pred().begin()->str = "NN"; // Texas/NNPS->NN
		else if (tag == "NNPS" && aux_is_verbatim(word))
			diter->pred().begin()->str = "NNP";

		if(tag == "DT" || tag == "CD")
			NN_trigger = true;
		if(NN_trigger && tag == "NNP") {
			//if( d->gender_proper_name(word) == "" ) {// Proper nouns of people remain proper nouns
			diter->pred().begin()->str = "NN";
			NN_trigger = false;
			//}
		}
		if ( (tag == "NNPS" || tag == "NNS") && !NN_trigger && d->gender_proper_name(word) != "")
			diter->pred().begin()->str = "NNP"; // Charles/NNPS -> NNP
		if(NN_trigger && tag == "NNPS") {
			diter->pred().begin()->str = "NNS";
		}
		if(tag != "DT" && !aux_is_noun(tag) && !aux_is_adjective(tag) )
			NN_trigger = false;


		++diter;
	}

	return to_return;
}

static bool ends_with_ing(const string &word)
{
	int pos = word.rfind("ing");
	if (pos != string::npos && pos > 0 && pos == word.size() - 3) {
		return true;
	}
	return false;
}

vector<string> tagger::guess_missing_tags(vector<string> tagged)
{
	vector<string>::iterator tagged_iter;
	vector<string>::iterator words_iter;
	string word, base, tag;


	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	vector<pair<vector<string>::iterator, vector<string>::iterator> > iter_pairs;
	int m = 0;
	while (boost::next(tagged_iter) != tagged.end()) {
		string dummy_str;
		word = *words_iter;
		tag = *tagged_iter;

		if (info->isSureTag(word)) {
			++tagged_iter;
			++words_iter;
			++m;
			continue;
		}

		double prob_before = info->get_freq(word, tag, *boost::prior(tagged_iter));
		if (prob_before == 0) {
			double to_ret = 0;
			string base_tmp;
			//base_tmp= info->get_conj(word,tag);
			to_ret = info->regular_tag(word, tag, &base_tmp);
			if ((base_tmp == "[wrong_tag]" || to_ret == 0) && !(isupper(words_orig.at(m).at(0)) // If a name is a proper noun (by choice of the writer) it has to remain a noun
			&& (tag != "NNP" || tag != "NNPS") && m != 0) && !(tag == "POS" && word == "\'s")
					&& !(tag == "CD" && is_number(word))) {
				// The item is tagged wrong because is not in the freq.txt file.
				// Therefore, save the iterators to the word and tag to process
				iter_pairs.push_back(make_pair(words_iter, tagged_iter));
			}
		}
		++tagged_iter;
		++words_iter;
		++m;
	}

	double prob_after;
	vector<pair<vector<string>::iterator, string> > substitutions;
	for (int n = 0; n < iter_pairs.size(); ++n) {
		words_iter = iter_pairs.at(n).first;
		tagged_iter = iter_pairs.at(n).second;
		word = *words_iter;
		tag = *tagged_iter;
		pair<string, double> subst_choice;
		subst_choice.second = 0;

		for (int m = 0; m < tags.size(); ++m) {
			string new_tag = tags.at(m);
			string base_tmp;
			base_tmp = info->get_conj(word, new_tag);
			if (base_tmp == "")
				base_tmp = word;
			if (info->is_candidate_name(base_tmp) || info->is_candidate_verb(base_tmp)) {
				prob_after = info->general_tag(new_tag, *boost::prior(tagged_iter));
				prob_after += info->general_tag_back(new_tag, *boost::next(tagged_iter));
				if (boost::prior(tagged_iter) != tagged.begin())
					prob_after += info->general_tag_prior(new_tag, *boost::prior(boost::prior(tagged_iter)));
				string new_tag = tags.at(m);
				double to_ret = 0;
				base_tmp = info->get_conj(word, new_tag);
				//cout << "NEW2::" << word << ", " << new_tag << ", " << base_tmp << ", " << prob_after << " " << base_tmp << endl;
				if ((info->is_valid_general_tag(new_tag) || info->is_valid_verb_tag(new_tag)) && base_tmp != ""
						&& subst_choice.second < prob_after) {
					//cout << "NEW::" << new_tag << endl;
					subst_choice.second = prob_after;
					subst_choice.first = new_tag;
				}
				if (base_tmp == "" && ends_with_ing(word)) {
					subst_choice.second = 1;
					subst_choice.first = "VBG";
				}
			}
		}
		if (subst_choice.first != "")
			substitutions.push_back(make_pair(tagged_iter, subst_choice.first));
	}

	vector<vector<string>::iterator> already_done;
	for (int n = 0; n < substitutions.size(); ++n) {
		*substitutions.at(n).first = substitutions.at(n).second;
		already_done.push_back(substitutions.at(n).first);
	}


	// check that verbs are tagged as verbs, and nouns as nouns
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (boost::next(tagged_iter) != tagged.end()) {
		word = *words_iter;
		tag = *tagged_iter;
		string base_tmp;
		base_tmp = info->get_conj(word, tag);
		if (base_tmp == "")
			base_tmp = word;
		if (tag == "NNS" && !info->is_candidate_name(base_tmp) && info->is_candidate_verb(base_tmp)) {
			*tagged_iter = "VBZ";
			already_done.push_back(tagged_iter);
		}
		if (tag == "VBZ" && !info->is_candidate_verb(base_tmp)) {
			*tagged_iter = "NNS";
			already_done.push_back(tagged_iter);
		}
		if (tag == "VBP" && !info->is_candidate_verb(base_tmp)) {
			*tagged_iter = "NN";
			already_done.push_back(tagged_iter);
		}
		if (aux_is_noun(tag) && !info->is_candidate_name(base_tmp) && info->is_candidate_verb(base_tmp)) {
			*tagged_iter = "VB";
			already_done.push_back(tagged_iter);
		}
		if (aux_is_verb(tag) && !info->is_candidate_verb(base_tmp)) {
			*tagged_iter = "NN";
			already_done.push_back(tagged_iter);
		}
		++words_iter;
		++tagged_iter;
	}

	// If uncertain, it is NN
	for (int n = 0; n < iter_pairs.size(); ++n) {
		words_iter = iter_pairs.at(n).first;
		tagged_iter = iter_pairs.at(n).second;
		if (!shortfind(already_done, tagged_iter))
			*tagged_iter = "NN";
	}

	// some words are NNS but tagged NN
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (boost::next(tagged_iter) != tagged.end()) {
		word = *words_iter;
		tag = *tagged_iter;
		if (tag == "NN" || tag == "NNP") {
			string base_tmp;
			double to_ret = info->regular_tag(word, "NNS", &base_tmp);
			if (to_ret != 0) {
				*tagged_iter = "NNS";
			}
		}
		++words_iter;
		++tagged_iter;
	}

	// last touch for some specific words
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (boost::next(tagged_iter) != tagged.end()) {
		word = *words_iter;
		tag = *tagged_iter;
		if (word == "meeting" && *boost::prior(tagged_iter) == "DT")
			*tagged_iter = "NN";
		if (word == "gas" && tag == "NNS")
			*tagged_iter = "NN";
		if (word == "gas" && tag == "NNPS")
			*tagged_iter = "NN";
		++words_iter;
		++tagged_iter;
	}


	return tagged;
}


static bool there_is_verb_between(vector<string>::iterator iter, vector<string>::iterator end)
{	
	for(; iter != end; ++iter) {
		if(iter != end && aux_is_verb(*iter) ) {
			return true;
		}
	}

	return false;
}

static bool there_is_article_between(vector<string>::iterator iter, vector<string>::iterator end)
{
	for(; iter != end; ++iter) {
		if(iter != end && aux_is_article(*iter) )
			return true;
	}

	return false;
}

static bool there_is_WP_between(vector<string>::iterator iter, vector<string>::iterator end)
{
	for(; iter != end; ++iter) {
		if(iter != end && (*iter == "WP" || *iter == "WDT" || *iter == "WRB") )
			return true;
	}

	return false;
}

static bool there_is_period_between(vector<string>::iterator iter, vector<string>::iterator end)
{
	for(; iter != end; ++iter) {
		if(iter != end && (*iter == "-period-") )
			return true;
	}

	return false;
}


static bool there_is_adverb_between(vector<string>::iterator iter, vector<string>::iterator end)
{
	for(; iter != end; ++iter) {
		if(iter != end && aux_is_adverb(*iter) )
			return true;
	}

	return false;
}


static string join_wikidata_words(vector<string>::iterator iter, vector<string>::iterator end)
{
	string to_return;
	for(; iter != end; ++iter) {
		to_return+= *iter;
		if(boost::next(iter) != end)
			to_return += "_";
	}

	return to_return;
}

bool compare_qs(const string &lhs, const string &rhs)
{
	try {
		int l= boost::lexical_cast<int>(lhs.substr(1,lhs.size()));
		int r= boost::lexical_cast<int>(rhs.substr(1,lhs.size()));
		if(l < r)
			return true;
	} catch(...) {
		///
	}
	return false;
}

string order_qs(string qs)
{
	string to_return;
	vector<string> strs;
	boost::split(strs,qs,boost::is_any_of("|"));
	sort(strs.begin(),strs.end(),compare_qs);
	strs.erase(std::unique(strs.begin(), strs.end()), strs.end());
	for(int n=0; n < strs.size(); ++n) {
		to_return += strs.at(n);
		if(n != strs.size()-1)
			to_return += "|";
	}


	return to_return;
}

vector<string> tagger::substitute_wikidata_qs(vector<string> tagged)
{	
	vector<string>::iterator tags_iter_begin;
	vector<string>::iterator tags_iter_end;
	vector<string>::iterator words_iter_begin;
	vector<string>::iterator words_iter_end;
	tags_iter_begin = tagged.begin() + 1;
	words_iter_begin = words.begin();

	for(; words_iter_begin != words.end() && tags_iter_begin != tagged.end(); ++words_iter_begin, ++tags_iter_begin) {
		words_iter_end = words.end();
		tags_iter_end  = tagged.end()-1;
		for(; words_iter_end != words_iter_begin && tags_iter_end != tags_iter_begin; --words_iter_end, --tags_iter_end) {

			if( there_is_verb_between(tags_iter_begin,tags_iter_end) )
				continue;
			if( distance(words_iter_begin,words_iter_end) == 1 && aux_is_article(*tags_iter_begin))
			  	continue;
			if( aux_is_article(*boost::prior(tags_iter_end)) )
			  	continue;
			if( aux_is_preposition(*tags_iter_begin))
			  	continue;
			if( there_is_adverb_between(tags_iter_begin,tags_iter_end) )
			 	continue;			
			if( there_is_WP_between(tags_iter_begin,tags_iter_end) )
			 	continue;			
			if( there_is_period_between(tags_iter_begin,tags_iter_end) )
			 	continue;

			string name    = join_wikidata_words(words_iter_begin, words_iter_end);
			vector<int> qs = info->find_wikidata_q(name, *tags_iter_begin);

			if( qs.size() == 0 )
				continue;
			string first_word;
			if(distance(words_iter_begin,words_iter_end) == 1)
				first_word = *words_iter_begin;
			else
				first_word = join_wikidata_words(words_iter_begin,words_iter_end);
			string q_add_str= "";
			for(int qn=0; qn < qs.size(); ++qn) {
				int q = qs.at(qn);
				q_add_str += (string)"Q" + boost::lexical_cast<string>(q);
				if(qn != qs.size()-1)
					q_add_str += "|";
			}
			if(*words_iter_begin == "the" && distance(words_iter_begin,words_iter_end) != 1) {
				name        = join_wikidata_words(words_iter_begin+1, words_iter_end);
				qs          = info->find_wikidata_q(name, *tags_iter_begin);
				if(qs.size() > 0) {
					//first_word += '|';
					//first_word += join_wikidata_words(words_iter_begin+1, words_iter_end);
					first_word = join_wikidata_words(words_iter_begin+1, words_iter_end);
					q_add_str += '|';
				}
				for(int qn=0; qn < qs.size(); ++qn) {
					int q = qs.at(qn);
					q_add_str += (string)"Q" + boost::lexical_cast<string>(q);
					if(qn != qs.size()-1)
						q_add_str += "|";
				}
			}
			q_add_str = order_qs(q_add_str);
			string qstr = first_word + ":" + q_add_str;
			words.erase (words_iter_begin,words_iter_end) ;
			tagged.erase(tags_iter_begin,tags_iter_end) ;

			words.insert (words_iter_begin,qstr );
			tagged.insert(tags_iter_begin,"NN");
		}
	}

	return tagged;
}


vector<string> tagger::post_process_original(vector<string> tagged)
{
	aux_post_process_original(words, tagged, info, is_question_);
	return tagged;
}

std::pair<vector<FuzzyPred>, vector<FuzzyPred> > tagger::join_names(vector<FuzzyPred> tagged, vector<FuzzyPred> tagged_simple)
{
	vector<FuzzyPred>::iterator tagged_iter = tagged.begin();
	vector<FuzzyPred>::iterator tagged_end = tagged.end();
	vector<FuzzyPred>::iterator tagged_simple_iter = tagged_simple.begin();
	vector<FuzzyPred>::iterator tagged_simple_end = tagged_simple.end();
	++tagged_iter;
	++tagged_simple_iter;
	while (tagged.size() && tagged_iter != tagged_end) {
		if (tagged_iter != tagged.begin() && tagged_iter != tagged.end()) {
			string prior_head_str = boost::prior(tagged_iter)->pred().begin()->str;
			string head_str = tagged_iter->pred().begin()->str;
			string prior_child_str = boost::prior(tagged_iter)->pred().begin().firstChild()->str;
			string child_str = tagged_iter->pred().begin().firstChild()->str;
			if ((prior_head_str == "NNP") // || prior_head_str == "NNPS") // cannot join NNPS_NNP
			&& (head_str == "NNP" || head_str == "NNPS" || (prior_head_str == "CD" && head_str == "CD"))
					&& child_str.find("[verbatim]") == string::npos // cannot join NNP with "[verbatim]"
					&& prior_child_str.find("[verbatim]") == string::npos // cannot join NNP with "[verbatim]"
							) {
				child_str = prior_child_str + "_" + child_str;
				PredTree pt(head_str);
				pt.appendChild(pt.begin(), child_str);
				tagged_iter->pred() = pt;
				tagged_iter = tagged.erase(boost::prior(tagged_iter));
				PredTree pt_simple("N");
				pt_simple.appendChild(pt_simple.begin(), child_str);
				tagged_simple_iter->pred() = pt_simple;
				tagged_simple_iter = tagged_simple.erase(boost::prior(tagged_simple_iter));
				tagged_end = tagged.end();
				tagged_simple_end = tagged_simple.end();
			}
		}
		++tagged_iter;
		++tagged_simple_iter;
	}

	return std::make_pair(tagged, tagged_simple);
}

vector<string> tagger::post_process_tags(vector<string> tagged)
// add the tag AUX for auxiliary verbs (when needed)
{
	vector<string>::iterator tagged_iter = tagged.begin() + 1;
	vector<string>::iterator aux_iter;
	vector<string>::iterator words_iter = words.begin();
	string word, base, tag;
	bool aux = false;

	while (boost::next(tagged_iter) != tagged.end()) {
		word = *words_iter;
		tag = *tagged_iter;
		if (tag == "-period-" && word == "?")
			*tagged_iter = "-QM-";

		++tagged_iter;
		++words_iter;
	}
	return tagged;
}

bool tagger::isHiddenQuestion(const vector<string> &tagged, const vector<string> &words)
{
	if (tagged.size() < 2 || words.size() < 1)
		return false;
	string tag = tagged.at(1);
	string word = words.at(0);


	if (tag == "MD")
		return true;
	if (aux_is_verb(tag) && tag != "VBG") {
		string base = info->get_conj(word, tag);
		if (base == "")
			base = word;
		if (base == "do" || base == "be" || base == "have")
			return true;
	}
	return false;
}

static pair<vector<string>, vector<string> > change_period_into_qm(vector<string> tagged, vector<string> words)
{
	vector<string>::iterator period_pos = find(tagged.begin(), tagged.end(), "-period-");
	if (period_pos != tagged.end()) {
		int pos = std::distance(tagged.begin(), period_pos);
		tagged.at(pos) = "-period-";
		words.at(pos - 1) = "?";
	} else {
		words.push_back("?");
		tagged.at(words.size()) = "-period-";
	}

	return make_pair(tagged, words);
}

void tagger::do_tag()
{
	word_tags.clear();
	tagged_preds.clear();
	tagged_preds_simpl.clear();

	word_tags.resize(words.size() + 2); // The first and last term are just for punctuation
	word_tags.at(0) = " ";
	word_tags.at(words.size()) = "-period-";
	init_random_tags();

	vector<vector<string> > measured_tags = measure();

	if (!is_question_ && measured_tags.size() && isHiddenQuestion(measured_tags.at(0), words)) {
		is_question_ = true;
		pair<vector<string>, vector<string> > tags_words_pair = change_period_into_qm(measured_tags.at(0), words);
		measured_tags.at(0) = tags_words_pair.first;
		words = tags_words_pair.second;
	}

	map<vector<string>, int> count_tags;
	vector<vector<string> >::iterator tags_iter = measured_tags.begin();
	vector<string> tmp_tags;

	while (tags_iter != measured_tags.end()) {
		//	  print_vector_string(*tags_iter);
		++count_tags[*tags_iter];
		++tags_iter;
	}

	vector<pair<vector<string>, double> > tags_freq;
	map<vector<string>, int>::iterator count_iter = count_tags.begin();
	int msize = measured_tags.size();
	while (count_iter != count_tags.end()) {
		tags_freq.push_back(make_pair(count_iter->first, count_iter->second / (double) msize));
		++count_iter;
	}

	if (!(tags_freq.front().first.size() > 1 && tags_freq.front().first.at(1) == "-period-") // the first tag is always ""
	) {
		tags_freq.front().first = guess_missing_tags(tags_freq.front().first);
		tags_freq.front().first = post_process_original(tags_freq.front().first);

		///-WikiData start
		if(wi_.isWikidata())
			tags_freq.front().first = substitute_wikidata_qs(tags_freq.front().first);
		///-WikiData end
	}
	sort(tags_freq.begin(), tags_freq.end(), compare);

	vector<string> tags_simpl = simplify_tags(words, tags_freq.front().first); // Transform NN, NNS, NNP -> N , ....

	for (int n = 0; n < tags_simpl.size(); ++n)
		tags_simpl = post_process_tags(tags_simpl); // Add "AUX" tag and ? -> -QM-

	vector<string>::iterator titer_orig = tags_freq.front().first.begin();
	vector<string>::iterator titer = tags_simpl.begin();
	vector<string>::iterator titer_end = tags_simpl.end();
	vector<string>::iterator words_iter = words.begin();
	++titer;
	++titer_orig;
	while (words_iter != words.end()) {
		PredTree predtree_simpl(*titer);
		predtree_simpl.appendChild(predtree_simpl.begin(), *words_iter);
		tagged_preds_simpl.push_back(predtree_simpl);

		PredTree predtree_orig(*titer_orig);

		string to_append= substitute_commas(*words_iter);
		predtree_orig.appendChild(predtree_orig.begin(), to_append);
		tagged_preds.push_back(predtree_orig);

		//std::cout << Predicate(predtree) << " " ;
		if (*titer == "-period-" || *titer == "-QM-") /// this is to avoid double puctuation at the end. CHANGE THIS!!
			break;
		++titer;
		++titer_orig;
		++words_iter;
	}

	pair<vector<FuzzyPred>, vector<FuzzyPred> > joined = join_names(tagged_preds, tagged_preds_simpl); // joins proper names together
	tagged_preds = joined.first;
	tagged_preds_simpl = joined.second;

	tagged_preds = this->convert_proper_names(tagged_preds);
}
