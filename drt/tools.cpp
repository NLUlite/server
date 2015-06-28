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



#include"tools.hpp"

static const bool debug = false;

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


double max(double a, double b)
{
	return a > b ? a : b;
}
double min(double a, double b)
{
	return a < b ? a : b;
}

string get_local_what()
// list of meanings for "what/WP"
{
	//return "!person|place|time|mountain|support|moon|planet|name|life|animal|hero|country|player|league|team|profession|playmaker|goalscorer|officer|medal|price|thing|love|event|happening|material|metal|road|place|company|junction|terminus|vehicle|[NNP]|thing";
	return "[what]";
}

bool verb_supports_indirect_obj(const string &name)
{
	vector<string> ind_verbs;
	ind_verbs.push_back("give");
	ind_verbs.push_back("wish");
	ind_verbs.push_back("make");
	ind_verbs.push_back("make_it");
	ind_verbs.push_back("find");
	ind_verbs.push_back("buy");
	ind_verbs.push_back("pay");
	ind_verbs.push_back("purchase");
	ind_verbs.push_back("show");
	ind_verbs.push_back("call");
	ind_verbs.push_back("tell");
	ind_verbs.push_back("sell");
	ind_verbs.push_back("regard_as");
	ind_verbs.push_back("set");
	ind_verbs.push_back("name");
	ind_verbs.push_back("promise");
	ind_verbs.push_back("consider");
	ind_verbs.push_back("build");
	ind_verbs.push_back("bring");
	ind_verbs.push_back("pass");
	ind_verbs.push_back("cost");


	if (find(ind_verbs.begin(), ind_verbs.end(), name) != ind_verbs.end())
		return true;

	return false;
}


vector<string> get_communication_verbs()
{
	vector<string> communication_verbs;

	communication_verbs.push_back("say");
	communication_verbs.push_back("deny");
	communication_verbs.push_back("argue");
	//communication_verbs.push_back("speak"); // "speak" does not introduce an allocution
	communication_verbs.push_back("write");
	communication_verbs.push_back("think");
	communication_verbs.push_back("tell");
	communication_verbs.push_back("hope");
	communication_verbs.push_back("report");
	communication_verbs.push_back("predict");

	return communication_verbs;
}

bool is_transitive(const string &str)
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

bool is_motion_verb_without_automatic_motion_to(const string &str)
{
	vector<string> candidates; // intransitive verbs
	candidates.push_back("reach");
	candidates.push_back("meet");
	candidates.push_back("flick");
	candidates.push_back("tour");
	candidates.push_back("encounter");

	if (find(candidates.begin(), candidates.end(), str) != candidates.end())
		return true;
	return false;
}


bool is_day_of_the_week(const string &str)
{
	vector<string> candidates; // intransitive verbs
	candidates.push_back("monday");
	candidates.push_back("tuesday");
	candidates.push_back("wednesday");
	candidates.push_back("thursday");
	candidates.push_back("friday");
	candidates.push_back("saturday");
	candidates.push_back("sunday");

	if (find(candidates.begin(), candidates.end(), str) != candidates.end())
		return true;
	return false;
}

void switch_children(DrtPred &pred)
{
	string fstr = extract_first_tag(pred);
	string sstr = extract_second_tag(pred);
	string str_tmp;
	str_tmp = fstr;
	fstr = sstr;
	sstr = str_tmp;
	implant_first(pred, fstr);
	implant_second(pred, sstr);
}

void switch_subj_obj(DrtPred &pred)
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

DrtVect create_end_from_subject(DrtVect drtvect, int vpos)
{
	if (!drtvect.at(vpos).is_verb())
		return drtvect;
	string verb_ref = extract_first_tag(drtvect.at(vpos));
	string subj_ref = extract_subject(drtvect.at(vpos));
	implant_subject(drtvect.at(vpos), "none");

	DrtPred end_pred(string("@DATIVE(") + verb_ref + "," + subj_ref + ")");
	drtvect.push_back(end_pred);

	return drtvect;
}

bool is_AUX(const DrtPred &pred)
{
	string head = extract_header(pred);

	vector<string> candidates;
	candidates.push_back("AUX");
	candidates.push_back("PASSIVE_AUX");

	if (find(candidates.begin(), candidates.end(), head) != candidates.end())
		return true;
	return false;
}

bool is_delete(const DrtPred &pred)
{
	string head = extract_header(pred);

	if (head.find(":DELETE") != string::npos)
		return true;
	return false;
}

bool is_CC(const DrtPred &pred)
{
	string head = extract_header(pred);

	vector<string> candidates;
	candidates.push_back("and");
	candidates.push_back("or");
	candidates.push_back("but");

	if (find(candidates.begin(), candidates.end(), head) != candidates.end())
		return true;
	return false;
}

bool is_punctuation(const DrtPred &pred)
{
	string tag = pred.tag();
	if (tag.find("-period-") != string::npos)
		return true;
	return false;
}


bool is_composed_prep(const DrtPred &pred)
{
	string head = extract_header(pred);
	if (head.find("[place_prep]") != string::npos)
		return true;
	return false;
}

bool is_passive(const DrtPred &pred)
{
	string head = extract_header(pred);
	if (head.find("PASSIVE") != string::npos)
		return true;
	return false;
}

bool points_to_name(const DrtPred &pred)
// Returns false if the predicate points to a reference (the man) or verb. It is
// used to avoid a percolation to the verb in phrases like "the man of
// steel is superman": Steel is referred to man, not to was.
{
	string ftag = extract_first_tag(pred);
	if (ftag.find("name") != string::npos)
		return true;
	return false;
}

bool points_to_WDT(const DrtVect &pre_drt, int pos)
// Returns true if the predicate points to a reference WDT or WP
{
	string stag = extract_second_tag(pre_drt.at(pos));
	for (int n = 0; n < pre_drt.size(); ++n) {
		string ref = extract_first_tag(pre_drt.at(n));
		string tag = pre_drt.at(n).tag();
		string head = extract_header(pre_drt.at(n));
		if (ref == stag && (tag == "WDT" || tag == "WP") && head.find(":DELETE") == string::npos)
			return true;
	}
	return false;
}

bool points_to_name_or_ref(const DrtPred &pred)
// Returns true if the predicate points to a reference (e.g: "the man").
{
	string ftag = extract_first_tag(pred);
	if (ftag.find("ref") != string::npos || ftag.find("name") != string::npos)
		return true;
	return false;
}

bool points_to_ref(const DrtPred &pred)
// Returns true if the predicate points to a reference (e.g: "the man").
{
	string ftag = extract_first_tag(pred);
	if (ftag.find("ref") != string::npos)
		return true;
	return false;
}

bool points_to_verb(const DrtPred &pred)
// Returns true if the predicate points to a verb
{
	string ftag = extract_first_tag(pred);
	if (ftag.find("verb") != string::npos)
		return true;
	return false;
}

bool is_verbatim(const DrtPred &pred)
// Returns true if the predicate points to a verb
{
	string head = extract_header(pred);
	if (head.find("[verbatim]") != string::npos)
		return true;
	return false;
}

bool is_date(const string &str)
{
	if (str.find("[date]_") != string::npos)
		return true;
	return false;
}

bool is_clock(const string &str)
{
	if (str.find("[clock]_") != string::npos)
		return true;
	return false;
}

bool string_is_subord(const string &str)
{
	if (str.find("[subord]_") != string::npos)
		return true;
	return false;
}

bool is_place(const string &str)
{
	if (str.find("[place]_") != string::npos)
		return true;
	return false;
}

bool is_verbatim(const string &str)
{
	if (str.find("[verbatim]_") != string::npos)
		return true;
	return false;
}

bool is_article(const string &str)
{
	if (str == "DT" || str == "PDT")
		return true;
	return false;
}

bool is_valid_article(const string &str)
// some DT can be associated to prepositions
{
	if (str == "this" || str == "that" || str == "these" || str == "those" || str == "all" || str == "none" || str == "each"
	    || str == "both")
		return true;
	return false;
}

bool is_than(const string &str)
// some DT can be associated to prepositions
{
	if (str == "than" || str == "-than")
		return true;
	return false;
}

bool is_modal(const string &str)
{
	if (str == "MD")
		return true;
	return false;
}

bool is_POS(const string &str)
{
	if (str == "POS")
		return true;
	return false;
}

bool is_comma(const string &str)
{
	if (str == "-comma-")
		return true;
	return false;
}

bool is_parenthesis(const string &str)
{
	if (str == "-LBR-" || str == "-RBR-" || str == "\"")
		return true;
	return false;
}

bool is_numeral(const string &str)
{
	if (str == "CD")
		return true;
	return false;
}

bool is_verb(const string &str)
{
	if (is_modal(str))
		return true;
	if (str == "AUX" || str == "VBP" || str == "VBZ" || str == "VBD" || str == "VB" || str == "VBN" || str == "VBG")
		return true;
	return false;
}

bool is_conj(const string &str)
{
	if (str == "CC" || str == "-comma-")
		return true;
	return false;
}

bool is_PRP(const string &str)
{
	if (str == "PRP")
		return true;
	return false;
}

bool is_name(const string &str)
{
	if (str == "NN" || str == "NNS" || str == "NNP" || str == "NNPS" || str == "PRP" || str == "JJ" || str == "JJS"
	    || str == "CD" || str == "$" || str == "UH")
		return true;
	return false;
}

bool is_adverb(const string &str)
{
	if (str == "RB" || str == "RP" || str == "RBR")
		return true;
	return false;
}

bool is_adjective(const string &str)
{
	if (str == "JJ" || str == "JJR" || str == "JJS" //|| str== "CD"
	    || str == "PRP$" || str == "VBJ")
		return true;
	return false;
}

bool is_preposition(const string &str)
{
	if (str == "IN" || str == "TO" || str == "OF" || str == "POS"
	    || str == "CC" // this is not strictly speaking a preposition, but the percolation demands a CC to be in this list
	    || str == "PREP" || str == "\"" || str == "-LBR-")
		return true;
	return false;
}

bool is_WDT(const string &str)
{
	if (str == "WDT")
		return true;
	return false;
}

bool is_WRB(const string &str)
{
	if (str == "WRB")
		return true;
	return false;
}

bool is_WP(const string &str)
{
	if (str == "WP" || str == "!WP")
		return true;
	return false;
}

bool is_WP_pos(const string &str)
{
	if (str == "WP$")
		return true;
	return false;
}

int find_prep_with_target(vector<DrtPred> &pre_drt, const vector<string> &tags, string from_str)
{
	for (int n = 0; n < tags.size(); ++n) {
		string stag = extract_second_tag(pre_drt.at(n));
		if (is_preposition(tags.at(n)) && !is_parenthesis(tags.at(n)) // the percolation MUST be inside a PRN
		    && stag == from_str) {
			return n;
		}
	}
	return -1;
}

int find_verb_with_subject(vector<DrtPred> &pre_drt, const vector<string> &tags, string from_str)
{
	for (int n = 0; n < tags.size(); ++n) {
		string subj = extract_subject(pre_drt.at(n));
		if (is_verb(tags.at(n)) && subj == from_str) {
			return n;
		}
	}
	return -1;
}

int find_verb_with_object(vector<DrtPred> &pre_drt, const vector<string> &tags, string from_str)
{
	for (int n = 0; n < tags.size(); ++n) {
		string obj = extract_object(pre_drt.at(n));
		if (is_verb(tags.at(n)) && obj == from_str) {
			return n;
		}
	}
	return -1;
}

int find_conj_with_first_tag(vector<DrtPred> &pre_drt, const vector<string> &tags, string from_str, string head)
{
	for (int n = 0; n < pre_drt.size(); ++n) {
		string head_tmp = extract_header(pre_drt.at(n));
		if (head != "" && head != head_tmp)
			continue;
		string fref = extract_first_tag(pre_drt.at(n));
		if (pre_drt.at(n).is_complement() && fref == from_str) {
			return n;
		}
	}
	return -1;
}

vector<int> find_all_compl_with_first_tag(vector<DrtPred> &pre_drt, const vector<string> &tags, string from_str, string head )
{
	vector<int> to_return;
	for (int n = 0; n < pre_drt.size(); ++n) {
		if (!pre_drt.at(n).is_complement())
			continue;
		string head_tmp = extract_header(pre_drt.at(n));
		if (head != "" && head_tmp.find(head) == string::npos )
			continue;
		string ftag = extract_first_tag(pre_drt.at(n));
		if (ftag == from_str)
			to_return.push_back(n);
	}
	return to_return;
}

vector<int> find_all_compl_with_first_tag_no_delete(vector<DrtPred> &pre_drt, const vector<string> &tags, string from_str, string head )
{
	vector<int> to_return;
	for (int n = 0; n < pre_drt.size(); ++n) {
		if (!pre_drt.at(n).is_complement())
			continue;
		string head_tmp = extract_header(pre_drt.at(n));
		if(head_tmp.find(":DELETE") != -1)
			continue;
		if (head != "" && head_tmp.find(head) == string::npos )
			continue;
		string ftag = extract_first_tag(pre_drt.at(n));
		if (ftag == from_str)
			to_return.push_back(n);
	}
	return to_return;
}


bool is_specification_end(const vector<DrtPred> pre_drt, int pos)
// name1(A), @COMPL(A,B), name2(B); B is at the end of a specification
{
     string pos_ref= extract_second_tag(pre_drt.at(pos));

     for(int n=0; n < pre_drt.size(); ++n) {
		if(pre_drt.at(n).is_complement()
		   || is_preposition(pre_drt.at(n).tag())
		   ) {
			string fref = extract_first_tag(pre_drt.at(n));
			string sref = extract_second_tag(pre_drt.at(n));
			if(sref == pos_ref && ref_is_name(fref)) {
				return true;
			}
		}
     }
     return false;
}

bool is_specification_of_NNP(const vector<DrtPred> pre_drt, int pos)
// name1(A), @COMPL(A,B), name2(B); B is at the end of a specification
{
     string pos_ref= extract_second_tag(pre_drt.at(pos));

     for(int n=0; n < pre_drt.size(); ++n) {
		if(pre_drt.at(n).is_complement()
		   || is_preposition(pre_drt.at(n).tag())
		   ) {
			string fref = extract_first_tag(pre_drt.at(n));
			string sref = extract_second_tag(pre_drt.at(n));
			if(sref == pos_ref && ref_is_name(fref)) {
				int m= find_name_with_string(pre_drt,fref);
				if(m == -1)
					return false; // it is not a specification
				if( pre_drt.at(m).is_proper_name() )
					return true;
				return false;
			}
		}
     }
     return false;
}


vector<int> find_all_compl_with_second_tag(vector<DrtPred> &pre_drt, const vector<string> &tags, string from_str, string head)
{
	vector<int> to_return;
	for (int n = 0; n < pre_drt.size(); ++n) {
		if (!pre_drt.at(n).is_complement())
			continue;
		string head_tmp = extract_header(pre_drt.at(n));
		if (head != "" && head != head_tmp)
			continue;
		string stag = extract_second_tag(pre_drt.at(n));
		if (stag == from_str)
			to_return.push_back(n);
	}
	return to_return;
}

vector<int> find_all_compl_with_first_and_second_tag(vector<DrtPred> &pre_drt, const vector<string> &tags, string first_tag, string second_tag)
{
	vector<int> to_return;
	for (int n = 0; n < pre_drt.size(); ++n) {
		if (!pre_drt.at(n).is_complement())
			continue;
		string ftag = extract_first_tag(pre_drt.at(n));
		string stag = extract_second_tag(pre_drt.at(n));
		if (ftag == first_tag && stag == second_tag)
			to_return.push_back(n);
	}
	return to_return;
}

vector<int> find_all_prep_with_second_tag(vector<DrtPred> &pre_drt, const vector<string> &tags, string from_str, string head)
{
	vector<int> to_return;
	for (int n = 0; n < pre_drt.size(); ++n) {
		if (!pre_drt.at(n).is_preposition())
			continue;
		string head_tmp = extract_header(pre_drt.at(n));
		if (head != "" && head != head_tmp)
			continue;
		string stag = extract_second_tag(pre_drt.at(n));
		if (stag == from_str)
			to_return.push_back(n);
	}
	return to_return;
}

vector<int> find_complement_with_name(const vector<DrtPred> &pre_drt, const string &name)
{
	vector<int> to_return;
	for (int n = 0; n < pre_drt.size(); ++n) {
		if (!pre_drt.at(n).is_complement())
			continue;
		string name_tmp = pre_drt.at(n).name();
		if (name == name_tmp)
			to_return.push_back(n);
	}
	return to_return;
}

int find_complement_with_first_tag(const vector<DrtPred> &pre_drt, string ref, const string &head)
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


int find_complement_with_target(const vector<DrtPred> &pre_drt, string ref, const string &head)
{
	for (int n = 0; n < pre_drt.size(); ++n) {
		string head_tmp = extract_header(pre_drt.at(n));
		if (head != "" && head != head_tmp)
			continue;
		string sref = extract_second_tag(pre_drt.at(n));
		if (pre_drt.at(n).is_complement() && sref == ref) {
			return n;
		}
	}
	return -1;
}

int find_complement_with_first_and_second_tag(const vector<DrtPred> &pre_drt, string fref, string sref, const string &head)
{
	vector<string> strs;
	boost::split(strs,head,boost::is_any_of("|"));
	for (int n = 0; n < pre_drt.size(); ++n) {
		string head_tmp = extract_header(pre_drt.at(n));
		if (head != "" && !shortfind(strs,head_tmp) )
			continue;
		string fref2 = extract_first_tag(pre_drt.at(n));
		string sref2 = extract_second_tag(pre_drt.at(n));
		if (pre_drt.at(n).is_complement() && fref == fref2 && sref == sref2) {

			return n;
		}
	}
	return -1;
}



bool first_tag_is_incomplete(const DrtPred &pred)
{
	string str = extract_first_tag(pred);
	if (str.find("from") != string::npos || str.find("prev") != string::npos)
		return true;
	return false;
}

string get_first_verb_ref(const DrtVect &drtvect)
{
	string to_return = "";
	DrtVect::const_iterator diter = drtvect.begin(), dend = drtvect.end();
	for (; diter != dend; ++diter) {
		if (diter->is_verb()) {
			to_return = extract_first_tag(*diter);
			break;
		}
	}
	return to_return;
}

string get_last_verb_ref(const DrtVect &drtvect)
{
	string to_return = "";
	DrtVect::const_iterator diter = drtvect.begin(), dend = drtvect.end();
	for (; diter != dend; ++diter) {
		if (diter->is_verb()) {
			to_return = extract_first_tag(*diter);
		}
	}
	return to_return;
}

string get_first_ref_with_tag(const DrtVect &drtvect, const string &tag)
{
	string to_return = "";
	DrtVect::const_iterator diter = drtvect.begin(), dend = drtvect.end();
	for (; diter != dend; ++diter) {
		if (diter->tag() == tag) {
			to_return = extract_first_tag(*diter);
			break;
		}
	}
	return to_return;
}

pair<string, int> get_first_ref_and_pos_with_tag(const DrtVect &drtvect, const string &tag)
{
	string to_return = "";
	DrtVect::const_iterator diter = drtvect.begin(), dend = drtvect.end();
	int n = 0;
	for (; diter != dend; ++diter, ++n) {
		if (diter->tag() == tag) {
			to_return = extract_first_tag(*diter);
			break;
		}
	}
	return make_pair(to_return, n);
}

bool is_unit_of_measure(const string &str)
{
	metric *d = metric_singleton::get_metric_instance();
	if (d->hypernym_dist(str, "metric_linear_unit", 3) > 0.1) {
		return true;
	}
	return false;
}

bool is_quantifier_name(const string &str)
{
	if (str == "some" || str == "many" || str == "all" || str == "most")
		return true;

	return false;
}

bool is_auxiliary_name(const string &str)
{
	if (str == "be" || str == "have" || str == "do")
		return true;

	return false;
}

int find_prep_with_first_tag(vector<DrtPred> &pre_drt, const vector<string> &tags, string from_str, string header)
{
	for (int n = 0; n < tags.size(); ++n) {
		string fref = extract_first_tag(pre_drt.at(n));
		string hstr = extract_header(pre_drt.at(n));
		if (header == "") 
			header = hstr;
		if (is_preposition(tags.at(n)) && !is_parenthesis(tags.at(n)) // the percolation MUST be inside a PRN
		    && hstr == header
		    && fref == from_str) {
			return n;
		}
	}
	return -1;
}


bool is_lonely_verb(vector<DrtPred> pre_drt, int pos)
{
	int size = pre_drt.size();

	string verb_ref = extract_first_tag(pre_drt.at(pos));

	vector<int> element_pos = find_int_attached_to_verb(pre_drt, pos);
	vector<int> element_pos2 = find_all_element_with_second_string(pre_drt, verb_ref);
	if (element_pos2.size() == 0) {
		for (int n = 0; n < element_pos.size(); ++n) {
			string head = extract_header(pre_drt.at(element_pos.at(n)));
			if (element_pos.at(n) != pos && head != "@TIME" && head != "@MODAL" && head != "@AUXILIARY")
				return false;
		}
		return true;
	}

	return false;
}

bool is_lonely_name(vector<DrtPred> pre_drt, int pos)
{
	int size = pre_drt.size();

	string name_ref = extract_first_tag(pre_drt.at(pos));

	for (int n = 0; n < size; ++n) {
	     if( pre_drt.at(n).is_delete() )
			continue;
		if (n != pos && pre_drt.at(n).is_name()) {
			string tmp_ref = extract_first_tag(pre_drt.at(n));
			if (name_ref == tmp_ref)
				return false;
		}
		if (n != pos && pre_drt.at(n).is_complement()) {
			string sref = extract_second_tag(pre_drt.at(n));
			if (name_ref == sref)
				return false;
		}
		if (pre_drt.at(n).is_verb()) {
			string tmp_ref = extract_subject(pre_drt.at(n));
			if (name_ref == tmp_ref)
				return false;
			tmp_ref = extract_object(pre_drt.at(n));
			if (name_ref == tmp_ref)
				return false;
		}
	}

	return true;
}

bool is_lonely_WP(vector<DrtPred> pre_drt, int pos)
{
	int size = pre_drt.size();

	string name_ref = extract_first_tag(pre_drt.at(pos));

	for (int n = 0; n < size; ++n) {
	     if( pre_drt.at(n).is_delete() )
			continue;
		if (n != pos && pre_drt.at(n).is_WP()) {
			string tmp_ref = extract_first_tag(pre_drt.at(n));
			if (name_ref == tmp_ref)
				return false;
		}
		if (n != pos && pre_drt.at(n).is_complement()) {
			string sref = extract_second_tag(pre_drt.at(n));
			if (name_ref == sref)
				return false;
		}
		if (pre_drt.at(n).is_verb()) {
			string tmp_ref = extract_subject(pre_drt.at(n));
			if (name_ref == tmp_ref)
				return false;
			tmp_ref = extract_object(pre_drt.at(n));
			if (name_ref == tmp_ref)
				return false;
		}
	}

	return true;
}


bool is_lonely_name(vector<DrtPred> pre_drt, string name_ref)
{
	int size = pre_drt.size();

	int num = 0;
	for (int n = 0; n < size; ++n) {
	     if( pre_drt.at(n).is_delete() )
			continue;
		if (pre_drt.at(n).is_name()) {
			string tmp_ref = extract_first_tag(pre_drt.at(n));
			if (name_ref == tmp_ref)
				++num;
		}
		if (pre_drt.at(n).is_complement()) {
			string sref = extract_second_tag(pre_drt.at(n));
			if (name_ref == sref)
				++num;
		}
		if (pre_drt.at(n).is_verb()) {
			string tmp_ref = extract_subject(pre_drt.at(n));
			if (name_ref == tmp_ref)
				return false;
			tmp_ref = extract_object(pre_drt.at(n));
			if (name_ref == tmp_ref)
				++num;
		}
		if(num > 1)
		     return false;
	}

	return true;
}


bool is_lonely_name_no_verbs(vector<DrtPred> pre_drt, int pos)
{
	int size = pre_drt.size();

	string name_ref = extract_first_tag(pre_drt.at(pos));

	for (int n = 0; n < size; ++n) {
		if (n != pos && pre_drt.at(n).is_name()) {
			string tmp_ref = extract_first_tag(pre_drt.at(n));
			if (name_ref == tmp_ref)
				return false;
		}
		if (n != pos && pre_drt.at(n).is_complement()) {
			string tmp_ref = extract_first_tag(pre_drt.at(n));
			if (name_ref == tmp_ref)
				return false;
		}
	}

	return true;
}

bool is_lonely_adjective_no_verbs(vector<DrtPred> pre_drt, int pos)
{
	int size = pre_drt.size();

	string name_ref = extract_first_tag(pre_drt.at(pos));

	for (int n = 0; n < size; ++n) {
		if (n != pos && (pre_drt.at(n).is_name() || pre_drt.at(n).is_adjective())) {
			string tmp_ref = extract_first_tag(pre_drt.at(n));
			if (name_ref == tmp_ref)
				return false;
		}
	}

	return true;
}



bool has_first_tag(const DrtPred &pred)
{
	string tag = extract_first_tag(pred);
	if (tag.find("prev") == string::npos && tag.find("from") == string::npos
			&& tag.find("next") == string::npos && tag.find("to") == string::npos )
		return true;
	return false;
}

bool has_second_tag(const DrtPred &pred)
{
	string tag = extract_second_tag(pred);
	if (tag.find("next") == string::npos && tag.find("to") == string::npos
			&& tag.find("prev") == string::npos && tag.find("from") == string::npos )
		return true;
	return false;
}

bool has_subject(const DrtPred &pred)
{
	string subj = extract_subject(pred);
	if (subj.find("subj") == string::npos)
		return true;
	return false;
}

bool has_subject_nish(const DrtPred &pred)
// A nish subject has the precedence over a verb subject
{
	string subj = extract_subject(pred);
	if (subj.find("ref") != string::npos || subj.find("name") != string::npos)
		return true;
	return false;
}

bool has_object(const DrtPred &pred)
{
	string obj = extract_object(pred);
	if (debug)
		cout << "OBJ::: " << obj << endl;
	if (obj.find("obj") == string::npos && obj.find("none") == string::npos && obj.find("subj") == string::npos // The passive verbs invert the tags
	    )
		return true;
	return false;
}

bool has_complements(const DrtVect drtvect, int n)
{
	if (!drtvect.at(n).is_verb())
		return false;
	string vref = extract_first_tag(drtvect.at(n));
	vector<int> poz = find_all_element_with_string(drtvect, vref);
	for (int m = 0; m < poz.size(); ++m) {
		int j = poz.at(m);
		string header = extract_header(drtvect.at(j));
		if (drtvect.at(j).is_complement() && header != "@TIME" 
		    // && header != "@MODAL" // NO! "may be" is perfectly legitimate
		    )
			return true;
	}
	return false;
}

bool has_pure_complements(const DrtVect drtvect, int n)
{
	if (!drtvect.at(n).is_verb())
		return false;
	string vref = extract_first_tag(drtvect.at(n));
	vector<int> poz = find_all_element_with_string(drtvect, vref);
	for (int m = 0; m < poz.size(); ++m) {
		int j = poz.at(m);
		string header = extract_header(drtvect.at(j));
		if (drtvect.at(j).is_complement() && header != "@TIME"
		    && header != "@MODAL"
		    && !ref_is_verb( extract_second_tag(drtvect.at(j)) )
		    )
			return true;
	}
	return false;
}


bool verb_object(const DrtPred &pred)
{
	string subj = extract_object(pred);
	if (subj.find("verb") != string::npos)
		return true;
	return false;
}

bool verb_subject(const DrtPred &pred)
{
	string subj = extract_subject(pred);
	if (subj.find("verb") != string::npos)
		return true;
	return false;
}

bool tag_is_candidate_subject(const string &tag, const string &name)
{
	if (name == "the" || name == "a" || name == "an" || (tag == "DT" && name == "no")) /// bad solution
		return false;
	if (tag == "NN" || tag == "NNS" || tag == "NNP" || tag == "NNPS" || tag == "DT" || tag == "EX" || tag == "JJ" || tag == "JJS"
	    || tag == "JJR" || tag == "PRP" || tag == "PRP$" || tag == "MD" || tag == "WP" || tag == "WP$" || tag == "WDT"
	    // || tag == "AUX" || tag == "VBP" || tag == "VBD" || tag == "VBZ" || tag == "VB" /// VB shouldn't be here
	    || tag == "VBG" || tag == "VBN" || tag == "CD" || tag == "$")
		return true;

	return false;
}

bool tag_is_candidate_object(const string &tag, const string &name)
{
	if (name == "the" || name == "a" || name == "an" || (tag == "DT" && name == "no")) /// bad solution
		return false;
	if (tag == "NN" || tag == "NNS" || tag == "NNP" || tag == "NNPS" || tag == "DT" || tag == "JJ" || tag == "JJS"
	    || tag == "EX" /// BAD SOLUTION, this is to ensure that EX is assigned in questions
	    || tag == "JJR" || tag == "PRP" || tag == "PRP$" || tag == "WDT" || tag == "WP" || tag == "WP$" || tag == "VBN"
	    || tag == "VBD" || tag == "VB" || tag == "VBG" || tag == "CD" || tag == "$")
		return true;

	return false;
}

bool is_sbar_verb(const vector<DrtPred> &drt, int n, DrtPred *sbar_pred)
{
	string subord_ref = extract_first_tag(drt.at(n));
	vector<DrtPred>::const_iterator piter = drt.begin();
	vector<DrtPred>::const_iterator pend = drt.end();
	for (; piter != pend; ++piter) {
		//if(piter->pred().begin()->str == "@SUBORD") {
		string fref = extract_first_tag(*piter);
		string sref = extract_second_tag(*piter);
		if (piter->is_complement() && ref_is_verb(fref) && ref_is_verb(sref)) {
			string second_tag = extract_second_tag(*piter);
			if (second_tag == subord_ref) {
				*sbar_pred = *piter;
				return true;
			}
		}
	}
	return false;
}

bool is_generic(const string &str)
{
	if (str.find("[*]") != string::npos)
		return true;
	return false;
}

bool verb_is_singular(DrtPred pred, vector<DrtPred> speech)
// returns true if the verb in "pred" has a subject that supports a singular conjugation for the verb
{
	metric *d = metric_singleton::get_metric_instance();
	tagger *tagg = parser_singleton::get_tagger_instance();
	if (pred.is_verb()) {
		string subj_ref = extract_subject(pred);
		for (int n = 0; n < speech.size(); ++n) {
		     string header = extract_header(speech.at(n));
		     bool is_what = ( header == get_local_what() );
		     if(is_what) {
				string fref = extract_first_tag(speech.at(n));
				string tag = pred.tag();
				if (fref == subj_ref && tag == "VBP")
					return false;
				if (fref == subj_ref && tag == "VBZ")
					return true;
		     }
		     if (speech.at(n).is_name() && !speech.at(n).is_PRP() && !is_what) {
				string fref = extract_first_tag(speech.at(n));
				string tag = speech.at(n).tag();
				string lemma = tagg->get_info()->get_conj(header, "NNS");
				if (lemma == "")
					lemma = header;
				if (fref == subj_ref && (d->has_synset(lemma) || is_generic(header)) && speech.at(n).is_plural()) {
					return false;
				}
			}
			if (speech.at(n).is_WP() || speech.at(n).is_WDT()) { 
			     // A WP always agrees with the verb
				string fref = extract_first_tag(speech.at(n));
				string tag = pred.tag();
				if (fref == subj_ref && tag == "VBP")
					return false;
				if (fref == subj_ref && tag == "VBZ")
					return true;
			}
			if (speech.at(n).is_PRP()) {
				string fref = extract_first_tag(speech.at(n));
				string header = extract_header(speech.at(n));
				if (fref == subj_ref && (header == "he" || header == "she" || header == "it")) {
					return true;
				}
				if (fref == subj_ref && (header == "they" || header == "i" || header == "we" || header == "you" || header == "us")) {
					return false;
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

int num_elements_connected(const vector<DrtPred> &speech, const string ref)
// return the number of element connected to the reference by @AND or @OR
{
	metric *d = metric_singleton::get_metric_instance();
	int to_return = 0;
	for (int n = 0; n < speech.size(); ++n) {
		if (speech.at(n).is_name()) {
			string fref = extract_first_tag(speech.at(n));
			if (fref == ref)
				to_return = 1;
		}
		if (speech.at(n).is_complement()) {
			string fref = extract_first_tag(speech.at(n));
			string head = extract_header(speech.at(n));
			if (fref == ref && (head == "@AND" || head == "@OR")) {
				to_return = 2;
				break;
			}
		}
	}

	return to_return;
}

bool has_plural_ref(const vector<DrtPred> &speech, const string ref)
// return true if the element connected to the reference is plural
{
	metric *d = metric_singleton::get_metric_instance();
	for (int n = 0; n < speech.size(); ++n) {
		if (speech.at(n).is_name()) {
			string fref = extract_first_tag(speech.at(n));
			if (fref == ref && speech.at(n).is_plural())
				return true;
		}
	}

	return false;
}

vector<int> find_coupled_non_verb(vector<DrtPred> &pre_drt, const vector<string> &tags, int n)
{
	vector<int> vcoupled;
	string str1 = extract_first_tag(pre_drt.at(n));
	vcoupled = find_non_verb_with_string(pre_drt, tags, str1);

	return vcoupled;
}

string extract_verb_name(const DrtPred &verb)
{
	return extract_first_tag(verb);
}

DrtPred substitute_with(DrtPred to_modify, const DrtPred &old_target, const DrtPred &new_target)
{
	DrtPred original(to_modify);
	string old_subj = extract_first_tag(old_target);
	string new_subj = extract_first_tag(new_target);

	vector<string> children = to_modify.extract_children();
	for (int n = 0; n < children.size(); ++n) {
		if (children.at(n) == new_subj) { // bad substitution
			return original;
		}
		if (children.at(n) == old_subj) {
			children.at(n) = new_subj;
		}
	}
	to_modify.implant_children(children);
	return to_modify;
}

vector<DrtPred> substitute_ref(vector<DrtPred> &pre_drt, const string &from_str, const string &to_str)
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

vector<DrtPred> substitute_ref_with_name(vector<DrtPred> &pre_drt, const string &from_str, const string &to_str,
								 const string &name)
{
	vector<DrtPred> predicates(pre_drt);

	for (int n = 0; n < predicates.size(); ++n) {
		vector<string> children = predicates.at(n).extract_children();
		string head = extract_header(predicates.at(n));
		if (head != name)
			continue;
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

vector<DrtPred> substitute_ref_safe(vector<DrtPred> &pre_drt, const string &from_str, const string &to_str)
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

vector<DrtPred> substitute_ref_unsafe(vector<DrtPred> &pre_drt, const string &from_str, const string &to_str)
// returns the same pre_drt if there is a bad substitution, i.e., if there is an element which would point to itself
{
	vector<DrtPred> predicates(pre_drt);

	for (int n = 0; n < predicates.size(); ++n) {
		vector<string> children = predicates.at(n).extract_children();
		for (int m = 0; m < children.size(); ++m) {
			if (children.at(m) == from_str) {
				children.at(m) = to_str;
			}
		}
		predicates.at(n).implant_children(children);
	}
	return predicates;
}


int find_closest_WDT_name(vector<DrtPred> &pre_drt, int pos_WDT)
{
	int pos = pos_WDT;

	for (; pos >= 0; --pos) {
		if (pre_drt.at(pos).is_name()) { // take the first name before the WDT
			break;
		}
	}

	return pos;
}

int find_verb_with_string(const vector<DrtPred> &pre_drt, vector<string> tags, string str)
{
	int m = 0;

	vector<DrtPred>::const_iterator diter = pre_drt.begin();

	for (; diter != pre_drt.end() && m < tags.size(); ++diter, ++m) {
		string fref = extract_first_tag(*diter);
		if (fref == str && is_verb(tags.at(m)) && !is_AUX(extract_header(*diter))) {
			return m;
		}
	}
	if (m == tags.size())
		m = -1;
	return m;
}

vector<int> find_non_verb_with_string(const vector<DrtPred> &pre_drt, vector<string> tags, string str)
{
	vector<int> ret_m;
	int m = 0;

	vector<DrtPred>::const_iterator diter = pre_drt.begin();

	for (; diter != pre_drt.end() && m < tags.size(); ++diter, ++m) {
		string fref = extract_first_tag(*diter);
		if (fref == str && !is_verb(tags.at(m))) {
			ret_m.push_back(m);
		}
	}
	return ret_m;
}

bool there_is_conjunction_between(vector<DrtPred> &pre_drt, int m, int n)
{
	int start, end;

	if (m < n) {
		start = m;
		end = n;
	} else {
		start = n;
		end = m;
	}
	for (int j = start; j <= end; ++j) {
		if (pre_drt.at(j).is_conj())
			return true;
	}
	return false;
}

int find_coupled_verb(vector<DrtPred> &pre_drt, const vector<string> &tags, int n)
{
	int ncoupled = -1;
	string str_subj = extract_subject(pre_drt.at(n));
	string str_obj = extract_object(pre_drt.at(n));

	if (str_obj.find("verb") != std::string::npos) { // if the object of the verb at n is a verb
		ncoupled = find_verb_with_string(pre_drt, tags, str_obj);
	} else if (str_subj.find("verb") != std::string::npos) { // if the subject of the verb at n is a verb
		ncoupled = find_verb_with_string(pre_drt, tags, str_subj);
		if (there_is_conjunction_between(pre_drt, ncoupled, n))
			ncoupled = -1;
	}

	return ncoupled;
}

bool close_loop_str(const DrtVect &to_return, const string &fref, const string &sref)
{
	for (int n = 0; n < to_return.size(); ++n) {
		if (to_return.at(n).is_verb()) {
			string subj = extract_subject(to_return.at(n));
			string obj = extract_object(to_return.at(n));
			if (subj == fref && obj == sref)
				return true;
			int m = find_verb_with_string(to_return, obj);
			if (m != -1) {
				string verb_str = extract_first_tag(to_return.at(m));
				if (fref == verb_str)
					return true;
			}
		}
		if (to_return.at(n).is_complement() || to_return.at(n).is_preposition() || to_return.at(n).is_POS()
				|| is_CC(to_return.at(n))) {
			string r1 = extract_first_tag(to_return.at(n));
			string r2 = extract_second_tag(to_return.at(n));
			if (r1 == fref && r2 == sref)
				return true;
		}
	}

	return false;
}

bool close_loop(const DrtVect &to_return, int pos1, int pos2)
{
	string fref = extract_first_tag(to_return.at(pos1));
	string sref = extract_first_tag(to_return.at(pos2));
	for (int n = 0; n < to_return.size(); ++n) {
		if (to_return.at(n).is_verb()) {
			string subj = extract_subject(to_return.at(n));
			string obj = extract_object(to_return.at(n));
			if (subj == fref && obj == sref)
				return true;
		}
		if (to_return.at(n).is_complement() || to_return.at(n).is_preposition() || to_return.at(n).is_POS()
				|| is_CC(to_return.at(n))) {
			string r1 = extract_first_tag(to_return.at(n));
			string r2 = extract_second_tag(to_return.at(n));
			if (r1 == fref && r2 == sref)
				return true;
		}
	}

	return false;
}
