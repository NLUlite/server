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



#include"WisdomInfo.hpp"

const bool debug = false;

WisdomInfo::WisdomInfo()
{
	accuracy_level_       = 5;
	num_answers_          = 10;
	timeout_              = 4;
	max_refs_             = 3000000;
	max_candidates_refs_  = 50;
	max_candidates_       = 50;
	num_hyponyms_         = 2;
	fixed_time_           = 10; // 10 seconds before the search in Knowledge runs out
	solver_options_       = "";
	skip_presuppositions_ = false;
	skip_solver_          = false;
	do_solver_            = false;
	add_data_             = true;
	word_intersection_    = true;
	use_pertaynims_       = true;
	use_synonyms_         = false;
	use_hyponyms_         = true;
	load_clauses_         = true;
	implicit_verb_        = false;
	is_wikidata_          = false;
}

void WisdomInfo::setAccuracyLevel(int n)
{
	if (n > 5)
		n = 5;
	if (n < 0)
		n = 0;
	accuracy_level_= n;
}

void WisdomInfo::setNumAnswers(int n)
{
	if (n > 100)
		n = 100;
	if (n < 0)
		n = 0;
	num_answers_= n;
}

void WisdomInfo::setTimeout(double w)
{
	if (w < 1)
		w = 1;
	timeout_= w;
}

void WisdomInfo::setFixedTime(double w)
{
	if (w < 1)
		w = 1;
	fixed_time_= w;
}

void WisdomInfo::setMaxRefs(int i)
{
	if (i < 1)
		i = 1;
	max_refs_= i;
}

void WisdomInfo::setNumHyponyms(int i)
{
	if (i < 0)
		i = 0;
	num_hyponyms_= i;
}

void WisdomInfo::setMaxCandidatesRefs(int i)
{
	if (i < 1)
		i = 1;
	max_candidates_refs_= i;
}

void WisdomInfo::setMaxCandidates(int i)
{
	if (i < 1)
		i = 1;
	max_candidates_= i;
}


void WisdomInfo::setSolverOptions(const string &str)
{
	solver_options_= str;
}

void WisdomInfo::setImplicitVerb(const string &str)
{
	if(debug)
		cout << "SET_IMPLICIT::: " << str << endl;

	if(str.find("false") != string::npos)
		implicit_verb_= false;
	else if(str.find("true") != string::npos)
		implicit_verb_= true;
}

void WisdomInfo::setUseSynonyms(const string &str)
{
	if(str.find("false") != string::npos)
		use_synonyms_= false;
	else if(str.find("true") != string::npos)
		use_synonyms_= true;
}

void WisdomInfo::setUseHyponyms(const string &str)
{
	if(str.find("false") != string::npos)
		use_hyponyms_= false;
	else if(str.find("true") != string::npos)
		use_hyponyms_= true;
}


void WisdomInfo::setSkipPresuppositions(const string &str)
{
	if(str.find("false") != string::npos)
		skip_presuppositions_= false;
	else if(str.find("true") != string::npos)
		skip_presuppositions_= true;
}

void WisdomInfo::setSkipSolver(const string &str)
{
	if(str.find("false") != string::npos)
		skip_solver_= false;
	else if(str.find("true") != string::npos)
		skip_solver_= true;
}


void WisdomInfo::setAddData(const string &str)
{
	if(str.find("false") != string::npos)
		add_data_= false;
	else if(str.find("true") != string::npos)
		add_data_= true;
}

void WisdomInfo::setDoSolver(const string &str)
{
	if(str.find("false") != string::npos)
		do_solver_= false;
	else if(str.find("true") != string::npos)
		do_solver_= true;
}

void WisdomInfo::setWordIntersection(const string &str)
{
	if(str.find("false") != string::npos)
		word_intersection_= false;
	else if(str.find("true") != string::npos)
		word_intersection_= true;
}

void WisdomInfo::setWikidata(const string &str)
{
	if(str.find("false") != string::npos)
		is_wikidata_= false;
	else if(str.find("true") != string::npos)
		is_wikidata_= true;
}


void WisdomInfo::setUsePertaynims(const string &str)
{
	if(str.find("false") != string::npos)
		use_pertaynims_= false;
	else if(str.find("true") != string::npos)
		use_pertaynims_= true;
}

void WisdomInfo::setLoadClauses(const string &str)
{
	if(str.find("false") != string::npos)
		load_clauses_= false;
	else if(str.find("true") != string::npos)
		load_clauses_= true;
}



bool WisdomInfo::getAddData()
{
	return add_data_;
}

bool WisdomInfo::getWordIntersection()
{
	return word_intersection_;
}

bool WisdomInfo::getUsePertaynims()
{
	return use_pertaynims_;
}

void WisdomInfo::startTime()
{
	start_time_= clock();
}

double WisdomInfo::getTimeout()
{
	clock_t current_time = clock();
	double elapsed = (current_time - start_time_) / (double) CLOCKS_PER_SEC;
	return timeout_ - elapsed;
}

int WisdomInfo::getMaxRefs()
{
	return max_refs_;
}

int WisdomInfo::getMaxCandidatesRefs()
{
	return max_candidates_refs_;
}

int WisdomInfo::getMaxCandidates()
{
	return max_candidates_;
}

bool WisdomInfo::getUseSynonyms()
{
	return use_synonyms_;
}

bool WisdomInfo::getUseHyponyms()
{
	return use_hyponyms_;
}


bool WisdomInfo::getLoadClauses()
{
	return load_clauses_;
}

bool WisdomInfo::getImplicitVerb()
{
	return implicit_verb_;
}

bool WisdomInfo::isWikidata()
{
	return is_wikidata_;
}

int WisdomInfo::getNumHyponyms()
{
	return num_hyponyms_;
}
