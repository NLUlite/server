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



#ifndef __DRT_TOOLS__
#define __DRT_TOOLS__

#include<iostream>
#include<algorithm>
#include<vector>
#include<cmath>
#include<map>
#include<string>
#include<fstream>
#include<boost/tuple/tuple.hpp>
#include<boost/utility.hpp>
#include<boost/algorithm/string.hpp>
#include<boost/lexical_cast.hpp>
#include"../infero/infero.hpp"
#include"complements.hpp"
#include"DrtPred.hpp"
#include"metric.hpp"
#include"metric_singleton.hpp"
#include"constituents.hpp"
#include"parser_singleton.hpp"

using std::string;
using std::vector;
using std::map;
using boost::tuple;
using boost::make_tuple;
using std::pair;
using std::make_pair;
using std::cout;
using std::endl;

#include"tools.hpp"

double max(double a, double b);
double min(double a, double b);
string get_local_what();
vector<string> get_communication_verbs();
bool is_transitive(const string &str);
bool is_motion_verb_without_automatic_motion_to(const string &str);
bool is_day_of_the_week(const string &str);
void switch_children(DrtPred &pred);
void switch_subj_obj(DrtPred &pred);
DrtVect create_end_from_subject(DrtVect drtvect, int vpos);
bool is_AUX(const DrtPred &pred);
bool is_delete(const DrtPred &pred);
bool is_CC(const DrtPred &pred);
bool is_composed_prep(const DrtPred &pred);
bool is_passive(const DrtPred &pred);
bool points_to_name(const DrtPred &pred);
bool points_to_WDT(const DrtVect &pre_drt, int pos);
bool points_to_name_or_ref(const DrtPred &pred);
bool points_to_ref(const DrtPred &pred);
bool points_to_verb(const DrtPred &pred);
bool is_verbatim(const DrtPred &pred);
bool is_date(const string &str);
bool is_clock(const string &str);
bool string_is_subord(const string &str);
bool is_place(const string &str);
bool is_verbatim(const string &str);
bool is_article(const string &str);
bool is_valid_article(const string &str);
bool is_than(const string &str);
bool is_modal(const string &str);
bool is_POS(const string &str);
bool is_comma(const string &str);
bool is_parenthesis(const string &str);
bool is_numeral(const string &str);
bool is_verb(const string &str);
bool is_conj(const string &str);
bool is_PRP(const string &str);
bool is_name(const string &str);
bool is_adverb(const string &str);
bool is_adjective(const string &str);
bool is_preposition(const string &str);
bool is_WDT(const string &str);
bool is_WRB(const string &str);
bool is_WP(const string &str);
bool is_WP_pos(const string &str);
bool is_punctuation(const DrtPred &pred);
int find_prep_with_target(vector<DrtPred> &pre_drt, const vector<string> &tags, string from_str);
int find_verb_with_subject(vector<DrtPred> &pre_drt, const vector<string> &tags, string from_str);
int find_verb_with_object(vector<DrtPred> &pre_drt, const vector<string> &tags, string from_str);
int find_conj_with_first_tag(vector<DrtPred> &pre_drt, const vector<string> &tags, string from_str, string head = "");
vector<int> find_all_compl_with_first_tag(vector<DrtPred> &pre_drt, const vector<string> &tags, string from_str, string head = "");
vector<int> find_all_compl_with_first_tag_no_delete(vector<DrtPred> &pre_drt, const vector<string> &tags, string from_str, string head = "");
vector<int> find_all_compl_with_second_tag(vector<DrtPred> &pre_drt, const vector<string> &tags, string from_str, string head = "");
vector<int> find_all_compl_with_first_and_second_tag(vector<DrtPred> &pre_drt, const vector<string> &tags, string first_tag, string second_tag);
vector<int> find_all_prep_with_second_tag(vector<DrtPred> &pre_drt, const vector<string> &tags, string from_str, string head = "");
vector<int> find_complement_with_name(const vector<DrtPred> &pre_drt, const string &name);
int find_complement_with_first_and_second_tag(const vector<DrtPred> &pre_drt, string fref, string sref, const string &head = "");
int find_complement_with_first_tag(const vector<DrtPred> &pre_drt, string ref, const string &head = "");
int find_complement_with_target(const vector<DrtPred> &pre_drt, string ref, const string &head = "");
bool first_tag_is_incomplete(const DrtPred &pred);
string get_first_verb_ref(const DrtVect &drtvect);
string get_last_verb_ref(const DrtVect &drtvect);
string get_first_ref_with_tag(const DrtVect &drtvect, const string &tag);
pair<string, int> get_first_ref_and_pos_with_tag(const DrtVect &drtvect, const string &tag);
bool is_unit_of_measure(const string &str);
bool is_quantifier_name(const string &str);
bool is_auxiliary_name(const string &str);
int find_prep_with_first_tag(vector<DrtPred> &pre_drt, const vector<string> &tags, string from_str, string header = "");
bool is_lonely_verb(vector<DrtPred> pre_drt, int pos);
bool is_lonely_name(vector<DrtPred> pre_drt, int pos);
bool is_lonely_name(vector<DrtPred> pre_drt, string name_ref);
bool is_lonely_name_no_verbs(vector<DrtPred> pre_drt, int pos);
bool is_lonely_WP(vector<DrtPred> pre_drt, int pos);
bool is_lonely_adjective_no_verbs(vector<DrtPred> pre_drt, int pos);
bool is_specification_end(const vector<DrtPred> pre_drt, int pos);
bool is_specification_of_NNP(const vector<DrtPred> pre_drt, int pos);
bool has_first_tag(const DrtPred &pred);
bool has_second_tag(const DrtPred &pred);
bool has_subject(const DrtPred &pred);
bool has_subject_nish(const DrtPred &pred);
bool has_object(const DrtPred &pred);
bool has_complements(const DrtVect drtvect, int n);
bool has_pure_complements(const DrtVect drtvect, int n);
bool verb_object(const DrtPred &pred);
bool verb_subject(const DrtPred &pred);
bool tag_is_candidate_subject(const string &tag, const string &name);
bool tag_is_candidate_object(const string &tag, const string &name);
bool is_sbar_verb(const vector<DrtPred> &drt, int n, DrtPred *sbar_pred);
bool is_generic(const string &str);
bool verb_is_singular(DrtPred pred, vector<DrtPred> speech);
int num_elements_connected(const vector<DrtPred> &speech, const string ref);
bool has_plural_ref(const vector<DrtPred> &speech, const string ref);
vector<int> find_coupled_non_verb(vector<DrtPred> &pre_drt, const vector<string> &tags, int n);
string extract_verb_name(const DrtPred &verb);
DrtPred substitute_with(DrtPred to_modify, const DrtPred &old_target, const DrtPred &new_target);
vector<DrtPred> substitute_ref(vector<DrtPred> &pre_drt, const string &from_str, const string &to_str);
vector<DrtPred> substitute_ref_with_name(vector<DrtPred> &pre_drt, const string &from_str, const string &to_str, const string &name);
vector<DrtPred> substitute_ref_safe(vector<DrtPred> &pre_drt, const string &from_str, const string &to_str);
vector<DrtPred> substitute_ref_unsafe(vector<DrtPred> &pre_drt, const string &from_str, const string &to_str);
int find_closest_WDT_name(vector<DrtPred> &pre_drt, int pos_WDT);
int find_verb_with_string(const vector<DrtPred> &pre_drt, vector<string> tags, string str);
vector<int> find_non_verb_with_string(const vector<DrtPred> &pre_drt, vector<string> tags, string str);
bool there_is_conjunction_between(vector<DrtPred> &pre_drt, int m, int n);
int find_coupled_verb(vector<DrtPred> &pre_drt, const vector<string> &tags, int n);
bool close_loop_str(const DrtVect &to_return, const string &fref, const string &sref);
bool close_loop(const DrtVect &to_return, int pos1, int pos2);

bool verb_supports_indirect_obj(const string &name);

#endif // __DRT_TOOLS__
