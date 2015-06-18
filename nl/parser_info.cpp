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



#include"parser_info.hpp"
#include<boost/thread.hpp>

using std::string;
using std::map;
using std::pair;
using std::make_pair;
using std::cout;
using std::endl;

const bool debug = false;


template <class T>
static bool shortfind(const vector<T> &vect, const T &element) 
{
     if(find(vect.begin(),vect.end(),element) == vect.end())
          return false;
     return true;
}

template <class T>
static void print_vector(std::vector<T> &vs)
{
     typename vector<T>::iterator tags_iter= vs.begin();
     while ( tags_iter < vs.end() ) {
	  std::cout << (*tags_iter) << " ";
	  ++ tags_iter;
     }
     std::cout << std::endl;
}

template <class T>
static void print_vector_return(std::vector<T> &vs)
{
     typename vector<T>::iterator tags_iter= vs.begin();
     while ( tags_iter < vs.end() ) {
	  std::cout << (*tags_iter) << " \n";
	  ++ tags_iter;
     }
     std::cout << std::endl;
}


template <class T>
static void print_vector_pointers(std::vector<T> &vs)
{
     typename vector<T>::iterator tags_iter= vs.begin();
     while ( tags_iter < vs.end() ) {
	  std::cout << (*(*tags_iter)) << " \n";
	  ++ tags_iter;
     }
     std::cout << std::endl;
}


ClauseContainer::ClauseContainer(const string &s)
{
     clause_str_= s;
}

void ClauseContainer::setWeigth(double w2)
{
     w_= w2;
}


void ClauseContainer::setConsequenceInfo(double ci)
{
     cons_info_= ci;
}

void ClauseContainer::setHypothesisInfo(double hi)
{
     hyp_info_= hi;
}
     
Clause ClauseContainer::getClause()
{
     Clause to_return(clause_str_);
     
     to_return.setWeigth(w_);
     to_return.setConsequenceInfo(cons_info_);
     to_return.setHypothesisInfo(hyp_info_);
     
     return to_return;
}

static bool verb_supports_indirect_obj(const string &name)
{
     vector<string> ind_verbs;
     ind_verbs.push_back("give");
     ind_verbs.push_back("wish");
     ind_verbs.push_back("make");
     ind_verbs.push_back("buy");
     ind_verbs.push_back("pay");
     ind_verbs.push_back("show");     
     ind_verbs.push_back("call");
     ind_verbs.push_back("regard_as");     
     ind_verbs.push_back("set");
     ind_verbs.push_back("promise");

     if( find(ind_verbs.begin(), ind_verbs.end(), name) != ind_verbs.end() )
	  return true;

     return false;
}


static string extract_hyp(const string &clause)
{
     vector<string> strs, hyp;
     string ret_string;
     
     boost::split(strs,clause, boost::is_any_of(":") );
     if(strs.size() != 2)
	  throw(std::invalid_argument("Badly formed clause."));
     strs.at(1).erase(0,1); // erase the '-'    
     boost::split(hyp,strs.at(1), boost::is_any_of(",") );    
     //ret_string= hyp.at(0);
     int n=0;
     if(hyp.size() > 0) {
      	  ret_string+= hyp.at(0);
	  ++n;
     }
     if(hyp.size() > 1) {
	  ret_string+= hyp.at(1);
       	  ++n;
     }
     // if(hyp.size() > 2) {
     //  	  ret_string+= hyp.at(2);
     //   	  ++n;
     // }
     //for(; n< hyp.size()-1; ++n)
     for(; n< hyp.size(); ++n)
       	  //ret_string += string(" X");
	  ret_string += hyp.at(n);
     //ret_string += hyp.at(hyp.size()-1);
     return ret_string;
}

static string extract_cons(const string &clause)
{
     vector<string> strs;
     
     boost::split(strs,clause, boost::is_any_of(":") );
     if(strs.size() != 2)
	  throw(std::invalid_argument("Badly formed clause."));
     return Predicate(strs.at(0)).pred().begin()->str;
}

static vector<string> get_feet_from_clause(const Clause &clause)
{
     vector<string > hyp_tags;
     vector<FuzzyPred> hyp= clause.getHypothesis();
     vector<FuzzyPred>::iterator hyp_iter= hyp.begin();

     for(;hyp_iter != hyp.end(); ++hyp_iter)
	  hyp_tags.push_back( hyp_iter->getOrder().first );
     return hyp_tags;
}

void parser_info::load_feet_clauses(const char *f)
{
     std::ifstream file;
     char article[10000];
     int num_children, freq;
     map<string, int> hyp_freq;
     map<string, int> clause_freq;
     map<string, int> cons_freq;
     map<pair<string,string>, int> cons_hyp_freq;
     string clause_str;
     vector<string> all_clauses;
     vector<string> data;

     file.open(f);
     if(! file.bad() ) {
	  std::cerr <<  "Loading clauses from \'" << f << "\'." << std::endl;
	  while( !file.eof() ) {
	       file.getline(article,10000);
	       boost::split(data, article, boost::is_any_of("\t"));
	       //cout << article << std::flush << endl;
	       if(data.size()  == 4) {
		    clause_str= data.at(0);
		    num_children= boost::lexical_cast<int>(data.at(2));
		    freq= boost::lexical_cast<int>(data.at(3));
		    clause_freq[clause_str] = +freq;
		    hyp_freq[extract_hyp(clause_str)] += freq;
		    cons_freq[extract_cons(clause_str)] += freq;
		    cons_hyp_freq[make_pair(extract_cons(clause_str), extract_hyp(clause_str)) ] += freq;
		    all_clauses.push_back(clause_str);		    
	       }
	  }
     }
     else
	  throw std::length_error(std::string("File") + f + " finished unexpectedly.");
     file.close();

     // Sets the weigth of each clause
     vector<string>::iterator clause_iter= all_clauses.begin();
     while (clause_iter != all_clauses.end() ) {
          Clause real_clause(*clause_iter);
	  ClauseContainer clause_tmp(*clause_iter);
	  clause_tmp.setWeigth( (double)cons_hyp_freq[make_pair(extract_cons(*clause_iter), extract_hyp(*clause_iter)) ] 
				/ hyp_freq[extract_hyp(*clause_iter)] 
				*exp(-1/sqrt( clause_freq[*clause_iter] ) ) // the uncertaincy is less when clause_freq is high
				);
	  clause_tmp.setConsequenceInfo(cons_freq[extract_cons(*clause_iter)]);
	  clause_tmp.setHypothesisInfo(hyp_freq[extract_hyp(*clause_iter)]);
	  
	  matching_feet[ get_feet_from_clause(real_clause) ].push_back( clause_tmp );
	  ++clause_iter;
     }
}

void parser_info::load_bulk_clauses(const char *f)
{
     std::ifstream file;
     char article[10000];
     int num_children, freq;
     map<pair<string, int>,int > headers;
/// Operator < for clauses does not work properly, I have to use a string instead
     map<string, int> hyp_freq;
     map<string, int> clause_freq;
     map<string, int> cons_freq;
     map<pair<string,string>, int> cons_hyp_freq;
     string clause_str;
     vector<string> all_clauses;
     vector<string> data;

     file.open(f);
     if(! file.bad() ) {
	  std::cerr <<  "Loading clauses from \'" << f << "\'." << std::endl;
	  while( file.good() ) {
	       file.getline(article,10000);	       
	       boost::split(data, article, boost::is_any_of("\t"));	       
	       if(data.size()  == 4) {
		    clause_str= data.at(0);
		    num_children= boost::lexical_cast<int>(data.at(2));
		    freq= boost::lexical_cast<int>(data.at(3));
		    clause_freq[clause_str] += freq;
		    headers[ make_pair( data.at(1), num_children ) ] += freq;
		    cons_freq[extract_cons(clause_str)] += freq;
		    hyp_freq[extract_hyp(clause_str)] += freq;
		    cons_hyp_freq[make_pair(extract_cons(clause_str), extract_hyp(clause_str)) ] += freq;
		    all_clauses.push_back(clause_str);
	       }
	  }
     }
     else
	  throw std::length_error(std::string("File") + f + " finished unexpectedly.");
     file.close();

     // Sets the weigth of each clause
     vector<string>::iterator clause_iter= all_clauses.begin();
     while (clause_iter != all_clauses.end() ) {
          Clause real_clause(*clause_iter);
	  ClauseContainer clause_tmp(*clause_iter);
	  clause_tmp.setWeigth( (double)cons_hyp_freq[make_pair(extract_cons(*clause_iter), extract_hyp(*clause_iter)) ] 
				/ hyp_freq[extract_hyp(*clause_iter)] 
				*exp(-1/sqrt( clause_freq[*clause_iter] ) ) // the uncertaincy is less when clause_freq is high
				);
	  clause_tmp.setConsequenceInfo(cons_freq[extract_cons(*clause_iter)]);
	  clause_tmp.setHypothesisInfo(hyp_freq[extract_hyp(*clause_iter)]);

	  matching_bulk[ get_feet_from_clause(real_clause) ].push_back( clause_tmp );
	  ++clause_iter;
     }
}


void parser_info::load_roots_clauses(const char *f)
{
     std::ifstream file;
     char article[10000];
     int num_children, freq;
     map<pair<string, int>,int > headers;
/// Operator < for clauses does not work properly, I have to use a string instead
     map<string, int> hyp_freq;
     map<string, int> clause_freq;
     map<string, int> cons_freq;
     map<pair<string,string>, int> cons_hyp_freq;
     string clause_str;
     vector<string> all_clauses;
     vector<string> data;

     file.open(f);
     if(! file.bad() ) {
	  std::cerr <<  "Loading clauses from \'" << f << "\'." << std::endl;
	  while( file.good() ) {
	       file.getline(article,10000);
	       boost::split(data, article, boost::is_any_of("\t"));
	       if(data.size()  == 4) {
		    clause_str= data.at(0);
		    num_children= boost::lexical_cast<int>(data.at(2));
		    freq= boost::lexical_cast<int>(data.at(3));
		    clause_freq[clause_str] += freq;
		    headers[ make_pair( data.at(1), num_children ) ] += freq;
		    cons_freq[extract_cons(clause_str)] += freq;
		    hyp_freq[extract_hyp(clause_str)] += freq;
		    cons_hyp_freq[make_pair(extract_cons(clause_str), extract_hyp(clause_str)) ] += freq;
		    all_clauses.push_back(clause_str);
	       }
	  }
     }
     else
	  throw std::length_error(std::string("File") + f + " finished unexpectedly.");
     file.close();

     // Sets the weigth of each clause
     vector<string>::iterator clause_iter= all_clauses.begin();
     while (clause_iter != all_clauses.end() ) {
          Clause real_clause(*clause_iter);
	  ClauseContainer clause_tmp(*clause_iter);
	  clause_tmp.setWeigth( (double)cons_hyp_freq[make_pair(extract_cons(*clause_iter), extract_hyp(*clause_iter)) ] 
				/ hyp_freq[extract_hyp(*clause_iter)] 
				*exp(-1/sqrt( clause_freq[*clause_iter] ) ) // the uncertainty is less when clause_freq is high
				);
	  clause_tmp.setConsequenceInfo(cons_freq[extract_cons(*clause_iter)]);
	  clause_tmp.setHypothesisInfo(hyp_freq[extract_hyp(*clause_iter)]);

	  matching_roots[ get_feet_from_clause(real_clause) ].push_back( clause_tmp );
	  ++clause_iter;
     }
}

void parser_info::load_matching_clauses(const char *f)
{
     std::ifstream file;
     char article[10000];
     string clause_str;
     vector<string> all_clauses, data;

     file.open(f);
     if(! file.bad() ) {
	  std::cerr <<  "Loading clauses from \'" << f << "\'." << std::endl;
	  while( file.good() ) {
	       file.getline(article,10000);
	       boost::split(data, article, boost::is_any_of("\t"));
	       clause_str= data.at(0);
	       if	(clause_str != "")
	     	  all_clauses.push_back(clause_str);
	  }
     }
     else
	  throw std::length_error(std::string("File") + f + " finished unexpectedly.");
     file.close();

     // Sets the weigth of each clause
     vector<string>::iterator clause_iter= all_clauses.begin();
     while (clause_iter != all_clauses.end() ) {
     	if(debug)
     		std::cerr << "CITER::: " << *clause_iter << endl;
     	Clause real_clause(*clause_iter);
     	matching_corrections.push_back(real_clause);
     	++clause_iter;
     }
}
