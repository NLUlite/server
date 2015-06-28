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



#include"WisdomServer.hpp"

extern "C" {
#include"ran.h"
}

const bool debug = false;
boost::mutex io_mutex_for_writing;
boost::mutex io_mutex_for_scheduling;


static string substitute_string(string str, const string& orig, const string& replace)
{
	int pos = 0;
	while ((pos = str.find(orig, pos)) != std::string::npos) {
		str.replace(pos, orig.size(), replace);
		pos += replace.size();
	}
	return str;
}


WisdomServer::WisdomServer(int port, string dir, int nthreads, bool wikidata_proxy) :
		port_(port), data_dir_(dir)
{
	// Set listening port
	port_ = port;

	// The home directory
	home_dir_ = getenv("HOME");

	Parameters *ps = parameters_singleton::instance();
	data_dir_ = home_dir_ + "/NLUlite/data";
	if (dir == "")
		ps->setDir(data_dir_);
	else
		ps->setDir(dir);

	ps->setNumThreads(nthreads); // the default number of threads
	ps->setWikidataProxy(wikidata_proxy);
	if(wikidata_proxy) {
		// initializes wikidata info
		WikidataInfo *wdi= WikidataSingleton::instance();
	}

	// start the first wisdom
	string first_ID = "0";
	Wisdom w;
//	w.loadFile( data_dir_ + "/common.wisdom"); // Done in the Wisdom() class
	pi_ = new PhraseInfo();
	w.setPhraseInfo(pi_);
	w.ask("What smiles?");
	w_[first_ID] = w;
	id_list_.push_back(first_ID);
	wisdom_last_num_ = 0;

	// Allocate the Writer*
	writer_ = new Writer(w.getKnowledge());

	server_is_available_ = new bool(true);

}

WisdomServer::~WisdomServer()
{
	delete pi_;
	delete writer_;
	delete server_is_available_;
}

template<class T>
static void print_vector(std::stringstream &ss, const std::vector<T> &vs)
{
	typename vector<T>::const_iterator tags_iter = vs.begin();
	while (tags_iter < vs.end()) {
		ss << (*tags_iter) << " ";
		++tags_iter;
	}
}

string WisdomServer::get_string_answer_with_clauses(const vector<Answer> &answers)
{
	vector<Answer>::const_iterator aiter = answers.begin();
	vector<Answer>::const_iterator aend = answers.end();

	std::stringstream ret_stream;

	for (; aiter != aend; ++aiter) {
		vector<DrtPred> answer = aiter->getDrsAnswer();
		vector<vector<clause_vector> > clauses = aiter->getClauseHistory();
		drt matching_drt = aiter->getDrtAnswer();
		double w = aiter->getWeigth();

		ret_stream << "<div id=item>" << endl;
		ret_stream << "<answer>" << endl;
		print_vector(ret_stream, answer);
		ret_stream << writer_->write(answer);
		ret_stream << "</br>" << endl;
		ret_stream << "<answer>" << endl;

		// write the clauses
		for (int n = 0; n < clauses.size(); ++n) {
			ret_stream << "<clauselevel>" << endl;
			vector<clause_vector> clevel = clauses.at(n);
			for (int j = 0; j < clevel.size(); ++n) {
				ret_stream << "<clause>" << endl;
				ret_stream << "<a title=\"";
				ret_stream << clevel.at(j).getLink();
				ret_stream << "\"" << endl;
				ret_stream << " href=\"";
				ret_stream << clevel.at(j).getLink();
				ret_stream << "\">" << endl;
				ret_stream << writer_->write(clevel.at(j));
				ret_stream << "</a>" << endl;
				ret_stream << "</clause>" << endl;
			}
			ret_stream << "</clauselevel>" << endl;
		}
		ret_stream << "<clauselevel>" << endl;
		ret_stream << "<clause>" << endl;
		ret_stream << "<a title=\"";
		ret_stream << matching_drt.getLink();
		ret_stream << "\"" << endl;
		ret_stream << " href=\"";
		ret_stream << matching_drt.getLink();
		ret_stream << "\">" << endl;
		ret_stream << matching_drt.getText();
		ret_stream << "</a>" << endl;
		ret_stream << "</clause>" << endl;
		ret_stream << "</clauselevel>" << endl;
		ret_stream << "</div id=item>" << endl;
	}

	string ret_str = ret_stream.str();
	return ret_str;
}

static inline vector<clause_vector> flatten_rules(const vector<vector<clause_vector> > &cvs)
{
	vector<clause_vector> to_return;

	for (int n = 0; n < cvs.size(); ++n) {
		to_return.insert(to_return.end(), cvs.at(n).begin(), cvs.at(n).end());
	}

	return to_return;
}

static void print_vector_stream(std::stringstream &ff, const std::vector<DrtPred> &vs)
{
	vector<DrtPred>::const_iterator tags_iter = vs.begin();
	while (tags_iter < vs.end()) {
		tags_iter->print(ff);
		if (boost::next(tags_iter) != vs.end())
			ff << ", ";
		++tags_iter;
	}
}

static string xml_substitutions(string text)
{
	text = substitute_string(text,"&","&amp;");
	text = substitute_string(text,"\"","&quot;");
	text = substitute_string(text,"\'","&apos;");
	text = substitute_string(text,"<","&lt;");
	text = substitute_string(text,">","&gt;");
	return text;
}

string WisdomServer::get_string_answer(const ArbiterAnswer &ra, const string &question_ID_str)
{
	vector<Answer> answers = ra.getAnswers();
	string status_str = ra.getStatus();

	vector<Answer>::const_iterator aiter = answers.begin();
	vector<Answer>::const_iterator aend = answers.end();

	std::stringstream ret_stream;

	ret_stream << "<answers>" << endl;
	ret_stream << "<qID>" << endl;
	ret_stream << question_ID_str << endl;
	ret_stream << "</qID>" << endl;

	ret_stream << "<status>" << endl;
	ret_stream << status_str << endl;
	ret_stream << "</status>" << endl;

	for (; aiter != aend; ++aiter) {
		vector<DrtPred> answer = aiter->getDrsAnswer();
		vector<vector<clause_vector> > clauses = aiter->getClauseHistory();
		drt matching_drt = aiter->getDrtAnswer();
		double w = aiter->getWeigth();
		vector<pair<string, string> > WP_answers = aiter->getWPAnswer();
		vector<vector<clause_vector> > rules_tmp = aiter->getClauseHistory();
		vector<drt> clause_answer = aiter->getClauseAnswer();
		vector<clause_vector> rules_answer = flatten_rules(rules_tmp);

		ret_stream << "<item>" << endl;

		ret_stream << "<text>";
		string text_str = matching_drt.getText();
		text_str = xml_substitutions(text_str);
		ret_stream << text_str;
		ret_stream << "</text>" << endl;

		ret_stream << "<link>";
		string link_str = matching_drt.getLink();
		link_str = xml_substitutions(link_str);
		ret_stream << link_str;
		ret_stream << "</link>" << endl;

		ret_stream << "<drs>";
		print_vector_stream(ret_stream, matching_drt.predicates_with_references());
		ret_stream << "</drs>" << endl;

		ret_stream << "<weight>";
		ret_stream << w;
		ret_stream << "</weight>" << endl;

		ret_stream << "<data>" << endl;
		for (int n = 0; n < WP_answers.size(); ++n) {
			ret_stream << "<dataitem>" << endl;
			ret_stream << "<WP>";
			ret_stream << WP_answers.at(n).first;
			ret_stream << "</WP>";
			ret_stream << "<name>";
			ret_stream << WP_answers.at(n).second;
			ret_stream << "</name>";
			ret_stream << "</dataitem>" << endl;
		}
		ret_stream << "</data>" << endl;

		ret_stream << "<rules>" << endl;

		for (int n = 0; n < clause_answer.size(); ++n) {
			ret_stream << "<ruleitem>" << endl;
			ret_stream << "<text>";
			ret_stream << clause_answer.at(n).getText();
			ret_stream << "</text>" << endl;
			ret_stream << "<link>";
			ret_stream << clause_answer.at(n).getLink();
			ret_stream << "</link>" << endl;
			ret_stream << "</ruleitem>" << endl;
		}
		for (int n = 0; n < rules_answer.size(); ++n) {
			ret_stream << "<ruleitem>" << endl;
			ret_stream << "<text>";
			ret_stream << rules_answer.at(n).getText();
			ret_stream << "</text>" << endl;
			ret_stream << "<link>";
			ret_stream << rules_answer.at(n).getLink();
			ret_stream << "</link>" << endl;
			ret_stream << "</ruleitem>" << endl;
		}

		ret_stream << "</rules>" << endl;

		ret_stream << "</item>" << endl;
	}

	ret_stream << "</answers>" << endl;

	string ret_str = ret_stream.str();
	return ret_str;
}

static bool is_valid_input(const string &str)
{
	if (str.size())
		return true;
	return false;
}

static int extract_ID(const string &str)
{
	int to_return = 0;
	int first, last;
	first = str.find("ID=") + 3;
	last = str.find(' ', first);
	//cout << "last:: " << last << endl;
	if (last == string::npos)
		last = str.find('>', first);
	try {
		to_return = boost::lexical_cast<int>(str.substr(first, last - first));
	} catch (std::exception &e) {
		//cout << "id:: " << str.substr(first,last-first) << endl;
		//std::cerr << e.what() << endl;
	}

	return to_return;
}

static string extract_string(const string &str, const string &key)
{
	string to_return;
	int first, last;
	string kstr = key + "=";
	first = str.find(kstr) + kstr.size();
	last = str.find(' ', first);
	if (last == string::npos)
		last = str.find('>', first);
	to_return = str.substr(first, last - first);

	return to_return;
}

static string extract_key(const string &str)
{
	string to_return;
	int first, last;
	first = str.find("key=") + 4;
	last = str.find(' ', first);
	if (last == string::npos)
		last = str.find('>', first);
	to_return = str.substr(first, last - first);
	//cout << "key:: " << str.substr(first,last-first) << endl;

	return to_return;
}

static string extract_password(const string &str)
{
	string to_return;
	int first, last;
	first = str.find("passwd=") + 7;
	last = str.find(' ', first);
	if (last == string::npos)
		last = str.find('>', first);
	to_return = str.substr(first, last - first);

	return to_return;
}

static int extract_time(const string &str)
{
	string to_return;
	int value = -1;
	int first, last;
	first = str.find("timer=") + 6;
	last = str.find(' ', first);
	if (last == string::npos)
		last = str.find('>', first);
	to_return = str.substr(first, last - first);
	if (debug)
		cout << "deadline:: " << str.substr(first, last - first) << endl;

	try {
		value = boost::lexical_cast<int>(to_return);
	} catch (std::exception &e) {
	}

	return value;
}

static string get_link(string line)
{
	line.erase(0, 2);
	int pos_end = line.find("%]");
	line.erase(pos_end, 2);

	string to_return = line;

	return to_return;
}

void WisdomServer::create_discourses_from_string(string str, int global_num, Wisdom *w)
{
	if(w == 0)
		return;

	vector<drt_collection> to_return;
	string data = "";
	string link, old_link;
	char c_line[50000];

	// the element like [% ... %] are put in a separate line
	int opening, closing = 0;
	while (true) {
		opening = str.find("[%", closing);
		if (opening == string::npos)
			break;
		closing = str.find("%]", opening);
		str.insert(opening, "\n");
		str.insert(closing + 3, "\n");
	}

	std::stringstream ss(str);
	bool first = true;
	while (ss.good()) {
		ss.getline(c_line, 50000);
		string line(c_line);
		if (line.find("[%") != string::npos && line.rfind("%]") != string::npos) {
			link = get_link(line);
			if (data.size()) {
			     drt_collection *dc;
			     if (link.size()) {
				  dc = new drt_collection(data, global_num++, Context(), pi_, old_link);
			     } else {
				  dc= new drt_collection(data, global_num++, Context(), pi_);
			     }
			     //to_return.push_back(dc);
			     w->addDiscourse(*dc);
			     delete dc;
			     data = "";
			     old_link = link;
			}
		} else {
			data.append(line + "\n");
			old_link = link;
		}
	}
	if (data.size()) {
		drt_collection dc(data, global_num, Context(), pi_, link);
		w->addDiscourse(dc);
	}
}

static string get_new_question_ID()
{
	string to_return;

	int num_ID = iran(), safe = 0;
	string str_ID = boost::lexical_cast<string>(num_ID);

	num_ID = iran();
	str_ID = boost::lexical_cast<string>(num_ID);

	to_return = str_ID;

	return to_return;
}

string WisdomServer::get_new_id()
{
	string to_return;

	int num_ID = iran(), safe = 0;
	string str_ID = boost::lexical_cast<string>(num_ID);

	while (find(id_list_.begin(), id_list_.end(), str_ID) != id_list_.end()) {
		num_ID = iran();
		str_ID = boost::lexical_cast<string>(num_ID);
		++safe;
		if (safe > 100000) // maximum number of attempts
			throw(std::runtime_error("cannot find an ID for the new Wisdom)"));
	}

	id_list_.push_back(str_ID);
	to_return = str_ID;

	return to_return;
}

string WisdomServer::get_new_writer_id()
{
	string to_return;

	int num_ID = iran(), safe = 0;
	string str_ID = boost::lexical_cast<string>(num_ID);

	while (find(writer_id_list_.begin(), writer_id_list_.end(), str_ID) != writer_id_list_.end()) {
		num_ID = iran();
		str_ID = boost::lexical_cast<string>(num_ID);
		++safe;
		if (safe > 100000) // maximum number of attempts
			throw(std::runtime_error("cannot find an ID for the new Wisdom)"));
	}
	writer_id_list_.push_back(str_ID);
	to_return = str_ID;

	return to_return;
}

static vector<DrtPred> string_to_drs_cast(const string &s)
{
	string str = s;
	boost::erase_all(str, string(" ")); // strip all the spaces from the string

	// saving the consequence
	int p, size = str.size();
	int depth = 0;
	int p1 = 0, p2;
	// the string
	vector<string> strs;
	for (p = 0; p < size; ++p) {
		if (str.at(p) == '(')
			++depth;
		if (str.at(p) == ')')
			--depth;
		if (depth < 0)
			throw(std::invalid_argument("Badly formed drs."));
		if (str.at(p) == ',' && depth == 0) {
			p2 = p;
			strs.push_back(str.substr(p1, p2 - p1));
			p1 = p2 + 1;
		}
	}
	strs.push_back(str.substr(p1, p - p1));

	vector<string>::iterator si = strs.begin();
	vector<string>::iterator se = strs.end();
	vector<DrtPred> to_return;
	to_return.resize(strs.size());
	vector<DrtPred>::iterator hi = to_return.begin();
	for (; si != se; ++si, ++hi) {
		*hi = DrtPred(*si);
	}

	return to_return;
}

static string peel_eof(string input)
{
	int pos = input.find("<eof>");
	if (pos != string::npos)
		input.erase(pos, 5);
	return input;
}

string WisdomServer::process_input(string input)
{
	input = peel_eof(input);

	if (input.find("<question") == 0) {
		int sq, eq = input.size();
		sq = input.find('>');
		string intro = input.substr(0, sq);
		string question = input.substr(sq + 1, eq);
		if (question.size() && question.at(question.size() - 1) == '\n')
			question.at(question.size() - 1) = '?';
		else
			question += '?';
		int ID = extract_ID(intro);

		string ID_str = boost::lexical_cast<string>(ID);

		string question_ID_str = get_new_question_ID();
		ArbiterAnswer ra = w_[ID_str].ask(question, question_ID_str); // compute the question
		string answer_str = get_string_answer(ra, question_ID_str); // Format the answer as a readable string

		return answer_str;

	} else if (input.find("<wikidata_question ") == 0) {
		int sq, eq = input.size();
		sq = input.find('>');
		string intro = input.substr(0, sq);
		string question = input.substr(sq + 1, eq);
		if (question.size() && question.at(question.size() - 1) == '\n')
			question.at(question.size() - 1) = '?';
		else
			question += '?';
		int ID = extract_ID(intro);

		string ID_str = boost::lexical_cast<string>(ID);

		string question_ID_str = get_new_question_ID();
		ArbiterAnswer ra = w_[ID_str].askWikidata(question, question_ID_str); // compute the question
		string answer_str = get_string_answer(ra, question_ID_str); // Format the answer as a readable string

		return answer_str;

	} else if (input.find("<wisdom_question ") == 0) {
		int sq, eq = input.size();
		sq = input.find('>');
		string intro = input.substr(0, sq);

		string ID_str      = extract_string(intro, "ID");
		string from_ID_str = extract_string(intro, "from_ID");


		string question_ID_str = get_new_question_ID();

		ArbiterAnswer ra = w_[ID_str].ask(w_[from_ID_str].getQuestions(), question_ID_str); // compute the question
		string answer_str = get_string_answer(ra, question_ID_str); // Format the answer as a readable string


		return answer_str;

	} else if (input.find("<match ") == 0) {
		int sq, eq = input.size();
		sq = input.find('>');
		string intro = input.substr(0, sq);
		string question = input.substr(sq + 1, eq);
		int ID = extract_ID(intro);
		string ID_str = boost::lexical_cast<string>(ID);

		string question_ID_str = get_new_question_ID();
		ArbiterAnswer ra = w_[ID_str].match(question, question_ID_str); // compute the question
		string answer_str = get_string_answer(ra, question_ID_str); // Format the answer as a readable string
		return answer_str;

	} else if (input.find("<match_drs ") == 0) {
		int sq, eq = input.size();
		sq = input.find('>');
		string intro = input.substr(0, sq);
		string data = input.substr(sq + 1, eq);
		vector<string> strs;
		boost::split(strs, data, boost::is_any_of(";"));
		if (strs.size() != 2)
			return "";
		string drs_str = strs.at(0);
		string question = strs.at(1);
		DrtVect drtvect = create_drtvect(drs_str);
		drt drs(drtvect);


		int ID = extract_ID(intro);
		string ID_str = boost::lexical_cast<string>(ID);

		string question_ID_str = get_new_question_ID();
		ArbiterAnswer ra = w_[ID_str].matchDrsWithText(drs, question, question_ID_str); // compute the question
		string answer_str = get_string_answer(ra, question_ID_str); // Format the answer as a readable string
		return answer_str;

	} else if (input.find("<erase_wisdom ") == 0) {
		int sd, ed = input.size();
		sd = input.find('>');
		string intro = input.substr(0, sd);
		int ID = extract_ID(intro);
		string ID_str = boost::lexical_cast<string>(ID);
		w_.erase(ID_str);
		Wisdom wtmp;
		wtmp.setPhraseInfo(pi_);
		w_[ID_str] = wtmp;
		return ID_str;

	} else if (input.find("<data") == 0) {
		int sd, ed = input.size();
		sd = input.find('>');
		string intro = input.substr(0, sd);
		string data = input.substr(sd + 1, ed);


		if (data.size() && data.at(data.size() - 1) == '\n')
			data.at(data.size() - 1) = '.';
		else
			data += '.';
		int ID = extract_ID(intro);
		string ID_str = boost::lexical_cast<string>(ID);
		++global_num_;
		create_discourses_from_string(data, global_num_,&w_[ID_str]);
		return "<ok>";

	} else if (input.find("<publish") == 0) {
		int sd, ed = input.size();
		sd = input.find('>');
		string intro = input.substr(0, sd);
		string data = input.substr(sd + 1, ed);
		int ID = extract_ID(intro);
		string key = extract_key(intro);
		string passwd = extract_password(intro);
		int deadline = extract_time(intro);
		string ID_str = boost::lexical_cast<string>(ID);

		if (debug)
			cout << "DD::: " << deadline << " " << key << endl;

		map<string, string>::iterator kiter = keys_.find(key);
		if (kiter == keys_.end()) {
			keys_[key] = ID_str;
			password_[key] = passwd;
			if (deadline != -1) {
				int start_time = time(0);
				timer_.push_back(boost::make_tuple(start_time, deadline, key));
			}
			return "<ok>";
		} else
			return "<error>";

	} else if (input.find("<get_published") == 0) {
		int sd, ed = input.size();
		sd = input.find('>');
		string intro = input.substr(0, sd);
		string data = input.substr(sd + 1, ed);
		int ID = extract_ID(intro);
		string key = extract_key(intro);
		string ID_str = boost::lexical_cast<string>(ID);
		map<string, string>::iterator kiter = keys_.find(key);
		if (kiter == keys_.end()) {
			return "<error>";
		} else {
			w_.erase(ID_str); // the old wisdom is deleted
			string new_ID = kiter->second;
			return new_ID;
		}

	} else if (input.find("<erase_published") == 0) {
		int sd, ed = input.size();
		sd = input.find('>');
		string intro = input.substr(0, sd);
		string data = input.substr(sd + 1, ed);
		string key = extract_key(intro);
		string passwd = extract_password(intro);
		map<string, string>::iterator kiter = keys_.find(key);
		if (kiter == keys_.end()) {
			return "<error>";
		} else if (password_[key] == passwd) {
			keys_.erase(key); // the old wisdom is deleted
			return "<ok>";
		}
		return "<error>";

	} else if (input.find("<list_published>") == 0) {
		string text = "<answer>\n";
		map<string, string>::iterator kiter = keys_.begin();
		for (; kiter != keys_.end(); ++kiter) {
			text += "<item>";
			text += kiter->first;
			text += "</item>\n";
		}
		text += "</answer>\n";
		return text;

	} else if (input.find("<save ") == 0) {
		int sd, ed = input.size();
		sd = input.find('>');
		string intro = input.substr(0, sd);
		int ID = extract_ID(intro);
		string ID_str = boost::lexical_cast<string>(ID);
		string save_str = w_[ID_str].writeString(); // save the wisdom to a string
		return save_str;

	} else if (input.find("<save_rdf ") == 0) {
		int sd, ed = input.size();
		sd = input.find('>');
		string intro = input.substr(0, sd);
		int ID = extract_ID(intro);
		string ID_str = boost::lexical_cast<string>(ID);
		string save_str = w_[ID_str].writeStringRDF(); // save the wisdom to a string
		return save_str;

	} else if (input.find("<load") == 0) {
		int sd, ed = input.size();
		sd = input.find('>');
		string intro = input.substr(0, sd);
		string data = input.substr(sd + 1, ed);
		int ID = extract_ID(intro);
		string ID_str = boost::lexical_cast<string>(ID);
		w_[ID_str].loadString(data); // load the wisdom from a string
		return "<ok>";

	} else if (input.find("<wisdom_parameters ") == 0) {
		int sd, ed = input.size();
		sd = input.find('>');
		string intro = input.substr(0, sd);
		int ID = extract_ID(intro);
		string ID_str = boost::lexical_cast<string>(ID);
		int accuracy_level             = boost::lexical_cast<int>(extract_string(intro, "accuracy_level"));
		int num_answers                = boost::lexical_cast<int>(extract_string(intro, "num_answers"));
		string solver_options          = extract_string(intro, "solver_options");
		string skip_presuppositions    = extract_string(intro, "skip_presuppositions");
		string skip_solver             = extract_string(intro, "skip_solver");
		string add_data                = extract_string(intro, "add_data");
		string do_solver               = extract_string(intro, "do_solver");
		string timeout_str             = extract_string(intro, "timeout");
		string fixed_time_str          = extract_string(intro, "fixed_time");
		string word_intersection       = extract_string(intro, "word_intersection");
		string use_pertaynims          = extract_string(intro, "use_pertaynims");
		string max_refs_str            = extract_string(intro, "max_refs");
		string max_candidates_refs_str = extract_string(intro, "max_candidates_refs");
		string max_candidates_str      = extract_string(intro, "max_candidates");
		string use_synonyms            = extract_string(intro, "use_synonyms");
		string use_hyponyms            = extract_string(intro, "use_hyponyms");
		string num_hyponyms_str        = extract_string(intro, "num_hyponyms");
		string load_clauses            = extract_string(intro, "load_clauses");
		string implicit_verb           = extract_string(intro, "implicit_verb");

		double timeout          = boost::lexical_cast<double>(timeout_str);
		double fixed_time       = boost::lexical_cast<double>(fixed_time_str);
		int max_refs            = boost::lexical_cast<int>(max_refs_str);
		int max_candidates_refs = boost::lexical_cast<int>(max_candidates_refs_str);
		int max_candidates      = boost::lexical_cast<int>(max_candidates_str);
		int num_hyponyms        = boost::lexical_cast<int>(num_hyponyms_str);

		WisdomInfo wi;
		wi.setAccuracyLevel(accuracy_level);
		wi.setNumAnswers(num_answers);
		wi.setSolverOptions(solver_options);
		wi.setSkipPresuppositions(skip_presuppositions);
		wi.setSkipSolver(skip_solver);
		wi.setDoSolver(do_solver);
		wi.setAddData(add_data);
		wi.setTimeout(timeout);
		wi.setFixedTime(fixed_time);
		wi.setWordIntersection(word_intersection);
		wi.setUsePertaynims(use_pertaynims);
		wi.setMaxRefs(max_refs);
		wi.setMaxCandidatesRefs(max_candidates_refs);
		wi.setMaxCandidates(max_candidates);
		wi.setUseSynonyms(use_synonyms);
		wi.setUseHyponyms(use_hyponyms);
		wi.setNumHyponyms(num_hyponyms);
		wi.setLoadClauses(load_clauses);
		wi.setImplicitVerb(implicit_verb);

		w_[ID_str].setWisdomInfo(wi);

		return "<ok>";

	} else if (input.find("<erase") == 0) {
		int sd, ed = input.size();
		sd = input.find('>');
		string intro = input.substr(0, sd);
		int ID = extract_ID(intro);
		string ID_str = boost::lexical_cast<string>(ID);
		w_.erase(ID_str);
		return "<ok>";

	} else if (input.find("<writer_new ") == 0) {
		int sd, ed = input.size();
		sd = input.find('>');
		string intro = input.substr(0, sd);
		int wisdom_ID = extract_ID(intro);
		string wisdom_ID_str = boost::lexical_cast<string>(wisdom_ID);
		string writer_ID_str = get_new_writer_id();
		/// DO the check that the wisdom_ID exists
		Knowledge &k = w_[wisdom_ID_str].getKnowledge();
		writers_[writer_ID_str] = new Writer(k);
		return writer_ID_str;

	} else if (input.find("<writer_erase") == 0) {
		int sd, ed = input.size();
		sd = input.find('>');
		string intro = input.substr(0, sd);
		int writer_ID = extract_ID(intro);
		string writer_ID_str = boost::lexical_cast<string>(writer_ID);
		delete writers_[writer_ID_str];
		writers_.erase(writer_ID_str);
		return "<ok>";

	} else if (input.find("<writer_write ") == 0) {
		int sd, ed = input.size();
		sd = input.find('>');
		string intro = input.substr(0, sd);
		string data = input.substr(sd + 1, ed);
		int writer_ID = extract_ID(intro);
		string writer_ID_str = boost::lexical_cast<string>(writer_ID);
		string ret_str = writers_[writer_ID_str]->write(string_to_drs_cast(data));
		return ret_str;

	} else if (input.find("<writer_write_answer ") == 0) {
		int sd, ed = input.size();
		sd = input.find('>');
		string intro = input.substr(0, sd);
		string data = input.substr(sd + 1, ed);
		int writer_ID = extract_ID(intro);

		if (data.size() && data.at(data.size() - 1) == '\n')
			data.resize(data.size() - 1);

		vector<string> strs;
		boost::split(strs, data, boost::is_any_of(":"));

		if (debug)
			cout << "WRITE_ANSWER:::" << data << endl;

		if (strs.size() != 3)
			throw(std::runtime_error("Server: cannot find writer and answer ID."));
		string wID = strs.at(0);
		string qID = strs.at(1);
		int num_kav = boost::lexical_cast<int>(strs.at(2));
		string comment = w_[wID].getComment(qID);
		return comment;

	} else if (input.find("<server ") == 0) {
		int sd, ed = input.size();
		sd = input.find('>');
		string intro = input.substr(0, sd);
		int num_threads = boost::lexical_cast<int>(extract_string(intro, "threads"));
		Parameters *ps = parameters_singleton::instance();
		ps->setNumThreads(num_threads); // the default number of threads
		return "<ok>";
	} else
		return "<error>";
}


string WisdomServer::process_input_for_scheduling(string input)
{
	input = peel_eof(input);


	if (input.find("<test>") == 0) {
		return "<ok>";

	} else if (input.find("<new_wisdom") == 0) {

		//boost::mutex::scoped_lock lock(io_mutex_for_scheduling); // the Wisdom class is not thread-safe
		string ID_str = get_new_id();
		Wisdom wtmp;
		wtmp.setPhraseInfo(pi_);
		w_[ID_str] = wtmp;
		return ID_str;

	} else if (input.find("<new_wikidata") == 0) {

			//boost::mutex::scoped_lock lock(io_mutex_for_scheduling); // the Wisdom class is not thread-safe
			string ID_str = get_new_id();
			Wisdom wtmp;
			WisdomInfo wi;
			Parameters *ps = parameters_singleton::instance();
			if(ps->getWikidataProxy()) {
				wi.setWikidata("true");
				wtmp.setPhraseInfo(pi_);
				wtmp.setWisdomInfo(wi);
				w_[ID_str] = wtmp;
				return ID_str;
			} else {
				return "<error>";
			}

	} else if (input.find("<is_available") == 0) {
		if( *server_is_available_ )
			return "<ok>";
		else
			return "<error>";

	} else
		return "<error>";
}


void WisdomServer::eraseExpired()
{
	for (int n = 0; n < timer_.size(); ++n) {
		int start_time = timer_.at(n).get<0>();
		int deadline = timer_.at(n).get<1>();
		string key = timer_.at(n).get<2>();
		int current_time = time(0);
		map<string, string>::iterator kiter = keys_.find(key);
		if (kiter != keys_.end() && current_time - start_time > deadline) {
			keys_.erase(key); // the old wisdom is deleted
		}
	}
}


void WisdomServer::handleConnection(boost::asio::io_service *ioservice, tcp::acceptor *acceptor)
{
	while (true) {
		try {
			tcp::socket socket(*ioservice);
			acceptor->accept(socket);

			// Reads the client's question
			boost::system::error_code ignored_error;
			boost::asio::streambuf received_buf;
			boost::asio::read_until(socket, received_buf, "<eof>");
			std::istream received(&received_buf);
			string question = "";
			char c_str[50000];
			while (received.good()) {
				received.getline(c_str, 50000);
				string tmp_str(c_str);
				question += tmp_str + "\n";
			}

			if (!is_valid_input(question))
				continue;

			string answer_str;

			// Try to process commands that do not need mutex
			answer_str = process_input_for_scheduling(question);
			if(answer_str.size() && answer_str != "<error>") {
				if (answer_str.size())
					boost::asio::write(socket, boost::asio::buffer(answer_str), boost::asio::transfer_all(), ignored_error);
				continue; // no need to lock the system
			} else {
				// Otherwise process the commands that need mutex conditions
				boost::mutex::scoped_lock lock(io_mutex_for_writing); // the Wisdom class is not thread-safe

				// Erase expired wisdoms
				this->eraseExpired();

				*server_is_available_ = false;
				answer_str = process_input(question);
				*server_is_available_ = true;
				// Send the formatted answer back
				if (answer_str.size())
					boost::asio::write(socket, boost::asio::buffer(answer_str), boost::asio::transfer_all(), ignored_error);
			}
			// The following is necessary in the final version!!!
		} catch (std::exception &e) {
			///
		}
	}
}


void WisdomServer::run()
{
	boost::asio::io_service ioservice;

	tcp::acceptor *acceptor;
	try {
		acceptor = new tcp::acceptor(ioservice, tcp::endpoint(tcp::v4(), port_));
	} catch (std::exception &e) {
		cout << e.what() << endl;
		cout << "Server terminated." << endl << endl;
		exit(-1);
	}
	global_num_ = 0;

	this->handleConnection(&ioservice, acceptor);

	delete acceptor;
}
