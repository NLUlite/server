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



#include"metric.hpp"

const bool debug = false;

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

template<class K, class T>
static vector<T> get_map_element(boost::unordered_map<K, vector<T> > &map_element, K &key)
{
	vector<T> to_return;
	typename boost::unordered_map<K, vector<T> >::iterator miter = map_element.find(key);
	if (miter != map_element.end()) {
		to_return = miter->second;
	}
	return to_return;
}

int get_map_single_element(boost::unordered_map<string, int> &map_element, const string &key)
{
	int to_return = 0;
	boost::unordered_map<string, int>::iterator miter = map_element.find(key);
	if (miter != map_element.end()) {
		to_return = miter->second;
	}
	return to_return;
}

static string get_names(char *article_line)
{
	string str(article_line);
	int start = str.find(",") + 1;
	int end = str.find_last_of(",");
	//std::cout << start << ", "<< end << std::endl;
	return str.substr(start, end - start);
}

static pair<int, string> get_number_and_names(char *article_line)
{
	string str(article_line);
	int start = str.find(",") + 1;
	int end = str.find("|");
	//std::cout << start << ", "<< end << std::endl;
	string lexname = str.substr(0, start - 1);
	//cout << lexname << endl;
	if (lexname.size() == 0)
		return make_pair(0, "");
	int number = boost::lexical_cast<int>(lexname);
	return make_pair(number, str.substr(start, end - start));
}

static pair<int, string> get_number_and_names_for_nouns(char *article_line)
{
	string str(article_line);
	int start = str.find(",") + 1;
	int end = str.find("|");
	//std::cout << start << ", "<< end << std::endl;
	string lexname = str.substr(0, start - 1);
	//cout << lexname << endl;
	if (lexname.size() == 0)
		return make_pair(0, "");
	int number = boost::lexical_cast<int>(lexname);
	return make_pair(number, str.substr(start, end - start));
}

static pair<int, string> get_number_and_names_for_adv(char *article_line)
{
	string str(article_line);
	int start = str.find(",") + 1;
	int end = str.find(",", start);
	string lexname = str.substr(0, start - 1);
	//cout << lexname << endl;
	if (lexname.size() == 0)
		return make_pair(0, "");
	int number = boost::lexical_cast<int>(lexname);
	return make_pair(number, str.substr(start, end - start));
}

void metric::load_nominalization(const char *f)
{
	std::ifstream file;
	string orth, verb;
	int freq;
	int n = 1;
	file.open(f);
	if (!file.bad()) {
		std::cerr << "Loading nominalization from \'" << f << "\'." << std::endl;
		while (file.good()) {
			file >> orth >> verb;
			noun_from_verb_[verb] = orth;
			verb_from_noun_[orth] = verb;
			// else if(data.size()  != 0)
			// 		 throw std::length_error(std::string("File ") + f + " does not have the right format.");
		}
	} else
		throw std::length_error(std::string("File") + f + " finished unexpectedly.");
	file.close();
}


void metric::load_word_freq(const char *f)
{
	std::ifstream file;
	string word;
	int freq;
	int n = 1;
	file.open(f);
	if (!file.bad()) {
		std::cerr << "Loading word frequencies from \'" << f << "\'." << std::endl;
		while (file.good()) {
			file >> word >> freq;
			word_freq[word] = freq;
			// else if(data.size()  != 0)
			// 		 throw std::length_error(std::string("File ") + f + " does not have the right format.");
		}
	} else
		throw std::length_error(std::string("File") + f + " finished unexpectedly.");
	file.close();
}

void metric::load_countries(const char *f)
{
	std::ifstream file;
	string word;
	int n = 1;
	file.open(f);
	if (!file.bad()) {
		std::cerr << "Loading countries' names from \'" << f << "\'." << std::endl;
		while (file.good()) {
			file >> word;
			countries_map_[word] = true;
		}
	} else
		throw std::length_error(std::string("File") + f + " finished unexpectedly.");
	file.close();
}

void metric::load_male_names(const char *f)
{
	std::ifstream file;
	string word;
	int freq;
	int n = 1;
	char membuf[100];
	file.open(f);
	if (!file.bad()) {
		std::cerr << "Loading male proper names from \'" << f << "\'." << std::endl;
		while (file.good()) {
			file.getline(membuf, 100);
			std::stringstream ss(membuf);
			ss >> word;
			if (word == "")
				continue;
			for (int i = 0; i < word.size(); ++i)
				word.at(i) = std::tolower(word.at(i));
			people_names_[word] = "male";
		}
	} else
		throw std::length_error(std::string("File") + f + " finished unexpectedly.");
	file.close();
}

void metric::load_female_names(const char *f)
{
	std::ifstream file;
	string word;
	int freq;
	int n = 1;
	char membuf[100];
	file.open(f);
	if (!file.bad()) {
		std::cerr << "Loading male proper names from \'" << f << "\'." << std::endl;
		while (file.good()) {
			file.getline(membuf, 100);
			std::stringstream ss(membuf);
			ss >> word;
			if (word == "")
				continue;
			for (int i = 0; i < word.size(); ++i)
				word.at(i) = std::tolower(word.at(i));
			people_names_[word] = "female";
		}
	} else
		throw std::length_error(std::string("File") + f + " finished unexpectedly.");
	file.close();
}

void metric::load_verb_levin(const char *f)
{
	map<int, string> verb_map;
	verb_map[29] = "verb.body";
	verb_map[30] = "verb.change";
	verb_map[31] = "verb.cognition";
	verb_map[32] = "verb.communication";
	verb_map[33] = "verb.competition";
	verb_map[34] = "verb.consumption";
	verb_map[35] = "verb.contact";
	verb_map[36] = "verb.creation";
	verb_map[37] = "verb.emotion";
	verb_map[38] = "verb.motion";
	verb_map[39] = "verb.perception";
	verb_map[40] = "verb.possession";
	verb_map[41] = "verb.social";
	verb_map[42] = "verb.stative";
	verb_map[43] = "verb.weather";

	std::ifstream file;
	string word;
	int lev_num;
	int n = 1;
	char membuf[10000];

	file.open(f);
	if (!file.bad()) {
		std::cerr << "Loading verb levins from \'" << f << "\'." << std::endl;
		while (file.good()) {
			file.getline(membuf, 10000);
			//cout << "membuf:::" << membuf << endl;
			if (membuf[0] == '#')
				continue; // This line is a comment
			std::stringstream ss(membuf);
			ss >> word >> lev_num;
			verb_levin[word] = verb_map[lev_num];
			//cout << "levin:::" << word << " [" << verb_map[lev_num] << "]" << endl;
			// else if(data.size()  != 0)
			// 		 throw std::length_error(std::string("File ") + f + " does not have the right format.");
		}
	} else
		throw std::length_error(std::string("File") + f + " finished unexpectedly.");
	file.close();
}

void metric::load_noun_levin(const char *f)
{
	map<int, string> noun_map;
	noun_map[3] = "noun.Tops";
	noun_map[4] = "noun.act";
	noun_map[5] = "noun.animal";
	noun_map[6] = "noun.artifact";
	noun_map[7] = "noun.attribute";
	noun_map[8] = "noun.body";
	noun_map[9] = "noun.cognition";
	noun_map[10] = "noun.communication";
	noun_map[11] = "noun.event";
	noun_map[12] = "noun.feeling";
	noun_map[13] = "noun.food";
	noun_map[14] = "noun.group";
	noun_map[15] = "noun.location";
	noun_map[16] = "noun.motive";
	noun_map[17] = "noun.object";
	noun_map[18] = "noun.person";
	noun_map[19] = "noun.phenomenon";
	noun_map[20] = "noun.plant";
	noun_map[21] = "noun.possession";
	noun_map[22] = "noun.process";
	noun_map[23] = "noun.quantity";
	noun_map[24] = "noun.relation";
	noun_map[25] = "noun.shape";
	noun_map[26] = "noun.state";
	noun_map[27] = "noun.substance";
	noun_map[28] = "noun.time";

	std::ifstream file;
	string word;
	int lev_num;
	int n = 1;
	char membuf[10000];

	file.open(f);
	if (!file.bad()) {
		std::cerr << "Loading noun levins from \'" << f << "\'." << std::endl;
		while (file.good()) {
			file.getline(membuf, 10000);
			if (membuf[0] == '#')
				continue; // This line is a comment
			std::stringstream ss(membuf);
			ss >> word >> lev_num;
			noun_levin[word] = noun_map[lev_num];
			if (debug)
				cout << word << " " << lev_num << endl;
			// else if(data.size()  != 0)
			// 		 throw std::length_error(std::string("File ") + f + " does not have the right format.");
		}
	} else
		throw std::length_error(std::string("File") + f + " finished unexpectedly.");
	file.close();
}

void metric::load_synsets(const char *f)
{
	std::ifstream file;
	char article[1024];
	vector<string> data;
	string names_to_split;
	vector<string>::iterator syn_iter;
	int n = 1;
	file.open(f);
	if (!file.bad()) {
		std::cerr << "Loading synsets from \'" << f << "\'." << std::endl;
		while (file.good()) {
			file.getline(article, 1024);
			if (article[0] == '#')
				continue; // This line is a comment
			pair<int, string> lexname_and_names = get_number_and_names_for_nouns(article);
			n = lexname_and_names.first;
			string names_to_split = lexname_and_names.second;
			for (int i = 0; i < names_to_split.size(); ++i)
				names_to_split.at(i) = std::tolower(names_to_split.at(i));

			boost::split(data, names_to_split, boost::is_any_of(" "));
			if (data.size() > 0) {
				syn_iter = data.begin();
				for (; syn_iter != data.end(); ++syn_iter) {
					synsets_strings[n].push_back(*syn_iter);
					synsets_map[*syn_iter].push_back(n);
				}
				++n;
			}
		}
	} else
		throw std::length_error(std::string("File") + f + " finished unexpectedly.");
	file.close();
}

void metric::load_adv_synsets(const char *f)
{
	std::ifstream file;
	char article[1024];
	vector<string> data;
	string names_to_split;
	vector<string>::iterator syn_iter;
	int n = 1;
	file.open(f);
	if (!file.bad()) {
		std::cerr << "Loading adverb synsets from \'" << f << "\'." << std::endl;
		while (file.good()) {
			file.getline(article, 1024);
			if (article[0] == '#')
				continue; // This line is a comment
			pair<int, string> lexname_and_names = get_number_and_names_for_adv(article);
			n = lexname_and_names.first;
			string names_to_split = lexname_and_names.second;
			for (int i = 0; i < names_to_split.size(); ++i)
				names_to_split.at(i) = std::tolower(names_to_split.at(i));

			boost::split(data, names_to_split, boost::is_any_of(" "));
			if (data.size() > 0) {
				syn_iter = data.begin();
				for (; syn_iter != data.end(); ++syn_iter) {
					adv_strings_map[n].push_back(*syn_iter);
					adv_int_map[*syn_iter].push_back(n);
				}
				++n;
			}
		}
	} else
		throw std::length_error(std::string("File") + f + " finished unexpectedly.");
	file.close();
}

void metric::load_hypernyms(const char *f)
{
	std::ifstream file;
	char article[1024];
	vector<string> data;
	vector<string>::iterator hyper_iter;
	file.open(f);
	if (!file.bad()) {
		std::cerr << "Loading synsets from \'" << f << "\'." << std::endl;
		while (file.good()) {
			file.getline(article, 1024);
			if (article[0] == '#')
				continue; // This line is a comment
			//cout << article << endl;
			boost::split(data, article, boost::is_any_of(","));
			if (data.size() > 1) {
				hyper_iter = data.begin();
				++hyper_iter;
				for (; hyper_iter != data.end(); ++hyper_iter) {
					hypernyms_map[boost::lexical_cast<int>(data.at(0))].push_back(boost::lexical_cast<int>(*hyper_iter));
					reverse_hypernyms_map_[boost::lexical_cast<int>(*hyper_iter)].push_back(boost::lexical_cast<int>(data.at(0)));
				}
			}
			// else if(data.size()  != 0)
			// 		 throw std::length_error(std::string("File ") + f + " does not have the right format.");
		}
	} else
		throw std::length_error(std::string("File") + f + " finished unexpectedly.");
	file.close();
}

void metric::load_verb_synsets(const char *f)
{
	std::ifstream file;
	char article[1024];
	vector<string> data;
	string names_to_split;
	vector<string>::iterator syn_iter;
	file.open(f);
	if (!file.bad()) {
		std::cerr << "Loading synsets from \'" << f << "\'." << std::endl;
		while (file.good()) {
			file.getline(article, 1024);
			if (article[0] == '#')
				continue; // This line is a comment
			pair<int, string> lexname_and_names = get_number_and_names(article);
			int lexname = lexname_and_names.first;
			string names_to_split = lexname_and_names.second;
			for (int i = 0; i < names_to_split.size(); ++i)
				names_to_split.at(i) = std::tolower(names_to_split.at(i));

			boost::split(data, names_to_split, boost::is_any_of(" "));
			if (data.size() > 0) {
				syn_iter = data.begin();
				for (; syn_iter != data.end(); ++syn_iter) {
					verb_synsets_strings[lexname].push_back(*syn_iter);
					verb_synsets_map[*syn_iter].push_back(lexname);
				}
			}
			// else if(data.size()  != 0)
			// 		 throw std::length_error(std::string("File ") + f + " does not have the right format.");
		}
	} else
		throw std::length_error(std::string("File") + f + " finished unexpectedly.");
	file.close();
}

void metric::load_verb_hypernyms(const char *f)
{
	std::ifstream file;
	char article[1024];
	vector<string> data;
	vector<string>::iterator hyper_iter;
	file.open(f);
	if (!file.bad()) {
		std::cerr << "Loading synsets from \'" << f << "\'." << std::endl;
		while (file.good()) {
			file.getline(article, 1024);
			if (article[0] == '#')
				continue; // This line is a comment
			boost::split(data, article, boost::is_any_of(","));
			if (data.size() > 1) {
				hyper_iter = data.begin();
				++hyper_iter;
				for (; hyper_iter != data.end(); ++hyper_iter) {
					if (hyper_iter->size()) {
						verb_hypernyms_map[boost::lexical_cast<int>(data.at(0))].push_back(boost::lexical_cast<int>(*hyper_iter));
						reverse_verb_hypernyms_map[boost::lexical_cast<int>(*hyper_iter)].push_back(boost::lexical_cast<int>(boost::lexical_cast<int>(data.at(0))));
					}
				}
			}
		}
	} else
		throw std::length_error(std::string("File") + f + " finished unexpectedly.");
	file.close();
}

void metric::load_adj_similar(const char *f)
{
	std::ifstream file;
	char article[10000];
	vector<string> data;
	vector<string>::iterator hyper_iter;
	file.open(f);
	if (!file.bad()) {
		std::cerr << "Loading similar adjectives from \'" << f << "\'." << std::endl;
		while (file.good()) {
			file.getline(article, 10000);
			if (article[0] == '#')
				continue; // This line is a comment
			boost::split(data, article, boost::is_any_of(","));
			if (data.size() > 1) {
				hyper_iter = data.begin();
				++hyper_iter;
				for (; hyper_iter != data.end(); ++hyper_iter) {
					if (hyper_iter->size()) {
						adj_similar_map[boost::lexical_cast<int>(*hyper_iter)].push_back(
								boost::lexical_cast<int>(data.at(0)));
						reverse_adj_similar_map[boost::lexical_cast<int>(data.at(0))].push_back(boost::lexical_cast<int>(*hyper_iter));
						//cout << boost::lexical_cast<int>(data.at(0)) << " " << boost::lexical_cast<int>(*hyper_iter) << endl;
					}
				}
			}
		}
	} else
		throw std::length_error(std::string("File") + f + " finished unexpectedly.");
	file.close();
}

void metric::load_adj_pertainym(const char *f)
{
	std::ifstream file;
	char article[1024];
	vector<string> data;
	vector<string>::iterator hyper_iter;
	file.open(f);
	if (!file.bad()) {
		std::cerr << "Loading pertainym adjectives from \'" << f << "\'." << std::endl;
		while (file.good()) {
			file.getline(article, 1024);
			if (article[0] == '#')
				continue; // This line is a comment
			boost::split(data, article, boost::is_any_of(","));
			if (data.size() > 1) {
				hyper_iter = data.begin();
				++hyper_iter;
				for (; hyper_iter != data.end(); ++hyper_iter) {
					if (hyper_iter->size()) {
						adj_pertainym_map[boost::lexical_cast<int>(*hyper_iter)].push_back(
								boost::lexical_cast<int>(data.at(0)));
						reverse_adj_pertainym_map[boost::lexical_cast<int>(data.at(0))].push_back(
													boost::lexical_cast<int>(boost::lexical_cast<int>(*hyper_iter)));
					}
				}
			}
			// else if(data.size()  != 0)
			// 		 throw std::length_error(std::string("File ") + f + " does not have the right format.");
		}
	} else
		throw std::length_error(std::string("File") + f + " finished unexpectedly.");
	file.close();
}

void metric::load_adv_pertainym(const char *f)
{
	std::ifstream file;
	char article[1024];
	vector<string> data;
	vector<string>::iterator hyper_iter;
	file.open(f);
	if (!file.bad()) {
		std::cerr << "Loading pertainym adjectives from \'" << f << "\'." << std::endl;
		while (file.good()) {
			file.getline(article, 1024);
			if (article[0] == '#')
				continue; // This line is a comment
			boost::split(data, article, boost::is_any_of(","));
			if (data.size() > 1) {
				hyper_iter = data.begin();
				++hyper_iter;
				for (; hyper_iter != data.end(); ++hyper_iter) {
					if (hyper_iter->size()) {
						adv_pertainym_map[boost::lexical_cast<int>(data.at(0))].push_back(
								boost::lexical_cast<int>(*hyper_iter));
					}
				}
			}
		}
	} else
		throw std::length_error(std::string("File") + f + " finished unexpectedly.");
	file.close();
}

void metric::load_adj_lexnames(const char *f)
{
	std::ifstream file;
	char article[1024];
	vector<string> data;
	string names_to_split;
	vector<string>::iterator syn_iter;
	file.open(f);
	if (!file.bad()) {
		std::cerr << "Loading synsets from \'" << f << "\'." << std::endl;
		while (file.good()) {
			file.getline(article, 1024);
			if (article[0] == '#')
				continue; // This line is a comment
			pair<int, string> lexname_and_names = get_number_and_names(article);
			int lexname = lexname_and_names.first;
			string names_to_split = lexname_and_names.second;
			for (int i = 0; i < names_to_split.size(); ++i)
				names_to_split.at(i) = std::tolower(names_to_split.at(i));

			boost::split(data, names_to_split, boost::is_any_of(" ,"));
			if (data.size() > 0) {
				syn_iter = data.begin();
				for (; syn_iter != data.end(); ++syn_iter) {
					adj_strings_map[lexname].push_back(*syn_iter);
					adj_int_map[*syn_iter].push_back(lexname);
				}
			}
		}
	} else
		throw std::length_error(std::string("File") + f + " finished unexpectedly.");
	file.close();
}

pair<int, int> metric::has_intersection_and_position(vector<int> &num_syn1, vector<int> &num_syn2)
{
	vector<int>::iterator num_syn_iter1;

	int n = 0;
	num_syn_iter1 = num_syn1.begin();
	for (; num_syn_iter1 != num_syn1.end(); ++num_syn_iter1, ++n) {
		if (find(num_syn2.begin(), num_syn2.end(), *num_syn_iter1) != num_syn2.end())
			return make_pair(*num_syn_iter1, n);
	}
	return make_pair(0, 1000);
}

int metric::has_intersection(vector<int> &num_syn1, vector<int> &num_syn2)
{
	vector<int>::iterator num_syn_iter1;
	num_syn_iter1 = num_syn1.begin();
	for (; num_syn_iter1 != num_syn1.end(); ++num_syn_iter1) {
		if (find(num_syn2.begin(), num_syn2.end(), *num_syn_iter1) != num_syn2.end())
			return *num_syn_iter1;
	}

	return 0;
}

static int get_position(int pos, vector<int> &num_syn1)
{
	int size = num_syn1.size(), num_chunk = 0, n = 0;

	for (; n < size && num_chunk < pos; ++n) {
		num_chunk += num_syn1.at(n);
	}
	return n;
}

vector<string> metric::get_intersection()
{
	MapIntString::iterator mapiter = synsets_strings.find(intersection_);
	if (mapiter != synsets_strings.end())
		return mapiter->second;
	return vector<string>();
}

vector<string> metric::verb_get_intersection()
{
	MapIntString::iterator mapiter = verb_synsets_strings.find(intersection_);
	if (mapiter != verb_synsets_strings.end())
		return mapiter->second;
	return vector<string>();
}

double metric::separation(const string &s1, const string &s2, const int &max_sep)
{
	double ret_separation;
	MapStringInt::iterator synsets_map_iter1;
	MapStringInt::iterator synsets_map_iter2;
	vector<int> num_syn1, num_syn_new1;
	vector<int> num_syn2, num_syn_new2;
	vector<int> total1, total2;
	vector<int> sizes; // the sizes of the synsets1;

	vector<int>::iterator syn_iter1, syn_iter2;
	int num1, num2;

	this->intersection_ = -1; // The previous intersection result is cancelled
	ret_separation = 0;

	synsets_map_iter1 = synsets_map.find(s1);
	synsets_map_iter2 = synsets_map.find(s2);
	if (synsets_map_iter1 != synsets_map.end() && synsets_map_iter2 != synsets_map.end()) {
		num_syn1 = synsets_map_iter1->second;
		syn_iter1 = num_syn1.begin();
		num_syn2 = synsets_map_iter2->second;
		syn_iter2 = num_syn2.begin();
		total1.insert(total1.begin(), num_syn1.begin(), num_syn1.end());
		total2.insert(total2.begin(), num_syn2.begin(), num_syn2.end());
		sizes.push_back(num_syn2.size());
		pair<int, int> inter = has_intersection_and_position(total1, total2);
		if (inter.first) {
			this->intersection_ = inter.first;
			return ret_separation;
		}
		++ret_separation;
		while (ret_separation < max_sep) {
			for (; syn_iter1 != num_syn1.end(); ++syn_iter1) { // Evolution of s1
				vector<int> new_syn1 = get_map_element(hypernyms_map, *syn_iter1);
				num_syn_new1.insert(num_syn_new1.begin(), new_syn1.begin(), new_syn1.end());
				sizes.push_back(new_syn1.size());
			}
			for (; syn_iter2 != num_syn2.end(); ++syn_iter2) { // Evolution of s2
				vector<int> new_syn2 = get_map_element(hypernyms_map, *syn_iter2);
				num_syn_new2.insert(num_syn_new2.begin(), new_syn2.begin(), new_syn2.end());
			}
			++ret_separation;
			total1.insert(total1.begin(), num_syn_new1.begin(), num_syn_new1.end());
			total2.insert(total2.begin(), num_syn_new2.begin(), num_syn_new2.end());
			pair<int, int> inter = has_intersection_and_position(total1, total2);
			if (inter.first) {
				this->intersection_ = inter.first;
				return ret_separation;
			}
			num_syn1 = num_syn_new1;
			syn_iter1 = num_syn1.begin();
			num_syn2 = num_syn_new2;
			syn_iter2 = num_syn2.begin();
		}
	}

	return max_sep;
}

double metric::verb_separation(const string &s1, const string &s2, const int &max_sep)
{
	double ret_separation;
	MapStringInt::iterator synsets_map_iter1;
	MapStringInt::iterator synsets_map_iter2;
	vector<int> num_syn1, num_syn_new1;
	vector<int> num_syn2, num_syn_new2;
	vector<int> total1, total2;
	vector<int> sizes; // the sizes of the synsets1;

	vector<int>::iterator syn_iter1, syn_iter2;
	int num1, num2;

	this->intersection_ = -1; // The previous intersection result is cancelled
	ret_separation = 0;

	synsets_map_iter1 = verb_synsets_map.find(s1);
	synsets_map_iter2 = verb_synsets_map.find(s2);
	if (synsets_map_iter1 != verb_synsets_map.end() && synsets_map_iter2 != verb_synsets_map.end()) {
		num_syn1 = synsets_map_iter1->second;
		syn_iter1 = num_syn1.begin();
		num_syn2 = synsets_map_iter2->second;
		syn_iter2 = num_syn2.begin();
		total1.insert(total1.begin(), num_syn1.begin(), num_syn1.end());
		total2.insert(total2.begin(), num_syn2.begin(), num_syn2.end());
		sizes.push_back(num_syn2.size());
		pair<int, int> inter = has_intersection_and_position(total1, total2);
		if (inter.first) {
			this->intersection_ = inter.first;
			return ret_separation;
		}
		++ret_separation;
		while (ret_separation < max_sep) {
			for (; syn_iter1 != num_syn1.end(); ++syn_iter1) { // Evolution of s1
				vector<int> new_syn1 = get_map_element(verb_hypernyms_map, *syn_iter1);
				num_syn_new1.insert(num_syn_new1.begin(), new_syn1.begin(), new_syn1.end());
				sizes.push_back(new_syn1.size());
			}
			for (; syn_iter2 != num_syn2.end(); ++syn_iter2) { // Evolution of s2
				vector<int> new_syn2 = get_map_element(verb_hypernyms_map, *syn_iter2);
				num_syn_new2.insert(num_syn_new2.begin(), new_syn2.begin(), new_syn2.end());
			}
			++ret_separation;
			total1.insert(total1.begin(), num_syn_new1.begin(), num_syn_new1.end());
			total2.insert(total2.begin(), num_syn_new2.begin(), num_syn_new2.end());
			pair<int, int> inter = has_intersection_and_position(total1, total2);
			if (inter.first) {
				this->intersection_ = inter.first;
				return ret_separation;
			}
			num_syn1 = num_syn_new1;
			syn_iter1 = num_syn1.begin();
			num_syn2 = num_syn_new2;
			syn_iter2 = num_syn2.begin();
		}
	}

	return max_sep;
}

vector<string> metric::get_hypernyms_of_adjective(const string &s1, int max_sep)
{
	vector<string> to_return;

	MapStringInt::iterator synsets_map_iter1;
	vector<int> num_syn1, num_syn_new1;
	vector<int> total1, total2;
	vector<int>::iterator syn_iter1;
	int num1;

	int ret_separation = 0;
	synsets_map_iter1 = adj_int_map.find(s1);
	vector<int> all_syns;
	if (synsets_map_iter1 != adj_int_map.end()) {
		num_syn1 = synsets_map_iter1->second;
		all_syns.insert(all_syns.end(),num_syn1.begin(),num_syn1.end());
		syn_iter1 = num_syn1.begin();
		++ret_separation;
		while (ret_separation < max_sep) {
			for (; syn_iter1 != num_syn1.end(); ++syn_iter1) { // Evolution of s1
				vector<int> new_syn1 = get_map_element(adj_similar_map, *syn_iter1);
				num_syn_new1.insert(num_syn_new1.begin(), new_syn1.begin(), new_syn1.end());
			}
			++ret_separation;
			num_syn1 = num_syn_new1;
			syn_iter1 = num_syn1.begin();
			all_syns.insert(all_syns.end(),num_syn1.begin(),num_syn1.end());
		}
	}

	for (int n = 0; n < all_syns.size(); ++n) {
		vector<string> syn_str = get_map_element(adj_strings_map, all_syns.at(n));
		to_return.insert(to_return.end(), syn_str.begin(), syn_str.end());
		if(debug) {
			for(int m=0; m < syn_str.size(); ++m) {
				cout << "SYN_ADJ::: " << syn_str.at(m) << endl;
			}
		}
	}

	return to_return;
}

vector<string> metric::get_hyponyms_of_adjective(const string &s1, int max_sep)
{
	vector<string> to_return;

	MapStringInt::iterator synsets_map_iter1;
	vector<int> num_syn1, num_syn_new1;
	vector<int> total1, total2;
	vector<int>::iterator syn_iter1;
	int num1;

	int ret_separation = 0;
	synsets_map_iter1 = adj_int_map.find(s1);
	vector<int> all_syns;
	if (synsets_map_iter1 != adj_int_map.end()) {
		num_syn1 = synsets_map_iter1->second;
		all_syns.insert(all_syns.end(),num_syn1.begin(),num_syn1.end());
		syn_iter1 = num_syn1.begin();
		++ret_separation;
		while (ret_separation < max_sep) {
			for (; syn_iter1 != num_syn1.end(); ++syn_iter1) { // Evolution of s1
				vector<int> new_syn1 = get_map_element(reverse_adj_similar_map, *syn_iter1);
				num_syn_new1.insert(num_syn_new1.begin(), new_syn1.begin(), new_syn1.end());
			}
			++ret_separation;
			num_syn1 = num_syn_new1;
			syn_iter1 = num_syn1.begin();
			all_syns.insert(all_syns.end(),num_syn1.begin(),num_syn1.end());
		}
	}

	for (int n = 0; n < all_syns.size(); ++n) {
		vector<string> syn_str = get_map_element(adj_strings_map, all_syns.at(n));
		to_return.insert(to_return.end(), syn_str.begin(), syn_str.end());
		if(debug) {
			for(int m=0; m < syn_str.size(); ++m) {
				cout << "SYN_ADJ::: " << syn_str.at(m) << endl;
			}
		}
	}

	return to_return;
}


vector<string> metric::get_hypernyms_of_noun(const string &s1, int max_sep)
{
	vector<string> to_return;

	MapStringInt::iterator synsets_map_iter1;
	vector<int> num_syn1, num_syn_new1;
	vector<int> total1, total2;
	vector<int>::iterator syn_iter1;
	int num1;
	int ret_separation = 0;
	synsets_map_iter1 = synsets_map.find(s1);
	vector<int> all_syns;
	if (synsets_map_iter1 != synsets_map.end()) {
		num_syn1 = synsets_map_iter1->second;
		all_syns.insert(all_syns.end(),num_syn1.begin(),num_syn1.end());
		syn_iter1 = num_syn1.begin();
		++ret_separation;
		while (ret_separation < max_sep) {
			for (; syn_iter1 != num_syn1.end(); ++syn_iter1) { // Evolution of s1
				vector<int> new_syn1 = get_map_element(hypernyms_map, *syn_iter1);
				num_syn_new1.insert(num_syn_new1.begin(), new_syn1.begin(), new_syn1.end());
			}
			++ret_separation;
			num_syn1 = num_syn_new1;
			all_syns.insert(all_syns.end(),num_syn1.begin(),num_syn1.end());
			syn_iter1 = num_syn1.begin();
		}
	}

	for (int n = 0; n < all_syns.size(); ++n) {
		vector<string> syn_str = get_map_element(synsets_strings, all_syns.at(n));
		to_return.insert(to_return.end(), syn_str.begin(), syn_str.end());

		if(debug) {
			cout << "SYNSETS:: " << endl;
			print_vector(syn_str);
		}
	}
	return to_return;
}

vector<string> metric::get_synonyms_of_noun(const string &s1)
{
	vector<string> to_return;

	MapStringInt::iterator synsets_map_iter1;
	vector<int> num_syn1;
	vector<int> total1, total2;
	vector<int>::iterator syn_iter1;
	int num1;
	int ret_separation = 0;
	synsets_map_iter1 = synsets_map.find(s1);
	vector<int> all_syns;
	if (synsets_map_iter1 != synsets_map.end()) {
		num_syn1 = synsets_map_iter1->second;
		all_syns.insert(all_syns.end(),num_syn1.begin(),num_syn1.end());
		for (int n = 0; n < all_syns.size(); ++n) {
			vector<string> syn_str = get_map_element(synsets_strings, all_syns.at(n));
			to_return.insert(to_return.end(), syn_str.begin(), syn_str.end());
			if(debug) {
				cout << "SYNONYM:: " << endl;
				print_vector(syn_str);
			}
		}
	}
	return to_return;
}


vector<string> metric::get_hyponyms_of_noun(const string &s1, int max_sep)
{
	vector<string> to_return;

	MapStringInt::iterator synsets_map_iter1;
	vector<int> num_syn1, num_syn_new1;
	vector<int> total1, total2;
	vector<int>::iterator syn_iter1;
	int num1;
	int ret_separation = 0;
	synsets_map_iter1 = synsets_map.find(s1);
	vector<int> all_syns;
	if (synsets_map_iter1 != synsets_map.end()) {
		num_syn1 = synsets_map_iter1->second;
		all_syns.insert(all_syns.end(),num_syn1.begin(),num_syn1.end());
		syn_iter1 = num_syn1.begin();
		++ret_separation;
		while (ret_separation < max_sep) {
			for (; syn_iter1 != num_syn1.end(); ++syn_iter1) { // Evolution of s1
				vector<int> new_syn1 = get_map_element(reverse_hypernyms_map_, *syn_iter1);
				num_syn_new1.insert(num_syn_new1.begin(), new_syn1.begin(), new_syn1.end());
			}
			++ret_separation;
			num_syn1 = num_syn_new1;
			all_syns.insert(all_syns.end(),num_syn1.begin(),num_syn1.end());
			syn_iter1 = num_syn1.begin();
		}
	}

	for (int n = 0; n < all_syns.size(); ++n) {
		vector<string> syn_str = get_map_element(synsets_strings, all_syns.at(n));
		to_return.insert(to_return.end(), syn_str.begin(), syn_str.end());

		if(debug) {
			cout << "SYNSETS:: " << endl;
			print_vector(syn_str);
		}
	}
	return to_return;
}


vector<string> metric::get_hypernyms_of_verb(const string &s1, int max_sep)
{
	vector<string> to_return;

	MapStringInt::iterator synsets_map_iter1;
	vector<int> num_syn1, num_syn_new1;
	vector<int> total1, total2;
	vector<int>::iterator syn_iter1;
	int num1;

	int ret_separation = 0;
	synsets_map_iter1 = verb_synsets_map.find(s1);
	vector<int> all_syns;
	if (synsets_map_iter1 != verb_synsets_map.end()) {
		num_syn1 = synsets_map_iter1->second;
		all_syns.insert(all_syns.end(),num_syn1.begin(),num_syn1.end());
		syn_iter1 = num_syn1.begin();
		++ret_separation;
		while (ret_separation < max_sep) {
			for (; syn_iter1 != num_syn1.end(); ++syn_iter1) { // Evolution of s1
				vector<int> new_syn1 = get_map_element(verb_hypernyms_map, *syn_iter1);
				num_syn_new1.insert(num_syn_new1.begin(), new_syn1.begin(), new_syn1.end());
			}
			++ret_separation;
			num_syn1 = num_syn_new1;
			all_syns.insert(all_syns.end(),num_syn1.begin(),num_syn1.end());
			syn_iter1 = num_syn1.begin();
		}
	}

	for (int n = 0; n < all_syns.size(); ++n) {
		vector<string> syn_str = get_map_element(verb_synsets_strings, all_syns.at(n));
		to_return.insert(to_return.end(), syn_str.begin(), syn_str.end());
	}

	return to_return;
}

vector<string> metric::get_hyponyms_of_verb(const string &s1, int max_sep)
{
	vector<string> to_return;

	MapStringInt::iterator synsets_map_iter1;
	vector<int> num_syn1, num_syn_new1;
	vector<int> total1, total2;
	vector<int>::iterator syn_iter1;
	int num1;

	int ret_separation = 0;
	synsets_map_iter1 = verb_synsets_map.find(s1);
	vector<int> all_syns;
	if (synsets_map_iter1 != verb_synsets_map.end()) {
		num_syn1 = synsets_map_iter1->second;
		all_syns.insert(all_syns.end(),num_syn1.begin(),num_syn1.end());
		syn_iter1 = num_syn1.begin();
		++ret_separation;
		while (ret_separation < max_sep) {
			for (; syn_iter1 != num_syn1.end(); ++syn_iter1) { // Evolution of s1
				vector<int> new_syn1 = get_map_element(reverse_verb_hypernyms_map, *syn_iter1);
				num_syn_new1.insert(num_syn_new1.begin(), new_syn1.begin(), new_syn1.end());
			}
			++ret_separation;
			num_syn1 = num_syn_new1;
			all_syns.insert(all_syns.end(),num_syn1.begin(),num_syn1.end());
			syn_iter1 = num_syn1.begin();
		}
	}

	for (int n = 0; n < all_syns.size(); ++n) {
		vector<string> syn_str = get_map_element(verb_synsets_strings, all_syns.at(n));
		to_return.insert(to_return.end(), syn_str.begin(), syn_str.end());
	}

	return to_return;
}


double metric::hypernym(const string &s1, vector<int> num_syn1, const int &max_sep)
{
	double ret_separation;
	MapStringInt::iterator synsets_map_iter1;
	vector<int> num_syn_new1;
	vector<int> total1, total2;
	vector<int>::iterator syn_iter1;
	int num1;

	synsets_map_iter1 = synsets_map.find(s1);
	if (synsets_map_iter1 != synsets_map.end()) {
		total2 = synsets_map_iter1->second;
	} else
		return max_sep;

	ret_separation = 0;
	if (num_syn1.size() != 0) {
		syn_iter1 = num_syn1.begin();
		total1.insert(total1.begin(), num_syn1.begin(), num_syn1.end());
		if (int inter = has_intersection(total1, total2)) {
			this->intersection_ = inter;
			return ret_separation;
		}
		++ret_separation;
		while (ret_separation < max_sep) {
			for (; syn_iter1 != num_syn1.end(); ++syn_iter1) { // Evolution of s1
				vector<int> new_syn1 = get_map_element(hypernyms_map, *syn_iter1);
				num_syn_new1.insert(num_syn_new1.begin(), new_syn1.begin(), new_syn1.end());
			}
			++ret_separation;
			total1 = num_syn_new1;
			if (int inter = has_intersection(total1, total2)) {
				this->intersection_ = inter;
				return ret_separation;
			}
			num_syn1 = num_syn_new1;
			syn_iter1 = num_syn1.begin();
		}
	}

	return max_sep;
}

double metric::hypernym(const string &s1, const string &s2, const int &max_sep)
{
	double ret_separation;
	MapStringInt::iterator synsets_map_iter1;
	vector<int> num_syn1, num_syn_new1;
	vector<int> total1, total2;
	vector<int>::iterator syn_iter1;
	int num1;

	synsets_map_iter1 = synsets_map.find(s2);
	if (synsets_map_iter1 != synsets_map.end()) {
		total2 = synsets_map_iter1->second;
	} else
		return max_sep;

	ret_separation = 0;
	synsets_map_iter1 = synsets_map.find(s1);
	if (synsets_map_iter1 != synsets_map.end()) {
		num_syn1 = synsets_map_iter1->second;
		syn_iter1 = num_syn1.begin();
		total1.insert(total1.begin(), num_syn1.begin(), num_syn1.end());
		if (int inter = has_intersection(total1, total2)) {
			this->intersection_ = inter;
			return ret_separation;
		}
		++ret_separation;
		while (ret_separation < max_sep) {
			for (; syn_iter1 != num_syn1.end(); ++syn_iter1) { // Evolution of s1
				vector<int> new_syn1 = get_map_element(hypernyms_map, *syn_iter1);
				num_syn_new1.insert(num_syn_new1.begin(), new_syn1.begin(), new_syn1.end());
			}
			++ret_separation;
			total1 = num_syn_new1;
			if (int inter = has_intersection(total1, total2)) {
				this->intersection_ = inter;
				return ret_separation;
			}
			num_syn1 = num_syn_new1;
			syn_iter1 = num_syn1.begin();
		}
	}

	return max_sep;
}

double metric::verb_hypernym(const string &s1, const string &s2, const int &max_sep)
{
	double ret_separation;
	MapStringInt::iterator synsets_map_iter1;
	vector<int> num_syn1, num_syn_new1;
	vector<int> total1, total2;
	vector<int>::iterator syn_iter1;
	int num1;

	synsets_map_iter1 = verb_synsets_map.find(s2);
	if (synsets_map_iter1 != verb_synsets_map.end()) {
		total2 = synsets_map_iter1->second;
	} else
		return max_sep;

	ret_separation = 0;
	synsets_map_iter1 = verb_synsets_map.find(s1);
	if (synsets_map_iter1 != verb_synsets_map.end()) {
		num_syn1 = synsets_map_iter1->second;
		syn_iter1 = num_syn1.begin();
		total1.insert(total1.begin(), num_syn1.begin(), num_syn1.end());
		if (int inter = has_intersection(total1, total2)) {
			this->intersection_ = inter;
			return ret_separation;
		}
		++ret_separation;
		while (ret_separation < max_sep) {
			for (; syn_iter1 != num_syn1.end(); ++syn_iter1) { // Evolution of s1
				vector<int> new_syn1 = get_map_element(verb_hypernyms_map, *syn_iter1);
				num_syn_new1.insert(num_syn_new1.begin(), new_syn1.begin(), new_syn1.end());
			}
			++ret_separation;
			total1 = num_syn_new1;
			if (int inter = has_intersection(total1, total2)) {
				this->intersection_ = inter;
				return ret_separation;
			}
			num_syn1 = num_syn_new1;
			syn_iter1 = num_syn1.begin();
		}
	}

	return max_sep;
}

static inline double max(double a, double b)
{
	return a > b ? a : b;
}
static inline double min(double a, double b)
{
	return a < b ? a : b;
}

double metric::jico_helper(const string &a, const string &b, const int &max_d)
{
	double sep = separation(a, b, max_d);
	if (sep == 0)
		return 1;
	if (sep == max_d)
		return 0;
	++sep;
	vector<string> str = get_intersection();
	if (str.size() == 0)
		return 0;
	double ic1, ic2, ic12;
	double f1, f2, f12;
	vector<string> strs;
	boost::split(strs, a, boost::is_any_of("_"));
	f1 = get_map_single_element(word_freq, strs.back());
	boost::split(strs, b, boost::is_any_of("_"));
	f2 = get_map_single_element(word_freq, strs.back());
	++f1;
	++f2;
	ic1 = log(f1);
	ic2 = log(f2);
	boost::split(strs, str.at(0), boost::is_any_of("_"));

	f12 = get_map_single_element(word_freq, strs.back());
	++f12;
	ic12 = log(f12);
	double jico = ic1 + ic2 - 2 * ic12;
	double maxs = max(ic1, ic2);
	maxs = max(maxs, ic12);
	if (maxs == 0)
		return 0;
	return 1 - fabs(jico) / (2 * fabs(maxs));
}

double metric::jico_dist(const string &a, const string &b, const int &max_d)
{
	return min(jico_helper(a, b, max_d), jico_helper(b, a, max_d));
	//return jico_helper(b,a,max_d);
}

double metric::distance(const string &a, const string &b)
{
	double ic1, ic2, ic12;
	double f1, f2, f12;
	vector<string> strs;
	boost::split(strs, a, boost::is_any_of("_"));
	f1 = get_map_single_element(word_freq, strs.back());
	boost::split(strs, b, boost::is_any_of("_"));
	f2 = get_map_single_element(word_freq, strs.back());
	++f1;
	++f2;
	ic1 = log(f1);
	ic2 = log(f2);

	if(debug) {
		cout << "DISTANCE::: " << ic1 << " " << ic2 << " " << f1 << " " << f2 << endl;
	}

	if(ic1 == 0 && ic2 == 0)
		return 1;

	double hyper = (ic1 - ic2) / max(ic1, ic2);
	return 1-fabs(hyper);
}

double metric::w_jico_helper(const string &a, const string &b, const int &max_d)
{
	double sep = separation(a, b, max_d);
	if (sep == 0)
		return 1;
	if (sep == max_d)
		return 0;
	++sep;
	vector<string> str = get_intersection();
	if (str.size() == 0)
		return 0;
	double ic1, ic2, ic12;
	double f1, f2, f12;
	f1 = get_map_single_element(word_freq, a);
	f2 = get_map_single_element(word_freq, b);
	++f1;
	++f2;
	ic1 = log(f1);
	ic2 = log(f2);
	vector<string> strs;
	boost::split(strs, str.at(0), boost::is_any_of("_"));

	f12 = get_map_single_element(word_freq, strs.back());
	++f12;
	ic12 = log(f12) * sep;

	double jico = ic1 + ic2 - 2 * ic12;
	double maxs = max(ic1, ic2);
	maxs = max(maxs, ic12);
	if (maxs == 0)
		return 0;
	return 1 - fabs(jico) / (2 * fabs(maxs));
}

double metric::weighted_jico_dist(const string &a, const string &b, const int &max_d)
{
	return min(w_jico_helper(a, b, max_d), w_jico_helper(b, a, max_d));
	//return jico_helper(b,a,max_d);
}

double metric::hypernym_helper(const string &a, const string &b, const int &max_d)
{
	double sep = this->hypernym(a, b, max_d);
	if (sep == 0)
		return 1;
	if (sep == max_d)
		return 0;
	double ic1, ic2, ic12;
	double f1, f2, f12;
	vector<string> strs;

	f1 = get_map_single_element(word_freq, a);
	if (f1 == 0) {
		boost::split(strs, a, boost::is_any_of("_"));
		f1 = get_map_single_element(word_freq, strs.back());
	}

	f2 = get_map_single_element(word_freq, b);
	if (f2 == 0) {
		boost::split(strs, b, boost::is_any_of("_"));
		f2 = get_map_single_element(word_freq, strs.back());
	}
	++f1;
	++f2;
	ic1 = log(f1);
	ic2 = log(f2);
	if(debug)
		cout << ic1 << ", "<< ic2 << ", " << ic12 << endl;
	double hyper = (ic1 - ic2) / max(ic1, ic2);
	return 1 - fabs(hyper);
}

double metric::hypernym_dist(const string &a, const string &b, const int &max_d)
{
	return hypernym_helper(a, b, max_d);
}

double metric::hypernym_helper(const string &a, vector<int> b, const int &max_d)
{
	double sep = this->hypernym(a, b, max_d);
	if (sep == 0)
		return 1;
	if (sep == max_d)
		return 0;
	return 1;
}

double metric::hypernym_dist(const string &a, vector<int> b, const int &max_d)
{
	return hypernym_helper(a, b, max_d);
}

double metric::verb_jico_helper(const string &a, const string &b, const int &max_d)
{
	double sep = verb_separation(a, b, max_d);
	if (sep == 0)
		return 1;
	if (sep == max_d)
		return 0;
	++sep;
	vector<string> str = verb_get_intersection();
	if (str.size() == 0)
		return 0;
	double ic1, ic2, ic12;
	double f1, f2, f12;
	f1 = get_map_single_element(word_freq, a);
	f2 = get_map_single_element(word_freq, b);
	++f1;
	++f2;
	ic1 = log(f1);
	ic2 = log(f2);
	vector<string> strs;
	boost::split(strs, str.at(0), boost::is_any_of("_"));

	f12 = get_map_single_element(word_freq, strs.front()); // if you have "put_up", chooses only "put"
	++f12;
	ic12 = log(f12) * log(sep);

	double jico = ic1 + ic2 - 2 * ic12;
	double maxs = max(ic1, ic2);
	maxs = max(maxs, ic12);
	if (maxs == 0)
		return 0;
	return 1 - fabs(jico) / (2 * fabs(maxs));
}

double metric::verb_jico_dist(const string &a, const string &b, const int &max_d)
{
	return min(verb_jico_helper(a, b, max_d), verb_jico_helper(b, a, max_d));
}

double metric::verb_hypernym_helper(const string &a, const string &b, const int &max_d)
{
	double sep = this->verb_hypernym(a, b, max_d);
	if (sep == 0)
		return 1;
	if (sep == max_d)
		return 0;
	double ic1, ic2, ic12;
	double f1, f2, f12;
	vector<string> strs;
	boost::split(strs, a, boost::is_any_of("_"));
	f1 = get_map_single_element(word_freq, strs.back());
	boost::split(strs, b, boost::is_any_of("_"));
	f2 = get_map_single_element(word_freq, strs.back());
	++f1;
	++f2;
	ic1 = log(f1);
	ic2 = log(f2);

	double hyper = (ic1 - ic2) / max(ic1, ic2);
	return 1 - fabs(hyper);
}

double metric::verb_hypernym_dist(const string &a, const string &b, const int &max_d)
{
	return verb_hypernym_helper(a, b, max_d);
}

string metric::get_levin_verb(const string &s)
{
	MapStringString::iterator verb = verb_levin.find(s);
	if (verb != verb_levin.end())
		return verb->second;
	else
		return "";
}

string metric::get_levin_noun(const string &s)
{
	MapStringString::iterator noun = noun_levin.find(s);
	if (noun != noun_levin.end())
		return noun->second;
	else
		return "";
}

bool metric::has_synset(const string &str)
{
	MapStringInt::iterator synsets_map_iter;
	synsets_map_iter = synsets_map.find(str);
	if (synsets_map_iter != synsets_map.end())
		return true;
	return false;
}

bool metric::has_verb(const string &str)
{
	MapStringString::iterator verb_levin_iter;
	verb_levin_iter = verb_levin.find(str);
	if (verb_levin_iter != verb_levin.end())
		return true;
	return false;
}

bool metric::has_noun(const string &str)
{
	MapStringString::iterator noun = noun_levin.find(str);
	if (noun != noun_levin.end())
		return true;
	return false;
}

string metric::gender_proper_name(const string &str)
{
	vector<string> strings;
	boost::split(strings, str, boost::is_any_of("_"));
	for (int n = 0; n < strings.size(); ++n) {
		MapStringString::iterator people_names_iter;
		people_names_iter = people_names_.find(strings.at(n));
		if (people_names_iter != people_names_.end())
			return people_names_iter->second;
	}
	return "";
}

vector<int> metric::adj_lexnames(const string &adj)
// returns the lexnames of the names pertaining to the adjective
{
	vector<int> to_return;

	MapStringInt::iterator miter = adj_int_map.find(adj);

	if (miter == adj_int_map.end())
		return vector<int>(); // No adjective name found
	vector<int> mint = miter->second;
	for (int n = 0; n < mint.size(); ++n) {
		MapIntInt::iterator lexnames_iter = adj_pertainym_map.find(mint.at(n));
		if (lexnames_iter != adj_pertainym_map.end()) { // and adjective can pertain to a name (woody -> wood)
			vector<int> lexnames = lexnames_iter->second;
			to_return.insert(to_return.end(), lexnames.begin(), lexnames.end());
		} else { // if it is a derivative adjective find the original one (wooden -> woody)
			MapIntInt::iterator original_iter = adj_similar_map.find(mint.at(n));
			if (original_iter != adj_similar_map.end()) {
				vector<int> originals = original_iter->second;
				for (int m = 0; m < originals.size(); ++m) {
					MapIntInt::iterator lexnames_iter2 = adj_pertainym_map.find(originals.at(m));
					if (lexnames_iter2 != adj_pertainym_map.end()) {
						vector<int> lexnames2 = lexnames_iter2->second;
						to_return.insert(to_return.end(), lexnames2.begin(), lexnames2.end());
					}
				}
			}
		}
	}
	return to_return;
}



vector<int> metric::adv_lexnames(const string &adv)
// returns the lexnames of the names pertaining to the adjective
{
	vector<int> to_return;

	MapStringInt::iterator miter = adv_int_map.find(adv);
	if (miter == adv_int_map.end())
		return vector<int>(); // No adjective name found
	vector<int> mint = miter->second;
	for (int n = 0; n < mint.size(); ++n) {
		if (debug) {
			cout << "PERTAY2::: " << mint.at(n) << endl;
		}
		MapIntInt::iterator lexnames_iter = adv_pertainym_map.find(mint.at(n));
		if (lexnames_iter != adv_pertainym_map.end()) { // and adjective can pertain to a name (woody -> wood)
			if (debug) {
				puts("MINT:::");
				print_vector(mint);
			}
			vector<int> lexnames = lexnames_iter->second;
			to_return.insert(to_return.end(), lexnames.begin(), lexnames.end());
		}
	}
	return to_return;
}

bool metric::is_country(const string &word)
{
	MapStringBool::iterator miter = countries_map_.find(word);
	if (miter != countries_map_.end())
		return true;
	return false;
}


bool metric::is_adjective(const string &adj)
{
	MapStringInt::iterator miter= adj_int_map.find(adj);
	if (miter != adj_int_map.end())
		return true;
	return false;
}

bool metric::is_adverb(const string &adv)
{
	MapStringInt::iterator miter = adv_int_map.find(adv);
	if (miter != adv_int_map.end())
		return true;
	return false;
}

double metric::pertains_to_name(const string &adj, const string &name, int max)
{
	vector<int> lexnames = this->adj_lexnames(adj);
	return this->hypernym_dist(name, lexnames, max);
}

double metric::pertains_to_adj(const string &adv, const string &name, int max)
{
	vector<int> lexnames = this->adv_lexnames(adv);
	vector<string> adv_synsets;
	for (int n = 0; n < lexnames.size(); ++n) {
		MapIntString::iterator miter = adj_strings_map.find(lexnames.at(n));
		if (miter != adv_strings_map.end()) {
			vector<string> tmp_synsets = miter->second;
			adv_synsets.insert(adv_synsets.end(), tmp_synsets.begin(), tmp_synsets.end());
		}
	}
	for (int n = 0; n < adv_synsets.size(); ++n) {
		if (adv_synsets.at(n) == name)
			return 1;
	}
	return 0;
}

vector<string> metric::get_pertainyms(const string &adj)
{
	vector<int> lexnames = this->adj_lexnames(adj);
	vector<string> to_return;
	for (int n = 0; n < lexnames.size(); ++n) {
		MapIntString::iterator striter = synsets_strings.find(lexnames.at(n));
		if (striter != synsets_strings.end()) {
			to_return.insert(to_return.end(), striter->second.begin(), striter->second.end());
		}
	}

	return to_return;
}

vector<string> metric::get_reverse_pertainyms(const string &adj)
{
	vector<int> lexnames = this->noun_lexnames(adj);
	vector<string> to_return;
	for (int n = 0; n < lexnames.size(); ++n) {
		MapIntInt::iterator striter = reverse_adj_pertainym_map.find(lexnames.at(n));
		if (striter != reverse_adj_pertainym_map.end()) {
			vector<int> adj_int_vect = striter->second;
			for(int m=0; m < adj_int_vect.size(); ++m) {
				MapIntString::iterator striter2 = adj_strings_map.find(adj_int_vect.at(m));
				to_return.insert(to_return.end(), striter2->second.begin(), striter2->second.end());
			}
		}
	}

	return to_return;
}


vector<string> metric::get_adv_pertainyms(const string &adv)
{
	vector<int> lexnames = this->adv_lexnames(adv);
	if (debug) {
		puts("MINT2:::");
		print_vector(lexnames);
	}
	vector<string> to_return;
	for (int n = 0; n < lexnames.size(); ++n) {
		MapIntString::iterator striter = adj_strings_map.find(lexnames.at(n));
		if (striter != adj_strings_map.end()) {
			to_return.insert(to_return.end(), striter->second.begin(), striter->second.end());
		}
	}
	return to_return;
}

double metric::lexical_dist(const string &a, const string &b)
{
	vector<string> strs;
	double f1, f2, ic1, ic2;
	boost::split(strs, a, boost::is_any_of("_"));
	f1 = get_map_single_element(word_freq, strs.back());
	boost::split(strs, b, boost::is_any_of("_"));
	f2 = get_map_single_element(word_freq, strs.back());
	//if(f1 ==0 || f2==0)
	//  return 0;
	++f1;
	++f2;
	ic1 = log(f1);
	ic2 = log(f2);

	if(ic1 == 0 && ic2 == 0)
		return 1;
	double hyper = (ic1 - ic2) / max(ic1, ic2);

	return 1 - fabs(hyper);
}

bool strings_intersect(const string &head_str, const string &rhs_str)
{
	vector<string> head_strs;
	boost::split(head_strs, head_str, boost::is_any_of("|"));

	vector<string> ref_strs;
	boost::split(ref_strs, rhs_str, boost::is_any_of("|"));

	double distance = 0;
	vector<string>::const_iterator riter = ref_strs.begin();
	vector<string>::const_iterator rend = ref_strs.end();
	vector<string>::const_iterator liter = head_strs.begin();
	vector<string>::const_iterator lend = head_strs.end();

	for (; liter != lend; ++liter) {
		for (riter = ref_strs.begin(); riter != rend; ++riter) {
			if (*riter == *liter)
				return true;
		}
	}
	return false;
}

static PredTree::iterator find_data(const PredTree &ptree, const string &rhs)
{
	const PredTree::iterator end = ptree.end();
	for (PredTree::iterator here = ptree.begin(); here != end; ++here)
		if (strings_intersect(rhs, here->str))
			return here;
	return ptree.end();
}

double metric::hypernym_dist_from_trees(const string &data, const string &question,
		const vector<boost::shared_ptr<Predicate> > &ktrees, int max)
{
	double dist = 0;

	for (int k = 0; k < ktrees.size(); ++k) {
		PredTree &ptree = ktrees.at(k)->pred();
		PredTree::iterator ipred = find_data(ptree, data);
		if (ipred == ptree.end())
			continue; // data not found in this tree;
		PredTree::iterator to_the_head = ipred;
		while (to_the_head != ptree.end()) {
			if (strings_intersect(to_the_head->str, question)) {
				dist = 1;
				break; /// WRONG! You should search in all the trees
			}
			to_the_head = to_the_head.parent();
		}
	}

	return dist;
}

bool metric::is_title(const string &name)
{
	map<string, string> titles;
	titles["obe"] = "state";
	titles["cb"] = "state";
	titles["kbe"] = "state";
	titles["dfc"] = "state";

	titles["bsc"] = "academic";
	titles["msc"] = "academic";
	titles["phd"] = "academic";

	map<string, string>::iterator miter = titles.find(name);

	if (miter != titles.end()) {
		return true;
	}

	return false;
}

vector<int> metric::noun_lexnames(const string &s)
{
	vector<int> to_return;
	MapStringInt::iterator miter = synsets_map.find(s);
	if (miter != synsets_map.end()) {
		to_return = miter->second;
	}
	return to_return;
}

vector<int> metric::verb_lexnames(const string &s)
{
	vector<int> to_return;
	MapStringInt::iterator miter = verb_synsets_map.find(s);
	if (miter != verb_synsets_map.end()) {
		to_return = miter->second;
	}
	return to_return;
}

void metric::initWhat()
{
	what_vector_.push_back("!person");
	what_vector_.push_back("mission");
	what_vector_.push_back("snake");
	what_vector_.push_back("family");
	what_vector_.push_back("car");
	what_vector_.push_back("place");
	what_vector_.push_back("politician");
	what_vector_.push_back("chancellor");
	what_vector_.push_back("time");
	what_vector_.push_back("mountain");
	what_vector_.push_back("support");
	what_vector_.push_back("moon");
	what_vector_.push_back("planet");
	what_vector_.push_back("name");
	what_vector_.push_back("life");
	what_vector_.push_back("animal");
	what_vector_.push_back("hero");
	what_vector_.push_back("country");
	what_vector_.push_back("player");
	what_vector_.push_back("league");
	what_vector_.push_back("team");
	what_vector_.push_back("profession");
	what_vector_.push_back("playmaker");
	what_vector_.push_back("goalscorer");
	what_vector_.push_back("officer");
	what_vector_.push_back("medal");
	what_vector_.push_back("price");
	what_vector_.push_back("thing");
	what_vector_.push_back("love");
	what_vector_.push_back("event");
	what_vector_.push_back("happening");
	what_vector_.push_back("material");
	what_vector_.push_back("medal");
	what_vector_.push_back("road");
	what_vector_.push_back("place");
	what_vector_.push_back("company");
	what_vector_.push_back("junction");
	what_vector_.push_back("terminus");
	what_vector_.push_back("vehicle");
	what_vector_.push_back("[NNP]");
	what_vector_.push_back("thing");
}

metric::metric()
{
	this->initWhat();
}

vector<string> metric::get_what()
{
	return what_vector_;
}

string metric::getNounFromVerb(const string &s)
{
	MapStringString::iterator miter = noun_from_verb_.find(s);
	if(miter != noun_from_verb_.end() )
		return miter->second;
	return "";
}

string metric::getVerbFromNoun(const string &s)
{
	MapStringString::iterator miter = verb_from_noun_.find(s);
	if(miter != noun_from_verb_.end() )
		return miter->second;
	return "";
}
