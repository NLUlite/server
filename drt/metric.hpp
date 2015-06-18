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



#ifndef __METRIC__
#define __METRIC__

#include<iostream>
#include<algorithm>
#include<vector>
#include<map>
#include<boost/unordered_map.hpp>
#include<string>
#include<fstream>
#include<sstream>
#include<cmath>
#include<boost/tuple/tuple.hpp>
#include<boost/utility.hpp>
#include<boost/algorithm/string.hpp>
#include<boost/lexical_cast.hpp>
#include<boost/shared_ptr.hpp>
#include"../fopl/fopl.hpp"


using std::pair;
using std::make_pair;
using std::map;
using std::vector;
using std::string;
using std::cout;
using std::endl;
using boost::unordered_map;


typedef unordered_map< int, vector<string> > MapIntString;
typedef unordered_map< int, vector<int> > MapIntInt;
typedef unordered_map< string, vector<int> > MapStringInt;
typedef unordered_map< string, int > MapStringNvInt;
typedef unordered_map< string, bool > MapStringBool;
typedef unordered_map< string, string > MapStringString;

class metric {   
     
	MapStringBool countries_map_;

     MapIntString synsets_strings;
     MapStringInt synsets_map;
     MapIntInt hypernyms_map;
     MapIntInt reverse_hypernyms_map_;

     MapIntString verb_synsets_strings;
     MapStringInt verb_synsets_map;     
     MapIntInt verb_hypernyms_map;
     MapIntInt reverse_verb_hypernyms_map;
     
     MapIntString adj_strings_map;
     MapStringInt adj_int_map;
     MapIntInt adj_similar_map;
     MapIntInt reverse_adj_similar_map;
     MapIntInt adj_pertainym_map;
     MapIntInt reverse_adj_pertainym_map;

     MapIntString adv_strings_map;
     MapStringInt adv_int_map;
     MapIntInt adv_pertainym_map;


     MapStringNvInt word_freq;
     MapStringString verb_levin;
     MapStringString noun_levin;

     MapStringString people_names_;

     MapStringString noun_from_verb_;
     MapStringString verb_from_noun_;

     vector<string> what_vector_;

     int intersection_;

     pair<int,int> has_intersection_and_position(vector<int> &num_syn1, vector<int> &num_syn2);
     int has_intersection(vector<int> &num_syn1, vector<int> &num_syn2);
     double jico_helper(const string &, const string &, const int &max_d);
     double w_jico_helper(const string &, const string &, const int &max_d);

     double verb_jico_helper(const string &, const string &, const int &max_d);
     double hypernym_helper(const string &, const string &, const int &max_d);
     double hypernym_helper(const string &, vector<int>, const int &max_d);
     double verb_hypernym_helper(const string &, const string &, const int &max_d);
     
     double lexical_dist(const string &a, const string &b);

     void initWhat();

public:
     metric();
     
     void load_countries(const char *f);

     void load_synsets(const char *f);
     void load_hypernyms(const char *f);

     void load_verb_synsets(const char *f);
     void load_verb_hypernyms(const char *f);
     
     void load_adj_lexnames(const char *f);
     void load_adj_similar(const char *f);
     void load_adj_pertainym(const char *f);

     void load_adv_synsets(const char *f);
     void load_adv_pertainym(const char *f);

     void load_word_freq(const char *f);
     void load_verb_levin(const char *f);
     void load_noun_levin(const char *f);    

     void load_male_names(const char *f);
     void load_female_names(const char *f);

     void load_nominalization(const char *f);


     double distance(const string &, const string &);
     double separation(const string &, const string &, const int &);
     double jico_dist(const string &, const string &, const int &max_d=4);
     double weighted_jico_dist(const string &, const string &, const int &max_d=4);
     double hypernym(const string &, const string &, const int &);
     double hypernym(const string &, vector<int> , const int &);
     double hypernym_dist(const string &, const string &, const int &max_d=4);     
     double hypernym_dist(const string &, vector<int>, const int &max_d=4);     
     vector<int> noun_lexnames(const string &s);

     double verb_separation(const string &, const string &, const int &);
     double verb_jico_dist(const string &, const string &, const int &max_d=4);
     double verb_hypernym(const string &, const string &, const int &);
     double verb_hypernym_dist(const string &, const string &, const int &max_d=4);
     
     vector<string> get_hypernyms_of_adjective(const string &s1, int max_sep);
     vector<string> get_hypernyms_of_noun(const string &s1, int max_sep);
     vector<string> get_synonyms_of_noun(const string &s1);
     vector<string> get_hypernyms_of_verb(const string &s1, int max_sep);
     

     vector<string> get_hyponyms_of_adjective(const string &s1, int max_sep);
     vector<string> get_hyponyms_of_noun(const string &s1, int max_sep);
     vector<string> get_hyponyms_of_verb(const string &s1, int max_sep);

     vector<string> verb_get_intersection();
     vector<int> verb_lexnames(const string &s);

     bool has_synset(const string &str);

     vector<string> get_intersection();
     string get_levin_verb(const string &s);
     string get_levin_noun(const string &s);
     string gender_proper_name(const string &str);

     bool has_verb(const string &s);
     bool has_noun(const string &s);
     
     string getNounFromVerb(const string &s);
     string getVerbFromNoun(const string &s);


     vector<int> adj_lexnames(const string &adj);
     double pertains_to_name(const string &adj, const string &name, int max=4);
     vector<string> get_pertainyms(const string &adj);
     vector<string> get_reverse_pertainyms(const string &adj);

     vector<int> adv_lexnames(const string &adv);
     double pertains_to_adj(const string &adv, const string &name, int max=4);
     vector<string> get_adv_pertainyms(const string &adv);


     bool is_title(const string &name);
     bool is_country(const string &name);
     bool is_adjective(const string &name);
     bool is_adverb(const string &name);
     
     double hypernym_dist_from_trees(const string &data, const string &question, const vector<boost::shared_ptr<Predicate> > &ktrees, int max=4);

     vector<string> get_what();
};



#endif // __METRIC__
