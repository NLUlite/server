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



#ifndef __WISDOM_INFO__
#define __WISDOM_INFO__

#include<iostream>
#include<string>

using std::string;
using std::cout;
using std::endl;

class WisdomInfo {

	int accuracy_level_;
	int num_answers_;
	int max_refs_;             // max refs a name can have when pointing to an action
	int max_candidates_refs_ ; // max refs to be processed for matching
	int max_candidates_;       // max candidates to be matched
	int num_hyponyms_;         // number of hyponym levels to use in Knowledge

	string solver_options_;
	bool skip_presuppositions_, skip_solver_;
	bool add_data_;
	double timeout_;
	double fixed_time_;
	bool do_solver_;
	bool word_intersection_;  // the intersection within two answers is based on similar words
	bool use_pertaynims_;     // the intersection within two answers is based on similar words
	bool use_synonyms_;       // synonyms or pertainyms in Knowledge ?
	bool use_hyponyms_;       // synonyms or pertainyms in Knowledge ?
	bool load_clauses_;
	bool implicit_verb_;      // implicit verb for questions
	bool is_wikidata_;
	clock_t start_time_;


public:
	WisdomInfo();
	void setAddData(const string &str);
	void setAccuracyLevel(int);
	void setNumAnswers(int);
	void setSolverOptions(const string &);
	void setSkipPresuppositions(const string &);
	void setSkipSolver(const string &);
	void setTimeout(double);
	void setFixedTime(double);
	void setDoSolver(const string &);
	void setWordIntersection(const string &);
	void setUsePertaynims(const string &);
	void setMaxRefs(int i);
	void setMaxCandidatesRefs(int i);
	void setMaxCandidates(int i);
	void setNumHyponyms(int i);
	void setUseSynonyms(const string &);
	void setUseHyponyms(const string &);
	void setLoadClauses(const string &);
	void setImplicitVerb(const string &);
	void setWikidata(const string &);


	bool getAddData();
	bool getWordIntersection();
	int getAccuracyLevel() { return accuracy_level_; }
	int getNumAnswers() { return num_answers_; }
	string getSolverOptions() { return solver_options_; }
	bool getSkipPresuppositions() { return skip_presuppositions_; }
	bool getSkipSolver() { return skip_solver_; }
	bool getDoSolver() { return do_solver_; }
	double getTimeout();
	double getFixedTime() { return fixed_time_; }
	bool getUsePertaynims();
	bool getUseSynonyms();
	bool getUseHyponyms();
	bool getLoadClauses();
	bool getImplicitVerb();
	bool isWikidata();

	int getMaxRefs();
	int getMaxCandidatesRefs();
	int getMaxCandidates();
	int getNumHyponyms();


	void startTime();
};

#endif // __WISDOM_INFO__
