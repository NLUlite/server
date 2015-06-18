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



#ifndef __DRTPRED__
#define __DRTPRED__

#include<iostream>
#include<fstream>
#include<boost/utility.hpp>
#include<algorithm>
#include<vector>
#include<boost/lexical_cast.hpp>
#include<boost/algorithm/string.hpp>
//#include<boost/algorithm/string/classification.hpp>

#include"../fopl/fopl.hpp"

using std::string;
using std::vector;
using std::pair;
using std::make_pair;
using std::cout;
using std::endl;


struct DrtMgu : vector<pair<string,string> > {
     void add(const string &in, const string &out) {push_back(make_pair(in, out));}
     void add(const DrtMgu &u);
     void addWithoutUnification(const DrtMgu &u);
     void uniVal(int num);
     void addReverse(const DrtMgu &u);
     void operator /(const DrtMgu &);
     bool operator <(const DrtMgu &) const;

     void print(std::ostream &out) const;
};

class DrtPred {

     string name_, tag_;
     string anaphora_level_;

     bool is_question_, is_intersection_;
     string question_word_;

     bool is_pivot_, is_anaphora_;
     
     double weigth;
     friend void operator / (std::vector<DrtPred> &predVect, const DrtMgu &upg);
     
     string header_;
     vector<string> children_;

public:
     DrtPred() { is_question_= false; is_pivot_= false; is_intersection_ = false; is_anaphora_ = false;}
     DrtPred(const std::string &s, const double w=1);
     explicit DrtPred(const PredTree &t, const double w=1);
     DrtPred(const Predicate &p, const double w=1);
     DrtPred(const DrtPred &p); // it has to be defined otherwise DrtPred(Predicate) is used as a constructor
     ~DrtPred() {}

     double getWeigth() const { return weigth; }
     void setWeigth(double w) { weigth= w; }

     string name() const { return name_; }
     string tag() const { return tag_; }
     void setName(const string &str) { name_= str; }
     void setTag(const string &str) { tag_= str; }

     void setAnaphoraLevel(const string &str) { anaphora_level_= str; }
     string anaphoraLevel() const { return anaphora_level_; }
     
     bool is_plural() const;
     bool is_verb() const;
     bool is_VBN() const;
     bool is_conj() const;
     bool is_date() const;
     bool is_place() const;
     bool is_verbatim() const;
     bool is_name() const;
     bool is_number() const;
     bool is_proper_name() const;
     bool is_parenthesis() const;
     bool is_complement() const;
     bool is_adverb() const;
     bool is_comma() const;
     bool is_adjective() const;
     bool is_preposition() const;
     bool is_delete() const;
     bool is_PRP() const;
     bool is_POS() const;
     bool is_WDT() const;
     bool is_WP() const;
     bool is_WP_pos() const;
     bool is_WRB() const;
     bool is_modal() const;
     bool is_generic() const;
     bool is_sentence() const;
     bool is_what() const;
     bool is_article() const;
     bool isQuantifier() const;

     bool is_question() const { return is_question_; }
     void set_question() { is_question_=true; }
     void set_question(bool b) { is_question_=b; }

     bool is_intersection() const { return is_intersection_; }
     void set_intersection() { is_intersection_=true; }

     bool is_anaphora() const { return is_anaphora_; }
     void set_anaphora() { is_anaphora_=true; }

     string get_question_word() const { return question_word_; }
     void set_question_word(const string &s) { question_word_= s; }

     void set_pivot(bool b=true) {is_pivot_= b;}
     bool is_pivot() const {return is_pivot_;}

     virtual bool equalAtoms(const PTEl &lhs, const PTEl &rhs) const;
     
     void print(std::ostream &out) const;

     string extract_header() const;
     string extract_child(int n) const;
     vector<string> extract_children() const;

     DrtPred implant_header(const string &str);
     DrtPred implant_child(const string &str, int n);
     DrtPred implant_children(const vector<string> &str);

     void operator / (const DrtMgu &);     
     bool operator == (const DrtPred &rhs) const;
     bool operator < (const DrtPred &rhs) const;


     bool unify(const DrtPred &rhs);
     bool unify(const DrtPred &rhs, DrtMgu *mgu) const;

     void uniVal(int num);
};


typedef vector<DrtPred> DrtVect;

void operator / (vector<DrtPred> &predVect, const DrtMgu &upg);
void operator / (std::vector<DrtVect> &predVect, const DrtMgu &upg);

std::ostream &operator << (std::ostream &, const DrtPred &);
std::ostream &operator << (std::ostream &, const DrtMgu &);


//class DrtVect : public vector<DrtPred>
//{
//public:
//     DrtVect() {}
//     DrtVect(const string &str);
//     DrtVect(const vector<DrtPred> &d) {*this= d;}
//     
//     DrtVect & operator / (const DrtMgu &u) {(*this)/u; return *this;}
//     DrtVect & operator / (const genDrtMgu &u) {(*this)/u; return *this;}
//};






int get_position_from_predicate(const std::vector<DrtPred> &, const DrtPred &pred);
vector<DrtPred> get_predicates_from_position(const vector<DrtPred> preds, const vector<int> &pos);
vector<int> find_int_attached_to_verb(const vector<DrtPred> &preds, int m);
vector<DrtPred> find_attached_to_verb(const vector<DrtPred> &pre_drt, int pos);
vector<DrtPred> find_all_attached_to_verb(vector<DrtPred> preds, int m);

int find_pivot_with_string(const vector<DrtPred> &pre_drt, string str);
vector<int> find_all_element_with_string(const vector<DrtPred> &pre_drt, string str);
vector<int> find_all_element_with_second_string(const vector<DrtPred> &pre_drt, string str);
vector<int> find_all_element_with_string_no_delete(const vector<DrtPred> &pre_drt, string str);
vector<int> find_all_element_with_string_everywhere(const vector<DrtPred> &pre_drt, string str);
vector<int> find_all_names_with_string_no_delete(const vector<DrtPred> &pre_drt, string str);
vector<int> find_all_joined_element_with_string(const vector<DrtPred> &pre_drt, string str);
vector<int> find_all_adverbs_with_string(const vector<DrtPred> &pre_drt, string str);

int find_element_with_string(const vector<DrtPred> &pre_drt, string str);
int find_name_with_string(const vector<DrtPred> &pre_drt, string str);
int find_pivot_name_with_string(const vector<DrtPred> &pre_drt, string str);
int find_verb_with_string(const vector<DrtPred> &pre_drt, string str);
vector<int> find_next_of_string(const vector<DrtPred> &pre_drt, const string &str);
vector<int> get_elements_next_of(const std::vector<DrtPred> &, int);
vector<DrtPred> get_elements_next_of(const std::vector<DrtPred> &, const DrtPred &pred);
vector<DrtPred> find_subordinates_of_verb(const vector<DrtPred> &preds, int m);


vector<string> find_string_object_of_verb(const vector<DrtPred> &preds, int m);
vector<string> find_string_subject_of_verb(const vector<DrtPred> &preds, int m);

vector<DrtPred> find_object_of_verb(const vector<DrtPred> &preds, int m);
vector<DrtPred> find_subject_of_verb(const vector<DrtPred> &preds, int m);
int find_int_subject_of_verb(const vector<DrtPred> &preds, int m);
vector<DrtPred> find_adverb_of_verb(const vector<DrtPred> &preds, int m);
vector<vector<DrtPred> > find_complements_of_verb(const vector<DrtPred> &preds, int m);
vector<pair<pair<string,string>, vector<string> > > find_string_complement_of_verb(const vector<DrtPred> &preds, int m);
DrtPred find_pointer_to_verb(const vector<DrtPred> &preds, int m);

string extract_subject(const DrtPred &name);
string extract_object(const DrtPred &name);

vector<string> extract_children(const DrtPred &pred);
string extract_header(const DrtPred &pred);
string extract_first_tag(const DrtPred &pred);
string extract_second_tag(const DrtPred &pred);
string extract_third_tag(const DrtPred &pred);

bool points_to_subject(const vector<DrtPred> &pre_drt, const DrtPred &pred);
bool points_to_object(const vector<DrtPred> &pre_drt, const DrtPred &pred);

DrtPred implant_first( DrtPred &pred, const string &str);
DrtPred implant_second( DrtPred &pred, const string &str);
DrtPred implant_subject( DrtPred &pred, const string &name);
DrtPred implant_subject_safe( DrtPred &pred, const string &name);
DrtPred implant_object_safe( DrtPred &pred, const string &name);
DrtPred implant_object( DrtPred &pred, const string &name);
DrtPred implant_subject_or_object( DrtPred &pred, const string &name);
DrtPred implant_header(DrtPred &pred, const string &header);
DrtPred add_header(DrtPred &pred, const string &header);
DrtPred implant_children(DrtPred &pred, const vector<string> &str);

double get_single_distance(const string &str);
string get_string_distance(const string &str);
void drt_sort(vector <DrtPred> &);

bool ref_is_ref(const string &ftag);
bool ref_is_name(const string &ftag);
bool ref_is_name_no_ref(const string &ftag);
bool ref_is_verb(const string &ftag);


DrtVect create_drtvect(const string &s);

vector<DrtVect> get_linked_drtvect_from_single_drtvect(const DrtVect &d);
int find_verb_with_object(vector<DrtPred> &pre_drt, string from_str);
int find_verb_with_subject(vector<DrtPred> &pre_drt, string from_str);

#endif // __DRTPRED__
