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



#include"Anaphora.hpp"

const bool debug = false;

static inline double max(double a, double b)
{
	return a > b ? a : b;
}
static inline double min(double a, double b)
{
	return a < b ? a : b;
}

template<class T>
static void print_pair_vector(std::vector<T> &vs)
{
	typename vector<T>::iterator tags_iter = vs.begin();
	while (tags_iter < vs.end()) {
		std::cout << tags_iter->first << "," << tags_iter->second << " ";
		++tags_iter;
	}
	std::cout << std::endl;
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

template<class T>
static void print_vector(const std::vector<T> &vs)
{
	typename vector<T>::const_iterator tags_iter = vs.begin();
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

static vector<pair<string, string> > erase_empty_ref(const vector<pair<string, string> > &ret_inst)
{
	vector<pair<string, string> > ret_inst_erased;

	for (int n = 0; n < ret_inst.size(); ++n) {
		string fref, sref;
		fref = ret_inst.at(n).first;
		sref = ret_inst.at(n).second;
		if (fref != "" && sref != "")
			ret_inst_erased.push_back(ret_inst.at(n));
	}

	return ret_inst_erased;
}

static string cut_numbers(string str)
{
	int n;
	for (n = 0; n < str.size(); ++n) {
		if (isdigit(str.at(n)))
			break;
	}
	str = str.substr(0, n);
	return str;
}

static vector<pair<int, int> > get_related_phrases(int n)
// This function simply returns the position of the phrases that are
// allowed to do anaphora with the n-th phrase.
{
	vector<pair<int, int> > ret_conn;
	int m = n - 1;
	while (n - m < 5 && m >= 0) {
		ret_conn.push_back(make_pair(n, m));
		--m;
	}
	std::reverse(ret_conn.begin(), ret_conn.end());
	return ret_conn;
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

static double get_string_vector_distance(metric *d, const string head_str, const vector<string> ref_strs)
/// This function should just be in Match.cpp
{

	vector<string> head_strs;
	boost::split(head_strs, head_str, boost::is_any_of("|"));

	double distance = 0;
	vector<string>::const_iterator riter = ref_strs.begin();
	vector<string>::const_iterator rend = ref_strs.end();
	vector<string>::const_iterator liter = head_strs.begin();
	vector<string>::const_iterator lend = head_strs.end();

	bool invert = false;
	for (; liter != lend; ++liter) {
		for (riter = ref_strs.begin(); riter != rend; ++riter) {
			if (*liter == "[*]" || *riter == "[*]")
				return 1;
			if (*liter == *riter) {
				distance = 1;
				continue;
			}
			string rstr = *riter, lstr = *liter;
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
				distance = max(distance, d->hypernym_dist(lstr, rstr, 9));
			} else {
				distance = max(distance, 1 - d->hypernym_dist(lstr, rstr, 9));
				invert = false;
			}
		}
	}

	return distance;
}

static vector<string> get_reference_string(string ref_str)
{
	vector<string> ret_str;
	ret_str.push_back(ref_str);
	if (ref_str == "he" || ref_str == "male" || ref_str == "man" || ref_str == "person") {
		ret_str.push_back("man");
		ret_str.push_back("person");
		ret_str.push_back("male_person");
		ret_str.push_back("adult");
		ret_str.push_back("educator");
	} else if (ref_str == "she" || ref_str == "female" || ref_str == "woman") {
		ret_str.push_back("woman");
		ret_str.push_back("person");
		ret_str.push_back("female_person");
		ret_str.push_back("adult");
		ret_str.push_back("educator");
	} else if (ref_str == "it" || ref_str == "thing" || ref_str == "this" || ref_str == "that") {
		ret_str.push_back("!person");
		ret_str.push_back("thing");
		ret_str.push_back("material");
		ret_str.push_back("building");
		ret_str.push_back("country");
		ret_str.push_back("army");
		ret_str.push_back("domestic_animal");
		ret_str.push_back("beast");
		ret_str.push_back("bird");
		ret_str.push_back("socialism");
		ret_str.push_back("capitalism");
		ret_str.push_back("political_movement");
		ret_str.push_back("step");
		ret_str.push_back("train");
		ret_str.push_back("step");
		ret_str.push_back("game");
		ret_str.push_back("test");
		ret_str.push_back("country");
		ret_str.push_back("peninsula");
		ret_str.push_back("continent");
		ret_str.push_back("reptile");
	} else if (ref_str == "animal") {
		ret_str.push_back("mammal");
		ret_str.push_back("beast");
	} else if (ref_str == "[place]_there") {
		ret_str.push_back("place");
		ret_str.push_back("pub");
		ret_str.push_back("bar");
		ret_str.push_back("cinema");
	} else
		ret_str.push_back(ref_str);
	return ret_str;
}

static bool is_subject_of_verb(DrtVect &preds, const string &ref)
{
	int m = find_verb_with_subject(preds, ref);
	if (m != -1)
		return true;
	return false;
}
static bool is_object_of_verb(DrtVect &preds, const string &ref)
{
	int m = find_verb_with_object(preds, ref);
	if (m != -1)
		return true;
	return false;
}
static DrtVect delete_all_nouns_with_ref(DrtVect drtvect, const string &ref)
{
	DrtVect to_return;
	for (int n = 0; n < drtvect.size(); ++n) {
		string fref = extract_first_tag(drtvect.at(n));
		if (drtvect.at(n).is_name() && fref == ref)
			to_return.push_back(drtvect.at(n));
	}
	return to_return;
}
static DrtVect delete_all_elements_with_pos(DrtVect drtvect, vector<int> pos)
{
	DrtVect to_return;
	for (int n = 0; n < drtvect.size(); ++n) {
		if (!shortfind(pos, n))
			to_return.push_back(drtvect.at(n));
	}
	return to_return;
}
static DrtVect get_all_elements_with_pos(DrtVect drtvect, vector<int> pos)
{
	DrtVect to_return;
	for (int n = 0; n < drtvect.size(); ++n) {
		if (shortfind(pos, n))
			to_return.push_back(drtvect.at(n));
	}
	return to_return;
}

static vector<DrtPred> substitute_ref(vector<DrtPred> &pre_drt, const string &from_str, const string &to_str)
{
	vector<DrtPred> predicates(pre_drt);

	for (int n = 0; n < predicates.size(); ++n) {
		vector<string> children = predicates.at(n).extract_children();
		for (int m = 0; m < children.size(); ++m) {
			if (children.at(m) == to_str) {
				predicates.at(n) = pre_drt.at(n);
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

static DrtVect get_drtvect_with_switced_elements(const DrtVect &drt_to, const DrtVect &drt_from, const string &ref_to,
		const string &ref_from)
{
	DrtVect to_return;

	delete_all_nouns_with_ref(drt_from, ref_from);
	to_return = drt_from;
	vector<int> pos = find_all_names_with_string_no_delete(drt_to, ref_to);
	vector<DrtPred> new_preds = get_all_elements_with_pos(drt_to, pos);
	substitute_ref(new_preds, ref_to, ref_from);
	to_return.insert(to_return.end(), new_preds.begin(), new_preds.end());

	return to_return;
}

double Anaphora::getLikeliness(const drt &drt_to, const drt &drt_from, const string &ref_to, const string &ref_from, bool is_plural)
{
	double w = 0;

	DrtVect pred_from = drt_from.predicates_with_references();
	DrtVect pred_to = drt_to.predicates_with_references();

	if (is_subject_of_verb(pred_from, ref_from) && is_subject_of_verb(pred_to, ref_to))
		w += 1;
	if (is_object_of_verb(pred_from, ref_from) && is_object_of_verb(pred_to, ref_to))
		w += 1;

	int m = find_element_with_string(pred_to,ref_to);
	if(m != -1 && pred_to.at(m).is_PRP() )
		w -= 0.01;

	if( is_plural )
		w += 0.1;

	int m2 = find_element_with_string(pred_from,ref_from);
	if(m2 != -1 && extract_header(pred_from.at(m2) ) == "[place]_there")
		w += 0.1;

	// DrtVect pred_to_rate = get_drtvect_with_switced_elements(pred_to, pred_from, ref_to, ref_from);

	//// This is too slow now!!
	// insert when you implement a fast Knowledge in the rating sentences
	//w += sense_->rate(pred_to_rate);
	////

	return w;
}

static bool is_connected_to_proper_name(const DrtVect &predicates, const string fref)
{
	vector<int> poz = find_all_names_with_string_no_delete(predicates, fref);
	if (poz.size() == 0)
		return false;
	metric *d = metric_singleton::get_metric_instance();
	for (int m = 0; m < poz.size(); ++m) {
		int j = poz.at(m);
		string header = extract_header(predicates.at(j));
		if (predicates.at(j).is_proper_name() && d->gender_proper_name(header) != "")
			return true;
	}

	return false;
}

static bool is_generic(const string &str)
{
	if (str.find("[*]") != string::npos)
		return true;
	return false;
}

static bool pred_is_singular(vector<DrtPred> speech, DrtPred pred)
// returns true if the pred in "pred" is compatible with a singular subject
{
	metric *d = metric_singleton::get_metric_instance();
	tagger *tagg = parser_singleton::get_tagger_instance();
	if (pred.is_name()) {
		string subj_ref = extract_first_tag(pred);
		for (int n = 0; n < speech.size(); ++n) {
			if (speech.at(n).is_name() && !speech.at(n).is_PRP()) {
				string fref = extract_first_tag(speech.at(n));
				string header = extract_header(speech.at(n));
				if (header == "specie")
					return false;
				string tag = speech.at(n).tag();
				string lemma = tagg->get_info()->get_conj(header, "NNS");
				if (lemma == "")
					lemma = header;
				if (fref == subj_ref && (d->has_synset(lemma) || is_generic(header)) && speech.at(n).is_plural()) {
					return false;
				}
			}
			if (speech.at(n).is_PRP()) {
				string fref = extract_first_tag(speech.at(n));
				string header = extract_header(speech.at(n));

				if (fref == subj_ref && (header == "he" || header == "she" || header == "it" || header == "i")) {
					return true;
				}
				if (fref == subj_ref && (header == "they" || header == "we")) {
					return true;
				}
			}
			if (speech.at(n).is_complement()) {
				string fref = extract_first_tag(speech.at(n));
				string head = extract_header(speech.at(n));
				if (fref == subj_ref && (head == "@AND" || head == "@OR"))
					return false;
			}
		}
	}

	return true;
}

static int find_complement_with_first_tag(const vector<DrtPred> &pre_drt, string ref, const string &head)
{
	for (int n = 0; n < pre_drt.size(); ++n) {
		string head_tmp = extract_header(pre_drt.at(n));
		if (head != "" && head != head_tmp)
			continue;
		string sref = extract_first_tag(pre_drt.at(n));
		if (pre_drt.at(n).is_complement() && sref == ref) {
			return n;
		}
	}
	return -1;
}


static string find_quantity(const DrtVect &pred, const string &ref)
{
	string to_return= "";

	int m= find_complement_with_first_tag(pred,ref,"@QUANTITY");
	if( m != -1) {
		string sref = extract_second_tag(pred.at(m));
		int m2= find_name_with_string(pred,sref);
		if( m2 != -1) {
			to_return = extract_header(pred.at(m2));
		}
	}

	return to_return;
}

static bool specifications_are_compatible(const DrtVect &pred, const DrtVect &pred_from, const string &ref, const string &ref_from)
{

	string q1= find_quantity(pred, ref);
	string q2= find_quantity(pred_from, ref_from);
	if (q1 == q2)
		return true;
	return false;
}


vector<pair<string, string> > Anaphora::getInstantiationWithPreds(const drt &drt_sample,
		                     const drt &drt_from, vector<DrtPred> &ref, vector<double> &likelinesses )
//returns the pair <new_reference, old_reference>
{
	vector<pair<string, string> > ret_inst(ref.size());
	vector<double> separations(ref.size(), 0);

	metric *d = metric_singleton::get_metric_instance();
	vector<DrtPred> predicates(drt_sample.predicates_with_references());
	vector<DrtPred> predicates_from(drt_from.predicates_with_references());
	vector<string> already_assigned(ref.size(), "");

	vector<string> strs;
	for (int m = 0; m < ref.size(); ++m) {
		if (!ref.at(m).is_pivot())
			continue;
		string pred_head_str = extract_header(ref.at(m));
		string pred_child_str = extract_first_tag(ref.at(m));
		for (int n = 0; n < predicates.size(); ++n) {
			if (!predicates.at(n).is_pivot())
				continue;
			string head_str = extract_header(predicates.at(n));
			string child_str = extract_first_tag(predicates.at(n));

			if( !specifications_are_compatible(predicates, predicates_from, child_str, pred_child_str) )
				continue;

			if (((child_str.find("name") != string::npos && !predicates.at(n).is_adjective())
					|| predicates.at(n).is_proper_name() || predicates.at(n).is_PRP() || predicates.at(n).is_plural())
					&& !(pred_child_str.find("name") != string::npos && ref.at(m).is_plural())) {
				string anaphora_level = ref.at(m).anaphoraLevel();
				vector<string> ref_str = get_reference_string(pred_head_str);
				if (ref.at(m).is_proper_name() && !predicates.at(n).is_proper_name())
					continue; // "Rome" cannot do anaphora with "home"

				if (ref.at(m).is_proper_name() && predicates.at(n).is_proper_name()
						&& !contained_NNP(pred_head_str, head_str))
					continue;

				if (ref.at(m).is_proper_name() && d->gender_proper_name(pred_head_str) != "") { ///
					ref_str.push_back("person");
					ref_str.push_back("human_being");
				}
				// things are not symmetric because you can say "Henrik is at home. He is happy" but not "he is at home. Henrik is happy"
				if (!ref.at(m).is_proper_name() && !ref.at(m).is_PRP() && predicates.at(n).is_proper_name())
					continue; // "Rome" cannot do anaphora with "home"
				if (is_connected_to_proper_name(predicates, child_str)
				//predicates.at(n).is_proper_name()
				// && d->gender_proper_name(head_str) != ""
						) {  ///
					head_str += "|person";
					head_str += "|human_being";
				}

				vector<string>::iterator assigned_iter = find(already_assigned.begin(), already_assigned.end(),
						pred_child_str);
				if (assigned_iter == already_assigned.end() || assigned_iter == already_assigned.begin() + m) {
					double dist = get_string_vector_distance(d, head_str, ref_str);
					bool is_plural= false;
					if (!pred_is_singular(predicates, predicates.at(n))
						&& (pred_head_str == "they" || pred_head_str == "them")) {
						dist = 1; // associate they to plurals
						is_plural = true;
					}
					double likeliness = getLikeliness(predicates, predicates_from, child_str, pred_child_str, is_plural);
					if (predicates.at(n).is_plural() && !ref.at(m).is_plural())
						dist = 0; // only a plural can link to another plural
					if (predicates.at(n).is_PRP() && ref.at(m).is_PRP() && pred_head_str == head_str)
						dist = 1; // associate they to plurals
					if (dist > 0.4 && dist > separations.at(m) && likeliness > likelinesses.at(m) // only what makes sense is instantiated (likeliness == 0 is not considered)
							) {
						if (anaphora_level.find(predicates.at(n).anaphoraLevel()) != string::npos) {
							separations.at(m) = dist;
							likelinesses.at(m) = likeliness;
							already_assigned.at(m) = pred_child_str;
							ret_inst.at(m) = make_pair(child_str, pred_child_str); // child_str is the new ref that overwrites pred_child_str
						}
					}
				}
			}
		}
	}
	ret_inst = erase_empty_ref(ret_inst);

	return ret_inst;
}

vector<References> Anaphora::getReferences()
{
	vector<References> phrase_references(drt_collection_.size());

	vector<drt>::const_iterator drtiter = drt_collection_.begin();
	vector<drt>::const_iterator drtend = drt_collection_.end();

	if (drtiter == drtend)
		return phrase_references;

	vector<DrtPred> tmp = drtiter->predicates();

	// Give a different number to different phrases, so that tags
	// cannot overlap
	vector<vector<DrtPred> > references;
	for (int n = 0; drtiter != drtend; ++drtiter, ++n) {
		vector<DrtPred> tmp_drt = drtiter->predicates();
		//print_vector(tmp_drt);
		// Find the references
		vector<DrtPred> tmp_refs = drtiter->get_references_with_preds();
		references.push_back(tmp_refs);
	}

	// Separates the drt in different connections: only the references
	// within these connections are instantiated
	vector<pair<int, int> > connected;
	for (int n = 1; n < drt_collection_.size(); ++n) {
		vector<pair<int, int> > tmp_connection = get_related_phrases(n); // find the phrases related to n
		connected.insert(connected.end(), tmp_connection.begin(), tmp_connection.end());
	}
	// Do the instantiation and solve the donkey phrases
	phrase_references.resize(drt_collection_.size());
	if(drt_collection_.size())
		phrase_references.at(0).insert(phrase_references.at(0).end(),internal_refs_.begin(), internal_refs_.end());

	vector<pair<string, string> > prev_ref;

	vector<double> likelinesses;
	int phrase_from_old = -1;
	for (int n = 0; n < connected.size(); ++n) {
		//for (int n = connected.size()-1; n >= 0; --n) { // the closest sentence first
		int phrase_from = connected.at(n).first;
		int phrase_to = connected.at(n).second;
		vector<DrtPred> ref = references.at(phrase_from);
		drt tmp_drt_from = drt_collection_.at(phrase_from);
		drt tmp_drt_to = drt_collection_.at(phrase_to);
		tmp_drt_to.set_references(prev_ref);
		if( phrase_from != phrase_from_old) {
			likelinesses = vector<double>(ref.size(), 0);
			phrase_from_old = phrase_from;
		}
		vector<pair<string, string> > tmp_inst = this->getInstantiationWithPreds(tmp_drt_to, tmp_drt_from, ref, likelinesses);

		if (tmp_inst.size())
			phrase_references.at(phrase_from).insert(phrase_references.at(phrase_from).begin(), tmp_inst.begin(),
					tmp_inst.end());
		prev_ref.insert(prev_ref.begin(), tmp_inst.begin(), tmp_inst.end());
	}
	return phrase_references;
}

static vector<pair<string, string> > get_donkey_instantiation_with_preds(const drt &drt_sample, vector<string> &ref)
{
	vector<pair<string, string> > ret_inst(ref.size());
	vector<double> separations(ref.size(), 0);
	metric *d = metric_singleton::get_metric_instance();
	vector<DrtPred> predicates(drt_sample.predicates_with_references());
	vector<string> already_assigned(ref.size(), "");

	for (int m = 0; m < ref.size(); ++m) {
		for (int n = 0; n < predicates.size(); ++n) {
			if (!predicates.at(n).is_pivot())
				continue;
			string head_str = extract_header(predicates.at(n));
			string child_str = extract_first_tag(predicates.at(n));

			if ((child_str.find("name") != string::npos && !predicates.at(n).is_adjective())
					|| predicates.at(n).is_proper_name() || predicates.at(n).is_PRP()) {
				string pred_head_str = ref.at(m);
				pred_head_str = cut_numbers(pred_head_str);
				vector<string> ref_str = get_reference_string(pred_head_str);


				if (predicates.at(n).is_proper_name() && d->gender_proper_name(head_str) != "") {  ///
					head_str += "|person";
					head_str += "|human_being";
				}
				vector<string>::iterator assigned_iter = find(already_assigned.begin(), already_assigned.end(), child_str);
				if (assigned_iter == already_assigned.end() || assigned_iter == already_assigned.begin() + m) {
					double dist = get_string_vector_distance(d, head_str, ref_str);
					if (!pred_is_singular(predicates, predicates.at(n)) && pred_head_str == "plural")
						dist = 1; // associate they to plurals
					if (dist > 0.8 && dist > separations.at(m)) {
						separations.at(m) = dist;
						already_assigned.at(m) = child_str;
						ret_inst.at(m) = make_pair(child_str, ref.at(m));
					}
				}
			}
		}
	}

	ret_inst = erase_empty_ref(ret_inst);
	return ret_inst;
}

vector<References> Anaphora::getDonkeyReferences()
{
	vector<References> phrase_references(drt_collection_.size());


	if (drt_collection_.size() == 0)
		return phrase_references;

	// Resolve the ambiguity in the first sentence
	if (drt_collection_.at(0).has_donkey()) {
		vector<pair<string, string> > donkey_refs = drt_collection_.at(0).get_donkey_instantiation();
		drt_collection_.at(0).add_references(donkey_refs);
		phrase_references.at(0).insert(phrase_references.at(0).end(), donkey_refs.begin(), donkey_refs.end());
	}

	// Separates the drt in different connections: only the references
	// within these connections are instantiated
	vector<pair<int, int> > connected;
	for (int n = 1; n < drt_collection_.size(); ++n) {
		vector<pair<int, int> > tmp_connection = get_related_phrases(n); // find the phrases related to n
		connected.insert(connected.end(), tmp_connection.begin(), tmp_connection.end());

	}

	vector<pair<string, string> > own_refs;
	for (int n = 0; n < connected.size(); ++n) {
		int phrase_from = connected.at(n).first;
		int phrase_to = connected.at(n).second;

		if (drt_collection_.at(phrase_from).has_donkey()) {
			vector<string> donkeys = drt_collection_.at(phrase_from).get_donkey();
			drt tmp_drt(drt_collection_.at(phrase_to));
			vector<pair<string, string> > donkey_refs = get_donkey_instantiation_with_preds(tmp_drt, donkeys);
			if (donkey_refs.size() == 0) {
				donkey_refs = drt_collection_.at(phrase_from).get_donkey_instantiation();
			}
			if (donkey_refs.size() > 0) {
				//drt_collection_.at(phrase_from).add_references(donkey_refs);
				phrase_references.at(phrase_from).insert(phrase_references.at(phrase_from).end(), donkey_refs.begin(),
						donkey_refs.end());
			}
		}
	}

	return phrase_references;
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


static DrtVect define_subject(DrtVect drtvect, References *internal_refs)
// the subject in the first sentence are all new (not given) => ref -> name
{
	for(int n=0; n < drtvect.size(); ++n) {
		string fref= extract_first_tag(drtvect.at(n));
		if(ref_is_ref(fref) && drtvect.at(n).is_name() && is_subject_of_verb(drtvect,fref)) {
			int ref_pos= fref.find("ref") + 3;
			if(ref_pos == -1)
				continue;
			string new_ref = (string)"name" + fref.substr(ref_pos,fref.size() );
			drtvect= substitute_ref_safe(drtvect,fref,new_ref);
			internal_refs->push_back(make_pair(new_ref,fref) );
			break; // only the first subject is transformed
		}
	}
	return drtvect;
}

static vector<drt> handle_first_sentence(vector<drt> drt_collection_, References *internal_refs)
{
	if(drt_collection_.size() == 0)
		return drt_collection_;

	drt first_drt = drt_collection_.at(0);
	DrtVect drtvect= first_drt.predicates_with_references();

	drtvect = define_subject(drtvect, internal_refs);
	first_drt.setPredicates(drtvect);
	drt_collection_.at(0) = first_drt;

	return drt_collection_;
}

Anaphora::Anaphora(const vector<drt> dc, PhraseInfo *pi) :
		drt_collection_(dc), pi_(pi)
{
	sense_ = pi_->getSense();

	drt_collection_ = handle_first_sentence(drt_collection_, &internal_refs_);
}

Anaphora::~Anaphora()
{
}

static vector<DrtPred> get_NNP_list(const DrtVect &drtvect)
{
	vector<DrtPred> to_return;
	for (int n = 0; n < drtvect.size(); ++n) {
		if (drtvect.at(n).is_proper_name())
			to_return.push_back(drtvect.at(n));
	}
	return to_return;
}

static References insert_NNP_map_and_get_references(map<string, DrtPred> &NNP_map, const vector<DrtPred> &NNP_list)
{
	References references;
	for (int n = 0; n < NNP_list.size(); ++n) {
		string header = extract_header(NNP_list.at(n));
		string fref = extract_first_tag(NNP_list.at(n));
		vector<string> strs;
		boost::split(strs, header, boost::is_any_of("_"));
		for (int m = 0; m < strs.size(); ++m) {
			string noun = strs.at(m);
			map<string, DrtPred>::iterator miter = NNP_map.find(noun);
			if (miter == NNP_map.end())
				NNP_map[noun] = NNP_list.at(n);
			else {
				string old_ref = extract_first_tag(miter->second);
				references.push_back(make_pair(fref, old_ref));
			}
		}
	}
	return references;
}

static void insert_NNP_map(map<string, pair<DrtPred, DrtVect> > &NNP_map, const vector<DrtPred> &NNP_list, const DrtVect &drtvect)
{
	for (int n = 0; n < NNP_list.size(); ++n) {
		string header = extract_header(NNP_list.at(n));
		string fref = extract_first_tag(NNP_list.at(n));
		vector<string> strs;
		boost::split(strs, header, boost::is_any_of("_"));
		for (int m = 0; m < strs.size(); ++m) {
			string noun = strs.at(m);
			map<string, pair<DrtPred,DrtVect> >::iterator miter = NNP_map.find(noun);
			if (miter == NNP_map.end())
				NNP_map[noun] = make_pair(NNP_list.at(n),drtvect);
			else {
				string old_ref = extract_first_tag(miter->second.first);
			}
		}
	}
}

string find_first_NNP_ref(const vector<drt> &drt_collection)
{
	vector<drt>::const_iterator drtiter = drt_collection.begin();
	vector<drt>::const_iterator drtend = drt_collection.end();

	string to_return = "";

	metric *d = metric_singleton::get_metric_instance();
	for (; drtiter != drtend; ++drtiter) {
		DrtVect drtvect = drtiter->predicates_with_references();
		for (int n = 0; n < drtvect.size(); ++n) {
			if (drtvect.at(n).is_proper_name()) {
				string header = extract_header(drtvect.at(n));
				if (d->gender_proper_name(header) != "") {
					to_return = extract_first_tag(drtvect.at(n));
					return to_return;
				}
			}
		}
	}

	return to_return;
}

vector<References> Anaphora::getUninstantiated()
{
	vector<References> to_return(drt_collection_.size());
	metric *d= metric_singleton::get_metric_instance();

	map<string, pair<DrtPred, DrtVect> > NNP_map;
	vector<drt>::const_iterator drtiter = drt_collection_.begin();
	vector<drt>::const_iterator drtend = drt_collection_.end();
	int n = 0;
	for (; drtiter != drtend; ++drtiter, ++n) {
		DrtVect drtvect = drtiter->predicates_with_references();
		vector<DrtPred> NNP_list = get_NNP_list(drtvect);
		insert_NNP_map(NNP_map, NNP_list, drtvect);
	}

	// get the first NNP of a person
	string first_NNP_ref = find_first_NNP_ref(drt_collection_);
	drtiter = drt_collection_.begin();
	drtend = drt_collection_.end();
	n = 0;
	// assign the first NNP ref to all the PRP that are not instantiated
	for (; drtiter != drtend; ++drtiter, ++n) {
		DrtVect drtvect = drtiter->predicates_with_references();
		for (int m = 0; m < drtvect.size(); ++m) {
			string tag = drtvect.at(m).tag();
			string fref = extract_first_tag(drtvect.at(m));
			string header = extract_header(drtvect.at(m));

			if (tag == "PRP" && (header == "he" || header == "she") && ref_is_ref(fref)) {
				if(first_NNP_ref != "")
					to_return.at(n).push_back(make_pair(first_NNP_ref, fref));
			}
		}
	}

	drtiter = drt_collection_.begin();
	drtend = drt_collection_.end();
	n = 0;
	// assign the first NNP ref to all the NNP with the same name
	for (; drtiter != drtend; ++drtiter, ++n) {
		DrtVect drtvect = drtiter->predicates_with_references();
		for (int m = 0; m < drtvect.size(); ++m) {
			string tag = drtvect.at(m).tag();
			string fref = extract_first_tag(drtvect.at(m));
			string header = extract_header(drtvect.at(m));
			map<string, pair<DrtPred,DrtVect> >::iterator miter = NNP_map.find(header);
			if (tag == "NNP" && miter != NNP_map.end()
			    // && d->gender_proper_name(header) != "" // must be a person
			) {
				DrtVect drtvect_NNP = miter->second.second;
				string fref_NNP= extract_first_tag(miter->second.first);
				if ( !specifications_are_compatible(drtvect_NNP, drtvect, fref_NNP, fref) )
					continue;
				string NNP_ref = fref_NNP;
				if (fref != NNP_ref && NNP_ref != "")
					to_return.at(n).push_back(make_pair(NNP_ref, fref));
			}
		}
	}

	return to_return;
}
