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



#include"drt_builder.hpp"
#include"tools.hpp"
#include"drt_builder-corrector.hpp"
#include"phrase.hpp"

const bool debug = false;
const bool measure_time = false;

template<class T>
void print_vector(std::vector<T> &vs)
{
	typename vector<T>::iterator tags_iter = vs.begin();
	while (tags_iter < vs.end()) {
		std::cout << (*tags_iter) << " ";
		++tags_iter;
	}
	std::cout << std::endl;
}
template<class T>
bool shortfind(const vector<T> &vect, const T &element)
{
	if (find(vect.begin(), vect.end(), element) == vect.end())
		return false;
	return true;
}


static vector<DrtPred> get_drt_from_names(const vector<string> &names, const vector<string> &tags)
{
	vector<DrtPred> to_return;
	vector<string>::const_iterator siter = names.begin();
	int num = 0, n;
	for (n = 0; siter != names.end() && n < tags.size(); ++siter, ++n) {
		PredTree pred = Predicate(*siter).pred();
		if (is_verb(tags.at(n)) || is_modal(tags.at(n))) {
			pred.appendChild(pred.begin(), string("verb") + boost::lexical_cast<string>(n));
			pred.appendChild(pred.begin(), string("subj") + boost::lexical_cast<string>(n));
			pred.appendChild(pred.begin(), string("obj") + boost::lexical_cast<string>(n));
		} else if (is_preposition(tags.at(n))) {
			pred.appendChild(pred.begin(), string("from") + boost::lexical_cast<string>(n));
			pred.appendChild(pred.begin(), string("to") + boost::lexical_cast<string>(n));
		} else if (is_conj(tags.at(n))) {
			pred.appendChild(pred.begin(), string("prev") + boost::lexical_cast<string>(n));
			pred.appendChild(pred.begin(), string("next") + boost::lexical_cast<string>(n));
		} else if (is_WRB(tags.at(n))) {
			pred.appendChild(pred.begin(), string("prev") + boost::lexical_cast<string>(n));
			pred.appendChild(pred.begin(), string("next") + boost::lexical_cast<string>(n));
		} else if (*siter == "this" || *siter == "that" || *siter == "these" || *siter == "those" || is_PRP(tags.at(n)))
			pred.appendChild(pred.begin(), string("ref") + boost::lexical_cast<string>(n));
		else
			pred.appendChild(pred.begin(), string("name") + boost::lexical_cast<string>(n));

		DrtPred drt_tmp(pred);
		drt_tmp.setTag(tags.at(n));
		drt_tmp.setName(names.at(n));
		drt_tmp.setAnaphoraLevel("base");

		to_return.push_back(drt_tmp);

		// Loading tags and names into the drt elements
	}
	return to_return;
}

DrtVect set_all_anaphora_levels(DrtVect drtvect)
{
	for (int n = 0; n < drtvect.size(); ++n) {
		drtvect.at(n).setAnaphoraLevel("base");
	}
	return drtvect;
}

double get_siblings_distance(const DrtPred &pred, int pos)
{
	vector<string> children = pred.extract_children();

	double tot_distance = 0;
	int n;
	for (n = 0; n < children.size(); ++n) {
		double tmp_dist = fabs(get_single_distance(children.at(n)) - n);
		tot_distance += tmp_dist;
	}
	return tot_distance;
}

static vector<DrtPred> process_adjectives(vector<DrtPred> names, int pos1, int pos2)
{
	string str1 = extract_header(names.at(pos1));
	string str2 = extract_header(names.at(pos2));
	string str1child = extract_first_tag(names.at(pos1));
	string str2child = extract_first_tag(names.at(pos2));

	implant_first(names.at(pos2), str1child);

	return names;
}

bool there_are_only_names_between(vector<string> tags, int pos1, int pos2)
{
	int start, end;

	if (pos1 < pos2) {
		start = pos1;
		end = pos2;
	} else {
		start = pos2;
		end = pos1;
	}
	for (int n = start; n <= end; ++n) {
		if (tags.at(n) == "-LBR-" || tags.at(n) == "-RBR-")
			continue;
		if (!is_name(tags.at(n)) && !is_adjective(tags.at(n)) && tags.at(n) != "RBS" // second most voted person
				)
			return false;
	}
	return true;
}

bool only_one_RB_between(vector<string> tags, int pos1, int pos2)
{
	int start, end;

	if (pos1 < pos2) {
		start = pos1;
		end = pos2;
	} else {
		start = pos2;
		end = pos1;
	}
	int num_RB = 0;
	for (int n = start; n <= end; ++n) {
		if (tags.at(n) == "RB")
			num_RB++;
		if (tags.at(n) != "RB" || num_RB > 1)
			return false;
	}
	return true;
}

bool there_are_only_names_between(DrtVect pre_drt, int pos1, int pos2)
{
	int start, end;

	if (pos1 < pos2) {
		start = pos1;
		end = pos2;
	} else {
		start = pos2;
		end = pos1;
	}
	for (int n = start; n <= end; ++n) {
		if (pre_drt.at(n).tag() == "-LBR-" || pre_drt.at(n).tag() == "-RBR-")
			continue;
		if (!pre_drt.at(n).is_name() && !pre_drt.at(n).is_adjective() && pre_drt.at(n).tag() != "RBS" // second most voted person
				)
			return false;
	}
	return true;
}

static vector<DrtPred> process_names(vector<DrtPred> names, int pos1, int pos2)
{
	//vector<DrtPred> to_return;
	string str1 = extract_header(names.at(pos1));
	string str2 = extract_header(names.at(pos2));
	string str1child = extract_first_tag(names.at(pos1)); //.pred().begin().node->firstChild->data.str;
	string str2child = extract_first_tag(names.at(pos2)); // .pred().begin().node->firstChild->data.str;

	if (str1child.find("ref") == string::npos) { // if the pointed name is not already referenced by an article
		DrtPred tmp_drt(str1 + "(" + str2child + ")");
		tmp_drt.setName(names.at(pos1).name());
		tmp_drt.setTag(names.at(pos1).tag());
		tmp_drt.set_pivot(names.at(pos1).is_pivot());
		names.at(pos1) = tmp_drt;
		if(debug) {
			cout << "PROCESS_NAMES::: " << str2child << " " << str1child << endl;
		}
		names= substitute_ref(names,str1child,str2child);
	} else {
		DrtPred tmp_drt(str2 + "(" + str1child + ")");
		tmp_drt.setName(names.at(pos2).name());
		tmp_drt.setTag(names.at(pos2).tag());
		tmp_drt.set_pivot(names.at(pos2).is_pivot());
		names.at(pos2) = tmp_drt;
	}

	return names;
}

static vector<DrtPred> process_articles(vector<DrtPred> names, int pos1, int pos2)
{
	//vector<DrtPred> to_return;
	string str1 = extract_header(names.at(pos1));
	string str2 = extract_header(names.at(pos2));
	string str1child = extract_first_tag(names.at(pos1));
	string str2child = extract_first_tag(names.at(pos2));

	int position = get_single_distance(str2child);

	if (str1 == "the" || str1 == "this" || str1 == "that" || str1 == "these" || str1 == "those") {
		implant_first(names.at(pos2), "ref" + boost::lexical_cast<string>(position));
		if (get_single_distance(str1child) == pos1) // change it only if it has not been changed before
			implant_first(names.at(pos1), "ref" + boost::lexical_cast<string>(position));
		return names;
	} else if (str1 == "a" || str1 == "an" || str1 == "some" || str1 == "no" || str1 == "both" || str1 == "either"
			|| str1 == "all" || str1 == "each" || str1 == "another" || str1 == "some" || str1 == "any" || str1 == "every") {
		implant_first(names.at(pos2), "name" + boost::lexical_cast<string>(position));
		if (get_single_distance(str1child) == pos1) // change it only if it has not been changed before
			implant_first(names.at(pos1), "name" + boost::lexical_cast<string>(position));
		return names;
	}

	return names;
}

static vector<DrtPred> process_adverb(vector<DrtPred> names, int pos1, int pos2)
{
	string str1 = extract_header(names.at(pos1));
	string str2 = extract_header(names.at(pos2));
	string subj1 = extract_first_tag(names.at(pos1));
	string subj2 = extract_first_tag(names.at(pos2));

	double distance1 = fabs(pos1 - pos2);
	double distance2 = fabs(get_single_distance(subj2) - pos2);
	if (distance2 != 0 && distance2 < distance1)
		return names;

	implant_first(names.at(pos2), subj1);
	return names;
}

static vector<DrtPred> process_comparative_adverb(vector<DrtPred> names, int pos1, int pos2)
// RBR
{
	string str1 = extract_header(names.at(pos1));
	string str2 = extract_header(names.at(pos2));
	string subj1 = extract_first_tag(names.at(pos1));
	string subj2 = extract_first_tag(names.at(pos2));

	//std::cout << str1 << " " << str2 << std::endl;
	//double distance1= fabs(get_single_distance(subj1)-pos2);
	double distance1 = fabs(pos1 - pos2);
	double distance2 = fabs(get_single_distance(subj2) - pos2);
	if (distance2 != 0 && distance2 < distance1)
		return names;

	if (ref_is_name(subj2) && ref_is_verb(subj1))
		return names;

	implant_first(names.at(pos2), subj1);
	return names;
}

static vector<DrtPred> add_subject(vector<DrtPred> names, int pos1, int pos2)
{
	//vector<DrtPred> to_return;
	string str1 = extract_header(names.at(pos1));
	string str2 = extract_header(names.at(pos2));
	string subj1 = extract_first_tag(names.at(pos1));
	string subj2 = extract_subject(names.at(pos2));

	double distance1 = fabs(pos1 - pos2);
	double distance2 = fabs(get_single_distance(subj2) - pos2);
	if (distance2 != 0 && distance1 > distance2)
		return names;

	implant_subject(names.at(pos2), subj1);

	return names;
}

static vector<DrtPred> add_object(vector<DrtPred> names, int pos1, int pos2)
{
	//vector<DrtPred> to_return;
	string str1 = extract_header(names.at(pos1));
	string str2 = extract_header(names.at(pos2));
	string subj1 = extract_first_tag(names.at(pos1));
	string subj2 = extract_third_tag(names.at(pos2));

	//double distance1= fabs(get_single_distance(subj1)-pos2);
	double distance1 = fabs(pos1 - pos2);
	double distance2 = fabs(get_single_distance(subj2) - pos2);
	if (distance2 != 0 && distance1 > distance2)
		return names;

	implant_object(names.at(pos2), subj1);

	return names;
}

static vector<DrtPred> add_object_safe(vector<DrtPred> names, int pos1, int pos2)
{
	//vector<DrtPred> to_return;
	string str1 = extract_header(names.at(pos1));
	string str2 = extract_header(names.at(pos2));
	string subj1 = extract_first_tag(names.at(pos1));
	string subj2 = extract_third_tag(names.at(pos2));

	//double distance1= fabs(get_single_distance(subj1)-pos2);
	double distance1 = fabs(pos1 - pos2);
	double distance2 = fabs(get_single_distance(subj2) - pos2);
	if (distance2 != 0 && distance1 > distance2)
		return names;

	string sref = extract_subject(names.at(pos2));
	if (sref != subj1)
		implant_object(names.at(pos2), subj1);

	return names;
}

static vector<DrtPred> add_indirect_object(vector<DrtPred> names, int pos1, int pos2)
{
	//vector<DrtPred> to_return;
	string str1 = extract_header(names.at(pos1));
	string str2 = extract_header(names.at(pos2));
	string subj1 = extract_first_tag(names.at(pos1));
	string subj2 = extract_first_tag(names.at(pos2));

	DrtPred ind_obj(string("@DATIVE(") + subj1 + "," + subj2 + ")");
	ind_obj.setName("");
	names.push_back(ind_obj);

	return names;
}

static vector<DrtPred> process_single_adjective(vector<DrtPred> names, int pos1)
{
	//vector<DrtPred> to_return;
	string str1 = extract_header(names.at(pos1));

	implant_first(names.at(pos1), "adj_" + boost::lexical_cast<string>(pos1));

	return names;
}

static vector<DrtPred> process_preposition_from(vector<DrtPred> names, int pos1, int pos2)
{
	//vector<DrtPred> to_return;
	string str1 = extract_header(names.at(pos1));
	string str2 = extract_header(names.at(pos2));
	string subj1 = extract_first_tag(names.at(pos1));
	string last_subj1 = extract_second_tag(names.at(pos1));
	string subj2 = extract_first_tag(names.at(pos2));
	string last_subj2 = extract_second_tag(names.at(pos2));

	if (str1 == "a" || str1 == "an" || str1 == "the")
		return names;

	// If trying to implant the same reference do nothing
	if (subj1 == last_subj2)
		return names;

	bool element_was_specification_end = false;
	bool element_is_specification_end  = is_specification_end(names,pos2);

	int old_pos= find_element_with_string(names,subj2);

	if(old_pos != -1)
		element_was_specification_end = is_specification_end(names,old_pos);


	if ( names.at(pos2).tag() == "CC"
	    && !( element_is_specification_end && !element_was_specification_end)
	) {
		if(debug) {
			cout << "CC_PREV1::: " << subj2 << " " << last_subj2 << endl;
			cout << "CC_PREV2::: " << element_is_specification_end
					<< " "
					<< element_was_specification_end << endl;
		}
		return names;
	}


	// If the preposition already points to a verb and the new
	// candidate is name, do nothing (unless for IN(of), which always points to the closest name)
	// There is an exception for subjects. A preposition always prefer a subject over the verb
	// A "ref" is preferred to "verb"
	if (ref_is_verb(subj2) && !ref_is_verb(subj1) && !ref_is_ref(subj1) && str2 != "of" && str2 != "by") {
		int m = find_verb_with_string(names, subj2);
		if (m != -1) {
			string sref = extract_subject(names.at(m));
			if (sref != subj1)
				return names;
		}
	}

	double adistance1 = pos1 - pos2;
	double adistance2 = get_single_distance(subj2) - pos2;
	if (adistance1 > 0 && adistance2 < 0 && ref_is_verb(subj1) // a preposition pointing to the verb on the right is to be preferred
			) {
		implant_first(names.at(pos2), subj1);
		return names;
	}

	double distance1 = fabs(pos1 - pos2);
	double distance2 = fabs(get_single_distance(subj2) - pos2);
	if (distance2 != 0 && distance1 > distance2)
		return names;


	implant_first(names.at(pos2), subj1);

	return names;
}

static vector<DrtPred> process_preposition_from_preposition(vector<DrtPred> names, int pos1, int pos2)
// the preposition points to another preposition
{
	string str1 = extract_header(names.at(pos1));
	string str2 = extract_header(names.at(pos2));
	string subj1 = extract_first_tag(names.at(pos1));
	string last_subj1 = extract_second_tag(names.at(pos1));
	string subj2 = extract_first_tag(names.at(pos2));
	string last_subj2 = extract_second_tag(names.at(pos2));

	double distance1 = fabs(pos1 - pos2);
	double distance2 = fabs(get_single_distance(subj2) - pos2);

	bool element_was_specification_end = false;
	bool element_is_specification_end  = is_specification_end(names,pos2);

	int old_pos= find_element_with_string(names,subj2);

	if(old_pos != -1)
		element_was_specification_end = is_specification_end(names,old_pos);


	if ( names.at(pos2).tag() == "CC"
	    && !( element_is_specification_end && !element_was_specification_end)
	) {
		if(debug) {
			cout << "CC_PREV1::: " << subj2 << " " << last_subj2 << endl;
			cout << "CC_PREV2::: " << element_is_specification_end
					<< " "
					<< element_was_specification_end << endl;
		}
		return names;
	}



	if (distance2 != 0 && distance1 > distance2)
		return names;

	// If the preposition already points to a verb and the new
	// candidate is name, do nothing
	if (ref_is_verb(subj2) && ref_is_name(last_subj1) && str2 != "of") {
		return names;
	}

	// If trying to implant the same reference do nothing
	if (last_subj1 == last_subj2)
		return names;

	implant_first(names.at(pos2), last_subj1);

	return names;
}

static vector<DrtPred> process_preposition_to(vector<DrtPred> names, int pos1, int pos2)
{
	if(debug) {
		cout << "PREP_TO::: " << endl;
		print_vector(names);
	}

	string str1 = extract_header(names.at(pos1));
	string str2 = extract_header(names.at(pos2));
	string subj1 = extract_first_tag(names.at(pos1));
	string subj2 = extract_second_tag(names.at(pos2));
	string first_subj2 = extract_first_tag(names.at(pos2));

	if (str1 == "a" || str1 == "an" || str1 == "the")
		return names;

	double distance1 = fabs(pos1 - pos2);
	double distance2 = fabs(get_single_distance(subj2) - pos2);
	if ( distance2 != 0 && distance1 > distance2 )
		return names;

	// If trying to implant the same reference do nothing
	if (subj1 == first_subj2)
		return names;

	// Check that the preposition does not point to a WDT (WDT is preferrable to anything else)
	int m = find_element_with_string(names,subj2);
	if(m != -1 && names.at(m).is_WDT())
		return names;

	implant_second(names.at(pos2), subj1);

	return names;
}

static vector<DrtPred> process_WRB_from(vector<DrtPred> names, int pos1, int pos2)
{
	string str1 = extract_header(names.at(pos1));
	string str2 = extract_header(names.at(pos2));
	string subj1 = extract_first_tag(names.at(pos1));
	string subj2 = extract_first_tag(names.at(pos2));
	string subj22 = extract_second_tag(names.at(pos2));

	if (subj1 == subj22)
		return names;

	double distance1 = fabs(pos1 - pos2);
	double distance2 = fabs(get_single_distance(subj2) - pos2);
	if (distance2 != 0 && distance1 > distance2)
		return names;

	if (subj1.find("verb") != std::string::npos) {
		implant_first(names.at(pos2), subj1);
	}

	return names;
}

static vector<DrtPred> process_WRB_to(vector<DrtPred> names, int pos1, int pos2)
{
	string str1 = extract_header(names.at(pos1));
	string str2 = extract_header(names.at(pos2));
	string subj1 = extract_first_tag(names.at(pos1));
	string subj12 = extract_first_tag(names.at(pos2));
	string subj2 = extract_second_tag(names.at(pos2));

	if (subj1 == subj12)
		return names;

	if (ref_is_verb(subj1) && !ref_is_verb(subj2)) {
		implant_second(names.at(pos2), subj1);
		return names;
	}

	double distance1 = fabs(pos1 - pos2);
	double distance2 = fabs(get_single_distance(subj2) - pos2);
	if (distance2 != 0 && distance1 > distance2)
		return names;

	if (subj2.find("name") == std::string::npos || subj2.find("verb") == std::string::npos) {
		implant_second(names.at(pos2), subj1);
	}
	return names;
}

static vector<DrtPred> process_modal_from(vector<DrtPred> names, int pos1, int pos2)
{
	string str1 = extract_header(names.at(pos1));
	string str2 = extract_header(names.at(pos2));
	string subj1 = extract_first_tag(names.at(pos1));
	string subj2 = extract_first_tag(names.at(pos2));

	double distance1 = fabs(pos1 - pos2);
	double distance2 = fabs(get_single_distance(subj2) - pos2);
	if (distance2 != 0 && distance1 > distance2)
		return names;

	implant_first(names.at(pos2), subj1);

	return names;
}

static vector<DrtPred> process_modal_to(vector<DrtPred> names, int pos1, int pos2)
{
	string str1 = extract_header(names.at(pos1));
	string str2 = extract_header(names.at(pos2));
	string subj1 = extract_first_tag(names.at(pos1));
	string subj2 = extract_third_tag(names.at(pos2));

	double distance1 = fabs(pos1 - pos2);
	double distance2 = fabs(get_single_distance(subj2) - pos2);
	if (distance2 != 0 && distance1 > distance2)
		return names;
	implant_second(names.at(pos2), subj1);

	return names;
}

static vector<DrtPred> process_conj_prev(vector<DrtPred> names, int pos1, int pos2)
{
	string str1 = extract_header(names.at(pos1));
	string str2 = extract_header(names.at(pos2));
	string subj1 = extract_first_tag(names.at(pos1));
	string subj2 = extract_first_tag(names.at(pos2));
	string last_subj2 = extract_second_tag(names.at(pos2));

	double distance1 = fabs(pos1 - pos2);
	double distance2 = fabs(get_single_distance(subj2) - pos2);

	bool element_was_specification_end= false;
	bool element_is_specification_end = is_specification_end(names,pos2);

	int old_pos= find_element_with_string(names,subj2);

	if(old_pos != -1)
		element_was_specification_end = is_specification_end(names,old_pos);

	if(debug) {
		cout << "CONJ_PREV1::: " << subj2 << " " << last_subj2 << endl;
		cout << "CONJ_PREV2::: " << element_is_specification_end
			<< " "
			<< element_was_specification_end << endl;
	}

	if (distance2 != 0 && distance1 > distance2
	    && !( element_is_specification_end && !element_was_specification_end)
	)
		return names;

	// If trying to implant the same reference do nothing
	if (subj1 == last_subj2)
		return names;

	implant_first(names.at(pos2), subj1);

	return names;
}
static vector<DrtPred> process_conj_next(vector<DrtPred> names, int pos1, int pos2)
{
	string str1 = extract_header(names.at(pos1));
	string str2 = extract_header(names.at(pos2));
	string subj1 = extract_first_tag(names.at(pos1));
	string first_subj2 = extract_first_tag(names.at(pos2));
	string subj2 = extract_second_tag(names.at(pos2));

	double distance1 = fabs(pos1 - pos2);
	double distance2 = fabs(get_single_distance(subj2) - pos2);
	if (distance2 != 0 && distance1 > distance2)
		return names;

	// If trying to implant the same reference do nothing
	if (subj1 == first_subj2)
		return names;

	implant_second(names.at(pos2), subj1);

	return names;
}

static vector<DrtPred> sign_duplicates(vector<DrtPred> to_return)
{
	// erase duplicates
	DrtVect already_seen;
	for (int n = 0; n < to_return.size(); ++n) {
		if (find(already_seen.begin(), already_seen.end(), to_return.at(n)) != already_seen.end()) {
			add_header(to_return.at(n), ":DELETE");
			to_return.at(n).name() += ":DELETE";
		} else
			already_seen.push_back(to_return.at(n));
	}
	return to_return;
}

static vector<DrtPred> transform_all_verbs_to_vb(vector<DrtPred> to_return)
{
	for (int n = 0; n < to_return.size(); ++n) {
		string tag = to_return.at(n).tag();
		if (to_return.at(n).is_verb() && tag != "VB") {
			if(tag == "VBG") {
				string vref = extract_first_tag(to_return.at(n));
				int m = find_complement_with_target(to_return,vref,"@GENITIVE");
				if(m != -1) {
					string fref = extract_first_tag(to_return.at(m));
					if(!ref_is_verb(fref) )
						continue;
				}
			}
			to_return.at(n).setTag("VB");
		}
	}
	return to_return;
}

static vector<DrtPred> delete_irrelevant(vector<DrtPred> to_return)
{
	// erase duplicates
	DrtVect already_seen;
	for (int n = 0; n < to_return.size(); ++n) {
		if (find(already_seen.begin(), already_seen.end(), to_return.at(n)) != already_seen.end()) {
			to_return.erase(to_return.begin() + n);
			--n;
		} else
			already_seen.push_back(to_return.at(n));
	}

	vector<DrtPred>::iterator siter = to_return.begin();
	string name, tag;
	while (siter != to_return.end()) {
		name = extract_header(*siter);
		tag = siter->tag();
		if (name == "the" || name == "a" || name == "an" || name == "each" || name == "both" || name == "every" || name == "all"
				|| name == "another" || name == "some" || name == "every" || name == "either"
				|| name == "-period-" || name == "?" || name == "!"
				|| name == "-colon-" || name == "AUX" || name == "PASSIVE_AUX" || name == "PREP" || name == "but"
				|| name == "and" || name == "or" || name == "although" || name == "which" ///
				|| name == "-comma-" || name == "-LBR-" || name == "-RBR-" || name == "of" || name == "for" || name == "by"
				|| name == "in" || name == "\"" || name == "CONDITION" || name == "whose" || name == "CONJUNCTION"
				|| name == "DISJUNCTION" || name == "COORDINATION" || name == "-END-PRN-"
				|| name.find("[prn]") != string::npos || name.find(":DELETE") != string::npos || name == "nil" /// In case an invalid predicate is constructed
				|| tag == "EX" || tag == "WDT") {
			to_return.erase(siter);
			continue;
		}
		int pos = name.find(":POST_AUX");
		if (pos != string::npos) {
			implant_header(*siter, name.substr(0, pos));
		}
		pos = name.find(":PASSIVE_POST_AUX");
		if (pos != string::npos) {
			implant_header(*siter, name.substr(0, pos));
		}
		string fstr = extract_first_tag(*siter);
		++siter;
	}
	return to_return;
}

static vector<DrtPred> get_not_connected(const DrtVect &orig_drt, const vector<DrtVect> &connected)
{
	DrtVect not_connected, connected_list;

	for (int m = 0; m < connected.size(); ++m) {
		connected_list.insert(connected_list.end(), connected.at(m).begin(), connected.at(m).end());
	}
	for (int n = 0; n < orig_drt.size(); ++n) {
		if (!shortfind(connected_list, orig_drt.at(n))
				&& !orig_drt.at(n).is_complement()
				&& !orig_drt.at(n).is_preposition()
				&& orig_drt.at(n).tag() != "-comma-"
				&& orig_drt.at(n).tag() != "CC"
						//&& orig_drt.at(n).tag() != "EX"
				&& !orig_drt.at(n).is_parenthesis()
				&& !is_punctuation(orig_drt.at(n))
				&& !orig_drt.at(n).is_adverb()
				&& !orig_drt.at(n).is_article()
				&& !orig_drt.at(n).is_delete()
				&& !is_AUX(orig_drt.at(n) )
				) {
			not_connected.push_back(orig_drt.at(n));
		}
	}

	if (debug) {
		std::cout << "UNCONNECTED:: " << std::endl;
		print_vector(not_connected);
	}

	return not_connected;
}

static bool has_verb(const DrtVect &drtvect)
{
	for(int n = 0; n < drtvect.size(); ++n) {
		string fref = extract_first_tag(drtvect.at(n));
		if(drtvect.at(n).is_verb() )
			return true;
	}
	return false;
}


static bool is_singular_exception(const DrtVect &drt, int n)
{
	if (!drt.at(n).is_verb() )
		return false;

	vector<DrtPred> preds= find_subject_of_verb(drt,n);

	for (int m = 0; m < preds.size(); ++m) {
		string sheader = extract_header(preds.at(m) );
		if(sheader == "specie")
			return true;
	}

	return false;
}


static bool is_belief_verb(const DrtPred &pred)
{
	string header = extract_header(pred);
	header = header.substr(0,header.find(":") );

	vector<string> candidates;
	candidates.push_back("believe");
	candidates.push_back("estimate");
	candidates.push_back("consider");

	if(debug)
		cout << "BELIEF::: " << header << endl;

	if(shortfind(candidates,header))
		return true;

	return false;
}


static double compute_likeliness(vector<DrtPred> &drt, const vector<string> &tags, Sense *sense, DrtPred *error, bool is_question)
// The original drt form (the one before delete_irrelevant() ) must be fed to this function
{
	int size = drt.size();
	if (size == 0)
		return -1;

	for (int n = 0; n < size; ++n) {
		string head = extract_header(drt.at(n));
		if (debug) {
			cout << "RP::: " << drt.at(n).tag() << "," << head << endl;
		}
		if (drt.at(n).tag() == "RP" && head.find(":DELETE") == string::npos) {
			*error = (string) "lonely_RP" + "(" + boost::lexical_cast<string>(n) + ")";
			return -1; // isolated RP mean that the phrase is not parsed correctly
		}
		if (drt.at(n).tag() == "CC" && head.find(":DELETE") == string::npos) {
			string header = extract_header(drt.at(n));
			string fref = extract_first_tag(drt.at(n));
			if (header == "@AND" && ref_is_verb(fref)) {
				if (debug)
					cout << "UNPROCESSED_CC::: " << drt.at(n) << endl;
				*error = (string) "unprocessed_CC" + "(" + boost::lexical_cast<string>(n) + ")";
				return -1; // isolated RP mean that the phrase is not parsed correctly
			}
		}
		if (drt.at(n).is_complement() && !drt.at(n).is_WRB() && head.find(":DELETE") == string::npos
				) {
			string header = extract_header(drt.at(n));
			string tag = drt.at(n).tag();
			if(header == "@AND" || header == "@OR" || header == "@CONJUNCTION" || header == "@COORDINATION" || header == "@DISJUNCTION")
				continue;
			if(tag == "-comma-" || tag == "CC")
				continue;
			string sref   = extract_second_tag(drt.at(n));
			if (ref_is_verb(sref)) {
				int m = find_verb_with_string(drt, sref);
				if (m == -1)
					continue;
				if ((drt.at(n).name() == "to" || drt.at(n).name() == "that" || drt.at(n).name() == "if" || drt.at(n).name() == "whether" )
						&& drt.at(m).tag() == "VBG") {
					if (debug) {
						cout << "WRONG_TENSE_FOR_SUBORD1::: " << drt.at(n).tag() << " " << drt.at(n).name() << endl;
					}
					*error = DrtPred( (string) "wrong_tense_for_subordinate_VBG" + "(" + boost::lexical_cast<string>(n) + ")" );
					return -1; // of reaches/VBZ is not correct (of only support VBG)
				}
				if (drt.at(n).name() == "of"
						&& drt.at(m).tag() != "VBG") {
					if (debug) {
						cout << "WRONG_TENSE_FOR_SUBORD2::: " << drt.at(n).tag() << " " << drt.at(n).name() << endl;
					}
					*error = DrtPred( (string) "wrong_tense_for_subordinate_non-VBG" + "(" + boost::lexical_cast<string>(n) + ")" );
					return -1;
				}
			}
		}
		if (drt.at(n).is_complement() && !drt.at(n).is_delete() && head == "@TIME_BETWEEN") {
			string sref = extract_second_tag(drt.at(n));
			int num = num_elements_connected(drt, sref);
			bool has_plural = has_plural_ref(drt, sref);
			if (num != 2 && !has_plural) {
				if (debug) {
					cout << "TIME_BETWEEN_TAG::: " << drt.at(n).tag() << endl;
				}
				*error = DrtPred( (string) "unprocessed_BETWEEN" + "(" + boost::lexical_cast<string>(n) + ")" );
				return -1;
			}
		}
		if (drt.at(n).is_complement() && drt.at(n).tag() != "CC" && !drt.at(n).is_delete() ) {
			string fref = extract_first_tag(drt.at(n));
			string sref = extract_second_tag(drt.at(n));
			;
			if (!has_first_tag(fref)) {
				if (debug) {
					cout << "COMPLEMENT_WITH_NO_PRIOR::: " << drt.at(n).tag() << endl;
				}
				*error = DrtPred( (string) "complement_with_no_prior" + "(" + boost::lexical_cast<string>(n) + ")" );
				return -1;
			}
		}
		if (drt.at(n).tag() == "IN" && head != "PREP" && head != "CONDITION" && !drt.at(n).is_complement() && !drt.at(n).is_delete()) {
			if (debug)
				cout << "COMPLEMENT_TO_NOWHERE::: " << drt.at(n) << endl;
			*error = (string) "unprocessed_BETWEEN" + "(" + boost::lexical_cast<string>(n) + ")";
			return -1;
		}
		if (drt.at(n).is_complement() && head == "@SUBORD") {
			if (debug)
				cout << "SUBORD_TO_NAME::: " << drt.at(n) << endl;
			string sref = extract_second_tag(drt.at(n));
			if (!ref_is_verb(sref)) {
				*error = DrtPred( (string) "subordinate_points_to_name" + "(" + boost::lexical_cast<string>(n) + ")" );
				return -1;
			}
		}
		if (drt.at(n).is_modal() && !is_AUX(drt.at(n))) {
			if (debug)
				cout << "FREE_MODAL::: " << drt.at(n) << endl;
			*error = DrtPred( (string) "free_modal" + "(" + boost::lexical_cast<string>(n) + ")" );
			return -1;
		}
		if (drt.at(n).is_complement() && head == "@SUBORD") {
			if (debug)
				cout << "PASSIVE_WITH_SUBORD::: " << drt.at(n) << endl;
			string fref = extract_first_tag(drt.at(n));
			string sref = extract_second_tag(drt.at(n));
			int m1 = find_verb_with_string(drt, fref);
			int m2 = find_verb_with_string(drt, sref);

			if (m1 == -1 || m2 == -1)
				continue;
			if (!is_passive(drt.at(m1)) || is_belief_verb(drt.at(m1)) )
				continue;
			string o1 = extract_object(drt.at(m1));
			string s2 = extract_subject(drt.at(m2));
			if (o1 != s2) {
				*error = DrtPred( (string) "passive_with_subordinate" + "(" + boost::lexical_cast<string>(n) + ")" );
				return -1;
			}
		}
		if (drt.at(n).is_verb()) {
			if (debug)
				cout << "EQUAL_SUBJ_OBJ::: " << drt.at(n) << endl;
			string sref = extract_subject(drt.at(n));
			string oref = extract_object(drt.at(n));
			if (sref != "none" && oref != "none" && sref == oref) {
				*error = DrtPred( (string) "equal_subject_and_object" + "(" + boost::lexical_cast<string>(n) + ")" );
				return -1;
			}
		}
		if (drt.at(n).is_verb()) {
			string oref = extract_object(drt.at(n));
			int m = find_complement_with_target(drt, oref);
			if (m != -1 && extract_first_tag(drt.at(m)) == extract_first_tag(drt.at(n))
					&& extract_header(drt.at(m)) != "@OWN"
					&& extract_header(drt.at(m)).find(":DELETE") == string::npos
					&& !is_AUX(drt.at(n))
							) {
				if (debug)
					cout << "PREP_AND_OBJ::: " << drt.at(n) << " " << drt.at(m) << endl;
				*error = DrtPred( (string) "preposition_and_object" + "(" + boost::lexical_cast<string>(n) + ")" );
				return -1;
			}
		}
		if (drt.at(n).is_verb()) {
			string sref = extract_subject(drt.at(n));
			int m = find_complement_with_target(drt, sref);
			if (m != -1 && extract_first_tag(drt.at(m)) == extract_first_tag(drt.at(n))
					&& extract_header(drt.at(m)) != "@OWN"
					&& extract_header(drt.at(m)).find(":DELETE") == string::npos
					&& !is_AUX(drt.at(n))
			) {
				if (debug) {
					cout << "PREP_AND_SUBJ::: " << drt.at(n) << " " << drt.at(m) << endl;
				}
				*error = DrtPred( (string) "preposition_and_subject" + "(" + boost::lexical_cast<string>(n) + ")" );
				return -1;
			}
		}
		if (drt.at(n).is_verb() && is_lonely_verb(drt, n) && !is_AUX(drt.at(n)) )  {
			if (debug)
				cout << "LONELY_VERB::: " << drt.at(n) << endl;
			*error = DrtPred( (string) "lonely_verb" + "(" + boost::lexical_cast<string>(n) + ")" );
			return -1;
		}
		if (is_question && drt.at(n).is_WP() && is_lonely_WP(drt, n) && !drt.at(n).is_delete() )  {
			if (debug)
				cout << "LONELY_WP::: " << drt.at(n) << endl;
			*error = DrtPred( (string) "lonely_WP" + "(" + boost::lexical_cast<string>(n) + ")" );
			return -1;
		}
		DrtPred sbar_pred;
		if (drt.at(n).is_verb() && (drt.at(n).tag() == "VBZ" || drt.at(n).tag() == "VBP" || drt.at(n).tag() == "VBD")
				&& is_sbar_verb(drt, n, &sbar_pred) // a verb with VBZ should not be a subordinate (only VBG and VB are allowed)
						) {
			if (debug)
				cout << "DUMMY_SUBORD::: " << drt.at(n) << endl;
			if (sbar_pred.name() == "[dummy]") {
				*error = DrtPred( (string) "dummy_subordinate" + "(" + boost::lexical_cast<string>(n) + ")" );
				return -1;
			}
		}

		 // a double dative can exist if there are two recipients: "David gives apples to John and to Maria".
		if (drt.at(n).is_complement() && extract_header(drt.at(n)).find("@DATIVE") != string::npos
		) {
			string fref = extract_first_tag(drt.at(n));
			int m = find_verb_with_string(drt,fref);
			if (m == -1)
				continue;
			if(has_object(drt.at(m)))
				continue;
			vector<int> poz= find_all_compl_with_first_tag_no_delete(drt,tags,fref,"@DATIVE");
			if (debug)
				cout << "DOUBLE_DATIVE::: " << drt.at(n) << endl;
			if(poz.size() >1 ) {
				*error = DrtPred( (string) "double_dative" + "(" + boost::lexical_cast<string>(n) + ")" );
				return -1;
			}
		}
		if (drt.at(n).is_verb() && head == "be" && !has_object(drt.at(n)) // "to be" with no object and complements is not likely (except when it means "exist")
				&& !has_complements(drt, n)) {
			if (debug)
				cout << "BE_WITH_NO_OBJECT::: " << drt.at(n) << endl;
			*error = DrtPred( (string) "be_with_no_object" + "(" + boost::lexical_cast<string>(n) + ")" );
			return -1;
		}

		string fref = extract_first_tag(drt.at(n));
		string sref = extract_second_tag(drt.at(n));
		if (head == "\'s" && (!ref_is_name(sref) || !ref_is_name(fref))) {
			*error = DrtPred( (string) "wrong_POS" + "(" + boost::lexical_cast<string>(n) + ")" );
			return -1;
		}

		// check that the subject (plural, singular) matches the verb
		if (drt.at(n).is_verb() && !is_passive(drt.at(n))
				&& !string_is_subord(extract_subject(drt.at(n))) // for detached questions subordinates
				&& !is_singular_exception(drt,n)
				&& ((verb_is_singular(drt.at(n), drt) && drt.at(n).tag() == "VBP")
						|| (!verb_is_singular(drt.at(n), drt) && drt.at(n).tag() == "VBZ"))) {
			if (debug) {
				puts("SINGULAR::: ");
			}
			*error = DrtPred( (string) "wrong_tense_for_singular" + "(" + boost::lexical_cast<string>(n) + ")" );
			return -1;
		}

		// @BEFORE(A,A) is not accepted
		if (drt.at(n).is_complement()
			&& !drt.at(n).is_delete()
			&& extract_first_tag(drt.at(n)) == extract_second_tag(drt.at(n))
		) {
			if (debug) {
				puts("FLAT_COMPLEMENT::: ");
			}
			*error = DrtPred( (string) "flat_complement" + "(" + boost::lexical_cast<string>(n) + ")" );
			return -1;
		}

		// check that the subject (plural, singular) matches the verb
		if (drt.at(n).is_verb() && has_object(drt.at(n))) {
			if (debug) {
				puts("CC_OBJECT::: ");
			}
			string oref = extract_object(drt.at(n));
			vector<int> poz = find_all_names_with_string_no_delete(drt,oref);
			if(poz.size() == 0) {
				*error = DrtPred( (string) "CC_object" + "(" + boost::lexical_cast<string>(n) + ")" );
				return -1;
			}
		}

		// Unconnected nouns should not be there
		vector<DrtVect> conn = get_linked_drtvect_from_single_drtvect(drt);
		vector<DrtPred> nc   = get_not_connected(drt, conn);
		if ( nc.size() && has_verb(drt) ) {
			if (debug) {
				puts("UNCONNECTED::: ");
			}
			*error = DrtPred( (string) "unconnected_nouns" + "(" + boost::lexical_cast<string>(n) + ")" );
			return -1;
		}
	}

	double likeliness = sense->rate(drt);
	if (debug)
		cout << "LIKELINESS::" << likeliness << endl;

	return likeliness;
}

static vector<DrtPred> change_WDT(vector<DrtPred> &pre_drt, int pos_WDT, int pos_prep, int pos_name, int pos_verb)
{
	if (extract_header(pre_drt.at(pos_WDT)).find(":DELETE") != string::npos) // continue if it has already been processed
		return pre_drt;

	vector<DrtPred> to_return(pre_drt);

	if (pos_WDT == -1 || pos_verb == -1)
		return to_return;

	if (pos_name == -1 || extract_header(pre_drt.at(pos_WDT)) == "which") {
		pos_name = find_closest_WDT_name(pre_drt, pos_WDT);
		if (pos_name == -1) // if really there is no name to associate with, do nothing and return
			return to_return;
	}

	string name_ref = extract_first_tag(to_return.at(pos_name));
	string WDT_ref = extract_first_tag(to_return.at(pos_WDT));
	if (pos_prep != -1) { // in which, to whom, ...

		to_return = substitute_ref(to_return, WDT_ref, name_ref);

		if (pos_verb != -1) {
			to_return.at(pos_prep) = implant_first(to_return.at(pos_prep), extract_first_tag(to_return.at(pos_verb)));
		}
		to_return.at(pos_prep) = implant_second(to_return.at(pos_prep), name_ref);

		implant_header(to_return.at(pos_WDT), extract_header(to_return.at(pos_name)) + ":DELETE");
		to_return.at(pos_WDT).setTag(to_return.at(pos_name).tag());
	} else { // the person *who* wrote the letter

		bool hassubject = has_subject(to_return.at(pos_verb));
		bool hasobject = has_object(to_return.at(pos_verb));

		//if( !hassubject && hasobject)
		if (!hassubject)
			implant_subject(to_return.at(pos_verb), WDT_ref);
		//if( hassubject && !hasobject)
		else if (!hasobject)
			implant_object(to_return.at(pos_verb), WDT_ref);
		to_return = substitute_ref(to_return, WDT_ref, name_ref);
		implant_header(to_return.at(pos_WDT), extract_header(to_return.at(pos_WDT)) + ":DELETE");
	}

	return to_return;
}

static bool has_the(const DrtVect &drtvect, const string &ref)
{
	for(int n=0; n < drtvect.size(); ++n) {
		string fref = extract_first_tag(drtvect.at(n));
		if(fref == ref) {
			string header = extract_header(drtvect.at(n));
			string tag = drtvect.at(n).tag();
			if (tag == "DT" && header == "the")
				return true;
		}
	}
	return false;
}

static DrtVect process_WRB_to_nouns(vector<DrtPred> &pre_drt, const vector<string> &tags, vector<string> names,
		vector<pair<pair<int, int>, constituents> > connections)
{
	DrtVect to_return(pre_drt);

	int pos1, pos2;
	string str1, str2, tag1, tag2;
	string new_tag = "";
	int place_pos = -1;

	int new_dist = 0, old_dist = -1;
	for (int n = 0; n < connections.size(); ++n) { // assign missing links to prepositions and WRBs
		pos1 = connections.at(n).first.first;
		pos2 = connections.at(n).first.second;
		str1 = names.at(pos1);
		str2 = names.at(pos2);
		tag1 = tags.at(pos1);
		tag2 = tags.at(pos2);
		new_dist = pos2 - pos1;
		if (tag2 == "WRB"
				&& first_tag_is_incomplete(to_return.at(pos2))
				&& is_name(tag1)
				&& has_the(pre_drt,extract_first_tag(to_return.at(pos1)))
				&& new_dist > 0
			) {
			new_tag = extract_first_tag(to_return.at(pos1));
			implant_first(to_return.at(pos2),new_tag);
		}
	}
	return to_return;
}

static DrtVect process_ifs_with_last_verb(vector<DrtPred> &pre_drt, const vector<string> &tags, vector<string> names,
		vector<pair<pair<int, int>, constituents> > connections)
{
	DrtVect to_return(pre_drt);

	int pos1, pos2;
	string str1, str2, tag1, tag2;
	string new_tag = "";
	int if_pos = -1;

	int new_dist = 0, old_dist = -1;
	for (int n = 0; n < connections.size(); ++n) { // assign missing links to prepositions and WRBs
		pos1 = connections.at(n).first.first;
		pos2 = connections.at(n).first.second;
		str1 = names.at(pos1);
		str2 = names.at(pos2);
		tag1 = tags.at(pos1);
		tag2 = tags.at(pos2);
		new_dist = fabs(pos2 - pos1);
		if (is_preposition(tag1) && str1 == "if" && first_tag_is_incomplete(to_return.at(pos1)) && is_verb(tag2)
				&& new_dist > old_dist) {
			new_tag = extract_first_tag(to_return.at(pos2));
			if_pos = pos1;
			old_dist = new_dist;
		}
	}

	if (if_pos != -1) {
		string stag = extract_second_tag(to_return.at(if_pos));
		if (stag != new_tag)
			implant_first(to_return.at(if_pos), new_tag);
	}
	return to_return;
}

static vector<DrtPred> resolve_WP_connections(vector<DrtPred> &pre_drt, const vector<string> &tags, vector<string> names,
		vector<pair<pair<int, int>, constituents> > connections)
{
	vector<DrtPred> to_return(pre_drt);
	int size = connections.size();
	int pos_WDT = -1;
	int pos_prep = -1;
	int pos_name = -1;
	int pos_verb = -1;
	int pos1, pos2;

	for (int n = 0; n < connections.size(); ++n) {  // find where the WDT is
		pos1 = connections.at(n).first.first;
		pos2 = connections.at(n).first.second;
		//if(to_return.at(pos1).is_WDT() || to_return.at(pos1).is_WP())
		if (to_return.at(pos1).is_WP())
			pos_WDT = pos1;
		//if(to_return.at(pos2).is_WDT() || to_return.at(pos2).is_WP())
		if (to_return.at(pos2).is_WP())
			pos_WDT = pos2;
		if (pos_WDT != -1) {
			for (int m = 0; m < connections.size(); ++m) { // find what is attached to the WDT
				pos1 = connections.at(m).first.first;
				pos2 = connections.at(m).first.second;
				if (pos1 == pos_WDT) {
					if (pos_prep == -1 && to_return.at(pos2).is_preposition()) {
						pos_prep = pos2;
						continue;
					}
					if (pos_name == -1 && to_return.at(pos2).is_name()) {
						pos_name = pos2;
						continue;
					}
					if (  //pos_verb == -1
					pos_verb < pos2  // the verb has to be on the right
					//&& (pos_verb - pos_WDT) > fabs(pos_verb - pos2) // the closest verb wins
					&& to_return.at(pos2).is_verb()) {
						pos_verb = pos2;
						continue;
					}
				} else if (pos2 == pos_WDT) {
					if (pos_prep == -1 && to_return.at(pos1).is_preposition()) {
						pos_prep = pos1;
						continue;
					}
					if (pos_name == -1 && to_return.at(pos1).is_name()) {
						pos_name = pos1;
						continue;
					}
					if (pos_verb == -1 && to_return.at(pos1).is_verb()) {
						pos_verb = pos1;
						continue;
					}
				}
			}
			to_return = change_WDT(to_return, pos_WDT, pos_prep, pos_name, pos_verb);
			pos_WDT = pos_prep = pos_name = pos_verb = -1;
		}
	}

	return to_return;
}

static vector<DrtPred> process_indirect_allocution(vector<DrtPred> &pre_drt, const vector<string> &tags, vector<string> names,
		vector<pair<pair<int, int>, constituents> > connections)
{
	vector<DrtPred> to_return(pre_drt);

	vector<string> communication_verbs = get_communication_verbs();
	// communication_verbs.push_back("call");
	// communication_verbs.push_back("do");
	// communication_verbs.push_back("help");

	for (int n = 0; n < to_return.size(); ++n) {
		string head = extract_header(to_return.at(n));
		if (head == "@SUB-OBJ") {
			string fref = extract_first_tag(to_return.at(n));
			if (!ref_is_verb(fref))
				continue;
			int m = find_verb_with_string(to_return, fref);
			if (m == -1)
				continue;
			string vhead = extract_header(to_return.at(m));
			vhead = vhead.substr(0, vhead.find(":"));
			if (shortfind(communication_verbs, vhead)) {
				implant_header(to_return.at(n), "@PARENT-ALLOCUTION");
				to_return.at(n).name() = "@PARENT-ALLOCUTION";
				switch_children(to_return.at(n));
			}
		}
	}

	return to_return;
}

static vector<DrtPred> add_THAT_subordinate(vector<DrtPred> &pre_drt, int pos_THAT, int pos_verb, int pos_second_verb)
{
	vector<DrtPred> to_return(pre_drt);

	if (pos_THAT == -1)
		return to_return;
	if (pos_verb == -1 || pos_second_verb == -1) {
		implant_header(to_return.at(pos_THAT), extract_header(to_return.at(pos_THAT)) + ":DELETE");
		return to_return;
	}
	string first_verb_ref = extract_first_tag(to_return.at(pos_verb));
	string second_verb_ref = extract_first_tag(to_return.at(pos_second_verb));

	DrtPred tmp(string("@SUB-OBJ(") + first_verb_ref + "," + second_verb_ref + ")");
	implant_header(to_return.at(pos_THAT), extract_header(to_return.at(pos_THAT)) + ":DELETE");

	to_return.push_back(tmp);

	return to_return;
}

static vector<DrtPred> change_THAT(vector<DrtPred> &pre_drt, int pos_THAT, int pos_name, int pos_verb)
{
	vector<DrtPred> to_return(pre_drt);

	if (pos_THAT == -1)
		return to_return;
	if (pos_name == -1) {
		implant_header(to_return.at(pos_THAT), extract_header(to_return.at(pos_THAT)) + ":DELETE");
		return to_return;
	}
	string name_ref = extract_first_tag(to_return.at(pos_name));

	if (pos_verb != -1) {
		bool hassubject = has_subject(pre_drt.at(pos_verb));
		bool hasobject = has_object(pre_drt.at(pos_verb));
		string verb_ref = extract_first_tag(to_return.at(pos_verb));
		if (verb_ref != name_ref) {  // avoids a verb with itself as a subject
			if (hassubject && !hasobject)
				implant_object(to_return.at(pos_verb), name_ref);
			if (!hassubject && !hasobject) {
				if (is_passive(to_return.at(pos_verb)))
					implant_object(to_return.at(pos_verb), name_ref);
				else
					implant_subject(to_return.at(pos_verb), name_ref);
			}
			if (!hassubject && hasobject)
				implant_subject(to_return.at(pos_verb), name_ref);
			if (hasobject) {
				string obj_verb = extract_object(to_return.at(pos_verb));
				if (points_to_verb(obj_verb)) {
					int m = find_verb_with_string(to_return, obj_verb);
					if (m != -1) {
						string subj = extract_subject(to_return.at(m));
						if (!has_object(pre_drt.at(m))) {
							implant_object(to_return.at(m), name_ref);
						}
					}
				}
			}
		}
		implant_header(to_return.at(pos_THAT), extract_header(to_return.at(pos_THAT)) + ":DELETE");
	}

	return to_return;
}

static int find_post_aux(const DrtVect &pre_drt, int n)
{
	string obj_ref = extract_object(pre_drt.at(n));
	vector<int> m = find_all_element_with_string(pre_drt, obj_ref);

	for (int j = 0; j < m.size(); ++j) {
		int pos = m.at(j);
		if (pre_drt.at(pos).is_verb() && !is_AUX(pre_drt.at(pos)))
			return pos;
	}
	return -1;
}

static vector<DrtPred> prepare_THAT(vector<DrtPred> &pre_drt, const vector<string> &tags, vector<string> names,
		vector<pair<pair<int, int>, constituents> > connections)
{
	vector<DrtPred> to_return(pre_drt);

	for(int n=0; n < to_return.size(); ++n) {
		string header = extract_header(to_return.at(n));
		string tag = to_return.at(n).tag();
		if (tag == "IN" && header == "that") {
			string fref = extract_first_tag(to_return.at(n));
			if(!ref_is_name(fref) && !ref_is_verb(fref)) {
				//int pos= find_complement_with_target(to_return,fref);
				//if(pos == -1)
				int pos= find_prep_with_target(to_return,tags,fref);
				if(pos != -1) {
					implant_first(to_return.at(n),extract_first_tag(to_return.at(pos)));
				}
			}
		}
	}
	return to_return;
}

static vector<DrtPred> resolve_THAT(vector<DrtPred> &pre_drt, const vector<string> &tags, vector<string> names,
		vector<pair<pair<int, int>, constituents> > connections)
// for phrases like: "the person that david called is not at home"
{
	vector<DrtPred> to_return(pre_drt);
	int size = connections.size();
	int pos_THAT = -1;
	int pos_name = -1;
	int pos_verb = -1;
	int pos_second_verb = -1;
	int pos1, pos2;

	for (int n = 0; n < connections.size(); ++n) {  // find where the WDT is
		pos1 = connections.at(n).first.first;
		pos2 = connections.at(n).first.second;
		if (to_return.at(pos1).is_preposition() && extract_header(to_return.at(pos1)) == "that")
			pos_THAT = pos1;
		if (to_return.at(pos2).is_preposition() && extract_header(to_return.at(pos2)) == "that")
			pos_THAT = pos2;
		if (pos_THAT != -1) {
			for (int m = 0; m < connections.size(); ++m) { // find what is attached to the WDT
				pos1 = connections.at(m).first.first;
				pos2 = connections.at(m).first.second;
				if (pos1 == pos_THAT) {
					if (pos_name == -1 && to_return.at(pos2).is_name()) {
						pos_name = pos2;
						continue;
					}
					if (  //pos_verb == -1 &&
					to_return.at(pos2).is_verb()) {
						if (pos_verb != -1 && pos_verb != pos2)
							pos_second_verb = pos2;
						else
							pos_verb = pos2;
						continue;
					}
				} else if (pos2 == pos_THAT) {
					if (pos_name == -1 && to_return.at(pos1).is_name()) {
						pos_name = pos1;
						continue;
					}
					if (  //pos_verb == -1 &&
					to_return.at(pos1).is_verb()) {
						if (pos_verb != -1 && pos_verb != pos1)
							pos_second_verb = pos1;
						else
							pos_verb = pos1;
						continue;
					}
				}
			}
			bool verb_case = false;
			if (pos_verb != -1 && pos_second_verb != -1) {
				verb_case = true;
				if ((!is_AUX(to_return.at(pos_verb)) || to_return.at(pos_verb).is_modal())
						&& (!is_AUX(to_return.at(pos_second_verb)) || to_return.at(pos_second_verb).is_modal())) {
					to_return = add_THAT_subordinate(to_return, pos_THAT, pos_verb, pos_second_verb);
					if (is_AUX(to_return.at(pos_second_verb))) {
						pos_second_verb = find_post_aux(to_return, pos_second_verb);
					}
					to_return = change_THAT(to_return, pos_THAT, pos_name, pos_second_verb);
				}
				DrtPred tmp_pred(to_return.at(pos_THAT));
				// they decided that it was for the best
				implant_header(tmp_pred, "@SUBORD");
				tmp_pred.setName("that");
				to_return.push_back(tmp_pred);
			}
			if (pos_verb != -1 && !verb_case && pos_THAT != -1) {
				// "The proof that they are awake"
				if (is_AUX(to_return.at(pos_verb))) {
					pos_verb = find_post_aux(to_return, pos_verb);
				}
				if(pos_name != -1 && pos_name < pos_verb) {
					string f_gen = extract_first_tag(to_return.at(pos_name));
					string s_gen = extract_first_tag(to_return.at(pos_verb));
					DrtPred gen_pred("@GENITIVE(" + f_gen + "," + s_gen + ")");
					gen_pred.setName("that");
					add_header(to_return.at(pos_THAT), ":DELETE");
					to_return.push_back(gen_pred);
				}
			}
			if (debug) {
				puts("THAT_NAME:::");
			}
			pos_THAT = pos_name = pos_verb = pos_second_verb = -1;
		}
	}

	// for THAT that have not been assigned
	for (int n = 0; n < to_return.size(); ++n) {
		string header = extract_header(to_return.at(n));
		string fref = extract_first_tag(to_return.at(n));
		string sref = extract_second_tag(to_return.at(n));
		if (header == "that"
		    && ref_is_verb(fref)
		    && ref_is_verb(sref)
		) {
			DrtPred tmp_pred(to_return.at(n));
			implant_header(tmp_pred, "@SUBORD");
			tmp_pred.setName("that");
			to_return.push_back(tmp_pred);
			add_header(to_return.at(n), ":DELETE");
		}
		if (header == "that"
			&& ref_is_name(fref)
			&& ref_is_verb(sref)
		) {
			DrtPred tmp_pred(to_return.at(n));
			implant_header(tmp_pred, "@GENITIVE");
			tmp_pred.setName("that");
			to_return.push_back(tmp_pred);
			add_header(to_return.at(n), ":DELETE");
		}
	}


	return to_return;
}

static int find_cause(const vector<DrtPred> &drtvect, const DrtPred &verb)
{
	int pos = -1;

	string verb_str = extract_first_tag(verb);
	vector<int> positions = find_all_element_with_string(drtvect, verb_str);

	for (int n = 0; n < positions.size(); ++n) {
		string head = extract_header(drtvect.at(positions.at(n)));
		if (head == "@CAUSED_BY") {
			pos = positions.at(n);
			break;
		}
	}

	return pos;
}

static vector<DrtPred> join_verb_with_adverb(vector<DrtPred> to_return, const vector<string> &tags, vector<string> names)
{
	int pos1, pos2;
	string str, str1, str2, tag;

	metric *d = metric_singleton::get_metric_instance();

	for (int n = 0; n < to_return.size(); ++n) {
		if (to_return.at(n).is_adverb()) {
			string ref = extract_first_tag(to_return.at(n));
			vector<int> candidate_verbs = find_all_element_with_string(to_return, ref);
			for (int j = 0; j < candidate_verbs.size(); ++j) {
				int m = candidate_verbs.at(j);
				if (to_return.at(m).is_verb()) {
					if (extract_header(to_return.at(m)).find("AUX") == 0
							|| extract_header(to_return.at(m)).find("PASSIVE_AUX") == 0)
						continue; // auxiliaries are erased in the end

					str1 = to_return.at(m).name();
					str2 = to_return.at(n).name();
					string tmp_verb_name = str1 + "_" + str2;

					if (d->has_verb(tmp_verb_name)) {
						implant_header(to_return.at(m), tmp_verb_name);
						to_return.at(m).setName(tmp_verb_name);
						implant_header(to_return.at(n), extract_header(to_return.at(n)) + ":DELETE");
						to_return.at(n).name() += ":DELETE";
					}
				}
			}
		}
	}
	return to_return;
}

static DrtVect process_age(DrtVect pre_drt, vector<string> &tags, vector<string> &names)
{
	if(pre_drt.size() == 0)
		return pre_drt;

	vector<string> age_triggers;
	age_triggers.push_back("year");
	age_triggers.push_back("month");
	age_triggers.push_back("day");

	for (int n = 0; n < pre_drt.size() - 1; ++n) {
		string fref1 = extract_first_tag(pre_drt.at(n));
		string header = extract_header(pre_drt.at(n));
		string fref2 = extract_first_tag(pre_drt.at(n + 1));
		string next_header = extract_header(pre_drt.at(n + 1));
		if (fref1 == fref2 && shortfind(age_triggers, header) && next_header == "old") {
			int m = find_verb_with_object(pre_drt, fref1);
			if (m == -1)
				continue;
			implant_header(pre_drt.at(n + 1), next_header + ":DELETE");
			string vref = extract_first_tag(pre_drt.at(m));
			implant_object(pre_drt.at(m), "none");
			DrtPred agepred(string("@AGE(") + vref + "," + fref1 + ")");
			pre_drt.push_back(agepred);
		}
	}

	for (int n = 0; n < pre_drt.size(); ++n) {
		string fref = extract_first_tag(pre_drt.at(n));
		string header = extract_header(pre_drt.at(n));
		if (header == "old") {
			int m = find_complement_with_target(pre_drt, fref);
			if (m == -1)
				continue;
			int m2 = find_complement_with_first_tag(pre_drt, fref);
			if (m2 == -1)
				continue;
			string cheader1 = extract_header(pre_drt.at(m));
			string cheader2 = extract_header(pre_drt.at(m2));
			if( (cheader1 == "@MORE")
				&& cheader2 == "@COMPARED_TO"
			) {
				int mverb= find_verb_with_string(pre_drt,extract_first_tag(pre_drt.at(m)));
				int mnoun= find_name_with_string(pre_drt,extract_second_tag(pre_drt.at(m2)));
				int mquantity = find_complement_with_first_tag(pre_drt, extract_second_tag(pre_drt.at(m2)), "@QUANTITY");
				if(mverb != -1 && mnoun != -1 && mquantity != -1) {
					implant_header(pre_drt.at(m) ,":DELETE");
					implant_header(pre_drt.at(m2),":DELETE");
					implant_header(pre_drt.at(n) ,":DELETE");
					implant_header(pre_drt.at(mquantity) ,"@MORE_THAN");
					string vref= extract_first_tag(pre_drt.at(mverb));
					string sref= extract_first_tag(pre_drt.at(mnoun));
					DrtPred age_pred("@AGE(" + vref + "," + sref + ")");
					pre_drt.push_back(age_pred);
				}
			}
		}
		if (header == "young") {
			int m = find_complement_with_target(pre_drt, fref);
			if (m == -1)
				continue;
			int m2 = find_complement_with_first_tag(pre_drt, fref);
			if (m2 == -1)
				continue;
			string cheader1 = extract_header(pre_drt.at(m));
			string cheader2 = extract_header(pre_drt.at(m2));
			if( (cheader1 == "@MORE")
					&& cheader2 == "@COMPARED_TO"
			) {
				int mverb= find_verb_with_string(pre_drt,extract_first_tag(pre_drt.at(m)));
				int mnoun= find_name_with_string(pre_drt,extract_second_tag(pre_drt.at(m2)));
				int mquantity = find_complement_with_first_tag(pre_drt, extract_second_tag(pre_drt.at(m2)), "@QUANTITY");
				if(mverb != -1 && mnoun != -1 && mquantity != -1) {
					implant_header(pre_drt.at(m) ,":DELETE");
					implant_header(pre_drt.at(m2),":DELETE");
					implant_header(pre_drt.at(n) ,":DELETE");
					implant_header(pre_drt.at(mquantity) ,"@LESS_THAN");
					string vref= extract_first_tag(pre_drt.at(mverb));
					string sref= extract_first_tag(pre_drt.at(mnoun));
					DrtPred age_pred("@AGE(" + vref + "," + sref + ")");
					pre_drt.push_back(age_pred);
				}
			}
		}
	}

	return pre_drt;
}

static bool is_times_RB(const string &str)
{
	if (str.find("-times") != string::npos)
		return true;
	return false;
}

static string get_number_times(const string &str)
{
	string num_str = "0";
	int pos = str.find("-times");
	if (pos > 0 && pos != string::npos) {
		num_str = str.substr(0, pos);
	}
	return num_str;
}

static DrtVect process_times_RB(DrtVect pre_drt, vector<string> &tags, vector<string> &names)
{
	for (int n = 0; n < pre_drt.size(); ++n) {
		string fref = extract_first_tag(pre_drt.at(n));
		string header = extract_header(pre_drt.at(n));
		if (is_times_RB(header) && !pre_drt.at(n).is_question()) {
			implant_header(pre_drt.at(n), header + ":DELETE");
			string num_str = get_number_times(header);
			string dummyname = string("dummyname") + boost::lexical_cast<string>(n);
			string dummyname_num = string("dummyname_num_") + boost::lexical_cast<string>(n);
			DrtPred tmp_pred(string("times/NN") + "(" + dummyname + ")");
			DrtPred timespred(string("@TIMES(") + fref + "," + dummyname + ")");
			DrtPred quantpred(string("@QUANTITY(") + dummyname + "," + dummyname_num + ")");
			DrtPred num_pred(num_str + "(" + dummyname_num + ")");
			pre_drt.push_back(tmp_pred);
			pre_drt.push_back(timespred);
			pre_drt.push_back(quantpred);
			pre_drt.push_back(num_pred);
		}
	}

	return pre_drt;
}

static bool has_hashtag(const string &str)
{
	if (str.find("#") != string::npos)
		return true;
	return false;
}

static DrtVect process_hashtags(DrtVect pre_drt, vector<string> &tags, vector<string> &names)
// this function modifies the names!
{
	map<string, vector<int> > hashtags;

	// search the names with #
	for (int n = 0; n < names.size(); ++n) {
		if (has_hashtag(names.at(n))) {
			string head = names.at(n);
			int pos = head.find("#");
			string htag = head.substr(pos, -1);
			head = head.substr(0, pos);
			names.at(n) = head;
			implant_header(pre_drt.at(n), head);
			pre_drt.at(n).name() = head;
			hashtags[htag].push_back(n);
		}
	}

	vector<pair<string, string> > ref_pair;
	map<string, vector<int> >::iterator hashiter = hashtags.begin();
	for (; hashiter != hashtags.end(); ++hashiter) {
		vector<int> positions = hashiter->second;
		string first_ref = extract_first_tag(pre_drt.at(positions.at(0)));
		for (int n = 1; n < positions.size(); ++n) {
			string second_ref = extract_first_tag(pre_drt.at(positions.at(n)));
			ref_pair.push_back(make_pair(second_ref, first_ref));
		}
		// Elements with hastag become questions
		for (int n = 0; n < positions.size(); ++n) {
			pre_drt.at(positions.at(n)).set_question();
			pre_drt.at(positions.at(n)).set_question_word(pre_drt.at(positions.at(n)).name());
		}
	}

	for (int n = 0; n < ref_pair.size(); ++n) {
		string ref = ref_pair.at(n).first;
		string name = ref_pair.at(n).second;
		pre_drt = substitute_ref(pre_drt, ref, name);
	}

	return pre_drt;
}

static vector<DrtPred> join_verb_with_preposition(vector<DrtPred> to_return, const vector<string> &tags, vector<string> names)
{
	int pos1, pos2;
	string str, str1, str2, tag;

	metric *d = metric_singleton::get_metric_instance();

	for (int n = 0; n < to_return.size(); ++n) {
		if (to_return.at(n).is_preposition()) {
			string ref = extract_first_tag(to_return.at(n));
			if (ref.find("verb") == string::npos) // If the preposition is not connected to a verb do noting
				continue;
			vector<int> candidate_verbs = find_all_element_with_string(to_return, ref);
			for (int j = 0; j < candidate_verbs.size(); ++j) {
				int m = candidate_verbs.at(j);
				if (to_return.at(m).is_verb()) {
					if (extract_header(to_return.at(m)).find("AUX") == 0
							|| extract_header(to_return.at(m)).find("PASSIVE_AUX") == 0)
						continue; // auxiliaries are erased in the end
					str1 = to_return.at(m).name();
					str2 = to_return.at(n).name();
					if (str2 != "out")
						continue; /// Only "out" is considered
					string tmp_verb_name = str1 + "_" + str2;
					if (d->has_verb(tmp_verb_name)) {
						string prep_to = extract_second_tag(to_return.at(n));
						string obj = extract_object(to_return.at(m));
						if (has_object(to_return.at(m)) && obj != prep_to) // If the verb already have an object different from the proposition
							continue;
						implant_header(to_return.at(m), tmp_verb_name);
						to_return.at(m).setName(tmp_verb_name);
						implant_header(to_return.at(n), extract_header(to_return.at(n)) + ":DELETE");
						to_return.at(n).name() += ":DELETE";
						if (!has_object(to_return.at(m))
								) {
							// substitute the object of the
							// verb with what is pointed to by
							// the preposition
							implant_object(to_return.at(m), prep_to);
						}
					}
				}
			}
		}
	}
	return to_return;
}

static vector<DrtPred> process_auxiliary_like_verbs(vector<DrtPred> to_return, const vector<string> &tags, const vector<string> &names,
		vector<pair<pair<int, int>, constituents> > &connections)
{
	int pos1, pos2;
	string str1, str2, tag1, tag2;

	metric *d = metric_singleton::get_metric_instance();

	for (int n = 0; n < connections.size(); ++n) { // conjunctions
		pos1 = connections.at(n).first.first;
		pos2 = connections.at(n).first.second;
		str1 = names.at(pos1);
		str2 = names.at(pos2);
		tag1 = tags.at(pos1);
		tag2 = tags.at(pos2);

		if (is_verb(tag1)
				&& (str1 == "make" || str1 == "let")
				&& (tag2 == "VB" || tag2 == "VBP") ) {
			if (extract_header(to_return.at(pos1)).find("AUX") == 0
					|| extract_header(to_return.at(pos1)).find("PASSIVE_AUX") == 0)
				continue;
			string vref1= extract_first_tag(to_return.at(pos1));
			string vref2= extract_first_tag(to_return.at(pos2));
			DrtPred subord_pred("@SUBORD(" + vref1 + "," + vref2 + ")");
			subord_pred.setName("to");
			if(!shortfind(to_return,subord_pred)) {
				to_return.push_back(subord_pred);
				to_return.at(pos2).setTag("VB");
			}
		}
	}

	return to_return;
}

static vector<DrtPred> join_verb_with_RP(vector<DrtPred> to_return, const vector<string> &tags, const vector<string> &names,
		vector<pair<pair<int, int>, constituents> > &connections)
{
	int pos1, pos2;
	string str1, str2, tag1, tag2;

	metric *d = metric_singleton::get_metric_instance();

	for (int n = 0; n < connections.size(); ++n) { // conjunctions
		pos1 = connections.at(n).first.first;
		pos2 = connections.at(n).first.second;
		str1 = names.at(pos1);
		str2 = names.at(pos2);
		tag1 = tags.at(pos1);
		tag2 = tags.at(pos2);

		if (is_verb(tag1) && tag2 == "RP") {
			if (extract_header(to_return.at(pos1)).find("AUX") == 0
					|| extract_header(to_return.at(pos1)).find("PASSIVE_AUX") == 0)
				continue;
			string ref_parent = extract_first_tag(to_return.at(pos1));
			string ref_child = extract_first_tag(to_return.at(pos2));
			str1 = to_return.at(pos1).name();
			str2 = to_return.at(pos2).name();
			string tmp_verb_name = str1 + "_" + str2;
			if (d->has_verb(tmp_verb_name)) {
				implant_header(to_return.at(pos1), tmp_verb_name);
				to_return.at(pos1).setName(tmp_verb_name);
				add_header(to_return.at(pos2), ":DELETE");
				to_return.at(pos2).name() += ":DELETE";
			}
		}
	}

	return to_return;
}

static vector<DrtPred> join_verb_with_name(vector<DrtPred> to_return, const vector<string> &tags, vector<string> names)
{
	int pos1, pos2;
	string str, str1, str2, tag;

	metric *d = metric_singleton::get_metric_instance();
	vector<pair<string, string> > ref_pair;

	for (int n = 0; n < to_return.size(); ++n) {
		if (to_return.at(n).is_verb()) {
			if (extract_header(to_return.at(n)).find("AUX") == 0 || extract_header(to_return.at(n)).find("PASSIVE_AUX") == 0)
				continue;
			string ref = extract_object(to_return.at(n));
			vector<int> candidate_names = find_all_element_with_string(to_return, ref);
			for (int j = 0; j < candidate_names.size(); ++j) {
				int m = candidate_names.at(j);
				if (to_return.at(m).is_name()) {
					str1 = to_return.at(n).name();
					str2 = to_return.at(m).name();
					string tmp_verb_name = str1 + "_" + str2;
					if (d->has_verb(tmp_verb_name)) {
						implant_header(to_return.at(n), tmp_verb_name);
						to_return.at(n).setName(tmp_verb_name);
						implant_object(to_return.at(n), "none");

						string name_ref = extract_first_tag(to_return.at(m));
						string verb_ref = extract_first_tag(to_return.at(n));
						ref_pair.push_back(make_pair(name_ref, verb_ref));
						add_header(to_return.at(m), ":DELETE");
						to_return.at(m).name() += ":DELETE";
					}
				}
			}
		}
	}

	for (int n = 0; n < ref_pair.size(); ++n) {
		string ref = ref_pair.at(n).first;
		string name = ref_pair.at(n).second;
		to_return = substitute_ref(to_return, ref, name);
	}

	return to_return;
}

static bool verbs_can_join(DrtVect pre_drt, int n2, int m2, vector<int> prn_depths, bool is_question)
// Example: he was angry and crushed the car. "was" and "crushed" are not connected.
/// This should be refined. AUX and VBN connects even after a
/// conjunction, but the AUX exists alone as well
{
	int n, m;
	if (n2 < m2) {
		n = n2;
		m = m2;
	} else {
		n = m2;
		m = n2;
	}

	int orig_depth= prn_depths.at(n);
	if ( orig_depth != prn_depths.at(m) )
		return false;
	for (; n < m; ++n) {
		if ( orig_depth == prn_depths.at(n)
			&&
			(
			     (!is_question && pre_drt.at(n).is_conj()) || pre_drt.at(n).is_comma() || (!is_question && pre_drt.at(n).is_WP())
				|| (!is_question && pre_drt.at(n).is_WDT()) || (!is_question && pre_drt.at(n).is_name())
			)
		) { // he was angry and crushed the car
			return false;
		}
	}
	return true;
}

static bool aux_on_left(const DrtVect &pre_drt, int n2, int m2)
// n < m
{
	int n, m;
	if (n2 < m2) {
		n = n2;
		m = m2;
	} else {
		n = m2;
		m = n2;
	}

	string tag1 = pre_drt.at(n).tag();
	string tag2 = pre_drt.at(m).tag();
	if (tag1 == "MD" && tag2 != "MD")
		return true;

	string str1 = extract_header(pre_drt.at(n));
	string str2 = extract_header(pre_drt.at(m));

	if (debug) {
		cout << "AUX_HEADER:::" << str1;
	}

	if (str1 == "have" && str2 == "be")
		return true; // beware of the double auxiliary!
	if (str1 == "have" && str2 == "have")
		return true;
	if (str1 == "be" && str2 == "be")
		return true;
	if (str1 == "do" && str2 == "do")
		return true;

	if (str1 == "be" && str2 != "be" && tag2 != "MD")
		return true;
	if (str1 == "have" && str2 != "have" && tag2 != "MD")
		return true;
	if (str1 == "do" && str2 != "do" && tag2 != "MD")
		return true;

	return false;
}

static vector<DrtPred> collapse_verbs(vector<DrtPred> &pre_drt, const vector<string> &tags, vector<string> names, vector<int> prn_depths,
		bool is_question)
{
	vector<DrtPred> to_return(pre_drt);
	int size = tags.size();
	string str, tag;
	tagger *tagg = parser_singleton::get_tagger_instance();

	// Build the list of connected werbs (like "were thinking", "has started", "has been", "been waiting")
	vector<pair<int, int> > verb_pairs;
	vector<int> already_joined;
	for (int n = 0; n < size; ++n) {
		str = names.at(n);
		tag = tags.at(n);
		if (is_verb(tag)) {
			int m = find_coupled_verb(pre_drt, tags, n);
			if (debug) {
				cout << "COUPLED" << m << endl;
			}

			if (m != -1 && m != n && find(verb_pairs.begin(), verb_pairs.end(), make_pair(m, n)) == verb_pairs.end()
					&& find(verb_pairs.begin(), verb_pairs.end(), make_pair(n, m)) == verb_pairs.end()) {
				if (find(already_joined.begin(), already_joined.end(), m) != already_joined.end()
						&& find(already_joined.begin(), already_joined.end(), n) == already_joined.end())
					continue;
				if (find(already_joined.begin(), already_joined.end(), n) != already_joined.end()
						&& find(already_joined.begin(), already_joined.end(), m) == already_joined.end())
					continue;

				// Always put the auxiliary on the left
				// and check that the two verbs can join
				// The aux must be on the left

				if (n < m && verbs_can_join(to_return, n, m, prn_depths, is_question)) {
					if (!aux_on_left(to_return, m, n))
						continue;
					verb_pairs.push_back(make_pair(n, m));
					if (!is_auxiliary_name(names.at(m)) && !is_modal(tags.at(m)))
						already_joined.push_back(m); // in "have been applied" been can still join to applied
					already_joined.push_back(n);
				} else if (verbs_can_join(to_return, m, n, prn_depths, is_question)) {
					if (!aux_on_left(to_return, m, n))
						continue;
					verb_pairs.push_back(make_pair(m, n));
					already_joined.push_back(m);
					if (!is_auxiliary_name(names.at(n)) && !is_modal(tags.at(n)))
						already_joined.push_back(n);
				}
			}
		}
	}

	for (int n = 0; debug && n < verb_pairs.size(); ++n) {
		int pos1 = verb_pairs.at(n).first, pos2 = verb_pairs.at(n).second;
		string arrow;
		if (pos1 > pos2)
			arrow = " <- ";
		else
			arrow = " -> ";
		std::cout << names.at(pos1) << " (" << pos1 << ", " << tags.at(pos1) << ")" << arrow << names.at(pos2) << " (" << pos2
				<< ", " << tags.at(pos2) << ")" << std::endl;

	}
	if (debug) {
		std::cout << "------------" << std::endl;
	}

	// recognizes the tense of the the base verb, save it, and sign
	// the auxiliary verbs for deletion
	vector<DrtPred> tenses;
	for (int n = 0; n < verb_pairs.size(); ++n) {
		int pos1 = verb_pairs.at(n).first, pos2 = verb_pairs.at(n).second;
		if (names.at(pos1) == "be" && tags.at(pos2).find("VBG") != string::npos) {
			if (tags.at(pos1) == "VBP" || tags.at(pos1) == "VBZ") {
				string verb_str = extract_verb_name(to_return.at(pos2));
				DrtPred tmp_tense(string("@TIME(") + verb_str + "," + "present|continuative" + ")");
				tenses.push_back(tmp_tense);
			} else if (tags.at(pos1) == "VBD") {
				string verb_str = extract_verb_name(to_return.at(pos2));
				DrtPred tmp_tense(string("@TIME(") + verb_str + "," + "past|continuative" + ")");
				tenses.push_back(tmp_tense);
			}
			// sign the auxiliary verb to be deleted
			implant_header(to_return.at(pos1), "AUX");
			add_header(to_return.at(pos2), ":POST_AUX");
			names.at(pos2) += ":POST_AUX";
			names.at(pos1) = "AUX";
		} else if (names.at(pos1) == "be" && tags.at(pos2).find("VBN") != string::npos) {
			if (tags.at(pos1) == "VB" || tags.at(pos1) == "VBP" || tags.at(pos1) == "VBZ") {
				string verb_str = extract_verb_name(to_return.at(pos2));
				DrtPred tmp_tense(string("@TIME(") + verb_str + "," + "present" + ")");
				tenses.push_back(tmp_tense);
			} else if (tags.at(pos1) == "VBD") {
				string verb_str = extract_verb_name(to_return.at(pos2));
				DrtPred tmp_tense(string("@TIME(") + verb_str + "," + "past" + ")");
				tenses.push_back(tmp_tense);
			} else if (tags.at(pos1) == "VBG") {
				string verb_str = extract_verb_name(to_return.at(pos2));
				DrtPred tmp_tense(string("@TIME(") + verb_str + "," + "past|continuative" + ")");
				tenses.push_back(tmp_tense);
			}
			// sign the auxiliary verb to be deleted
			implant_header(to_return.at(pos1), "PASSIVE_AUX");
			add_header(to_return.at(pos2), ":PASSIVE_POST_AUX");
			names.at(pos1) = "PASSIVE_AUX";
			names.at(pos2) += ":PASSIVE_POST_AUX";
		} else if (names.at(pos1) == "have" && names.at(pos2).find("be") == string::npos
				&& tags.at(pos2).find("VBN") != string::npos) { /// It should be just VBN: improve the tagging
			if (tags.at(pos1) == "VB" || tags.at(pos1) == "VBP" || tags.at(pos1) == "VBZ") {
				string verb_str = extract_verb_name(to_return.at(pos2));
				DrtPred tmp_tense(string("@TIME(") + verb_str + "," + "past" + ")");
				tenses.push_back(tmp_tense);
			} else if (tags.at(pos1) == "VBD") {
				string verb_str = extract_verb_name(to_return.at(pos2));
				DrtPred tmp_tense(string("@TIME(") + verb_str + "," + "past|before_past" + ")");
				tenses.push_back(tmp_tense);
			}
			// sign the auxiliary verb to be deleted
			implant_header(to_return.at(pos1), "AUX");
			add_header(to_return.at(pos2), ":POST_AUX");
			names.at(pos2) += ":POST_AUX";
			names.at(pos1) = "AUX";
		} else if (names.at(pos1) == "do" && tags.at(pos2).find("VB") != string::npos) {
			if (tags.at(pos1) == "VBP" || tags.at(pos1) == "VBZ") {
				string verb_str = extract_verb_name(to_return.at(pos2));
				DrtPred tmp_tense(string("@TIME(") + verb_str + "," + "present" + ")");
				tenses.push_back(tmp_tense);
			} else if (tags.at(pos1) == "VBD") {
				string verb_str = extract_verb_name(to_return.at(pos2));
				DrtPred tmp_tense(string("@TIME(") + verb_str + "," + "past" + ")");
				tenses.push_back(tmp_tense);
			}
			// sign the auxiliary verb to be deleted
			implant_header(to_return.at(pos1), "AUX");
			add_header(to_return.at(pos2), ":POST_AUX");
			names.at(pos2) += ":POST_AUX";
			names.at(pos1) = "AUX";
		} else if (names.at(pos1) == "have" && names.at(pos2).find("be") != string::npos) {
			if (tags.at(pos1) == "VB" || tags.at(pos1) == "VBP" || tags.at(pos1) == "VBZ") {
				string verb_str = extract_verb_name(to_return.at(pos2));
				implant_header(to_return.at(pos1), "AUX");
				add_header(to_return.at(pos2), ":POST_AUX");
				names.at(pos2) += ":POST_AUX";
				names.at(pos1) = "AUX";
			} else if (tags.at(pos1) == "VBD") {
				string verb_str = extract_verb_name(to_return.at(pos2));
				implant_header(to_return.at(pos1), "AUX");
				add_header(to_return.at(pos2), ":POST_AUX");
				names.at(pos2) += ":POST_AUX";
				names.at(pos1) = "AUX";
			}
		} else if (is_modal(tags.at(pos1)) && names.at(pos1) == "will") { // Process the future modal "will"
			string verb_str = extract_verb_name(to_return.at(pos2));
			string modal_str = extract_header(to_return.at(pos1));
			implant_header(to_return.at(pos1), "AUX");
			names.at(pos1) = "AUX";
			tenses.push_back(DrtPred("@TIME(" + verb_str + ",future)"));
		} else if (is_modal(tags.at(pos1))) { // Process the auxiliary verbs (should, may, ...)
			string verb_str = extract_verb_name(to_return.at(pos2));
			string modal_str = extract_header(to_return.at(pos1));
			implant_header(to_return.at(pos1), "AUX");
			names.at(pos1) = "AUX";
			tenses.push_back(DrtPred("@MODAL(" + verb_str + "," + modal_str + ")"));

		} else if (names.at(pos1) == "have:POST_AUX" && tags.at(pos2).find("VBN") != string::npos) {
			if (tags.at(pos1).find("VBN") != string::npos || tags.at(pos1) == "VB") {
				string verb_str = extract_verb_name(to_return.at(pos2));
				DrtPred tmp_tense(string("@TIME(") + verb_str + "," + "past|before_past" + ")");
				tenses.push_back(tmp_tense);
				// sign the auxiliary verb to be deleted
				implant_header(to_return.at(pos1), "AUX");
				add_header(to_return.at(pos2), ":POST_AUX");
				names.at(pos2) += ":POST_AUX";
				names.at(pos1) = "AUX";
			}
		} else if (names.at(pos1) == "be:POST_AUX" && tags.at(pos2).find("VBG") != string::npos) {
			if (tags.at(pos1).find("VBN") != string::npos) {
				string verb_str = extract_verb_name(to_return.at(pos2));
				DrtPred tmp_tense(string("@TIME(") + verb_str + "," + "past|continuative" + ")");
				tenses.push_back(tmp_tense);
				// sign the auxiliary verb to be deleted
				implant_header(to_return.at(pos1), "AUX");
				add_header(to_return.at(pos2), ":POST_AUX");
				names.at(pos2) += ":POST_AUX";
				names.at(pos1) = "AUX";
			}
		} else if (names.at(pos1) == "be:POST_AUX" && tags.at(pos2).find("VBN") != string::npos) {
			if (tags.at(pos1).find("VBN") != string::npos) {
				string verb_str = extract_verb_name(to_return.at(pos2));
				DrtPred tmp_tense(string("@TIME(") + verb_str + "," + "past|before_past" + ")");
				tenses.push_back(tmp_tense);

				implant_header(to_return.at(pos1), "PASSIVE_AUX");
				add_header(to_return.at(pos2), ":PASSIVE_POST_AUX");
				names.at(pos2) += ":PASSIVE_POST_AUX";
				names.at(pos1) = "PASSIVE_AUX";
			}
			if (tags.at(pos1).find("VBG") != string::npos) {
				string verb_str = extract_verb_name(to_return.at(pos2));
				DrtPred tmp_tense(string("@TIME(") + verb_str + "," + "past|before_past|continuative" + ")");
				tenses.push_back(tmp_tense);

				implant_header(to_return.at(pos1), "PASSIVE_AUX");
				add_header(to_return.at(pos2), ":PASSIVE_POST_AUX");
				names.at(pos2) += ":PASSIVE_POST_AUX";
				names.at(pos1) = "PASSIVE_AUX";
			}
		}
	}

	vector<string> passive_verbs;
	passive_verbs.push_back("know");
	passive_verbs.push_back("born");
	// Process single verbs
	for (int n = 0; n < tags.size(); ++n) {
		if (is_verb(tags.at(n))) {
			if (names.at(n).find("AUX") == string::npos) { // It must not be connected to an auxiliary verb
				if (tags.at(n) == "VB" || tags.at(n).find("VBP") != string::npos || tags.at(n).find("VBZ") != string::npos) {
					string verb_str = extract_verb_name(to_return.at(n));
					DrtPred tmp_tense(string("@TIME(") + verb_str + "," + "present" + ")");
					tenses.push_back(tmp_tense);
				} else if (tags.at(n).find("VBD") != string::npos) { /// Improve the tagging!!!!
					string verb_str = extract_verb_name(to_return.at(n));
					DrtPred tmp_tense(string("@TIME(") + verb_str + "," + "past" + ")");
					tenses.push_back(tmp_tense);
				} else if (tags.at(n).find("VBN") != string::npos) { /// Improve the tagging!!!!
					// The single VBNs conntected with "by ..." are all passive verbs ("like in paid by David")
					int pos_cause = find_cause(to_return, to_return.at(n));
					if (pos_cause != -1) {
						string verb_str = extract_verb_name(to_return.at(n));
						names.at(n) += ":PASSIVE_POST_AUX";
					} else if (shortfind(passive_verbs, names.at(n))) {
						string verb_str = extract_verb_name(to_return.at(n));
						names.at(n) += ":PASSIVE_POST_AUX";
					} else {
						string verb_str = extract_verb_name(to_return.at(n));
						to_return.at(n).tag() = "VBD";
						DrtPred tmp_tense(string("@TIME(") + verb_str + "," + "past" + ")");
						tenses.push_back(tmp_tense);
					}
				}
			}
		}
	}

	// Add the tenses and modals to the drt vector
	to_return.insert(to_return.end(), tenses.begin(), tenses.end());
	// Connects to the base verb to the elements connected to the
	// auxiliary verbs, and the base verb takes both the subject and
	// the object (if present) of the auxiliary
	for (int n = 0; n < verb_pairs.size(); ++n) {
		int pos1 = verb_pairs.at(n).first, pos2 = verb_pairs.at(n).second;
		if (extract_header(to_return.at(pos1)) != "AUX" && extract_header(to_return.at(pos1)) != "PASSIVE_AUX"
				&& extract_header(to_return.at(pos1)) != "MODAL")
			continue;
		vector<int> vcoupled = find_coupled_non_verb(pre_drt, tags, pos1);
		for (int i = 0; i < vcoupled.size(); ++i) {
			// substitute the tag to the auxiliary with a tag
			// pointing to where the auxiliary verb points
			to_return.at(vcoupled.at(i)) = substitute_with(to_return.at(vcoupled.at(i)), to_return.at(pos1),
					to_return.at(pos2));
		}
		// Implant the subject in the subordinate verb
		if (has_subject(to_return.at(pos1)) && !verb_subject(to_return.at(pos1)))
			to_return.at(pos2) = implant_subject(to_return.at(pos2), extract_subject(to_return.at(pos1)));
		if (has_subject(to_return.at(pos1)) && verb_subject(to_return.at(pos1)) && !has_subject(to_return.at(pos2))
		// If the AUX is connected to a verb and there is no other subject, take the subject from the verb pointed at by the AUX
				) {
			int m = find_verb_with_string(to_return, extract_subject(to_return.at(pos1)));
			to_return.at(pos2) = implant_subject(to_return.at(pos2), extract_subject(to_return.at(m)));
		}
		// Implant the object in the subordinate verb
		if (has_object(to_return.at(pos1)) && !has_object(to_return.at(pos2)) && !verb_object(to_return.at(pos1))) {
			implant_object(to_return.at(pos2), extract_object(to_return.at(pos1)));
		}
		//to_return.at(pos1)= implant_first( to_return.at(pos1), extract_first_tag(to_return.at(pos2)) );
		string to_subst = extract_first_tag(to_return.at(pos1));
		string subst_with = extract_first_tag(to_return.at(pos2));
		to_return = substitute_ref(to_return, to_subst, subst_with);
	}
	// The subject of a passive form becomes the object, the cause
	// (if present) becomes the subject. If the passive verb has an
	// object, this object becomes an indirect object of the active
	// form.
	vector<DrtPred> complements_to_add;

	for (int n = 0; n < tags.size(); ++n) {
		if (is_verb(tags.at(n)) && names.at(n).find(":PASSIVE_POST_AUX") != string::npos) {
			string obj_str = extract_subject(to_return.at(n));
			if (ref_is_verb(obj_str))
				obj_str = "obj"; // verbs cannot be objects at this level (only before collapse_verbs() )
			int pos_cause = find_cause(to_return, to_return.at(n));

			if (has_object(to_return.at(n))) {
				// string indirect_obj= obj_str;
				// obj_str= extract_object(to_return.at(n));
				string indirect_obj = extract_object(to_return.at(n));
				string verb_ref = extract_first_tag(to_return.at(n));
				DrtPred tmp_pred(string("@DATIVE(") + verb_ref + "," + indirect_obj + ")");
				tmp_pred.setName("as");
				complements_to_add.push_back(tmp_pred);
			}

			if (pos_cause != -1) {
				string subj_str = extract_second_tag(to_return.at(pos_cause));
				add_header(to_return.at(pos_cause), ":DELETE");
				add_header(to_return.at(pos_cause), ":DELETE");
				to_return.at(n) = implant_subject(to_return.at(n), subj_str);
				to_return.at(n) = implant_object(to_return.at(n), obj_str);
			} else {
				to_return.at(n) = implant_subject(to_return.at(n), "subj");
				to_return.at(n) = implant_object(to_return.at(n), obj_str);
			}
		}
	}

	if (debug) {
		puts("LAST_COLLAPSE:::");
		print_vector(to_return);
	}

	// Process the future tense. Up to this point, the future tense
	// is represented in two predicates @TIME(verbx,future) and
	// @TIME(verbx,tense1). Here the tense becomes only
	// @TIME(verbx,future|tense1). Notice that @TIME(verbx,future)
	// always comes first the other predicate
	vector<string> future_verbs;
	for (int n = 0; n < to_return.size(); ++n) {
		string head = extract_header(to_return.at(n));
		if (head == "@TIME") {
			string first = extract_first_tag(to_return.at(n));
			string second = extract_second_tag(to_return.at(n));
			if (second == "future") {
				future_verbs.push_back(first);
				head += ":DELETE";
				implant_header(to_return.at(n), head);
			} else if (find(future_verbs.begin(), future_verbs.end(), first) != future_verbs.end()) {
				string to_implant = "future|" + second;
				to_return.at(n) = implant_second(to_return.at(n), to_implant);
			}
		}
	}

	to_return.insert(to_return.end(), complements_to_add.begin(), complements_to_add.end());

	return to_return;
}

static DrtPred percolate_to_verb(vector<DrtPred> &pre_drt, const vector<string> &tags, int n)
/// Implement a recursive algorithm !!!
{
	if (pre_drt.at(n).is_verb())
		return pre_drt.at(n);

	DrtPred to_return(pre_drt.at(n));

	if (to_return.is_verb()) // do nothing
		return to_return;

	string from_str = extract_first_tag(pre_drt.at(n));
	if (ref_is_verb(from_str))
		return to_return;
	int m = find_verb_with_subject(pre_drt, tags, from_str);
	if (m == -1) {
		m = find_verb_with_object(pre_drt, tags, from_str);
	}
	if (m == -1) { // if there is no verb, try to find a preposition
		m = find_prep_with_target(pre_drt, tags, from_str);
	}
	if (m != -1) { // There might not be such a verb
		string verb_str = extract_first_tag(pre_drt.at(m));
		if (ref_is_verb(verb_str))
			to_return = implant_first(pre_drt.at(n), verb_str);
		else {
			m = find_verb_with_object(pre_drt, tags, verb_str);
			if (m == -1) // if there is no verb, try to find a preposition
				m = find_prep_with_target(pre_drt, tags, verb_str);
			if (m != -1) { // There might not be such a verb
				string verb_str = extract_first_tag(pre_drt.at(m));
				if (ref_is_verb(verb_str))
					to_return = implant_first(pre_drt.at(n), verb_str);
				else {
					m = find_verb_with_object(pre_drt, tags, verb_str);
					if (m == -1) // if there is no verb, try to find a preposition
						m = find_prep_with_target(pre_drt, tags, verb_str);
					if (m != -1) { // There might not be such a verb
						string verb_str = extract_first_tag(pre_drt.at(m));
						if (ref_is_verb(verb_str))
							to_return = implant_first(pre_drt.at(n), verb_str);
						else {
							m = find_verb_with_object(pre_drt, tags, verb_str);
							if (m == -1) // if there is no verb, try to find a preposition
								m = find_prep_with_target(pre_drt, tags, verb_str);
							if (m != -1) { // There might not be such a verb
								string verb_str = extract_first_tag(pre_drt.at(m));
								to_return = implant_first(pre_drt.at(n), verb_str);
							}
						}
					}
				}
			}
		}
	}
	return to_return;
}




static DrtPred percolate_second_tag_to_verb(vector<DrtPred> &pre_drt, const vector<string> &tags, int n)
{
	DrtPred to_return(pre_drt.at(n));

	if (to_return.is_verb()) // do nothing
		return to_return;

	string to_str = extract_second_tag(pre_drt.at(n));
	int m = find_verb_with_subject(pre_drt, tags, to_str);
	if (m == -1) {
		m = find_verb_with_object(pre_drt, tags, to_str);
	}
	if (m != -1) { // There might not be such a verb
		string verb_str = extract_first_tag(pre_drt.at(m));
		to_return = implant_first(pre_drt.at(n), verb_str);
	}
	return to_return;
}

static int percolate_second_tag_to_verb_integer(vector<DrtPred> &pre_drt, const vector<string> &tags, int n)
{
	if (is_verb(tags.at(n)))
		return n;

	DrtPred to_return(pre_drt.at(n));

	string to_str = extract_second_tag(pre_drt.at(n));
	int m = find_verb_with_string(pre_drt, tags, to_str);
	if (m == -1) {
		int m = find_verb_with_subject(pre_drt, tags, to_str);
	}
	if (m == -1) {
		m = find_verb_with_object(pre_drt, tags, to_str);
	}
	return m;
}

static int percolate_to_verb_integer(vector<DrtPred> &pre_drt, const vector<string> &tags, int n)
{
	if (is_verb(tags.at(n)))
		return n;

	DrtPred to_return(pre_drt.at(n));

	string from_str = extract_first_tag(pre_drt.at(n));
	int m = find_verb_with_subject(pre_drt, tags, from_str);
	if (m == -1) {
		m = find_verb_with_object(pre_drt, tags, from_str);
	}
	return m;
}

static int percolate_to_name_integer(vector<DrtPred> &pre_drt, const vector<string> &tags, int n)
{
	if (is_verb(tags.at(n)))
		return n;

	DrtPred to_return(pre_drt.at(n));

	string from_str = extract_first_tag(pre_drt.at(n));
	int m = find_element_with_string(pre_drt, from_str);
	if (m == -1) {
		m = find_verb_with_object(pre_drt, tags, from_str);
	}
	return m;
}

static DrtPred percolate_second_to_verb(vector<DrtPred> &pre_drt, const vector<string> &tags, int n)
{
	if (is_verb(tags.at(n)))
		return pre_drt.at(n);

	DrtPred to_return(pre_drt.at(n));

	string to_str = extract_second_tag(pre_drt.at(n));
	if (find_verb_with_string(pre_drt, to_str) != -1) // if the second tag already points to a verb do nothing
		return to_return;
	int m = find_verb_with_subject(pre_drt, tags, to_str);
	if (m == -1) {
		m = find_verb_with_object(pre_drt, tags, to_str);
	}
	if (m != -1) { // The check is necessary, there might not be such a verb
		string verb_str = extract_first_tag(pre_drt.at(m));
		to_return = implant_second(pre_drt.at(n), verb_str);
	}
	if (m != -1) { // The check is necessary, there might not be such a verb
		string verb_str = extract_first_tag(pre_drt.at(m));
		to_return = implant_second(pre_drt.at(n), verb_str);
	}
	return to_return;
}

static int get_connected_prev(const vector<DrtPred> &pre_drt, int pos)
{
	string prev_str = extract_first_tag(pre_drt.at(pos));

	for (int n = 0; n < pre_drt.size(); ++n) {
		string head_str = extract_header(pre_drt.at(n));
		if (head_str.size() && head_str.at(0) == '@')
			continue;
		string fref = extract_first_tag(pre_drt.at(n));
		if (fref == prev_str) {
			return n;
		}
	}
	return -1;
}

static int get_connected_next(const vector<DrtPred> &pre_drt, int pos)
{
	string next_str = extract_second_tag(pre_drt.at(pos));
	int to_return = -1;
	for (int n = 0; n < pre_drt.size(); ++n) {
		string head_str = extract_header(pre_drt.at(n));
		if ((head_str.size() && head_str.at(0) == '@') || pre_drt.at(n).tag() == "IN" || pre_drt.at(n).tag() == "TO")
			continue;
		string fref = extract_first_tag(pre_drt.at(n));
		if (fref == next_str && (pre_drt.at(n).is_verb() || pre_drt.at(n).is_WP() || pre_drt.at(n).is_pivot())) {
			to_return = n;
		}
	}
	return to_return;
}

static int get_connected_prior(const vector<DrtPred> &pre_drt, int pos)
{
	string prior_str = extract_first_tag(pre_drt.at(pos));

	for (int n = 0; n < pre_drt.size(); ++n) {
		string head_str = extract_header(pre_drt.at(n));
		if ((head_str.size() && head_str.at(0) == '@') || pre_drt.at(n).tag() == "DT")
			continue;
		string fref = extract_first_tag(pre_drt.at(n));
		if (fref == prior_str) {
			return n;
		}
	}
	return -1;
}

static bool is_motion_verb(const vector<DrtPred> &pre_drt, const vector<string> &tags, int pos)
{
	if (!pre_drt.at(pos).is_verb())
		return false;

	DrtPred verb = pre_drt.at(pos);

	metric *d = metric_singleton::get_metric_instance();
	string prior_name = extract_header(verb);

	if (d->get_levin_verb(prior_name) == "verb.motion")
		return true; /// it does not work for composite tenses ("is going")

	string obj = extract_object(verb);
	int m = find_verb_with_string(pre_drt, obj);
	if (m != -1) {
		verb = pre_drt.at(m);
		string prior_name = extract_header(verb);
		if (d->get_levin_verb(prior_name) == "verb.motion")
			return true; /// it does not work for composite tenses ("is going")

		obj = extract_object(verb);
		int m = find_verb_with_string(pre_drt, obj);
		if (m != -1) {
			verb = pre_drt.at(m);
			string prior_name = extract_header(verb);
			if (d->get_levin_verb(prior_name) == "verb.motion")
				return true; /// it does not work for composite tenses ("is going")
		}
	}
	return false;
}

static complements_type get_complements_type(const vector<DrtPred> &pre_drt, const vector<string> &tags, int pos)
{
	metric *d = metric_singleton::get_metric_instance();
	complements_type type;
	string str_name = extract_header(pre_drt.at(pos));
	int m = get_connected_next(pre_drt, pos);
	int prev = get_connected_prior(pre_drt, pos);
	if (m == -1)
		return type;
	string next_name = extract_header(pre_drt.at(m));
	string next_tag = pre_drt.at(m).tag();
	int colon_pos = next_name.find(":");
	if (colon_pos != string::npos)
		next_name = next_name.substr(0, colon_pos);

	bool is_writing_implement = false;
	if (d->hypernym(next_name, "writing_implement", 6) < 6)
		is_writing_implement = true;

	if (is_date(next_name) && (str_name == "on" || str_name == "in" || str_name == "at")) {
		type.set_time_at();
		return type;
	}

	if (str_name == "in" && is_writing_implement) {
		type.set_company();  /// create @TOOL complement!!
		return type;
	}

	if (str_name == "to" && pre_drt.at(m).is_verb()) { // && pre_drt.at(prev).is_verb()) {
		type.set_subord_end();
		return type;
	}

	bool motion_verb = false;
	if (prev != -1) {
		string prior_name = extract_header(pre_drt.at(prev));
		if (prior_name == "get")
			motion_verb = true; // "got to number one"
		else
			motion_verb = is_motion_verb(pre_drt, tags, prev);
	}

	if (str_name == "of") { // genitive
		type.set_genitive();
		return type;
	}

	if (str_name == "according-to") { // According to David, ...
		type.set_according_to();
		return type;
	}

	if (is_composed_prep(pre_drt.at(pos))) { // composed preposition like "in-and-beyond"
		type.set_place_at();
		return type;
	}

	if ( str_name == "than" || str_name == "like" || str_name == "such_as") { // comparative
		type.set_comparative();
		return type;
	}

	if (str_name == "more-than") { // comparative
		type.set_more_than();
		return type;
	}
	if (str_name == "less-than") { // comparative
		type.set_less_than();
		return type;
	}

	if (str_name == "\'s") { // saxon genitive
		type.set_sax_genitive();
		return type;
	}

	if (str_name == "by" && is_date(next_name)) { // by the mid-1990s
		type.set_time_at();
		return type;
	}

	if (str_name == "by" && is_clock(next_name)) { // by the mid-1990s
		type.set_clock_at();
		return type;
	}

	if (str_name == "by" && (tags.at(m) == "VBG" || is_name(tags.at(m)))) { // cause complement
		type.set_cause();
		return type;
	}

	if (str_name == "because" || str_name == "so") { // cause complement
		type.set_cause();
		return type;
	}

	if (str_name == "for" && is_verb(tags.at(m))) { // cause complement
		type.set_cause();
		return type;
	}

	if (str_name == "as" // "he acted as Prospero"
	&& pre_drt.at(m).is_proper_name()) {
		type.set_topic();
		return type;
	}

	if (str_name == "for") {
		if (tags.at(m) == "VBG" || is_name(tags.at(m)) || is_adjective(tags.at(m)) || is_article(tags.at(m))
				|| is_WP(tags.at(m))) { // cause complement
			type.set_motivation();
			return type;
		}
	}

	double dist_to_topic = 1 - d->jico_dist("theme", next_name, 6);
	double dist_to_event = 1 - d->jico_dist("event", next_name, 6);
	double dist_to_object = 1 - d->jico_dist("thing", next_name, 6);
	double dist_to_person = 1 - d->jico_dist("person", next_name, 6);
	double dist_to_space = 1 - d->jico_dist("place", next_name, 6);
	double dist_to_state = 1 - d->jico_dist("nation", next_name, 6);
	double dist_to_time = 1 - d->jico_dist("time", next_name, 6);

	if (next_name == "line")
		dist_to_space = 0; // "they moved on the French line"
	if (next_name == "what")
		dist_to_object = 0; // "what engaged what of what"
	if (pre_drt.at(m).tag() == "CD")
		dist_to_time = 0; // "what engaged what of what"
	if (next_name.find("century") != string::npos)
		dist_to_time = 0; // "from the 13th century"
	if (next_name.find("onward") != string::npos)
		dist_to_time = 0; // "from the 13th century onward"
	if (next_name.find("information") != string::npos)
		dist_to_space = 0; // "from the 13th century"
	if (next_name.find("group") != string::npos)
		dist_to_space = 0; // in close groups
	if (next_name.find("arena") != string::npos)
		dist_to_space = 0;
	if (next_name.find("system") != string::npos)
		dist_to_space = 0;
	if (next_name.find("bc") != string::npos)
		dist_to_time = 0;
	if (next_name.find("power") != string::npos)
		dist_to_space = 0;
	if (next_name.find("nation") != string::npos)
		dist_to_space = 0;
	if (next_name.find("water") != string::npos) // breathe under[@TOPIC->@PLACE_AT] water
			dist_to_space = 0;

	if (next_name.find("number") && str_name == "as") {
		type.set_topic();
		return type;
	}

	if (debug)
		cout << "COMPL_DIST:::" << dist_to_space << " " << dist_to_time << endl;

	if ((next_tag == "CD" || next_name == "%") && str_name == "up-to") {
		type.set_less_than();
		return type;
	}

	if (str_name == "under" && next_name != "water") {
		if (dist_to_topic < dist_to_space || dist_to_topic < dist_to_event) {
			type.set_topic();
			return type;
		}
	}

	if (str_name == "about") {
		if (dist_to_topic < dist_to_space || dist_to_topic < dist_to_event) {
			type.set_topic();
			return type;
		}
	}

	// some complements are always time-like
	if (is_date(next_name) || is_clock(next_name) || is_day_of_the_week(next_name)) { // it is a time-like complement
		if (str_name == "to") {
			type.set_time_to();
			return type;
		}
		if (str_name == "from") {
			type.set_time_from();
			return type;
		}
		if (str_name == "across" || str_name == "through" || str_name == "throughout") {
			type.set_time_through();
			return type;
		}
		if (str_name == "at" || str_name == "in" || str_name == "on") {
			if (is_clock(next_name))
				type.set_clock_at();
			else
				type.set_time_at();
			return type;
		}
		if (str_name == "between") {
			type.set_time_between();
			return type;
		}
	}

	if ((dist_to_object < dist_to_time && dist_to_object < dist_to_space && dist_to_object < dist_to_state)
			|| (dist_to_person < dist_to_time && dist_to_person < dist_to_space && dist_to_person < dist_to_state)) { // end complement
		if (str_name == "to") {
			type.set_end();
			return type;
		}
		if (str_name == "as") {
			type.set_topic();
			return type;
		}
	} else if ((dist_to_time < dist_to_space || is_date(next_name) || is_clock(next_name)) && !motion_verb) { // it is a time-like complement
		if (str_name == "to") {
			type.set_time_to();
			return type;
		}
		if (str_name == "from") {
			type.set_time_from();
			return type;
		}
		if (str_name == "across" || str_name == "through" || str_name == "throughout") {
			type.set_time_through();
			return type;
		}
		if (str_name == "at" || str_name == "in" || str_name == "on") {
			if (is_clock(next_name))
				type.set_clock_at();
			else
				type.set_time_at();
			return type;
		}
		if (str_name == "between") {
			type.set_time_between();
			return type;
		}
	}
	if (str_name == "to") {
		type.set_motion_to();
		return type;
	}
	if (str_name == "against") {
		type.set_motion_against();
		return type;
	}
	if (str_name == "from") {
		type.set_motion_from();
		return type;
	}
	if (str_name == "across" || str_name == "through" || str_name == "throughout") {
		type.set_motion_through();
		return type;
	}
	if (str_name == "at" || str_name == "in" || str_name == "on" || str_name == "out" || str_name == "under" /// you need other types of complement
	|| str_name == "below" /// you need other types of complement
	|| str_name == "over" /// you need other types of complement
	|| str_name == "near" /// you need other types of complement
	|| str_name == "around" /// you need other types of complement
	|| str_name == "up-to" /// you need other types of complement
	|| str_name == "between" /// you need other types of complement
	|| str_name == "behind" /// you need other types of complement
	|| str_name == "inside" /// you need other types of complement
	|| str_name == "upon" /// you need other types of complement
	|| str_name == "within" /// you need other types of complement
	|| str_name == "amid" /// you need other types of complement
	|| str_name == "amidst" /// you need other types of complement
	|| str_name == "above" /// you need other types of complement
	|| str_name == "in_and_out"
	|| str_name == "along" || str_name == "off") {
		type.set_place_at();
		return type;
	}
	if (str_name == "up" || str_name == "down") {
		type.set_motion_to();
		return type;
	}
	if (str_name == "by") {
		type.set_cause();
		return type;
	}
	if (str_name == "since") {
		type.set_time_from();
		return type;
	}
	if (str_name == "until") {
		type.set_time_before();
		return type;
	}
	if ( //str_name == "when"
		//||
	str_name == "as" || str_name == "while" || str_name == "during") {
		type.set_time_at();
		return type;
	}
	if (str_name == "with" || str_name == "among" || str_name == "alongside") {
		type.set_company();
		return type;
	}
	if (str_name == "though" || str_name == "without" || str_name == "except" || str_name == "other-than"
			|| str_name == "despite" || str_name == "outside" || str_name == "instead_of" ///
					) {
		type.set_exclusion();
		return type;
	}
	if (str_name == "after" || str_name == "beyond") {
		type.set_time_after();
		return type;
	}
	if (str_name == "before") {
		type.set_time_before();
		return type;
	}
	if (str_name == "about" || str_name == "topic" || str_name == "whether") {
		type.set_topic();
		return type;
	}
	if (str_name == "into" || str_name == "onto") {
		type.set_motion_to();
		return type;
	}
	if (str_name == "DUMMY-PREP-DATE") {
		type.set_time_at();
		return type;
	}
	if (str_name == "DUMMY-PREP-PLACE") {
		type.set_place_at();
		return type;
	}
	if (is_clock(next_name)) {
		type.set_clock_at();
		return type;
	}
	if (is_date(next_name)) {
		type.set_time_at();
		return type;
	}
	return type;
}

static bool points_to_place(const DrtVect &drtvect, const DrtPred &preposition_pred)
{
	string fref = extract_first_tag(preposition_pred);
	int m = find_complement_with_target(drtvect, fref, "@PLACE_AT");
	if (m == -1)
		return false;
	return true;
}

static bool points_to_PRP(const DrtVect &drtvect, const DrtPred &preposition_pred)
{
	string fref = extract_first_tag(preposition_pred);
	int m = find_name_with_string(drtvect, fref);
	if (m == -1)
		return false;
	if (drtvect.at(m).tag() == "PRP")
		return true;
	return false;
}

static bool points_to_date(const DrtVect &drtvect, const DrtPred &preposition_pred)
{
	string fref = extract_first_tag(preposition_pred);
	int m = find_name_with_string(drtvect, fref);
	if (m == -1)
		return false;
	if (drtvect.at(m).is_date())
		return true;
	return false;
}

static bool points_to_unit_of_measure(const DrtVect &drtvect, const DrtPred &preposition_pred)
{
	string fref = extract_first_tag(preposition_pred);
	vector<int> poz = find_all_element_with_string(drtvect, fref);
	if (poz.size() == 0)
		return false;
	bool is_unit = false, has_number = false;
	for (int n = 0; n < poz.size(); ++n) {
		if (is_unit_of_measure(extract_header(drtvect.at(poz.at(n))))) {
			if (debug) {
				cout << "MEASURE::: " << drtvect.at(poz.at(n)) << endl;
			}
			is_unit = true;
		}
		// the unit must be connected to a quantity: "he is 100 km from home."
		if (drtvect.at(poz.at(n)).tag() == "CD") {
			if (debug) {
				cout << "CD::: " << drtvect.at(poz.at(n)) << endl;
			}
			has_number = true;
		}
	}
	if (is_unit && has_number)
		return true;
	return false;
}

static vector<DrtPred> correct_double_dative(const vector<DrtPred> &pre_drt, const vector<string> &tags, const vector<string> &names,
		bool is_question)
{
	vector<DrtPred> to_return(pre_drt);
	for (int n = 0; n < pre_drt.size(); ++n) {
		if(extract_header(to_return.at(n)) == "@DATIVE") {
			string fref = extract_first_tag(to_return.at(n));
			string sref = extract_second_tag(to_return.at(n));
			vector<int> poz= find_all_compl_with_first_tag(to_return,tags,fref,"@DATIVE");
			if(poz.size() == 1)
				continue;
			int m = find_verb_with_string(to_return,fref);
			if (m == -1)
				continue;
			if( has_object(to_return.at(m)) )
				continue;
			for (int j=0; j < poz.size(); ++j) {
				int pos= poz.at(j);
				if (to_return.at(pos).name() == "") { // if the dative was created by add_indirect_object
					implant_object(to_return.at(m),sref);
					add_header(to_return.at(pos),":DELETE");
					break;
				}
			}
		}
	}

	return to_return;
}

static vector<DrtPred> name_complements(const vector<DrtPred> &pre_drt, const vector<string> &tags, const vector<string> &names,
		bool is_question)
{
	vector<DrtPred> to_return(pre_drt);
	vector<DrtPred> complements; // the complements_type is defined in complements.hpp

	for (int n = 0; n < tags.size(); ++n) {
		if (!is_preposition(tags.at(n)))
			continue;
		complements_type type = get_complements_type(to_return, tags, n);
		string head = extract_header(to_return.at(n));
		if (type.is_time_at()) {
			// Percolate the first tag to a verb. If there is no verb nothing changes
			if (!points_to_verb(to_return.at(n)))
				to_return.at(n) = percolate_to_verb(to_return, tags, n);
			string name1_pos_str = extract_first_tag(to_return.at(n));
			string name2_pos_str = extract_second_tag(to_return.at(n));
			string type_str = type.get_type();
			DrtPred tmp_compl(string("@") + type_str + "(" + name1_pos_str + "," + name2_pos_str + ")");
			tmp_compl.setName(head);
			implant_header(to_return.at(n), "PREP");
			complements.push_back(tmp_compl);
		} else if (type.is_place()) {
			if ((points_to_name(to_return.at(n)) || points_to_PRP(pre_drt, to_return.at(n)))
					&& !points_to_subject(pre_drt, to_return.at(n)) && !(points_to_place(complements, to_return.at(n)) // a person is in a car in Wellington in New Zealand
					&& type.is_place_at()) && !points_to_unit_of_measure(pre_drt, to_return.at(n)))
				to_return.at(n) = percolate_to_verb(to_return, tags, n); // If there is no verb nothing changes
			string verb_pos_str = extract_first_tag(to_return.at(n));
			string name_pos_str = extract_second_tag(to_return.at(n));
			string type_str = type.get_type();
			DrtPred tmp_compl(string("@") + type_str + "(" + verb_pos_str + "," + name_pos_str + ")");
			tmp_compl.setName(head);
			implant_header(to_return.at(n), "PREP");
			complements.push_back(tmp_compl);
		} else if (type.is_cause()) {
			if (!is_question && (points_to_name(to_return.at(n)) || points_to_PRP(pre_drt, to_return.at(n)))
					&& !points_to_subject(pre_drt, to_return.at(n)))
				to_return.at(n) = percolate_to_verb(to_return, tags, n); // If there is no verb nothing changes
			if (head == "so") {
				to_return.at(n) = percolate_to_verb(to_return, tags, n); // If there is no verb nothing changes
			}
			string verb_pos_str = extract_first_tag(to_return.at(n));
			string name_pos_str = extract_second_tag(to_return.at(n));
			string type_str = type.get_type();
			DrtPred tmp_compl(string("@") + type_str + "(" + verb_pos_str + "," + name_pos_str + ")");
			if (head == "so") // "so" is the opposite of "because"
				switch_children(tmp_compl);
			tmp_compl.setName(head);
			implant_header(to_return.at(n), "PREP");
			complements.push_back(tmp_compl);
		} else if (type.is_time() || type.is_end() || type.is_topic() || type.is_motivation() || type.is_company()
				|| type.is_according_to()) {
			if ((points_to_name(to_return.at(n)) || !points_to_PRP(pre_drt, to_return.at(n)))
					&& !points_to_subject(pre_drt, to_return.at(n)))
				to_return.at(n) = percolate_to_verb(to_return, tags, n); // If there is no verb nothing changes
			string verb_pos_str = extract_first_tag(to_return.at(n));
			string name_pos_str = extract_second_tag(to_return.at(n));
			string type_str = type.get_type();
			DrtPred tmp_compl(string("@") + type_str + "(" + verb_pos_str + "," + name_pos_str + ")");
			tmp_compl.setName(head);
			implant_header(to_return.at(n), "PREP");
			complements.push_back(tmp_compl);
		} else if (type.is_subord_end()) {
			string fpos_str = extract_first_tag(to_return.at(n));
			if (!points_to_verb(fpos_str))
				to_return.at(n) = percolate_to_verb(to_return, tags, n); // If there is no verb nothing changes
			string verb_pos_str = extract_first_tag(to_return.at(n));
			string name_pos_str = extract_second_tag(to_return.at(n));
			string type_str = type.get_type();
			DrtPred tmp_compl(string("@") + type_str + "(" + verb_pos_str + "," + name_pos_str + ")");
			tmp_compl.setName(head);
			implant_header(to_return.at(n), "PREP");
			complements.push_back(tmp_compl);
		}
		else if (type.is_allocution() || type.is_exclusion()
	              || type.is_comparative() || type.is_more_than() || type.is_less_than() // I like that more than a piano falling on my head
				) {
			DrtPred orig_pred= to_return.at(n);
			to_return.at(n) = percolate_second_to_verb(to_return, tags, n); // If there is no verb nothing changes
			if(close_loop_str(to_return,extract_first_tag(to_return.at(n)),extract_second_tag(to_return.at(n))))
				to_return.at(n) = orig_pred;
			string name1_pos_str = extract_first_tag(to_return.at(n));
			string name2_pos_str = extract_second_tag(to_return.at(n));
			string type_str = type.get_type();
			DrtPred tmp_compl(string("@") + type_str + "(" + name1_pos_str + "," + name2_pos_str + ")");
			tmp_compl.setName(head);
			implant_header(to_return.at(n), "PREP");
			complements.push_back(tmp_compl);
		} else if (type.is_genitive()) {
			if (points_to_date(to_return, to_return.at(n)))
				to_return.at(n) = percolate_to_verb(to_return, tags, n); // If there is no verb nothing changes
			string name1_pos_str = extract_first_tag(to_return.at(n));
			string name2_pos_str = extract_second_tag(to_return.at(n));
			string type_str = type.get_type();
			DrtPred tmp_compl(string("@") + type_str + "(" + name1_pos_str + "," + name2_pos_str + ")");
			tmp_compl.setName(head);
			implant_header(to_return.at(n), "PREP");
			complements.push_back(tmp_compl);
		} else if (type.is_sax_genitive()) {
			string name1_pos_str = extract_first_tag(to_return.at(n));
			string name2_pos_str = extract_second_tag(to_return.at(n));
			string type_str = type.get_type();
			DrtPred tmp_compl(string("@GENITIVE") + "(" + name2_pos_str + "," + name1_pos_str + ")");
			tmp_compl.setName(head);
			implant_header(to_return.at(n), "PREP");
			complements.push_back(tmp_compl);
		}
	}

	to_return.insert(to_return.end(), complements.begin(), complements.end());

	return to_return;
}

static DrtVect percolate_no_DT_to_verb(DrtVect pre_drt, vector<string> &tags, vector<string> &names)
{
	for (int n = 0; n < pre_drt.size(); ++n) {
		string fref = extract_first_tag(pre_drt.at(n));
		string header = extract_header(pre_drt.at(n));
		if (pre_drt.at(n).tag() == "DT" && header == "no") {
			pre_drt.at(n).setTag("RB");
			implant_header(pre_drt.at(n), "not");
			pre_drt.at(n) = percolate_to_verb(pre_drt, tags, n);
		}
	}
	return pre_drt;
}

static vector<DrtPred> process_impersonal(const vector<DrtPred> &pre_drt, const vector<string> &tags, const vector<string> &names)
// the impersonal PRP is connected to the next "that". For example, "it" as a subject for a passive form
{
	vector<DrtPred> to_return(pre_drt);

	vector<pair<string, string> > ref_pair;

	for (int n = 0; n < tags.size(); ++n) {
		if (is_PRP(tags.at(n)) && names.at(n) == "it") {
			string subj = extract_first_tag(to_return.at(n));
			int m = find_verb_with_subject(to_return, tags, subj);
			if (m != -1) {
				string head_verb = extract_header(to_return.at(m));
				if (head_verb == "be") {
					string passive_str = extract_object(to_return.at(m));
					int m2 = find_verb_with_string(to_return, tags, passive_str);
					if (m2 != -1 && m2 < tags.size() && tags.at(m2) == "VBN") {
						// Impersonal "it" !!
						int pos_that = n;
						for (; pos_that < tags.size(); ++pos_that) {
							if (names.at(pos_that).find("that") != string::npos) {
								add_header(to_return.at(n), ":DELETE");
								to_return.at(n).name() += ":DELETE";
								string ref_it = extract_first_tag(to_return.at(n));
								string ref_that = extract_second_tag(to_return.at(pos_that));
								implant_subject(to_return.at(m2), ref_it);
								ref_pair.push_back(make_pair(ref_it, ref_that));
								break;
							}
						}
					}
				}
			}
		}
	}

	for (int n = 0; n < ref_pair.size(); ++n) {
		string ref = ref_pair.at(n).first;
		string name = ref_pair.at(n).second;
		to_return = substitute_ref(to_return, ref, name);
	}

	return to_return;
}

class coord_type {
	string type_;
public:
	string get_type()
	{
		return type_;
	}

	void set_conjunction()
	{
		type_ = "CONJUNCTION";
	}
	bool is_conjunction()
	{
		return type_ == "CONJUNCTION";
	}
	void set_coordination()
	{
		type_ = "COORDINATION";
	}
	bool is_coordination()
	{
		return type_ == "COORDINATION";
	}
	void set_disjunction()
	{
		type_ = "DISJUNCTION";
	}
	bool is_disjunction()
	{
		return type_ == "DISJUNCTION";
	}
	void set_conditional()
	{
		type_ = "CONDITION";
	}
	bool is_conditional()
	{
		return type_ == "CONDITION";
	}
};

static bool is_coord_exception(const string &prev_name, const string &conj_name)
{
	map<string, vector<string> > exc;
	exc["if"].push_back("wonder");
	exc["when"].push_back("wonder");

	map<string, vector<string> >::iterator exc_iter;
	vector<string>::iterator vect_iter;

	exc_iter = exc.find(conj_name);
	if (exc_iter != exc.end()) {
		vect_iter = find(exc_iter->second.begin(), exc_iter->second.end(), prev_name);
		if (vect_iter != exc_iter->second.end())
			return true;
	}
	return false;
}

static coord_type get_coord_type(const vector<DrtPred> &pre_drt, const vector<string> &tags, int pos)
{
	if (pos >= pre_drt.size())
		return coord_type();
	string str_name = extract_header(pre_drt.at(pos));
	coord_type type;
	int mp = get_connected_prev(pre_drt, pos);
	int mn = get_connected_next(pre_drt, pos);
	if (mp == -1 || mn == -1 || mp >= tags.size() || mn >= tags.size())
		return type;
	string prev_name = extract_header(pre_drt.at(mp));
	string next_name = extract_header(pre_drt.at(mn));

	if (str_name == "if") { // conditional form
		if (!is_coord_exception(prev_name, str_name)) {
			// Example: in "wonder if" the "if" is not a conditional conjunction
			type.set_conditional();
			return type;
		}
	}
	if (is_verb(tags.at(mp)) || is_verb(tags.at(mn))) {
		if (str_name == "and" || str_name == "-comma-") { // conjunction
			type.set_conjunction();
			return type;
		} else if (str_name == "or") { // conjunction
			type.set_coordination();
			return type;
		}
		if (str_name == "but" || str_name == "although") { // disjunction
			type.set_disjunction();
			return type;
		}
	}
	return type;
}

static vector<DrtPred> substitute_subj_or_obj(const string &first_verb_pos_str, const string &second_verb_pos_str,
		vector<DrtPred> pre_drt, const vector<string> &tags)
{
	int first_pos = find_verb_with_string(pre_drt, first_verb_pos_str);
	int second_pos = find_verb_with_string(pre_drt, second_verb_pos_str);

	if (first_pos == -1 || second_pos == -1)
		return pre_drt;

	if (!pre_drt.at(first_pos).is_verb() || !pre_drt.at(second_pos).is_verb()) // further check that you are going to modify only verbs
		return pre_drt;

	string subj2 = extract_subject(pre_drt.at(second_pos));
	string obj2 = extract_object(pre_drt.at(second_pos));
	if (subj2.find("subj") != string::npos) {
		string subj1 = extract_subject(pre_drt.at(first_pos));
		pre_drt.at(second_pos) = implant_subject(pre_drt.at(second_pos), subj1);
	} else if (obj2.find("obj") != string::npos) {
		string obj1 = extract_subject(pre_drt.at(first_pos));
		pre_drt.at(second_pos) = implant_object(pre_drt.at(second_pos), obj1);
	}

	return pre_drt;
}

static vector<DrtPred> substitute_subj_with_name(const string &first_verb_pos_str, const string &second_verb_pos_str,
		vector<DrtPred> pre_drt, const vector<string> &tags)
// substitute the subject or object if the verb is not associated to a name.
// does not make the substitution if the candidate obj match the subj.
{
	int first_pos = find_verb_with_string(pre_drt, tags, first_verb_pos_str);
	int second_pos = find_verb_with_string(pre_drt, tags, second_verb_pos_str);

	if (first_pos == -1 || second_pos == -1)
		return pre_drt;

	if (!pre_drt.at(first_pos).is_verb() || !pre_drt.at(second_pos).is_verb()) // further check that you are going to modify only verbs
		return pre_drt;

	string subj2 = extract_subject(pre_drt.at(second_pos));
	string obj2 = extract_object(pre_drt.at(second_pos));
	if (subj2.find("subj") != string::npos || subj2.find("verb") != string::npos) {
		string subj1 = extract_subject(pre_drt.at(first_pos));
		if (subj1 != obj2)
			pre_drt.at(second_pos) = implant_subject(pre_drt.at(second_pos), subj1);
	}

	return pre_drt;
}

static vector<DrtPred> substitute_passive_subj_with_name(const string &first_verb_pos_str, const string &second_verb_pos_str,
		vector<DrtPred> pre_drt, const vector<string> &tags)
// substitute the subject or object if the verb is not associated to a name.
// does not make the substitution if the candidate obj match the subj.
{
	int first_pos = find_verb_with_string(pre_drt, tags, first_verb_pos_str);
	int second_pos = find_verb_with_string(pre_drt, tags, second_verb_pos_str);

	if (first_pos == -1 || second_pos == -1)
		return pre_drt;

	bool second_is_passive = false;
	string second_head = extract_header(pre_drt.at(second_pos));
	if (second_head.find("PASSIVE") != string::npos)
		second_is_passive = true;

	if (!pre_drt.at(first_pos).is_verb() || !pre_drt.at(second_pos).is_verb()) // further check that you are going to modify only verbs
		return pre_drt;

	string subj2 = extract_subject(pre_drt.at(second_pos));
	string obj2 = extract_object(pre_drt.at(second_pos));
	if (!has_subject(pre_drt.at(second_pos))) {
		string subj1 = extract_object(pre_drt.at(first_pos));
		if (subj1 != obj2) {
			if (second_is_passive) {
				if(!has_object(pre_drt.at(second_pos)) )
					pre_drt.at(second_pos) = implant_object(pre_drt.at(second_pos), subj1);
			} else {
				pre_drt.at(second_pos) = implant_subject(pre_drt.at(second_pos), subj1);
			}
		}
	}

	return pre_drt;
}

static vector<DrtPred> substitute_subj_or_obj_with_name(const string &first_verb_pos_str, const string &second_verb_pos_str,
		vector<DrtPred> pre_drt, const vector<string> &tags)
// substitute the subject or object if the verb is not associated to a name.
// does not make the substitution if the candidate obj match the subj.
{
	int first_pos = find_verb_with_string(pre_drt, tags, first_verb_pos_str);
	int second_pos = find_verb_with_string(pre_drt, tags, second_verb_pos_str);

	if (first_pos == -1 || second_pos == -1)
		return pre_drt;

	if (!pre_drt.at(first_pos).is_verb() || !pre_drt.at(second_pos).is_verb()) // further check that you are going to modify only verbs
		return pre_drt;

	string subj2 = extract_subject(pre_drt.at(second_pos));
	string obj2 = extract_object(pre_drt.at(second_pos));
	if (subj2.find("subj") != string::npos || subj2.find("verb") != string::npos) {
		string subj1 = extract_subject(pre_drt.at(first_pos));
		if (subj1 != obj2)
			pre_drt.at(second_pos) = implant_subject(pre_drt.at(second_pos), subj1);
	} else if (obj2.find("obj") != string::npos || obj2.find("verb") != string::npos) {
		string obj1 = extract_subject(pre_drt.at(first_pos));
		if (obj1 != subj2)
			pre_drt.at(second_pos) = implant_object(pre_drt.at(second_pos), obj1);
	}

	return pre_drt;
}

static vector<DrtPred> substitute_subj_or_obj_with_string(const string &subst_string, const string &second_verb_pos_str,
		vector<DrtPred> pre_drt, const vector<string> &tags)
// does not make the substitution if the candidate obj match the subj
{
	int second_pos = find_verb_with_string(pre_drt, tags, second_verb_pos_str);

	if (second_pos == -1)
		return pre_drt;

	string subj2 = extract_subject(pre_drt.at(second_pos));
	string obj2 = extract_object(pre_drt.at(second_pos));
	if (subj2.find("subj") != string::npos && subst_string != obj2) {
		pre_drt.at(second_pos) = implant_subject(pre_drt.at(second_pos), subst_string);
	} else if (obj2.find("obj") != string::npos && subst_string != subj2) {
		//cout << "OBJ::: " << subst_string << ", " << obj2<< endl;
		pre_drt.at(second_pos) = implant_object(pre_drt.at(second_pos), subst_string);
	}

	return pre_drt;
}

static int has_own(const vector<DrtPred> &pre_drt, int n)
{
	string owned_str = extract_first_tag(pre_drt.at(n));

	for (int n = 0; n < pre_drt.size(); ++n) {
		string name = extract_header(pre_drt.at(n));
		;
		string child = extract_first_tag(pre_drt.at(n));
		if (name == "own" && child == owned_str)
			return n;
	}
	return -1;
}

static vector<int> find_prior_of_string_first(const vector<DrtPred> &pre_drt, const vector<string> &tags, const string &str)
{
	vector<int> ret_m;
	int m = 0;

	vector<DrtPred>::const_iterator diter = pre_drt.begin();

	for (; diter != pre_drt.end(); ++diter, ++m) {
		string first_child = diter->extract_children().front();
		string last_child = diter->extract_children().back();
		if (first_child != str && last_child == str)
			ret_m.push_back(m);
	}
	return ret_m;
}
static vector<int> find_prior_of_string_continue(const vector<DrtPred> &pre_drt, const vector<string> &tags, const string &str)
{
	vector<int> ret_m;
	int m = 0;

	vector<DrtPred>::const_iterator diter = pre_drt.begin();

	for (; diter != pre_drt.end(); ++diter, ++m) {
		string first_child = extract_first_tag(*diter);
		if (diter->is_verb()) {
			if (first_child == str)
				ret_m.push_back(m);
		} else {
			string last_child = diter->extract_children().back();
			if (last_child == str)
				ret_m.push_back(m);
		}
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

static string percolate_to_first_name(const vector<DrtPred> &pre_drt, const vector<string> &tags, int n)
{
	string ret_str("none");
	string owned = extract_first_tag(pre_drt.at(n));

	vector<int> priors = find_prior_of_string_first(pre_drt, tags, owned);
	int m = 0;
	while (m < priors.size() && priors.size() != 0 && m < pre_drt.size()) {
		if (priors.at(m) > 0 && priors.at(m) < pre_drt.size()) {

			if (pre_drt.at(priors.at(m)).is_name()) {
				ret_str = extract_first_tag(pre_drt.at(priors.at(m)));
				return ret_str;
			} else if (pre_drt.at(priors.at(m)).is_verb()) {
				string subj = extract_subject(pre_drt.at(priors.at(m)));
				vector<int> priors_tmp = find_prior_of_string_continue(pre_drt, tags, subj);
				priors_tmp = delete_duplicates(priors_tmp, priors.begin(), priors.end());
				priors.insert(priors.end(), priors_tmp.begin(), priors_tmp.end());
			} else {
				string from = extract_first_tag(pre_drt.at(priors.at(m)));
				vector<int> priors_tmp = find_prior_of_string_continue(pre_drt, tags, from);
				priors_tmp = delete_duplicates(priors_tmp, priors.begin(), priors.end());
				priors.insert(priors.end(), priors_tmp.begin(), priors_tmp.end());
			}
		}
		++m;
	}
	return ret_str;
}

static vector<string> get_reference_string(string ref_str)
{
	vector<string> ret_str;
	if (ref_str == "he" || ref_str == "his" || ref_str == "him") {
		ret_str.push_back("man");
		ret_str.push_back("person");
		ret_str.push_back("male_person");
		ret_str.push_back("adult");
		ret_str.push_back("educator");
	} else if (ref_str == "she" || ref_str == "her") {
		ret_str.push_back("woman");
		ret_str.push_back("person");
		ret_str.push_back("female_person");
		ret_str.push_back("adult");
		ret_str.push_back("educator");
	} else if (ref_str == "it" || ref_str == "its") {
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
		ret_str.push_back("snake");
	}
	if (ref_str == "his-or-her") {
		ret_str.push_back("man");
		ret_str.push_back("person");
		ret_str.push_back("male_person");
		ret_str.push_back("female_person");
		ret_str.push_back("adult");
		ret_str.push_back("educator");
	} else
		ret_str.push_back(ref_str);
	return ret_str;
}

static vector<string> get_WP_reference_string(string ref_str)
{
	vector<string> ret_str;
	if (ref_str == "who" || ref_str == "whom") {
		ret_str.push_back("man");
		ret_str.push_back("royal");
		ret_str.push_back("dynasty");
		ret_str.push_back("politician");
		ret_str.push_back("professional");
		ret_str.push_back("person");
		ret_str.push_back("male_person");
		ret_str.push_back("female_person");
		ret_str.push_back("ethnic_group");
	} else if (ref_str == "which") {
		ret_str.push_back("!person");
		ret_str.push_back("dance");
		ret_str.push_back("professional");
		ret_str.push_back("thing");
		ret_str.push_back("basis");
		ret_str.push_back("material");
		ret_str.push_back("domestic_animal");
		ret_str.push_back("beast");
		ret_str.push_back("branch");
		ret_str.push_back("bird");
		ret_str.push_back("agent");
		ret_str.push_back("step");
		ret_str.push_back("cap");
		ret_str.push_back("serie");
		ret_str.push_back("installment");
		ret_str.push_back("snake");
		ret_str.push_back("station");
		ret_str.push_back("planet");
		ret_str.push_back("moon");
		ret_str.push_back("star");
		ret_str.push_back("war");
		ret_str.push_back("tool");
		ret_str.push_back("school");
		ret_str.push_back("way");
	}
	if (ref_str == "that") {
		ret_str.push_back("!person");
		ret_str.push_back("thing");
		ret_str.push_back("school");
		ret_str.push_back("tool");
		ret_str.push_back("basis");
		ret_str.push_back("medal");
		//ret_str.push_back("material");
		ret_str.push_back("domestic_animal");
		ret_str.push_back("beast");
		ret_str.push_back("bird");
		ret_str.push_back("agent");
		ret_str.push_back("man");
		ret_str.push_back("child");
		ret_str.push_back("stone");
		ret_str.push_back("person");
		ret_str.push_back("male_person");
		ret_str.push_back("step");
		ret_str.push_back("cap");
		ret_str.push_back("serie");
		ret_str.push_back("installment");
		ret_str.push_back("snake");
		ret_str.push_back("station");
		ret_str.push_back("planet");
		ret_str.push_back("moon");
		ret_str.push_back("star");
		ret_str.push_back("war");
		ret_str.push_back("tool");
		ret_str.push_back("way");
	} else
		ret_str.push_back(ref_str);
	return ret_str;
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
	for (; liter != lend; ++liter)
		for (; riter != rend; ++riter) {
			if (*liter == *riter) {
				distance = 1;
				continue;
			}
			if(*riter == "person" && d->gender_proper_name(*liter) != "")
				return 1;
			if(*riter == "man" && d->gender_proper_name(*liter) == "male")
				return 1;
			if(*riter == "woman" && d->gender_proper_name(*liter) == "female")
				return 1;

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
				distance = max(distance, d->hypernym_dist(lstr, rstr, 5));
			} else {
				distance = max(distance, 1 - d->hypernym_dist(lstr, rstr, 5));
				invert = false;
			}
		}

	return distance;
}

static double get_pred_string_vector_distance(metric *d, const DrtPred &pred, const vector<string> ref_strs)
/// This function should just be in Match.cpp
{
	string head_str = extract_header(pred);

	return get_string_vector_distance(d, head_str, ref_strs);
}

static bool is_subject_of_verb(const DrtVect drtvect, const string ref)
{
	for(int n=0; n < drtvect.size(); ++n) {
		if(!drtvect.at(n).is_verb() )
			continue;
		string sref= extract_subject(drtvect.at(n) );
		if (sref == ref)
			return true;
	}
	return false;
}

static bool is_object_of_verb(const DrtVect drtvect, const string ref)
{
	for(int n=0; n < drtvect.size(); ++n) {
		if(!drtvect.at(n).is_verb() )
			continue;
		string oref= extract_object(drtvect.at(n) );
		if (oref == ref)
			return true;
	}
	return false;
}


static string find_WP_matching_previous_name(const vector<DrtPred> &pre_drt, const vector<string> &tags, int n,
		const string &name, bool need_plural)
{
	string owner_str;

	vector<string> candidates = get_WP_reference_string(name);
	metric *d = metric_singleton::get_metric_instance();
	// Tries to find the closes subject or object
	bool already_assigned= false;
	double old_w = -1;
	for (int j = n; j >= 0; --j) {
		if (!pre_drt.at(j).is_name())
			continue;
		string fref= extract_first_tag(pre_drt.at(j) );
		if (!is_subject_of_verb(pre_drt, fref) && !is_object_of_verb(pre_drt, fref))
			continue;
		double w = get_pred_string_vector_distance(d, pre_drt.at(j), candidates);
		if (debug) {
			cout << "WP_MATCH1::: " << w << " " << j << " " << need_plural << " " << old_w << " " << name << endl;
		}
		bool is_plural = pre_drt.at(j).is_plural();
		if (need_plural && is_plural != need_plural)
			continue;
		if (w > 0.8 && w > old_w) {
			owner_str = extract_first_tag(pre_drt.at(j));
			already_assigned = true;
			old_w = w;
			if (debug) {
				cout << "WP_MATCH2::: " << w << " " << owner_str << endl;
			}
		}
	}
	// Tries all the rest
	old_w = -1;
	for (int j = n; j >= 0 && !already_assigned; --j) {
		if (!pre_drt.at(j).is_name())
			continue;
		if (is_specification_end(pre_drt, j))
			continue;
		double w = get_pred_string_vector_distance(d, pre_drt.at(j), candidates);
		if (debug) {
			cout << "WP_MATCH1::: " << w << " " << j << " " << need_plural << " " << old_w << " " << name << endl;
		}
		bool is_plural = pre_drt.at(j).is_plural();
		if (need_plural && is_plural != need_plural)
			continue;
		if (w > 0.8 && w > old_w) {
			owner_str = extract_first_tag(pre_drt.at(j));
			old_w = w;
			if (debug) {
				cout << "WP_MATCH2::: " << w << " " << owner_str << endl;
			}
		}
	}

// If the WP needs a plural then select the closest plural
	if (owner_str == "" && need_plural) {
		for (int j = n; j >= 0; --j) {
			if (!pre_drt.at(j).is_name())
				continue;
			bool is_plural = pre_drt.at(j).is_plural();
			if (is_plural) {
				owner_str = extract_first_tag(pre_drt.at(j));
				break;
			}
		}
	}

	return owner_str;
}

static string find_matching_previous_name(vector<DrtPred> &pre_drt, const vector<string> &tags, int n, const string &name, bool need_plural)
{
	string owner_str;

	vector<string> candidates = get_reference_string(name);
	metric *d = metric_singleton::get_metric_instance();

	bool plural_trigger = false;
	if (name == "their" || name == "they" || name == "them")
		plural_trigger = true;

	for (int j = n; j >= 0; --j) {
		if (!pre_drt.at(j).is_name() || !pre_drt.at(j).is_pivot())
			continue;
		if (plural_trigger && pre_drt.at(j).is_plural()) {
			owner_str = extract_first_tag(pre_drt.at(j));
			return owner_str;
		}
		double w = 0;
		if ( (name == "his" ) && d->gender_proper_name(extract_header(pre_drt.at(j))) != "male")
			w = 1;
		else if ( (name == "her") && d->gender_proper_name(extract_header(pre_drt.at(j))) != "female")
			w = 1;
		else
			w = get_pred_string_vector_distance(d, pre_drt.at(j), candidates);
		if (w > 0.8) {
			owner_str = extract_first_tag(pre_drt.at(j));
			return owner_str;
		}
	}

	return owner_str;
}

static vector<DrtPred> donkey_phrases(vector<DrtPred> pre_drt, const vector<string> &tags, const vector<string> &names)
{
	for (int n = 0; n < tags.size(); ++n) {
		if (tags.at(n) == "PRP$") {
			//pre_drt.at(n).pred().begin()->str += ":DELETE";
			string name = names.at(n);
			string owned_str = extract_first_tag(pre_drt.at(n));
			bool need_plural= false;
			string owner_str = find_matching_previous_name(pre_drt, tags, n, name, need_plural);
			int own_pos = has_own(pre_drt, n);
			if (own_pos == -1) {
				if (names.at(n) == "his")
					owner_str = "man" + boost::lexical_cast<string>(n) + "|" + owner_str;
				else if (names.at(n) == "her")
					owner_str = "woman" + boost::lexical_cast<string>(n) + "|" + owner_str;
				else if (names.at(n) == "his-or-her")
					owner_str = "man" + boost::lexical_cast<string>(n) + "|" + owner_str;
				else if (names.at(n) == "its")
					owner_str = "thing" + boost::lexical_cast<string>(n) + "|" + owner_str;
				else if (names.at(n) == "their")
					owner_str = "plural" + boost::lexical_cast<string>(n) + "|" + owner_str;
				add_header(pre_drt.at(n), ":DELETE");
			} else {
				add_header(pre_drt.at(own_pos), ":DELETE");
			}
			if (owner_str != "" && owned_str != "")
				pre_drt.push_back(DrtPred(string("@OWN(" + owner_str + "," + owned_str + ")")));
		}
	}
	return pre_drt;
}

static bool has_plural_verb(DrtVect &drtvect, const vector<string> &tags, int pos)
{
	string fref = extract_first_tag(drtvect.at(pos));
	int m = find_verb_with_subject(drtvect, tags, fref);
	if (m != -1) {
		bool is_passive = false;
		string oref = extract_object(drtvect.at(m));
		string header = extract_header(drtvect.at(m));
		if (ref_is_verb(oref) && (header == "be" || header == "have"))
			is_passive = true;
		if (drtvect.at(m).tag() == "VBP" && !is_passive)
			return true;
	}
	return false;
}

static string find_closest_prior_WP(DrtVect &drtvect, int pos)
{
	for (int n = pos - 1; n >= 0; --n) {
		if (drtvect.at(n).is_WP() || drtvect.at(n).is_WDT())
			return extract_first_tag(drtvect.at(n));
	}
	return "";
}

static vector<DrtPred> resolve_and_to(vector<DrtPred> to_return, const vector<string> &tags, vector<string> names,
		bool is_question)
// and/CC(verb1,to2) to/TO(to2,verb2) -> and/CC(verb1,verb2) to:DELETE/TO(to2,verb2)
{
	for (int n = 0; n < to_return.size(); ++n) {
		string fref= extract_first_tag (to_return.at(n) );
		string sref= extract_second_tag(to_return.at(n) );
		if(debug) {
			cout << "TO0::" << to_return.at(n) << endl;
		}
		if( is_conj(to_return.at(n).tag()) && ref_is_verb(fref)
				&& !has_second_tag(to_return.at(n))
		) {
			int m = find_prep_with_first_tag(to_return,tags,sref,"to");
			if(debug) {
				cout << "TO::" << sref << endl;
			}
			if(m == -1)
				continue;
			string sref2= extract_second_tag(to_return.at(m) );
			implant_second(to_return.at(n), sref2);
			add_header(to_return.at(m),":DELETE");
		}
	}

	return to_return;
}

static vector<DrtPred> resolve_WDT_lazy(vector<DrtPred> &pre_drt, const vector<string> &tags, vector<string> names,
		bool is_question)
{
	vector<DrtPred> to_return(pre_drt);
	vector<pair<string, string> > ref_pair;
	for (int n = 0; n < to_return.size(); ++n) {
		if (to_return.at(n).is_WDT() || to_return.at(n).is_WP()) {
			string head = extract_header(to_return.at(n));
			if (is_question && head == "whom") // "who is the brother of whom?"
				continue;
			string pointed_ref;
			bool need_plural = has_plural_verb(to_return, tags, n);
			if (head == "[prior_WP]") {
				pointed_ref = find_closest_prior_WP(pre_drt, n);
			} else {
				pointed_ref = find_WP_matching_previous_name(pre_drt, tags, n, names.at(n), need_plural);
			}
			if (debug) {
				cout << "WDT_LAZY2::: " << pointed_ref << endl;
			}
			if (pointed_ref == "" && !need_plural) {
				// If nothing was found try to search for a plural
				need_plural = true;
				string pointed_ref = find_WP_matching_previous_name(pre_drt, tags, n, names.at(n), need_plural);
			}
			if (pointed_ref == "")
				continue;
			string fstr = extract_first_tag(to_return.at(n));
			ref_pair.push_back(make_pair(fstr, pointed_ref));
			add_header(to_return.at(n), ":DELETE");
		}
	}
	for (int n = 0; n < ref_pair.size(); ++n) {
		string ref = ref_pair.at(n).first;
		string name = ref_pair.at(n).second;
		if (debug) {
			cout << "WDT_LAZY::: " << ref << " " << name << endl;
		}
		to_return = substitute_ref(to_return, ref, name);
	}
	return to_return;
}

vector<DrtPred> clean_coord(const DrtVect &drtvect, vector<DrtPred> coord)
{
	// maps the elements to the header
	map<pair<string, string>, vector<string> > already_found;

	for (int n = 0; n < coord.size(); ++n) {
		string fref = extract_first_tag(coord.at(n));
		string sref = extract_second_tag(coord.at(n));
		string head = extract_header(coord.at(n));

		if (head == "@CONJUNCTION" || head == "@COORDINATION") {
			DrtPred tmp_pred = coord.at(n);
			implant_header(tmp_pred, "@SUBORD");
			switch_children(tmp_pred);
			if (shortfind(drtvect, tmp_pred)) {
				coord.erase(coord.begin() + n);
				--n;
			}
		}
	}
	return coord;
}

static vector<DrtPred> process_coord(const vector<DrtPred> &pre_drt, const vector<string> &tags, const vector<string> &names)
{
	vector<DrtPred> to_return(pre_drt);
	vector<DrtPred> coord;

	for (int n = 0; n < names.size(); ++n) {
		coord_type type = get_coord_type(to_return, tags, n);
		if (type.is_conjunction() || type.is_coordination() || type.is_disjunction() || type.is_conditional()) {
			string ref1 = extract_first_tag(to_return.at(n));
			string ref2 = extract_second_tag(to_return.at(n));
			if (ref1.find("verb") == string::npos) // if it is not associated to a verb try to find the verb
				to_return.at(n) = percolate_to_verb(to_return, tags, n); // If there is no verb nothing changes
			if (ref2.find("verb") == string::npos)
				to_return.at(n) = percolate_second_to_verb(to_return, tags, n); // If there is no verb nothing changes
			string first_verb_pos_str = extract_first_tag(to_return.at(n));
			string second_verb_pos_str = extract_second_tag(to_return.at(n));
			string type_str = type.get_type();
			DrtPred tmp_compl(string("@") + type_str + "(" + first_verb_pos_str + "," + second_verb_pos_str + ")");
			tmp_compl.setName(names.at(n));
			implant_header(to_return.at(n), type_str);
			coord.push_back(tmp_compl);
		}
	}

	coord = clean_coord(pre_drt, coord); // select competing coords

	// Modifies the subjects or the objects of the subordinates
	for (int n = 0; n < coord.size(); ++n) {
		if (extract_header(coord.at(n)).find("@CONDITION") != string::npos)
			// The subject or object of the conditional does not depend on the other phrase
			/// This is not true!! Correct this!!!
			continue;
		string first_verb_pos_str = extract_first_tag(coord.at(n));
		string second_verb_pos_str = extract_second_tag(coord.at(n));
		int m = find_verb_with_string(to_return, first_verb_pos_str);
		int m2 = find_verb_with_string(to_return, second_verb_pos_str);
		if (m == -1 || m2 == -1)
			continue;
		string head = extract_header(to_return.at(m));
		string head2 = extract_header(to_return.at(m2));
		if (head.find("PASSIVE") == string::npos && head2.find("PASSIVE") == string::npos) { // the verb is in the active form
			to_return = substitute_subj_with_name(first_verb_pos_str, second_verb_pos_str, to_return, tags);
		} else {
			to_return = substitute_passive_subj_with_name(first_verb_pos_str, second_verb_pos_str, to_return, tags);
		}
	}

	to_return.insert(to_return.end(), coord.begin(), coord.end());

	return to_return;
}

static vector<DrtPred> process_complements_from_names_and_adverbs(const vector<DrtPred> &pre_drt, const vector<string> &tags,
		const vector<string> &names)
{
	vector<DrtPred> to_return(pre_drt);

	vector<string> time_like;
	time_like.push_back("yesterday");
	time_like.push_back("tomorrow");
	time_like.push_back("today");
	time_like.push_back("year");
	time_like.push_back("month");

	for (int n = 0; n < to_return.size(); ++n) {
		string head = extract_header(to_return.at(n));
		if (head == "@CONJUNCTION") {
			string fref = extract_first_tag(to_return.at(n));
			string sref = extract_second_tag(to_return.at(n));
			if (points_to_name_or_ref(fref) && points_to_verb(sref)) {
				vector<int> candidates = find_all_element_with_string(pre_drt, fref);
				for (int m = 0; m < candidates.size(); ++m) {
					string candidate = extract_header(to_return.at(candidates.at(m)));
					if (find(time_like.begin(), time_like.end(), candidate) != time_like.end()) {
						// substitute the conjunction with a time complement
						add_header(to_return.at(n), ":DELETE");
						to_return.at(n).name() += ":DELETE";
						DrtPred tmp_pred(string("@TIME_AT(") + sref + "," + fref + ")");
						to_return.push_back(tmp_pred);
					}
				}
			} else if (points_to_name_or_ref(sref) && points_to_verb(fref)) {
				vector<int> candidates = find_all_element_with_string(pre_drt, fref);
				for (int m = 0; m < candidates.size(); ++m) {
					string candidate = extract_header(to_return.at(candidates.at(m)));
					if (find(time_like.begin(), time_like.end(), candidate) != time_like.end()) {
						// substitute the conjunction with a time complement
						add_header(to_return.at(n), ":DELETE");
						to_return.at(n).name() += ":DELETE";
						DrtPred tmp_pred(string("@TIME_AT(") + fref + "," + sref + ")");
						to_return.push_back(tmp_pred);
					}
				}
			}
		}
	}
	return to_return;
}

static vector<DrtPred> process_prep_WDT_from_verb(const vector<DrtPred> &pre_drt, const vector<string> &tags,
		const vector<string> &names, vector<pair<pair<int, int>, constituents> > &connections, bool is_question)
{
	vector<DrtPred> to_return(pre_drt);
	int pos1, pos2;
	string str1, str2, tag1, tag2;

// "the country that_2/WDT we know_1 about_(1,2)/IN"
	for (int n = 0; n < connections.size(); ++n) { // connects numerals and names with @QUANTITY(number,name)
		pos1 = connections.at(n).first.first;
		pos2 = connections.at(n).first.second;
		str1 = to_return.at(pos1).name();
		str2 = to_return.at(pos2).name();
		tag1 = to_return.at(pos1).tag();
		tag2 = to_return.at(pos2).tag();

		if (tag1 == "WDT" && is_preposition(tag2)) {
			if (has_second_tag(to_return.at(pos2)))
				continue; // the preposition is already assigned
			string wdt_ref = extract_first_tag(to_return.at(pos1));
			string verb_ref = extract_first_tag(to_return.at(pos2));
			implant_second(to_return.at(pos2), wdt_ref);
			int m = find_verb_with_subject(to_return, tags, wdt_ref);
			if (m != -1 && extract_first_tag(to_return.at(m)) == verb_ref) {
				implant_subject(to_return.at(m), "none");
				continue;
			}
			m = find_verb_with_object(to_return, tags, wdt_ref);
			if (debug) {
				cout << "FIND_OBJ::: " << m << " " << wdt_ref << endl;
			}
			if (m != -1 && extract_first_tag(to_return.at(m)) == verb_ref) {
				implant_object(to_return.at(m), "none");
				continue;
			}
		}
	}
	if (debug) {
		puts("FROM_VERB::");
		print_vector(to_return);
	}

	return to_return;
}

static vector<DrtPred> process_WP_from_AUX(const vector<DrtPred> &pre_drt, const vector<string> &tags,
		const vector<string> &names, vector<pair<pair<int, int>, constituents> > &connections, bool is_question)
{
	vector<DrtPred> to_return(pre_drt);
	int pos1, pos2;
	string str1, str2, tag1, tag2;

// If a WP is linked to an AUX, connect the WP to the POST_AUX
	for (int n = 0; n < connections.size(); ++n) { // connects numerals and names with @QUANTITY(number,name)
		pos1 = connections.at(n).first.first;
		pos2 = connections.at(n).first.second;
		str1 = to_return.at(pos1).name();
		str2 = to_return.at(pos2).name();
		tag1 = to_return.at(pos1).tag();
		tag2 = to_return.at(pos2).tag();

		if (tag1 == "WP" && is_question && is_verb(tag2) && is_AUX(to_return.at(pos2))
				&& !is_passive(to_return.at(pos2))
		) {
			string fref = extract_first_tag(to_return.at(pos1));
			if (!is_lonely_name(to_return, fref))
				continue;
			int cref = find_complement_with_target(to_return,fref);
			if (cref != -1)
				continue;
			if (debug) {
				cout << "WP_REF2::: " << fref << endl;
			}
			string vref = extract_object(to_return.at(pos2));
			int m = find_verb_with_string(to_return, vref);
			if (m == -1)
				continue;
			// find the first verb with either no subject or no object
			if (!has_subject(to_return.at(m)) && fref != extract_object(to_return.at(m)))
				implant_subject(to_return.at(m), fref);
			if (!has_object(to_return.at(m)) && fref != extract_subject(to_return.at(m)))
				implant_object(to_return.at(m), fref);
		}
	}
	return to_return;
}

static vector<DrtPred> process_WP(const vector<DrtPred> &pre_drt, const vector<string> &tags, const vector<string> &names)
{
	vector<DrtPred> to_return(pre_drt);

	vector<DrtPred> wps;

	for (int n = 0; n < tags.size(); ++n) {
		if (pre_drt.at(n).is_WP() && !pre_drt.at(n).is_delete()
				) {
			string wp_str = string("name") + boost::lexical_cast<string>(n);
			/// BAD SOLUTION. You should check if the wp_str is used elewhere.

			string wp_class_str;
			string name_str = names.at(n);
			name_str = name_str.substr(0, name_str.find(':'));
			if (name_str == "what")
				wp_class_str = get_local_what();
			else if (name_str == "who" || name_str == "whom")
				wp_class_str = "person";
			DrtPred tmp_wp(wp_class_str + "(" + wp_str + ")");
			tmp_wp.setTag("NN");
			tmp_wp.setName(wp_class_str);
			tmp_wp.set_pivot(true);

			// Set the new word as a question if the WP is a question
			if (pre_drt.at(n).is_question()) {
				tmp_wp.set_question();
				tmp_wp.set_question_word(name_str);
				if (debug) {
					cout << names.at(n) << endl;
				}
			}

			add_header(to_return.at(n), ":DELETE");
			wps.push_back(tmp_wp);
		}
	}

	to_return.insert(to_return.end(), wps.begin(), wps.end());

	return to_return;
}

static vector<DrtPred> process_WP_pos(vector<DrtPred> &pre_drt, const vector<string> &tags, const vector<string> &names,
		bool is_question)
{
	vector<DrtPred> to_return(pre_drt);
	vector<DrtPred> wps;
	if (is_question) {
		for (int n = 0; n < tags.size(); ++n) {
			if (pre_drt.at(n).is_WP_pos()) {
				string wp_str = string("name[pos]") + boost::lexical_cast<string>(n);
				/// BAD SOLUTION. You should check if the wp_str is used elsewhere.
				string fref = extract_first_tag(pre_drt.at(n));
				DrtPred tmp_possessive(string("@GENITIVE|@OWNED_BY(") + fref + "," + wp_str + ")");
				tmp_possessive.set_question();
				tmp_possessive.set_question_word(names.at(n));
				tmp_possessive.setTag("WP$");
				to_return.at(n) = tmp_possessive;
			}
		}
	} else {
		for (int n = 0; n < tags.size(); ++n) {
			if (pre_drt.at(n).is_WP_pos()) {
				string fref = extract_first_tag(pre_drt.at(n));
				bool need_plural= false;
				string wp_str = find_matching_previous_name(pre_drt, tags, n, "he", need_plural);
				if (wp_str == "") {
					wp_str = find_matching_previous_name(pre_drt, tags, n, "she", need_plural);
				}
				if (wp_str == "") {
					wp_str = find_matching_previous_name(pre_drt, tags, n, "it", need_plural);
				}
				if (wp_str == "") {
					wp_str = find_matching_previous_name(pre_drt, tags, n, "they", need_plural);
				}
				if (wp_str == "")
					continue;
				DrtPred tmp_possessive(string("@GENITIVE(") + fref + "," + wp_str + ")");
				to_return.at(n) = tmp_possessive;
			}
		}
	}

	to_return.insert(to_return.end(), wps.begin(), wps.end());

	return to_return;
}

static vector<DrtPred> loose_complements(const vector<DrtPred> &pre_drt, const vector<string> &tags, const vector<string> &names)
{
	vector<DrtPred> to_return(pre_drt);
	int size = to_return.size();

	map<string, string> map_compl;
	map_compl["in"] = "@PLACE_AT";
	map_compl["to"] = "@MOTION_TO|@TIME_TO|@DATIVE";
	map_compl["for"] = "@FOR";
	map_compl["with"] = "@WITH";
	map_compl["as"] = "@TOPIC|@TIME_AT";
	map_compl["at"] = "@TIME_AT|@PLACE_AT";
	map_compl["of"] = "@GENITIVE";
	map_compl["like"] = "@TOPIC|@COMPARED_TO";
	map_compl["about"] = "@TOPIC";
	map_compl["after"] = "@AFTER";
	map_compl["before"] = "@BEFORE";
	map_compl["at-what-time"] = "@CLOCK_AT";

	for (int n = 0; n < size; ++n) {
		string head_str = extract_header(to_return.at(n));
		if (to_return.at(n).is_name()) {
			string ref = extract_first_tag(to_return.at(n));
			int m = find_verb_with_object(to_return, tags, ref);
			if (m == -1)
				continue;
			int prep_pos = find_prep_with_first_tag(to_return, tags, ref);
			if (prep_pos == -1)
				continue;
			string prep_to_ref = extract_second_tag(to_return.at(prep_pos));
			if (!ref_is_name(prep_to_ref) && !ref_is_verb(prep_to_ref)) { // person VERB for? -> VERB(A,B,C), person(C), for(C,D) -> VERB(A,B,C), for(A,D)
				string verb_ref = extract_first_tag(to_return.at(m));
				string prep_ref = extract_first_tag(to_return.at(prep_pos));
				implant_first(to_return.at(prep_pos), verb_ref);
				implant_second(to_return.at(prep_pos), prep_ref);
				implant_object(to_return.at(m), "none");
			}
		}
	}

	for (int n = 0; n < size; ++n) {
		string head_str = extract_header(to_return.at(n));
		if (to_return.at(n).is_question() && !to_return.at(n).is_WP() && !to_return.at(n).is_WDT()) {
			string quest_ref = extract_first_tag(to_return.at(n));
			int m = find_verb_with_subject(to_return, tags, quest_ref);
			int m2 = find_verb_with_object(to_return, tags, quest_ref);
			if (m != -1) {
				string verb_ref = extract_first_tag(to_return.at(m));
				int prep_pos = find_prep_with_first_tag(to_return, tags, verb_ref);
				if (prep_pos == -1)
					continue;
				string prep_to_ref = extract_second_tag(to_return.at(prep_pos));
				if (!ref_is_name(prep_to_ref) && !ref_is_verb(prep_to_ref)) {
					// person VERB for? -> VERB(A,B,C), person(B), for(A,D) -> VERB(A,none,C), for(A,B) person(B)
					implant_second(to_return.at(prep_pos), quest_ref);
					implant_subject(to_return.at(m), "none");
					// un:DELETE question predicate
					string head = extract_header(to_return.at(n));
					head = head.substr(0, head.find(':'));
					implant_header(to_return.at(n), head);
					to_return.at(n).name() = head;
				}
			} else if (m2 != -1) {
				string verb_ref = extract_first_tag(to_return.at(m2));
				int prep_pos = find_prep_with_first_tag(to_return, tags, verb_ref);
				if (prep_pos == -1)
					continue;
				string prep_to_ref = extract_second_tag(to_return.at(prep_pos));
				if (!ref_is_name(prep_to_ref) && !ref_is_verb(prep_to_ref)) {
					// person VERB for? -> VERB(A,B,C), person(B), for(A,D) -> VERB(A,none,C), for(A,B) person(B)
					implant_second(to_return.at(prep_pos), quest_ref);
					implant_object(to_return.at(m2), "none");
					// un:DELETE question predicate
					string head = extract_header(to_return.at(n));
					head = head.substr(0, head.find(':'));
					implant_header(to_return.at(n), head);
					to_return.at(n).name() = head;
				}
			}
		}
	}

	for (int n = 0; n < size; ++n) {
		string head_str = extract_header(to_return.at(n));
		if (to_return.at(n).is_preposition() && head_str.find("PREP") == string::npos
				&& head_str.find(":DELETE") == string::npos && head_str.find("@") == string::npos) {
			string sref = extract_second_tag(to_return.at(n));

			string new_str = "@PLACE_AT";
			map<string, string>::iterator miter = map_compl.find(head_str);
			if (miter != map_compl.end())
				new_str = miter->second;
			implant_header(to_return.at(n), new_str);
			to_return.at(n).setTag("");
		}
	}
	return to_return;
}

static vector<DrtPred> percolate_WP_object_to_subordinates(const vector<DrtPred> &pre_drt, const vector<string> &tags,
		const vector<string> &names)
{
	vector<DrtPred> to_return(pre_drt);
	int size = to_return.size();
	for (int n = 0; n < size; ++n) {
		if (to_return.at(n).is_WP() ) {
			string fref= extract_first_tag(to_return.at(n));
			int m = find_verb_with_object(to_return,fref);
			if (m == -1)
				continue;
			string vref = extract_first_tag(to_return.at(m));
			int m2= find_complement_with_first_tag(to_return,vref,"@SUBORD");
			if (m2 == -1)
				continue;
			string sref2 = extract_second_tag(to_return.at(m2));
			int m3 = find_verb_with_string(to_return,sref2);
			if (m3 == -1)
				continue;
			implant_object(to_return.at(m),"none");
			implant_object(to_return.at(m3),fref);
		}
	}
	return to_return;
}

static vector<DrtPred> last_touch_complements_in_questions(const vector<DrtPred> &pre_drt, const vector<string> &tags,
		const vector<string> &names)
// complements in questions can be ambiguous: "to" can be @MOTION_TO or @DATIVE
{
	vector<DrtPred> to_return(pre_drt);
	int size = to_return.size();

	map<string, string> map_compl;
	map_compl["@MOTION_TO"] = "@MOTION_TO|@TIME_TO|@DATIVE";
	map_compl["@DATIVE"] = "@MOTION_TO|@TIME_TO|@DATIVE";
	map_compl["@PLACE_AT"] = "@PLACE_AT|@MOTION_FROM|@MOTION_TO|@MOTION_THROUGH";
	map_compl["@TIME_AT"] = "@TIME_AT|@CLOCK_AT|@TIME_FROM|@TIME_TO|@TOPIC";
	map_compl["@CAUSED_BY"] = "@CAUSED_BY|@WITH|@DATIVE";
	map_compl["@QUANTITY"] = "@QUANTITY|@MORE_THAN|@LESS_THAN";
	map_compl["@SIZE"] = "@SIZE|@QUANTITY|@TIME_DURATION";
	map_compl["@ALLOCUTION"] = "@ALLOCUTION|@TOPIC";

	map<pair<string, string>, string> map_compl_with_prep;
	map_compl_with_prep[make_pair("@MOTION_TO", "to")] = "@MOTION_TO|@TIME_TO|@DATIVE";
	map_compl_with_prep[make_pair("@DATIVE", "as")] = "@DATIVE";
	map_compl_with_prep[make_pair("@DATIVE", "at")] = "@DATIVE|@TIME_AT";
	map_compl_with_prep[make_pair("@PLACE_AT", "at")] = "@PLACE_AT";
	map_compl_with_prep[make_pair("@PLACE_AT", "from")] = "@MOTION_FROM|@TIME_FROM";
	map_compl_with_prep[make_pair("@PLACE_AT", "on")] = "@PLACE_AT|@TIME_AT|@MOTION_FROM|@MOTION_TO|@MOTION_THROUGH";
	map_compl_with_prep[make_pair("@TIME_AT", "on")] = "@PLACE_AT|@TIME_AT";
	map_compl_with_prep[make_pair("@PLACE_AT", "where")] = "@PLACE_AT|@MOTION_FROM|@MOTION_TO|@MOTION_THROUGH";
	map_compl_with_prep[make_pair("@TIME_AT", "when")] = "@TIME_AT|@CLOCK_AT|@TIME_FROM|@TIME_TO|@TOPIC|@BEFORE|@AFTER";
	map_compl_with_prep[make_pair("@CAUSED_BY", "because")] = "@CAUSED_BY|@WITH|@DATIVE";
	map_compl_with_prep[make_pair("@CAUSED_BY", "why")] = "@CAUSED_BY";
	map_compl_with_prep[make_pair("@WITH", "with")] = "@CAUSED_BY|@WITH";
	map_compl_with_prep[make_pair("@FOR", "for")] = "@CAUSED_BY|@FOR";
	map_compl_with_prep[make_pair("@TOPIC", "on")] = "@TOPIC|@TIME_AT";
	map_compl_with_prep[make_pair("@TOPIC", "under")] = "@TOPIC|@PLACE_AT";
	map_compl_with_prep[make_pair("@TIME_THROUGH", "through")] = "@TIME_THROUGH|@MOTION_THROUGH";
	map_compl_with_prep[make_pair("@MOTION_THROUGH", "through")] = "@TIME_THROUGH|@MOTION_THROUGH";
	map_compl_with_prep[make_pair("@CAUSED_BY", "how")] = "@CAUSED_BY|@WITH|@DATIVE|@TOPIC|@COMPARED_TO";

	for (int n = 0; n < size; ++n) {
		string head_str = extract_header(to_return.at(n));
		if (to_return.at(n).is_complement()
		//&& to_return.at(n).is_question() /// Do not use! questions must be as ambiguous as possible
		) {
			string prep_str = to_return.at(n).name();
			map<pair<string, string>, string>::iterator miter = map_compl_with_prep.find(make_pair(head_str, prep_str));
			if (miter != map_compl_with_prep.end()) {
				implant_header(to_return.at(n), miter->second);
			} else {
				map<string, string>::iterator miter2 = map_compl.find(head_str);
				if (miter2 != map_compl.end()) {
					implant_header(to_return.at(n), miter2->second);
				}
			}
		}
	}
	return to_return;
}

static vector<DrtPred> last_touch_time(const vector<DrtPred> &pre_drt, const vector<string> &tags, const vector<string> &names)
// @TIME_AT that refers to a CD makes the CD become [date]_CD
{
	vector<DrtPred> to_return(pre_drt);
	int size = to_return.size();

	for (int n = 0; n < size; ++n) {
		string head_str = extract_header(to_return.at(n));
		if (head_str == "@TIME_AT" || head_str == "@AFTER" || head_str == "@BEFORE") {
			string sref = extract_second_tag(to_return.at(n));
			int m = find_element_with_string(to_return, sref);
			if (m != -1 && to_return.at(m).is_number() && !to_return.at(m).is_date() // a date does not need to become [date]_[date]_
					) {
				head_str = extract_header(to_return.at(m));
				head_str = string("[date]_") + head_str;
				implant_header(to_return.at(m), head_str);
				to_return.at(m).name() = head_str;
			}
		}
	}
	return to_return;
}

static vector<DrtPred> collapse_OR(const vector<DrtPred> &pre_drt, const vector<string> &tags, const vector<string> &names)
// for questions: color or bird -> color|bird + set_question
{
	vector<DrtPred> to_return(pre_drt);
	int size = to_return.size();

	for (int n = 0; n < size; ++n) {
		string head_str = extract_header(to_return.at(n));
		if(debug) {
			cout << "CO0R::: " << head_str << endl;
		}
		if (pre_drt.at(n).is_conj() && head_str == "or") {
			string fref = extract_first_tag (to_return.at(n));
			string sref = extract_second_tag(to_return.at(n));

			int m1 = find_name_with_string(to_return, fref);
			int m2 = find_name_with_string(to_return, sref);
			if(debug) {
				cout << "COR::: " << m1 << endl;
				cout << "COR::: " << m2 << endl;
			}

			if (m1 == -1 || m2 == -1)
				continue;

			string head_str1 = extract_header(to_return.at(m1));
			string head_str2 = extract_header(to_return.at(m2));

			add_header(to_return.at(n),":DELETE");
			add_header(to_return.at(m2),":DELETE");

			implant_header(to_return.at(m1), head_str1+"|"+head_str2);

		}
	}
	return to_return;
}


static vector<DrtPred> eliminate_redundant_objects(const vector<DrtPred> &pre_drt, const vector<string> &tags,
		const vector<string> &names)
// in questions generic objects are useless
{
	vector<DrtPred> to_return(pre_drt);
	int size = to_return.size();

	vector<string> communication_verbs = get_communication_verbs();

	// What does David say? -> david() say() what() -> david() say() @ALLOCUTION() what()
	for (int n = 0; n < size; ++n) {
		string head_str = extract_header(to_return.at(n));
		if (head_str.find(get_local_what()) != string::npos) {
			string ref = extract_first_tag(to_return.at(n));
			int m = find_verb_with_object(to_return, tags, ref);
			if (m == -1)
				continue;
			string head = extract_header(to_return.at(m));
			head = head.substr(0, head.find(':'));

			if (shortfind(communication_verbs, head)) {
				// Eliminates "say what?"
				// and substitutes it with say() @ALLOCUTION() what() ?
				implant_object(to_return.at(m), "none");
				add_header(to_return.at(n), "|[*]|[S]");
				string vref = extract_first_tag(to_return.at(m));
				string oref = extract_first_tag(to_return.at(n));
				DrtPred tmp_pred(string("@ALLOCUTION(") + vref + "," + oref + ")");
				to_return.push_back(tmp_pred);
			}
		}
	}

	for (int n = 0; n < size; ++n) {
		string head_str = extract_header(to_return.at(n));
		if (head_str.find(get_local_what()) != string::npos) {
			string ref = extract_first_tag(to_return.at(n));
			int m = find_verb_with_object(to_return, tags, ref);
			if (m == -1)
				continue;
			string head = extract_header(to_return.at(m));
			head = head.substr(0, head.find(":"));
			int prep_pos = find_prep_with_first_tag(to_return, tags, ref);
			string sref = extract_second_tag(to_return.at(m));
			vector<int> poz = find_all_names_with_string_no_delete(to_return, sref);
			if (prep_pos != -1 && poz.size() == 0) { // what VERB for? : VERB(A,B,C), what(C), for(C,D) -> VERB(A,B,C), for(A,D)
				string verb_ref = extract_first_tag(to_return.at(m));
				string prep_ref = extract_first_tag(to_return.at(prep_pos));
				implant_first(to_return.at(prep_pos), verb_ref);
				implant_second(to_return.at(prep_pos), prep_ref);
				implant_object(to_return.at(m), "none");
				to_return.at(n).name() = get_local_what(); /// undelete!
				implant_header(to_return.at(n), get_local_what());
			}
			/// the following is unnecessary in general because of the "[S]" in "what".
			/// It is only used for "do what?"
			else if (head == "do") {
				string head = extract_header(to_return.at(m));
				if (head.find("PASSIVE") == string::npos) {
					// Eliminates "does what?"
					/// This should be changed, by leaving "what" and adding a category as verbs
					to_return.at(n).name() += ":DELETE";
					add_header(to_return.at(n), ":DELETE");
					to_return.at(m).set_question();
					to_return.at(m).set_question_word("do-something");
					implant_object(to_return.at(m), "none");
				}
			}
		}
	}

	return to_return;
}

static vector<DrtPred> eliminate_redundant_subjects(const vector<DrtPred> &pre_drt, const vector<string> &tags,
		const vector<string> &names)
// in questions generic subject cannot be associated to the verb "happen"
{
	vector<DrtPred> to_return(pre_drt);
	int size = to_return.size();

	for (int n = 0; n < size; ++n) {
		string head_str = extract_header(to_return.at(n));
		if (head_str.find(get_local_what()) != string::npos || to_return.at(n).is_WP()) {
			string ref = extract_first_tag(to_return.at(n));
			int m = find_verb_with_subject(to_return, tags, ref);
			if (m != -1) {
				if (extract_header(to_return.at(m)) == "happen") {
					to_return.at(n).name() += ":DELETE";
					add_header(to_return.at(n), ":DELETE");

					// sign the verb as a predicate to answer
					to_return.at(m).set_question();
					to_return.at(m).set_question_word("what-happens");
					continue;
				}
				// If there is a name associated to the same reference
				vector<int> mm = find_all_element_with_string(to_return, ref);
				if (mm.size() > 1) {
					int nnames = 0;
					string head_str2;
					for (int pos = 0; pos < mm.size(); ++pos) {
						head_str2 = extract_header(to_return.at(mm.at(pos)));
						if (to_return.at(mm.at(pos)).is_name() && !to_return.at(mm.at(pos)).is_delete()
								&& head_str2.find(get_local_what()) == string::npos) {
							if (nnames == 0) {
								// sign the first name as a predicate to answer
								string head = extract_header(to_return.at(mm.at(pos)));
								to_return.at(mm.at(pos)).set_question();
								to_return.at(mm.at(pos)).set_question_word(head);
							}
							++nnames;
						}
					}
					if (nnames > 0) {
						to_return.at(n).name() += ":DELETE";
						add_header(to_return.at(n), ":DELETE");
					}
				}
			}
		}
	}

	return to_return;
}

static vector<DrtPred> sign_some_predicates_as_questions(vector<DrtPred> pre_drt, const vector<string> &tags,
		const vector<string> &names)
//
{
	vector<DrtPred>::iterator diter = pre_drt.begin();
	vector<DrtPred>::iterator dend = pre_drt.end();

	bool WP_trigger = false;

	for (; diter != dend; ++diter) {
		if (diter->is_WP() || diter->is_WRB()) {
			WP_trigger = true;
			string ref_str = extract_first_tag(*diter);
			vector<int> preds_int = find_all_element_with_string(pre_drt, ref_str);
			for (int m = 0; m < preds_int.size(); ++m) {
				if (debug) {
					cout << "WRB::: " << m << " " << pre_drt.at(preds_int.at(m)) << endl;
				}
				if (preds_int.at(m) < tags.size() && (is_WP(tags.at(preds_int.at(m))) || is_WRB(tags.at(preds_int.at(m))))) {
					string head = extract_header(pre_drt.at(preds_int.at(m)));
					pre_drt.at(preds_int.at(m)).set_question();
					pre_drt.at(preds_int.at(m)).set_question_word(head);
				}
			}
		}
	}
	if (!WP_trigger) {
		diter = pre_drt.begin();
		// If there were no WP take the first |
		for (; diter != dend; ++diter) {
			if(diter->is_name() && extract_header(*diter).find('|') != string::npos) {
				string head = extract_header(*diter);
				diter->set_question();
				diter->set_question_word(head);
				WP_trigger = true;
				break;
			}
		}
	}
	if (!WP_trigger) {
		diter = pre_drt.begin();
		// If there were no WP take the first name
		for (; diter != dend; ++diter) {
			if (diter->is_verb()) {
				// if a verb is before the name then it is a y/n question
				break;
			}


			if (diter->is_name() && !diter->is_number() && !diter->is_article() && !diter->is_question()) {
				if(!diter->is_pivot())
					continue; // eye color:[pivot]; [pivot] must be the question word
				string head = extract_header(*diter);
				diter->set_question();
				diter->set_question_word(head);
				break;
			}
		}
	}
	return pre_drt;
}

static vector<DrtPred> save_name_conjunctions(const vector<DrtPred> &pre_drt, const vector<string> &tags,
		const vector<string> &names)
{
	vector<DrtPred> to_return(pre_drt);
	int size = to_return.size();

	for (int n = 0; n < size; ++n) {
		string head_str = extract_header(to_return.at(n));
		if (head_str == "and") {
			string fref = extract_first_tag(to_return.at(n));
			string sref = extract_second_tag(to_return.at(n));
			if (points_to_name_or_ref(fref) && points_to_name_or_ref(sref)) {
				implant_header(to_return.at(n), "@AND");
				to_return.at(n).name() = "@AND";
			}
		} else if (head_str == "or") {
			string fref = extract_first_tag(to_return.at(n));
			string sref = extract_second_tag(to_return.at(n));
			if (points_to_name_or_ref(fref) && points_to_name_or_ref(sref)) {
				implant_header(to_return.at(n), "@OR");
				to_return.at(n).name() = "@OR";
			}
		}
	}

	return to_return;
}

static bool has_only_adjectives(const DrtVect &drtvect, const string &ref)
{
	for(int n = 0; n < drtvect.size(); ++n) {
		string fref = extract_first_tag(drtvect.at(n));
		if( fref == ref
				&& !drtvect.at(n).is_adjective()
				&& !drtvect.at(n).is_article()
				&& !drtvect.at(n).is_preposition()
				&& !drtvect.at(n).is_conj()
				&& !drtvect.at(n).is_complement()) {
			return false;
		}
	}
	return true;
}


static vector<DrtPred> process_declaratives(const vector<DrtPred> &pre_drt, const vector<string> &tags,
		const vector<string> &names)
// process names connected to other names by a comma, like in "Alan, the prime minister of Britain, ..."; "Alan" and "minister" should share the same reference tag.
{
	vector<DrtPred> to_return(pre_drt);
	int size = to_return.size();

	vector<pair<string, string> > ref_pair;

	for (int n = 0; n < size; ++n) {
		string head_str = extract_header(to_return.at(n));
		string tag = to_return.at(n).tag();
		if (head_str.find("-comma-") != string::npos || (tag == "CC" && head_str == "and")
				|| (tag == "CC" && head_str == "or")) {
			string fref = extract_first_tag(to_return.at(n));
			string sref = extract_second_tag(to_return.at(n));

			if ((points_to_name(sref) || points_to_ref(sref)) && points_to_verb(fref)) {
				// If the conjunction points to a name and a verb,
				// take the subject of the verb instead of the
				// verb. Only for verbs that are on the right of the name.

				int m = find_verb_with_string(to_return, tags, fref);

				if (debug) {
					cout << "PROCESS_DECL:::" << m << ", " << n << endl;
				}

				if (m != -1) {
					if (has_subject(to_return.at(m)) && m > n) {
						string subj_ref = extract_subject(pre_drt.at(m));
						implant_first(to_return.at(n), subj_ref);
					} else {
						add_header(to_return.at(n), ":DELETE");
					}
				}
			}
		}
	}

	// process the declarative Adam, the president, decided ...

	/// This system is not ready yet

//	for (int n = 0; n < size; ++n) {
//		string head_str = extract_header(to_return.at(n));
//		string fref = extract_first_tag(to_return.at(n));
//		string sref = extract_second_tag(to_return.at(n));
//		if (head_str.find("-comma-") != string::npos
//				&& !has_only_adjectives(to_return,fref)
//				&& (points_to_name(fref) || points_to_ref(fref))
//				&& (points_to_name(sref) || points_to_ref(sref))
//		) {
//			ref_pair.push_back(make_pair(sref, fref));
//		}
//	}

	for (int n = 0; n < ref_pair.size(); ++n) {
		string ref = ref_pair.at(n).first;
		string name = ref_pair.at(n).second;
		to_return = substitute_ref_safe(to_return, ref, name);
	}

	return to_return;
}

static vector<DrtPred> process_specification(const vector<DrtPred> &pre_drt, const vector<string> &tags,
		const vector<string> &names)
{
	vector<DrtPred> to_return(pre_drt);
	int size = pre_drt.size();

	vector<pair<string, string> > ref_pair;

	for (int n = 0; n < size; ++n) {
		string head_str = extract_header(to_return.at(n));
		if (head_str.size() && head_str.at(0) == '@') {
			if (points_to_ref(to_return.at(n))) {
				string ref = extract_first_tag(to_return.at(n));
				int int_ref = get_single_distance(ref);
				string name = string("name") + boost::lexical_cast<string>(int_ref);
				ref_pair.push_back(make_pair(ref, name));
			}
		}
	}

	for (int n = 0; n < ref_pair.size(); ++n) {
		string ref = ref_pair.at(n).first;
		string name = ref_pair.at(n).second;
		to_return = substitute_ref(to_return, ref, name);
	}

	return to_return;
}

string set_ref_number(string ref, int new_num)
{
	int size = ref.size();
	int end;

	for (int n = 0; n < size; ++n) {
		if (isdigit(ref.at(n))) {
			end = n;
			break;
		}
	}

	ref = ref.substr(0, end);
	ref += boost::lexical_cast<string>(new_num);

	return ref;

}

static vector<DrtPred> process_LBR(vector<DrtPred> to_return, const vector<string> &tags, const vector<string> &names,
		vector<pair<pair<int, int>, constituents> > &connections)
{
	int size = to_return.size();

	for (int n = 0; n < size; ++n) {
		string head = extract_header(to_return.at(n));
		if (head == "-LBR-") {
			implant_header(to_return.at(n), "@PRN");
			to_return.at(n).name() = "@PRN";
		}
	}

	return to_return;
}

static bool has_than(const DrtVect &pre_drt, int pos)
{
	string ref1 = extract_first_tag(pre_drt.at(pos));
	for (int n = pos; n < pre_drt.size(); ++n) {
		string ref2 = extract_first_tag(pre_drt.at(n));
		string head = extract_header(pre_drt.at(n));
		if (ref1 == ref2 && head.find("@COMPARED_") != string::npos) {
			return true;
		}
	}
	return false;
}

static DrtVect attach_comparatives(vector<DrtPred> to_return, const vector<string> &tags, const vector<string> &names,
		vector<pair<pair<int, int>, constituents> > &connections)
// I am taller than my brother
{
	int pos1, pos2;
	string str1, str2, tag1, tag2;

	for (int n = 0; n < connections.size(); ++n) {
		pos1 = connections.at(n).first.first;
		pos2 = connections.at(n).first.second;
		str1 = to_return.at(pos1).name();
		str2 = to_return.at(pos2).name();
		tag1 = to_return.at(pos1).tag();
		tag2 = to_return.at(pos2).tag();

		if ((tag1 == "JJR" || tag1 == "RBR") && is_verb(tag2)) {
			if (!has_than(to_return, pos1))
				continue;
			string name = "@COMPARED";
			string head = str1;
			bool JJR_trigger = false;
			if (head == "more")
				name = "@MORE";
			else if (head == "less")
				name = "@LESS";
			else if (tag1 == "JJR") {
				name = "@MORE";
				JJR_trigger = true;
			}
			DrtPred tmp_comp(name + "(dummy1,dummy2)");
			string JJR_ref = extract_first_tag(to_return.at(pos1));
			string verb_ref = extract_first_tag(to_return.at(pos2));
			implant_first(tmp_comp, verb_ref);
			implant_second(tmp_comp, JJR_ref);
			// if JJR_ref is object of the verb, implant "none" as object
			string obj_str = extract_object(to_return.at(pos2));
			if (obj_str == JJR_ref)
				implant_object(to_return.at(pos2), "none");
			to_return.push_back(tmp_comp);
			if (!JJR_trigger) {
				implant_header(to_return.at(pos1), extract_header(to_return.at(pos1)) + ":DELETE");
			} else {
				tagger_info *t = parser_singleton::get_tagger_info_instance();
				string header = extract_header(to_return.at(pos1));
				header = t->get_base_superlative(header);
				implant_header(to_return.at(pos1), header);
				to_return.at(pos2).setTag("JJ");
			}
		}
		if ((tag2 == "JJR" || tag2 == "RBR") && is_verb(tag1)) {
			if (!has_than(to_return, pos2))
				continue;
			string name = "@COMPARED";
			string head = str2;
			bool JJR_trigger = false;
			if (head == "more")
				name = "@MORE";
			else if (head == "less")
				name = "@LESS";
			else if (tag2 == "JJR") {
				name = "@MORE";
				JJR_trigger = true;
			}
			DrtPred tmp_comp(name + "(dummy1,dummy2)");
			string JJR_ref = extract_first_tag(to_return.at(pos2));
			string verb_ref = extract_first_tag(to_return.at(pos1));
			implant_first(tmp_comp, verb_ref);
			implant_second(tmp_comp, JJR_ref);
			// if JJR_ref is object of the verb, implant "none" as object
			string obj_str = extract_object(to_return.at(pos1));
			if (obj_str == JJR_ref)
				implant_object(to_return.at(pos1), "none");
			to_return.push_back(tmp_comp);
			if (!JJR_trigger) {
				implant_header(to_return.at(pos2), extract_header(to_return.at(pos2)) + ":DELETE");
			} else {
				tagger_info *t = parser_singleton::get_tagger_info_instance();
				string header = extract_header(to_return.at(pos2));
				header = t->get_base_superlative(header);
				implant_header(to_return.at(pos2), header);
				to_return.at(pos2).setTag("JJ");
			}
		}
	}

	return to_return;
}
static vector<DrtPred> flatten_LBR(vector<DrtPred> to_return, const vector<string> &tags, const vector<string> &names,
		vector<pair<pair<int, int>, constituents> > &connections)
// process the names that has not been tagged as adverbs. These names have been left alone in the drt_builder
{
	int pos1, pos2;
	string str1, str2, tag1, tag2;

	tagger *tagg = parser_singleton::get_tagger_instance();

	vector<DrtPred> quants;

	vector<pair<string, string> > ref_pair;

	for (int n = 0; n < connections.size(); ++n) {
		pos1 = connections.at(n).first.first;
		pos2 = connections.at(n).first.second;
		str1 = to_return.at(pos1).name();
		str2 = to_return.at(pos2).name();
		tag1 = to_return.at(pos1).tag();
		tag2 = to_return.at(pos2).tag();

		if (str1 == "-LBR-" && is_name(tag2) && !is_numeral(tag2) && there_are_only_names_between(tags, pos1, pos2)) {
			string fref = extract_first_tag(to_return.at(pos1));
			string sref = extract_first_tag(to_return.at(pos2));
			ref_pair.push_back(make_pair(sref, fref));
		}
		if (str2 == "-LBR-" && is_name(tag1) && !is_numeral(tag1) && there_are_only_names_between(tags, pos1, pos2)) {
			string fref = extract_first_tag(to_return.at(pos2));
			string sref = extract_first_tag(to_return.at(pos1));
			ref_pair.push_back(make_pair(sref, fref));
		}
		if (str1 == "-LBR-" && is_verb(tag2)) {
			string fref = extract_first_tag(to_return.at(pos1));
			string sref = extract_object(to_return.at(pos2));
			if (ref_is_verb(fref))
				ref_pair.push_back(make_pair(sref, fref));
		}
		if (str2 == "-LBR-" && is_verb(tag1)) {
			string fref = extract_first_tag(to_return.at(pos2));
			string sref = extract_object(to_return.at(pos1));
			if (ref_is_verb(fref))
				ref_pair.push_back(make_pair(sref, fref));
		}
	}

	for (int n = 0; n < ref_pair.size(); ++n) {
		string ref = ref_pair.at(n).first;
		string name = ref_pair.at(n).second;
		to_return = substitute_ref(to_return, ref, name);
	}
	return to_return;
}

static vector<DrtPred> process_lonely_names_and_adverbs(vector<DrtPred> to_return,
		vector<pair<pair<int, int>, constituents> > &connections)
// process the names that has not been tagged as adverbs. These names have been left alone in the drt_builder
{
	int pos1, pos2;
	string str1, str2, tag1, tag2;

	tagger *tagg = parser_singleton::get_tagger_instance();

	vector<DrtPred> quants;

	vector<pair<string, string> > ref_pair;

	for (int n = 0; n < connections.size(); ++n) {
		pos1 = connections.at(n).first.first;
		pos2 = connections.at(n).first.second;
		str1 = to_return.at(pos1).name();
		str2 = to_return.at(pos2).name();
		tag1 = to_return.at(pos1).tag();
		tag2 = to_return.at(pos2).tag();

		if (is_name(tag1) && is_verb(tag2) && tagg->get_info()->get_conj(str1, "RB") != "") {
			if (is_lonely_name(to_return, pos1)) {
				string name_ref = extract_first_tag(to_return.at(pos1));
				string verb_ref = extract_first_tag(to_return.at(pos2));
				ref_pair.push_back(make_pair(name_ref, verb_ref));
				to_return.at(pos1).setTag("RB");
			}
		}
		if (is_name(tag2) && is_verb(tag1) && tagg->get_info()->get_conj(str2, "RB") != "") {
			if (is_lonely_name(to_return, pos2)) {
				string name_ref = extract_first_tag(to_return.at(pos2));
				string verb_ref = extract_first_tag(to_return.at(pos1));
				ref_pair.push_back(make_pair(name_ref, verb_ref));
				to_return.at(pos2).setTag("RB");
			}
		}

	}

	for (int n = 0; n < ref_pair.size(); ++n) {
		string ref = ref_pair.at(n).first;
		string name = ref_pair.at(n).second;
		to_return = substitute_ref(to_return, ref, name);
	}

	return to_return;
}

static vector<DrtPred> post_process_passive(vector<DrtPred> to_return, vector<pair<pair<int, int>, constituents> > &connections)
{
	int pos1, pos2;
	string str1, str2, tag1, tag2;

	for (int n = 0; n < connections.size(); ++n) {
		pos1 = connections.at(n).first.first;
		pos2 = connections.at(n).first.second;
		str1 = extract_header(to_return.at(pos1));
		str2 = extract_header(to_return.at(pos2));
		if (str1.find("PASSIVE_AUX") != string::npos && str2.find("PASSIVE_POST_AUX") != string::npos) {
			if (!has_object(to_return.at(pos2)) && has_subject(to_return.at(pos1))) {
				string ref = extract_subject(to_return.at(pos1));
				implant_object(to_return.at(pos2), ref);
			}
		}
	}

	return to_return;
}

static vector<DrtPred> process_HOWTO(vector<DrtPred> to_return, vector<string> tags,
		vector<pair<pair<int, int>, constituents> > &connections, bool is_question)
{
	int pos1, pos2;
	string str1, str2, tag1, tag2;

	vector<int> already_done;
	for (int n = 0; n < connections.size(); ++n) {
		pos1 = connections.at(n).first.first;
		pos2 = connections.at(n).first.second;
		str1 = to_return.at(pos1).name();
		str2 = to_return.at(pos2).name();
		tag1 = to_return.at(pos1).tag();
		tag2 = to_return.at(pos2).tag();

		if (str1 == "how" && tag1 == "WRB" && tag2 == "TO"
				&& fabs(pos1 - pos2) == 1
		) {
			string sref = extract_second_tag(to_return.at(pos2));
			string fref = extract_first_tag(to_return.at(pos1));
			if (sref != fref)
				implant_second(to_return.at(pos1), sref);
			if(is_question) {
				implant_header(to_return.at(pos2), "HOWTO:DELETE");
				to_return.at(pos2).name() = "HOWTO:DELETE";
			}
			if(!is_question)
				add_header(to_return.at(pos1), ":DELETE");
		}
	}

	return to_return;
}

static bool number_pred_is_singular(const DrtPred &pred)
{
	string header = extract_header(pred);
	try {
		double w = boost::lexical_cast<double>(header);
		if (w == 1)
			return true;
	} catch (std::exception &e) {
		///
	}
	return false;
}

static vector<DrtPred> add_nouns_to_quantities(vector<DrtPred> to_return, vector<string> tags,
		vector<pair<pair<int, int>, constituents> > &connections)
// 3/CD(A) be/V(C,A,B) happy/JJ(B) -> 3/CD(A) person|!person(A) be/V(C,A,B) happy/JJ(B)
{
	for (int n = 0; n < to_return.size(); ++n) {
		if (to_return.at(n).is_number() && !to_return.at(n).is_date() && is_lonely_name_no_verbs(to_return, n)) {
			string fref = extract_first_tag(to_return.at(n));
			int cpos = find_complement_with_first_tag(to_return, fref, "GENITIVE");
			if(cpos != -1)
				continue;
			int vpos = find_verb_with_subject(to_return, tags, fref);
			if (vpos == -1)
				vpos = find_verb_with_object(to_return, tags, fref);
			if (vpos != -1) {
				string dummyname = string("dummyref") + boost::lexical_cast<string>(n);
				bool is_singular = number_pred_is_singular(to_return.at(n));
				DrtPred tmp_pred;
				if (is_singular)
					tmp_pred = DrtPred(string("thing/NN") + "(" + dummyname + ")");
				else
					tmp_pred = DrtPred(string("thing/NNS") + "(" + dummyname + ")");
				to_return.push_back(tmp_pred);
				string quant_ref = string("name") + boost::lexical_cast<string>(to_return.size());
				implant_first(to_return.at(n), quant_ref);
				DrtPred quant_pred(string("@QUANTITY") + "(" + fref + "," + quant_ref + ")");
				to_return.push_back(quant_pred);
				to_return = substitute_ref(to_return, fref, dummyname);
			}
		}
	}

	return to_return;
}

static vector<DrtPred> add_nouns_to_quantifiers(vector<DrtPred> to_return, vector<string> tags,
		vector<pair<pair<int, int>, constituents> > &connections)
// some/CD(A) be/V(C,A,B) happy/JJ(B) -> some/CD(A) person|!person(A) be/V(C,A,B) happy/JJ(B)
{
	for (int n = 0; n < to_return.size(); ++n) {
		if (is_quantifier_name(extract_header(to_return.at(n)))
				&& is_lonely_name_no_verbs(to_return, n)
				) {
			string fref = extract_first_tag(to_return.at(n));
			int vpos = find_verb_with_subject(to_return, tags, fref);
			if (vpos == -1)
				vpos = find_verb_with_object(to_return, tags, fref);
			if (vpos != -1) {
				string dummyname = string("dummyref") + boost::lexical_cast<string>(n);
				DrtPred tmp_pred(string("thing/NNS") + "(" + dummyname + ")");
				to_return.push_back(tmp_pred);
				string quant_ref = string("name") + boost::lexical_cast<string>(to_return.size());
				implant_first(to_return.at(n), quant_ref);
				DrtPred quant_pred(string("@QUANTIFIER") + "(" + fref + "," + quant_ref + ")");
				to_return.push_back(quant_pred);
				to_return = substitute_ref(to_return, fref, dummyname);
			}
		}
	}

	return to_return;
}

static vector<DrtPred> add_nouns_to_comparatives(vector<DrtPred> to_return, vector<string> tags,
		vector<pair<pair<int, int>, constituents> > &connections)
{
	for (int n = 0; n < to_return.size(); ++n) {;
		if (to_return.at(n).tag() == "JJR" && is_lonely_adjective_no_verbs(to_return, n) && !to_return.at(n).is_delete()) {
			string fref = extract_first_tag(to_return.at(n));
			int vpos = find_verb_with_subject(to_return, tags, fref);
			if (vpos == -1)
				vpos = find_verb_with_object(to_return, tags, fref);
			if (vpos != -1) {
				tagger_info *t = parser_singleton::get_tagger_info_instance();
				string dummyname = string("dummyname") + boost::lexical_cast<string>(n);
				DrtPred tmp_pred(string("thing/NN") + "(" + dummyname + ")");
				to_return.push_back(tmp_pred);
				string quant_ref = string("name") + boost::lexical_cast<string>(to_return.size());
				implant_first(to_return.at(n), quant_ref);
				string header = extract_header(to_return.at(n));
				header = t->get_base_superlative(header);
				implant_header(to_return.at(n), header);
				to_return.at(n).setTag("JJ");
				DrtPred quant_pred(string("@MORE") + "(" + fref + "," + quant_ref + ")");
				to_return.push_back(quant_pred);
				to_return = substitute_ref(to_return, fref, dummyname);
			}
		}
	}

	// process "David is more calm". the case "David is more calm than Maria" is already done in attach_comparatives()
	for (int n = 0; n < to_return.size(); ++n) {
		string head = extract_header(to_return.at(n));
		head = head.substr(0, head.find(":"));
		if (to_return.at(n).tag() == "JJR" && head == "more") {
			string fref = extract_first_tag(to_return.at(n));
			int vpos = find_verb_with_subject(to_return, tags, fref);
			if (vpos == -1)
				vpos = find_verb_with_object(to_return, tags, fref);
			if (vpos != -1) {
				vector<int> poz = find_all_names_with_string_no_delete(to_return, fref);
				string dummyname2 = string("dummyname2") + boost::lexical_cast<string>(n);
				for (int m = 0; m < poz.size(); ++m) {
					int pos = poz.at(m);
					if (pos == n)
						continue;
					implant_first(to_return.at(pos), dummyname2);
				}
				tagger_info *t = parser_singleton::get_tagger_info_instance();
				string dummyname = string("dummyname") + boost::lexical_cast<string>(n);
				DrtPred tmp_pred(string("thing/NN") + "(" + dummyname + ")");
				to_return.push_back(tmp_pred);
				string header = extract_header(to_return.at(n));
				implant_header(to_return.at(n), header + ":DELETE");
				DrtPred quant_pred(string("@MORE") + "(" + fref + "," + dummyname2 + ")");
				to_return.push_back(quant_pred);
				to_return = substitute_ref(to_return, fref, dummyname);
			}
		}
	}

	return to_return;
}

static string get_quantifier_title(const string &str)
{
	string to_return = "@QUANTITY";
	if (str.find("[more-than]") != string::npos)
		to_return = "@MORE_THAN";
	if (str.find("[less-than]") != string::npos)
		to_return = "@LESS_THAN";
	return to_return;
}

static string simplify_number(const string &str)
{
	string to_return = str;
	string more_str = "[more-than]_";
	if (str.find(more_str) != string::npos) {
		to_return = str.substr(more_str.size(), str.size());
	}
	string less_str = "[less-than]_";
	if (str.find(less_str) != string::npos) {
		to_return = str.substr(less_str.size(), str.size());
	}
	return to_return;
}

static vector<DrtPred> process_unit_of_measure_place(vector<DrtPred> to_return, vector<string> tags,
		vector<pair<pair<int, int>, constituents> > &connections)
{
	int pos1, pos2;
	string str1, str2, tag1, tag2;

	vector<DrtPred> quants;

	int qnum = 1; //  the reference number for the numeral

	vector<pair<string, string> > ref_pair;

	// be(verb1,name0,name4) circa(verb1) 1400000000(name4) km(name4) from(verb1,ref7)
	// -> be(verb1,name0,name4) circa(verb1) 1400000000(name4) km(name4) from(name4,ref7)
	// -> be(verb1,name0,none) circa(verb1) @PLACE_AT(verb1,name4) 1400000000(name4) km(name4) from(name4,ref7)
	vector<int> already_done;
	for (int n = 0; n < connections.size(); ++n) { // connects numerals and names with @QUANTITY(number,name)
		pos1 = connections.at(n).first.first;
		pos2 = connections.at(n).first.second;
		str1 = to_return.at(pos1).name();
		str2 = to_return.at(pos2).name();
		tag1 = to_return.at(pos1).tag();
		tag2 = to_return.at(pos2).tag();

		if ( //is_unit_of_measure(str1)
		points_to_unit_of_measure(to_return, to_return.at(pos1)) && is_preposition(tag2)
				&& find(already_done.begin(), already_done.end(), pos1) == already_done.end()) {
			string fref = extract_first_tag(to_return.at(pos1));
			to_return.at(pos2) = implant_first(to_return.at(pos2), fref);
			already_done.push_back(pos1);

			int m = find_verb_with_object(to_return, fref);
			if (m != -1) {
				implant_object(to_return.at(m), "none");
				DrtPred tmp_place("@PLACE_AT(dummy1,dummy2)");
				implant_first(tmp_place, extract_first_tag(to_return.at(m)));
				implant_second(tmp_place, fref);
				to_return.push_back(tmp_place);
			}
		}
	}

	for (int n = 0; n < ref_pair.size(); ++n) {
		string ref = ref_pair.at(n).first;
		string name = ref_pair.at(n).second;
		to_return = substitute_ref(to_return, ref, name);
	}
	return to_return;
}

static vector<DrtPred> process_quantities(vector<DrtPred> to_return, vector<string> tags,
		vector<pair<pair<int, int>, constituents> > &connections)
{
	int pos1, pos2;
	string str1, str2, tag1, tag2;

	vector<DrtPred> quants;

	int qnum = 1; //  the reference number for the numeral

	vector<pair<string, string> > ref_pair;

	vector<int> already_done;
	for (int n = 0; n < connections.size(); ++n) { // connects numerals and names with @QUANTITY(number,name)
		pos1 = connections.at(n).first.first;
		pos2 = connections.at(n).first.second;
		str1 = to_return.at(pos1).name();
		str2 = to_return.at(pos2).name();
		tag1 = to_return.at(pos1).tag();
		tag2 = to_return.at(pos2).tag();

		if (!there_are_only_names_between(tags, pos1, pos2))
			continue;

		if (is_numeral(tag1) && !is_date(str1) && !is_numeral(tag2) && is_name(tag2)
				&& find(already_done.begin(), already_done.end(), pos1) == already_done.end()) {
			string title = get_quantifier_title(str1);
			DrtPred tmp_quant(title + "(dummy1,dummy2)");
			string number_ref = extract_first_tag(to_return.at(pos1));
			string orig_ref = number_ref;
			string name_ref = extract_first_tag(to_return.at(pos2));
			number_ref = set_ref_number(number_ref, to_return.size() + qnum);
			to_return.at(pos1) = implant_first(to_return.at(pos1), number_ref);
			++qnum;
			tmp_quant = implant_second(tmp_quant, number_ref);
			tmp_quant = implant_first(tmp_quant, name_ref);
			quants.push_back(tmp_quant);
			already_done.push_back(pos1);
			if (orig_ref != name_ref)
				ref_pair.push_back(make_pair(orig_ref, number_ref));
			implant_header(to_return.at(pos1), simplify_number(extract_header(to_return.at(pos1))));

		} else if (is_numeral(tag2) && !is_date(str2) && !is_numeral(tag1) && is_name(tag1)
				&& find(already_done.begin(), already_done.end(), pos2) == already_done.end()) {
			string title = get_quantifier_title(str1);
			DrtPred tmp_quant(title + "(dummy1,dummy2)");
			string number_ref = extract_first_tag(to_return.at(pos2));
			string orig_ref = number_ref;
			string name_ref = extract_first_tag(to_return.at(pos1));
			number_ref = set_ref_number(number_ref, to_return.size() + qnum);
			to_return.at(pos2) = implant_first(to_return.at(pos2), number_ref);
			++qnum;
			tmp_quant = implant_second(tmp_quant, number_ref);
			tmp_quant = implant_first(tmp_quant, name_ref);
			quants.push_back(tmp_quant);
			already_done.push_back(pos2);
			if (orig_ref != name_ref)
				ref_pair.push_back(make_pair(orig_ref, number_ref));
			implant_header(to_return.at(pos2), simplify_number(extract_header(to_return.at(pos2))));
		}
	}
	for (int n = 0; n < ref_pair.size(); ++n) {
		string ref = ref_pair.at(n).first;
		string name = ref_pair.at(n).second;
		to_return = substitute_ref(to_return, ref, name);
	}
	to_return.insert(to_return.end(), quants.begin(), quants.end());

	return to_return;
}

static vector<DrtPred> process_quantifiers(vector<DrtPred> to_return, vector<string> tags,
		vector<pair<pair<int, int>, constituents> > &connections)
{
	int pos1, pos2;
	string str1, str2, tag1, tag2;

	vector<DrtPred> quants;

	int qnum = 1; //  the reference number for the numeral

	vector<pair<string, string> > ref_pair;

	vector<int> already_done;
	for (int n = 0; n < connections.size(); ++n) { // connects numerals and names with @QUANTITY(number,name)
		pos1 = connections.at(n).first.first;
		pos2 = connections.at(n).first.second;
		str1 = to_return.at(pos1).name();
		str2 = to_return.at(pos2).name();
		tag1 = to_return.at(pos1).tag();
		tag2 = to_return.at(pos2).tag();

		if (is_quantifier_name(str1) && !is_date(str1) && !is_numeral(tag2) && is_name(tag2)
				&& find(already_done.begin(), already_done.end(), pos1) == already_done.end()) {
			DrtPred tmp_quant("@QUANTIFIER(dummy1,dummy2)");
			string quantifier_ref = extract_first_tag(to_return.at(pos1));
			string name_ref = str1;
			implant_first(tmp_quant, quantifier_ref);
			implant_second(tmp_quant, name_ref);
			add_header(to_return.at(pos1), ":DELETE");
			to_return.at(pos1).name() += ":DELETE";
			quants.push_back(tmp_quant);

		} else if (is_quantifier_name(tag2) && !is_date(str2) && !is_numeral(tag1) && is_name(tag1)
				&& find(already_done.begin(), already_done.end(), pos2) == already_done.end()) {
			DrtPred tmp_quant("@QUANTIFIER(dummy1,dummy2)");
			string quantifier_ref = extract_first_tag(to_return.at(pos2));
			string name_ref = str2;
			implant_first(tmp_quant, quantifier_ref);
			implant_second(tmp_quant, name_ref);
			add_header(to_return.at(pos2), ":DELETE");
			to_return.at(pos2).name() += ":DELETE";
			quants.push_back(tmp_quant);
		}
	}
	to_return.insert(to_return.end(), quants.begin(), quants.end());

	return to_return;
}

static vector<DrtPred> process_comparatives(vector<DrtPred> to_return, vector<string> tags,
		vector<pair<pair<int, int>, constituents> > &connections)
// JJR are treated as CD
{
	int pos1, pos2;
	string str1, str2, tag1, tag2;
	vector<DrtPred> quants;
	int qnum = 1; //  the reference number for the numeral
	vector<pair<string, string> > ref_pair;
	tagger_info *t = parser_singleton::get_tagger_info_instance();

	vector<int> already_done;
	for (int n = 0; n < connections.size(); ++n) { // connects numerals and names with @QUANTITY(number,name)
		pos1 = connections.at(n).first.first;
		pos2 = connections.at(n).first.second;
		str1 = to_return.at(pos1).name();
		str2 = to_return.at(pos2).name();
		tag1 = to_return.at(pos1).tag();
		tag2 = to_return.at(pos2).tag();

		if (!there_are_only_names_between(tags, pos1, pos2))
			continue;

		if (tag1 == "JJR" && is_name(tag2) && find(already_done.begin(), already_done.end(), pos1) == already_done.end()) {
			string title = "@MORE";
			DrtPred tmp_quant(title + "(dummy1,dummy2)");
			string number_ref = extract_first_tag(to_return.at(pos1));
			string orig_ref = number_ref;
			string name_ref = extract_first_tag(to_return.at(pos2));
			number_ref = set_ref_number(number_ref, to_return.size() + qnum);
			to_return.at(pos1) = implant_first(to_return.at(pos1), number_ref);
			string header = extract_header(to_return.at(pos1));
			header = t->get_base_superlative(header);
			if (header.find("more") != string::npos)
				continue; // more is processed elsewhere
			implant_header(to_return.at(pos1), header);
			to_return.at(pos1).setTag("JJ");
			++qnum;
			tmp_quant = implant_second(tmp_quant, number_ref);
			tmp_quant = implant_first(tmp_quant, name_ref);
			quants.push_back(tmp_quant);
			already_done.push_back(pos1);
			if (orig_ref != name_ref)
				ref_pair.push_back(make_pair(orig_ref, number_ref));
			implant_header(to_return.at(pos1), simplify_number(extract_header(to_return.at(pos1))));

		} else if (tag2 == "JJR" && is_name(tag1)
				&& find(already_done.begin(), already_done.end(), pos2) == already_done.end()) {
			string title = "@MORE";
			DrtPred tmp_quant(title + "(dummy1,dummy2)");
			string number_ref = extract_first_tag(to_return.at(pos2));
			string orig_ref = number_ref;
			string name_ref = extract_first_tag(to_return.at(pos1));
			number_ref = set_ref_number(number_ref, to_return.size() + qnum);
			to_return.at(pos2) = implant_first(to_return.at(pos2), number_ref);
			string header = extract_header(to_return.at(pos2));
			if (header.find("more") != string::npos)
				continue; // more is processed elsewhere
			header = t->get_base_superlative(header);
			implant_header(to_return.at(pos2), header);
			to_return.at(pos1).setTag("JJ");
			++qnum;
			tmp_quant = implant_second(tmp_quant, number_ref);
			tmp_quant = implant_first(tmp_quant, name_ref);
			quants.push_back(tmp_quant);
			already_done.push_back(pos2);
			if (orig_ref != name_ref)
				ref_pair.push_back(make_pair(orig_ref, number_ref));
			implant_header(to_return.at(pos2), simplify_number(extract_header(to_return.at(pos2))));
		}
	}
	for (int n = 0; n < ref_pair.size(); ++n) {
		string ref = ref_pair.at(n).first;
		string name = ref_pair.at(n).second;
		to_return = substitute_ref(to_return, ref, name);
	}
	to_return.insert(to_return.end(), quants.begin(), quants.end());

	for(int n=0; n < to_return.size(); ++n) {
		string header = extract_header(to_return.at(n));
		if(to_return.at(n).is_complement() && header == "@COMPARED_TO") {
			string sref = extract_second_tag(to_return.at(n));
			int m= find_verb_with_object(to_return,sref);
			if(m != -1) {
				implant_object(to_return.at(m),"none");
			}
		}
	}

	return to_return;
}

static vector<DrtPred> translate_somethings(vector<DrtPred> to_return, const vector<string> &tags)
{
	for (int n = 0; n < to_return.size(); ++n) {
		string head = extract_header(to_return.at(n));
		if (to_return.at(n).is_name() && head == "something") {
			implant_header(to_return.at(n), get_local_what());
			to_return.at(n).name() = get_local_what();
		}
		if (to_return.at(n).is_name() && head == "someone") {
			implant_header(to_return.at(n), "person");
			to_return.at(n).name() = "person";
		}
		if (to_return.at(n).is_name() && head == "somebody") {
			implant_header(to_return.at(n), "person");
			to_return.at(n).name() = "person";
		}
		if (to_return.at(n).is_name() && head == "everybody") {
			implant_header(to_return.at(n), "person");
			to_return.at(n).name() = "person";
			string fref = extract_first_tag(to_return.at(n) );
			DrtPred tmp_pred("@QUANTIFIER("+fref+",all)");
			to_return.push_back(tmp_pred);
		}
		if (to_return.at(n).is_name() && head == "nobody") {
			implant_header(to_return.at(n), "person");
			to_return.at(n).name() = "person";
			string fref = extract_first_tag(to_return.at(n) );
			DrtPred tmp_pred("@QUANTIFIER("+fref+",all)");
			int pos_verb = percolate_to_verb_integer(to_return,tags,n);
			DrtPred not_pred;
			if (pos_verb != -1) {
				string vref = extract_first_tag(to_return.at(pos_verb));
				not_pred = DrtPred("not/RB(" + vref + ")");
			} else {
				not_pred = DrtPred("not/RB(" + fref + ")");
			}
			to_return.push_back(tmp_pred);
			to_return.push_back(not_pred);
		}
	}

	return to_return;
}

static vector<DrtPred> process_somethings(vector<DrtPred> to_return, vector<pair<pair<int, int>, constituents> > &connections)
{
	int pos1, pos2;
	string str1, str2, tag1, tag2;

	vector<DrtPred> quants;

	int qnum = 1; //  the reference number for the numeral

	for (int n = 0; n < connections.size(); ++n) { // connects numerals and names with @QUANTITY(number,name)
		pos1 = connections.at(n).first.first;
		pos2 = connections.at(n).first.second;
		str1 = to_return.at(pos1).name();
		str2 = to_return.at(pos2).name();
		tag1 = to_return.at(pos1).tag();
		tag2 = to_return.at(pos2).tag();

		if (str2 == "something") {
			if (is_verb(tag1) && str1 == "do" && points_to_object(to_return, to_return.at(pos2))) {
				add_header(to_return.at(pos2), ":DELETE");
				to_return.at(pos2).name() += ":DELETE";
			} else {
				implant_header(to_return.at(pos2), get_local_what());
				to_return.at(pos2).name() = get_local_what();
			}
		} else if (str1 == "something") {
			if (is_verb(tag2) && str2 == "do" && points_to_object(to_return, to_return.at(pos1))) {
				add_header(to_return.at(pos1), ":DELETE");
				to_return.at(pos1).name() += ":DELETE";
			} else {
				implant_header(to_return.at(pos1), get_local_what());
				to_return.at(pos1).name() = get_local_what();
			}
		}
	}

	return to_return;
}

bool is_candidate_prep_to(const composition &comp, int npred, int ncandidate, const constituents &constit)
{
	std::stringstream str_constit;
	composition_tree ct = comp.tree();
	composition_tree::height_iterator ctiter(ct, 1);
	constituents cprepto;
	for (int m = 0; ctiter != ct.end(); ++ctiter, ++m) { // parse all the elements until the verb is reached
		if (m == ncandidate)
			cprepto = *ctiter;
	}

	std::stringstream sstream;
	cprepto.print(sstream);
	string sprepto = sstream.str();

	// only a "NP" can be a preposition to. constituents like NP_i/NP_j are not valid
	if (sprepto.find("\\") != string::npos || sprepto.find("/") != string::npos)
		return false;
	return true;
}

bool is_subject(const composition &comp, int nverb, int ncandidate, const constituents &constit, bool is_aux = false)
{
	std::stringstream str_constit;
	constit.print_like_pred(str_constit);
	vector<constituents> checks;
	checks.push_back(string("/(\\(S[dcl],") + str_constit.str() + "),S[dcl])"); // ((S[dcl]_2\NP_6)_6/S[dcl]_7)_7
	checks.push_back(string("/(S[q],") + str_constit.str() + ")");
	if (is_aux) // "What uses the conventional way?" has "conventional way" as an object
		checks.push_back(string("/(S[wq],") + str_constit.str() + ")");
	checks.push_back(string("/(/(VP,S[wq]),") + str_constit.str() + ")"); //((VP_2/S[wq]_1)_1/NP_4)_4
	checks.push_back(string("/(/(S[q],VP),") + str_constit.str() + ")"); // ((S[q]_9/VP_4)_4/NP_11)_11
	//checks.push_back( string("/(/(/(S[q],S[dcl]),_A),") + str_constit.str() + ")" ); //(((S[q]_0/S[dcl]_1)_1/NP_3)_3/NP_5)_5
	checks.push_back(string("/(/(/(S[q],S[dcl]),") + str_constit.str() + "),_A)"); //(((S[q]_0/S[dcl]_1)_1/NP_3)_3/NP_5)_5
	checks.push_back(string("/(/(/(S[q],S[dcl]),VP),") + str_constit.str() + ")"); //(((S[q]_0/S[dcl]_1)_1/VP_3)_3/NP_5)_5
	checks.push_back(string("/(/(/(S[wq],VP),_A),") + str_constit.str() + ")"); // (((S[wq]_1/VP_5)_5/NP_7)_7/NP_9)_9
	checks.push_back(string("/(/(\\(S[wq],_A),VP),") + str_constit.str() + ")"); // (((S[wq]_0\NP_12)_12/VP_13)_13/NP_15)_15
	checks.push_back(string("/(\\(/(S[q],S[dcl]),") + str_constit.str() + "),_A)"); // (((S[q]_0/S[dcl]_1)_1\NP_3)_3/NP_4)_4
	checks.push_back(string("/(/(\\(S[q],") + str_constit.str() + "),VP),_A)"); // (((S[q]_0\NP_2)_2/VP_3)_3/NP_5)_5
	checks.push_back(string("/(\\(S[q],") + str_constit.str() + "),S[dcl])"); //((S[q]_0\NP_1)_1/S[dcl]_2)_2
	checks.push_back(string("/(/(\\(\\(S[q],_A),") + str_constit.str() + "),S[dcl]),_B)"); // ((((S[q]_0\NP_1)_1\NP_4)_4/S[dcl]_8)_8/NP_10)_10
	checks.push_back(string("/(/(/(S[wq],S[dcl]),VP),") + str_constit.str() + ")"); // (((S[wq]_0/S[dcl]_1)_1/VP_3)_3/NP_5)_5
	checks.push_back(string("/(/(/(S[q],_A),_B),") + str_constit.str() + ")"); //(((S[q]_0/NP_1)_1/NP_3)_3/NP_5)_5
	checks.push_back(string("/(/(VP,/(S[q],S[q]),_B),") + str_constit.str() + ")"); // ((VP_4/(S[q]_1/S[q]_0)_0)_0/NP_6)_6
	checks.push_back(string("/(\\(S[q],") + str_constit.str() + "),_A)"); //((S[q]_0\NP_2)_2/NP_3)_3
	checks.push_back(string("/(/(\\(S[wq],_A),") + str_constit.str() + "),_B)"); // (((S[wq]_0\NP_1)_1/NP_3)_3/NP_5)_5
	checks.push_back(string("/(/(/(\\(S[wq],_A),S[dcl]),_B),") + str_constit.str() + ")"); // ((((S[wq]_0\NP_1)_1/S[dcl]_2)_2/NP_4)_4/NP_6)_6
	if (str_constit.str().find("[ex]") != string::npos) {
		checks.push_back(string("/(/(S[dcl],VP),") + str_constit.str() + ")"); // ((S[dcl]_9/VP_4)_4/NP[ex]_11)_11
	}
	checks.push_back(string("/(/(\\(S[wq]_0,") + str_constit.str() + "),S[dcl]),_A)"); // (((S[wq]_0\NP_1)_1/S[dcl]_2)_2/NP_4)_4
	checks.push_back(string("/(/(S[q],_A),") + str_constit.str() + ")");
	checks.push_back(string("/(/(S[wq],_A),") + str_constit.str() + ")");
	checks.push_back(string("/(S[inv],") + str_constit.str() + ")");
	checks.push_back(string("\\(S[dcl],") + str_constit.str() + ")");
	checks.push_back(string("\\(\\(S[dcl],") + str_constit.str() + "),S[dcl])");
	checks.push_back(string("\\(VP,") + str_constit.str() + ")");
	checks.push_back(string("/(\\(VP,") + str_constit.str() + "),_A)");
	checks.push_back(string("\\(\\(S[dcl],S[dcl]),") + str_constit.str() + ")");
	checks.push_back(string("/(\\(\\(S[dcl],_A),") + str_constit.str() + "),_B)"); // (((S[dcl]_1\NP_3)_3\NP_13)_13/NP_18)_18
	checks.push_back(string("/(\\(S[dcl],") + str_constit.str() + "),_A)");
	checks.push_back(string("\\(\\(S[dcl],_A),") + str_constit.str() + ")");
	checks.push_back(string("/(\\(S[dcl],") + str_constit.str() + "),VP)"); // ((S[dcl]_0\NP_9)_8/VP_10)_9;
	checks.push_back(string("\\(\\(S[dcl],") + str_constit.str() + "),_A)"); //((S[dcl]_0\NP_1)_1\NP_2)_2

	checks.push_back(string("/(\\(\\(S[dcl],") + str_constit.str() + "),S[dcl]),_B)"); //(((S[dcl]_0\NP_1)_1\S[dcl]_3)_3/NP_9)_9
	checks.push_back(string("/(/(\\(S[dcl],") + str_constit.str() + "),S[dcl]),VP)"); // (((S[dcl]_0\NP_4)_3/S[dcl]_6)_5/VP_7)_6
	checks.push_back(string("/(/(/(\\(S[dcl],") + str_constit.str() + "),S[dcl]),VP),_A)"); // ((((S[dcl]_0\NP_4)_3/S[dcl]_6)_5/VP_7)_6/NP_8)_7
	checks.push_back(string("/(/(\\(S[dcl],") + str_constit.str() + "),S[dcl]),_A)"); // (((S[dcl]_1\NP_2)_1/S[dcl]_7)_6/NP_8)_7
	checks.push_back(string("/(\\(/(S[dcl],_A),") + str_constit.str() + "),_C)"); // (((S[dcl]_0/NP_1)_0\NP_1)_0/NP_2)_1

	checks.push_back(string("/(\\(_A,") + str_constit.str() + "),VP)"); //((NP_13\NP[ex]_13)_13/VP_14)_14

	checks.push_back(string("/(/(/(\\(S[dcl],") + str_constit.str() + "),S[dcl]),_A),_B)"); // ((((S[dcl]_1\NP_30)_30/S[dcl]_31)_31/NP_32)_32/NP_33)_33
	checks.push_back(string("/(/(\\(S[dcl],") + str_constit.str() + "),VP),VP)");
	checks.push_back(string("/(/(\\(/(S[dcl],_A),") + str_constit.str() + "),VP),_B)"); // ((((S[dcl]_0/NP_1)_1\NP_2)_2/VP_3)_3/NP_4)_4
	checks.push_back(string("/(/(\\(S[dcl],") + str_constit.str() + "),VP),_A)"); // (((S[dcl]_0\NP_1)_1/VP_2)_2/NP_3)_3

	checks.push_back(string("/(/(VP,\\(VP,") + str_constit.str() + ")),_A)"); // ((VP_7/(VP_3\NP_6)_6)_6/NP_9)_9

	checks.push_back(string("/(/(\\(/(S[dcl],S[dcl]),") + str_constit.str() + "),_A),_B)"); // ((((S[dcl]_8/S[dcl]_7)_7\NP_10)_10/NP_11)_11/NP_13)_13
	checks.push_back(string("\\(\\(_A,S[dcl]),") + str_constit.str() + ")"); // ((NP_18\S[dcl]_1)_18\NP_21)_21
	checks.push_back(string("/(/(\\(\\(S[dcl],_A),") + str_constit.str() + "),VP),_B)"); // ((((S[dcl]_0\NP_1)_1\NP_7)_7/VP_10)_10/NP_12)_12

	checks.push_back(string("/(\\(\\(\\(S[dcl],_A),S[dcl]),") + str_constit.str() + "),_B)"); // ((((S[dcl]_0\NP_1)_1\S[dcl]_4)_4\NP_14)_14/NP_19)_19
	checks.push_back(string("/(\\(\\(\\(S[dcl],_A),_B),") + str_constit.str() + "),_C)"); // (((S[dcl]_2\NP_4)_4\NP_5)_5/S[dcl]_6)_6
	checks.push_back(string("/(/(\\(\\(S[dcl],_A),") + str_constit.str() + "),S[dcl]),VP)"); //((((S[dcl]_0\NP_1)_1\NP_7)_7/S[dcl]_8)_8/VP_9)_9
	checks.push_back(string("/(\\(/(/(S[dcl],VP),_A),") + str_constit.str() + "),_C)"); //((((S[dcl]_0/VP_1)_1/NP_2)_2\NP_2)_2/PP_9)_9
	checks.push_back(string("/(\\(\\(S[dcl],") + str_constit.str() + "),_A),_B)"); //(((S[dcl]_0\NP_18)_18\NP_19)_19/NP_20)_20
	checks.push_back(string("/(/(_A,/(S[q],VP)),") + str_constit.str() + ")"); //((NP_4/(S[q]_0/VP_2)_2)_2/NP_6)_6

	//checks.push_back( string("/(/(\\(\\(S[dcl],") + str_constit.str() + "),S[dcl]),S[dcl]),_A)" ); // ((((S[dcl]_0\NP_1)_1\S[dcl]_4)_4/S[dcl]_12)_12/NP_14)_14

	checks.push_back(string("/(/(\\(") + str_constit.str() + ",S[dcl]),_A),_B)"); // (((NP_1\S[dcl]_4)_4/NP_18)_18/NP_20)_20

	checks.push_back(string("/(\\(\\(S[dcl],S[dcl]),") + str_constit.str() + "),VP)"); //(((S[dcl]_0\S[dcl]_0)_0\NP_7)_7/VP_9)_9
	checks.push_back(string("\\(\\(/(S[dcl],S[dcl]),_A),") + str_constit.str() + ")"); // (((S[dcl]_0/S[dcl]_1)_1\NP_1)_1\NP_2)_2
	checks.push_back(string("/(\\(\\(S[dcl],_A),") + str_constit.str() + "),S[dcl])"); //(((S[dcl]_0\NP_0)_0\NP_1)_1/S[dcl]_3)_3

	checks.push_back(string("/(/(S[dcl],\\(S[dcl],") + str_constit.str() + ")),_A)"); //((S[dcl]_2/(S[dcl]_0\NP_0)_0)_0/VP_4)_4
	checks.push_back(string("/(/(\\(/(\\(_A,S[dcl]),_B),") + str_constit.str() + "),_C),_D)"); //(((((NP_3\S[dcl]_2)_3/PP_8)_8\NP_8)_8/NP_10)_10/NP_11)_11
	checks.push_back(string("/(S[dcl],\\(S[dcl],") + str_constit.str() + "))"); //(S[dcl]_3/(S[dcl]_0\NP_1)_1)_1
	checks.push_back(string("/(/(VP,\\(S[dcl],") + str_constit.str() + ")),_A)"); // ((VP_10/(S[dcl]_6\NP_9)_9)_9/NP_12)_12
	checks.push_back(string("/(\\(\\(\\(S[dcl],_A),S[dcl]),") + str_constit.str() + "),_B)"); // ((((S[dcl]_0\NP_10)_10\S[dcl]_11)_11\NP_17)_17/NP_18)_18
	checks.push_back(string("/(/(\\(\\(\\(\\(S[dcl],_A),S[dcl]),_B),") + str_constit.str() + "),_C),_D)"); // ((((((S[dcl]_0\NP_10)_10\S[dcl]_11)_11\NP_17)_17\NP_24)_24/NP_25)_25/NP_27)_27
	checks.push_back(string("\\(/(/(/(S[dcl],_A),S[dcl]),S[dcl]),") + str_constit.str() + ")"); // ((((S[dcl]_11/NP_7)_7/S[dcl]_13)_13/S[dcl]_15)_15\NP_17)_17
	checks.push_back(string("/(\\(/(\\(S[dcl],S[dcl]),S[dcl]),") + str_constit.str() + "),_A)"); // ((((S[dcl]_0\S[dcl]_6)_6/S[dcl]_14)_14\NP_16)_16/NP_17)_17
	checks.push_back(string("/(\\(\\(_A,/(S[dcl],S[dcl])),") + str_constit.str() + "),_B)"); //(((NP_2\(S[dcl]_1/S[dcl]_0)_0)_2\NP_7)_7/NP_8)_8
	checks.push_back(string("/(\\(/(/(S[dcl],S[dcl]),S[dcl]),") + str_constit.str() + "),_B)"); // ((((S[dcl]_0/S[dcl]_1)_1/S[dcl]_3)_3\NP_5)_5/NP_6)_6

	checks.push_back(string("\\(/(S[dcl],_A),") + str_constit.str() + ")");
	checks.push_back(string("/(\\(\\(S[dcl],S[dcl]),") + str_constit.str() + "),_A)");
	checks.push_back(string("/(\\(/(S[dcl],S[dcl]),") + str_constit.str() + "),_A)");
	checks.push_back(string("/(\\(S[ng],") + str_constit.str() + "),_A)");
	checks.push_back(string("\\(S[ng],") + str_constit.str() + ")");
	checks.push_back(string("/(\\(S[to],") + str_constit.str() + "),_A)");
	checks.push_back(string("/(\\(S[wq],") + str_constit.str() + "),_A)");
	checks.push_back(string("\\(S[wq],") + str_constit.str() + ")");

	checks.push_back(string("/(") + str_constit.str() + ",_A)"); // S[dcl] can be a subject //// Error!!!!
	checks.push_back(string("\\(_A,") + str_constit.str() + ")"); // NP_i\NP_j //// Error!?
	checks.push_back(string("/(") + str_constit.str() + ",VP)"); // S[dcl] can be a subject //// Error!!!!

	// After an indirect object
	checks.push_back(string("/(/(\\(S[dcl],") + str_constit.str() + "),_A),_B)"); //(((S[dcl]_0\NP_1)_1/NP_2)_2/NP_3)_3
	checks.push_back(string("/(/(\\(VP,") + str_constit.str() + "),_A),_B)"); //(((VP\NP_1)_1/NP_2)_2/NP_3)_3

	// For WDTs
	checks.push_back(string("/(/(") + str_constit.str() + ",S[dcl]),VP)");

	composition_tree ct = comp.tree();
	composition_tree::height_iterator ctiter(ct, 1);
	constituents cverb;
	for (int m = 0; ctiter != ct.end(); ++ctiter, ++m) { // parse all the elements until the verb is reached
		if (m == nverb)
			cverb = *ctiter;
	}

	// For PRNs
	if (cverb == constit)  // NP_1
		return true;

	// Generic check for S[dcl]
	std::stringstream sstream;
	cverb.print(sstream);
	string sverb = sstream.str();
	string ssubj = str_constit.str();
	int spos = sverb.find(ssubj);
	if (spos != string::npos && ssubj.find("NP") != string::npos) {
		if (spos > 0 && sverb.at(spos - 1) == '\\') {
			string sleft = sverb.substr(0, spos);
			if ((sleft.find("S[dcl]") != string::npos || sleft.find("VP") != string::npos)
					&& sleft.find("S[wq]") == string::npos && sleft.find("S[q]") == string::npos
					&& sleft.find("S[ng]") == string::npos && sleft.find("S[inv]") == string::npos)
				return true;
		}
	}

	// Check with unification by using the sample consistuents in "checks"
	FTree<pair<string, int> > cverb_tree(cverb.tree());
	FTree<pair<string, int> >::iterator cverb_iter = cverb_tree.begin();
	vector<constituents>::iterator checks_iter = checks.begin();
	for (; checks_iter != checks.end(); ++checks_iter) {
		//cout << *checks_iter<< endl;
		FTree<pair<string, int> > cverb_subtree(cverb_iter);
		constituents cverb_tmp(cverb_subtree);
		if (checks_iter->can_unify(cverb_tmp))
			return true;
	}

	return false;
}

DrtVect sign_pivots(DrtVect pre_drt, const composition &comp)
{
	composition_tree ct = comp.tree();
	composition_tree::height_iterator ctiter(ct, 1);
	constituents cname;

	for (int m = 0; ctiter != ct.end(); ++ctiter, ++m) { // parse all the elements until the verb is reached
		if (pre_drt.at(m).is_name() || pre_drt.at(m).is_article()) {
			std::stringstream sstream;
			ctiter->print(sstream);
			string sname = sstream.str();
			// only a "NP" can be a pivot. constituents like NP_i/NP_j are not valid
			if (sname.find("\\") == string::npos && sname.find("/") == string::npos) {
				pre_drt.at(m).set_pivot();
			}
		}
	}

	return pre_drt;
}

bool is_object(const composition &comp, int nverb, int ncandidate, const constituents &constit)
{
	std::stringstream str_constit;
	constit.print_like_pred(str_constit);
	vector<constituents> checks;

	checks.push_back(string("/(/(\\(S[dcl],_A),S[dcl]),") + str_constit.str() + ")"); //(((S[dcl]_1\NP_3)_3/S[dcl]_4)_4/NP_5)_5

	checks.push_back(string("/(/(S[q],") + str_constit.str() + "),_A)");
	checks.push_back(string("/(/(S[wq],") + str_constit.str() + "),_A)");
	checks.push_back(string("/(S[wq],") + str_constit.str() + ")");
	checks.push_back(string("/(\\(S[dcl],_A),") + str_constit.str() + ")");
	checks.push_back(string("/(/(S[dcl],_A),") + str_constit.str() + ")");
	checks.push_back(string("/(/(VP,_A),") + str_constit.str() + ")");
	checks.push_back(string("\\(/(S[dcl],") + str_constit.str() + "),_A)");
	checks.push_back(string("/(S[dcl],") + str_constit.str() + ")");
	checks.push_back(string("/(VP,") + str_constit.str() + ")");
	checks.push_back(string("/(\\(S[ng],_A),") + str_constit.str() + ")");
	checks.push_back(string("/(\\(S[to],_A),") + str_constit.str() + ")");
	checks.push_back(string("/(\\(S[wq],_A),") + str_constit.str() + ")");
	checks.push_back(string("/(/(/(\\(S[wq],_A),S[dcl]),") + str_constit.str() + "),_B)"); // ((((S[wq]_0\NP_1)_1/S[dcl]_2)_2/NP_4)_4/NP_6)_6
	checks.push_back(string("/(\\(\\(S[dcl],S[dcl]),_A),") + str_constit.str() + ")");
	checks.push_back(string("/(\\(/(S[dcl],S[dcl]),_A),") + str_constit.str() + ")");
	checks.push_back(string("/(\\(\\(S[dcl],_A),\\(S[dcl],_B)),") + str_constit.str() + ")");
	checks.push_back(string("/(/(\\(S[dcl],_A),\\(S[dcl],_B)),") + str_constit.str() + ")");
	checks.push_back(string("/(/(/(/(S[wq],S[wq]),S[dcl]),") + str_constit.str() + "),_A)"); //((((S[wq]_1/S[wq]_0)_0/S[dcl]_3)_3/NP_5)_5/NP[ex]_7)_7
	checks.push_back(string("/(/(/(S[q],VP),") + str_constit.str() + "),_A)"); //(((S[q]_0/VP_1)_1/NP_3)_3/NP[ex]_5)_5
	//checks.push_back( string("/(/(/(S[q],S[dcl]),") + str_constit.str() + "),_A)" ); //(((S[q]_0/S[dcl]_1)_1/NP_3)_3/NP_5)_5
	checks.push_back(string("/(/(/(S[q],_A),") + str_constit.str() + "),_B)"); //(((S[q]_0/NP_2)_2/NP_4)_4/NP_6)_6
	checks.push_back(string("/(/(/(S[q],S[dcl]),") + str_constit.str() + "),_B)"); // (((S[q]_0/S[dcl]_1)_1/NP_3)_3/NP[ex]_5)_5

	checks.push_back(string("/(/(VP,VP),") + str_constit.str() + ")"); // ((VP_2/VP_4)_4/NP_6)_6

	checks.push_back(string("/(/(\\(/(S[dcl],_A),_B),VP),") + str_constit.str() + "_B)"); // ((((S[dcl]_0/NP_1)_1\NP_2)_2/VP_3)_3/NP_4)_4
	checks.push_back(string("/(/(\\(S[dcl],_A),S[dcl]),") + str_constit.str() + ")"); // (((S[dcl]_0\NP_1)_1/S[dcl]_2)_2/NP_3)_3

	checks.push_back(string("/(/(\\(S[dcl],_A),VP),") + str_constit.str() + ")"); // (((S[dcl]_0\NP_1)_1/VP_2)_2/NP_3)_3

	checks.push_back(string("/(\\(\\(\\(S[dcl],_A),S[dcl]),_B),") + str_constit.str() + ")"); // ((((S[dcl]_0\NP_1)_1\S[dcl]_4)_4\NP_14)_14/NP_19)_19
	checks.push_back(string("/(\\(\\(\\(S[dcl],_A),_B),_C),") + str_constit.str() + ")"); // (((S[dcl]_2\NP_4)_4\NP_5)_5/S[dcl]_6)_6

	checks.push_back(string("/(\\(/(S[dcl],") + str_constit.str() + "),_B),_C)"); // (((S[dcl]_0/NP_1)_0\NP_1)_0/NP_2)_1
	checks.push_back(string("/(\\(\\(S[dcl],_A),S[dcl]),") + str_constit.str() + ")"); //(((S[dcl]_0\NP_1)_1\S[dcl]_3)_3/NP_9)_9
	checks.push_back(string("/(VP,") + str_constit.str() + ")"); // ((VP_7/NP_7)_7/NP_9)_9
	checks.push_back(string("/(/(VP,") + str_constit.str() + "),_A)"); // (VP_7/NP_7)_7
	checks.push_back(string("/(_A,") + str_constit.str() + ")"); // (NP_7/NP_8)_8 /// this is for phrases with the VBG (... by congratulating him)

	checks.push_back(string("/(/(\\(S[dcl],_A),S[dcl]),") + str_constit.str() + ")"); //(((S[dcl]_0\NP_0)_0/S[dcl]_1)_1/NP_3)_3

	checks.push_back(string("/(\\(/(S[dcl],_A),_B),") + str_constit.str() + ")"); //(((S[dcl]_7/NP_14)_14\NP_15)_15/VP_18)_18
	checks.push_back(string("/(\\(\\(\\(S[dcl],_A),S[dcl]),_B),") + str_constit.str() + ")"); // ((((S[dcl]_0\NP_10)_10\S[dcl]_11)_11\NP_17)_17/NP_18)_18
	checks.push_back(string("/(/(\\(\\(\\(\\(S[dcl],_A),S[dcl]),_A),_B),_C),") + str_constit.str() + ")"); // ((((((S[dcl]_0\NP_10)_10\S[dcl]_11)_11\NP_17)_17\NP_24)_24/NP_25)_25/NP_27)_27
	checks.push_back(string("/(/(VP,VP),") + str_constit.str() + ")"); //((VP_28/VP_30)_30/NP_32)_32
	checks.push_back(string("/(/(/(\\(S[dcl],_A),S[dcl]),") + str_constit.str() + "),_C)"); // ((((S[dcl]_0\NP_13)_13/S[dcl]_14)_14/NP_16)_16/NP_18)_18

	// After indirect objects
	checks.push_back(string("/(/(\\(S[dcl],_A),") + str_constit.str() + "),_B)"); //(((S[dcl]_0\NP_2)_2/NP_3)_3/NP_4)_4
	checks.push_back(string("/(/(\\(VP,_A),") + str_constit.str() + "),_B)"); //(((S[dcl]_0\NP_2)_2/NP_3)_3/NP_4)_4
	checks.push_back(string("/(/(S[dcl],") + str_constit.str() + "),_B)");
	checks.push_back(string("/(/(VP,") + str_constit.str() + "),_B)");

	// For questions
	checks.push_back(string("/(") + str_constit.str() + ",_A)");
	checks.push_back(string("/(/(S[q],S[q]),") + str_constit.str() + ")"); // ((S[q]_1/S[q]_0)_0/NP_3)_3

	composition_tree ct = comp.tree();
	composition_tree::height_iterator ctiter(ct, 1);
	constituents cverb;
	for (int m = 0; ctiter != ct.end(); ++ctiter, ++m) { // parse all the elements until the verb is reached
		if (m == nverb)
			cverb = *ctiter;
	}

	FTree<pair<string, int> > cverb_tree(cverb.tree());
	FTree<pair<string, int> >::iterator cverb_iter = cverb_tree.begin();
	//for(; cverb_iter != cverb_tree.end(); ++cverb_iter) {
	{
		vector<constituents>::iterator checks_iter = checks.begin();
		for (; checks_iter != checks.end(); ++checks_iter) {
			FTree<pair<string, int> > cverb_subtree(cverb_iter);
			constituents cverb_tmp(cverb_subtree);
			if (checks_iter->can_unify(cverb_tmp))
				return true;
		}
	}

	return false;
}

bool is_valid_aux(const constituents &constit)
{
	std::stringstream str_constit;
	constit.print_like_pred(str_constit);
	string str = str_constit.str();
	//cout << "VALID:: "<< str << endl;

	if (str.find("S") == string::npos && str.find("VP") == string::npos)
		return false;

	return true;
}

static bool is_valid_prep_constit(const constituents &constit)
{
	std::stringstream str_constit;
	constit.print_like_pred(str_constit);
	string str = str_constit.str();
	//cout << "VALID:: "<< str << endl;

	if (str.find("/") != string::npos || str.find("\\") != string::npos)
		return false;

	return true;
}

bool is_indirect_object(const composition &comp, int nverb, int ncandidate, const constituents &constit)
{
	std::stringstream str_constit;
	constit.print_like_pred(str_constit);
	vector<constituents> checks;
	checks.push_back(string("/(/(\\(S[dcl],_A),_B),") + str_constit.str() + ")");
	checks.push_back(string("/(/(\\(VP,_A),_B),") + str_constit.str() + ")");
	checks.push_back(string("/(/(S[dcl],_B),") + str_constit.str() + ")");
	checks.push_back(string("/(/(VP,_B),") + str_constit.str() + ")");
	//checks.push_back( string("/(/(VP,") +str_constit.str()+"),_B)" );
	checks.push_back(string("/(/(\\(\\(\\(\\(S[dcl],_A),S[dcl]),_A),_B),") + str_constit.str() + "),_C)"); // ((((((S[dcl]_0\NP_10)_10\S[dcl]_11)_11\NP_17)_17\NP_24)_24/NP_25)_25/NP_27)_27
	checks.push_back(string("/(/(/(\\(S[dcl],_A),VP),") + str_constit.str() + "),_C)"); //((((S[dcl]_1\NP_2)_2/VP_10)_10/NP_12)_12/NP_14)_14
	checks.push_back(string("/(/(\\(S[wq],_A),_B),") + str_constit.str() + ")"); // (((S[wq]_0\NP_1)_1/NP_2)_2/NP_4)_4
	checks.push_back(string("/(/(/(\\(S[wq],_A),S[dcl]),") + str_constit.str() + "),_B)"); // ((((S[wq]_0\NP_1)_1/S[dcl]_2)_2/NP_4)_4/NP_6)_6
	checks.push_back(string("/(/(/(\\(S[dcl],_A),S[dcl]),") + str_constit.str() + "),_B)"); // ((((S[dcl]_22\NP_24)_24/S[dcl]_25)_25/NP_27)_27/NP_29)_29
	checks.push_back(string("/(/(/(S[dcl],VP),") + str_constit.str() + "),_B)"); // (((S[dcl]_53/VP_51)_51/NP_55)_55/NP_57)_57
	checks.push_back(string("/(/(/(VP,S[dcl]),")+str_constit.str()+"),_A)"); // (((VP_53/S[dcl]_55)_55/NP_57)_57/NP_59)_59
	composition_tree ct = comp.tree();
	composition_tree::height_iterator ctiter(ct, 1);
	constituents cverb;
	for (int m = 0; ctiter != ct.end(); ++ctiter, ++m) { // parse all the elements until the verb is reached
		if (m == nverb)
			cverb = *ctiter;
	}

	FTree<pair<string, int> > cverb_tree(cverb.tree());
	FTree<pair<string, int> >::iterator cverb_iter = cverb_tree.begin();
	//for(; cverb_iter != cverb_tree.end(); ++cverb_iter) {
	{
		vector<constituents>::iterator checks_iter = checks.begin();
		for (; checks_iter != checks.end(); ++checks_iter) {
			FTree<pair<string, int> > cverb_subtree(cverb_iter);
			constituents cverb_tmp(cverb_subtree);
			if (checks_iter->can_unify(cverb_tmp))
				return true;
		}
	}

	return false;
}

bool contain_question(const vector<DrtPred> &predicates)
{
	for (int n = 0; n < predicates.size(); ++n) {
		if (extract_header(predicates.at(n)) == "?")
			return true;
	}
	return false;
}

bool contain_condition(const vector<DrtPred> &predicates)
{
	for (int n = 0; n < predicates.size(); ++n) {
		if (extract_header(predicates.at(n)) == "@CONDITION")
			return true;
	}
	return false;
}

bool contain_fake_question(const vector<DrtPred> &predicates)
// "people that fly"
{
	if (predicates.size() > 2 && predicates.at(0).is_name()
			&& (((predicates.at(1).tag() == "IN") && extract_header(predicates.at(1)) == "that")
					|| (predicates.at(1).is_WP() || predicates.at(1).is_WDT())))
		return true;
	return false;
}

static bool subject_is_EX(const DrtVect &drtvect, int pos)
{
	vector<DrtPred> subj = find_subject_of_verb(drtvect, pos);
	if (debug) {
		cout << "EX2:: " << subj.size() << endl;
	}
	if (subj.size() == 0)
		return false;
	for (int n = 0; n < subj.size(); ++n) {
		if (debug) {
			cout << "EX:: " << subj.at(n) << " " << subj.at(n).tag() << endl;
		}
		if (subj.at(n).tag() == "EX")
			return true;
	}
	return false;
}

static bool subject_agrees_with_verb(const vector<DrtPred> &drt, int from, int to)
{
	if (!is_passive(drt.at(from))
			&& !string_is_subord(extract_subject(drt.at(from))) // for detached questions subordinates
			&& ((verb_is_singular(drt.at(from), drt) && drt.at(to).tag() == "VBP")
					|| (!verb_is_singular(drt.at(from), drt) && drt.at(to).tag() == "VBZ"))) {
		return false;
	}

	return true;
}

static vector<DrtPred> raise_subject_from_preposition(vector<DrtPred> pre_drt, const vector<string> &tags, int n)
{
	DrtPred prep(pre_drt.at(n));
	string first_verb_pos_str = extract_first_tag(pre_drt.at(n));
	string second_verb_pos_str = extract_second_tag(pre_drt.at(n));

	int pos_from = find_verb_with_string(pre_drt, tags, first_verb_pos_str);
	int pos_to = find_verb_with_string(pre_drt, tags, second_verb_pos_str);
	if (pos_to == -1)
		return pre_drt;
	if (has_subject(pre_drt.at(pos_to))) // if the verb at 'pos_to' already has a subject changes nothing
		return pre_drt;
	if (has_object(pre_drt.at(pos_to)) && is_passive(pre_drt.at(pos_to))) // if the verb at 'pos_to' already has a subject changes nothing
		return pre_drt;
	if (points_to_name_or_ref(first_verb_pos_str)) {
		pre_drt.at(pos_to) = implant_subject_safe(pre_drt.at(pos_to), first_verb_pos_str);
	}
	if (pos_from == -1)
		return pre_drt;
	if (pos_from >= tags.size() || pos_to >= tags.size())
		return pre_drt;
	else if (is_verb(tags.at(pos_to)) && is_verb(tags.at(pos_from)) && !is_passive(pre_drt.at(pos_from))) {
		if (has_subject(pre_drt.at(pos_from)) && !subject_is_EX(pre_drt, pos_from)
				&& subject_agrees_with_verb(pre_drt, pos_from, pos_to))
			pre_drt.at(pos_to) = implant_subject_safe(pre_drt.at(pos_to), extract_subject(pre_drt.at(pos_from)));
		else if (has_object(pre_drt.at(pos_from)))
			pre_drt.at(pos_to) = implant_subject_safe(pre_drt.at(pos_to), extract_object(pre_drt.at(pos_from)));
	} else if (is_verb(tags.at(pos_to)) && is_verb(tags.at(pos_from)) && is_passive(pre_drt.at(pos_from))) {
		pre_drt.at(pos_to) = implant_subject_safe(pre_drt.at(pos_to), extract_object(pre_drt.at(pos_from)));
	}

	return pre_drt;
}

static vector<DrtPred> raise_subject_from_verb(vector<DrtPred> pre_drt, const vector<string> &tags, int n)
{
	string second_verb_pos_str = extract_object(pre_drt.at(n));
	int pos_to = find_verb_with_string(pre_drt, tags, second_verb_pos_str);
	if (pos_to == -1)
		return pre_drt;
	pre_drt.at(pos_to) = implant_subject(pre_drt.at(pos_to), extract_subject(pre_drt.at(n)));
	return pre_drt;
}

static bool does_percolate_to_verb(vector<DrtPred> pre_drt, vector<string> &tags, int n)
{
	if (is_verb(tags.at(n)))
		return true;

	DrtPred to_return(pre_drt.at(n));

	string from_str = extract_first_tag(pre_drt.at(n));
	int m = find_verb_with_subject(pre_drt, tags, from_str);
	if (m == -1) {
		m = find_verb_with_object(pre_drt, tags, from_str);
	}
	if (m != -1)
		return true;

	return false;
}

vector<DrtPred> tough_move(vector<DrtPred> pre_drt, const vector<pair<pair<int, int>, constituents> > &connections,
		const composition &comp, const vector<string> &tags, const vector<string> &names, vector<bool> *mask)
{
	string str1, str2, tag1, tag2;
	int pos1, pos2;

	constituents check("/(/(\\(\\(S[adj],_subject),_object),_verb),_adj)");
	constituents adj("_adj");
	constituents subj("_subject");
	constituents obj("_object");
	constituents toverb("_verb");

	constituents_mgu mgu;
	int pos_subj, pos_obj, pos_verb_aux, pos_toverb, pos_verb, pos_adj;

	bool tough_trigger = false;
	composition_tree ctree = comp.tree();
	composition_tree::height_iterator ctree_iter(ctree, 1);
	int pos_aux;
	for (pos_aux = 0; ctree_iter != ctree.end(); ++ctree_iter, ++pos_aux) {
		if (check.unify(*ctree_iter, &mgu)) {

			subj / mgu;
			obj / mgu;
			toverb / mgu;
			adj / mgu;

			pos_adj = comp.find_head(adj);
			pos_subj = comp.find_head(subj);
			pos_obj = comp.find_head(obj);
			pos_toverb = comp.find_head(toverb);
			pos_verb_aux = pos_aux;

			if (is_adjective(tags.at(pos_adj)) && is_verb(tags.at(pos_verb_aux))) {
				tough_trigger = true;
				if (debug) {
					cout << adj << " " << pos_adj << endl;
					cout << subj << " " << pos_subj << endl;
					cout << obj << " " << pos_obj << endl;
					cout << toverb << " " << pos_toverb << endl;
					cout << "verb_aux: " << pos_verb_aux << endl;
				}
				break;
			}
		}
	}
	if (!tough_trigger)
		return pre_drt;

	for (int n = 0; n < connections.size(); ++n) { // find where is the subject verb
		pos1 = connections.at(n).first.first;
		pos2 = connections.at(n).first.second;
		tag1 = tags.at(pos1);
		tag2 = tags.at(pos2);
		if (pos1 == pos_toverb && is_verb(tag2)) {
			pos_verb = pos2;
		}
		if (pos2 == pos_toverb && is_verb(tag1)) {
			pos_verb = pos1;
		}

	}
	if (pos_subj != -1)
		pre_drt = add_subject(pre_drt, pos_subj, pos_verb);
	pre_drt = add_object(pre_drt, pos_obj, pos_verb);
	pre_drt = add_subject(pre_drt, pos_verb, pos_verb_aux);
	pre_drt = add_object(pre_drt, pos_adj, pos_verb_aux);

	// Prepare "to" to delete
	add_header(pre_drt.at(pos_toverb), ":DELETE");
	// Set the processed predicates as unmodifiable
	mask->at(pos_obj) = false;
	mask->at(pos_adj) = false;
	mask->at(pos_verb) = false;
	mask->at(pos_toverb) = false;
	mask->at(pos_verb_aux) = false;

	return pre_drt;
}

static bool check_drt_sanity(const DrtVect &drtvect)
{
	for (int n = 0; n < drtvect.size(); ++n) {
		if (drtvect.at(n).is_verb()) {
			string subj_ref = extract_subject(drtvect.at(n));
			string obj_ref = extract_object(drtvect.at(n));
			string header = extract_header(drtvect.at(n));
			if (subj_ref == obj_ref && !is_AUX(header))
				return false;
		}
	}
	return true;
}

static vector<DrtPred> internal_anaphoras_backward(vector<DrtPred> &pre_drt, const vector<string> &tags, vector<string> names)
{
	vector<DrtPred> to_return(pre_drt);
	int size = to_return.size();

	//vector<pair<string,string> > name_refs;
	vector<DrtPred> name_refs;

	metric *d = metric_singleton::get_metric_instance();
	vector<pair<string, string> > ref_pair;
	vector<int> elements_to_delete;

	for (int n = 0; n < size; ++n) {
		if (!to_return.at(n).is_pivot())
			continue;
		if (to_return.at(n).is_name() && !to_return.at(n).is_PRP() && !to_return.at(n).is_date()
		    && !is_specification_end(to_return, n)
		) {
			string single_head_ref;
			string single_ref;
			single_ref = extract_first_tag(to_return.at(n));
			DrtPred tmp_pred(to_return.at(n));
			if (to_return.at(n).is_proper_name() && d->gender_proper_name(extract_header(to_return.at(n))) != "") {
				implant_header(tmp_pred, "person");
			} else {
				implant_header(tmp_pred, extract_header(to_return.at(n)));
			}
			name_refs.push_back(tmp_pred);
		}
		if (to_return.at(n).is_name() && to_return.at(n).is_proper_name() && d->gender_proper_name(extract_header(to_return.at(n))) != ""
				&& !is_specification_of_NNP(to_return, n)
		) { // people's names can be a specification
			string single_head_ref;
			string single_ref;
			single_ref = extract_first_tag(to_return.at(n));
			DrtPred tmp_pred(to_return.at(n));
			implant_header(tmp_pred, "person");
			name_refs.push_back(tmp_pred);
		}
		if (to_return.at(n).is_PRP()) {
			string PRP_head_str = extract_header(to_return.at(n));
			bool plural_trigger = false;
			if (PRP_head_str == "they" || PRP_head_str == "them")
				plural_trigger = true;
			vector<string> PRP_strs = get_reference_string(PRP_head_str);
			int chosen_ref = -1;
			double chosen_ref_weight = 0;

			for (int m = 0; m < name_refs.size(); ++m) {
				if (plural_trigger && name_refs.at(m).is_plural()) {
					chosen_ref = m;
					chosen_ref_weight = 1;
					continue;
				}
				if (plural_trigger && !name_refs.at(m).is_plural()) {
					continue;
				}
				if (!plural_trigger && name_refs.at(m).is_plural()) {
					continue;
				}
				string head_str = extract_header(name_refs.at(m));
				if (debug) {
					cout << "PRP_STRS::: " << head_str << ", ";
					print_vector(PRP_strs);
				}
				double tmp_weight = get_string_vector_distance(d, head_str, PRP_strs);
				if (debug) {
					cout << "PRP_STRS2::: " << tmp_weight << endl;
				}
				if (tmp_weight > 0.8 && tmp_weight > chosen_ref_weight) {
					chosen_ref = m;
					chosen_ref_weight = tmp_weight;
				}
			}
			if (chosen_ref != -1) {
				string PRP_ref = extract_first_tag(to_return.at(n));
				string subst_ref = extract_first_tag(name_refs.at(chosen_ref));

				int m= find_verb_with_object(to_return,subst_ref);

				if(m != -1 && find_complement_with_first_and_second_tag(to_return,extract_first_tag(to_return.at(m)),PRP_ref) != -1  )
					continue; // You cannot do anaphora with an object from the end of the complement -> likeliness == -1

				m= find_verb_with_subject(to_return,subst_ref);
				if(m != -1 && find_complement_with_first_and_second_tag(to_return,extract_first_tag(to_return.at(m)),PRP_ref) != -1  )
					continue; // You cannot do anaphora with an object from the end of the complement -> likeliness == -1

				if(debug) {
					cout << "ANAPHORA_BACKWARD:: " << subst_ref << " " << PRP_ref << endl;
				}

				elements_to_delete.push_back(n);
				ref_pair.push_back(make_pair(PRP_ref, subst_ref));
			}
		}
	}

	for (int n = 0; n < ref_pair.size(); ++n) {
		string PRP_ref = ref_pair.at(n).first;
		string name_ref = ref_pair.at(n).second;
		DrtVect orig_drt = to_return;
		to_return = substitute_ref(to_return, PRP_ref, name_ref);
		bool is_valid_substitution = check_drt_sanity(to_return);
		if (!is_valid_substitution) {
			to_return = orig_drt;
		} else {
			int pos = elements_to_delete.at(n);
			if (pos < to_return.size()) {
				add_header(to_return.at(pos), ":DELETE");
			}
		}
	}

	return to_return;
}

static vector<DrtPred> internal_anaphoras_forward(vector<DrtPred> &pre_drt, const vector<string> &tags, vector<string> names)
{
	vector<DrtPred> to_return(pre_drt);
	int size = to_return.size();

	vector<pair<string, string> > name_refs;

	metric *d = metric_singleton::get_metric_instance();
	vector<pair<string, string> > ref_pair;

	for (int n = 0; n < size; ++n) {
		if (to_return.at(n).is_name() && !to_return.at(n).is_PRP()) {
			string single_head_ref;
			string single_ref;
			single_ref = extract_first_tag(to_return.at(n));
			if (to_return.at(n).is_proper_name()) {
				single_head_ref = "person";
			} else
				single_head_ref = extract_header(to_return.at(n));
			name_refs.push_back(make_pair(single_head_ref, single_ref));
		}
		if (to_return.at(n).is_PRP()) {
			string PRP_head_str = extract_header(to_return.at(n));
			vector<string> PRP_strs = get_reference_string(PRP_head_str);
			int chosen_ref = -1;
			double chosen_ref_weight = 0;
			for (int m = 0; m < name_refs.size(); ++m) {
				string head_str = name_refs.at(m).first;
				double tmp_weight = get_string_vector_distance(d, head_str, PRP_strs);
				if (tmp_weight > 0.8 && tmp_weight > chosen_ref_weight) {
					chosen_ref = m;
					chosen_ref_weight = tmp_weight;
				}
			}
			if (chosen_ref != -1) {
				string PRP_ref = extract_first_tag(to_return.at(n));
				string subst_ref = name_refs.at(chosen_ref).second;
				add_header(to_return.at(n), ":DELETE"); // set the PRP for deletion (it is not necessary now)
				ref_pair.push_back(make_pair(PRP_ref, subst_ref));
			}
		}
	}

	for (int n = 0; n < ref_pair.size(); ++n) {
		string PRP_ref = ref_pair.at(n).first;
		string name_ref = ref_pair.at(n).second;
		to_return = substitute_ref(to_return, PRP_ref, name_ref);
	}

	return to_return;
}

static vector<DrtPred> process_allocutions(const vector<DrtPred> &pre_drt, const vector<string> &tags,
		const vector<string> &names)
{
	vector<DrtPred> to_return(pre_drt);
	vector<string> communication_verbs = get_communication_verbs();
	vector<string> already_parsed;
	for (int n = 0; n < to_return.size(); ++n) {
		if (to_return.at(n).is_verb()) {
			string head = extract_header(to_return.at(n));
			head = head.substr(0, head.find(':'));
			string vref = extract_first_tag(to_return.at(n));
			string sref = extract_object(to_return.at(n));
			string oref = extract_object(to_return.at(n));
			if (shortfind(communication_verbs, head)) {
				vector<int> objs = find_all_element_with_string(to_return, oref);
				for (int m = 0; m < objs.size(); ++m) {
					int pos= objs.at(m);
					string fref= extract_first_tag(to_return.at(pos));
					if(shortfind(already_parsed, fref) )
						continue;
					if (is_verbatim(to_return.at(objs.at(m)))) {
						DrtPred tmp_pred(string("@PARENT-ALLOCUTION(") + oref + "," + vref + ")");
						to_return.push_back(tmp_pred);
						implant_object(to_return.at(n), "none");
						already_parsed.push_back(fref);
					} else {
						DrtPred tmp_pred(string("@ALLOCUTION(") + vref + "," + oref + ")");
						to_return.push_back(tmp_pred);
						implant_object(to_return.at(n), "none");
						already_parsed.push_back(fref);
					}
				}
			}
		}
	}

	return to_return;
}

static vector<DrtPred> process_lonely_verbatim(const vector<DrtPred> &pre_drt, const vector<string> &tags,
		const vector<string> &names)
{
	vector<DrtPred> to_return(pre_drt);

	vector<string> communication_verbs = get_communication_verbs();

	for (int n = 0; n < to_return.size(); ++n) {
		if (is_verbatim(to_return.at(n))) {
			string fref = extract_first_tag(to_return.at(n));
			vector<int> compls = find_all_element_with_string(to_return, fref);
			bool alloc_trigger = false;
			for (int j = 0; j < compls.size(); ++j) {
				string head = extract_header(to_return.at(compls.at(j)));
				if (head == "@PARENT-ALLOCUTION")
					alloc_trigger = true;
			}
			if (!alloc_trigger) { // the verbatim is alone
				DrtPred tmppred(string("@PARENT-ALLOCUTION(") + fref + ",none)");
				to_return.push_back(tmppred);
			}
		}
	}
	return to_return;
}

static bool has_verb_preposition(const vector<DrtPred> &pre_drt, const string &verb_ref, const string &obj_ref)

{
	vector<string> candidate_sub;
	candidate_sub.push_back("@SUBORD");
	candidate_sub.push_back("@SUB-OBJ");
	candidate_sub.push_back("@PARENT-ALLOCUTION");

	for (int n = 0; n < pre_drt.size(); ++n) {
		string head = extract_header(pre_drt.at(n));
		if (find(candidate_sub.begin(), candidate_sub.end(), head) != candidate_sub.end()) {
			string fref = extract_first_tag(pre_drt.at(n));
			string sref = extract_second_tag(pre_drt.at(n));
			if (fref == verb_ref && sref == obj_ref)
				return true;
		}
	}

	return false;
}

static bool same_level_verbs(const DrtVect &pre_drt, int n2, int m2)
// check that the verbs are in the same level of the phrase (a verb in
// a parenthesis is not connected to a verb that appears on the right
// of the parenthesis.
{
	int n, m;
	if (n2 < m2) {
		n = n2;
		m = m2;
	} else {
		m = n2;
		n = m2;
	}

	if (m < 0 || n < 0)
		return false;
	for (; n < m; ++n) {
		string head = extract_header(pre_drt.at(n));
		if (head == "-RBR-")
			// " Anna (born in 1981) was a tennis player." -> "was" is not connected to "born"
			return false;
	}
	return true;
}

static bool list_has_only_adjectives(const DrtVect &drtvect, const vector<int> &poz)
{
	for (int n = 0; n < poz.size(); ++n) {
		int pos = poz.at(n);
		if (pos < 0 || pos >= drtvect.size())
			continue;
		if (!drtvect.at(pos).is_adjective())
			return false;
	}
	return true;
}

static bool list_has_only_NNPs(const DrtVect &drtvect, const vector<int> &poz)
{
	for (int n = 0; n < poz.size(); ++n) {
		int pos = poz.at(n);
		if (pos < 0 || pos >= drtvect.size())
			continue;
		if (drtvect.at(pos).tag() != "NNP")
			return false;
	}
	return true;
}

static bool list_has_only_PRPs(const DrtVect &drtvect, const vector<int> &poz)
{
	for (int n = 0; n < poz.size(); ++n) {
		int pos = poz.at(n);
		if (pos < 0 || pos >= drtvect.size())
			continue;
		if (drtvect.at(pos).tag() != "PRP")
			return false;
	}
	return true;
}

static bool list_has_only_verbatims(const DrtVect &drtvect, const vector<int> &poz)
{
	for (int n = 0; n < poz.size(); ++n) {
		int pos = poz.at(n);
		if (pos < 0 || pos >= drtvect.size())
			continue;
		if ( !is_verbatim(drtvect.at(pos)) )
			return false;
	}
	return true;
}




static vector<DrtPred> process_switched_END(const vector<DrtPred> &pre_drt, const vector<string> &tags,
		const vector<string> &names)
// switch an adjective tagged as an object with an @DATIVE
{
	vector<DrtPred> to_return(pre_drt);

	metric *d = metric_singleton::get_metric_instance();

	for (int n = 0; n < to_return.size(); ++n) {
		string header = extract_header(to_return.at(n));
		if (to_return.at(n).is_complement() && header == "@DATIVE") {
			string fref = extract_first_tag(to_return.at(n));
			string sref = extract_second_tag(to_return.at(n));
			int m = find_verb_with_string(to_return, fref);
			if (m == -1)
				continue;
			string header = extract_header(to_return.at(m));
			header = header.substr(0, header.find(":"));
			string levin = d->get_levin_verb(header);
			if (header == "get" || header == "be" || (levin == "verb.stative" && header != "find") )
				continue;
			string sobj = extract_object(to_return.at(m));

			// find out if the object is an adjective
			vector<int> obj_poz = find_all_names_with_string_no_delete(to_return, sobj);
			vector<int> end_poz = find_all_names_with_string_no_delete(to_return, sref);

			if (obj_poz.size() == 0 || end_poz.size() == 0)
				continue;

			bool obj_is_adjective = list_has_only_adjectives(to_return, obj_poz);
			bool end_is_adjective = list_has_only_adjectives(to_return, end_poz);
			bool obj_is_NNP = list_has_only_NNPs(to_return, obj_poz);
			bool end_is_NNP = list_has_only_NNPs(to_return, end_poz);
			bool obj_is_PRP = list_has_only_PRPs(to_return, obj_poz);
			bool end_is_PRP = list_has_only_PRPs(to_return, end_poz);

			if ((obj_is_adjective && !end_is_adjective) || (obj_is_NNP && !end_is_NNP && !end_is_adjective) || (!obj_is_PRP && end_is_PRP)) {
				// switch @DATIVE and the object
				implant_second(to_return.at(n), sobj);
				implant_object(to_return.at(m), sref);
			}
		}
	}

	return to_return;
}

static vector<DrtPred> process_switched_allocution(const vector<DrtPred> &pre_drt, const vector<string> &tags,
		const vector<string> &names)
// [verbatim]_hello(name1) say(verb2,name1,none) @ALLOCUTION(verb2,name3) david(name3)
// -> [verbatim]_hello(name1) say(verb2,name2,none) @ALLOCUTION(verb2,name1) david(name3)
{
	vector<DrtPred> to_return(pre_drt);

	metric *d = metric_singleton::get_metric_instance();

	for (int n = 0; n < to_return.size(); ++n) {
		string header = extract_header(to_return.at(n));
		if (to_return.at(n).is_complement() && header == "@ALLOCUTION") {
			string fref = extract_first_tag(to_return.at(n));
			string sref = extract_second_tag(to_return.at(n));
			int m = find_verb_with_string(to_return, fref);
			if (m == -1)
				continue;
			string header = extract_header(to_return.at(m));
			header = header.substr(0, header.find(":"));
			string subj = extract_subject(to_return.at(m));

			// find out if the subject is a verbatim
			vector<int> subj_poz  = find_all_names_with_string_no_delete(to_return, subj);
			vector<int> alloc_poz = find_all_names_with_string_no_delete(to_return, sref);

			if(debug) {
				cout << "@ALLOCUTION:::" << subj << endl;
			}

			if (subj_poz.size() == 0 || alloc_poz.size() == 0)
				continue;

			bool subj_is_verbatim  = list_has_only_verbatims(to_return, subj_poz);
			bool alloc_is_verbatim = list_has_only_verbatims(to_return, alloc_poz);
			if(debug) {
				cout << "@ALLOCUTION2:::" << subj_is_verbatim << " " << alloc_is_verbatim << endl;
			}

			if ((subj_is_verbatim && !alloc_is_verbatim)) {
				// switch @ALLOC and the subject
				implant_second(to_return.at(n), subj);
				implant_subject(to_return.at(m), sref);
			}
		}
	}

	return to_return;
}


static vector<DrtPred> process_subordinate_verbs(const vector<DrtPred> &pre_drt, const vector<string> &tags,
		const vector<string> &names)
{
	vector<DrtPred> to_return(pre_drt);

	for (int n = 0; n < tags.size(); ++n) {
		if (is_verb(tags.at(n)) && !is_AUX(pre_drt.at(n))) {
			string verb_ref = extract_first_tag(to_return.at(n));
			string obj_ref = extract_object(to_return.at(n));
			string subj_ref = extract_subject(to_return.at(n));
			if (ref_is_verb(obj_ref)) {
				int m1 = find_verb_with_string(to_return, tags, verb_ref);
				if (to_return.at(n).tag() == "VBN" && to_return.at(m1).name().find("PASSIVE") == string::npos) { // this is a passive verb not yet
																							  // processed: the candidate subject of the
																							  // verb is another verb
					int m = find_verb_with_string(to_return, tags, obj_ref);
					if (m != -1) {
						//string subj= extract_subject(to_return.at(m));
						string obj = extract_object(to_return.at(m));
						if (!ref_is_verb(obj)) {
							if (obj != subj_ref)
								implant_object(to_return.at(n), obj);
						} else {
							implant_object(to_return.at(n), "none");
						}
					}
				} else if (!has_verb_preposition(to_return, verb_ref, obj_ref) && verb_ref != obj_ref) {
					int m2 = find_verb_with_string(to_return, tags, obj_ref);
					if(m2 != -1) {
						vector<int> poz = find_all_element_with_string(to_return,extract_subject(to_return.at(m2)));
						// this is the expedition that/WDT reached everest -> reached is not a subordinate of be
						bool is_WDT = false;
						for(int n3=0; n3 < poz.size(); ++n3) {
							int m3 = poz.at(n3);
							if(to_return.at(m3).is_WDT()) {
								is_WDT = true;
								break;
							}
						}
						if(!is_WDT) {
							if (same_level_verbs(to_return, m1, m2)) {
								DrtPred sub_pred(string("@SUBORD(") + verb_ref + "," + obj_ref + ")");
								sub_pred.setName("[dummy]");
								to_return.push_back(sub_pred);
							}
							obj_ref = "none";
							implant_object(to_return.at(n), obj_ref);
						}
					}
				}
			}
			if (ref_is_verb(subj_ref)) {
				if (verb_ref != subj_ref) {
					int m = find_verb_with_string(to_return, tags, subj_ref);
					if (m != -1 && !is_passive(to_return.at(n)) ) {
						string subj = extract_subject(to_return.at(m));
						if (obj_ref != subj)
							implant_subject(to_return.at(n), subj);
					} else {
						implant_subject(to_return.at(n), "none");
					}
				}
			}
		}
	}

	return to_return;
}

static bool has_THAT(const DrtVect &pre_drt, int pos)
{
	string ref1 = extract_first_tag(pre_drt.at(pos));
	for (int n = pos; n < pre_drt.size(); ++n) {
		string ref2 = extract_first_tag(pre_drt.at(n));
		string head = extract_header(pre_drt.at(n));
		string tag = pre_drt.at(n).tag();
		if (ref1 == ref2 && (tag == "IN" || tag == "WDT") && head == "that") {
			return true;
		}
	}
	return false;
}

static DrtVect process_additional_auxiliary_verb(DrtVect to_return, const vector<string> &tags, const vector<string> &names)
{
	// process "get"
	for (int n = 0; n < to_return.size(); ++n) {
		string head = extract_header(to_return.at(n));
		if (to_return.at(n).is_complement() && head == "@SUBORD") {
			string fref = extract_first_tag(to_return.at(n));
			string sref = extract_second_tag(to_return.at(n));
			int pos1 = find_verb_with_string(to_return, fref);
			int pos2 = find_verb_with_string(to_return, sref);
			if (pos1 == -1 || pos2 == -1)
				continue;
			string header1 = extract_header(to_return.at(pos1));
			string header2 = extract_header(to_return.at(pos2));
			header1 = header1.substr(0, header1.find(':'));
			if (header1 == "get") {
				if (!has_object(to_return.at(pos2))) {
					string sref1 = extract_subject(to_return.at(pos1));
					implant_object(to_return.at(pos2), sref1);
					string sref2 = extract_subject(to_return.at(pos2));
					if(sref2 == sref1) {
						implant_subject(to_return.at(pos2),"none");
					}
				}
				implant_header(to_return.at(pos1), header1 + ":DELETE");
				DrtPred auxiliary((string) "@AUXILIARY(" + sref + ",get)");
				to_return.push_back(auxiliary);
				implant_header(to_return.at(n), header1 + ":DELETE");
				to_return = substitute_ref(to_return, fref, sref);
			}
			if (header1 == "keep") {
				if (!has_subject(to_return.at(pos2))) {
					string sref1 = extract_subject(to_return.at(pos1));
					implant_subject(to_return.at(pos2), sref1);
					string oref2 = extract_object(to_return.at(pos2));
					if(sref1 == oref2) {
						implant_object(to_return.at(pos2),"none");
					}
				}
				implant_header(to_return.at(pos1), header1 + ":DELETE");
				DrtPred auxiliary((string) "@AUXILIARY(" + sref + ",keep)");
				to_return.push_back(auxiliary);
				implant_header(to_return.at(n), header1 + ":DELETE");
				to_return = substitute_ref(to_return, fref, sref);
			}
		}
	}
	return to_return;
}

static DrtVect process_implicit_motion_to(DrtVect to_return, const vector<string> &tags, const vector<string> &names)
{
	metric *d = metric_singleton::get_metric_instance();

	vector<string> verb_candidates; // additional verb candidates that have implicit @MOTION_TO
	verb_candidates.push_back("continue");
	verb_candidates.push_back("expand");
	verb_candidates.push_back("go");

	vector<string> name_candidates; // names or RB that triggers an implicit @MOTION_TO
	name_candidates.push_back("north");
	name_candidates.push_back("south");
	name_candidates.push_back("east");
	name_candidates.push_back("west");
	name_candidates.push_back("up");
	name_candidates.push_back("down");
	name_candidates.push_back("right");
	name_candidates.push_back("left");
	name_candidates.push_back("home");
	name_candidates.push_back("somewhere");

	/// remember +"ward" and +"wise"

	// Process adverbs
	for (int n = 0; n < to_return.size(); ++n) {
		if (to_return.at(n).is_adverb()) {
			string head = extract_header(to_return.at(n));
			for (int j = 0; j < name_candidates.size(); ++j) {
				string name = name_candidates.at(j);
				string name_wise = name + "wise";
				string name_ward = name + "ward";
				if (head == name || head == name_wise || head == name_ward) {
					string RB_ref = extract_first_tag(to_return.at(n));
					int m = find_verb_with_string(to_return, RB_ref);
					if (m == -1)
						continue;
					string verb_head = extract_header(to_return.at(m));
					verb_head = verb_head.substr(0, verb_head.find(":"));
					string verb_levin = d->get_levin_verb(verb_head);
					if (shortfind(verb_candidates, verb_head) || verb_levin == "verb.motion") {
						string new_ref = string("dummyname") + boost::lexical_cast<string>(n);
						DrtPred motion_to(string("@MOTION_TO(") + RB_ref + "," + new_ref + ")");
						DrtPred name_pred(name + "/NN(" + new_ref + ")");
						to_return.push_back(motion_to);
						to_return.push_back(name_pred);
						add_header(to_return.at(n), ":DELETE");
						to_return.at(n).name() += ":DELETE";
						break;
					}
				}
			}
		}
	}

	// Process verbs that have a "candidate_name" as objects
	for (int n = 0; n < to_return.size(); ++n) {
		if (to_return.at(n).is_verb()) {
			string verb_head = extract_header(to_return.at(n));
			string verb_levin = d->get_levin_verb(verb_head);
			if (shortfind(verb_candidates, verb_head)
				|| (verb_levin == "verb.motion"
						&& !is_motion_verb_without_automatic_motion_to(verb_head)
				)
			) {
				string obj_ref = extract_object(to_return.at(n));
				vector<int> obj_refs = find_all_element_with_string(to_return, obj_ref);
				for (int m = 0; m < obj_refs.size(); ++m) {
					string obj_head = extract_header(to_return.at(obj_refs.at(m)));
					string verb_ref = extract_first_tag(to_return.at(n));
					DrtPred motion_to(string("@MOTION_TO(") + verb_ref + "," + obj_ref + ")");
					to_return.push_back(motion_to);
					implant_object(to_return.at(n), "none");
				}
			}
		}
	}

	return to_return;
}

static int find_first_CC(const DrtVect &drtvect, const string &ref)
{
	for (int n = 0; n < drtvect.size(); ++n) {
		string header = extract_header(drtvect.at(n));
		if (header == "@AND" || header == "@OR") {
			return n;
		}
	}

	return -1;
}

static string find_first_connected_with_CC(const DrtVect &drtvect, const string &ref)
{
	string to_return;

	for (int n = 0; n < drtvect.size(); ++n) {
		string header = extract_header(drtvect.at(n));
		if (header == "@AND" || header == "@OR") {
			string fref = extract_first_tag(drtvect.at(n));
			string sref = extract_second_tag(drtvect.at(n));
			if (fref == ref) {
				to_return = sref;
				break;
			}
		}
	}

	return to_return;
}

static DrtVect process_time_between(DrtVect to_return, const vector<string> &tags, const vector<string> &names)
{
	metric *d = metric_singleton::get_metric_instance();
	// Process adverbs
	for (int n = 0; n < to_return.size(); ++n) {
		string header = extract_header(to_return.at(n));
		if (to_return.at(n).is_complement() && header == "@TIME_BETWEEN") {
			string fref = extract_first_tag(to_return.at(n));
			string first_pos_str = extract_second_tag(to_return.at(n));
			string second_pos_str = find_first_connected_with_CC(to_return, first_pos_str);
			int CC_pos = find_first_CC(to_return, first_pos_str);
			if (CC_pos == -1 || second_pos_str == "")
				continue;
			add_header(to_return.at(CC_pos), ":DELETE");
			add_header(to_return.at(n), ":DELETE");
			DrtPred after_pred("@AFTER(" + fref + "," + first_pos_str + ")");
			DrtPred before_pred("@BEFORE(" + fref + "," + second_pos_str + ")");
			to_return.push_back(after_pred);
			to_return.push_back(before_pred);
		}
	}

	return to_return;
}

static DrtVect process_time_duration(DrtVect to_return, const vector<string> &tags, const vector<string> &names, bool is_question)
{
	metric *d = metric_singleton::get_metric_instance();

	vector<string> duration_nouns;
	duration_nouns.push_back("second");
	duration_nouns.push_back("minute");
	duration_nouns.push_back("hour");
	duration_nouns.push_back("day");
	duration_nouns.push_back("month");
	duration_nouns.push_back("year");
	duration_nouns.push_back("century");
	duration_nouns.push_back("millennia");

	vector<string> duration_verbs;
	duration_verbs.push_back("live");
	duration_verbs.push_back("last");
	duration_verbs.push_back("take");

	for (int n = 0; n < to_return.size(); ++n) {
		string header = extract_header(to_return.at(n));
		if (to_return.at(n).is_complement() && header == "@FOR") {
			string fref = extract_first_tag(to_return.at(n));
			int m = find_verb_with_string(to_return,fref);
			if ( m != -1 ) {
				string vheader = extract_header(to_return.at(m));
				if (shortfind(duration_verbs,vheader) ) {
					implant_header(to_return.at(n),"@TIME_DURATION");
					continue;
				}
			}
			string sref = extract_second_tag(to_return.at(n));
			m = find_name_with_string(to_return,sref);
			if ( m != -1 ) {
				string nheader = extract_header(to_return.at(m));
				if(debug) {
					cout << "DURATIONS::: " << nheader << " " << m << endl;
				}
				if ( shortfind(duration_nouns,nheader) ) {
					implant_header(to_return.at(n),"@TIME_DURATION");
					to_return.at(n) = percolate_to_verb(to_return,tags,n);
					continue;
				}
			}
		}
	}


	for (int n = 0; n < to_return.size(); ++n) {
		string header = extract_header(to_return.at(n));
		if ( shortfind(duration_verbs,header) ) {
			if( !has_object(to_return.at(n)) )
				continue;
			string vref = extract_first_tag(to_return.at(n));
			string oref = extract_object(to_return.at(n));
			int m = find_name_with_string(to_return,oref);
			string nheader = "";
			if ( m != -1 ) {
				nheader = extract_header(to_return.at(m));
			}
			if ( !shortfind(duration_nouns,nheader) )
				continue;
			implant_object(to_return.at(n),"none");
			DrtPred duration_pred("@TIME_DURATION(" +vref+ "," +oref+ ")");
			to_return.push_back(duration_pred);
		}
	}

	return to_return;
}





static bool is_subordinate_of_verb(const vector<DrtPred> &pre_drt, const string &subord_ref)
{
	vector<DrtPred>::const_iterator piter = pre_drt.begin();
	vector<DrtPred>::const_iterator pend = pre_drt.end();
	for (; piter != pend; ++piter) {
		string header = extract_header(*piter);
		string fref = extract_first_tag(*piter);
		string sref = extract_second_tag(*piter);
		if (piter->is_complement()
				&& header != "CONJUNCTION"
				&& header != "DISJUNCTION"
				&& header != "COORDINATION"
				&& fref != sref && ref_is_verb(fref) && ref_is_verb(sref)) {
			string second_tag = extract_second_tag(*piter);
			if (second_tag == subord_ref) {
				return true;
			}
		}
	}

	return false;
}

static DrtVect process_conditional_subordinate(DrtVect to_return, const vector<string> &tags, const vector<string> &names)
// @CONDITION -> @CAUSED_BY for subordinates
{
	for (int n = 0; n < to_return.size(); ++n) {
		string header = extract_header(to_return.at(n));
		if (to_return.at(n).is_complement() && header == "@CONDITION") {
			string fref = extract_first_tag(to_return.at(n));
			if(is_subordinate_of_verb(to_return,fref) ) {
				implant_header(to_return.at(n),"@CAUSED_BY");
			}
		}
	}

	return to_return;
}

static DrtVect process_adverbial_place_at(DrtVect to_return, const vector<string> &tags, const vector<string> &names)
{
	metric *d = metric_singleton::get_metric_instance();

	vector<string> name_candidates; // names or RB that trigger an implicit @MOTION_TO
	name_candidates.push_back("there");
	name_candidates.push_back("close");
	name_candidates.push_back("somewhere");
	name_candidates.push_back("anywhere");
	name_candidates.push_back("everywhere");
	name_candidates.push_back("nowhere");

	// Process adverbs
	for (int n = 0; n < to_return.size(); ++n) {
		if (to_return.at(n).is_adverb()) {
			string head = extract_header(to_return.at(n));
			for (int j = 0; j < name_candidates.size(); ++j) {
				string name = name_candidates.at(j);
				if (head == name) {
					if (head == "somewhere" || head == "anywhere" || head == "everywhere" || head == "nowhere")
						name = "place";
					string RB_ref = extract_first_tag(to_return.at(n));
					int m = find_verb_with_string(to_return, RB_ref);
					if (m == -1)
						continue;
					string new_ref = string("ref") + boost::lexical_cast<string>(n);
					DrtPred motion_to(string("@PLACE_AT(") + RB_ref + "," + new_ref + ")");
					name = (string) "[place]_" + name;
					DrtPred name_pred(name + "/NN(" + new_ref + ")");
					name_pred.set_pivot(true);
					to_return.push_back(motion_to);
					to_return.push_back(name_pred);
					add_header(to_return.at(n), ":DELETE");
					to_return.at(n).name() += ":DELETE";
					if (head == "nowhere") {
						DrtPred not_pred("not/RB(" + RB_ref + ")");
						to_return.push_back(not_pred);
						DrtPred q_pred("@QUANTIFIER(" + new_ref + ",all)");
						to_return.push_back(q_pred);
					}
					if (head == "everywhere") {
						DrtPred q_pred("@QUANTIFIER(" + new_ref + ",all)");
						to_return.push_back(q_pred);
					}

					break;
				}
			}
		}
	}

	return to_return;
}

static DrtVect process_adverbial_time_at(DrtVect to_return, const vector<string> &tags, const vector<string> &names)
{
	metric *d = metric_singleton::get_metric_instance();

	vector<string> name_candidates; // names or RB that trigger an implicit @MOTION_TO
	name_candidates.push_back("always");
	//name_candidates.push_back("a_lot");
	name_candidates.push_back("never");
	name_candidates.push_back("sometime");
	name_candidates.push_back("sometimes");
	name_candidates.push_back("no-more");

	tagger_info *info = parser_singleton::get_tagger_info_instance();
	vector<string> chrono = info->getChronoNames();
	name_candidates.insert(name_candidates.end(), chrono.begin(), chrono.end());

	// Process adverbs
	for (int n = 0; n < to_return.size(); ++n) {
		if (to_return.at(n).is_adverb()) {
			string head = extract_header(to_return.at(n));
			for (int j = 0; j < name_candidates.size(); ++j) {
				string name = name_candidates.at(j);
				if (head == name) {
					string RB_ref = extract_first_tag(to_return.at(n));
					int m = find_verb_with_string(to_return, RB_ref);
					if (m == -1)
						continue;

					string new_ref = string("dummyname") + boost::lexical_cast<string>(n);
					DrtPred motion_to(string("@TIME_AT(") + RB_ref + "," + new_ref + ")");
					DrtPred name_pred(name + "/NN(" + new_ref + ")");
					to_return.push_back(motion_to);
					to_return.push_back(name_pred);
					add_header(to_return.at(n), ":DELETE");
					to_return.at(n).name() += ":DELETE";
					break;
				}
			}
		}
	}

	return to_return;
}

DrtVect last_touch_references(DrtVect to_return, const vector<string> &tags, const vector<string> &names)
// the man that is on the sofa is sleeping. "the man" should not refer to any previous "man" in the discourse.
/// This is not quite correct.
{
	vector<pair<string, string> > ref_pair;
	for (int n = 0; n < to_return.size(); ++n) {
		if (to_return.at(n).is_name()) {
			string ref = extract_first_tag(to_return.at(n));
			if (ref_is_name(ref) && has_THAT(to_return, n)) {
				string name = string("name") + boost::lexical_cast<string>(n);
				ref_pair.push_back(make_pair(ref, name));
				//implant_first(to_return.at(n), name);
			}
		}
	}

	for (int n = 0; n < ref_pair.size(); ++n) {
		string PRP_ref = ref_pair.at(n).first;
		string name_ref = ref_pair.at(n).second;
		to_return = substitute_ref(to_return, PRP_ref, name_ref);
	}
	return to_return;
}

DrtVect last_touch_conjunctions(DrtVect pre_drt, const vector<string> &tags, const vector<string> &names)
// @CONJUNCTIONS(verb1,name1) -> V(verb,subj,name2) N(name2) @AND(name2, name1)
{
	vector<string> start_verbs;
	start_verbs.push_back("begin");
	start_verbs.push_back("start");

	for (int n = 0; n < pre_drt.size(); ++n) {
		if (extract_header(pre_drt.at(n)) == "CONJUNCTION") {
			string fref = extract_first_tag(pre_drt.at(n));
			string sref = extract_second_tag(pre_drt.at(n));
			if (ref_is_verb(fref) && ref_is_name(sref)) {
				implant_header(pre_drt.at(n), "@AND");
				pre_drt.at(n).name() = "@AND";
				int m = find_verb_with_string(pre_drt, fref);
				if (m != -1 && has_object(pre_drt.at(m))) {
					string obj = extract_object(pre_drt.at(m));
					implant_first(pre_drt.at(n), obj);
				}
			}
			if (ref_is_name(fref) && ref_is_verb(sref)) {
				implant_header(pre_drt.at(n), "@AND");
				pre_drt.at(n).name() = "@AND";
				int m = find_verb_with_string(pre_drt, sref);
				if (m != -1 && has_subject(pre_drt.at(m))) {
					string obj = extract_subject(pre_drt.at(m));
					implant_second(pre_drt.at(n), obj);
				}
			}
			if (ref_is_verb(fref) && ref_is_verb(sref)) {
				int m = find_verb_with_string(pre_drt, sref);
				int m2 = find_verb_with_string(pre_drt, fref);
				if (m == -1 || m2 == -1)
					continue;
				int m3= find_complement_with_first_and_second_tag(pre_drt,fref,sref);
				int m4= find_complement_with_first_and_second_tag(pre_drt,sref,fref);
				if(m3 != -1 || m4 != -1)
					continue; // the two verbs are already joined by a preposition
				if (pre_drt.at(m).tag() == "VBG") { // [while] sitting on the dock of the bay, I waste time
					implant_header(pre_drt.at(n), "@TIME_AT");
					pre_drt.at(n).name() = "while";
				} else if (pre_drt.at(m2).tag() == "VBG") { // [while] sitting on the dock of the bay, I waste time
					implant_header(pre_drt.at(n), "@TIME_AT");
					pre_drt.at(n).name() = "while";
					switch_children(pre_drt.at(n));
				}
			}
		}
	}
	return pre_drt;
}

DrtVect last_touch_commas(DrtVect pre_drt, const vector<string> &tags, const vector<string> &names)
// commas can become @AND if only names are involved
{
	for (int n = 0; n < pre_drt.size(); ++n) {
		if (pre_drt.at(n).is_comma()) {
			string fref = extract_first_tag(pre_drt.at(n));
			string sref = extract_second_tag(pre_drt.at(n));
			if (ref_is_name(fref) && ref_is_name(sref) && fref != sref) {
				implant_header(pre_drt.at(n), "@AND");
				pre_drt.at(n).name() = "@AND";
			}
		}
	}
	return pre_drt;
}

DrtVect last_touch_WRB(DrtVect pre_drt, const vector<string> &tags, const vector<string> &names)
{
	for (int n = 0; n < pre_drt.size(); ++n) {
		if (pre_drt.at(n).is_WRB()) {
			string head = extract_header(pre_drt.at(n));
			if (head == "where") {
				string fref = extract_first_tag(pre_drt.at(n));
				string sref = extract_second_tag(pre_drt.at(n));
				if (fref.find("verb") != string::npos && sref.find("verb") != string::npos) {
					implant_header(pre_drt.at(n), "@PLACE_AT");
					pre_drt.at(n).setName(head);
				}
			}
		}
	}
	return pre_drt;
}

DrtVect last_touch_END(DrtVect pre_drt, const vector<string> &tags, const vector<string> &names)
{
	metric *d = metric_singleton::get_metric_instance() ;
	for (int n = 0; n < pre_drt.size(); ++n) {
		string header= extract_header( pre_drt.at(n) );
		if (pre_drt.at(n).is_complement() && header == "@DATIVE") {
			string fref = extract_first_tag(pre_drt.at(n));
			string sref = extract_second_tag(pre_drt.at(n));
			int m= find_verb_with_string(pre_drt,fref);
			if (m != -1) {
				string levin = d->get_levin_verb(extract_header(pre_drt.at(m)) );
				if (levin == "verb.motion" )
					implant_header(pre_drt.at(n),"@MOTION_TO");
			}
		}
	}
	return pre_drt;
}

DrtVect last_touch_MOTION_TO(DrtVect pre_drt, const vector<string> &tags, const vector<string> &names)
{
	metric *d = metric_singleton::get_metric_instance();
	for (int n = 0; n < pre_drt.size(); ++n) {
		string header= extract_header( pre_drt.at(n) );
		if (pre_drt.at(n).is_complement() && header == "@MOTION_TO" ) {
			string fref = extract_first_tag(pre_drt.at(n));
			string sref = extract_second_tag(pre_drt.at(n));
			int m= find_verb_with_string(pre_drt,fref);
			if (m != -1) {
				string vheader = extract_header(pre_drt.at(m));
				string levin = d->get_levin_verb( vheader );
				if (levin != "verb.motion" && verb_supports_indirect_obj(vheader) )
					implant_header(pre_drt.at(n),"@MOTION_TO");
			}
		}
		if (pre_drt.at(n).is_complement() && header == "@PLACE_AT" ) {
			string fref = extract_first_tag(pre_drt.at(n));
			string sref = extract_second_tag(pre_drt.at(n));
			int m= find_verb_with_string(pre_drt,fref);
			int m2= find_element_with_string(pre_drt,sref);
			if (m != -1 && m2 != -1) {
				string there_header= extract_header(pre_drt.at(m2));
				if(there_header != "[place]_there")
					continue;
				string vheader = extract_header(pre_drt.at(m));
				string levin = d->get_levin_verb( vheader );
				if (levin == "verb.motion" )
					implant_header(pre_drt.at(n),"@MOTION_TO");
			}
		}
	}
	return pre_drt;
}


static int percolate_to_last_verb(DrtVect &to_return, const vector<string> &tags, int pos)
{
	int m = pos;

	if (m == -1)
		return m;

	string vref = extract_first_tag(to_return.at(m));
	int safe = 0;
	for (; safe < 100; ++safe) {
		int sub_prep_pos = find_conj_with_first_tag(to_return, tags, vref, "@SUBORD");
		if (sub_prep_pos != -1) {
			string sref = extract_second_tag(to_return.at(sub_prep_pos));
			vref = sref;
			int new_pos = find_verb_with_string(to_return, vref);
			if (new_pos == -1)
				break;
		} else
			break;
	}

	return m;
}

DrtVect last_touch_WP(DrtVect to_return, const vector<string> &tags, const vector<string> &names,
		vector<pair<pair<int, int>, constituents> > &connections, bool is_question)
// A WP or WDT that is not connected to a verb is now resolved as subject or object
{
	int pos1, pos2;
	string str1, str2, tag1, tag2;

	for (int n = 0; n < connections.size(); ++n) {
		pos1 = connections.at(n).first.first;
		pos2 = connections.at(n).first.second;
		str1 = to_return.at(pos1).name();
		str2 = to_return.at(pos2).name();
		tag1 = to_return.at(pos1).tag();
		tag2 = to_return.at(pos2).tag();

		if ((to_return.at(pos1).is_WP() || to_return.at(pos1).is_WDT()) && to_return.at(pos2).is_verb()
				&& !to_return.at(pos1).is_delete()) {
			string ref = extract_first_tag(to_return.at(pos1));
			string vref = extract_first_tag(to_return.at(pos2));
			int m = find_verb_with_string(to_return, tags, vref);
			if (m == -1)
				continue;
			m = percolate_to_last_verb(to_return, tags, m);
			if (m != -1) {
				string head = extract_header(to_return.at(m));
				string obj_ref = extract_object(to_return.at(m));
				string subj_ref = extract_subject(to_return.at(m));
				if (head.find("PASSIVE") == string::npos) { // the verb is in the active form
					if (!is_question) {
						if (!has_subject(to_return.at(m)) && obj_ref != ref)
							implant_subject(to_return.at(m), ref);
						else if (!has_object(to_return.at(m)) && subj_ref != ref)
							implant_object(to_return.at(m), ref);
					} else {
						if (!has_object(to_return.at(m)) && subj_ref != ref)
							implant_object(to_return.at(m), ref);
						else if (!has_subject(to_return.at(m)) && obj_ref != ref)
							implant_subject(to_return.at(m), ref);
					}
				} else {
					if (!has_object(to_return.at(m)))
						implant_object(to_return.at(m), ref);
					else if (obj_ref != ref && subj_ref != ref) { // if it already has a subject, add an indirect object
						DrtPred tmppred(string("@DATIVE(") + vref + "," + ref + ")");
						tmppred.setName("as");
						to_return.push_back(tmppred);
					}
				}
			}
		} else if ((to_return.at(pos2).is_WP() || to_return.at(pos2).is_WDT()) && to_return.at(pos1).is_verb()
				&& !to_return.at(pos2).is_delete()) {
			string ref = extract_first_tag(to_return.at(pos2));
			string vref = extract_first_tag(to_return.at(pos1));
			int m = find_verb_with_string(to_return, tags, vref);
			if (m != -1) {
				string head = extract_header(to_return.at(m));
				string obj_ref = extract_object(to_return.at(m));
				string subj_ref = extract_subject(to_return.at(m));
				if (head.find("PASSIVE") == string::npos) { // the verb is in the active form
					if (!has_subject(to_return.at(m)) && obj_ref != ref)
						implant_subject(to_return.at(m), ref);
					else if (!has_object(to_return.at(m)) && subj_ref != ref)
						implant_object(to_return.at(m), ref);
				} else {
					if (!has_object(to_return.at(m)))
						implant_object(to_return.at(m), ref);
					else if (obj_ref != ref && subj_ref != ref) { // if it already has a subject, add an indirect object
						DrtPred tmppred(string("@DATIVE(") + vref + "," + ref + ")");
						tmppred.setName("as");
						to_return.push_back(tmppred);
					}
				}
			}
		}
	}

	return to_return;
}

static int find_WRB(const DrtVect &pre_drt, string ref)
{
	for (int n = 0; n < pre_drt.size(); ++n) {
		if (!pre_drt.at(n).is_WRB())
			continue;
		string sref = extract_second_tag(pre_drt.at(n));
		if (sref == ref)
			return n;
	}
	return -1;
}

vector<DrtPred> duplicate_WRB(const vector<DrtPred> &pre_drt, const vector<string> &tags, const vector<string> &names,
		vector<pair<pair<int, int>, constituents> > &connections)
// "How to drive and to park" -> "how to drive and how to park"
{
	vector<DrtPred> to_return(pre_drt);

	vector<string> conjunctions;
	conjunctions.push_back("@DISJUNCTION");
	conjunctions.push_back("@CONJUNCTION");
	conjunctions.push_back("@COORDINATION");

	for (int n = 0; n < to_return.size(); ++n) {
		string head = extract_header(to_return.at(n));
		if (find(conjunctions.begin(), conjunctions.end(), head) != conjunctions.end()) {
			string fref = extract_first_tag(to_return.at(n));
			string sref = extract_second_tag(to_return.at(n));
			if (ref_is_verb(fref) && ref_is_verb(sref)) {
				int m1 = find_WRB(to_return, fref);
				int m2 = find_WRB(to_return, sref);
				if (m1 != -1 && m2 == -1) {
					DrtPred WRB_pred(to_return.at(m1));
					implant_second(WRB_pred, sref);
					to_return.push_back(WRB_pred);
				}
			}
		}
	}
	return to_return;
}

vector<DrtPred> commas_and_preposition(const vector<DrtPred> &pre_drt, const vector<string> &tags, const vector<string> &names)
// [*]/IN(name1,name2) -comma-(name1,verb) -> [*]/IN(verb,name2)
{
	vector<DrtPred> to_return(pre_drt);

	for (int n = 0; n < to_return.size(); ++n) {
		string head = extract_header(to_return.at(n));
		if (head.find("-comma-") != string::npos) {
			string fref = extract_first_tag(to_return.at(n));
			string sref = extract_second_tag(to_return.at(n));
			if (ref_is_verb(fref) || !ref_is_verb(sref))
				continue;
			int prep_pos = find_prep_with_first_tag(to_return, tags, fref);
			if (prep_pos == -1)
				continue;
			vector<int> noun_poz = find_all_names_with_string_no_delete(to_return, fref);
			if(noun_poz.size() != 0)
				continue;
			string head_pred = extract_header(to_return.at(prep_pos));
			if (head_pred != "-comma-") {
				add_header(to_return.at(n), ":DELETE");
				if (!ref_is_ref(fref) && !ref_is_verb(fref))
					implant_first(to_return.at(prep_pos), sref);
			}
		}
	}
	return to_return;
}

vector<DrtPred> commas_and_adverb(const vector<DrtPred> &pre_drt, const vector<string> &tags, const vector<string> &names)
// [*]/RB(name1) -comma-(name1,verb) -> [*]/RB(verb)
{
	vector<DrtPred> to_return(pre_drt);

	for (int n = 0; n < to_return.size(); ++n) {
		string head = extract_header(to_return.at(n));
		if (head.find("-comma-") != string::npos) {
			string fref = extract_first_tag(to_return.at(n));
			string sref = extract_second_tag(to_return.at(n));
			if (!ref_is_verb(fref) && ref_is_verb(sref)) {

				if (debug) {
					cout << "COMMAS_ADV::: " << fref << " " << sref << endl;
				}

				vector<int> poz = find_all_adverbs_with_string(to_return, fref);
				vector<int> poz_names = find_all_names_with_string_no_delete(to_return, fref);
				if (poz.size() == 0 || poz_names.size())
					continue;
				for (int m = 0; m < poz.size(); ++m) {
					int pos = poz.at(m);
					string head_pred = extract_header(to_return.at(pos));
					if (head_pred != "-comma-") {
						add_header(to_return.at(n), ":DELETE");
						implant_first(to_return.at(pos), sref);
					}
				}
			}
			if (!ref_is_verb(sref) && ref_is_verb(fref)) {
				vector<int> poz = find_all_adverbs_with_string(to_return, sref);
				if (poz.size() == 0)
					continue;
				for (int m = 0; m < poz.size(); ++m) {
					int pos = poz.at(m);
					string head_pred = extract_header(to_return.at(pos));
					if (head_pred != "-comma-") {
						add_header(to_return.at(n), ":DELETE");
						implant_first(to_return.at(pos), fref);
					}
				}
			}
		}
	}
	return to_return;
}

vector<DrtPred> delete_mirror_elements(const vector<DrtPred> &pre_drt, const vector<string> &tags, const vector<string> &names,
		vector<pair<pair<int, int>, constituents> > &connections)
{
	vector<DrtPred> to_return(pre_drt);

	vector<DrtPred> already_parsed;

	for (int n = 0; n < to_return.size(); ++n) {
		string header = extract_header(to_return.at(n));
		if (to_return.at(n).is_complement()) {
			if (shortfind(already_parsed, to_return.at(n))) {
				add_header(to_return.at(n), ":DELETE");
				to_return.at(n).name() += ":DELETE";
			} else {
				DrtPred tmp_pred = to_return.at(n);
				switch_children(tmp_pred);
				already_parsed.push_back(tmp_pred);
				if (header == "@PARENT-ALLOCUTION") {
					implant_header(tmp_pred, "@ALLOCUTION");
					already_parsed.push_back(tmp_pred);
				}
			}
		}
	}
	return to_return;
}

static DrtVect percolate_subject_to_subordinates(DrtVect to_return, const vector<string> &tags, const vector<string> &names)
{
	for (int n = 0; n < to_return.size(); ++n) {
		string fref = extract_first_tag(to_return.at(n));
		string sref = extract_second_tag(to_return.at(n));
		string header = extract_header(to_return.at(n));
		string name = to_return.at(n).name();
		if (to_return.at(n).is_complement() && header == "@SUBORD" && name == "to") {
			int m1 = find_verb_with_string(to_return, fref);
			int m2 = find_verb_with_string(to_return, sref);
			if (m1 == -1 || m2 == -1)
				continue;
			string header1 = extract_header(to_return.at(m1));
			string vref = extract_first_tag(to_return.at(m1));
			string o1 = extract_object(to_return.at(m1));
			string s2 = extract_subject(to_return.at(m2));
			string o2 = extract_object(to_return.at(m2));
			int m_obj1 = find_element_with_string(to_return,o1);

			bool has_dative= false;
			if(m_obj1 == -1) {
				if(debug) {
					cout << "DATIVE::: " << m_obj1 << endl;
				}
				int m_dative= find_complement_with_first_tag(to_return,vref,"@DATIVE");
				if(debug) {
					cout << "DATIVE2::: " << m_dative << " " << vref << endl;
				}
				if(m_dative == -1)
					continue;
				has_dative = true;
				o1 = extract_second_tag(to_return.at(m_dative));
				add_header(to_return.at(m_dative),":DELETE");
				m_obj1= find_element_with_string(to_return,o1);
				if(debug) {
					cout << "DATIVE3::: " << m_obj1 << endl;
				}
				if(m_obj1 == -1)
					continue;
			}
			if ( to_return.at(m_obj1).is_adjective() )
				continue;
			if (o1 != o2 && header1 != "be" && (has_object(to_return.at(m1)) || has_dative) && !has_subject(to_return.at(m2))) {
				implant_subject(to_return.at(m2), o1);
				//if(header1 != "make" && header1 != "get")
				implant_object(to_return.at(m1), "none");
				continue;
			}
			int m_for = find_complement_with_first_tag(to_return, fref, "@FOR");
			if (m_for == -1)
				continue;
			string for_ref = extract_second_tag(to_return.at(m_for));
			if (for_ref != o2 && !has_subject(to_return.at(m2))) {
				implant_subject(to_return.at(m2), for_ref);
				add_header(to_return.at(m_for), ":DELETE");
				continue;
			}
		}
	}
	return to_return;
}

vector<DrtPred> choose_competing_elements(const vector<DrtPred> &pre_drt, const vector<string> &tags, const vector<string> &names,
		vector<pair<pair<int, int>, constituents> > &connections)
{
	vector<DrtPred> to_return(pre_drt);

	vector<string> priority;
	priority.push_back("how");
	priority.push_back("HOWTO:DELETE");
	priority.push_back("@BEFORE");
	priority.push_back("@AFTER");
	priority.push_back("@MOTION_FROM|@TIME_FROM");
	priority.push_back("@MOTION_TO|@TIME_TO");
	priority.push_back("@PLACE_AT|@MOTION_FROM|@MOTION_TO|@MOTION_THROUGH");
	priority.push_back("@TIME_AT|@CLOCK_AT|@TIME_FROM|@TIME_TO|@TOPIC|@BEFORE|@AFTER");
	priority.push_back("@TOPIC|@COMPARED_TO");
	priority.push_back("@TIME_AT");
	priority.push_back("@TIME_FROM");
	priority.push_back("@TIME_TO");
	priority.push_back("@PLACE_AT");
	priority.push_back("@MOTION_TO");
	priority.push_back("@CAUSED_BY");
	priority.push_back("@GENITIVE");
	priority.push_back("@DATIVE");
	priority.push_back("@PARENT-ALLOCUTION");
	priority.push_back("@ALLOCUTION");
	priority.push_back("@PAR");
	priority.push_back("@SUBORD");
	priority.push_back("@DISJUNCTION");
	priority.push_back("@SUB-OBJ");
	priority.push_back("@CONJUNCTION");
	priority.push_back("@COORDINATION");
	priority.push_back("@AND");
	priority.push_back("@OR");

	vector<pair<string, string> > tag_pairs;
	map<pair<string, string>, int> map_pairs;

	for (int n = 0; n < to_return.size(); ++n) {
		string head0 = extract_header(to_return.at(n));
		if ((((to_return.at(n).is_complement() && head0 != "@TIME") || head0 == "how" /// temporary: assign a complement to these two
		|| head0 == "why") && head0.find(":DELETE") == string::npos) || head0 == "HOWTO:DELETE") {
			if(to_return.at(n).is_WRB() && to_return.at(n).is_question() ) //
				continue;
			string fref = extract_first_tag(to_return.at(n));
			string sref = extract_second_tag(to_return.at(n));
			if (!ref_is_name(sref) && !ref_is_verb(sref))
				sref = "dummy"; // complements that points to nothing are pointing to the same nothing
			pair<string, string> tagpair = make_pair(fref, sref);
			vector<pair<string, string> >::iterator prev_pair = find(tag_pairs.begin(), tag_pairs.end(), tagpair);
			map<pair<string, string>, int>::iterator map_iter = map_pairs.find(tagpair);

			if (prev_pair != tag_pairs.end() && map_iter != map_pairs.end()) {
				// If there already is a complement pointing to the same elements
				string head = extract_header(to_return.at(n));
				int pos_prior_int = map_pairs[tagpair];
				string head_prior = extract_header(to_return.at(pos_prior_int));

				vector<string>::iterator pos_now = find(priority.begin(), priority.end(), head);
				vector<string>::iterator pos_prior = find(priority.begin(), priority.end(), head_prior);

				if (std::distance(priority.begin(), pos_now) > std::distance(priority.begin(), pos_prior)) {
					// If the previous pair has a higher priority, then erase the current element
					add_header(to_return.at(n), ":DELETE");
					to_return.at(n).name() += ":DELETE";
				} else {
					map_pairs[tagpair] = n;
					add_header(to_return.at(pos_prior_int), ":DELETE");
					to_return.at(pos_prior_int).name() += ":DELETE";
				}
			} else {
				tag_pairs.push_back(tagpair);
				map_pairs[tagpair] = n;
			}
		}
	}
	return to_return;
}

static vector<DrtPred> meld_competing_elements(const vector<DrtPred> &pre_drt, const vector<string> &tags,
		const vector<string> &names, vector<pair<pair<int, int>, constituents> > &connections)
{
	vector<DrtPred> to_return(pre_drt);

	vector<boost::tuple<string, string, string> > competing;
	competing.push_back(boost::make_tuple("@PLACE_AT", "from", "@MOTION_FROM"));
	competing.push_back(boost::make_tuple("@PLACE_AT", "to", "@MOTION_TO"));
	competing.push_back(boost::make_tuple("@DATIVE", "for", "@FOR|@CAUSED_BY"));
	competing.push_back(boost::make_tuple("@DATIVE", "as", "@TIME_AT|@TOPIC"));
	competing.push_back(boost::make_tuple("@DATIVE", "at", "@PLACE_AT|@MOTION_TO"));
	competing.push_back(boost::make_tuple("@DATIVE", "with", "@WITH"));
	competing.push_back(boost::make_tuple("@DATIVE", "@WITH", "@WITH"));

	vector<pair<string, string> > tag_pairs;
	map<pair<string, string>, int> map_pairs;

	for (int n = 0; n < to_return.size(); ++n) {
		if (to_return.at(n).is_verb()) {
			for (int m = 0; m < competing.size(); ++m) {
				string candidate1 = competing.at(m).get<0>();
				string candidate2 = competing.at(m).get<1>();
				for (int i = 0; i < to_return.size(); ++i) {
					for (int j = 0; j < to_return.size(); ++j) {
						string head1 = extract_header(to_return.at(i));
						string head2 = extract_header(to_return.at(j));
						string ref2 = extract_second_tag(to_return.at(j));
						if (head1 == candidate1 && head2 == candidate2 && !(ref_is_name(ref2) || ref_is_verb(ref2))) { // the second candidate points to nothing
							add_header(to_return.at(i), ":DELETE");
							add_header(to_return.at(j), ":DELETE");
							to_return.at(i).name() += ":DELETE";
							to_return.at(j).name() += ":DELETE";
							DrtPred tmppred(to_return.at(i));
							string new_head = competing.at(m).get<2>();
							implant_header(tmppred, new_head);
							tmppred.name() = new_head;
							to_return.push_back(tmppred);
						}
					}
				}
				string head0 = extract_header(to_return.at(n));
			}
		}
	}

	return to_return;
}

vector<DrtPred> process_PAR(const vector<DrtPred> &pre_drt, const vector<string> &tags, const vector<string> &names,
		vector<pair<pair<int, int>, constituents> > &connections, bool is_question)
{
	if(pre_drt.size() == 0)
		return pre_drt;

	DrtVect to_return(pre_drt);
	// Nelson, a rear-admiral, won the battle of Trafalgar.
	// the first comma is not a conjunction, but introduces a parenthetical
	for (int n = 0; n < names.size(); ++n) {
		string head = extract_header(to_return.at(n));
		if (head == "-comma-[prn]") {
			string fref = extract_first_tag(to_return.at(n));
			string sref = extract_second_tag(to_return.at(n));
			vector<int> elements = find_all_compl_with_second_tag(to_return, tags, fref, "@AND");
			bool has_AND = elements.size() > 0 ? true : false;
			if (!has_AND)
				implant_header(to_return.at(n), "@PAR");
		}
	}

	return to_return;
}

vector<DrtPred> last_touch_PAR(const vector<DrtPred> &pre_drt, const vector<string> &tags, const vector<string> &names,
		vector<pair<pair<int, int>, constituents> > &connections, bool is_question)
// Albert - with whom David was a friend - was happy
// PRN(name1,verb) do(verb,...) -RBR-(name1) @WITH(name1,name2) -> do(verb,...) @WITH(verb,name2)
{
	DrtVect to_return(pre_drt);
	for (int n = 0; n < to_return.size(); ++n) {
		string head = extract_header(to_return.at(n));
		if (head == "@PRN") {
			string fref = extract_first_tag(to_return.at(n));
			string sref = extract_second_tag(to_return.at(n));
			if (!ref_is_name(fref))
				continue;
			bool RBR_trigger = false;
			vector<int> elements = find_all_element_with_string(to_return, fref);
			for (int m = 0; m < elements.size(); ++m) {
				string head2 = extract_header(to_return.at(elements.at(m)));
				if (head2 == "-RBR-") {
					RBR_trigger = true;
					break;
				}
			}
			for (int m = 0; RBR_trigger && m < elements.size(); ++m) {
				if (to_return.at(elements.at(m)).is_complement()) {
					implant_first(to_return.at(elements.at(m)), sref);
				}
			}
		}
	}

	return to_return;
}

static vector<DrtPred> process_lonely_articles(vector<DrtPred> pre_drt, const vector<string> &tags, const vector<string> &names)
{
	for (int n = 0; n < pre_drt.size(); ++n) {
		if (pre_drt.at(n).is_article()) {
			string header = extract_header(pre_drt.at(n));
			string fref = extract_first_tag(pre_drt.at(n));
			vector<int> poss = find_all_names_with_string_no_delete(pre_drt, fref);
			if (poss.size() > 0)
				continue;
			if (is_valid_article(header))
				pre_drt.at(n).setTag("NN");
		}
	}

	return pre_drt;
}

vector<DrtPred> resolve_RB(const vector<DrtPred> &pre_drt, const vector<string> &tags, const vector<string> &names,
		vector<pair<pair<int, int>, constituents> > &connections)
{
	int pos1, pos2;
	string str1, str2, tag1, tag2;

	DrtVect to_return(pre_drt);
	vector<pair<string, string> > ref_pair;

	for (int n = 0; n < connections.size(); ++n) { // assign missing links to RB
		pos1 = connections.at(n).first.first;
		pos2 = connections.at(n).first.second;
		str1 = names.at(pos1);
		str2 = names.at(pos2);
		tag1 = tags.at(pos1);
		tag2 = tags.at(pos2);

		string ref1 = extract_first_tag(to_return.at(pos1));
		string ref2 = extract_first_tag(to_return.at(pos2));

		if (tag2 == "RB" && !ref_is_verb(ref2)) {
			ref_pair.push_back(make_pair(ref2, ref1));
			break;
		}
		if (tag1 == "RB" && !ref_is_verb(ref1)) {
			ref_pair.push_back(make_pair(ref1, ref2));
			break;
		}
	}

	for (int n = 0; n < ref_pair.size(); ++n) {
		string PRP_ref = ref_pair.at(n).first;
		string name_ref = ref_pair.at(n).second;
		to_return = substitute_ref_with_name(to_return, PRP_ref, name_ref, "RB");
	}

	return to_return;
}

vector<DrtPred> resolve_articles(const vector<DrtPred> &pre_drt, const vector<string> &tags, const vector<string> &names,
		vector<bool> *mask, vector<pair<pair<int, int>, constituents> > &connections)
{
	int pos1, pos2;
	string str1, str2, tag1, tag2;

	DrtVect to_return(pre_drt);

	for (int n = 0; n < connections.size(); ++n) { // assign missing links to RB
		pos1 = connections.at(n).first.first;
		pos2 = connections.at(n).first.second;
		str1 = names.at(pos1);
		str2 = names.at(pos2);
		tag1 = tags.at(pos1);
		tag2 = tags.at(pos2);

		if(debug) {
			cout << "ARTICLES::: " << tag1 << " " << tag2 << " " << mask->at(pos1) << " " << mask->at(pos2) << endl;
			print_vector(to_return);
		}

		if (tag1 == "DT" && is_name(tag2) && mask->at(pos1) && mask->at(pos2)) {
			to_return = process_articles(to_return, pos1, pos2);
			if (!is_adjective(tag2))
				mask->at(pos1) = false;
		}
	}
	return to_return;
}

static DrtVect percolate_object_to_preposition(const vector<DrtPred> &pre_drt, const vector<string> &tags,
		const vector<string> &names, vector<pair<pair<int, int>, constituents> > &connections)
{
	int pos1, pos2;
	string str1, str2, tag1, tag2;

	DrtVect to_return(pre_drt);

	for (int n = 0; n < to_return.size(); ++n) {
		if (to_return.at(n).is_preposition()) {
			string head = extract_header(to_return.at(n));
			string fref = extract_first_tag(to_return.at(n));
			string sref = extract_second_tag(to_return.at(n));
			if (ref_is_verb(fref) && !ref_is_verb(sref) && !ref_is_name(sref) && head.find(":DELETE") == string::npos) {
				int m = find_verb_with_string(to_return, tags, fref);
				string obj = extract_object(to_return.at(m));
				implant_object(to_return.at(m), "none");
				implant_second(to_return.at(n), obj);
			}
		}
	}

	return to_return;
}

static string extract_adjective_from_header(const string &str)
{
	string to_return;
	int start, end;
	start = str.find("[");
	end = str.find("]");

	if (start == -1 || end == -1 || start + 1 >= end - 1)
		throw std::runtime_error(string("DrtBuilder: ") + str + " does not have an adjective.");

	return str.substr(start + 1, end - start - 1);
}

static DrtVect resolve_composite_comparatives(vector<DrtPred> to_return, const vector<string> &tags, const vector<string> &names,
		vector<pair<pair<int, int>, constituents> > &connections)
// "This is not as high a concern as my thesis"
// this(ref0) be(verb1,ref0,obj1) not(verb1) as[high](verb1,name5) concern(name5) thesis(ref8) @TOPIC(verb1,ref8) @TIME(verb1,present)
// -> this(ref0) be(verb1,ref0,name5) not(verb1) @COMPARED(verb1,[comp]name10) concern(name5) thesis(ref8) @COMPARED_TO([comp]name10,ref8) @TIME(verb1,present) high([comp]name10)
{
	int pos1, pos2;
	string str1, str2, tag1, tag2;

	for (int n = 0; n < to_return.size(); ++n) {
		string head = extract_header(to_return.at(n));
		if (head.find("as[") != string::npos) {
			string adj_str = extract_adjective_from_header(head);
			string fref = extract_first_tag(to_return.at(n));
			string sref = extract_second_tag(to_return.at(n));
			vector<int> candidates_as = find_complement_with_name(to_return, "as");
			int compared_to_pos = -1;
			for (int m = 0; m != candidates_as.size(); ++m) {
				if (candidates_as.at(m) == n)
					continue;
				string fref2 = extract_first_tag(to_return.at(candidates_as.at(m)));
				if (fref2 == fref) {
					compared_to_pos = candidates_as.at(m);
					break;
				}
			}
			string new_adj_ref = string("[compared_adj]name") + boost::lexical_cast<string>(to_return.size());
			DrtPred new_adj(adj_str + "/JJ" + "(" + new_adj_ref + ")");
			implant_header(to_return.at(n), "@COMPARED");
			implant_second(to_return.at(n), new_adj_ref);
			int vpos = find_verb_with_string(to_return, fref);
			if (vpos == -1 || has_object(to_return.at(vpos)))
				continue;
			implant_object(to_return.at(vpos), sref);
			if (compared_to_pos == -1)
				continue;
			implant_header(to_return.at(compared_to_pos), "@COMPARED_TO");
			implant_first(to_return.at(compared_to_pos), new_adj_ref);
			to_return.push_back(new_adj);
		}
	}
	return to_return;
}

static DrtVect process_question_WH_preds(const vector<DrtPred> &pre_drt, const vector<string> &tags, const vector<string> &names,
		vector<pair<pair<int, int>, constituents> > &connections)
{
	int pos1, pos2;
	string str1, str2, tag1, tag2;

	DrtVect to_return(pre_drt);
	vector<pair<string, string> > ref_pair;

	for (int n = 0; n < connections.size(); ++n) { // assign missing links to RB
		pos1 = connections.at(n).first.first;
		pos2 = connections.at(n).first.second;
		str1 = names.at(pos1);
		str2 = names.at(pos2);
		tag1 = tags.at(pos1);
		tag2 = tags.at(pos2);

		if (pos1 < pos2 && (is_WP(tag1)
		// || is_WRB(tag1)
				|| is_WP_pos(tag1) || is_WDT(tag1)) && there_are_only_names_between(tags, pos1 + 1, pos2) && is_name(tag2)) {
			string fref = extract_first_tag(to_return.at(pos1));
			string sref = extract_first_tag(to_return.at(pos2));
			ref_pair.push_back(make_pair(sref, fref));
			// if a WH pred contain a question sign the question in the new predicates
			if (to_return.at(pos1).is_question()) {
				to_return.at(pos2).set_question();
				to_return.at(pos2).set_question_word(names.at(pos2));
			}
		}
	}
	for (int n = 0; n < ref_pair.size(); ++n) {
		string PRP_ref = ref_pair.at(n).first;
		string name_ref = ref_pair.at(n).second;
		to_return = substitute_ref(to_return, PRP_ref, name_ref);
	}

	return to_return;
}


static DrtVect percolate_last_prepositions_to_verbs(const vector<DrtPred> &pre_drt, const vector<string> &tags, const vector<string> &names,
		vector<pair<pair<int, int>, constituents> > &connections)
{
	DrtVect to_return(pre_drt);

	for(int n=0; n < to_return.size()-1; ++n) {
		if(to_return.at(n+1).tag() == "-period-" && to_return.at(n).is_preposition() ) {
			to_return.at(n) = percolate_to_verb(to_return, tags, n );
			if(debug) {
				cout << "LAST_PREP:::" << to_return.at(n);
			}
		}
	}

	return to_return;
}

static DrtVect process_question_WRB_preds(const vector<DrtPred> &pre_drt, const vector<string> &tags, const vector<string> &names,
		vector<pair<pair<int, int>, constituents> > &connections)
{
	DrtVect to_return(pre_drt);

	for (int n = 0; n < to_return.size(); ++n) {
		if (to_return.at(n).is_WRB()) {
			string head = extract_header(to_return.at(n));
			string fref = extract_first_tag(to_return.at(n));
			string sref = extract_second_tag(to_return.at(n));
			if (!ref_is_verb(fref) && ref_is_verb(sref)) {
				switch_children(to_return.at(n));
			}
			if ((ref_is_verb(fref) && ref_is_name(sref)) || head == "how-many" || head == "how-many-times"
					|| head == "how-much") {
				string fref = extract_first_tag(to_return.at(n));
				string sref = extract_second_tag(to_return.at(n));
				int pos_verb = find_verb_with_string(to_return, tags, fref); // how many people are there?
				int pos_name = find_element_with_string(to_return, sref); // how many people are there?
				if (pos_name == -1 || pos_verb == -1)
					continue;
				string head = extract_header(to_return.at(pos_verb));
				if (to_return.at(pos_name).is_name() && !has_subject(to_return.at(pos_verb))
						&& head.find("PASSIVE") == string::npos) {
					implant_subject(to_return.at(pos_verb), sref);
				} else if (to_return.at(pos_name).is_name()
						 && !has_object(to_return.at(pos_verb))
						 && sref != extract_subject(to_return.at(pos_verb))
				) {
					implant_object(to_return.at(pos_verb), sref);
				}
				switch_children(to_return.at(n));
				string dummyname = string("dummyname") + boost::lexical_cast<string>(n);
				implant_second(to_return.at(n), dummyname);
			}
		}
	}

	return to_return;
}

static vector<DrtPred> adjust_WRB(vector<DrtPred> preds, const vector<string> &tags)
{
	for (int n = 0; n < preds.size(); ++n) {
		string head_str = extract_header(preds.at(n));
		if (preds.at(n).is_WRB()) {
			if (head_str == "why" || head_str == "how") {
				implant_header(preds.at(n), "@CAUSED_BY");
				preds.at(n).setName(head_str);
			}
			if (head_str == "whose") {
				implant_header(preds.at(n), "@GENITIVE");
				preds.at(n).setName(head_str);
			}
			if (head_str == "when") {
				implant_header(preds.at(n), "@TIME_AT");
				preds.at(n).setName(head_str);
			}
			if (head_str == "where") {
				implant_header(preds.at(n), "@PLACE_AT");
				preds.at(n).setName(head_str);
			}
			if (head_str == "at-what-time") {
				implant_header(preds.at(n), "@CLOCK_AT");
				preds.at(n).setName(head_str);
			}
			if (head_str == "how-many-times") {
				implant_header(preds.at(n), "@TIMES");
				preds.at(n).setName(head_str);
			}
			if (head_str == "how-many") {
				implant_header(preds.at(n), "@QUANTITY");
				string fref = extract_first_tag(preds.at(n));
				string sref = extract_second_tag(preds.at(n));
				int m2 = find_element_with_string(preds, sref); // how many people are there?
				if (m2 == -1)
					continue;
				if (m2 != -1 && preds.at(m2).is_name())
					continue;
				if (ref_is_verb(fref)) {
					int m = find_verb_with_string(preds, tags, fref);
					if (m != -1) {
						if (!has_object(preds.at(m))) {
							string dummyname = string("dummyref") + boost::lexical_cast<string>(m);
							DrtPred dummypred("person|!person/NN(" + dummyname + ")");
							implant_object(preds.at(m), dummyname);
							implant_first(preds.at(n), dummyname);
							preds.push_back(dummypred);
						}
					}
				}
			}
			if (head_str == "how-much") {
				implant_header(preds.at(n), "@QUANTITY");
				string fref = extract_first_tag(preds.at(n));
				string sref = extract_second_tag(preds.at(n));
				int m2 = find_element_with_string(preds, sref); // how much fuel do you have?
				if (m2 == -1)
					continue;
				if (m2 != -1 && preds.at(m2).is_name())
					continue;
				if (ref_is_verb(fref)) {
					int m = find_verb_with_string(preds, tags, fref);
					if (m != -1) {
						if (!has_subject(preds.at(m)) && !is_passive(preds.at(m))) {
							string dummyname = string("dummyname") + boost::lexical_cast<string>(m);
							DrtPred dummypred("usd|eur|money|years|%(" + dummyname + ")");
							implant_subject(preds.at(m), dummyname);
							implant_first(preds.at(n), dummyname);
							preds.push_back(dummypred);
						} else if (!has_object(preds.at(m))) {
							string dummyname = string("dummyname") + boost::lexical_cast<string>(m);
							DrtPred dummypred("usd|eur|money|years(" + dummyname + ")");
							implant_object(preds.at(m), dummyname);
							implant_first(preds.at(n), dummyname);
							preds.push_back(dummypred);
						}
					}
				}
			}
			if (head_str == "how-big") {
				implant_header(preds.at(n), "@SIZE");
				string fref = extract_first_tag(preds.at(n));
				string sref = extract_second_tag(preds.at(n));
				int m2 = find_element_with_string(preds, sref); // how long is the railroad?
				if (m2 == -1)
					continue;
				if (m2 != -1 && preds.at(m2).is_name())
					continue;
				if (ref_is_verb(fref)) {
					int m = find_verb_with_string(preds, tags, fref);
					if (m != -1) {
						if (!has_object(preds.at(m))) {
							string dummyname = string("dummyname") + boost::lexical_cast<string>(m);
							DrtPred dummypred("object|road|furniture|person(" + dummyname + ")");
							implant_object(preds.at(m), dummyname);
							implant_first(preds.at(n), dummyname);
							preds.push_back(dummypred);
						}
					}
				}
			}
			if (head_str == "how-old") {
				implant_header(preds.at(n), "@AGE");
			}
		}
	}
	return preds;
}

static DrtVect restore_originals(DrtVect to_return, const vector<string> &tags, const vector<string> &names)
{
	if(to_return.size() == 0)
		return to_return;

	for (int n = 0; n < names.size(); ++n) {
		if (is_preposition(tags.at(n))) {
			string ostr = names.at(n);
			ostr = ostr.substr(0, ostr.find(':'));
			string head = names.at(n) + "/" + tags.at(n);
			implant_header(to_return.at(n), head);
		}
		if (is_WDT(tags.at(n)) || is_WP(tags.at(n))) {
			string ostr = names.at(n);
			ostr = ostr.substr(0, ostr.find(':'));
			string head = names.at(n) + "/" + tags.at(n);
			implant_header(to_return.at(n), head);
		}
		to_return.at(n).setTag(tags.at(n));
	}
	return to_return;
}

static DrtVect change_money_symbols(DrtVect to_return, const vector<string> &tags, const vector<string> &names)
// $ -> usd
{
	for (int n = 0; n < names.size(); ++n) {
		if (names.at(n) == "$") {
			implant_header(to_return.at(n), "usd");
			to_return.at(n).setName("usd");
			to_return.at(n).setTag("NN");
		}
	}
	return to_return;
}

static DrtVect disentangle_subordinate_subjects(DrtVect to_return, const vector<string> &tags, const vector<string> &names)
{
	// find the verb references of subordinates (only for @SUBORD)
	vector<string> parent_verb_refs, sub_verb_refs;
	for (int n = 0; n < to_return.size(); ++n) {
		string fref = extract_first_tag(to_return.at(n));
		string sref = extract_second_tag(to_return.at(n));
		string head = extract_header(to_return.at(n));
		if (head == "@SUBORD" && to_return.at(n).is_complement() && ref_is_verb(sref)) {
			parent_verb_refs.push_back(sref);
			sub_verb_refs.push_back(sref);
		}
	}

	// find the subject_references of the parents
	vector<string> subj_refs;
	for (int n = 0; n < parent_verb_refs.size(); ++n) {
		int m = find_verb_with_string(to_return, parent_verb_refs.at(n));
		if (m == -1)
			continue;
		string subj_ref = extract_subject(to_return.at(m));
		subj_refs.push_back(subj_ref);
	}

	// modify the subjects of the subordinates
	for (int n = 0; n < sub_verb_refs.size(); ++n) {
		int m = find_verb_with_string(to_return, sub_verb_refs.at(n));
		if (m == -1)
			continue;
		string subj_ref = extract_subject(to_return.at(m));
		if (shortfind(subj_refs, subj_ref)) { // this subject is present in a parent sentence
			subj_ref = string("[subord]_") + boost::lexical_cast<string>(n) + "_" + subj_ref;
			implant_subject(to_return.at(m), subj_ref);
		}
	}

	return to_return;
}

static DrtVect convert_thereis_to_exist(DrtVect to_return, const vector<string> &tags, vector<string> &names)
{
	for (int n = 0; n < to_return.size(); ++n) {
		string fref = extract_first_tag(to_return.at(n));
		string header = extract_header(to_return.at(n));
		if (header == "there" && to_return.at(n).tag() == "EX") {
			int m = find_verb_with_subject(to_return, fref);
			if (m != -1 && m < names.size()) {
				string verb_header = extract_header(to_return.at(m));
				if (verb_header == "be") {
					names.at(m) = "exist";
					implant_header(to_return.at(m), "exist");
					to_return.at(m).setName("exist");
					add_header(to_return.at(n), ":DELETE");

					implant_subject(to_return.at(m), "none");
					switch_subj_obj(to_return.at(m));
				}
			}
			m = find_verb_with_object(to_return, fref);
			if (m != -1 && m < names.size()) {
				string verb_header = extract_header(to_return.at(m));
				if (verb_header == "be") {
					names.at(m) = "exist";
					implant_header(to_return.at(m), "exist");
					to_return.at(m).setName("exist");
					add_header(to_return.at(n), ":DELETE");
					implant_object(to_return.at(m), "none");
				}
			}
		}
	}
	return to_return;
}

static bool verb_supports_subordinate_subject(const string &str)
{
	vector<string> candidates;
	candidates.push_back("be");
	candidates.push_back("take");

	if(shortfind(candidates,str))
		return true;
	return false;
}


static bool has_blocking_adverb(const DrtVect &drtvect, int m)
{
	string fref= extract_header(drtvect.at(m));

	vector<string> blocking_adv;
	blocking_adv.push_back("ever");

	for(int n=0; n < drtvect.size(); ++n) {
		string header = extract_header(drtvect.at(n));
		if(drtvect.at(n).is_adverb() && shortfind(blocking_adv,header) )
			return true;
	}

	return false;
}

static DrtVect process_subordinate_subject(DrtVect to_return, const vector<string> &tags, const vector<string> &names,
		vector<pair<pair<int, int>, constituents> > &connections, const composition &comp)
{
	for (int n = 0; n < to_return.size(); ++n) {
		string fref = extract_first_tag(to_return.at(n));
		string sref = extract_second_tag(to_return.at(n));
		string header = extract_header(to_return.at(n));
		if (header == "@SUBORD") {
			int m = find_verb_with_string(to_return, fref);
			if (m == -1)
				continue;
			string verb_header = extract_header(to_return.at(m));
			if (!verb_supports_subordinate_subject(verb_header))
				continue;
			string subj_ref = extract_subject(to_return.at(m));
			int m2 = find_name_with_string(to_return, subj_ref);
			if (m2 == -1)
				continue;
			string subj_header = extract_header(to_return.at(m2));
			int m3 = find_verb_with_string(to_return, sref);
			if (m3 == -1)
				continue;
			if (subj_header == "it" && !has_blocking_adverb(to_return,m) ) {
				implant_subject(to_return.at(m3), "none");
				add_header(to_return.at(n), ":DELETE"); // delete "@SUBORD"
				add_header(to_return.at(m2), ":DELETE"); // delete "it"
				DrtPred act_pred((string) "[act]/NN#[pivot](" + subj_ref + ")");
				DrtPred genitive_pred((string) "@GENITIVE(" + subj_ref + "," + sref + ")");
				to_return.at(m3).setTag("VBG");
				to_return.push_back(act_pred);
				to_return.push_back(genitive_pred);
				if(debug) {
					puts("GENITIVE::: ");
				}
			}
		}
	}
	int pos1, pos2;
	string str1, str2, tag1, tag2;
	for (int n = 0; n < connections.size(); ++n) {
		pos1 = connections.at(n).first.first;
		pos2 = connections.at(n).first.second;
		str1 = names.at(pos1);
		str2 = names.at(pos2);
		tag1 = tags.at(pos1);
		tag2 = tags.at(pos2);
		constituents constit = connections.at(n).second;
		if ((tag1 == "VBG") && is_verb(tag2) && (is_subject(comp, pos1, pos2, constit) || is_object(comp, pos1, pos2, constit))
				&& !has_subject(to_return.at(pos2)) && !is_AUX(to_return.at(pos1)) // a VBG that is AUX cannot be subject
						) {
			implant_subject(to_return.at(pos1), "none");
			string vref1 = extract_first_tag(to_return.at(pos1));
			string vref2 = extract_first_tag(to_return.at(pos2));
			string dummyname = string("dummyref") + boost::lexical_cast<string>(pos1);
			DrtPred act_pred((string) "[act]/NN#[pivot](" + dummyname + ")");
			DrtPred genitive_pred((string) "@GENITIVE(" + dummyname + "," + vref1 + ")");
			to_return.at(pos1).setTag("VBG");
			implant_subject(to_return.at(pos2), dummyname);
			to_return.push_back(act_pred);
			to_return.push_back(genitive_pred);

			string o1 = extract_object(to_return.at(pos1));
			if (o1 == vref2)
				implant_object(to_return.at(pos1), "none");
		}
		if ((tag1 == "TO") && is_verb(tag2) && (is_subject(comp, pos1, pos2, constit) || is_object(comp, pos1, pos2, constit))
				&& !has_subject(to_return.at(pos2)) && !is_AUX(to_return.at(pos1)) // a VBG that is AUX cannot be subject
		) {
			if(debug) {
				cout << "TO233::: " << pos1 << " " << pos2 << endl;
			}
			string header1 = extract_header(to_return.at(pos2));
			if(header1 == "make" || header1 == "let")
				continue;
			string sref = extract_first_tag(to_return.at(pos1));
			int m = find_verb_with_string(to_return, sref);
			if(debug) {
				cout << "TO234::: " << pos1 << " " << pos2 << " " << m << endl;
			}
			if (m == -1)
				continue;
			if( !(pos1 == 0
				 || ( pos1 > 0 && to_return.at(pos1-1).tag() == "-comma-")
				 || ( pos1 > 0 && to_return.at(pos1-1).tag() == "IN")
				 )
			)
				continue;
			if (to_return.at(pos2).tag() != "VB")
				continue;
			implant_subject(to_return.at(pos2), "none");
			string vref1 = extract_first_tag(to_return.at(pos2));
			string vref2 = extract_first_tag(to_return.at(m));
			string dummyname = string("dummyref") + boost::lexical_cast<string>(pos2);
			DrtPred act_pred((string) "[act]/NN#[pivot](" + dummyname + ")");
			DrtPred genitive_pred((string) "@GENITIVE(" + dummyname + "," + vref1 + ")");
			to_return.at(pos2).setTag("VBG");
			implant_subject(to_return.at(m), dummyname);
			to_return.push_back(act_pred);
			to_return.push_back(genitive_pred);

			int m_sub = find_complement_with_first_tag(to_return, vref2, "@SUBORD");
			if(debug) {
				cout << "TO222::: " << vref2 << " " << m_sub << endl;
			}
			if (m_sub == -1)
				continue;
			string sub_ref2 = extract_second_tag(to_return.at(m_sub));
			if (sub_ref2 != vref1)
				continue;
			add_header(to_return.at(m_sub), ":DELETE");
		}
	}

	return to_return;
}

static DrtVect process_tough_move_drs(DrtVect to_return, const vector<string> &tags, const vector<string> &names)
{
	vector<string> trigger_strings;
	trigger_strings.push_back("easy");
	trigger_strings.push_back("simple");
	trigger_strings.push_back("tough");
	trigger_strings.push_back("hard");

	for (int n = 0; n < to_return.size(); ++n) {
		string fref = extract_first_tag(to_return.at(n));
		string sref = extract_second_tag(to_return.at(n));
		string header = extract_header(to_return.at(n));
		if (header == "@SUBORD") {
			int m = find_verb_with_string(to_return, fref);
			if (m == -1)
				continue;
			string verb_header = extract_header(to_return.at(m));
			if (verb_header != "be")
				continue;
			string subj_ref = extract_subject(to_return.at(m));
			string obj_ref = extract_object(to_return.at(m));
			int m2 = find_name_with_string(to_return, subj_ref);
			int m4 = find_name_with_string(to_return, obj_ref);
			if (m2 == -1 || m4 == -1)
				continue;
			string adj_header = extract_header(to_return.at(m4));
			if (!to_return.at(m4).is_adjective() || !shortfind(trigger_strings, adj_header))
				continue;
			string subj_header = extract_header(to_return.at(m2));
			int m3 = find_verb_with_string(to_return, sref);
			if (m3 == -1)
				continue;
			string dummyname = string("dummyref") + boost::lexical_cast<string>(m2);
			implant_subject(to_return.at(m3), "none");
			implant_object(to_return.at(m3), subj_ref);
			implant_subject(to_return.at(m), dummyname);
			add_header(to_return.at(n), ":DELETE"); // delete "@SUBORD"
			DrtPred act_pred((string) "[act]/NN#[pivot](" + dummyname + ")");
			DrtPred genitive_pred((string) "@GENITIVE(" + dummyname + "," + sref + ")");
			to_return.at(m3).setTag("VBG");
			to_return.push_back(act_pred);
			to_return.push_back(genitive_pred);
		}
	}
	return to_return;
}

static DrtVect process_able_move_drs(DrtVect to_return, const vector<string> &tags, const vector<string> &names)
{
	vector<string> trigger_strings;
	trigger_strings.push_back("able");
	trigger_strings.push_back("capable");

	for (int n = 0; n < to_return.size(); ++n) {
		string fref = extract_first_tag(to_return.at(n));
		string sref = extract_second_tag(to_return.at(n));
		string header = extract_header(to_return.at(n));
		if (header == "@SUBORD" && ref_is_verb(sref) && ref_is_verb(fref)) {
			int m = find_verb_with_string(to_return, fref);
			if (m == -1)
				continue;
			string verb_header = extract_header(to_return.at(m));
			if (verb_header != "be")
				continue;
			string subj_ref = extract_subject(to_return.at(m));
			string obj_ref = extract_object(to_return.at(m));
			int m2 = find_name_with_string(to_return, subj_ref);
			int m4 = find_name_with_string(to_return, obj_ref);
			if (m2 == -1 || m4 == -1)
				continue;
			string adj_header = extract_header(to_return.at(m4));
			if (!to_return.at(m4).is_adjective() || !shortfind(trigger_strings, adj_header))
				continue;
			int m3 = find_verb_with_string(to_return, sref);
			if (m3 == -1)
				continue;
			implant_first(to_return.at(n),obj_ref);
			implant_header(to_return.at(n),"@GENITIVE");
			// to_return.at(m3).setTag("VBG");

		}
	}
	return to_return;
}

static DrtVect RNR_for_adverbs(DrtVect orig, const vector<string> &tags, const vector<string> &names, bool is_question)
//dog/NNPS#[pivot](name0) smell/VB(verb2,name0,obj2) hear/VB(verb4,name0,obj4) well/RB(verb4) @CONJUNCTION(verb2,verb4)
// ->  ... , well/RB(verb2)
{
	DrtVect to_return(orig);

	for (int n = 0; !is_question && n < orig.size(); ++n) {
		string tag = orig.at(n).tag();
		if ( is_adverb(tag) ) {
			string fref = extract_first_tag(orig.at(n));
			if(!ref_is_verb(fref) )
				continue;
			int m= find_complement_with_target(orig,fref);
			if(m == -1)
				continue;
			if(debug) {
				cout << "TARGET::: " << orig.at(m) << endl;
			}
			string header = extract_header(orig.at(m));
			if(header != "@CONJUNCTION" &&  header != "@COORDINATION")
				continue;
			string vref1 = extract_first_tag(orig.at(m));
			if(!ref_is_verb(vref1) )
				continue;
			int m2= find_verb_with_string(orig,vref1);
			if(has_object(orig.at(m2)))
				continue; // verbs with object do not need adverbs
			if(has_pure_complements(orig, m2))
				continue; // verbs with complements do not need adverbs

			DrtPred new_adv(orig.at(n));
			implant_first(new_adv,vref1);
			to_return.push_back(new_adv);
		}
	}
	return to_return;
}


static DrtVect invert_genitive_for_CD(DrtVect to_return, const vector<string> &tags, const vector<string> &names)
// 1(A) @GENIITIVE(A,B) cyclones(B) -> 1(A) @QUANTIY(B,A) cyclones(B)
{
	for (int n = 0; n < to_return.size(); ++n) {
		string header = extract_header(to_return.at(n));
		if (header == "@GENITIVE") {
			string fref = extract_first_tag(to_return.at(n));
			string sref = extract_second_tag(to_return.at(n));
			vector<int> positions = find_all_names_with_string_no_delete(to_return, fref);
			if (positions.size() == 1 && positions.at(0) < tags.size() && (is_numeral(tags.at(positions.at(0))))
			) {
				to_return = substitute_ref(to_return, sref, fref);
				string quant_ref = string("name") + boost::lexical_cast<string>(to_return.size());
				implant_first(to_return.at(positions.at(0)), quant_ref);
				implant_header(to_return.at(n), "@QUANTITY");
				implant_first(to_return.at(n), fref);
				implant_second(to_return.at(n), quant_ref);
			}
			if (positions.size() == 1 && positions.at(0) < tags.size()
					&& (names.at(positions.at(0)) == "number" // && to_return.at(positions.at(0)).is_question()
					)
			) {
				implant_header(to_return.at(positions.at(0)), "[number]");
				to_return = substitute_ref(to_return, sref, fref);
				string quant_ref = string("name") + boost::lexical_cast<string>(to_return.size());
				implant_first(to_return.at(positions.at(0)), quant_ref);
				implant_header(to_return.at(n), "@QUANTITY");
				implant_first(to_return.at(n), fref);
				implant_second(to_return.at(n), quant_ref);
			}
		}
	}
	return to_return;
}

static DrtVect invert_genitive_for_quantifiers(DrtVect to_return, const vector<string> &tags, const vector<string> &names)
// some(A) @GENIITIVE(A,B) cyclones(B) -> some(A) @QUANTIFIER(B,A) cyclones(B)
{
	for (int n = 0; n < to_return.size(); ++n) {
		string header = extract_header(to_return.at(n));
		if (header == "@GENITIVE") {
			string fref = extract_first_tag(to_return.at(n));
			string sref = extract_second_tag(to_return.at(n));
			if (sref.find("presupp") != string::npos)
				continue; // the cycle must not consider presupposition
			vector<int> positions = find_all_element_with_string(to_return, fref);
			if (positions.at(0) < tags.size() && (tags.at(positions.at(0)) == "DT" || tags.at(positions.at(0)) == "JJS")
					&& is_quantifier_name(names.at(positions.at(0)))) {
				string header = extract_header(to_return.at(positions.at(0)));
				header = header.substr(0, header.find(":"));
				to_return = substitute_ref(to_return, sref, fref);
				implant_header(to_return.at(n), "@QUANTIFIER");
				implant_first(to_return.at(n), fref);
				implant_second(to_return.at(n), header);
				implant_header(to_return.at(positions.at(0)), header + ":DELETE");
			}
		}
	}
	return to_return;
}

static vector<DrtPred> process_implied_genitive(const DrtVect &drtvect, const vector<string> &tags, const vector<string> &names)
// snake(name1) species(name1) ->  snake(name1) species(name1) @GENITIVE(name1,[presupp_genitive]name1) snake([presupp_genitive]name1)
{
	vector<DrtPred> to_return;
	metric *d = metric_singleton::get_metric_instance();

	// extracts the adjectives
	vector<string> refs;
	map<string, vector<int> > map_ref;
	for (int n = 0; n < drtvect.size(); ++n) {
		string header = extract_header(drtvect.at(n));
		if (d->gender_proper_name(header) != "")
			continue; // "David Reed" is not "Reed of David"
		if (drtvect.at(n).is_name() && !drtvect.at(n).is_adjective() && !drtvect.at(n).is_article() && !drtvect.at(n).is_pivot()
				&& !is_delete(drtvect.at(n))) {
			string ref = extract_first_tag(drtvect.at(n));
			map_ref[ref].push_back(n);
			refs.push_back(ref);
		}
	}

	// Associate the adjective to the pivot name
	vector<string> already_done;
	int pnum = 0;
	for (int n = 0; n < drtvect.size(); ++n) {
		string ref = extract_first_tag(drtvect.at(n));
		if (drtvect.at(n).is_name() && drtvect.at(n).is_pivot() && !drtvect.at(n).is_adjective() && !is_delete(drtvect.at(n))
				&& find(already_done.begin(), already_done.end(), ref) == already_done.end()) {
			map<string, vector<int> >::iterator miter = map_ref.find(ref);
			if (miter != map_ref.end()) {
				vector<int> cpoz = miter->second;
				for (int m = 0; m < cpoz.size(); ++m) {
					int pos = cpoz.at(m);
					if (!there_are_only_names_between(drtvect, pos, n))
						continue;
					DrtPred target = drtvect.at(pos);
					string new_ref = "[presupp_genitive]_" + boost::lexical_cast<string>(pnum) + "_" + ref;
					DrtPred genitive(string("@GENITIVE(") + ref + "," + new_ref + ")");
					implant_first(target, new_ref);
					DrtVect tmpdrt;
					tmpdrt.push_back(genitive);
					tmpdrt.push_back(target);
					to_return.insert(to_return.end(), tmpdrt.begin(), tmpdrt.end());
					++pnum;
				}
			}
			already_done.push_back(ref);
		}
	}

	return to_return;
}

static DrtVect add_presupposition_elements(DrtVect to_return, const vector<string> &tags, const vector<string> &names)
{
	vector<DrtPred> to_add = process_implied_genitive(to_return, tags, names);
	to_return.insert(to_return.end(), to_add.begin(), to_add.end());
	return to_return;
}

static DrtVect change_anybody(DrtVect to_return, bool is_question)
{
	if (!is_question)
		return to_return;

	for (int n = 0; n < to_return.size(); ++n) {
		string head = extract_header(to_return.at(n));
		if (to_return.at(n).is_name() && head == "anything") {
			implant_header(to_return.at(n), "something");
			to_return.at(n).setName("something");
		}
		if (to_return.at(n).is_name() && head == "anyone") {
			implant_header(to_return.at(n), "someone");
			to_return.at(n).setName("someone");
		}
		if (to_return.at(n).is_name() && head == "anybody") {
			implant_header(to_return.at(n), "someone");
			to_return.at(n).setName("someone");
		}
	}

	return to_return;
}

DrtVect break_composed_nouns(DrtVect to_return)
{
	int size = to_return.size();
	metric *d = metric_singleton::get_metric_instance();
	for (int n = 0; n < size; ++n) {
		string head = extract_header(to_return.at(n));
		if(head.find(":Q") != string::npos) //-wikidate Wikidata names must not be broken
			continue;

		head = head.substr(0, head.find(":"));
		if (head.size() && to_return.at(n).is_name() && !to_return.at(n).is_complement() && !to_return.at(n).is_number()
				&& !isupper(head.at(0)) && extract_header(to_return.at(n)).find("[") == string::npos // no [date], etc...
				&& !is_place(extract_first_tag(to_return.at(n))) && head.find('_') != string::npos) {
			if (d->has_synset(head))
				continue;
			vector<string> strs;
			boost::split(strs, head, boost::is_any_of("_"));
			vector<DrtPred> new_preds;
			for (int m = 0; m < strs.size(); ++m) {
				DrtPred tmp_pred(to_return.at(n));
				string word = strs.at(m);
				string tag = to_return.at(n).tag();

				if (m != strs.size() - 1) {
					tmp_pred.set_pivot(false);
				}
				tagger_info *info = parser_singleton::get_tagger_info_instance();
				string base = info->get_conj(word, "NNS");
				if (base != "" && (tag == "NNS" || tag == "NNPS")) {
					word = base;
					tag = "NNS";
				}
				string new_tag = tag;
				base = info->get_conj(word, "NN");
				if ((tag == "NN" || tag == "NNP") && m != strs.size() - 1 && base != "") {
					new_tag = "NN";
				}

				tmp_pred.setName(word);
				tmp_pred.setTag(new_tag);
				implant_header(tmp_pred, word);
				new_preds.push_back(tmp_pred);
			}
			add_header(to_return.at(n), ":DELETE");
			to_return.insert(to_return.end(), new_preds.begin(), new_preds.end());
		}
	}

	return to_return;
}

DrtVect join_composed_nouns(DrtVect to_return)
{
	int size = to_return.size();
	metric *d = metric_singleton::get_metric_instance();
	tagger *tagg = parser_singleton::get_tagger_instance();
	for (int n = 0; n < size - 1; ++n) {
		string fref1 = extract_first_tag(to_return.at(n));
		string fref2 = extract_first_tag(to_return.at(n + 1));
		if (to_return.at(n).is_name() && to_return.at(n + 1).is_name() && fref1 == fref2) {
			string header = extract_header(to_return.at(n));
			string next_header = extract_header(to_return.at(n + 1));
			string new_header = header + "_" + next_header;

			if (d->pertains_to_name(header, "color", 6) > 0.5 || d->pertains_to_name(header, "newness", 6) > 0.5
					|| d->pertains_to_name(header, "political_orientation", 6) > 0.5
					|| d->hypernym_dist(header, "country", 8) > 0.5 || d->hypernym_dist(header, "color", 8) > 0.5
					|| d->pertains_to_name(header, "material", 6) > 0.5)
				continue; // blue cat -/-> blue_cat

			if (!d->has_synset(new_header)) {
				string next_header_tag= to_return.at(n+1).tag();
				if(next_header_tag == "NNS" || next_header_tag == "NNPS") {
					next_header = tagg->get_info()->conjugate(next_header, next_header_tag);
					string new_header = header + "_" + next_header;
					if (!d->has_synset(new_header))
						continue;
				} else
					continue;
			}

			to_return.at(n).setName(new_header);
			implant_header(to_return.at(n), new_header);
			add_header(to_return.at(n + 1), ":DELETE");
			if (to_return.at(n + 1).is_pivot())
				to_return.at(n).setTag(to_return.at(n + 1).tag());

			if (to_return.at(n + 1).is_pivot())
				to_return.at(n).set_pivot();
		}
	}

	return to_return;
}



boost::tuple<DrtVect, DrtVect, DrtPred> drt_builder::get_drt_form()
{
	vector<string> names = phrase_->get_names();
	vector<string> tags = phrase_->get_tags();
	vector<constituents> constit_list = phrase_->get_constit();
	vector<int> prn_depths= phrase_->get_prn_depths();
	if(debug) {
		puts("PRN_DEPTHS:::");
		print_vector(prn_depths);
	}

	vector<pair<pair<int, int>, constituents> > connections = phrase_->get_connections();
	composition comp = phrase_->get_composition();
	vector<bool> mask(names.size(), true);

	vector<DrtPred> to_return = get_drt_from_names(names, tags);
	bool is_question = contain_question(to_return);
	bool is_fake_question = contain_fake_question(to_return);
	if (is_fake_question)
		is_question = false;

	to_return = sign_pivots(to_return, comp);
	to_return = change_anybody(to_return, is_question);

	string str1, str2, tag1, tag2;
	int pos1, pos2;

	to_return = tough_move(to_return, connections, comp, tags, names, &mask);

	to_return = resolve_articles(to_return, tags, names, &mask, connections);

	for (int m = 0; m < 4; ++m) { // Bad Solution!! you should implement a substitution list
		for (int n = 0; n < connections.size(); ++n) { // Process nouns, possessive pronouns. Not adjectives
			pos1 = connections.at(n).first.first;
			pos2 = connections.at(n).first.second;
			str1 = names.at(pos1);
			str2 = names.at(pos2);
			tag1 = tags.at(pos1);
			tag2 = tags.at(pos2);

			if ( is_name(tag1)  && is_name(tag2)
					&& (there_are_only_names_between(tags, pos1, pos2) || only_one_RB_between(tags, pos1, pos2))) {
				to_return = process_names(to_return, pos1, pos2);
			}
		}
	}

	if (debug) {
		print_vector(to_return);
	}
	for (int n = 0; n < connections.size(); ++n) { // Process adjectives
		pos1 = connections.at(n).first.first;
		pos2 = connections.at(n).first.second;
		str1 = names.at(pos1);
		str2 = names.at(pos2);
		tag1 = tags.at(pos1);
		tag2 = tags.at(pos2);
		if ((is_adjective(tag1) && is_name(tag2) && there_are_only_names_between(tags, pos1, pos2))
				|| (is_adjective(tag1) && is_adjective(tag2) && there_are_only_names_between(tags, pos1, pos2))) {
			to_return = process_adjectives(to_return, pos2, pos1);
		} else if ((is_adjective(tag2) && is_name(tag1) && there_are_only_names_between(tags, pos1, pos2))
				|| (is_adjective(tag2) && is_adjective(tag1) && there_are_only_names_between(tags, pos1, pos2)))
			to_return = process_adjectives(to_return, pos1, pos2);
	}

	if (debug) {
		print_vector(to_return);
	}
	for (int n = 0; n < connections.size(); ++n) { // Process wh-determiners (which, ...) and wh-adverbs (what, where, ...)
		pos1 = connections.at(n).first.first;
		pos2 = connections.at(n).first.second;
		str1 = names.at(pos1);
		str2 = names.at(pos2);
		tag1 = tags.at(pos1);
		tag2 = tags.at(pos2);

		//WDT SHOULD HAVE JUST ONE CHILD. ALSO IMPROVE THE CCG TREE WITH EXCEPTIONS

		if (is_WRB(tag2) // || is_WP(tag2)
				) {
			if (pos1 < pos2)
				to_return = process_WRB_from(to_return, pos1, pos2);
			else
				to_return = process_WRB_to(to_return, pos1, pos2);
		}
		if (is_WRB(tag1) // || is_WP(tag1)
				) {
			if (pos2 < pos1)
				to_return = process_WRB_from(to_return, pos2, pos1);
			else
				to_return = process_WRB_to(to_return, pos2, pos1);
		}
	}
	if (debug) {
		print_vector(to_return);
	}
	for (int n = 0; n < connections.size(); ++n) { // Process verbs
		pos1 = connections.at(n).first.first;
		pos2 = connections.at(n).first.second;
		str1 = names.at(pos1);
		str2 = names.at(pos2);
		tag1 = tags.at(pos1);
		tag2 = tags.at(pos2);
		constituents constit = connections.at(n).second;

		if (is_verb(tag1)) {
			if (is_verb(tag2) && is_valid_aux(constit) && verbs_can_join(to_return, pos1, pos2, prn_depths, is_question)) { /// BAD SOLUTION! This is for processing auxiliary verbs. However you should use CCG to determine auxiliaries
				if (pos2 < pos1) {
					to_return = add_subject(to_return, pos2, pos1);
				} else {
					if (!has_object(to_return.at(pos1))) {
						if (is_transitive(str1))
							to_return = add_object(to_return, pos2, pos1);
					} else
						to_return = add_subject(to_return, pos1, pos2);
				}
			}
			if (constit != constit_list.at(pos2)) // only names like NP_1 can be subjects or objects
				continue;
			if (tag_is_candidate_subject(tag2, str2) //&& !has_subject_nish(to_return.at(pos1))
					&& !(tag2 == "EX" && str1 != "be") // only "be" can have a "there/EX"
					&& is_subject(comp, pos1, pos2, constit, is_auxiliary_name(str2) || is_modal(tag2))
					&& !(has_subject(to_return.at(pos1)) && is_transitive(str1) && is_object(comp, pos1, pos2, constit))) {
				if (is_question && verb_supports_indirect_obj(str1)) {
					// "what gives peace a chance?"
					to_return = add_object(to_return, pos2, pos1);
				} else {
					to_return = add_subject(to_return, pos2, pos1);
				}
			} else if (tag_is_candidate_object(tag2, str2)) {
				if (verb_supports_indirect_obj(str1) && is_indirect_object(comp, pos1, pos2, constit)) {
					to_return = add_indirect_object(to_return, pos1, pos2);
				} else if (is_transitive(str1) // && !has_object(to_return.at(pos1))
				&& is_object(comp, pos1, pos2, constit)
				//&& !is_subject(comp,pos1,pos2,constit)
						) {
					to_return = add_object(to_return, pos2, pos1);
				}
			}
		} else if (is_verb(tag2)) {
			// if(is_verb(tag1)) { /// BAD SOLUTION! This is for
			// 	    processing auxiliary verbs. However you
			// 	    should use CCG to determine auxiliaries
			// 	    (this instance never happens, since the
			// 	    test is done in the previous if) if(pos1 <
			// 	    pos2) to_return= add_subject(to_return,
			// 	    pos2, pos1); else to_return=
			// 	    add_object(to_return, pos2, pos1); }
			if (constit != constit_list.at(pos1)) // only names like NP_1 can be subjects or objects
				continue;
			if (tag_is_candidate_subject(tag1, str1) //&& !has_subject_nish(to_return.at(pos2))
					&& !(tag1 == "EX" && str2 != "be") // only "be" can have a "there/EX"
					&& is_subject(comp, pos2, pos1, constit, is_auxiliary_name(str1) || is_modal(tag1))
					&& !(has_subject(to_return.at(pos2)) && is_transitive(str2) && is_object(comp, pos2, pos1, constit))) {
				to_return = add_subject(to_return, pos1, pos2);
			} else if (tag_is_candidate_object(tag1, str1)) {
				if (verb_supports_indirect_obj(str2) && is_indirect_object(comp, pos2, pos1, constit)) {
					to_return = add_indirect_object(to_return, pos2, pos1);
				} else if (is_transitive(str2) // && !has_object(to_return.at(pos2))
				&& is_object(comp, pos2, pos1, constit)
				//&& !is_subject(comp,pos2,pos1,constit)
						) {
					to_return = add_object(to_return, pos1, pos2);
				}
			}
		}
		if (debug) {
			puts("VERBS_PPP:::");
			print_vector(to_return);
		}

	}

	for (int n = 0; n < connections.size(); ++n) { // Process objects that should be @DATIVE
		pos1 = connections.at(n).first.first;
		pos2 = connections.at(n).first.second;
		str1 = names.at(pos1);
		str2 = names.at(pos2);
		tag1 = tags.at(pos1);
		tag2 = tags.at(pos2);
		constituents constit = connections.at(n).second;

		if (is_verb(tag1) && !has_object(to_return.at(pos1)) ) {
			if (tag_is_candidate_object(tag2, str2)) {
				if (verb_supports_indirect_obj(str1) && !is_transitive(str1)
					&& is_object(comp, pos2, pos1, constit) && !is_indirect_object(comp, pos2, pos1, constit)) {
					to_return = add_indirect_object(to_return, pos1, pos2);
				}
			}
		}
	}


	if (debug) {
		puts("VERBS:::");
		print_vector(to_return);
	}

	for (int ncycles = 0; ncycles < 2; ++ncycles) {
		/// BAD SOLUTION: the problem are prepositions that are
		/// associated to other preposition, and then the other
		/// preposition points to another name.
		constituents constit, constit_old;
		for (int n = 0; n < connections.size(); ++n) { // Process prepositions and adverbs
			pos1 = connections.at(n).first.first;
			pos2 = connections.at(n).first.second;
			str1 = names.at(pos1);
			str2 = names.at(pos2);
			tag1 = tags.at(pos1);
			tag2 = tags.at(pos2);
			constit = connections.at(n).second;
			if (is_preposition(tag1) && mask.at(pos1)) {
				if (pos2 < pos1) {
					if ( is_preposition(tag2) && !is_POS(tag2)) {
						// the saxon genitive POS is excluded from the preposition list, because it would skew process_preposition_from_prepositions
						to_return = process_preposition_from_preposition(to_return, pos2, pos1);
					} else if (!is_parenthesis(tag2)) {
						if (!(is_POS(tag1) && is_verb(tag2)) // 's do not correlate to verbs!
						&& !(is_than(str2) && is_verb(tag1))) { // "than" do not correlate to verbs!
							to_return = process_preposition_from(to_return, pos2, pos1);
						}
					}
				} else if (is_name(tag2) || is_verb(tag2) || is_WP(tag2) || is_WDT(tag2) || (tag2 == "RB" && str2 == "then")
						|| tag2 == "JJR" || (is_article(tag2) && is_valid_article(str2))) {
					if (!(is_POS(tag1) && is_verb(tag2))) { // 's do not correlate to verbs!
						if ((!is_verb(tag2) && is_candidate_prep_to(comp, pos1, pos2, constit)) || is_verb(tag2)) {
							to_return = process_preposition_to(to_return, pos2, pos1);
						}
					}
				}
			}
			if (is_preposition(tag2) && mask.at(pos2)) {
				if (pos1 < pos2) {
					if ( is_preposition(tag1) && !is_POS(tag1)) {
						to_return = process_preposition_from_preposition(to_return, pos1, pos2);
					} else if (!is_parenthesis(tag1)) {
						if (!(is_POS(tag2) && is_verb(tag1)) // 's do not correlate to verbs!
						&& !(is_than(str2) && is_verb(tag1))) { // "than" do not correlate to verbs!
							to_return = process_preposition_from(to_return, pos1, pos2);
						}
					}
				} else if (is_name(tag1) || is_verb(tag1) || (tag1 == "RB" && str1 == "then") || tag1 == "JJR") {
					if (!(is_POS(tag2) && is_verb(tag1)) && is_candidate_prep_to(comp, pos2, pos1, constit)) { // 's do not correlate to verbs!
						if ((!is_verb(tag1) && is_candidate_prep_to(comp, pos2, pos1, constit)) || is_verb(tag1)) {
							to_return = process_preposition_to(to_return, pos1, pos2);
						}
					}
				}
			}
			if (tag2 == "RB" && is_verb(tag1))
				to_return = process_adverb(to_return, pos1, pos2);
			if (tag1 == "RB" && (is_verb(tag2) || is_adverb(tag2)))
				//if(tag1== "RB")
				to_return = process_adverb(to_return, pos2, pos1);
			if (tag2 == "RP")
				to_return = process_adverb(to_return, pos1, pos2);
			if (tag1 == "RP")
				to_return = process_adverb(to_return, pos2, pos1);

			if (tag2 == "RBR" && is_verb(tag1))
				to_return = process_comparative_adverb(to_return, pos1, pos2);
			if (tag1 == "RBR")
				to_return = process_comparative_adverb(to_return, pos2, pos1);
		}
	}
	if (debug) {
		print_vector(to_return);
	}

	to_return = resolve_and_to(to_return, tags, names, is_question);
	to_return = resolve_WDT_lazy(to_return, tags, names, is_question);

	for (int n = 0; n < connections.size(); ++n) { // assign missing links to WPs
		// This must be done after all the subjects and objects are processed
		pos1 = connections.at(n).first.first;
		pos2 = connections.at(n).first.second;
		str1 = names.at(pos1);
		str2 = names.at(pos2);
		tag1 = tags.at(pos1);
		tag2 = tags.at(pos2);

		if ((is_WP(tag1) || is_WDT(tag1)) && is_verb(tag2)) {
			string WP_ref = extract_first_tag(to_return.at(pos1));
			string subj_ref = extract_subject(to_return.at(pos2));
			string obj_ref = extract_object(to_return.at(pos2));
			string verb_ref = extract_first_tag(to_return.at(pos2));
			if (WP_ref == subj_ref || WP_ref == obj_ref)
				continue; // If the WP is already in the verb do nothing
			// otherwise the WP becomes subject or object (the one that is missing

			// If "what" is connected to the verb "do" then simply
			// erase the what. This is because the sentence "he
			// does what?" is a question about the action done.
			/// This should be in process_WP!!!!!
			string verb_str = to_return.at(pos2).name();
			tagger *tagg = parser_singleton::get_tagger_instance();
			string base = tagg->get_info()->get_conj(verb_str, tag2);
			if (base == "")
				base = verb_str;
			if (base == "do" && !ref_is_verb(extract_object(to_return.at(pos2))) // do not delete the "what" for and AUX
					) {
				add_header(to_return.at(pos1), ":DELETE");
				to_return.at(pos1).setName(":DELETE");
				names.at(pos1) += ":DELETE";
				continue;
			}
			to_return = substitute_subj_or_obj_with_string(WP_ref, verb_ref, to_return, tags);
		}
	}
	if (debug) {
		puts("WPVERBS:::");
		print_vector(to_return);
	}

	to_return = process_hashtags(to_return, tags, names); // modifies the names!


	// It must be after processing the preposition, to resolve cases like "in which ..."
	/// some thoughts: for question it is different: "who is the brother of whom?". All WP should not be assigned

	for (int n = 0; n < connections.size(); ++n) { // conjunctions
		pos1 = connections.at(n).first.first;
		pos2 = connections.at(n).first.second;
		str1 = names.at(pos1);
		str2 = names.at(pos2);
		tag1 = tags.at(pos1);
		tag2 = tags.at(pos2);

		if (is_conj(tag2) && (is_verb(tag1) || is_name(tag1) || is_WP(tag1))) {
			if (pos1 < pos2
					&& !(has_first_tag(to_return.at(pos2)) && !ref_is_verb(extract_first_tag(to_return.at(pos2))) )
			)
				to_return = process_conj_prev(to_return, pos1, pos2);
			else if (pos1 > pos2 && !has_second_tag(to_return.at(pos2)))
				to_return = process_conj_next(to_return, pos1, pos2);
		}
		if (is_conj(tag1) && (is_verb(tag2) || is_name(tag2) || is_WP(tag2))) {
			if (!has_first_tag(to_return.at(pos1)) && (pos2 < pos1 || has_second_tag(to_return.at(pos1)))) { // The previous tag can be after the preposition (if the next is already taken)
				to_return = process_conj_prev(to_return, pos2, pos1);
			} else if (pos2 > pos1
					 && !(has_second_tag(to_return.at(pos1)) && !ref_is_verb(extract_second_tag(to_return.at(pos1))) ) // nouns are preferred to verbs as items on the right
					 ) {
				to_return = process_conj_next(to_return, pos2, pos1);
			}
		}
	}

	to_return = percolate_last_prepositions_to_verbs(to_return, tags, names, connections);

	if (debug) {
		puts("MISSING0::");
		print_vector(to_return);

	}

	for (int n = 0; n < connections.size(); ++n) {
// if conjunctions are not connected to anything, then tries to
// connect them to prepositions. This step must be done after processing prepositions
		pos1 = connections.at(n).first.first;
		pos2 = connections.at(n).first.second;
		str1 = names.at(pos1);
		str2 = names.at(pos2);
		tag1 = tags.at(pos1);
		tag2 = tags.at(pos2);

		if (is_conj(tag2) && is_preposition(tag1)) {
			if (pos1 < pos2 && !has_first_tag(to_return.at(pos2))) {
				to_return = process_conj_prev(to_return, pos1, pos2);
			} else if (!has_second_tag(to_return.at(pos2))) {
				to_return = process_conj_next(to_return, pos1, pos2);
			}
		}
		if (is_conj(tag1) && is_preposition(tag2)) {
			if (pos2 < pos1 && !has_first_tag(to_return.at(pos1)))
				to_return = process_conj_prev(to_return, pos2, pos1);
			else if (!has_second_tag(to_return.at(pos1)))
				to_return = process_conj_next(to_return, pos2, pos1);
		}
	}
	if (debug) {
		puts("MISSING::");
		print_vector(to_return);

	}

	to_return = process_ifs_with_last_verb(to_return, tags, names, connections);
	to_return = process_WRB_to_nouns(to_return, tags, names, connections);

	constituents constit, constit_old;
	for (int n = 0; n < connections.size(); ++n) { // assign missing links to prepositions and WRBs
		pos1 = connections.at(n).first.first;
		pos2 = connections.at(n).first.second;
		str1 = names.at(pos1);
		str2 = names.at(pos2);
		tag1 = tags.at(pos1);
		tag2 = tags.at(pos2);
		constit = connections.at(n).second;
		if ((is_preposition(tag1) || is_WRB(tag1) || is_comma(tag1)) && mask.at(pos1)) {
			if (first_tag_is_incomplete(to_return.at(pos1)) || (points_to_WDT(to_return, pos1) && !is_question)) { // The preposition does not point at any previous element

				if(debug) {
					cout << "FT_WRB::: " << to_return.at(pos1) << endl;
				}


				int pos1_verb, pos2_verb;
				// pos1_verb is the position of the verb pointed by the prep in the second tag
				// pos2_verb is the position of the verb given by the connections
				string ref_orig = extract_second_tag(to_return.at(pos1));
				string ref2;
				if (to_return.at(pos2).is_WRB()) // for WRBs the percolation is inverted
					ref2 = extract_second_tag(to_return.at(pos2));
				else
					ref2 = extract_first_tag(to_return.at(pos2));
				if (!to_return.at(pos2).is_verb() && ref2 != ref_orig) {
					pos2_verb = percolate_to_verb_integer(to_return, tags, pos2);
					if (pos2_verb == -1) {
						pos2_verb = percolate_to_name_integer(to_return, tags, pos2);
					}
					if(debug) {
						cout << "FT_WRB2::: " << pos1_verb << " " << pos2_verb << endl;
					}
				} else
					pos2_verb = pos2;
				pos1_verb = percolate_second_tag_to_verb_integer(to_return, tags, pos1);
				if (pos1_verb == -1)
					pos1_verb = percolate_to_name_integer(to_return, tags, pos1);

				if(debug) {
					cout << "FT_WRB3::: " << pos1_verb << " " << pos2_verb << endl;
				}
				if(pos1_verb == pos2_verb)
					pos2_verb= pos2;

				if (pos1_verb != pos2_verb && pos1_verb != -1 && pos2_verb != -1) {
					string from_str = extract_first_tag(to_return.at(pos2_verb));
					string to_str = extract_second_tag(to_return.at(pos1));
					if (from_str != to_str // this last is to avoid "prep(same,same)"
						&& !close_loop_str(to_return, from_str, to_str) && !close_loop_str(to_return, to_str, from_str)
						&& !( is_question && prn_depths.at(pos2_verb) != prn_depths.at(pos1) )
					)
						implant_first(to_return.at(pos1), from_str);
				}
			}
		}
		if ((is_preposition(tag2) || is_WRB(tag2) || is_comma(tag2)) && mask.at(pos2)) {
			if (first_tag_is_incomplete(to_return.at(pos2))) { // The preposition does not point at any previous element
				int pos1_verb, pos2_verb;
				// pos1_verb is the position of the verb pointed by the prep in the second tag
				// pos2_verb is the position of the verb given by the connections
				if (!to_return.at(pos1).is_verb()) {
					pos1_verb = percolate_to_verb_integer(to_return, tags, pos1);
					if (pos1_verb == -1) {
						pos1_verb = percolate_to_name_integer(to_return, tags, pos1);
					}
				} else
					pos1_verb = pos1;
				pos2_verb = percolate_second_tag_to_verb_integer(to_return, tags, pos2);
				if (pos2_verb == -1)
					pos2_verb = percolate_to_name_integer(to_return, tags, pos2);
				if (pos1_verb != pos2_verb && pos1_verb != -1 && pos2_verb != -1) {
					string to_str = extract_second_tag(to_return.at(pos2_verb));
					string from_str;
					if (to_return.at(pos1_verb).is_WRB()) // for WRBs the percolation is inverted
						from_str = extract_second_tag(to_return.at(pos1_verb));
					else
						from_str = extract_first_tag(to_return.at(pos1_verb));
					if (from_str != to_str && !close_loop_str(to_return, from_str, to_str)
						&& !close_loop_str(to_return, to_str, from_str)
						&& !( is_question && prn_depths.at(pos2) != prn_depths.at(pos1_verb) )
					)
						implant_first(to_return.at(pos2), from_str);
				}
			}
		}

	}

	if (debug) {
		puts("AFTER_MISSING:::");
		print_vector(to_return);
	}

	// some preposition might be linked to conjunctions that point to verbs
	to_return = commas_and_preposition(to_return, tags, names);

	// some adverbs might be linked to conjunctions that point to verbs
	to_return = commas_and_adverb(to_return, tags, names);
	if (debug) {
		puts("AFTER_RB2:::");
		print_vector(to_return);
	}
	to_return = resolve_RB(to_return, tags, names, connections);

	// Associate RP to the verb /// Check if it is an adjunct!
	for (int n = 0; n < tags.size(); ++n) { // Process prepositions and adverbs
		str1 = names.at(n);
		tag1 = tags.at(n);

		if (tag1 == "RB" || tag1 == "RP") {
			string fref = extract_first_tag(to_return.at(n));
			if (fref.find("verb") == string::npos) // if it is not already connected to a verb
				to_return.at(n) = percolate_to_verb(to_return, tags, n); // If there is no verb nothing changes
		}
	}
	to_return = process_impersonal(to_return, tags, names);

	to_return = join_verb_with_RP(to_return, tags, names, connections);
	to_return = join_verb_with_name(to_return, tags, names);

	// you should join verbs with preposition before naming complements

	/// Problem with prepositions: deadlock!
	/// Preposition cannot associate to auxiliaries, but you know if a verb is AUX only after
	/// collapse_verbs. However, join_verb_with_preposition must be before name_complements,
	/// and name_complements must be before collapse_verbs. You must find a way to track AUX before collapse verbs

	to_return = join_verb_with_preposition(to_return, tags, names);
	to_return = process_unit_of_measure_place(to_return, tags, connections);
	to_return = convert_thereis_to_exist(to_return, tags, names);

	if (debug) {
		puts("NAME_COMPL0:::");
		print_vector(to_return);
	}
	to_return = name_complements(to_return, tags, names, is_question);
	if (debug) {
		puts("NAME_COMPL:::");
		print_vector(to_return);
	}
	to_return = correct_double_dative(to_return, tags, names, is_question);
	if (debug) {
		print_vector(to_return);
	}
	to_return = collapse_verbs(to_return, tags, names, prn_depths, is_question);

	for (int n = 0; n < connections.size(); ++n) { // assign missing links to WPs
		// This must be done after all the subjects and objects are processed
		pos1 = connections.at(n).first.first;
		pos2 = connections.at(n).first.second;
		str1 = names.at(pos1);
		str2 = names.at(pos2);
		tag1 = tags.at(pos1);
		tag2 = tags.at(pos2);

		// sometimes the verb "do" is connected by preposition "what are snakes able to do?"
		if ((is_WP(tag1) || is_WDT(tag1)) && is_preposition(tag2)) {
			string fref = extract_first_tag(to_return.at(pos2));
			string sref = extract_second_tag(to_return.at(pos2));

			if (debug) {
				cout << "DOBASE::: " << sref << endl;
			}

			int m = find_verb_with_string(to_return, sref);
			if (m == -1)
				continue;
			// If "what" is connected to the verb "do" then simply
			// erase the what. This is because the sentence "he
			// does what?" is a question about the action done.
			/// This should be in process_WP!!!!!
			string verb_str = to_return.at(m).name();
			tagger *tagg = parser_singleton::get_tagger_instance();
			string base = tagg->get_info()->get_conj(verb_str, tag2);
			if (base == "")
				base = verb_str;
			if (base == "do") {
				add_header(to_return.at(pos1), ":DELETE");
				to_return.at(pos1).setName(":DELETE");
				names.at(pos1) += ":DELETE";
				to_return.at(m).set_question();
				to_return.at(m).set_question_word("do-something");
				continue;
			}
		}
	}
	to_return = last_touch_END(to_return, tags, names); // must go BEFORE percolate_subject_to_subordinates
	if (debug) {
		puts("BEFORE_RAISING0::: ");
		print_vector(to_return);
	}
	if(!is_question) {
		to_return = process_subordinate_subject(to_return, tags, names, connections, comp); // must be done before the percolation to subordinates
	}
	if (debug) {
		puts("BEFORE_RAISING1::: ");
		print_vector(to_return);
	}
	if(!is_question) {
		to_return = percolate_subject_to_subordinates(to_return, tags, names);
	}
	if (debug) {
		puts("BEFORE_RAISING::: ");
		print_vector(to_return);
	}

	to_return = process_auxiliary_like_verbs(to_return, tags, names, connections);

	// Checks if a verb is connected to another verb and gives a
	// subject to the subordinate verb
	for (int n = 0; n < tags.size(); ++n) { // Process prepositions and adverbs
		str1 = names.at(n);
		tag1 = tags.at(n);

		if ((is_preposition(tag1) || is_conj(tag1))) {
			to_return = raise_subject_from_preposition(to_return, tags, n);
		}
	}

	if (debug) {
		puts("AFTER_RAISING::: ");
		print_vector(to_return);
	}

	for (int n = 0; n < connections.size(); ++n) {
		// This must be done after all the subjects and objects are processed
		pos1 = connections.at(n).first.first;
		pos2 = connections.at(n).first.second;
		str1 = names.at(pos1);
		str2 = names.at(pos2);
		tag1 = tags.at(pos1);
		tag2 = tags.at(pos2);

		// S[dcl]_1\S[dcl]_0 gets the subj from S[dcl]_0
		if (is_verb(tag1) && is_verb(tag2) && !is_AUX(to_return.at(pos1)) && !is_AUX(to_return.at(pos2))
				&& !to_return.at(pos1).is_delete() && !to_return.at(pos2).is_delete() && !is_passive(to_return.at(pos1))
				&& !is_passive(to_return.at(pos2)) && !has_subject(to_return.at(pos2))) {
			string s1 = extract_subject(to_return.at(pos1));
			string s2 = extract_subject(to_return.at(pos2));
			string o2 = extract_object(to_return.at(pos2));
			if (s2 != o2) {
				implant_subject(to_return.at(pos2), s1);
			}
		}
	}

	if (debug) {
		print_vector(to_return);
	}
	to_return = attach_comparatives(to_return, tags, names, connections);
	if (debug) {
		puts("THAT:::");
		print_vector(to_return);

	}
	to_return = prepare_THAT(to_return, tags, names, connections);
	if (debug) {
		puts("THAT2:::");
		print_vector(to_return);

	}
	to_return = resolve_THAT(to_return, tags, names, connections);
	if (debug) {
		puts("AFTER_THAT:::");
		print_vector(to_return);
	}
	to_return = join_verb_with_adverb(to_return, tags, names);
	if (debug) {
		puts("BEFORE_SWITCH:::");
		print_vector(to_return);

	}
	to_return = process_switched_END(to_return, tags, names); // process_switched_END() must go before resolving internal anaphoras
	if (debug) {
		puts("AFTER_SWITCH:::");
		print_vector(to_return);

	}
	to_return = internal_anaphoras_backward(to_return, tags, names);

	if (debug) {
		puts("COORD::: ");
		print_vector(to_return);
	}

	to_return = process_coord(to_return, tags, names);

	to_return = post_process_passive(to_return, connections);
	to_return = process_lonely_articles(to_return, tags, names);

	if (contain_question(to_return)) {
		// This goes before process_WP
		to_return = collapse_OR(to_return, tags, names);
		to_return = sign_some_predicates_as_questions(to_return, tags, names);
	}

	if (debug) {
		puts("BEFORE_WP::: ");
		print_vector(to_return);
	}
	to_return = process_WP(to_return, tags, names);
	if (debug) {
		puts("AFTER_WP::: ");
		print_vector(to_return);
	}

	to_return = process_WP_pos(to_return, tags, names, is_question);
	to_return = process_WP(to_return, tags, names);
	to_return = process_WP_from_AUX(to_return, tags, names, connections, is_question);
	to_return = process_prep_WDT_from_verb(to_return, tags, names, connections, is_question);
	to_return = name_complements(to_return, tags, names, is_question); // new complements might be present after process_prep_WDT_from_verb()

	to_return = process_HOWTO(to_return, tags, connections, is_question);
	to_return = process_specification(to_return, tags, names);
	to_return = donkey_phrases(to_return, tags, names);
	to_return = add_nouns_to_quantities(to_return, tags, connections);
	to_return = add_nouns_to_quantifiers(to_return, tags, connections);

	if (debug) {
		puts("DELE1::: ");
		print_vector(to_return);
	}
	to_return = add_nouns_to_comparatives(to_return, tags, connections);
	if (debug) {
		puts("DELE2::: ");
		print_vector(to_return);
	}

	to_return = process_quantities(to_return, tags, connections);
	to_return = process_quantifiers(to_return, tags, connections);
	to_return = process_comparatives(to_return, tags, connections);
	to_return = process_somethings(to_return, connections);
	to_return = process_declaratives(to_return, tags, names);
	if (debug) {
		puts("AFTER_COMP::: ");
		print_vector(to_return);
	}

	to_return = process_LBR(to_return, tags, names, connections);
	to_return = save_name_conjunctions(to_return, tags, names);
	to_return = process_complements_from_names_and_adverbs(to_return, tags, names);
	to_return = process_allocutions(to_return, tags, names);
	if (debug) {
		puts("AFTER_ALLOC::: ");
		print_vector(to_return);
	}
	to_return = process_switched_allocution(to_return, tags, names);
	to_return = process_lonely_verbatim(to_return, tags, names);
	to_return = resolve_composite_comparatives(to_return, tags, names, connections);
	to_return = process_age(to_return, tags, names);
	to_return = process_times_RB(to_return, tags, names);
	if (debug) {
		puts("AFTER_COMP2::: ");
		print_vector(to_return);
	}
	to_return = process_subordinate_verbs(to_return, tags, names);
	if (debug) {
		cout << "SUB:::" << endl;
		print_vector(to_return);
	}
	to_return = last_touch_WRB(to_return, tags, names);
	to_return = last_touch_commas(to_return, tags, names);

	if (debug) {
		cout << "LTWP0" << endl;
		print_vector(to_return);
	}

	to_return = last_touch_conjunctions(to_return, tags, names);
	to_return = last_touch_references(to_return, tags, names);

	to_return = process_indirect_allocution(to_return, tags, names, connections);
	if (debug) {
		cout << "LTWP1" << endl;
		print_vector(to_return);
	}

	to_return = last_touch_WP(to_return, tags, names, connections, is_question);
	if (debug) {
		print_vector(to_return);
	}
	to_return = process_PAR(to_return, tags, names, connections, is_question);
	to_return = last_touch_PAR(to_return, tags, names, connections, is_question);
	to_return = last_touch_time(to_return, tags, names);

	to_return = sign_duplicates(to_return);
	if (debug) {
		print_vector(to_return);
	}
	if (debug) {
		cout << "LTWP2" << endl;
		print_vector(to_return);
	}
	to_return = meld_competing_elements(to_return, tags, names, connections);
	if (debug) {
		cout << "LTWP3" << endl;
		print_vector(to_return);
	}
	to_return = process_implicit_motion_to(to_return, tags, names);
	if (debug) {
		cout << "LTWP4" << endl;
		print_vector(to_return);
	}
	to_return = process_adverbial_place_at(to_return, tags, names);
	to_return = process_adverbial_time_at(to_return, tags, names);
	to_return = process_time_between(to_return, tags, names);
	to_return = process_additional_auxiliary_verb(to_return, tags, names); // get, keep, ...
	to_return = process_conditional_subordinate(to_return, tags, names);
	//to_return = process_live_for(to_return, tags, names,is_question);
	to_return = process_time_duration(to_return, tags, names,is_question);


	to_return = last_touch_MOTION_TO(to_return, tags, names);

	if (contain_question(to_return)) {
		phrase_->set_question();
		if (debug) {
			puts("WH_BEFORE:::");
			print_vector(to_return);
		}
		to_return = process_question_WH_preds(to_return, tags, names, connections);
		if (debug) {
			puts("WH_AFTER:::");
			print_vector(to_return);
		}
		to_return = eliminate_redundant_subjects(to_return, tags, names);
		to_return = eliminate_redundant_objects(to_return, tags, names);
		if (debug) {
			print_vector(to_return);
			puts("LOOSE:::");
		}
		to_return = loose_complements(to_return, tags, names);
		if (debug) {
			print_vector(to_return);
			puts("LOOSE2:::");
		}
		to_return = percolate_object_to_preposition(to_return, tags, names, connections);
		if (debug) {
			print_vector(to_return);
			puts("LOOSE3:::");
		}
		to_return = process_question_WRB_preds(to_return, tags, names, connections);
		if (debug) {
			puts("ADJUST_WRB:::");
			print_vector(to_return);
		}
		to_return = adjust_WRB(to_return, tags);
		//to_return= process_questions_with_final_IN(to_return, tags)
		// some preposition were not associated to the correct name. They should be now.
		to_return = name_complements(to_return, tags, names, is_question);
		// Add some ambiguity to the complements
		to_return = last_touch_complements_in_questions(to_return, tags, names);
		to_return = percolate_WP_object_to_subordinates(to_return, tags, names);
		to_return = eliminate_redundant_objects(to_return, tags, names);
	}
	if (contain_condition(to_return)) {
		phrase_->set_condition();
	}
	if (!contain_question(to_return)) {
		to_return = adjust_WRB(to_return, tags);
	}

	to_return = duplicate_WRB(to_return, tags, names, connections);
	if (debug) {
		puts("COMPETING:::");
		print_vector(to_return);
	}
	to_return = choose_competing_elements(to_return, tags, names, connections);
	to_return = translate_somethings(to_return, tags);
	to_return = process_tough_move_drs(to_return, tags, names);
	to_return = process_able_move_drs(to_return, tags, names);
	if (debug) {
		cout << "CORRECTOR::" << endl;
	}
	to_return = corrector(to_return, tags, names, connections, is_question);
	if (debug) {
		cout << "AFTER_CORRECTOR::" << endl;
		print_vector(to_return);
	}
	to_return = choose_competing_elements(to_return, tags, names, connections);
	if (debug) {
		print_vector(to_return);
		cout << "MIRROR::" << endl;
	}
	to_return = delete_mirror_elements(to_return, tags, names, connections);
	to_return = change_money_symbols(to_return, tags, names);
	DrtVect originals = restore_originals(to_return, tags, names); // Before deleting elements

	to_return = break_composed_nouns(to_return);
	to_return = join_composed_nouns(to_return);
	if (debug) {
		puts("PRESUPP_GENITIVE:::");
		print_vector(to_return);
	}
	if (!is_question) // an additional genitive does not couple with sentences in question space
		to_return = add_presupposition_elements(to_return, tags, names);
	if (debug) {
		puts("PRESUPP_GENITIVE2:::");
		print_vector(to_return);
	}
	to_return = invert_genitive_for_CD(to_return, tags, names);
	to_return = invert_genitive_for_quantifiers(to_return, tags, names);
	to_return = RNR_for_adverbs(to_return, tags, names, is_question);

	// the likelikess is given by the sense that the drt makes as it is
	clock_t start;
	if (debug || measure_time)
		start = clock();

	DrtPred error;
	phrase_->set_likeliness(compute_likeliness(to_return, tags, phrase_->getInfo()->getSense(), &error, is_question));

	if (debug||measure_time) {
		clock_t end = clock();
		cout << "Mtime_sense::: " << (end - start) / (double) CLOCKS_PER_SEC << endl;
	}


	if (is_question) {
		to_return = disentangle_subordinate_subjects(to_return, tags, names);
	}
	// The following ones must be done after computing likeliness
	to_return = percolate_no_DT_to_verb(to_return, tags, names);
	to_return = transform_all_verbs_to_vb(to_return);
	to_return = set_all_anaphora_levels(to_return);
	to_return = delete_irrelevant(to_return);

	if (debug) {
		cout << ">>>>" << endl;
		print_vector(to_return);
	}

	return boost::make_tuple(to_return, originals, error);

}

drt_builder::drt_builder(phrase * phrase) :
		phrase_(phrase)
{
}
