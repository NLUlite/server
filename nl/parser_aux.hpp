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



#ifndef __PARSER_AUX__
#define __PARSER_AUX__


#include<iostream>
#include<fstream>
#include<stdexcept>
#include<vector>
#include<map>
#include<algorithm>
#include<string>
#include<cmath>
#include<boost/algorithm/string.hpp>
#include<boost/unordered_map.hpp>
#include"../infero/infero.hpp"
#include"tagger_info.hpp"
#include"tagger.hpp"
#include"parser_info.hpp"

using std::vector;
using std::map;
using boost::unordered_map;
using std::string;


template<class T>
void print_vector(std::stringstream &ss, const std::vector<T> &vs)
{
	typename vector<T>::const_iterator tags_iter = vs.begin();
	while (tags_iter < vs.end()) {
		ss << (*tags_iter) << " ";
		++tags_iter;
	}
}


// auxiliary functions for selecting the clauses to match
string extract_hyp(const string &clause);
string extract_cons(const string &clause);
vector<string> get_feet_from_clause(const Clause &clause);
bool is_relevant(const Clause &clause, const vector<FuzzyPred> &data);
bool is_valid_start(vector<string> &tags, vector<string>::iterator start);
bool is_valid_end(vector<string> &tags, vector<string>::iterator end);
vector<Clause> get_relevant_clauses_from_feet(const clauses_map &matching_feet, vector<FuzzyPred> data);
vector<Clause> get_exact_match(const clauses_map &matching_feet, vector<FuzzyPred> data);
bool is_root(const vector<FuzzyPred> &clauses);
vector<pair<vector<FuzzyPred>, Memory> > get_roots(const vector<pair<vector<FuzzyPred>, Memory> > data);


// auxiliary functions for post-processing the parsing

PredTree correct_period(PredTree to_return);
bool has_verb(PredTree::iterator pi, const PredTree &pt);
bool has_WRB(PredTree::iterator pi, const PredTree &pt);
PredTree name_subordinates(PredTree pt);
PredTree clean_subordinates(PredTree pt);
vector<string> get_feet(const PredTree &pt);
PredTree move_period_out(const PredTree &pt);
bool is_question(PredTree::iterator pi, PredTree::iterator pend);
FuzzyPred post_process_sbar(const Predicate &pred);
FuzzyPred apply_corrections_questions(FuzzyPred pred);
FuzzyPred apply_corrections(FuzzyPred pred, const vector<Clause> &sub_candidates);
PredTree correct_tree(PredTree &pt);
FuzzyPred post_process(const Predicate &pred);

// auxiliary functions for finding PRN subordinates 

bool is_opening_par(const FuzzyPred &pred);
bool is_closing_par(const FuzzyPred &pred);
vector<pair<string, vector<FuzzyPred> > > extract_prn(vector<FuzzyPred> *data, int *num_prn);
bool has_opening_nested(vector<FuzzyPred>::iterator ipred, vector<FuzzyPred> &data);
bool has_closing_nested(vector<FuzzyPred>::iterator ipred, vector<FuzzyPred> &data);
bool is_opening_nested(vector<FuzzyPred>::iterator ipred, vector<FuzzyPred> &data, const FuzzyPred &open_item, bool is_question_);



#endif // __PARSER_AUX__
