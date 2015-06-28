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



#include"tagger_aux.hpp"

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

bool has_number(string s)
{
	for (int n = 0; n < s.size(); ++n)
		if (isdigit(s.at(n)))
			return true;
	return false;
}

bool aux_is_cardinal_word(const string &word)
// 12th ?
{
	int pos = word.find("th");
	if (pos == string::npos)
		pos = word.find("st");
	if (pos == string::npos)
		pos = word.find("nd");
	if (pos == string::npos)
		pos = word.find("rd");
	if (pos == string::npos)
		return false;
	if (pos == 0)
		return false;
	if (isdigit(word.at(0)))
		return true;
	return false;
}


bool aux_is_transitive(const string &str)
/// Put this into metric!
{
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
	candidates.push_back("contribute");
	candidates.push_back("give_way");
	candidates.push_back("belong");
	candidates.push_back("die");
	candidates.push_back("lie");
	candidates.push_back("think");

	if (find(candidates.begin(), candidates.end(), str) != candidates.end())
		return false;
	return true;
}


bool aux_is_verbatim(const string &word)
{
	if (word.find("[verbatim]") != string::npos)
		return true;
	return false;
}
bool aux_is_adjective(const string &tag)
{
	if (tag == "JJ" || tag == "JJS" || tag == "JJR")
		return true;

	return false;
}

bool aux_is_conj(const string &tag)
{
	if (tag == "CC" || tag == "-comma-")
		return true;

	return false;
}

bool aux_is_article(const string &tag)
{
	if (tag == "DT" || tag == "PDT")
		return true;

	return false;
}

bool aux_is_preposition(const string &tag)
{
	if (tag == "IN" || tag == "TO")
		return true;

	return false;
}

bool aux_is_adverb(const string &tag)
{
	if (tag == "RB")
		return true;

	return false;
}

bool aux_is_CD(const string &tag)
{
	if (tag == "CD")
		return true;

	return false;
}

bool aux_is_PRP(const string &tag)
{
	if (tag == "PRP")
		return true;

	return false;
}

bool aux_is_verb(const string &tag)
{
	if (tag == "VB" || tag == "VBP" || tag == "VBD" || tag == "VBZ" || tag == "VBN" || tag == "VBG")
		return true;

	return false;
}

bool aux_is_noun(const string &tag)
{
	if (tag == "NN" || tag == "NNP" || tag == "NNS" || tag == "NNPS")
		return true;

	return false;
}

bool aux_is_proper_noun(const string &tag)
{
	if (tag == "NNP" || tag == "NNPS")
		return true;

	return false;
}

bool aux_is_plural(const string &tag)
{
	if (tag == "NNS" || tag == "NNPS")
		return true;

	return false;
}

bool aux_is_subject_PRP(const string &str)
{
	if (str == "he" || str == "she" || str == "it" || str == "we" || str == "they")
		return true;

	return false;
}

bool aux_is_someone(const string &tag)
{
	if (tag.find("someone") != string::npos)
		return true;

	return false;
}

bool aux_is_something(const string &tag)
{
	if (tag.find("something") != string::npos)
		return true;

	return false;
}

bool aux_is_date(const string &str)
{
	if (str.find("[date]") != string::npos)
		return true;
	return false;
}

bool aux_is_place(const string &str)
{
	if (str.find("[place]") != string::npos)
		return true;
	return false;
}


bool aux_verb_supports_indirect_obj(const string &name)
{
	vector<string> ind_verbs;
	ind_verbs.push_back("give");
	ind_verbs.push_back("wish");
	ind_verbs.push_back("make");
	ind_verbs.push_back("make_it");
	ind_verbs.push_back("consider");
	ind_verbs.push_back("find");
	ind_verbs.push_back("buy");
	ind_verbs.push_back("pay");
	ind_verbs.push_back("show");
	ind_verbs.push_back("call");
	ind_verbs.push_back("regard_as");
	ind_verbs.push_back("set");
	ind_verbs.push_back("promise");

	if (find(ind_verbs.begin(), ind_verbs.end(), name) != ind_verbs.end())
		return true;

	return false;
}


void aux_post_process_original(vector<string> words, vector<string> tagged, tagger_info *info, bool is_question_)
 // Correct the tags according for the English language
{
	
	vector<string>::iterator tagged_iter = tagged.begin() + 1;
	vector<string>::iterator aux_iter;
	vector<string>::iterator words_iter = words.begin();
	string word, base, tag;

	metric *d = metric_singleton::get_metric_instance();


	// will/VB -> will/MD
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (boost::next(tagged_iter) != tagged.end()) {
		word = *words_iter;
		tag = *tagged_iter;
		if (word == "will" && tag == "VB" ) {
			*tagged_iter = "MD";
		}
		++tagged_iter;
		++words_iter;
	}

	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (boost::next(tagged_iter) != tagged.end()) {
		string dummy_str;
		word = *words_iter;
		tag = *tagged_iter;
		string base = info->get_conj(word, "NNS");
		if (base != "" && tag == "VBZ"
			&& (*boost::prior(tagged_iter) == "VBD" || *boost::prior(tagged_iter) == "VBZ"
			   || *boost::prior(tagged_iter) == "VBP"
			   || (*boost::next(tagged_iter) == "VBD" && *boost::prior(tagged_iter) == "DT")
				)
		) {
			if (word.at(word.size() - 1) == 's')
				*tagged_iter = "NNS";
			else
				*tagged_iter = "NN";
		}
		++tagged_iter;
		++words_iter;
	}

	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (boost::next(tagged_iter) != tagged.end()) {
		string dummy_str;
		word = *words_iter;
		tag = *tagged_iter;
		string base = info->get_conj(word, "NNS");
		if (base != "" && tag == "VBZ"
			&& (*boost::prior(tagged_iter) == "JJR"
			&&  words_iter != words.end()
			&&  boost::next(words_iter) != words.end()
			&& (*boost::next(tagged_iter) == "IN" && *boost::next(words_iter) == "of")
			)
		) {
			if (word.at(word.size() - 1) == 's')
				*tagged_iter = "NNS";
			else
				*tagged_iter = "NN";
		}
		++tagged_iter;
		++words_iter;
	}


	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (boost::next(tagged_iter) != tagged.end()) {
		string dummy_str;
		word = *words_iter;
		tag = *tagged_iter;
		if (tag == "VBN"
			&& *boost::prior(tagged_iter) == "POS"
			&& *boost::next(tagged_iter) == "NN"
		) {
			*tagged_iter = "JJ";
		}
		++tagged_iter;
		++words_iter;
	}

	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (boost::next(tagged_iter) != tagged.end()) {
		string dummy_str;
		word = *words_iter;
		tag = *tagged_iter;
		if (word.find("_") != string::npos && tag == "JJ") {
			if (word.at(word.size() - 1) == 's')
				*tagged_iter = "NNS";
			else
				*tagged_iter = "NN";
		}
		++tagged_iter;
		++words_iter;
	}

	// alexander the great -> [verbatim]_alexander_the_great (if in Wordnet)
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (boost::next(tagged_iter) != tagged.end()) {
		word = *words_iter;
		tag = *tagged_iter;

		if (boost::next(words_iter) == words.end() || words_iter == words.begin()) {
			++tagged_iter;
			++words_iter;
			continue;
		}

		string prior_word = *boost::prior(words_iter);
		string prior_tag = *boost::prior(tagged_iter);
		string next_word = *boost::next(words_iter);
		string next_tag = *boost::next(tagged_iter);

		string candidate_name = prior_word + "_" + word + "_" + next_word;
		if (tag == "DT" && aux_is_noun(*boost::prior(tagged_iter))
				&& (aux_is_noun(*boost::next(tagged_iter)) || aux_is_adjective(*boost::next(tagged_iter)))
				&& d->has_noun(candidate_name)) {
			*boost::prior(words_iter) = (string) "[verbatim]_" + candidate_name;
			tagged_iter = tagged.erase(tagged_iter, tagged_iter + 2);
			words_iter = words.erase(words_iter, words_iter + 2);
			continue;
		}
		++tagged_iter;
		++words_iter;
	}


	// correct JJ tagged as VBD or VBN
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (boost::next(tagged_iter) != tagged.end()) {
		word = *words_iter;
		tag = *tagged_iter;
		if (!d->is_adjective(word)) {
			++tagged_iter;
			++words_iter;
			continue;
		}
		if ((*tagged_iter == "VBD" || *tagged_iter == "VBN")
				&& ( *boost::prior(tagged_iter) == "VBG")
				// || aux_is_adjective(*boost::prior(tagged_iter)) ) /// adressed in Triggers
				&& (aux_is_noun(*boost::next(tagged_iter)) || aux_is_adjective(*boost::next(tagged_iter)))) {
			*tagged_iter = "JJ";
		}
		++tagged_iter;
		++words_iter;
	}

	// correct JJ tagged as VBD or VBN
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (boost::next(tagged_iter) != tagged.end()) {
		word = *words_iter;
		tag = *tagged_iter;
		if ((*tagged_iter == "VBD" || *tagged_iter == "VBN") && aux_is_article(*boost::prior(tagged_iter))
				&& words_iter != words.begin()
				&& (*boost::prior(words_iter) == "a" || *boost::prior(words_iter) == "an"
						|| *boost::prior(words_iter) == "the")
				&& (aux_is_noun(*boost::next(tagged_iter)) || aux_is_adjective(*boost::next(tagged_iter)))) {
			*tagged_iter = "JJ";
		}
		++tagged_iter;
		++words_iter;
	}

	// answer/VBP is/VBZ -> answer/NN is/VBZ
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (boost::next(tagged_iter) != tagged.end()) {
		word = *words_iter;
		tag = *tagged_iter;

		if (boost::next(words_iter) == words.end()) {
			++tagged_iter;
			++words_iter;
			continue;
		}

		string next_word = *boost::next(words_iter);
		string next_tag = *boost::next(tagged_iter);

		string next_base = info->get_conj(next_word, next_tag);
		if (base == "")
			base = word;

		if (aux_is_verb(*tagged_iter) && info->is_candidate_name(word)
		    && aux_is_verb(next_tag) && (next_base == "be" || next_base == "have")
		) {
			*tagged_iter = "NN";
		}
		++tagged_iter;
		++words_iter;
	}



	// "then" before PRP(he) is RB
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (boost::next(tagged_iter) != tagged.end()) {
		word = *words_iter;
		tag = *tagged_iter;

		if (boost::next(words_iter) == words.end()) {
			++tagged_iter;
			++words_iter;
			continue;
		}

		string next_word = *boost::next(words_iter);
		string next_tag = *boost::next(tagged_iter);

		string candidate_name = word + "_" + next_word;
		if (tag == "IN" && word == "then" && next_tag == "PRP" && aux_is_subject_PRP(next_word)) {
			*tagged_iter = "RB";
		}
		++tagged_iter;
		++words_iter;
	}

	// we/PRP smile/NN->VBP
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (boost::next(tagged_iter) != tagged.end()) {
		word = *words_iter;
		tag = *tagged_iter;

		if (boost::next(words_iter) == words.end()) {
			++tagged_iter;
			++words_iter;
			continue;
		}

		string next_word = *boost::next(words_iter);
		string next_tag = *boost::next(tagged_iter);

		if (tag == "PRP" && aux_is_subject_PRP(word) && aux_is_noun(next_tag) && info->is_candidate_verb(next_word)) {
			*boost::next(tagged_iter) = "VBP";
		}
		++tagged_iter;
		++words_iter;
	}

	// "like" before VBG is IN
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (boost::next(tagged_iter) != tagged.end()) {
		word = *words_iter;
		tag = *tagged_iter;

		if (boost::next(words_iter) == words.end())
			break;
		if (boost::next(boost::next(words_iter)) == words.end())
			break;

		string next_tag = *boost::next(tagged_iter);
		string next_word = *boost::next(words_iter);

		string next_next_tag = *boost::next(boost::next(tagged_iter));
		string next_next_word = *boost::next(boost::next(words_iter));


		if ((word == "more" || word == "less") && next_next_word == "than" && d->is_adjective(next_word)) {
			*tagged_iter = "JJR";
			*boost::next(tagged_iter) = "JJ";
			*boost::next(boost::next(tagged_iter)) = "IN";
		}
		++tagged_iter;
		++words_iter;
	}

	// "like" before VBG is IN
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (boost::next(tagged_iter) != tagged.end()) {
		word = *words_iter;
		tag = *tagged_iter;

		string next_tag = *boost::next(tagged_iter);

		if (word == "like" && tag != "IN" && next_tag == "VBG") {
			*tagged_iter = "IN";
		}
		++tagged_iter;
		++words_iter;
	}

	// will/NNP mark/NNP -> will/MD mark/VB
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (boost::next(tagged_iter) != tagged.end()) {
		word = *words_iter;
		tag = *tagged_iter;
		if (word == "will" && tag == "NNP" && (aux_is_noun(*boost::next(tagged_iter)) || aux_is_verb(*boost::next(tagged_iter)))
				&& boost::next(words_iter) != words.end() && info->is_candidate_verb(*boost::next(words_iter))) {
			*tagged_iter = "MD";
			*boost::next(tagged_iter) = "VB";
		}
		++tagged_iter;
		++words_iter;
	}

	// what/WP will/NNP the/DT rain/NN do/VB -> will/MD
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (boost::next(tagged_iter) != tagged.end()) {
		word = *words_iter;
		tag = *tagged_iter;
		if (word == "will" && tag == "NNP" && aux_is_article(*boost::next(tagged_iter))) {
			*tagged_iter = "MD";
		}
		++tagged_iter;
		++words_iter;
	}

	// what/WP do/NNS ... -> what/WP do/VBP
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (boost::next(tagged_iter) != tagged.end()) {
		word = *words_iter;
		tag = *tagged_iter;
		if (*boost::prior(tagged_iter) == "WP" && word == "do" && tag != "VBP") {
			*tagged_iter = "VBP";
		}
		++tagged_iter;
		++words_iter;
	}

	// was/VBD first/JJ noticed/VBN ... -> was/VBD first/RB noticed/VBN
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (boost::next(tagged_iter) != tagged.end()) {
		word = *words_iter;
		tag = *tagged_iter;
		if (aux_is_verb(*boost::prior(tagged_iter)) && tag == "JJ"
				&& (*boost::next(tagged_iter) == "VBN" || *boost::next(tagged_iter) == "VBD")) {
			*tagged_iter = "RB";
		} else if (aux_is_verb(*boost::prior(tagged_iter)) && tag == "CD" && d->is_adverb(word)
				&& (*boost::next(tagged_iter) == "VBN" || *boost::next(tagged_iter) == "VBD")) {
			*tagged_iter = "RB";
		}
		++tagged_iter;
		++words_iter;
	}

	// from/IN independently/RB produced/VBD zines/NNS -> from/IN independently/RB produced/VBN zines/NNS
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	bool trigger_IN = false;
	while (boost::next(tagged_iter) != tagged.end()) {
		word = *words_iter;
		tag = *tagged_iter;
		if (!d->is_adjective(word)) {
			++tagged_iter;
			++words_iter;
			continue;
		}
		if (tag == "IN" && word != "that")
			trigger_IN = true;
		if (trigger_IN && (tag == "VBN" || tag == "VBD") && aux_is_noun(*boost::next(tagged_iter))) {
			*tagged_iter = "JJ";
			trigger_IN = false;
		}
		if (trigger_IN && tag != "RB" && tag != "IN") {
			trigger_IN = false;
		}
		++tagged_iter;
		++words_iter;
	}

	// in/IN close/VB groups/NNS -> in/IN close/JJ groups/NNS
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (boost::next(tagged_iter) != tagged.end()) {
		word = *words_iter;
		tag = *tagged_iter;
		if (!d->is_adjective(word)) {
			++tagged_iter;
			++words_iter;
			continue;
		}
		if (tag == "IN" && word != "that")
			trigger_IN = true;
		if (trigger_IN && tag == "VB" && *boost::prior(tagged_iter) == "IN" && aux_is_noun(*boost::next(tagged_iter))
				&& d->is_adjective(word)) {
			*tagged_iter = "JJ";
			trigger_IN = false;
		}
		++tagged_iter;
		++words_iter;
	}

	// Corrects VB that should be NN in composite verbs (take/V place/V -> take/V place/N)
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (boost::next(tagged_iter) != tagged.end()) {
		word = *words_iter;
		tag = *tagged_iter;

		if (boost::next(words_iter) == words.end()) {
			++tagged_iter;
			++words_iter;
			continue;
		}

		string next_word = *boost::next(words_iter);
		string next_tag = *boost::next(tagged_iter);

		base = info->get_conj(word, tag);
		if (base == "")
			base = word;

		string candidate_verb = base + "_" + next_word;
		if (aux_is_verb(tag) && aux_is_verb(next_tag) && d->has_verb(candidate_verb) && info->is_candidate_name(next_word))
			*boost::next(tagged_iter) = "NN";
		if (!aux_is_verb(tag) && aux_is_verb(next_tag) && d->has_verb(candidate_verb) && info->is_candidate_name(next_word)) {
			*tagged_iter = next_tag;
			*boost::next(tagged_iter) = "NN";
		}
		++tagged_iter;
		++words_iter;
	}

	// Corrects "has/VBZ since/IN won/VBN
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (boost::next(tagged_iter) != tagged.end()) {
		string dummy_str;
		word = *words_iter;
		tag = *tagged_iter;
		string word_prior, tag_prior;
		if (words_iter != words.begin()) {
			word_prior = *boost::prior(words_iter);
			tag_prior = *boost::prior(tagged_iter);
		}

		string base_prior = info->get_conj(word_prior, tag_prior);
		if (base_prior == "")
			base = word;

		if ((tag == "IN" && word == "since") && base_prior == "have" && *boost::next(tagged_iter) == "VBN") {
			*tagged_iter = "RB";
		}
		++tagged_iter;
		++words_iter;
	}

	// Corrects VBG or VBN that should be VB
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (boost::next(tagged_iter) != tagged.end()) {
		string dummy_str;
		word = *words_iter;
		tag = *tagged_iter;
		if ((tag == "VBG") && info->regular_tag(word, "VBG", &dummy_str) == 0 && info->is_candidate_verb(word)) {
			if (*boost::prior(tagged_iter) == "IN")
				*tagged_iter = "NN";
			else
				*tagged_iter = "VB";
		}
		string base = info->get_conj(word, "VBN");
		if ((tag == "VBN") && info->regular_tag(word, "VBN", &dummy_str) == 0 && base == "" && info->is_candidate_verb(word)) {
			if (*boost::prior(tagged_iter) == "IN")
				*tagged_iter = "NN";
			else
				*tagged_iter = "VB";
		}
		++tagged_iter;
		++words_iter;
	}

	// Corrects VB that should be NN after VBN
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (boost::next(tagged_iter) != tagged.end()) {
		string dummy_str;
		word = *words_iter;
		tag = *tagged_iter;
		if ((tag == "VB" || tag == "VBP") && info->is_candidate_name(word)
				&& (*boost::prior(tagged_iter) == "VBN" || *boost::prior(tagged_iter) == "VBD")) {
			*boost::prior(tagged_iter) = "VBN";
			*tagged_iter = "NN";
		}
		++tagged_iter;
		++words_iter;
	}

	// Corrects VB that should be NN at the end of the phrase
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (boost::next(tagged_iter) != tagged.end()) {
		string dummy_str;
		word = *words_iter;
		tag = *tagged_iter;
		if (tag == "VB" && *boost::prior(tagged_iter) == "-period-" && info->is_candidate_name(word)) {
			vector<string>::iterator tagged_iter2 = tagged_iter;
			--tagged_iter2;
			bool aux_is_name = false;
			for (; tagged_iter2 != tagged.begin(); --tagged_iter2) {
				if (*tagged_iter2 == "IN") {
					aux_is_name = true;
					break;
				}
				if (*tagged_iter2 == "DT") {
					aux_is_name = true;
					break;
				}
				if (*tagged_iter2 == "VBP" || *tagged_iter2 == "VBZ" || *tagged_iter2 == "VBD") {
					aux_is_name = false;
					break;
				}
				if (aux_is_name) {
					*tagged_iter = "NN";
				}
			}
		}
		++tagged_iter;
		++words_iter;
	}

	// Corrects "can" that should be "MD"
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (boost::next(tagged_iter) != tagged.end()) {
		string dummy_str;
		word = *words_iter;
		tag = *tagged_iter;
		if ((word == "can" || word == "may") && tag != "MD" && boost::next(words_iter) != words.end()
				&& info->is_candidate_verb(*boost::next(words_iter))
				&& !(is_question_ && aux_is_verb(*boost::prior(tagged_iter)))) {
			*tagged_iter = "MD";
			*boost::next(tagged_iter) = "VB";
		}
		++tagged_iter;
		++words_iter;
	}
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (boost::next(tagged_iter) != tagged.end()) {
		string dummy_str;
		word = *words_iter;
		tag = *tagged_iter;
		if (word == "can"  && tag == "VB") {
			*tagged_iter = "MD";
		}
		++tagged_iter;
		++words_iter;
	}

	// Corrects "IN(that)" that should be "DT(that)"
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (boost::next(tagged_iter) != tagged.end()) {
		word = *words_iter;
		tag = *tagged_iter;

		string base = info->get_conj(word, tag);
		if (base == "")
			base = word;

		if (base == "do" && (tag == "VBZ" || tag == "VBP" || tag == "VBD" || tag == "VB" || tag == "VBG")
				&& *boost::next(tagged_iter) == "IN" && *boost::next(words_iter) == "that")
			*boost::next(tagged_iter) = "DT";

		if (tag == "IN" && word == "that" && (*boost::prior(tagged_iter) == "WRB")) {
			*tagged_iter = "DT";
		}
		if (tag == "IN" && word == "that" && (*boost::prior(tagged_iter) == "IN" || *boost::prior(tagged_iter) == "TO")) {
			*tagged_iter = "DT";
		}
		if (is_question_ && tag == "IN" && word == "that"
				&& (*boost::prior(tagged_iter) == "VBZ" || *boost::prior(tagged_iter) == "VBD")
				&& (*boost::next(tagged_iter) == "DT" || *boost::next(tagged_iter) == "RB")) {
			*tagged_iter = "DT";
		}
		if (tag == "IN" && word == "that"
				&& (*boost::next(tagged_iter) == "-period-" || *boost::next(tagged_iter) == "-comma-"
						|| *boost::next(tagged_iter) == "CC")) {
			*tagged_iter = "DT";
		}
		if (tag == "IN" && word == "that" && *boost::next(tagged_iter) == "IN") {
			*tagged_iter = "DT";
		}
		++tagged_iter;
		++words_iter;
	}

	// Corrects "names" that should be "VBD"
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (boost::next(tagged_iter) != tagged.end()) {
		word = *words_iter;
		tag = *tagged_iter;
		string base;
		double w = info->regular_tag(word, "VBD", &base);
		if (w != 0 && (tag.find("NN") != string::npos || tag == "JJ") && (*boost::next(tagged_iter) == "DT")
				&& (*boost::prior(tagged_iter) == "-comma-" || *boost::prior(tagged_iter) == "CC")) {
			*tagged_iter = "VBD";
		}
		++tagged_iter;
		++words_iter;
	}

	// Corrects VBN that should be "VBD"
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (boost::next(tagged_iter) != tagged.end()) {
		word = *words_iter;
		tag = *tagged_iter;
		string base = info->get_conj(word, "VBD");
		if (base != "" && (tag == "VBN")
				&& (boost::next(tagged_iter)->find("NN") != string::npos || *boost::next(tagged_iter) == "DT")
				&& *boost::prior(tagged_iter) != "VBG" && *boost::prior(tagged_iter) != "RB"
				&& *boost::prior(tagged_iter) != "DT"
				&& !aux_is_adjective(*boost::prior(tagged_iter))
		) {
			*tagged_iter = "VBD";
		}
		if (base != "" && (tag == "VBN") && boost::prior(tagged_iter) == tagged.begin()) {
			*tagged_iter = "VBD";
		}
//		if (base != "" && !is_question_ && tag == "VBN"
//			&& (aux_is_noun(*boost::prior(tagged_iter)) || aux_is_adjective(*boost::prior(tagged_iter)))
//			&& *boost::next(tagged_iter) == "TO"
//		) {
//			*tagged_iter = "VBD";
//		}

		++tagged_iter;
		++words_iter;
	}


	bool aux = false;
	bool DT_trigger = false;
	bool MD_trigger = false;

	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;

	while (boost::next(tagged_iter) != tagged.end() && words_iter != words.end()) {
		word = *words_iter;
		tag = *tagged_iter;

		if (tag == "MD")
			MD_trigger = true;

		if (tag == "CC" || tag == "-comma-" || tag == "TO" || tag == "IN")
			MD_trigger = false;

		if (((*boost::prior(tagged_iter) == "DT" && words_iter != words.begin()
				&& (*boost::prior(words_iter) == "the" || *boost::prior(words_iter) == "a"))
				|| *boost::prior(tagged_iter) == "VB") && *tagged_iter == "VB" && !MD_trigger) {
			if (info->is_candidate_name(word))
				*tagged_iter = "NN";
			else {
				if (boost::prior(words_iter) != words.begin() && info->is_candidate_verb(*boost::prior(words_iter)))
					*boost::prior(tagged_iter) = "VBP";
			}
		}

		if (*tagged_iter == "VBG" && *boost::next(tagged_iter) == "VBZ")
			*tagged_iter = "NN";

		/// artificially solve the tag NN that is tagged as VBG (after a "to be")
		// and solve the tag JJ that is tagged as VBN (after a "the")
		if (!DT_trigger) {
			if (*tagged_iter == "DT" && word != "this" && word != "that") {
				DT_trigger = true;
				++tagged_iter;
				++words_iter;
				continue;
			}
		}
		if (DT_trigger && tag == "VBG") {
			DT_trigger = false;
			*tagged_iter = "NN";
		}
		if (DT_trigger && tag == "VBN") {
			DT_trigger = false;
			if (d->is_adjective(word))
				*tagged_iter = "JJ";
		}
		if (DT_trigger && tag == "VB" && info->is_candidate_name(word)) {
			DT_trigger = false;
			//*tagged_iter= "NN";
		}
		if (DT_trigger && (*boost::prior(tagged_iter) == "NN" && tag == "PRP")) {
			DT_trigger = false;
		}
		if (DT_trigger && (tag != "JJ" && tag != "JJS" && tag != "JJR" && tag != "PRP" && tag != "PRP$" && tag != "RB")) {
			DT_trigger = false;
		}

		++tagged_iter;
		++words_iter;
	}


	aux = false;
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	bool CC_trigger = false;
	while (boost::next(tagged_iter) != tagged.end() && words_iter != words.end()) {
		word = *words_iter;
		tag = *tagged_iter;


		/// artificially solve the tag VB that is tagged as VBP (after an AUX)
		//cout << aux << ", " << std::boolalpha << ", "<< word<< "," << tag <<endl;
		if (!aux && word == "had" && tag == "VBN")
			*tagged_iter = "VBD";

		if (!aux && (tag == "VBP" || tag == "VBZ" || tag == "VBD" || tag == "VBN" || tag == "VBG" || tag == "VB" || tag == "MD" ///
		)) {
			base = info->get_conj(word, tag);
			if (base == "")
				base = word;
			if (base == "have" || base == "be" || base == "do" || word == "have" || word == "be" || word == "do"
					|| base == "being" || base == "get" || info->is_modal_verb(base)) {
				aux = true;
				CC_trigger = false;
				++tagged_iter;
				++words_iter;
				continue;
			}
		}
		if (aux && tag == "VBZ") {
			if (!is_question_)
				aux = false;
			string base2 = info->get_conj(word, tag);
			if (info->is_candidate_name(base2))
				*tagged_iter = "NNS";
		}
		if (!aux && (tag == "VBD" || tag == "VBN") && word == "thought" && *boost::prior(tagged_iter) == "IN"
				&& words_iter != words.begin() && *boost::prior(words_iter) == "of") {
			*tagged_iter = "NN";
		}

		if (aux && word == "found") {
			/// "found", "left" and "thought" are the only names that does not fit this model
			aux = false;
			*tagged_iter = "VBN";
			*words_iter = "find";

			++tagged_iter;
			++words_iter;
			continue;
		}
		if (aux && word == "born") {
			/// "found", "left" and "thought" are the only names that does not fit this model
			aux = false;
			*tagged_iter = "VBN";
			*words_iter = "born";

			++tagged_iter;
			++words_iter;
			continue;
		}
		if (aux && word == "hit") {
			/// "found", "left" and "thought" are the only names that does not fit this model
			aux = false;
			*tagged_iter = "VBN";
			++tagged_iter;
			++words_iter;
			continue;
		}
		if (aux && word == "put") {
			/// "found", "left" and "thought" and "put" are the only names that does not fit this model
			aux = false;
			*tagged_iter = "VBN";
			++tagged_iter;
			++words_iter;
			continue;
		}
		if (!aux && word == "found" && tag != "VBD") {
			/// "found", "left" and "thought" are the only names that does not fit this model
			aux = false;
			*tagged_iter = "VBD";
			*words_iter = "find";
		}
		if (aux && word == "left") {
			/// "found", "left" and "thought" are the only names that does not fit this model
			aux = false;
			CC_trigger = false;
			*tagged_iter = "VBN";
			*words_iter = "leave";
			continue;
		}
		if (aux && base == "do" && word == "like" && tag == "IN") {
			aux = false;
			CC_trigger = false;
			*tagged_iter = "VB";
			continue;
		}
		if (aux && word == "set") {
			aux = false;
			*tagged_iter = "VBN";
		}
		if (aux && base == "be" && (word == "held" || word == "fed")) {
			aux = false;
			*tagged_iter = "VBN";
			++tagged_iter;
			++words_iter;
			continue;
		}
		if (aux && base == "be" && (aux_is_noun(tag) || aux_is_adjective(tag)) && *boost::next(tagged_iter) == "WRB") {
			string base_tmp = info->get_conj(word, "VBG");
			if (base_tmp != "") {
				aux = false;
				*tagged_iter = "VBG";
				++tagged_iter;
				++words_iter;
				continue;
			}
		}
		if (  //is_question_
			 //&&
		(base == "do" || base == "would" || base == "should") && tag == "IN" && word == "like") {
			/// "found", "left" and "thought" are the only names that does not fit this model
			if (words_iter != words.end() && boost::next(tagged_iter) != tagged.end() && *boost::next(tagged_iter) != "VBG") {
				aux = false;
				*tagged_iter = "VB";
			}
		}
		if (aux && tag == "VBP") {
			aux = false;
			*tagged_iter = "VB";
		}
		if (is_question_ && aux && (aux_is_noun(tag) || aux_is_adjective(tag)) && base != "be" && words_iter != words.end()
				&& ! (aux_is_article(*boost::prior(tagged_iter))
						&& ( *boost::prior(words_iter) == "a" || *boost::prior(words_iter) == "an" || *boost::prior(words_iter) == "the")
				)
				&& !aux_is_adjective(*boost::prior(tagged_iter)) // red flag
				&& boost::next(words_iter) != words.end() && info->is_candidate_verb(word)) {
			if (*boost::next(words_iter) == "?") {
				// how does a person fall/NN -> how does a person fall/VB
				aux = false;
				*tagged_iter = "VB";
			} else if (words_iter != words.begin()
					&& (aux_is_noun(*boost::next(tagged_iter)) || aux_is_article(*boost::next(tagged_iter)))
					&& aux_is_noun(*boost::prior(tagged_iter))) {
				// "when did someone climb everest?"
				string prior_word = *boost::prior(words_iter);
				string next_word = *boost::next(words_iter);
				if (!info->is_candidate_verb(prior_word) && !info->is_candidate_verb(next_word)) {
					aux = false;
					*tagged_iter = "VB";
				}
			}
		}
		if (is_question_ 
		    && aux 
		    && (base == "do" || base == "would" || base == "should")
		    && aux_is_noun(tag) && info->is_candidate_verb(word) && *boost::prior(tagged_iter) == "NNP") {
			aux = false;
			*tagged_iter = "VB";
		}
		if (is_question_ && aux && (base == "do" || base == "would" || base == "should") && tag == "NN"
				&& *boost::prior(tagged_iter) == "DT" && words_iter != words.begin() && *boost::prior(words_iter) == "that") {
			/// "found", "left" and "thought" are the only names that does not fit this model
			aux = false;
			*tagged_iter = "VB";
		}
		if (is_question_ && aux && (base == "have" || base == "be") && tag == "VB" && info->is_candidate_name(word)) {
			aux = false;
			*tagged_iter = "NN";
			++tagged_iter;
			++words_iter;
			continue;
		}
		if (!is_question_ && aux && base == "do" && tag == "VBP" && *boost::prior(tagged_iter) == "RB") {
			aux = false;
			*tagged_iter = "VB";
		}

		if (aux && tag == "VBD" && !CC_trigger) {
			base = info->get_conj(word, "VBN");
			aux = false;
			if (base != "")
				*tagged_iter = "VBN";
			tag = "VBD";
		}
		if (!aux && tag == "VBN" && *boost::prior(tagged_iter) != "IN") {
			base = info->get_conj(word, "VBD");
			aux = false;
			if (base != "")
				*tagged_iter = "VBD";
		}
		if (aux && (*boost::prior(tagged_iter) == "NN" && tag == "PRP")) {
			aux = false;
		}
		if (aux
				&& !(is_question_
						&& (tag == "NN" || tag == "NNS" || tag == "NNP" || tag == "NNPS" || tag == "JJ" || tag == "CD"
								|| tag == "DT" || tag == "PRP" || tag == "PRP$"))
				&& !(tag == "IN" && word == "DUMMY-PREP-DATE") && !(tag == "VBN" && word == "been")
				&& !(tag == "VB" && word == "be") && tag != "RB" && tag != "CC" && tag != "VBD" && tag != "VBP"
				&& tag != "VBZ" && !(tag == "VBG" && word == "being")) {
			aux = false;
			CC_trigger = false;
		}
		if (tag == "CC")
			CC_trigger = true;

		if (tag == "IN" && (word == "that" || word == "since"))
			aux = false;
		++tagged_iter;
		++words_iter;
	}


	// sometimes "found" is tagged as name
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (boost::next(tagged_iter) != tagged.end() && words_iter != words.end()) {
		word = *words_iter;
		tag = *tagged_iter;
		if ((aux_is_noun(tag) || tag == "VBN") && word == "found") {
			*tagged_iter = "VBN";
			*words_iter = "find";
		}
		++tagged_iter;
		++words_iter;
	}

	// artificially solve the VBP that is tagged as VB (not for questions)
	bool name_trigger = false;
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (!is_question_ && boost::next(tagged_iter) != tagged.end() && words_iter != words.end()) {
		word = *words_iter;
		tag = *tagged_iter;
		if (!name_trigger) {
			if (*tagged_iter == "DT" || *tagged_iter == "NN" || *tagged_iter == "NNP" || *tagged_iter == "NNS"
					|| *tagged_iter == "NNPS" || *tagged_iter == "JJ" || *tagged_iter == "JJR" || *tagged_iter == "PRP") {
				name_trigger = true;
				++tagged_iter;
				++words_iter;
				continue;
			}
		}
		if (is_question_) {
			if (*tagged_iter == "WDT" || *tagged_iter == "WP" || *tagged_iter == "WRB") {
				name_trigger = true;
				++tagged_iter;
				++words_iter;
				continue;
			}
		}

		if (name_trigger && (tag == "VBP" || tag == "VBZ" || tag == "VBN" || tag == "VBD" || tag == "MD" || tag == "TO")) {
			name_trigger = false;
		}

		if (name_trigger && tag == "VB") {
			name_trigger = false;
			*tagged_iter = "VBP";
		}

		++tagged_iter;
		++words_iter;
	}

	name_trigger = false;
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (is_question_ && boost::next(tagged_iter) != tagged.end() && words_iter != words.end()) {
		word = *words_iter;
		tag = *tagged_iter;

		if (*tagged_iter == "WDT" || *tagged_iter == "WP" || *tagged_iter == "WRB") {
			name_trigger = true;
			++tagged_iter;
			++words_iter;
			continue;
		}

		if (name_trigger && (tag == "VBP" || tag == "VBZ" || tag == "VBN" || tag == "VBD" || tag == "MD" || tag == "TO")) {
			name_trigger = false;
		}

		if (name_trigger && tag == "VB") {
			name_trigger = false;
			*tagged_iter = "VBP";
		}

		++tagged_iter;
		++words_iter;
	}

	// Transform the parenthesis into -LBR- and -RBR-
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (boost::next(tagged_iter) != tagged.end()) {
		word = *words_iter;
		tag = *tagged_iter;
		if (tag == "/") {
			*tagged_iter = "CC";
			*words_iter = "and";
		}
		if (tag == "(") {
			*tagged_iter = "-LBR-";
			*words_iter = "-LBR-";
		}
		if (tag == ")") {
			*tagged_iter = "-RBR-";
			*words_iter = "-RBR-";
		}
		++tagged_iter;
		++words_iter;
	}

	vector<string> months = info->getMonths();

	// shuffle the date in the format "March 1, 1983" as "1 March 1983"
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (boost::next(tagged_iter) != tagged.end() && words_iter != words.end()) {
		word = *words_iter;
		tag = *tagged_iter;

		if (tag == "NNP" && find(months.begin(), months.end(), word) != months.end() && *boost::next(tagged_iter) == "CD"
				&& boost::next(boost::next(tagged_iter)) != tagged.end()
				&& *boost::next(boost::next(tagged_iter)) == "-comma-"
				&& boost::next(boost::next(boost::next(tagged_iter))) != tagged.end()
				&& *boost::next(boost::next(boost::next(tagged_iter))) == "CD") {
			string date_tmp = "[date]_";
			date_tmp += *boost::next(words_iter) + "_";
			date_tmp += *words_iter + "_";
			date_tmp += *boost::next(boost::next(boost::next(words_iter)));
			*tagged_iter = "CD";
			*words_iter = date_tmp;
			tagged_iter = tagged.erase(tagged_iter + 1, tagged_iter + 4);
			words_iter = words.erase(words_iter + 1, words_iter + 4);
			continue;
		}
		++words_iter;
		++tagged_iter;
	}

	// AD 400 -> [date]_400
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (boost::next(tagged_iter) != tagged.end() && words_iter != words.end()) {
		word = *words_iter;
		tag = *tagged_iter;

		if (word == "ad" && *boost::next(tagged_iter) == "CD") {
			string date_tmp = "[date]_";
			date_tmp += *boost::next(words_iter);
			*tagged_iter = "CD";
			*words_iter = date_tmp;
			tagged_iter = tagged.erase(tagged_iter + 1);
			words_iter = words.erase(words_iter + 1);
			continue;
		}

		if (word == "ad" && words_iter != words.begin() && *boost::prior(tagged_iter) == "CD") {
			string date_tmp = "[date]_";
			date_tmp += *boost::prior(words_iter);
			*tagged_iter = "CD";
			*words_iter = date_tmp;
			tagged_iter = tagged.erase(tagged_iter - 1);
			words_iter = words.erase(words_iter - 1);
			continue;
		}
		++words_iter;
		++tagged_iter;
	}

	// 22:00 is a [clock]
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (boost::next(tagged_iter) != tagged.end() && words_iter != words.end()) {
		word = *words_iter;
		tag = *tagged_iter;

		if (tag == "CD") {
			int pos = word.find(":");
			if (pos == string::npos || pos == 0 || pos == word.size() - 1) {
				++words_iter;
				++tagged_iter;
				continue;
			}
			if (isdigit(word.at(pos - 1)) && isdigit(word.at(pos + 1))) {
				string clock_tmp = "[clock]_";
				clock_tmp += word;
				*words_iter = clock_tmp;
			}
		}
		++words_iter;
		++tagged_iter;
	}

	// "there" -> "[place]_there" when "DT NN there"
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (boost::next(tagged_iter) != tagged.end()) {
		word = *words_iter;
		tag = *tagged_iter;

		if (boost::next(words_iter) == words.end())
			break;
		string next_word = *boost::next(words_iter);
		string next_tag = *boost::next(tagged_iter);
		base = info->get_conj(next_word, next_tag);

		if (word == "there" && (boost::prior(tagged_iter)->find("NN") != string::npos)
				&& !(base == "be" && aux_is_verb(*boost::next(tagged_iter)))) {
			string date = "[place]_";
			date += word;
			*words_iter = date;
			*tagged_iter = "NN";
		}
		if (word == "there" && *boost::next(tagged_iter) == "PRP" && base != "be") {
			string date = "[place]_";
			date += word;
			*words_iter = date;
			*tagged_iter = "NN";
		}
		if (word == "there" && *boost::next(tagged_iter) == "-comma-" && base != "be") {
			string date = "[place]_";
			date += word;
			*words_iter = date;
			*tagged_iter = "NN";
		}
		++words_iter;
		++tagged_iter;
	}

	// some preposition are places ("like [place]_below in the image")
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (boost::next(tagged_iter) != tagged.end()) {
		word = *words_iter;
		tag = *tagged_iter;

		if (word == "below"
				&& (boost::prior(tagged_iter)->find("NN") != string::npos || *boost::prior(tagged_iter) == "IN"
				//&& boost::prior(tagged_iter) != tagged.begin()
				//&& *boost::prior(boost::prior(tagged_iter)) == "DT"
				)
				&& (*boost::prior(tagged_iter) == "IN" || *boost::prior(tagged_iter) == "-period-"
						|| *boost::prior(tagged_iter) == "-comma-" || *boost::prior(tagged_iter) == "CC")) {
			string date = "[place]_";
			date += word;
			*words_iter = date;
			*tagged_iter = "NN";
		}
		++words_iter;
		++tagged_iter;
	}

	// Groups together date numbers and names
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (boost::next(tagged_iter) != tagged.end()) {
		word = *words_iter;
		tag = *tagged_iter;

		if (tag == "CD"
				&& ((*boost::prior(tagged_iter) == "IN" && words_iter != words.begin() && *boost::prior(words_iter) != "of")
						|| *boost::prior(tagged_iter) == "TO") && !aux_is_noun(*boost::next(tagged_iter)) // I was here in 1945 and I loved it.
						) {
			try {
				int year = boost::lexical_cast<int>(word);
				if (year > 1950 && year < 2020) {
					/// Temporary: you have to implement a smarter solution for understanding a date!!!
					//if (true) {
					string date = "[date]_";
					date += word;
					*words_iter = date;
				}
			} catch (std::exception &e) {
			}
		} else if (tag == "NNP" && find(months.begin(), months.end(), word) != months.end()
				&& ( (*boost::prior(tagged_iter) == "IN" || *boost::prior(tagged_iter) == "TO")
					|| (boost::prior(tagged_iter) != tagged.begin()
						&& (*boost::prior(boost::prior(tagged_iter)) == "IN" || *boost::prior(boost::prior(tagged_iter)) == "TO") )
				   )
		) {

			if (word == "march" && *boost::prior(tagged_iter) == "PRP$") {
				++tagged_iter;
				++words_iter;
				continue; // "his march"
			}

			if (word == "may" && (*boost::prior(tagged_iter) == "PRP" || *boost::next(tagged_iter) == "RB")) {
				++tagged_iter;
				++words_iter;
				continue; // "they may"
			}

			bool prior, next;
			prior = next = false;

			string date = "[date]_";
			if (words_iter != words.begin() && *boost::prior(tagged_iter) == "CD") {
				date += *boost::prior(words_iter) + "_";
				prior = true;
			}
			date += word;
			if (words_iter != words.end() && boost::next(words_iter) != words.end() && *boost::next(tagged_iter) == "CD") {
				date += string("_") + *boost::next(words_iter);
				next = true;
			}

			words_iter = words.erase(words_iter);
			tagged_iter = tagged.erase(tagged_iter);
			if (next) {
				words_iter = words.erase(words_iter);
				tagged_iter = tagged.erase(tagged_iter);
			}
			if (prior) {
				words_iter = words.erase(words_iter - 1);
				tagged_iter = tagged.erase(tagged_iter - 1);
			}

			words.insert(words_iter, date);
			tagged.insert(tagged_iter, "CD");
			continue;

		} else if (word.find("mid-") != string::npos && has_number(word)) {
			bool prior, next;
			prior = next = false;

			string date = "[date]_" + word;

			*words_iter = date;
			*tagged_iter = "CD";

		}

		++tagged_iter;
		++words_iter;
	}

	vector<string> chrono = info->getChronoNames();
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (boost::next(tagged_iter) != tagged.end()) {
		word = *words_iter;
		tag = *tagged_iter;

		if (tag == "NN" && *boost::prior(tagged_iter) != "IN"
				&& !(aux_is_verb(*boost::prior(tagged_iter)) && *boost::next(tagged_iter) == "VBN")
				&& !(aux_is_verb(*boost::prior(tagged_iter)) && *boost::next(tagged_iter) == "VBD")
				&& *boost::prior(tagged_iter) != "TO" && *boost::prior(tagged_iter) != "DT"
				&& *boost::next(tagged_iter) != "POS" && find(chrono.begin(), chrono.end(), word) != chrono.end()) {
			*words_iter = string("[date]_") + *words_iter;
			words.insert(words_iter, "DUMMY-PREP-DATE");
			tagged.insert(tagged_iter, "IN");
			words_iter = words.begin();
			tagged_iter = tagged.begin() + 1;
			continue;
		}

		// "every 5 years"
		if (tag == "CD" && *boost::prior(tagged_iter) == "DT" && aux_is_noun(*boost::next(tagged_iter))
				&& words_iter != words.begin() && *boost::prior(words_iter) == "every" && words_iter != words.end()
				&& boost::next(words_iter) != words.end()
				&& (*boost::next(words_iter) == "days" || *boost::next(words_iter) == "week"
						|| *boost::next(words_iter) == "months" || *boost::next(words_iter) == "years")) {
			string prior_word = *boost::prior(words_iter);
			string next_word = *boost::next(words_iter);
			*words_iter = string("[date]_") + prior_word + "_" + word + "_" + next_word;
			words_iter = words.erase(words_iter - 1);
			tagged_iter = tagged.erase(tagged_iter - 1);
			words_iter = words.erase(words_iter + 1);
			tagged_iter = tagged.erase(tagged_iter + 1);

			words_iter = words.begin();
			tagged_iter = tagged.begin() + 1;
			continue;
		}
		// 5 years ago
		if (tag == "CD" && aux_is_noun(*boost::next(tagged_iter)) && words_iter != words.end()
				&& boost::next(words_iter) != words.end() && boost::next(boost::next(words_iter)) != words.end()
				&& (*boost::next(words_iter) == "days" || *boost::next(words_iter) == "weeks"
						|| *boost::next(words_iter) == "months" || *boost::next(words_iter) == "years")
				&& *boost::next(boost::next(words_iter)) == "ago") {
			string next_word = *boost::next(words_iter);
			string next_next_word = *boost::next(boost::next(words_iter));
			*words_iter = string("[date]_") + word + "_" + next_word + "_" + next_next_word;
			words_iter = words.erase(words_iter + 1);
			tagged_iter = tagged.erase(tagged_iter + 1);
			words_iter = words.erase(words_iter);
			tagged_iter = tagged.erase(tagged_iter);

			words_iter = words.begin();
			tagged_iter = tagged.begin() + 1;
			continue;
		}
		// years ago
		if (aux_is_noun(*tagged_iter) && words_iter != words.end() && boost::next(words_iter) != words.end()
				&& (word == "days" || word == "weeks" || word == "months" || word == "years")
				&& *boost::next(words_iter) == "ago") {
			string next_word = *boost::next(words_iter);
			*words_iter = string("[date]_") + word + "_" + next_word;
			words_iter = words.erase(words_iter + 1);
			tagged_iter = tagged.erase(tagged_iter + 1);

			words_iter = words.begin();
			tagged_iter = tagged.begin() + 1;
			continue;
		}

		if ((tag == "NN" || tag == "CD") && (aux_is_verb(*boost::prior(tagged_iter)) && *boost::next(tagged_iter) == "VBN")
				&& find(chrono.begin(), chrono.end(), word) != chrono.end()) {
			*tagged_iter = "RB";
		}

		if ((tag == "NN" || tag == "CD") && (aux_is_verb(*boost::prior(tagged_iter)) && *boost::next(tagged_iter) == "VBD")
				&& find(chrono.begin(), chrono.end(), word) != chrono.end()) {
			*tagged_iter = "RB";
		}

		++tagged_iter;
		++words_iter;
	}

	// IN(under) VBD(renewed) -> IN(under) VBN(renewed)
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (boost::next(tagged_iter) != tagged.end()) {
		word = *words_iter;
		tag = *tagged_iter;
		if (!d->is_adjective(word)) {
			++tagged_iter;
			++words_iter;
			continue;
		}
		if (tag == "VBD" && words_iter != words.begin() && *boost::prior(tagged_iter) == "IN"
				&& *boost::prior(words_iter) != "that" && *boost::prior(words_iter) != "if"
				&& *boost::prior(words_iter) != "whether" && *boost::prior(words_iter) != "because")
			*tagged_iter = "JJ";

		++tagged_iter;
		++words_iter;
	}

	// Artificially solve the tag JJ that is tagged VBN after a VBP
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (boost::next(tagged_iter) != tagged.end()) {
		word = *words_iter;
		tag = *tagged_iter;
		if (!d->is_adjective(word)) {
			++tagged_iter;
			++words_iter;
			continue;
		}
		if (tag == "VBN"
				&& (*boost::prior(tagged_iter) != "VBP" && *boost::prior(tagged_iter) != "VBN"
						&& *boost::prior(tagged_iter) != "VB" && *boost::prior(tagged_iter) != "VBZ"
						&& *boost::prior(tagged_iter) != "VBD" && *boost::prior(tagged_iter) != "WP"
						&& *boost::prior(tagged_iter) != "WDT"
						&& (*boost::prior(tagged_iter) != "RB" && boost::prior(tagged_iter) != tagged.begin()
								&& *boost::prior(boost::prior(tagged_iter)) != "VBZ"
								&& *boost::prior(boost::prior(tagged_iter)) != "VBP"
								&& *boost::prior(boost::prior(tagged_iter)) != "VB"))
				&& boost::next(tagged_iter)->find("NN") != string::npos)
			*tagged_iter = "JJ";

		++tagged_iter;
		++words_iter;
	}

	// A VBN after WP or WDT is VBD
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (boost::next(tagged_iter) != tagged.end()) {
		word = *words_iter;
		tag = *tagged_iter;

		if (tag == "VBN" && (*boost::prior(tagged_iter) == "WP" || *boost::prior(tagged_iter) == "WDT"))
			*tagged_iter = "VBD";

		++tagged_iter;
		++words_iter;
	}

	// If there is a CD with [date] put a preposition in front
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (boost::next(tagged_iter) != tagged.end()) {
		word = *words_iter;
		tag = *tagged_iter;

		if (tag == "CD" && *boost::prior(tagged_iter) != "IN" && *boost::prior(tagged_iter) != "TO"
				&& *boost::prior(tagged_iter) != "DT" && *boost::next(tagged_iter) != "POS"
				&& words_iter->find("[date]") != string::npos) {
			words.insert(words_iter, "DUMMY-PREP-DATE");
			tagged.insert(tagged_iter, "IN");
			words_iter = words.begin();
			tagged_iter = tagged.begin() + 1;
			continue;
		}

		++tagged_iter;
		++words_iter;
	}

	// If there is a NN with [place] put a preposition in front
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (boost::next(tagged_iter) != tagged.end()) {
		word = *words_iter;
		tag = *tagged_iter;

		if ((aux_is_noun(tag) || aux_is_adjective(tag)) && *boost::prior(tagged_iter) != "IN" && *boost::prior(tagged_iter) != "DT"
				&& *boost::next(tagged_iter) != "POS" && words_iter->find("[place]") != string::npos) {
			words.insert(words_iter, "DUMMY-PREP-PLACE");
			tagged.insert(tagged_iter, "IN");
			words_iter = words.begin();
			tagged_iter = tagged.begin() + 1;
			continue;
		}

		++tagged_iter;
		++words_iter;
	}

	// Corrects "DT(that)" that should be "WDT(that)"
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (boost::next(tagged_iter) != tagged.end()) {
		word = *words_iter;
		tag = *tagged_iter;
		if (tag == "DT" && word == "that"
				&& (*boost::prior(tagged_iter) == "NNP" || *boost::prior(tagged_iter) == "NN"
						|| *boost::prior(tagged_iter) == "NNS" || *boost::prior(tagged_iter) == "NNP"
						|| *boost::prior(tagged_iter) == "NNPS" || *boost::prior(tagged_iter) == "DT"
						|| *boost::prior(tagged_iter) == "CD")
				&& (*boost::next(tagged_iter) == "VBZ" || *boost::next(tagged_iter) == "VBP"
						|| *boost::next(tagged_iter) == "VBD" || *boost::next(tagged_iter) == "NNP"
						|| *boost::next(tagged_iter) == "NN" || *boost::next(tagged_iter) == "NNPS"
						|| *boost::next(tagged_iter) == "NNS" || *boost::next(tagged_iter) == "PRP"
						|| *boost::next(tagged_iter) == "PRP$" || *boost::next(tagged_iter) == "CD"
						|| *boost::next(tagged_iter) == "MD")) {
			*tagged_iter = "WDT";
		}
		if (tag == "DT" && word == "that" && words_iter != words.begin()
				&& (*boost::prior(words_iter) == "them" || *boost::prior(words_iter) == "him"))
			*tagged_iter = "WDT";
		++tagged_iter;
		++words_iter;
	}

	// Corrects "WDT(that)" that should be "DT(that)"
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (boost::next(tagged_iter) != tagged.end()) {
		word = *words_iter;
		tag = *tagged_iter;
		if (tag == "WDT" && word == "that" && aux_is_noun(*boost::next(tagged_iter))
				&& (*boost::prior(tagged_iter) == "TO" || *boost::prior(tagged_iter) == "IN")) {
			*tagged_iter = "DT";
		}
		++tagged_iter;
		++words_iter;
	}

	// Corrects "DT(that)" that should be "IN(that)"
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (boost::next(tagged_iter) != tagged.end()) {
		word = *words_iter;
		tag = *tagged_iter;
		if (tag == "DT" && word == "that" && aux_is_verb(*boost::prior(tagged_iter)) && (*boost::next(tagged_iter) == "EX")) {
			*tagged_iter = "IN";
		}
		if (tag == "DT" && word == "that" && *boost::prior(tagged_iter) == "DT") {
			*tagged_iter = "IN";
		}
		++tagged_iter;
		++words_iter;
	}

	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	bool say_trigger = false;
	while (boost::next(tagged_iter) != tagged.end()) {
		word = *words_iter;
		tag = *tagged_iter;

		base = info->get_conj(word, tag);
		if (base == "")
			base = word;
		if (base == "say" || base == "write" || base == "think" /// communications verbs
		|| base == "watch" || base == "mean" || base == "suggest" || base == "believe" || base == "discover"
				|| base == "doubt" || base == "forget" || base == "suspect" || base == "know" || base == "study"
				|| base == "remind" || base == "rule" || base == "estimate"
				|| base == "count" || (d->get_levin_verb(base) == "verb.communication"))
			say_trigger = true;

		if (tag == "DT" && word == "that" && say_trigger) {
			*tagged_iter = "IN";
		}
		++tagged_iter;
		++words_iter;
	}

	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	say_trigger = false;
	while (boost::next(tagged_iter) != tagged.end()) {
		word = *words_iter;
		tag = *tagged_iter;

		if (boost::next(words_iter) == words.end()) {
			++tagged_iter;
			++words_iter;
			continue;
		}

		string next_word = *boost::next(words_iter);
		string next_tag = *boost::next(tagged_iter);

		base = info->get_conj(word, "VBZ");
		if (base == "")
			base = word;
		if (base == "say" || base == "write" || base == "think" /// communications verbs
				|| base == "watch" || base == "mean" || base == "suggest" || base == "believe" || base == "discover" || base == "doubt"
						|| base == "forget" || base == "suspect" || base == "know" || base == "study"
						|| base == "remind" || base == "rule"
						|| base == "count" || (d->get_levin_verb(base) == "verb.communication"))
			say_trigger = true;

		if (tag == "NNS" && next_word == "that" && say_trigger) {
			/// you should also check that "the means/NNS that" is tagged correcly
			*tagged_iter = "VBZ";
		}
		++tagged_iter;
		++words_iter;
	}


	// After the communication verbs there cannot be a WDT
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (boost::next(tagged_iter) != tagged.end()) {
		word = *words_iter;
		tag = *tagged_iter;

		if (boost::next(words_iter) == words.end()) {
			++tagged_iter;
			++words_iter;
			continue;
		}

		base = info->get_conj(word, tag);
		if (base == "")
			base = word;
		if ((base == "say" || base == "write" || base == "think" /// communications verbs
		|| base == "word" || base == "mean" || base == "suggest" || base == "believe" || base == "discover" || base == "doubt"
				|| base == "forget" || base == "suspect" || base == "know" || base == "study"
				|| base == "remind" || base == "rule"
				|| base == "count" || d->get_levin_verb(base) == "verb.communication") && words_iter != words.end()
				&& *boost::next(words_iter) == "that" && *boost::next(tagged_iter) == "WDT") {
			*boost::next(tagged_iter) = "IN";
		}
		++tagged_iter;
		++words_iter;
	}

	// correct JJ tagged as VBN
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (boost::next(tagged_iter) != tagged.end()) {
		word = *words_iter;
		tag = *tagged_iter;

		if (!d->is_adjective(word)) {
			++tagged_iter;
			++words_iter;
			continue;
		}

		if (*tagged_iter == "VBN" && *boost::prior(tagged_iter) == "IN") {
			*tagged_iter = "JJ";
		}
		if (*tagged_iter == "VBN" && *boost::prior(tagged_iter) == "VBG"
				&& (aux_is_noun(*boost::next(tagged_iter)) || aux_is_adjective(*boost::next(tagged_iter)))) {
			*tagged_iter = "JJ";
		}
		if (*tagged_iter == "VBN" && *boost::prior(tagged_iter) == "CC" && *boost::next(tagged_iter) == "JJ") {
			*tagged_iter = "JJ";
		}
		if (*tagged_iter == "VBN" && *boost::prior(tagged_iter) == "VBN" && *boost::next(tagged_iter) == "JJ") {
			*tagged_iter = "JJ";
		}
		++tagged_iter;
		++words_iter;
	}


	// An NNS between two NNPs or NN becomes VBZ
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (boost::next(tagged_iter) != tagged.end()) {
		word = *words_iter;
		tag = *tagged_iter;

		base = info->get_conj(word, "VBZ");
		if (base == "")
			base = word;

		if (!info->is_candidate_verb(base)) {
			++tagged_iter;
			++words_iter;
			continue;
		}

		if (*tagged_iter == "NNS" && *boost::prior(tagged_iter) == "NNP" && *boost::next(tagged_iter) == "NNP") {
			*tagged_iter = "VBZ";
		}
		if (*tagged_iter == "NNS" && *boost::prior(tagged_iter) == "NN" && *boost::next(tagged_iter) == "NN") {
			*tagged_iter = "VBZ";
		}
		if (*tagged_iter == "NNS" && *boost::prior(tagged_iter) == "NNP" && *boost::next(tagged_iter) == "DT") {
			*tagged_iter = "VBZ";
		}
		if (*tagged_iter == "NNS" && *boost::prior(tagged_iter) == "NN" && *boost::next(tagged_iter) == "DT") {
			*tagged_iter = "VBZ";
		}
		if (*tagged_iter == "NNS" && *boost::prior(tagged_iter) == "RB" && *boost::next(tagged_iter) == "DT") {
			*tagged_iter = "VBZ";
		}
		if (*tagged_iter == "NNS" && *boost::prior(tagged_iter) == "NNPS" && *boost::next(tagged_iter) == "RB") {
			*tagged_iter = "VBZ";
		}
		if (*tagged_iter == "NNS" && *boost::prior(tagged_iter) == "WDT" && *boost::next(tagged_iter) == "DT") {
			*tagged_iter = "VBZ";
		}
		if (*tagged_iter == "NNS" && *boost::prior(tagged_iter) == "WDT" && *boost::next(tagged_iter) == "WDT") {
			*tagged_iter = "VBZ";
		}
		if (*tagged_iter == "NNS" && *boost::prior(tagged_iter) == "WDT" && *boost::next(tagged_iter) == "VBG") {
			*tagged_iter = "VBZ";
		}
		if (*tagged_iter == "NNS" && *boost::prior(tagged_iter) == "WP" && *boost::next(tagged_iter) == "WP") {
			*tagged_iter = "VBZ";
		}
		if (*tagged_iter == "NNS" && (*boost::prior(tagged_iter) == "IN" || *boost::prior(tagged_iter) == "WDT")
				&& words_iter != words.begin() && *boost::prior(words_iter) == "that") {
			*tagged_iter = "VBZ";
		}
		if (*tagged_iter != "VBZ" && *boost::prior(tagged_iter) == "DT" && words_iter != words.begin()
				&& *boost::prior(words_iter) == "that" && !is_question_) {
			*tagged_iter = "VBZ";
		}
		if (*tagged_iter == "NNS" && *boost::prior(tagged_iter) == "CC") {
			vector<string>::iterator tagged_iter2 = tagged_iter;
			--tagged_iter2;
			for (; tagged_iter2 != tagged.begin(); --tagged_iter2) {
				if (*tagged_iter2 == "NNS")
					break;
				if (*tagged_iter2 == "VBZ")
					*tagged_iter = "VBZ";
			}
		}
		++tagged_iter;
		++words_iter;
	}


	// A VBN after "the" can become JJ
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (boost::next(tagged_iter) != tagged.end()) {
		word = *words_iter;
		tag = *tagged_iter;

		if (!d->is_adjective(word)) {
			++tagged_iter;
			++words_iter;
			continue;
		}

		if ((*tagged_iter == "VBN" || *tagged_iter == "VBD") && words_iter != words.begin()
				&& *boost::prior(tagged_iter) == "DT"
				&& (*boost::prior(words_iter) == "the" || *boost::prior(words_iter) == "a")) {
			*tagged_iter = "JJ";
		}

		if ((*tagged_iter == "VBN" || *tagged_iter == "VBD") && words_iter != words.begin()
				&& *boost::prior(tagged_iter) == "CD" && d->is_adjective(word) && aux_is_noun(*boost::next(tagged_iter))) {
			string prior_word = *boost::prior(words_iter);

			if (!aux_is_date(prior_word))
				*tagged_iter = "JJ";
		}
		++tagged_iter;
		++words_iter;
	}

	// A NNS after PRP can become VBZ (not for question)
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (boost::next(tagged_iter) != tagged.end() && !is_question_) {
		word = *words_iter;
		tag = *tagged_iter;

		if ((*tagged_iter == "NNS" || tag == "NNP") && words_iter != words.begin() && *boost::prior(tagged_iter) == "PRP"
				&& (*boost::prior(words_iter) == "he" || *boost::prior(words_iter) == "she"
						|| *boost::prior(words_iter) == "it" || *boost::prior(words_iter) == "someone"
						|| *boost::prior(words_iter) == "something") && info->get_conj(word, "VBZ") != "") {
			*tagged_iter = "VBZ";
		}
		if ((*tagged_iter == "NN" || tag == "NNP" || tag == "VB") && words_iter != words.begin()
				&& *boost::prior(tagged_iter) == "PRP"
				&& (*boost::prior(words_iter) == "they" || *boost::prior(words_iter) == "you"
						|| *boost::prior(words_iter) == "i") && info->is_candidate_verb(word)) {

			*tagged_iter = "VBP";
		}

		++tagged_iter;
		++words_iter;
	}

	// A NN after PRP can become VBD
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (boost::next(tagged_iter) != tagged.end()) {
		word = *words_iter;
		tag = *tagged_iter;

		base = info->get_conj(word, "VBD");
		if (aux_is_noun(*tagged_iter) && base != "" && words_iter != words.begin()
				&& ((*boost::prior(tagged_iter) == "PRP"
						&& (*boost::prior(words_iter) == "he" || *boost::prior(words_iter) == "she"
								|| *boost::prior(words_iter) == "it" || *boost::prior(words_iter) == "they"
								|| *boost::prior(words_iter) == "you" || *boost::prior(words_iter) == "i"))
						|| (*boost::prior(tagged_iter) == "WP" && !aux_is_noun(*boost::next(tagged_iter))))) {
			*tagged_iter = "VBD";
			continue;
		}

		if (aux_is_noun(*tagged_iter) && base != "" && words_iter != words.begin() && *boost::prior(tagged_iter) == "CC"
				&& *boost::next(tagged_iter) == "RB") {
			*tagged_iter = "VBD";
			continue;
		}

		++tagged_iter;
		++words_iter;
	}

	// A NN preceding a PRP can become VBP
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (boost::next(tagged_iter) != tagged.end() && !is_question_) {
		word = *words_iter;
		tag = *tagged_iter;
		if (!info->is_candidate_verb(word)) {
			++tagged_iter;
			++words_iter;
			continue;
		}
		if (*tagged_iter == "NN" && words_iter != words.end() && (boost::next(words_iter) != words.end())
				&& (*boost::next(words_iter) == "him" || *boost::next(words_iter) == "her" || *boost::next(words_iter) == "it"
						|| *boost::next(words_iter) == "them")) {
			*tagged_iter = "VBP";
		}

		++tagged_iter;
		++words_iter;
	}

	// A NN after MD can become VB
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (boost::next(tagged_iter) != tagged.end()) {
		word = *words_iter;
		tag = *tagged_iter;
		if (!info->is_candidate_verb(word)) {
			++tagged_iter;
			++words_iter;
			continue;
		}
		if (*tagged_iter == "NN" && *boost::prior(tagged_iter) == "MD") {
			*tagged_iter = "VB";
		}
		if (*tagged_iter == "NNP" && *boost::prior(tagged_iter) == "MD") {
			*tagged_iter = "VB";
		}

		++tagged_iter;
		++words_iter;
	}

	// artificially solve the NN that is tagged as VB

	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (is_question_ && boost::next(tagged_iter) != tagged.end() && words_iter != words.end()) {
		word = *words_iter;
		tag = *tagged_iter;

		if (!info->is_candidate_name(word)) {
			++tagged_iter;
			++words_iter;
			continue;
		}

		if ((*boost::prior(tagged_iter) == "VBZ" || *boost::prior(tagged_iter) == "VBP" || *boost::prior(tagged_iter) == "VBD")
				&& (tag == "VB")) {
			*tagged_iter = "NN";
		}

		++tagged_iter;
		++words_iter;
	}

	// After TO ... RB... NN -> TO ...RB... VB
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	bool to_trigger = false;
	while (boost::next(tagged_iter) != tagged.end()) {
		word = *words_iter;
		tag = *tagged_iter;

		if (*tagged_iter == "TO"
		//&& *boost::next(tagged_iter) == "RB"
				) {
			to_trigger = true;
			++tagged_iter;
			++words_iter;
			continue;
		}
		if (to_trigger && info->is_candidate_verb(word)
				&& (*tagged_iter == "NN" || *tagged_iter == "NNP" || *tagged_iter == "JJ" || *tagged_iter == "VBN"
						|| *tagged_iter == "VBP" || *tagged_iter == "RB")) {
			*tagged_iter = "VB";
			to_trigger = false;
		}
		if (*tagged_iter != "RB") {
			to_trigger = false;
		}

		++tagged_iter;
		++words_iter;
	}

	// After MD ... RB... NN -> TO ...RB... VB
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	to_trigger = false;
	while (boost::next(tagged_iter) != tagged.end()) {
		word = *words_iter;
		tag = *tagged_iter;

		if (*tagged_iter == "MD"
		//&& *boost::next(tagged_iter) == "RB"
				) {
			to_trigger = true;
			++tagged_iter;
			++words_iter;
			continue;
		}
		if (to_trigger && info->is_candidate_verb(word)
				&& (*tagged_iter == "NN" || *tagged_iter == "NNP" || *tagged_iter == "JJ" || *tagged_iter == "VBN"
						|| *tagged_iter == "VBP")) {
			*tagged_iter = "VB";
			to_trigger = false;
		}
		if (*tagged_iter != "RB") {
			to_trigger = false;
		}

		++tagged_iter;
		++words_iter;
	}

	// A V after POS become NN
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (boost::next(tagged_iter) != tagged.end()) {
		word = *words_iter;
		tag = *tagged_iter;

		// if( info->get_conj(word, "NN") == "") {
		//      ++tagged_iter;
		//      ++words_iter;
		//      continue;
		// }
		base = info->get_conj(word, "NN");
		if (base == "" ) {
			base = word;
		}
		if ((*tagged_iter == "VBP" || *tagged_iter == "VBD" || *tagged_iter == "VBG" || *tagged_iter == "VB")
				&& info->is_candidate_name(base)
				&& (*boost::prior(tagged_iter) == "POS" || *boost::prior(tagged_iter) == "PRP$"
						|| (*boost::prior(tagged_iter) == "DT" && *boost::prior(words_iter) == "any")
						|| (*boost::prior(tagged_iter) == "RB" && *boost::prior(words_iter) == "any"))) {
			*tagged_iter = "NN";
		}
		if (*tagged_iter == "VBZ"
			&& (base != "[wrong_tag]" && info->is_candidate_name(base) && base != "i" && base != "do")
		     && (*boost::prior(tagged_iter) == "POS" || *boost::prior(tagged_iter) == "PRP$")) {
			*tagged_iter = "NNS";
		}
		++tagged_iter;
		++words_iter;
	}

	// the/DT final/JJ warning/VBG->NN
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	DT_trigger= false;
	while (!is_question_ && boost::next(tagged_iter) != tagged.end()) {
		word = *words_iter;
		tag = *tagged_iter;

		if (DT_trigger && *tagged_iter == "VBG" && info->is_candidate_name(word)
				&& *boost::prior(tagged_iter) == "JJ"
				&& ( *boost::next(tagged_iter) == "IN" || *boost::next(tagged_iter) == "TO" )
		) {
			*tagged_iter = "NN";
			DT_trigger = false;
		}
		if(!DT_trigger && !aux_is_article(tag) && tag == "PRP$") {
			DT_trigger = true;
		}

		if(DT_trigger && !aux_is_noun(tag) && !aux_is_article(tag) && !aux_is_adjective(tag) && tag != "PRP$") {
			DT_trigger = false;
		}

		++tagged_iter;
		++words_iter;
	}

	// A NN before a DT become VBP
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (boost::next(tagged_iter) != tagged.end()) {
		word = *words_iter;
		tag = *tagged_iter;

		if (!info->is_candidate_verb(word)) {
			++tagged_iter;
			++words_iter;
			continue;
		}
		if (*tagged_iter == "NN" && *boost::prior(tagged_iter) == "NNS" && *boost::next(tagged_iter) == "DT") {
			*tagged_iter = "VBP";
		}
		++tagged_iter;
		++words_iter;
	}

	// A NN before a TO|IN and after a CC
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (boost::next(tagged_iter) != tagged.end()) {
		word = *words_iter;
		tag = *tagged_iter;

		if (!info->is_candidate_verb(word)) {
			++tagged_iter;
			++words_iter;
			continue;
		}
		if (*tagged_iter == "NN" && *boost::prior(tagged_iter) == "CC"
				&& (*boost::next(tagged_iter) == "TO" || *boost::next(tagged_iter) == "IN")) {
			*tagged_iter = "VBP";
		}
		++tagged_iter;
		++words_iter;
	}

	// "on its own"
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (boost::next(tagged_iter) != tagged.end()) {
		word = *words_iter;
		tag = *tagged_iter;

		if (word == "own" && *boost::prior(tagged_iter) == "PRP$" && boost::prior(tagged_iter) != tagged.begin()
				&& *boost::prior(boost::prior(tagged_iter)) == "IN" && words_iter != words.begin()
				&& boost::prior(words_iter) != words.begin() && *boost::prior(boost::prior(words_iter)) == "on") {
			*tagged_iter = "NN";
		}
		++tagged_iter;
		++words_iter;
	}

	// An VBZ between a name and a verb becomes NNS
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (boost::next(tagged_iter) != tagged.end()) {
		word = *words_iter;
		tag = *tagged_iter;

		base = info->get_conj(word, "NNS");
		if (base == "" || base == "[wrong_tag]" || !info->is_candidate_name(base) || base == "i" || base == "do") {
			++tagged_iter;
			++words_iter;
			continue;
		}

		if (debug)
			cout << "SAYS::: " << word << " " << tag << " " << base << endl;

		if (*tagged_iter == "VBZ"
				&& (*boost::prior(tagged_iter) == "JJ" || *boost::prior(tagged_iter) == "NN"
						|| *boost::prior(tagged_iter) == "NNP")
				&& (*boost::next(tagged_iter) == "VB" || *boost::next(tagged_iter) == "VBP"
						|| *boost::next(tagged_iter) == "VBD" || *boost::next(tagged_iter) == "VBG")) {
			*tagged_iter = "NNS";
			if (*boost::next(tagged_iter) == "VB") {
				*boost::next(tagged_iter) = "VBP";
			}
		}
		if (*tagged_iter == "VBZ" && (aux_is_adjective(*boost::prior(tagged_iter)) || aux_is_noun(*boost::prior(tagged_iter)))
				&& (boost::next(tagged_iter)->find("-comma-") != string::npos
						|| boost::next(tagged_iter)->find("-period-") != string::npos) && words_iter != words.begin()
				&& !aux_is_someone(*boost::prior(words_iter)) && !aux_is_something(*boost::prior(words_iter))) {
			*tagged_iter = "NNS";
		}
		if (*tagged_iter == "VBZ" && words_iter != words.begin() && (*boost::prior(words_iter) == "the")) {
			*tagged_iter = "NNS";
		}
		++tagged_iter;
		++words_iter;
	}

	// A VBZ after IN becomes NNS
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (boost::next(tagged_iter) != tagged.end()) {
		word = *words_iter;
		tag = *tagged_iter;

		string base = info->get_conj(word, "NNS");
		if ( base == "" || !info->is_candidate_name(base) || base == "i" || base == "do") {
			++tagged_iter;
			++words_iter;
			continue;
		}
		if (*tagged_iter == "VBZ" && *boost::prior(tagged_iter) == "IN" && words_iter != words.begin()
				&& *boost::prior(words_iter) != "that") {
			*tagged_iter = "NNS";
		}
		++tagged_iter;
		++words_iter;
	}

	// A VBZ after a verb and before IN
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (boost::next(tagged_iter) != tagged.end()) {
		word = *words_iter;
		tag = *tagged_iter;
		string base = info->get_conj(word, "NNS");
		if (base == ""  || !info->is_candidate_name(base) || base == "i" || base == "do") {
			++tagged_iter;
			++words_iter;
			continue;
		}
		if (*tagged_iter == "VBZ" && aux_is_verb(*boost::prior(tagged_iter))
				&& (*boost::next(tagged_iter) == "IN" || *boost::next(tagged_iter) == "TO")) {
			*tagged_iter = "NNS";
		}
		++tagged_iter;
		++words_iter;
	}

	// Wearing/JJ -> VBG a mask in public is forbidden
	// It must go before adding "IN(that)"
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (boost::next(tagged_iter) != tagged.end()) {
		word = *words_iter;
		tag = *tagged_iter;

		if ((tag == "JJ" || tag == "NN") && (*boost::next(tagged_iter) == "DT" || *boost::next(tagged_iter) == "PRP$")) {

			base = info->get_conj(word, "VBG");
			if (base != "" && info->is_candidate_verb(base)) {
				*tagged_iter = "VBG";
			}
		}

		++words_iter;
		++tagged_iter;
	}

	// Adds "IN(that)" when is needed


	bool that_trigger = false, IN_trigger = false;
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	vector<string>::iterator word_pos = words.end();
	vector<string>::iterator tag_pos = tagged.end();
//say_trigger= false;
	bool do_cycle = true, aux_is_allowed = true;
	if (*boost::next(tagged.begin()) == "IN" // If I see the sun I am happy
	|| *boost::next(tagged.begin()) == "VBG" // having played together the pair is known for its effectiveness
			) {
		do_cycle = false;
		if (words.front() == "if" && find(words.begin(), words.end(), "then") != words.end()) // "if the theory is real, then I expect the particle it predicts"
			do_cycle = true;
	}
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (do_cycle && boost::next(tagged_iter) != tagged.end()) {
		word = *words_iter;
		tag = *tagged_iter;
		base = info->get_conj(word, tag);
		if (base == "")
			base = word;
		if ((tag == "DT" || tag == "PDT" || tag == "CD") && *boost::prior(tagged_iter) == "VBN") {
			if (words_iter == words.begin()) {
				++words_iter;
				++tagged_iter;
				continue;
			}
			string prior_word = *boost::prior(words_iter);
			base = info->get_conj(prior_word, "VBN");
			if (aux_verb_supports_indirect_obj(base)) {
				++words_iter;
				++tagged_iter;
				continue;
			}
			vector<string>::iterator ipos = tagged_iter;
			for (; ipos != tagged.end(); ++ipos) {
				if (aux_is_verb(*ipos)) {
					that_trigger = true;
					word_pos = words_iter;
					tag_pos = tagged_iter;
					break;
				}
				if (aux_is_conj(*ipos)) {
					break;
				}
			}
		}
		if (tag == "CD" && word == "1"
				&& (*boost::prior(tagged_iter) == "NN" || *boost::prior(tagged_iter) == "NNP"
						|| *boost::prior(tagged_iter) == "NNS" || *boost::prior(tagged_iter) == "NNPS")
				&& (*boost::next(tagged_iter) == "VBZ" || *boost::next(tagged_iter) == "VBD") && words_iter != words.begin()
				&& !aux_is_place(*boost::prior(words_iter)) && !aux_is_date(*boost::prior(words_iter))) {
			that_trigger = true;
			word_pos = words_iter;
			tag_pos = tagged_iter;
		}
		if ((tag == "DT" || tag == "PDT") && *boost::prior(tagged_iter) == "JJS") {
			that_trigger = true;
			word_pos = words_iter;
			tag_pos = tagged_iter;
		}
		if (tag == "PRP"
				&& (*boost::prior(tagged_iter) == "NN" || *boost::prior(tagged_iter) == "NNP"
						|| *boost::prior(tagged_iter) == "NNS" || *boost::prior(tagged_iter) == "NNPS"
						|| *boost::prior(tagged_iter) == "JJ") && word != "himself" && word != "herself" && word != "itself"
				&& words_iter != words.begin() && !aux_is_place(*boost::prior(words_iter))
				&& !aux_is_date(*boost::prior(words_iter))) {
			that_trigger = true;
			word_pos = words_iter;
			tag_pos = tagged_iter;
		}
		if ((tag == "DT")
				&& (*boost::prior(tagged_iter) == "NN" || *boost::prior(tagged_iter) == "NNP"
						|| *boost::prior(tagged_iter) == "NNS" || *boost::prior(tagged_iter) == "NNPS"
						|| *boost::prior(tagged_iter) == "JJ") && words_iter != words.begin()
				&& !aux_is_place(*boost::prior(words_iter)) && !aux_is_date(*boost::prior(words_iter))) {
			that_trigger = true;
			word_pos = words_iter;
			tag_pos = tagged_iter;
		}
		if ((tag == "NN" || tag == "NNP" || tag == "NNS" || tag == "NNPS")
				&& (*boost::prior(tagged_iter) == "VB" || *boost::prior(tagged_iter) == "VBP"
						|| *boost::prior(tagged_iter) == "VBZ" || *boost::prior(tagged_iter) == "VBD")) {
			string word2 = *boost::prior(words_iter);
			string tag2 = *boost::prior(tagged_iter);
			base = info->get_conj(word2, tag2);
			if (base == "")
				base = word2;
			if (!aux_is_transitive(base)) {
				that_trigger = true;
				word_pos = words_iter;
				tag_pos = tagged_iter;
			}
		}
		if (tag == "VBG" && (*boost::prior(tagged_iter) == "IN") && (*boost::next(tagged_iter) != "IN")
				&& (*boost::next(tagged_iter) != "TO") && (*boost::next(tagged_iter) != "PRP")
				&& !aux_is_verb(*boost::next(tagged_iter))) {
			that_trigger = true;
			word_pos = words_iter + 1;
			tag_pos = tagged_iter + 1;
		}
		if (is_question_ && tag == "VB"
				&& (*boost::next(tagged_iter) == "VBP" || *boost::next(tagged_iter) == "VBZ"
						|| *boost::next(tagged_iter) == "VBD")) {
			that_trigger = true;
			word_pos = words_iter + 1;
			tag_pos = tagged_iter + 1;
		}
		if (tag == "PRP" && (word == "he" || word == "she" || word == "they" || word == "i") && !is_question_
				&& (*boost::prior(tagged_iter) == "VBP" || *boost::prior(tagged_iter) == "VBZ"
						|| *boost::prior(tagged_iter) == "VBD" || *boost::prior(tagged_iter) == "VBN")) {
			that_trigger = true;
			word_pos = words_iter;
			tag_pos = tagged_iter;
		}
//		if (tag == "MD"
//				&& (*boost::prior(tagged_iter) == "VBP" || *boost::prior(tagged_iter) == "VBZ"
//						|| *boost::prior(tagged_iter) == "VBD" || *boost::prior(tagged_iter) == "VBN"
//						|| *boost::prior(tagged_iter) == "VB")) {
//			that_trigger = true;
//			word_pos = words_iter;
//			tag_pos = tagged_iter;
//		}
		if (that_trigger
				&& ((tag == "VB" && !is_question_) || tag == "VBN" || tag == "-comma-" || tag == "WDT" || tag == "WRB"
						|| tag == "VBG")) {
			that_trigger = false;
		}
		if (that_trigger && tag == "IN" && word == "that") {
			that_trigger = false;
		}
		if (aux_is_allowed && that_trigger && (tag == "VBP" || tag == "VBZ" || tag == "VBD" || tag == "MD")) {
			tagged_iter = tagged.insert(tag_pos, "IN");
			words_iter = words.insert(word_pos, "that");

			++tagged_iter;
			++words_iter;

			that_trigger = false;
		}
		if (!aux_is_allowed && that_trigger && (tag == "VBP" || tag == "VBZ" || tag == "VBD" || tag == "MD")) {
			aux_is_allowed = true;
		}

		if ((tag == "IN" && word == "if")
				|| (tag == "-comma-" && (*boost::next(tagged_iter) == "VBG" || *boost::next(tagged_iter) == "IN"))
				|| (tag == "-comma-" && (*boost::next(tagged_iter) == "CC"))) {
			aux_is_allowed = false;
			that_trigger = false;
		}

		++tagged_iter;
		++words_iter;
	}

	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (boost::next(tagged_iter) != tagged.end()) {
		word = *words_iter;
		tag = *tagged_iter;
		if (word == "that" && (tag == "IN" || tag == "WDT") && words_iter != words.end() && *boost::next(words_iter) == "that"
				&& *boost::next(tagged_iter) == "DT") {
			tagged_iter = tagged.erase(boost::next(tagged_iter));
			words_iter = words.erase(boost::next(words_iter));
		}
		++tagged_iter;
		++words_iter;
	}


	that_trigger = false, IN_trigger = false;
	bool write_trigger = false;
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	word_pos = words.end();
	tag_pos = tagged.end();
//say_trigger= false;
	while (words_iter != words.end() && boost::next(words_iter) != words.end()) {
		word = *words_iter;
		tag = *tagged_iter;
		base = info->get_conj(word, tag);
		if (base == "")
			base = word;
		if ((base == "say" || base == "write" || base == "think" || base == "watch" || base == "mean" || base == "suggest"
				|| base == "believe" || base == "doubt" || base == "study"
				|| base == "forget" || base == "suspect" || base == "know"
				|| base == "remind" || base == "rule"
		//|| d->get_levin_verb(base) == "verb.communication"
		)
//				&& base != "name"
//				&& base != "consider"
//				&& base != "call"
//				&& base != "blame"
				&& *boost::next(tagged_iter) != "CC" && *boost::next(tagged_iter) != "TO") { /// communications verbs
																			  //say_trigger= true;
			that_trigger = true;
			if (base == "write")
				write_trigger = true;
			word_pos = words_iter + 1;
			tag_pos = tagged_iter + 1;
			++tagged_iter;
			++words_iter;
			continue;
		}

		if (that_trigger && tag == "IN" && word == "that") {
			that_trigger = false;
		}
		if (that_trigger && tag == "WDT" && word == "that") {
			that_trigger = false;
		}
		if (that_trigger && tag == "WDT" && word == "which") {
			that_trigger = false;
		}

		string next_tag = *boost::next(tagged_iter);
		string next_word = *boost::next(words_iter);
		if (that_trigger && (tag == "VBP" || tag == "VBZ" || tag == "VBD" || tag == "VB" || tag == "MD")
				&& next_word.find("[verbatim]") == string::npos && !aux_is_something(next_word)
				&& ((next_tag == "PRP"
						&& (next_word == "he" || next_word == "she" || next_word == "they" || next_word == "i"
								|| next_word == "it"))
						|| (next_tag == "NN" || next_tag == "NNP" || next_tag == "CD"
								|| (next_tag == "IN" && next_word != "that"))
				//|| next_tag == "DT" // write the letter
				)) {

			tagged_iter = tagged.insert(tag_pos, "IN");
			words_iter = words.insert(word_pos, "that");

			++tagged_iter;
			++words_iter;

			that_trigger = false;
			write_trigger = false;
		}
		if (that_trigger && (tag == "VBP" || tag == "VBZ" || tag == "VBD" || tag == "MD" || tag == "VB") && !write_trigger // write the letter
				&& (next_tag == "DT" || aux_is_plural(next_tag))) {
			// check that there is a verb afterwards
			vector<string>::iterator tagged_iter2 = tagged_iter;
			for (; boost::next(tagged_iter2) != tagged.end(); ++tagged_iter2) {
				if (*tagged_iter2 == "VB" || *tagged_iter2 == "MD" || *tagged_iter2 == "VBP" || *tagged_iter2 == "VBZ"
						|| *tagged_iter2 == "VBD" || *tagged_iter2 == "VBN") {
					tagged_iter = tagged.insert(tag_pos, "IN");
					words_iter = words.insert(word_pos, "that");
					break;
				}
				if (*tagged_iter2 == "-comma-" || *tagged_iter2 == "MD" || *tagged_iter2 == "VBP" || *tagged_iter2 == "VBZ"
						|| *tagged_iter2 == "VBD" || *tagged_iter2 == "VBN")
					break;
			}

			that_trigger = false;
			write_trigger = false;
		}
		if (that_trigger && (tag == "-comma-" || tag == "CC")) {
			that_trigger = false;
			write_trigger = false;
		}

		++tagged_iter;
		++words_iter;
	}

// Adds "TO(to)" when is needed
//	to_trigger = false;
//	words_iter = words.begin();
//	tagged_iter = tagged.begin() + 1;
//	word_pos = words.end();
//	tag_pos = tagged.end();
//	while (*boost::next(tagged.begin()) != "IN"
//	&& boost::next(tagged_iter) != tagged.end()) {
//		word = *words_iter;
//		tag = *tagged_iter;
//
//		base = info->get_conj(word, tag);
//		if (base == "")
//			base = word;
//		if ( (base == "help" || base == "make" ) ///
//			 && (tag == "VB" || tag == "VBP" || tag == "VBZ" || tag == "VBD")
//		) {
//			to_trigger = true;
//			word_pos = words_iter + 1;
//			tag_pos = tagged_iter + 1;
//		}
//
//		if (to_trigger && (tag == "NN" && info->is_candidate_verb(word))) {
//			*tagged_iter = "VB";
//
//			tagged_iter = tagged.insert(tag_pos, "TO");
//			words_iter = words.insert(word_pos, "to");
//
//			++tagged_iter;
//			++words_iter;
//
//			to_trigger = false;
//		}
//		++tagged_iter;
//		++words_iter;
//	}

	// make the land around the barn blend/NN and unify -> make the land around the barn to blend/VB and unify
	to_trigger = false;
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	word_pos = words.end();
	tag_pos = tagged.end();
	while (boost::next(tagged_iter) != tagged.end()) {
		word = *words_iter;
		tag = *tagged_iter;

		base = info->get_conj(word, tag);
		if (base == "")
			base = word;
		if ( (base == "help" || base == "make")
				&& (tag == "VB" || tag == "VBP" || tag == "VBZ" || tag == "VBD")
		) {
			to_trigger = true;
		}

//		if (to_trigger && !aux_is_article(tag) && !aux_is_adjective(tag) && !aux_is_noun(tag) ) {
//			to_trigger = false; /// NO!, "around/IN the barn" contains IN
//		}

		if (to_trigger && tag == "NN" && info->is_candidate_verb(word)
			&& aux_is_noun(*boost::prior(tagged_iter))
			&& aux_is_conj(*boost::next(tagged_iter))
			&& boost::next(tagged_iter) != tagged.end()
			&& boost::next(tagged_iter) != tagged.end()
			&& boost::next(boost::next(tagged_iter)) != tagged.end()
			&& aux_is_verb(*boost::next(boost::next(tagged_iter)))
		) {
			*tagged_iter = "VB";

			tagged_iter = tagged.insert(tagged_iter, "TO");
			words_iter = words.insert(words_iter, "to");

			++tagged_iter;
			++words_iter;

			to_trigger = false;
		}
		++tagged_iter;
		++words_iter;
	}


	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (boost::next(tagged_iter) != tagged.end()) {
		word = *words_iter;
		tag = *tagged_iter;

		if (words_iter != words.begin() && (*boost::prior(words_iter) == "by" || *boost::prior(words_iter) == "for")) {
			++tagged_iter;
			++words_iter;
			continue; // a man falls by losing height
		}
		if (words_iter != words.begin()) {
			base = info->get_conj(*boost::prior(words_iter), tag);
			if (base == "")
				base = *boost::prior(words_iter);
		}

		if ((tag == "VBG" && (*boost::prior(tagged_iter) == "DT" || *boost::prior(tagged_iter) == "CC")
				&& (*boost::next(tagged_iter) == "NN" || *boost::next(tagged_iter) == "NNP"
						|| *boost::next(tagged_iter) == "JJ"))
				|| (tag == "VBG" && *boost::prior(tagged_iter) == "RBR" && boost::prior(tagged_iter) != tagged.begin()
						&& *boost::prior(boost::prior(tagged_iter)) == "DT"
						&& (*boost::next(tagged_iter) == "NN" || *boost::next(tagged_iter) == "NNP"
								|| *boost::next(tagged_iter) == "JJ"))
				|| (tag == "VBG" && *boost::prior(tagged_iter) == "PRP$"
						&& (*boost::next(tagged_iter) == "NN" || *boost::next(tagged_iter) == "NNP"
								|| *boost::next(tagged_iter) == "JJ"))
				|| (tag == "VBG" && *boost::prior(tagged_iter) == "DT" && words_iter != words.begin()
						&& (*boost::prior(words_iter) == "the" || *boost::prior(words_iter) == "a"
								|| *boost::prior(words_iter) == "an")
						&& (aux_is_noun(*boost::next(tagged_iter)) || aux_is_adjective(*boost::next(tagged_iter))))
				|| (tag == "VBG" && *boost::prior(tagged_iter) == "IN" && words_iter != words.begin()
						&& *boost::prior(words_iter) != "like"
						&& (*boost::next(tagged_iter) == "NN" || *boost::next(tagged_iter) == "NNP"
								|| *boost::next(tagged_iter) == "NNS" || *boost::next(tagged_iter) == "NNPS"
								|| *boost::next(tagged_iter) == "JJ"))
				|| (tag == "VBG" && aux_is_adjective(*boost::prior(tagged_iter)) && (aux_is_noun(*boost::next(tagged_iter))))
				|| (tag == "VBG" && aux_is_noun(*boost::prior(tagged_iter)) && (aux_is_noun(*boost::next(tagged_iter))))
				|| (tag == "VBG" && *boost::prior(tagged_iter) == "RBS" && (aux_is_noun(*boost::next(tagged_iter))))
				|| (tag == "VBG" && *boost::prior(tagged_iter) == "CC" && boost::prior(tagged_iter) != tagged.begin()
						&& *boost::prior(boost::prior(tagged_iter)) == "JJ"
						&& (*boost::next(tagged_iter) == "-period-" || *boost::next(tagged_iter) == "-comma-"))
				|| (tag == "VBG" && aux_is_verb(*boost::prior(tagged_iter)) && base == "have"
						&& aux_is_noun(*boost::next(tagged_iter)))) {
			//*tagged_iter = "VBJ";
			if (d->is_adjective(word)) {
				*tagged_iter = "JJ";
			}
		}
		if (tag == "VBG" && (*boost::prior(tagged_iter) == "VBN" || *boost::prior(tagged_iter) == "VBD")
				&& !info->is_auxiliary_verb(base) && info->is_candidate_name(word)) {
			*tagged_iter = "NN";
		}
		++tagged_iter;
		++words_iter;
	}

// sometimes VBG can be NN (when in a list of names)
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (boost::next(tagged_iter) != tagged.end()) {
		word = *words_iter;
		tag = *tagged_iter;

		if (boost::prior(tagged_iter) != tagged.begin() && tag == "VBG" && aux_is_conj(*boost::prior(tagged_iter))
				&& aux_is_noun(*boost::prior(boost::prior(tagged_iter))) && aux_is_conj(*boost::next(tagged_iter)))
			*tagged_iter = "NN";

		++tagged_iter;
		++words_iter;
	}

// about 2000 people were there.
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (boost::next(tagged_iter) != tagged.end()) {
		word = *words_iter;
		tag = *tagged_iter;

		if (tag == "IN" && *boost::next(tagged_iter) == "CD" && word == "about"
//               && (*boost::prior(tagged_iter) == "IN"
//                   || *boost::prior(tagged_iter) == ""
//                  )
				) {
			*tagged_iter = "RB";
			*words_iter = "circa";
		}

		if (tag == "IN" && *boost::next(tagged_iter) == "CD" && word == "around"
				&& (*boost::prior(tagged_iter) == "CC" || *boost::prior(tagged_iter) == "")) {
			*tagged_iter = "RB";
			*words_iter = "circa";
		}
		++tagged_iter;
		++words_iter;
	}

// An NN after WP is VBP
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (boost::next(tagged_iter) != tagged.end()) {
		word = *words_iter;
		tag = *tagged_iter;

		if (!info->is_candidate_verb(word)) {
			++tagged_iter;
			++words_iter;
			continue;
		}

		if (tag == "NN" && (*boost::prior(tagged_iter) == "WP") && (*boost::next(tagged_iter) != "VBP")
				&& (*boost::next(tagged_iter) != "VBZ") && (*boost::next(tagged_iter) != "VBD")
				&& (*boost::next(tagged_iter) != "MD")) {
			*tagged_iter = "VBP";
		}
		++tagged_iter;
		++words_iter;
	}

// "left" can be a name if before a DT
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (boost::next(tagged_iter) != tagged.end()) {
		word = *words_iter;
		tag = *tagged_iter;

		if (word == "left" && (tag == "VBD" || tag == "VBN")
				&& (*boost::next(tagged_iter) == "DT" || *boost::next(tagged_iter) == "MD"
						|| *boost::next(tagged_iter) == "VBP" || *boost::next(tagged_iter) == "VBZ"
						|| *boost::next(tagged_iter) == "VBD") && *boost::prior(tagged_iter) != "PRP") {
			*tagged_iter = "NN";
		}
		++tagged_iter;
		++words_iter;
	}

// Corrects "VB" after MD
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (is_question_ && boost::next(tagged_iter) != tagged.end()) {
		word = *words_iter;
		tag = *tagged_iter;
		if (tag == "VB" && info->is_candidate_name(word) && !info->is_candidate_verb(word)
				&& *boost::prior(tagged_iter) == "MD") {
			vector<string>::iterator tag_iter2 = tagged_iter;
			for (; tag_iter2 != tagged.end(); ++tag_iter2) {
				if (*tag_iter2 == "VB") {
					*tagged_iter = "NN";
					break;
				}
				if (*tag_iter2 != "RB") {
					break;
				}
			}
		}
		++tagged_iter;
		++words_iter;
	}

// IN at the end of the sentence becomes RP
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (boost::next(tagged_iter) != tagged.end()) {
		word = *words_iter;
		tag = *tagged_iter;

		// last verb must be a proper aux
		if(aux_is_verb(tag) ) {
			base = info->get_conj(word, tag);
		}

		if ( (tag == "IN" || tag == "TO")
			&& (base != "be" && base != "have" )
				&& (*boost::next(tagged_iter) == "-period-" || *boost::next(tagged_iter) == "-comma-"
						|| *boost::next(tagged_iter) == "CC" || *boost::next(tagged_iter) == "IN"
						|| *boost::next(tagged_iter) == "TO")) {
			if (word == "in" || word == "for" || word == "at" || word == "into" || word == "with" || word == "by"
					|| word == "about" || word == "out" || word == "up" || word == "off" || word == "through"
					|| word == "over" || word == "to") {
				*tagged_iter = "RP";
			} else if (word == "before" || word == "so" || word == "in_and_out") {
				*tagged_iter = "RB";
			}
		}
		++tagged_iter;
		++words_iter;
	}
// RP with no verb before becomes IN
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	bool verb_trigger = false;
	IN_trigger = false;
	while (boost::next(tagged_iter) != tagged.end()) {
		word = *words_iter;
		tag = *tagged_iter;

		if (tag == "VBP" || tag == "VBZ" || tag == "VBD" || tag == "VBN" || tag == "VB")
			verb_trigger = true;

		if (tag == "RP" && !verb_trigger && !IN_trigger) {
			*tagged_iter = "IN";
			verb_trigger = false;
			IN_trigger = false;
		}

		if (tag == "IN")
			IN_trigger = true;

		++tagged_iter;
		++words_iter;
	}

// some RP with a name after become IN
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (boost::next(tagged_iter) != tagged.end()) {
		word = *words_iter;
		tag = *tagged_iter;

		if (tag == "RP" && (word == "on" || word == "by" || word == "with")
				&& (*boost::next(tagged_iter) == "DT" || *boost::next(tagged_iter) == "NN"
						|| *boost::next(tagged_iter) == "NNS" || *boost::next(tagged_iter) == "NNP"
						|| *boost::next(tagged_iter) == "NNPS"
						|| aux_is_adjective(*boost::next(tagged_iter))
				)
		) {
			*tagged_iter = "IN";
		}
		++tagged_iter;
		++words_iter;
	}

// some RP after a CC becomes a RB
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (boost::next(tagged_iter) != tagged.end()) {
		word = *words_iter;
		tag = *tagged_iter;

		if (tag == "RP" && (*boost::prior(tagged_iter) == "CC")) {
			*tagged_iter = "CC";
		}
		++tagged_iter;
		++words_iter;
	}

// "named" is JJ if followed by NNP
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (boost::next(tagged_iter) != tagged.end()) {
		word = *words_iter;
		tag = *tagged_iter;
		if (!d->is_adjective(word)) {
			++tagged_iter;
			++words_iter;
			continue;
		}

		if ((word == "named" || word == "entitled") /// Use all communication verbs!!!
		&& (tag == "VBD") && (*boost::next(tagged_iter) == "NNP")) {
			*tagged_iter = "JJ";
		}
		++tagged_iter;
		++words_iter;
	}

	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (boost::next(tagged_iter) != tagged.end()) {
		word = *words_iter;
		tag = *tagged_iter;
		base = info->get_conj(word, tag);
		if (base == "")
			base = word;
		if (boost::next(words_iter) == words.end()) {
			++tagged_iter;
			++words_iter;
			continue;
		}
		if ((base == "say" || base == "speak" || base == "dream" || base == "write" || base == "suggest" || base == "believe"
				|| base == "forget" || base == "suspect" || base == "know" || base == "study"
				|| base == "remind" || base == "rule"
				|| d->get_levin_verb(base) == "verb.communication")
		/// Use all communication verbs!!!
				&& (*boost::next(words_iter) == "about")) {
			*boost::next(tagged_iter) = "IN";
		}
		++tagged_iter;
		++words_iter;
	}

// A NN of a material (paper, plastic) that is also a JJ is changed into JJ
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (boost::next(tagged_iter) != tagged.end()) {
		word = *words_iter;
		tag = *tagged_iter;
		if (!d->is_adjective(word)) {
			++tagged_iter;
			++words_iter;
			continue;
		}

		//cout << "WD:::" << word << ", " << d->pertains_to_name(word,"material",8) << endl;
		if ((tag == "NN" || tag == "NNP") && word != "material"
				&& (d->pertains_to_name(word, "material", 8) > 0.5 || d->hypernym_dist(word, "material", 8) > 0.5
						|| d->hypernym_dist(word, "metal", 8) > 0.5)
				&& (*boost::next(tagged_iter) == "NN" || *boost::next(tagged_iter) == "NNS"
						|| *boost::next(tagged_iter) == "NNP" || *boost::next(tagged_iter) == "NNPS")) {
			*tagged_iter = "JJ";
		}
		++tagged_iter;
		++words_iter;
	}

//words_iter= words.begin();
	tagged_iter = tagged.begin() + 1;
	while (boost::next(tagged_iter) != tagged.end()) {
		int sizet = std::distance(tagged.begin(), tagged_iter);
		if (tagged.size() >= 5 && tagged.at(sizet) == "-period-" && tagged.at(sizet - 1) == "VBZ"
				&& (tagged.at(sizet - 2) == "NN" || tagged.at(sizet - 2) == "JJ" || tagged.at(sizet - 2) == "JJR"
						|| tagged.at(sizet - 2) == "JJS") && tagged.at(sizet - 3) == "PRP$") {
			tagged.at(sizet - 1) = "NNS";
		}
		++tagged_iter;
	}

	tagged_iter = tagged.begin() + 1;
	while (boost::next(tagged_iter) != tagged.end()) {
		int sizet = std::distance(tagged.begin(), tagged_iter);
		if (tagged.size() >= 5 && tagged.at(sizet) == "-period-" && tagged.at(sizet - 1) == "IN" && tagged.at(sizet - 2) == "NN"
				&& tagged.at(sizet - 3) == "TO") {
			tagged.at(sizet - 2) = "VB";
		}
		++tagged_iter;
	}

// add a comma when it is forgotten by the journalist
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (boost::next(tagged_iter) != tagged.end()) {
		word = *words_iter;
		tag = *tagged_iter;

		if (words_iter == words.begin() && tag == "RB" && *boost::next(tagged_iter) == "IN"
				&& boost::next(boost::next(tagged_iter)) != tagged.end()
				&& (*boost::next(boost::next(tagged_iter)) == "WP" || *boost::next(boost::next(tagged_iter)) == "WDT")) {
			tagged_iter = tagged.insert(tagged_iter + 1, "-comma-");
			words_iter = words.insert(words_iter + 1, "-comma-");
		}
		++words_iter;
		++tagged_iter;
	}

// sometimes prepositions like "in an beyond" are used. For simplicity, the preposition becomes "[place_prep]_in-and-beyond"
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (boost::next(tagged_iter) != tagged.end()) {
		word = *words_iter;
		tag = *tagged_iter;

		if (words_iter != words.begin() && tag == "CC" && *boost::next(tagged_iter) == "IN"
				&& *boost::prior(tagged_iter) == "IN") {
			string head_prior = *boost::prior(words_iter);
			string head_next = *boost::next(words_iter);
			string head = string("[place_prep]_") + head_prior + "-" + word + "-" + head_next;
			*words_iter = head;
			*tagged_iter = "IN";
			words.erase(boost::next(words_iter));
			tagged.erase(boost::next(tagged_iter));
			words.erase(boost::prior(words_iter));
			tagged.erase(boost::prior(tagged_iter));
			words_iter = words.begin();
			tagged_iter = tagged.begin() + 1;
		}
		++words_iter;
		++tagged_iter;
	}

// "what/WDT" in questions as a first word becomes "what/WP"
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	bool transform = true;
	while (boost::next(tagged_iter) != tagged.end()) {
		word = *words_iter;
		tag = *tagged_iter;

		if (transform && tag == "WDT" && word == "what" && is_question_) {
			*tagged_iter = "WP";
		}
		++words_iter;
		++tagged_iter;
	}

// Adds EX(there) between "V(be)" and "IN(*)", provided there are only N, PRP$, J in-between
// "in the pool is the dog" -> "in the pool there is the dog"
	bool ex_trigger = false, start_trigger = true;
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	word_pos = words.end();
	tag_pos = tagged.end();
//say_trigger= false;
	while (words_iter != words.end() && boost::next(words_iter) != words.end()) {
		word = *words_iter;
		tag = *tagged_iter;

		base = info->get_conj(word, tag);

		if (ex_trigger && base == "be") {
			// check this is not a complement of a previous verb
			vector<string>::iterator tagged_iter2 = tagged_iter;
			--tagged_iter2;
			for (; boost::prior(tagged_iter2) != tagged.begin(); --tagged_iter2) {
				string tag2 = *tagged_iter2;
				if (aux_is_verb(tag2)) {
					ex_trigger = false;
					break;
				}
			}
			// check this is not a subordinate
			tagged_iter2 = tagged_iter;
			for (; tagged_iter2 != tagged.end(); ++tagged_iter2) {
				string tag2 = *tagged_iter2;
				if (tag2 == "VBG" || tag2 == "VBN") {
					ex_trigger = false;
					break;
				}
				if (tag2 == "DT" || tag2 == "NN" || tag2 == "NNP" || tag2 == "NNS" || tag2 == "NNPS") {
					break;
				}
			}
			if (ex_trigger) {
				tagged_iter = tagged.insert(tagged_iter, "EX");
				words_iter = words.insert(words_iter, "there");
				ex_trigger = false;
			}
		}

		if (ex_trigger && tag != "DT" && tag != "PRP$" && tag != "NN" && tag != "NNP" && tag != "NNS" && tag != "NNPS"
				&& !(tag == "PRP" && word == "them") && !(tag == "PRP" && word == "her") && !(tag == "PRP" && word == "him"))
			ex_trigger = false;
		if (start_trigger && tag == "IN" && word != "if" && word != "that" && word != "because"
				&& *boost::prior(tagged_iter) != "NN" && *boost::prior(tagged_iter) != "NNP"
				&& *boost::prior(tagged_iter) != "NNS" && *boost::prior(tagged_iter) != "NNPS"
				&& *boost::prior(tagged_iter) != "JJ" && *boost::prior(tagged_iter) != "JJR"
				&& *boost::prior(tagged_iter) != "JJS" && *boost::prior(tagged_iter) != "CD"
				&& *boost::prior(tagged_iter) != "DT") {
			ex_trigger = true;
		}

		if (tag == "VBP" || tag == "VBD" || tag == "VB") {
			start_trigger = false;
			ex_trigger = false;
		}

		++tagged_iter;
		++words_iter;
	}

// Corrects VBN that should be "VBD" if followed by a "IN(by)"
/// NO! it can be "sold by the 1990s"
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	bool aux_is_preceded_by_verb = false;
	while (boost::next(tagged_iter) != tagged.end()) {
		word = *words_iter;
		tag = *tagged_iter;

		string auxbase1 = info->get_conj(word, "VBP");
		string auxbase2 = info->get_conj(word, "VBZ");
		string auxbase3 = info->get_conj(word, "VBD");

		base = info->get_conj(word, "VBN");
		if (base != "" && tag == "VBD" && *boost::next(tagged_iter) == "IN" && *boost::next(words_iter) == "by")
			*tagged_iter = "VBN";
		if (word == "run" && *boost::next(tagged_iter) == "IN" && *boost::next(words_iter) == "by")
			*tagged_iter = "VBN";
		if (word == "built" && *boost::next(tagged_iter) == "IN")
			*tagged_iter = "VBN";
		if (base != "" && tag == "VBD" && aux_is_preceded_by_verb && *boost::next(tagged_iter) != "TO") {
			*tagged_iter = "VBN";
		}
		if (aux_is_preceded_by_verb && !aux_is_noun(tag) && !aux_is_date(word) && !aux_is_article(tag) && !aux_is_adjective(tag) && !aux_is_adverb(tag)
				&& !aux_is_preposition(tag)) {
			aux_is_preceded_by_verb = false;
		}
		if (aux_is_preceded_by_verb && tag == "IN" && word == "that") {
			aux_is_preceded_by_verb = false;
		}
		if (aux_is_preceded_by_verb && tag == "WDT" && word == "that") {
			aux_is_preceded_by_verb = false;
		}
		if (!aux_is_preceded_by_verb && aux_is_verb(tag) && (auxbase1 != "" || auxbase2 != "" || auxbase3 != "")) {
			aux_is_preceded_by_verb = true; // carried out negotiations aimed/VBD->VBN at ...
		}

		++tagged_iter;
		++words_iter;
	}

// process passive with implicit that/WDT be/VBP (a referendum led by ...)


	that_trigger = false;
	IN_trigger = false;
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	word_pos = words.end();
	tag_pos = tagged.end();
	while (!is_question_ && boost::next(tagged_iter) != tagged.end()) {
		word = *words_iter;
		tag = *tagged_iter;

		string prior_word;
		if (words_iter != words.begin())
			prior_word = *boost::prior(words_iter);

		if (debug)
			cout << "TAG33:: " << word << ", " << tag << ", " << that_trigger << endl;

		if (tag == "VBN" && (aux_is_noun(*boost::prior(tagged_iter)) || aux_is_adjective(*boost::prior(tagged_iter)))
				&& words_iter != words.begin() && !aux_is_date(*boost::prior(words_iter))) {
			that_trigger = true;
			word_pos = words_iter;
			tag_pos = tagged_iter;
		}
		if (word == "put" // a referendum put before the voters achieved a success...
		&& tag != "VBN" && aux_is_noun(*boost::prior(tagged_iter)) && *boost::next(tagged_iter) == "IN") {
			that_trigger = true;
			word_pos = words_iter;
			tag_pos = tagged_iter;
			*tagged_iter = "VBN";
		}
		if (word == "first" // a referendum put before the voters achieved a success...
		&& (tag == "RB" || tag == "JJ") && aux_is_noun(*boost::prior(tagged_iter)) && *boost::next(tagged_iter) == "VBD") {
			that_trigger = true;
			word_pos = words_iter;
			tag_pos = tagged_iter;
			*tagged_iter = "RB";
			*boost::next(tagged_iter) = "VBN";
		}
		if (that_trigger
				&& (tag == "IN" || aux_is_conj(tag) || word == "put" || prior_word == "led" || prior_word == "called"
						|| word == "first") //&& word == "by"
				) {
			tagged_iter = tagged.insert(tag_pos, "WDT");
			words_iter = words.insert(word_pos, "that");
			tagged_iter = tagged.insert(tagged_iter + 1, "VBP");
			words_iter = words.insert(words_iter + 1, "be");

			++tagged_iter;
			++words_iter;

			that_trigger = false;
		}
		if ( (that_trigger && (aux_is_noun(tag) || aux_is_adjective(tag) || aux_is_article(tag) || aux_is_conj(tag)) ) || aux_is_date(word)) {
			that_trigger = false;
		}
		++tagged_iter;
		++words_iter;
	}

// "Eagles that fly swim". "Fly" must be a verb
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (boost::next(tagged_iter) != tagged.end()) {
		word = *words_iter;
		tag = *tagged_iter;
		if (aux_is_noun(tag) && info->is_candidate_verb(word) && words_iter != words.begin()
				&& (*boost::prior(words_iter) == "that"
						&& (*boost::prior(tagged_iter) == "IN" || *boost::prior(tagged_iter) == "WDT"))
				&& aux_is_verb(*boost::next(tagged_iter)) && *boost::next(tagged_iter) != "VBN") {
			*tagged_iter = "VBP";
			if (*boost::next(tagged_iter) == "VB")
				*boost::next(tagged_iter) = "VBP";
		}
		++tagged_iter;
		++words_iter;
	}

// "... that cause desease". "cause" is VBP
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (boost::next(tagged_iter) != tagged.end()) {
		word = *words_iter;
		tag = *tagged_iter;
		if (aux_is_noun(tag) && info->is_candidate_verb(word) && words_iter != words.begin()
				&& (*boost::prior(words_iter) == "that"
						&& (*boost::prior(tagged_iter) == "IN" || *boost::prior(tagged_iter) == "WDT"))
				&& aux_is_noun(*boost::next(tagged_iter)))
			*tagged_iter = "VBP";
		++tagged_iter;
		++words_iter;
	}

// VBG after a cardinal (12th meeting) is a NN
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (boost::next(tagged_iter) != tagged.end()) {
		word = *words_iter;
		tag = *tagged_iter;
		base = info->get_conj(word, "VBG");
		if (base != "" && tag == "VBG" && *boost::prior(tagged_iter) == "CD" && aux_is_cardinal_word(*boost::prior(words_iter)))
			*tagged_iter = "NN";
		++tagged_iter;
		++words_iter;
	}

// RBR before IN(than) becomes JJR
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (boost::next(tagged_iter) != tagged.end()) {
		word = *words_iter;
		tag = *tagged_iter;
		if (tag == "RBR" && *boost::next(tagged_iter) == "IN" && *boost::next(words_iter) == "than")
			*tagged_iter = "JJR";
		++tagged_iter;
		++words_iter;
	}

// RBR(more) before NN becomes JJR
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (boost::next(tagged_iter) != tagged.end()) {
		word = *words_iter;
		tag = *tagged_iter;
		string adj = info->get_base_superlative(word);
		if (tag == "RBR" && ((word == "more" || word == "less") || d->is_adjective(adj)) && aux_is_noun(*boost::next(tagged_iter)))
			*tagged_iter = "JJR";
		++tagged_iter;
		++words_iter;
	}

// RBS(more) before NN or JJ becomes JJS
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (boost::next(tagged_iter) != tagged.end()) {
		word = *words_iter;
		tag = *tagged_iter;
		string adj = info->get_base_superlative(word);
		if (tag == "RBS" && (aux_is_noun(*boost::next(tagged_iter)) || aux_is_adjective(*boost::next(tagged_iter))))
			*tagged_iter = "JJS";
		++tagged_iter;
		++words_iter;
	}

// 6/CD times/NNS -> 6-times/RB
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (boost::next(tagged_iter) != tagged.end()) {
		word = *words_iter;
		tag = *tagged_iter;
		if (tag == "CD" && *boost::next(tagged_iter) == "NNS" && *boost::next(words_iter) == "times") {
			string new_str = word + "-" + "times";
			*tagged_iter = "RB";
			*words_iter = new_str;
			words_iter = words.erase(boost::next(words_iter));
			tagged_iter = tagged.erase(boost::next(tagged_iter));
			continue;
		}
		++tagged_iter;
		++words_iter;
	}

// using -> with
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (boost::next(tagged_iter) != tagged.end()) {
		word = *words_iter;
		tag = *tagged_iter;
		if (tag == "VBG" && word == "using" && (aux_is_article(*boost::next(tagged_iter)) || aux_is_noun(*boost::next(tagged_iter)))
				&& (aux_is_article(*boost::prior(tagged_iter)) || aux_is_noun(*boost::prior(tagged_iter)))) {
			*tagged_iter = "IN";
			*words_iter = "with";
			continue;
		}
		++tagged_iter;
		++words_iter;
	}
// follwing -> after
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (boost::next(tagged_iter) != tagged.end()) {
		word = *words_iter;
		tag = *tagged_iter;
		if (tag == "VBG" && word == "following"
				&& (aux_is_article(*boost::next(tagged_iter)) || aux_is_noun(*boost::next(tagged_iter))
						|| *boost::next(tagged_iter) == "PRP$")
				&& (words_iter == words.begin() || aux_is_article(*boost::prior(tagged_iter))
						|| aux_is_noun(*boost::prior(tagged_iter)))) {
			*tagged_iter = "IN";
			*words_iter = "after";
			continue;
		}
		++tagged_iter;
		++words_iter;
	}

// off/RP Egypt/NNP -> off/IN Egypt/NNP
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (boost::next(tagged_iter) != tagged.end()) {
		word = *words_iter;
		tag = *tagged_iter;
		if (tag == "RP" && word == "off" && (aux_is_article(*boost::next(tagged_iter)) || aux_is_noun(*boost::next(tagged_iter)))
				&& (aux_is_article(*boost::prior(tagged_iter)) || aux_is_noun(*boost::prior(tagged_iter)))) {
			*tagged_iter = "IN";
			continue;
		}
		++tagged_iter;
		++words_iter;
	}

// to where -> to what
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (boost::next(tagged_iter) != tagged.end()) {
		word = *words_iter;
		tag = *tagged_iter;
		if (boost::next(words_iter) == words.end()) {
			++tagged_iter;
			++words_iter;
			continue;
		}

		string next_word = *boost::next(words_iter);
		string next_tag = *boost::next(tagged_iter);

		if (((tag == "TO" && word == "to") || (tag == "IN" && word == "from")) && next_word == "where" && next_tag == "WRB") {
			*boost::next(tagged_iter) = "WP";
			*boost::next(words_iter) = "what";

			++tagged_iter;
			++words_iter;
		}
		++tagged_iter;
		++words_iter;
	}

// A VBP after "the" can become NN
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (boost::next(tagged_iter) != tagged.end()) {
		word = *words_iter;
		tag = *tagged_iter;
		if ((*tagged_iter == "VBP" || *tagged_iter == "VB") && words_iter != words.begin() && *boost::prior(tagged_iter) == "DT"
				&& (*boost::prior(words_iter) == "the" || *boost::prior(words_iter) == "a")) {
			*tagged_iter = "NN";
		}

		++tagged_iter;
		++words_iter;
	}

// A title ("phd", ...) after a NNP is added to the NNP
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (boost::next(tagged_iter) != tagged.end()) {
		word = *words_iter;
		tag = *tagged_iter;
		if (tag == "-comma-" && *boost::prior(tagged_iter) == "NNP" && boost::next(words_iter) != words.end()
				&& d->is_title(*boost::next(words_iter))) {
			string next_word = *boost::next(words_iter);
			*boost::prior(words_iter) += "_" + next_word;
			words_iter = words.erase(words_iter);
			tagged_iter = tagged.erase(tagged_iter);
			words_iter = words.erase(words_iter);
			tagged_iter = tagged.erase(tagged_iter);
			continue;
		}

		++tagged_iter;
		++words_iter;
	}

// newly/RB formed/VBD force/NN -> newly/RB formed/JJ force/NN
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (boost::next(tagged_iter) != tagged.end()) {
		word = *words_iter;
		tag = *tagged_iter;

		if (!d->is_adjective(word)) {
			++tagged_iter;
			++words_iter;
			continue;
		}

		if (tag == "VBD" && *boost::prior(tagged_iter) == "RB" && words_iter != words.begin()
				&& *boost::prior(words_iter) != "then" && aux_is_noun(*boost::next(tagged_iter))) {
			vector<string>::iterator tagged_iter2 = tagged_iter;
			for (; tagged_iter2 != tagged.begin(); --tagged_iter2) {
				string tag2 = *tagged_iter2;
				if (tag2 == "PRP") // "it directly contradicted"
					break;
				if (tag2 == "DT" || tag2 == "CC") {
					*tagged_iter = "JJ";
					break;
				}
			}
		}

		++tagged_iter;
		++words_iter;
	}

// David did the homework [by] focusing on the subject
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (!is_question_ && boost::next(tagged_iter) != tagged.end()) {
		word = *words_iter;
		tag = *tagged_iter;
		if (tag == "VBG" && aux_is_noun(*boost::prior(tagged_iter)) && *boost::next(tagged_iter) == "IN") {
			tagged_iter = tagged.insert(tagged_iter, "IN");
			words_iter = words.insert(words_iter, "by");
			++tagged_iter;
			++words_iter;
		}
		++tagged_iter;
		++words_iter;
	}

	// snakes taste the air by flicking/NN->VBG their tongue
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (!is_question_ && boost::next(tagged_iter) != tagged.end()) {
		word = *words_iter;
		tag = *tagged_iter;
		string base = info->get_conj(word, "VBG");

		if (tag == "NN"
			&& *boost::prior(tagged_iter) == "IN"
			&& words_iter != words.begin()
			&& *boost::prior(words_iter) == "by"
			&& base != ""
		) {
			*tagged_iter = "VBG";
		}
		++tagged_iter;
		++words_iter;
	}


// A NN after CC after VB becomes VB
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (boost::next(tagged_iter) != tagged.end()) {
		word = *words_iter;
		tag = *tagged_iter;
		if (!info->is_candidate_verb(word)) {
			++tagged_iter;
			++words_iter;
			continue;
		}

		if ( (*tagged_iter == "NN" || *tagged_iter == "VBP" ) && *boost::prior(tagged_iter) == "CC" && boost::prior(tagged_iter) != tagged.begin()
				&& *boost::prior(boost::prior(tagged_iter)) == "VB") {
			*tagged_iter = "VB";
		}

		++tagged_iter;
		++words_iter;
	}
	// A NN after CC after VB becomes VB
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (boost::next(tagged_iter) != tagged.end()) {
		word = *words_iter;
		tag = *tagged_iter;
		if (info->get_conj(word, "VBG") == "") {
			++tagged_iter;
			++words_iter;
			continue;
		}
		if ((aux_is_noun(tag) || aux_is_adjective(tag)) && *boost::prior(tagged_iter) == "CC"
				&& boost::prior(tagged_iter) != tagged.begin() && *boost::prior(boost::prior(tagged_iter)) == "VBG") {
			*tagged_iter = "VBG";
		}

		++tagged_iter;
		++words_iter;
	}

// A NN after WDT or IN(that) becomes VB
//	words_iter = words.begin();
//	tagged_iter = tagged.begin() + 1;
//	while (boost::next(tagged_iter) != tagged.end()) {
//		word = *words_iter;
//		tag = *tagged_iter;
//		if (!info->is_candidate_verb(word)) {
//			++tagged_iter;
//			++words_iter;
//			continue;
//		}
//		if ((aux_is_noun(*tagged_iter) || *tagged_iter == "JJ")
//				&& (*boost::prior(tagged_iter) == "WDT")
//		) {
//			*tagged_iter = "VBP";
//		}
//
//		++tagged_iter;
//		++words_iter;
//	}

// the best/RBS game/NN -> the best/JJS game/NN
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (boost::next(tagged_iter) != tagged.end()) {
		word = *words_iter;
		tag = *tagged_iter;
		if (*tagged_iter == "RBS" && *boost::prior(tagged_iter) == "DT"
				&& (aux_is_noun(*boost::next(tagged_iter)) || aux_is_adjective(*boost::next(tagged_iter)))) {
			*tagged_iter = "JJS";
		}

		++tagged_iter;
		++words_iter;
	}

// medieval painting/VBG -> medieval painting/NN
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (boost::next(tagged_iter) != tagged.end()) {
		word = *words_iter;
		tag = *tagged_iter;
		if (!info->is_candidate_name(word)) {
			++tagged_iter;
			++words_iter;
			continue;
		}
		if (*tagged_iter == "VBG" && *boost::prior(tagged_iter) == "NN" && words_iter != words.begin()
				&& d->pertains_to_name(*boost::prior(words_iter), "pastness", 6) > 0.4) {
			*tagged_iter = "NN";
		}

		++tagged_iter;
		++words_iter;
	}

// the/DT best/RBS preserved/VBD -> the/DT best/JJS preserved/JJ
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (boost::next(tagged_iter) != tagged.end()) {
		word = *words_iter;
		tag = *tagged_iter;
		base = info->get_conj(word, "VBN");
		if (base == "") {
			++tagged_iter;
			++words_iter;
			continue;
		}
		if (!d->is_adjective(word)) {
			++tagged_iter;
			++words_iter;
			continue;
		}
		if (*tagged_iter == "VBD" && (*boost::prior(tagged_iter) == "RBS" || *boost::prior(tagged_iter) == "JJS")
				&& boost::prior(tagged_iter) != tagged.begin() && *boost::prior(boost::prior(tagged_iter)) == "DT") {
			*tagged_iter = "JJ";
			*boost::prior(tagged_iter) = "JJS";
		}

		++tagged_iter;
		++words_iter;
	}

	// Enforces "rise/VB to/TO power/NN"
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (boost::next(tagged_iter) != tagged.end()) {
		word = *words_iter;
		tag = *tagged_iter;
		base = info->get_conj(word, tag);
		if ((word == "rose") && *boost::next(tagged_iter) == "TO" && boost::next(words_iter) != words.end()
				&& *boost::next(boost::next(words_iter)) == "power") {
			*tagged_iter = "VBD";
			*boost::next(boost::next(tagged_iter)) = "NN";
		}
		if ((base == "rise" || base == "come") && *boost::next(tagged_iter) == "TO" && boost::next(words_iter) != words.end()
				&& *boost::next(boost::next(words_iter)) == "power") {
			if (!aux_is_verb(tag))
				*tagged_iter = "VB";
			*boost::next(boost::next(tagged_iter)) = "NN";
		}

		++tagged_iter;
		++words_iter;
	}

// most/RBS manuscripts/NN -> most/JJS manuscripts/NN
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (boost::next(boost::next(tagged_iter)) != tagged.end() && boost::next(tagged_iter) != tagged.end()
			&& boost::next(words_iter) != words.end()) {
		word = *words_iter;
		tag = *tagged_iter;
		if (*tagged_iter == "RBS" && (words_iter == words.begin() || *boost::prior(tagged_iter) == "CC")
				&& aux_is_adjective(*boost::next(tagged_iter))) {
			*tagged_iter = "JJS";
		}

		++tagged_iter;
		++words_iter;
	}

// IN wimbledon, london -> IN wimbledon_london ...
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (boost::next(boost::next(tagged_iter)) != tagged.end() && boost::next(tagged_iter) != tagged.end()
			&& boost::next(words_iter) != words.end()) {
		word = *words_iter;
		tag = *tagged_iter;

		if (words_iter == words.begin() || boost::prior(tagged_iter) == tagged.begin()) {
			++tagged_iter;
			++words_iter;
			continue;
		}

		string prior_word = *boost::prior(words_iter);
		string next_word = *boost::next(words_iter);

		if (*tagged_iter == "-comma-" && words_iter != words.begin() && aux_is_noun(*boost::next(tagged_iter))
				&& aux_is_noun(*boost::prior(tagged_iter))
				&& (*boost::prior(boost::prior(tagged_iter)) == "IN" || *boost::prior(boost::prior(tagged_iter)) == "JJ")
				&& (*boost::next(boost::next(tagged_iter)) == "IN" || *boost::next(boost::next(tagged_iter)) == "-comma-"
						|| *boost::next(boost::next(tagged_iter)) == "-period-")
				&& ((d->hypernym_dist(prior_word, "district", 6) > 0.4 && d->hypernym_dist(next_word, "city", 6) > 0.4)
						|| (d->hypernym_dist(prior_word, "national_capital", 6) > 0.4
								&& d->hypernym_dist(next_word, "country", 6) > 0.4)
						|| (d->hypernym_dist(prior_word, "city", 6) > 0.4 && d->hypernym_dist(next_word, "country", 6) > 0.4)
						|| (d->hypernym_dist(prior_word, "district", 6) > 0.4
								&& d->hypernym_dist(next_word, "country", 6) > 0.4))) {
			string new_string = string("[place]_") + *boost::prior(words_iter) + "_" + next_word;
			*boost::prior(words_iter) = new_string;
			words_iter = words.erase(words_iter);
			tagged_iter = tagged.erase(tagged_iter);
			words_iter = words.erase(words_iter);
			tagged_iter = tagged.erase(tagged_iter);
			if (*tagged_iter == "-comma-") { // the fungus was found in sardinia, italy, in 2006
				words_iter = words.erase(words_iter);
				tagged_iter = tagged.erase(tagged_iter);
			}

			continue;
		}

		if (*tagged_iter == "-comma-" && words_iter != words.begin() && aux_is_noun(*boost::next(tagged_iter))
				&& aux_is_noun(*boost::prior(tagged_iter))
				&& (*boost::prior(boost::prior(tagged_iter)) == "IN" || *boost::prior(boost::prior(tagged_iter)) == "TO")
				&& (*boost::next(boost::next(tagged_iter)) == "IN" || *boost::next(boost::next(tagged_iter)) == "-comma-"
						|| *boost::next(boost::next(tagged_iter)) == "-period-")
				&& (next_word == "australia" // australia is not a country in Wordnet?!
				    || d->hypernym_dist(next_word, "country", 6) > 0.4
				    || d->hypernym_dist(next_word, "state", 6) > 0.4)
		) {
			string new_string = string("[place]_") + *boost::prior(words_iter) + "_" + next_word;
			*boost::prior(words_iter) = new_string;
			words_iter = words.erase(words_iter);
			tagged_iter = tagged.erase(tagged_iter);
			words_iter = words.erase(words_iter);
			tagged_iter = tagged.erase(tagged_iter);
			if (*tagged_iter == "-comma-") { // the fungus was found in sardinia, italy, in 2006
				words_iter = words.erase(words_iter);
				tagged_iter = tagged.erase(tagged_iter);
			}

			continue;
		}

		++tagged_iter;
		++words_iter;
	}


// wimbledon, london was-> wimbledon_london was
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (boost::next(boost::next(tagged_iter)) != tagged.end() && boost::next(tagged_iter) != tagged.end()
			&& boost::next(words_iter) != words.end()) {
		word = *words_iter;
		tag = *tagged_iter;

		if (words_iter == words.begin() || boost::prior(tagged_iter) == tagged.begin()) {
			++tagged_iter;
			++words_iter;
			continue;
		}

		string prior_word = *boost::prior(words_iter);
		string next_word = *boost::next(words_iter);

		if (*tagged_iter == "-comma-" && words_iter != words.begin() && aux_is_noun(*boost::next(tagged_iter))
				&& aux_is_noun(*boost::prior(tagged_iter)) && aux_is_verb(*boost::next(boost::next(tagged_iter)))
				&& (d->hypernym_dist(next_word, "city", 6) > 0.4 || d->hypernym_dist(next_word, "country", 6) > 0.4
						|| d->hypernym_dist(next_word, "national_capital", 6) > 0.4)
				&& (d->hypernym_dist(word, "city", 6) > 0.4 || d->hypernym_dist(word, "country", 6) > 0.4
						|| d->hypernym_dist(word, "national_capital", 6) > 0.4)) {
			*boost::prior(words_iter) += "_" + next_word;
			words_iter = words.erase(words_iter);
			tagged_iter = tagged.erase(tagged_iter);
			words_iter = words.erase(words_iter);
			tagged_iter = tagged.erase(tagged_iter);
			continue;
		}

		++tagged_iter;
		++words_iter;
	}

// Corrects POS tagged as NN
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (boost::next(tagged_iter) != tagged.end()) {
		word = *words_iter;
		tag = *tagged_iter;

		if (word == "\'" //&& aux_is_noun(tag)
		&& aux_is_noun(*boost::prior(tagged_iter)) && aux_is_noun(*boost::next(tagged_iter))) {
			*tagged_iter = "POS";
			*words_iter = "\'s";
		}

		++tagged_iter;
		++words_iter;
	}

// Corrects POS tagged as NN
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (boost::next(tagged_iter) != tagged.end()) {
		word = *words_iter;
		tag = *tagged_iter;

		if (word == "\'s" //&& aux_is_noun(tag)
		&& tag == "POS" && *boost::prior(tagged_iter) == "DT" && words_iter != words.begin()
				&& *boost::prior(words_iter) == "that") {
			*tagged_iter = "VBZ";
			*words_iter = "is";
		}

		if (word == "\'s" //&& aux_is_noun(tag)
				&& tag == "POS" && *boost::prior(tagged_iter) == "PRP" && words_iter != words.begin()
		) {
			*tagged_iter = "VBZ";
			*words_iter = "is";
		}

		++tagged_iter;
		++words_iter;
	}

// tried for is always tried/VBN for
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (boost::next(tagged_iter) != tagged.end()) {
		word = *words_iter;
		tag = *tagged_iter;

		if (word == "tried" && tag == "VBD" && *boost::next(tagged_iter) == "IN" && words_iter != words.end()
				&& *boost::next(words_iter) == "for") {
			*tagged_iter = "VBN";
		}

		++tagged_iter;
		++words_iter;
	}


// A NNS after who/WP (or after a proper name) can become VBZ (not for question)
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (boost::next(tagged_iter) != tagged.end()) {
		word = *words_iter;
		tag = *tagged_iter;

		base = info->get_conj(word, "VBZ");
		if (base == "" || !info->is_candidate_verb(base)) {
			++tagged_iter;
			++words_iter;
			continue;
		}

		if (*tagged_iter == "NNS" && words_iter != words.begin()
				&& (*boost::prior(tagged_iter) == "WP" && *boost::prior(words_iter) == "who")) {
			*tagged_iter = "VBZ";
		}

		if (*tagged_iter == "NNS" && words_iter != words.begin()
				&& (*boost::prior(tagged_iter) == "NNP" && d->gender_proper_name(*boost::prior(words_iter)) != "")) {
			*tagged_iter = "VBZ";
		}

		++tagged_iter;
		++words_iter;
	}


// the us/PRP -> the us/NNP
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (boost::next(tagged_iter) != tagged.end()) {
		word = *words_iter;
		tag = *tagged_iter;
		if (word == "us" && tag == "PRP"
			&& ( *boost::prior(tagged_iter) == "DT" || *boost::prior(tagged_iter) == "WDT" || *boost::prior(tagged_iter) == "WP")
		) {
			cout << "HERE!!!" << endl;

			*tagged_iter = "NNP";
		}
		++tagged_iter;
		++words_iter;
	}

// such/JJ spying/VBG -> such/PDT spying/NN
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (boost::next(tagged_iter) != tagged.end()) {
		word = *words_iter;
		tag = *tagged_iter;
		if (word == "such" && tag == "JJ"
				&& (*boost::next(tagged_iter) == "VBG")
		) {
			*tagged_iter = "PDT";
			*boost::next(tagged_iter) = "NN";
		}
		if (word == "such" && tag == "JJ"
				&& (*boost::next(tagged_iter) == "DT")
		) {
			*tagged_iter = "PDT";
		}
		++tagged_iter;
		++words_iter;
	}

// to/TO that/IN question/VBP -> to/TO that/DT question/NN
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (boost::next(tagged_iter) != tagged.end()) {
		word = *words_iter;
		tag = *tagged_iter;
		if (word == "that" && (tag == "IN" || tag == "WDT")
				&& (*boost::next(tagged_iter) == "VB" || *boost::next(tagged_iter) == "VBP")
				&& *boost::prior(tagged_iter) == "TO") {
			*tagged_iter = "DT";
			*boost::next(tagged_iter) = "NN";
		}
		++tagged_iter;
		++words_iter;
	}

// if there are no verbs in the phrase try to convert NNS to VBZ (English likes verbs)
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	vector<string>::iterator NNS_pos;
	bool has_verb = false;
	int num_NNS;
	string candidate_verb = "";
	while (boost::next(tagged_iter) != tagged.end()) {
		word = *words_iter;
		tag = *tagged_iter;
		if (aux_is_plural(tag)
				&& !(aux_is_preposition(*boost::prior(tagged_iter)) && words_iter != words.begin()
						&& *boost::prior(words_iter) != "that")) {
			NNS_pos = tagged_iter;
			candidate_verb = info->get_conj(word, "VBZ");
			++num_NNS;
		}
		if (!has_verb && aux_is_verb(tag)) {
			has_verb = true;
			break;
		}
		++tagged_iter;
		++words_iter;
	}
	if (!has_verb && num_NNS == 1 && info->is_candidate_verb(candidate_verb)) {
		*NNS_pos = "VBZ";
		has_verb = true;
	}

	// if there are still no verbs in the phrase try to convert the first NNS that can be a verb to a VBZ (English likes verbs)
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	candidate_verb = "";
	while (!has_verb && boost::next(tagged_iter) != tagged.end()) {
		word = *words_iter;
		tag = *tagged_iter;
		if (aux_is_plural(tag)
				&& !(aux_is_preposition(*boost::prior(tagged_iter)) && words_iter != words.begin()
						&& *boost::prior(words_iter) != "that")) {
			NNS_pos = tagged_iter;
			candidate_verb = info->get_conj(word, "VBZ");
			++num_NNS;
		}
		if (info->is_candidate_verb(candidate_verb)) {
			*tagged_iter = "VBZ";
			break;
		}
		++tagged_iter;
		++words_iter;
	}

// if there are still no verbs in the phrase try to convert NN to VBP (English likes verbs)
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	NNS_pos = tagged.begin();
	has_verb = false;
	num_NNS = 0;
	candidate_verb = "";
	while (boost::next(tagged_iter) != tagged.end()) {
		word = *words_iter;
		tag = *tagged_iter;
		if (tag == "NN") {
			NNS_pos = tagged_iter;
			candidate_verb = info->get_conj(word, "VBP");
			++num_NNS;
		}
		if (!has_verb && aux_is_verb(tag)) {
			has_verb = true;
			break;
		}
		++tagged_iter;
		++words_iter;
	}
	if (!has_verb && num_NNS == 1 && info->is_candidate_verb(candidate_verb)) {
		*NNS_pos = "VBP";
	}

	// WP VBZ VBZ -> WP VBZ NNS
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (boost::next(tagged_iter) != tagged.end()) {

		if(boost::next(words_iter) == words.end())
			break;

		word = *words_iter;
		tag = *tagged_iter;
		string next_word = *boost::next(words_iter);
		string next_tag = *boost::next(tagged_iter);

		base = info->get_conj(word, "NNS");
		string next_base = info->get_conj(next_word, "NNS");

		if (tag == "VBZ" && *boost::prior(tagged_iter) == "WP"
			&& *boost::next(tagged_iter) == "VBZ"
		) {
			string next_word = *boost::next(words_iter);
			if(next_base != "" && next_word != "is") // the next word must be convertible to a noun
				*boost::next(tagged_iter) = "NNS";
			else if(base != "")
				*tagged_iter = "NNS";
		}
		++tagged_iter;
		++words_iter;
	}

// A WDT after a verb becomes IN
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (boost::next(tagged_iter) != tagged.end()) {
		word = *words_iter;
		tag = *tagged_iter;
		if (word == "that" && tag == "WDT" && aux_is_verb(*boost::prior(tagged_iter))) {
			*tagged_iter = "IN";
		}
		++tagged_iter;
		++words_iter;
	}

// In questions all "that" are WDT after a noun, IN otherwise
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (is_question_ && boost::next(tagged_iter) != tagged.end()) {
		word = *words_iter;
		tag = *tagged_iter;
		if (word == "that" && tag == "IN" && aux_is_noun(*boost::prior(tagged_iter))) {
			*tagged_iter = "WDT";
		}
		++tagged_iter;
		++words_iter;
	}

// new york -> new_york
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (boost::next(tagged_iter) != tagged.end()) {
		word = *words_iter;
		tag = *tagged_iter;

		if (boost::next(words_iter) == words.end()) {
			++tagged_iter;
			++words_iter;
			continue;
		}

		if (!(aux_is_noun(*tagged_iter) || aux_is_adjective(*tagged_iter)) || !aux_is_noun(*boost::next(tagged_iter))) {
			++tagged_iter;
			++words_iter;
			continue;
		}

		string next_word = *boost::next(words_iter);
		string next_tag = *boost::next(tagged_iter);

		string candidate_name = word + "_" + next_word;


		if (d->hypernym_dist(candidate_name, "country") > 0.4 || d->hypernym_dist(candidate_name, "city") > 0.4) {
			*tagged_iter = "NNP";
			*words_iter = candidate_name;
			tagged.erase(tagged_iter + 1);
			words.erase(words_iter + 1);
			continue;
		}
		++tagged_iter;
		++words_iter;
	}

// a noun/NN that can be JJ and is before a NN, becomes JJ
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (boost::next(tagged_iter) != tagged.end()) {
		word = *words_iter;
		tag = *tagged_iter;
		if (!d->is_adjective(word)) {
			++tagged_iter;
			++words_iter;
			continue;
		}
		if ((tag == "NN" || tag == "NNP") && d->is_adjective(word)
				&& (aux_is_noun(*boost::next(tagged_iter)) || aux_is_adjective(*boost::next(tagged_iter)))
				&& !(aux_is_article(*boost::prior(tagged_iter)) && *boost::next(tagged_iter) == "-period-")
				&& !(aux_is_article(*boost::prior(tagged_iter)) && boost::next(words_iter) == words.end())) {
			*tagged_iter = "JJ";
		}
		++tagged_iter;
		++words_iter;
	}

// meeting/VBG before a verb becomes meeting/NN
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (boost::next(tagged_iter) != tagged.end()) {
		word = *words_iter;
		tag = *tagged_iter;
		if (tag == "VBG" && word == "meeting" && aux_is_verb(*boost::next(tagged_iter))
				&& (aux_is_noun(*boost::prior(tagged_iter)) || *boost::prior(tagged_iter) == "DT")) {
			*tagged_iter = "NN";
		}
		++tagged_iter;
		++words_iter;
	}

// the commission next/JJ meets when ... -> next/RB
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (boost::next(tagged_iter) != tagged.end()) {
		word = *words_iter;
		tag = *tagged_iter;
		if (tag == "JJ" && word == "next" && aux_is_verb(*boost::next(tagged_iter)) && (aux_is_noun(*boost::prior(tagged_iter)))) {
			*tagged_iter = "RB";
		}
		++tagged_iter;
		++words_iter;
	}

// He was playing cricket outside/RB
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (boost::next(tagged_iter) != tagged.end()) {
		word = *words_iter;
		tag = *tagged_iter;
		if (tag != "RB" && word == "outside"
				&& (*boost::next(tagged_iter) == "-period-" || *boost::next(tagged_iter) == "-comma-"
						|| *boost::next(tagged_iter) == "CC") && aux_is_noun(*boost::prior(tagged_iter))) {
			*tagged_iter = "RB";
		}
		++tagged_iter;
		++words_iter;
	}

// the general meeting is held ...
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (boost::next(tagged_iter) != tagged.end()) {
		word = *words_iter;
		tag = *tagged_iter;
		if (tag == "VBG" && word == "meeting" && aux_is_verb(*boost::next(tagged_iter))
				&& (aux_is_noun(*boost::prior(tagged_iter)) || *boost::prior(tagged_iter) == "DT")) {
			*tagged_iter = "RB";
		}
		++tagged_iter;
		++words_iter;
	}

// PDT NN -> DT NN
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (boost::next(tagged_iter) != tagged.end()) {
		word = *words_iter;
		tag = *tagged_iter;
		if (tag == "PDT" && aux_is_noun(*boost::next(tagged_iter))) {
			*tagged_iter = "DT";
		}
		++tagged_iter;
		++words_iter;
	}

// simple/JJ and/CC clean/VBP -> JJ CC JJ
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (boost::next(tagged_iter) != tagged.end()) {
		word = *words_iter;
		tag = *tagged_iter;

		if (!d->is_adjective(word)) {
			++tagged_iter;
			++words_iter;
			continue;
		}

		if (boost::prior(tagged_iter) == tagged.begin()) {
			++tagged_iter;
			++words_iter;
			continue;
		}
		if (aux_is_verb(tag) && d->is_adjective(word) && *boost::prior(tagged_iter) == "CC"
				&& *boost::prior(boost::prior(tagged_iter)) == "JJ") {
			*tagged_iter = "JJ";
		}
		++tagged_iter;
		++words_iter;
	}

	// make/VB it/PRP clear/VBP->JJ
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (boost::next(tagged_iter) != tagged.end()) {
		word = *words_iter;
		tag = *tagged_iter;

		if (words_iter == words.begin() || boost::prior(words_iter) == words.begin()) {
			++tagged_iter;
			++words_iter;
			continue;
		}
		if (!d->is_adjective(word)) {
			++tagged_iter;
			++words_iter;
			continue;
		}
		if (aux_is_verb(tag) && d->is_adjective(word) && *boost::prior(tagged_iter) == "PRP" && *boost::prior(words_iter) == "it"
				&& (*boost::prior(boost::prior(words_iter)) == "make" || *boost::prior(boost::prior(words_iter)) == "makes")) {
			*tagged_iter = "JJ";
			if (words_iter != words.end() && *boost::next(words_iter) == "that") {
				*boost::next(tagged_iter) = "IN";
			}
		}
		++tagged_iter;
		++words_iter;
	}

// "... not as high an operational concern as ..."
// You don't want high/JJ an/DT to trigger a subordinate
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (boost::next(tagged_iter) != tagged.end()) {
		word = *words_iter;
		tag = *tagged_iter;
		if (tag == "IN" && word == "as" && *boost::next(tagged_iter) == "JJ" && boost::next(tagged_iter) != tagged.end()
				&& (aux_is_article(*boost::next(boost::next(tagged_iter)))
				//  || aux_is_noun(*boost::next(boost::next(tagged_iter)))
				)) {
			string new_name = "as";
			new_name += "[" + *boost::next(words_iter) + "]";
			*words_iter = new_name;
			words_iter = words.erase(words_iter + 1);
			tagged_iter = tagged.erase(tagged_iter + 1);
			continue;
		}
		++tagged_iter;
		++words_iter;
	}

// "This came the day [when] his advisor ...
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (boost::next(tagged_iter) != tagged.end()) {
		word = *words_iter;
		tag = *tagged_iter;
		if ((word == "day" || word == "year" || word == "month") && aux_is_noun(tag)
				&& (aux_is_article(*boost::next(tagged_iter)) || *boost::next(tagged_iter) == "PRP$")) {
			tagged_iter = tagged.insert(tagged_iter + 1, "WRB");
			words_iter = words.insert(words_iter + 1, "when");
			continue;
		}
		++tagged_iter;
		++words_iter;
	}

// Corrects VBP or VB that should be NN after VBG
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (boost::next(tagged_iter) != tagged.end()) {
		string dummy_str;
		word = *words_iter;
		tag = *tagged_iter;
		if ((tag == "VB" || tag == "VBP") && info->is_candidate_name(word) && *boost::prior(tagged_iter) == "VBG") {
			*tagged_iter = "NN";
		}
		++tagged_iter;
		++words_iter;
	}

// to/TO us/NNP -> to/TO us/PRP
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (boost::next(tagged_iter) != tagged.end()) {
		string dummy_str;
		word = *words_iter;
		tag = *tagged_iter;
		if (word == "us" && aux_is_noun(tag)
				&& !(*boost::prior(tagged_iter) == "DT" || *boost::prior(tagged_iter) == "WP"
					|| *boost::prior(tagged_iter) == "WDT" || aux_is_adjective(*boost::prior(tagged_iter)))) {
			*tagged_iter = "PRP";
		}
		++tagged_iter;
		++words_iter;
	}

// "last" between V and VBN is RB
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (boost::next(tagged_iter) != tagged.end()) {
		string dummy_str;
		word = *words_iter;
		tag = *tagged_iter;
		if (word == "last" && tag == "JJ" && aux_is_verb(*boost::prior(tagged_iter)) && *boost::next(tagged_iter) == "VBN") {
			*tagged_iter = "RB";
		}
		++tagged_iter;
		++words_iter;
	}

// $ 8 -> 8 $ (for a better understanding in drt_builder)
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (boost::next(tagged_iter) != tagged.end()) {
		string dummy_str;
		word = *words_iter;
		tag = *tagged_iter;
		if (tag == "$" && *boost::next(tagged_iter) == "CD") {
			*boost::next(tagged_iter) = "$";
			*tagged_iter = "CD";
			string tmp_str = word;
			*words_iter = *boost::next(words_iter);
			*boost::next(words_iter) = tmp_str;
		}
		++tagged_iter;
		++words_iter;
	}

// if the second noun is NNP and the first is NN then the first becomes NNP
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (boost::next(tagged_iter) != tagged.end()) {
		string dummy_str;
		word = *words_iter;
		tag = *tagged_iter;
		if (tag == "NN" && words_iter == words.begin()
				&& (*boost::next(tagged_iter) == "NNP" || *boost::next(tagged_iter) == "NNPS")) {
			*tagged_iter = "NNP";
		}
		++tagged_iter;
		++words_iter;
	}

// reviews/VBZ were/VBD -> reviews/NNS were/VBD
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (boost::next(tagged_iter) != tagged.end()) {
		string dummy_str;
		word = *words_iter;
		tag = *tagged_iter;
		base = info->get_conj(word, "VBZ");
		if (tag == "VBZ" && info->is_candidate_verb(base) && *boost::next(tagged_iter) == "VBD") {
			*tagged_iter = "NNS";
		}
		++tagged_iter;
		++words_iter;
	}

// though/IN most/JJS -> but/CC most/JJS
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (boost::next(tagged_iter) != tagged.end()) {
		string dummy_str;
		word = *words_iter;
		tag = *tagged_iter;
		if (word == "though" && *boost::prior(tagged_iter) == "-comma-" && *boost::next(tagged_iter) == "JJS") {
			*tagged_iter = "CC";
			*words_iter = "but";
		}
		++tagged_iter;
		++words_iter;
	}
// though/NN most/RBS -> but/CC most/JJS
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (boost::next(tagged_iter) != tagged.end()) {
		string dummy_str;
		word = *words_iter;
		tag = *tagged_iter;
		if (word == "though" && *boost::prior(tagged_iter) == "-comma-" && *boost::next(tagged_iter) == "RBS") {
			*tagged_iter = "CC";
			*words_iter = "but";
			*boost::next(tagged_iter) = "JJS";
		}
		++tagged_iter;
		++words_iter;
	}

// p/NN t/NN|JJ barnum/NNP
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (boost::next(tagged_iter) != tagged.end()) {
		string dummy_str;
		word = *words_iter;
		tag = *tagged_iter;
		if (boost::next(words_iter) == words.end()) {
			++tagged_iter;
			++words_iter;
			continue;
		}
		if (word.size() == 1 && (tag == "NNP" || tag == "NN" || tag == "JJ") && aux_is_noun(*boost::next(tagged_iter))) {
			string new_tag = "NNP";
			string new_word = word + "_" + *boost::next(words_iter);
			*words_iter = new_word;
			*tagged_iter = new_tag;
			words_iter = words.erase(words_iter + 1);
			tagged_iter = tagged.erase(tagged_iter + 1);
			continue;
		}
		++tagged_iter;
		++words_iter;
	}

// gives/VB|VBP -> gives/VBZ
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (boost::next(tagged_iter) != tagged.end()) {
		string dummy_str;
		word = *words_iter;
		tag = *tagged_iter;
		string base = info->get_conj(word, "VBZ");
		if ((tag == "VBP" || tag == "VB") && base != "" && info->is_candidate_verb(base)) {
			*tagged_iter = "VBZ";
			continue;
		}
		++tagged_iter;
		++words_iter;
	}
// gives/VB|VBP -> gives/VBZ
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (boost::next(tagged_iter) != tagged.end()) {
		string dummy_str;
		word = *words_iter;
		tag = *tagged_iter;

		if ((tag == "VBP" || tag == "VB") && *boost::prior(tagged_iter) == "IN" && words_iter != words.begin()
				&& *boost::prior(words_iter) != "that" && info->is_candidate_name(word)) {
			*tagged_iter = "NN";
			continue;
		}
		++tagged_iter;
		++words_iter;
	}

// games/NN|NNP -> games/NNS|NNPS
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (boost::next(tagged_iter) != tagged.end()) {
		string dummy_str;
		word = *words_iter;
		tag = *tagged_iter;

		string base = info->get_conj(word, "NNS");
		if (tag == "NNP" && base != "") {
			*tagged_iter = "NNPS";
			continue;
		}
		if (tag == "NN" && base != "") {
			*tagged_iter = "NNS";
			continue;
		}

		++tagged_iter;
		++words_iter;
	}

// games/JJ at/IN -> games/NN at/IN
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (boost::next(tagged_iter) != tagged.end()) {
		string dummy_str;
		word = *words_iter;
		tag = *tagged_iter;

		string base = info->get_conj(word, tag);
		if (base == "")
			base = word;
		if (tag == "JJ" && info->is_candidate_name(base) && *boost::next(tagged_iter) == "IN"
				&& *boost::next(words_iter) != "than") {
			*tagged_iter = "NN";
			continue;
		}

		++tagged_iter;
		++words_iter;
	}

// the/DT inside/IN -> the/DT inside/NN
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (boost::next(tagged_iter) != tagged.end()) {
		string dummy_str;
		word = *words_iter;
		tag = *tagged_iter;

		if (tag == "IN" && info->is_candidate_name(word) && *boost::prior(tagged_iter) == "DT" && words_iter != words.begin()
				&& (*boost::prior(words_iter) == "the" || *boost::prior(words_iter) == "a")) {
			*tagged_iter = "NN";
			continue;
		}

		++tagged_iter;
		++words_iter;
	}

// over/IN 9000/CD -> more-than/IN 9000/CD
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (boost::next(tagged_iter) != tagged.end()) {
		string dummy_str;
		word = *words_iter;
		tag = *tagged_iter;

		if (tag == "IN" && word == "over" && *boost::next(tagged_iter) == "CD") {
			*words_iter = "more-than";
			continue;
		}

		++tagged_iter;
		++words_iter;
	}

	// more-than/IN 3/CD people -> [more-than]_3/CD people
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (boost::next(tagged_iter) != tagged.end()) {
		string dummy_str;
		word = *words_iter;
		tag = *tagged_iter;
		if (word == "more-than" && tag == "IN" && words_iter != words.end() && *boost::next(tagged_iter) == "CD") {
			string next_word = *boost::next(words_iter);
			*boost::next(words_iter) = string("[more-than]_") + next_word;
			tagged_iter = tagged.erase(tagged_iter);
			words_iter = words.erase(words_iter);
		}
		++tagged_iter;
		++words_iter;
	}

	// less-than/IN 3/CD people -> [less-than]_3/CD people
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (boost::next(tagged_iter) != tagged.end()) {
		string dummy_str;
		word = *words_iter;
		tag = *tagged_iter;
		if (word == "less-than" && tag == "IN" && words_iter != words.end() && *boost::next(tagged_iter) == "CD") {
			string next_word = *boost::next(words_iter);
			*boost::next(words_iter) = string("[less-than]_") + next_word;
			tagged_iter = tagged.erase(tagged_iter);
			words_iter = words.erase(words_iter);
		}
		++tagged_iter;
		++words_iter;
	}

// this/DT means/NNS -> this/DT means/VBZ
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (boost::next(tagged_iter) != tagged.end()) {
		string dummy_str;
		word = *words_iter;
		tag = *tagged_iter;

		base = info->get_conj(word, "VBZ");

		if (tag == "NNS" && *boost::prior(tagged_iter) == "DT" && words_iter != words.begin()
				&& (*boost::prior(words_iter) == "this" || *boost::prior(words_iter) == "that")
				&& info->is_candidate_verb(base)) {
			*tagged_iter = "VBZ";
			continue;
		}

		++tagged_iter;
		++words_iter;
	}

// more/JJR marked/VBD -> more/JJR marked/JJ
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (boost::next(tagged_iter) != tagged.end()) {
		string dummy_str;
		word = *words_iter;
		tag = *tagged_iter;

		if ((tag == "JJR" || tag == "RBR") && word == "more"
				&& (*boost::next(tagged_iter) == "VBD" || *boost::next(tagged_iter) == "VBN")
				&& boost::next(words_iter) != words.end() && d->is_adjective(*boost::next(words_iter))) {
			*tagged_iter = "JJR";
			*boost::next(tagged_iter) = "JJ";
			continue;
		}

		++tagged_iter;
		++words_iter;
	}

// alzheimer/NN 's/POS -comma-/-comma- -> alzheimer/NN 's/POS [*]/NN -comma-/-comma-
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (boost::next(tagged_iter) != tagged.end()) {
		string dummy_str;
		word = *words_iter;
		tag = *tagged_iter;

		if ((tag == "POS")
				&& (*boost::next(tagged_iter) == "-comma-" || *boost::next(tagged_iter) == "-period-"
						|| *boost::next(tagged_iter) == "CC" || aux_is_verb(*boost::next(tagged_iter)))) {
			words_iter = words.insert(words_iter + 1, "[*]");
			tagged_iter = tagged.insert(tagged_iter + 1, "NN");
		}

		++tagged_iter;
		++words_iter;
	}

	// print/NN a page -> print/VB a page
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (boost::next(tagged_iter) != tagged.end()) {
		string dummy_str;
		word = *words_iter;
		tag = *tagged_iter;

		string base = info->get_conj(word, "VB");
		if (base == "") {
			++tagged_iter;
			++words_iter;
			continue;
		}

		if (words_iter == words.begin() && *boost::next(tagged_iter) == "DT") {
			*tagged_iter = "VB";
		}
		++tagged_iter;
		++words_iter;
	}

	// AUX JJ VBN -> AUX RB VBN
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (boost::next(tagged_iter) != tagged.end()) {
		string dummy_str;
		word = *words_iter;
		tag = *tagged_iter;

		if (words_iter == words.begin()) {
			++tagged_iter;
			++words_iter;
			continue;
		}

		string prior_word = *boost::prior(words_iter);
		string prior_tag = *boost::prior(tagged_iter);

		string base = info->get_conj(word, tag);
		if (base != "be" && base != "have") {
			++tagged_iter;
			++words_iter;
			continue;
		}

		if (tag == "JJ" && aux_is_verb(*boost::prior(tagged_iter)) && *boost::next(tagged_iter) == "VBN" && d->is_adverb(word)) {
			*tagged_iter = "RB";
		}
		++tagged_iter;
		++words_iter;
	}

	// are there/RB->EX
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (is_question_ && boost::next(tagged_iter) != tagged.end()) {
		string dummy_str;
		word = *words_iter;
		tag = *tagged_iter;

		if (words_iter == words.begin()) {
			++tagged_iter;
			++words_iter;
			continue;
		}

		string prior_word = *boost::prior(words_iter);
		string prior_tag = *boost::prior(tagged_iter);

		string base = info->get_conj(prior_word, prior_tag);
		if (base == "be" && tag != "EX" && word == "there") {
			*tagged_iter = "EX";
		}
		++tagged_iter;
		++words_iter;
	}

	// please/VB print/NN a page -> print/VB a page
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (boost::next(tagged_iter) != tagged.end()) {
		string dummy_str;
		word = *words_iter;
		tag = *tagged_iter;

		string base = info->get_conj(word, "VB");
		if (base == "") {
			++tagged_iter;
			++words_iter;
			continue;
		}

		if (words_iter != words.begin() && *boost::prior(words_iter) == "please"
				&& (*boost::next(tagged_iter) == "DT" || *boost::next(tagged_iter) == "NNS")) {
			*tagged_iter = "VB";
			*boost::prior(tagged_iter) = "RB";
		}
		++tagged_iter;
		++words_iter;
	}

	// about/IN half/PDT -> circa/RB half/PDF
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (boost::next(tagged_iter) != tagged.end()) {
		string dummy_str;
		word = *words_iter;
		tag = *tagged_iter;

		if (words_iter != words.begin() && word == "about" && tag == "IN" && *boost::next(words_iter) == "half") {
			*tagged_iter = "RB";
			*words_iter = "circa";
		}
		++tagged_iter;
		++words_iter;
	}

	// -comma- bears/VBZ -comma- -> -comma- bears/NNS -comma-
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (boost::next(tagged_iter) != tagged.end()) {
		string dummy_str;
		word = *words_iter;
		tag = *tagged_iter;

		string base = info->get_conj(word, "VBZ");
		if (base == "") {
			++tagged_iter;
			++words_iter;
			continue;
		}

		if (*boost::prior(tagged_iter) == "-comma-" && *boost::next(tagged_iter) == "-comma-" && tag == "VBZ"
				&& info->is_candidate_name(base)) {
			*tagged_iter = "NNS";
		}
		++tagged_iter;
		++words_iter;
	}

	// in what/WP is/VBZ ...
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (boost::next(tagged_iter) != tagged.end()) {
		string dummy_str;
		word = *words_iter;
		tag = *tagged_iter;

		if (tag == "WP" && *boost::prior(tagged_iter) == "IN" && words_iter != words.begin()
				&& *boost::prior(words_iter) != "that"
				&& aux_is_verb(*boost::next(tagged_iter))
				) {
			if(*boost::next(tagged_iter) != "VBP") { // "that which" is supported by 3rd persons only
				words_iter = words.insert(words_iter + 1, "[prior_WP]");
				tagged_iter = tagged.insert(tagged_iter + 1, "WDT");
			} else if(words_iter != words.end() && info->is_candidate_name(*boost::next(words_iter)) ) {
				*boost::next(tagged_iter) = "NN";
			}
		}
		++tagged_iter;
		++words_iter;
	}

	// is what/WP some people believe happens ... -> what/WP [prior_WP]/WDT
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (boost::next(tagged_iter) != tagged.end() && !is_question_) {
		string dummy_str;
		word = *words_iter;
		tag = *tagged_iter;

		if (tag == "WP" && aux_is_verb(*boost::prior(tagged_iter)) && words_iter != words.end()
				&& boost::next(words_iter) != words.end()
				&& (aux_is_noun(*boost::next(tagged_iter)) || aux_is_article(*boost::next(tagged_iter))
						|| aux_is_CD(*boost::next(tagged_iter)) || aux_is_subject_PRP(*boost::next(words_iter)))) {
			words_iter = words.insert(words_iter + 1, "[prior_WP]");
			tagged_iter = tagged.insert(tagged_iter + 1, "WDT");
		}
		++tagged_iter;
		++words_iter;
	}

	// can study/VB, work/NN, -> can study/VB, work/VB,
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	bool tag_as_verb = false;
	while (boost::next(tagged_iter) != tagged.end()) {
		string dummy_str;
		word = *words_iter;
		tag = *tagged_iter;

		if (tag == "MD" && *boost::next(tagged_iter) == "VB") {
			tag_as_verb = true;
			++tagged_iter;
			++words_iter;
			++tagged_iter;
			++words_iter;
			continue;
		}
		if (tag_as_verb && tag == "NN" && info->is_candidate_verb(word)) {
			*tagged_iter = "VB";
		} else if (tag_as_verb && tag != "-comma-" && tag != "CC") {
			tag_as_verb = false;
		}
		++tagged_iter;
		++words_iter;
	}

	// to/TO study/VB, work/NN, -> to study/VB, work/VB,
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	tag_as_verb = false;
	while (boost::next(tagged_iter) != tagged.end()) {
		string dummy_str;
		word = *words_iter;
		tag = *tagged_iter;

		if (tag == "TO" && *boost::next(tagged_iter) == "VB") {
			tag_as_verb = true;
			++tagged_iter;
			++words_iter;
			++tagged_iter;
			++words_iter;
			continue;
		}
		if (tag_as_verb && tag == "NN" && info->is_candidate_verb(word)) {
			*tagged_iter = "VB";
		} else if (tag_as_verb && tag != "-comma-" && tag != "CC") {
			tag_as_verb = false;
		}
		++tagged_iter;
		++words_iter;
	}

	// when climbing/NN->VBG
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	tag_as_verb = false;
	while (boost::next(tagged_iter) != tagged.end()) {
		string dummy_str;
		word = *words_iter;
		tag = *tagged_iter;

		string base = info->get_conj(word, "VBG");
		if (base == "") {
			++tagged_iter;
			++words_iter;
			continue;
		}

		if (*boost::prior(tagged_iter) == "WRB" && aux_is_noun(tag)) {
			*tagged_iter = "VBG";
		}
		++tagged_iter;
		++words_iter;
	}

	// to live/VB means/NNS->VBZ to/TO ...
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	tag_as_verb = false;
	while (boost::next(tagged_iter) != tagged.end()) {
		string dummy_str;
		word = *words_iter;
		tag = *tagged_iter;

		string base = info->get_conj(word, "VBZ");
		if (base == "") {
			++tagged_iter;
			++words_iter;
			continue;
		}

		if (*boost::prior(tagged_iter) == "VB" && tag == "NNS" && *boost::next(tagged_iter) == "TO") {
			*tagged_iter = "VBZ";
		}
		++tagged_iter;
		++words_iter;
	}

	// "like" before "all" and after empy space is IN
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (boost::next(tagged_iter) != tagged.end()) {
		word = *words_iter;
		tag = *tagged_iter;

		if(boost::next(words_iter) == words.end() ) {
			++tagged_iter;
			++words_iter;
			continue;
		}

		string next_tag = *boost::next(tagged_iter);
		string next_word = *boost::next(words_iter);

		if (word == "number" && tag != "NN"
			&& next_tag == "IN"
			&& next_word == "of"
		) {
			*tagged_iter = "NN";
		}
		++tagged_iter;
		++words_iter;
	}

	// "cannot" -> "can not"
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (boost::next(tagged_iter) != tagged.end()) {
		word = *words_iter;
		tag = *tagged_iter;

		if (word == "cannot" && tag == "MD"
		) {
			*tagged_iter = "MD";
			*words_iter = "can";
			words_iter = words.insert(words_iter+1, "not");
			tagged_iter = tagged.insert(tagged_iter+1, "RB");

		}
		++tagged_iter;
		++words_iter;
	}


	// "like" before "all" and after empy space is IN
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (boost::next(tagged_iter) != tagged.end()) {
		word = *words_iter;
		tag = *tagged_iter;

		if(boost::next(words_iter) == words.end() ) {
			++tagged_iter;
			++words_iter;
			continue;
		}

		string next_tag = *boost::next(tagged_iter);
		string next_word = *boost::next(words_iter);

		if (word == "like" && tag != "IN"
			&& next_tag == "DT"
			//&& next_word == "all"
			&& words_iter == words.begin()
		) {
			*tagged_iter = "IN";
		}
		++tagged_iter;
		++words_iter;
	}

// usa/NN [date]_.../CD -> usa/NN DUMMY-PREP-DATE/IN [date]_.../CD
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (boost::next(tagged_iter) != tagged.end()) {
		string dummy_str;
		word = *words_iter;
		tag = *tagged_iter;
		if (aux_is_date(word) && *boost::prior(tagged_iter) != "IN" && *boost::prior(tagged_iter) != "TO"
				&& *boost::prior(tagged_iter) != "DT") {
			words_iter = words.insert(words_iter, "DUMMY-PRED-DATE");
			tagged_iter = tagged.insert(tagged_iter, "IN");
			continue;
		}
		++tagged_iter;
		++words_iter;
	}

	// which/WDT means/NNS that/IN -> which/WDT means/VBZ that/IN
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (boost::next(tagged_iter) != tagged.end()) {
		string dummy_str;
		word = *words_iter;
		tag = *tagged_iter;

		string base = info->get_conj(word, tag);

		if (tag == "NNS" && info->is_candidate_verb(base) && *boost::prior(tagged_iter) == "WDT"
				&& *boost::next(tagged_iter) == "IN" && boost::next(words_iter) != words.end()
				&& *boost::next(words_iter) == "that") {
			*tagged_iter = "VBZ";
		}
		++tagged_iter;
		++words_iter;
	}

// usa/NN [place]_.../NN -> usa/NN DUMMY-PREP-DATE/IN [date]_.../CD
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (boost::next(tagged_iter) != tagged.end()) {
		string dummy_str;
		word = *words_iter;
		tag = *tagged_iter;
		if (aux_is_place(word) && *boost::prior(tagged_iter) != "IN" && *boost::prior(tagged_iter) != "TO"
				&& *boost::prior(tagged_iter) != "DT") {
			words_iter = words.insert(words_iter, "DUMMY-PRED-PLACE");
			tagged_iter = tagged.insert(tagged_iter, "IN");
			continue;
		}
		++tagged_iter;
		++words_iter;
	}

/// [temporary] as/IN quickly/RB -> as-quickly/RB
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (boost::next(tagged_iter) != tagged.end()) {
		string dummy_str;
		word = *words_iter;
		tag = *tagged_iter;
		if (word == "as" && tag == "IN" && *boost::next(tagged_iter) == "RB" && boost::next(words_iter) != words.end()) {
			string new_str = word + "_" + *boost::next(words_iter);
			*words_iter = new_str;
			*tagged_iter = "RB";
			words_iter = words.erase(words_iter + 1);
			tagged_iter = tagged.erase(tagged_iter + 1);
			continue;
		}
		++tagged_iter;
		++words_iter;
	}

// flicking/VBP -> flicking/VBP
	words_iter = words.begin();
	tagged_iter = tagged.begin() + 1;
	while (boost::next(tagged_iter) != tagged.end()) {
		string dummy_str;
		word = *words_iter;
		tag = *tagged_iter;
		base = info->get_conj(word, "VBG");
		if (aux_is_verb(tag) && base != "") {
			*tagged_iter = "VBG";
		}
		++tagged_iter;
		++words_iter;
	}
}
