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



#include"drt_collection.hpp"

const bool debug = false;
const bool activate_context = false;
const bool commercial_version = true;
boost::mutex io_mutex_counter;

template<class T>
static void print_pair_vector(std::vector<T> &vs)
{
	typename vector<T>::iterator tags_iter = vs.begin();
	while (tags_iter < vs.end()) {
		std::cout << tags_iter->first << "," << tags_iter->second << " ";
		++tags_iter;
	}
	std::cout << std::endl;
}

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
static void print_vector(std::vector<T> &vs, std::stringstream &ss)
{
	typename vector<T>::iterator tags_iter = vs.begin();
	while (tags_iter < vs.end()) {
		ss << (*tags_iter);
		if (boost::next(tags_iter) != vs.end())
			ss << ", ";
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

static string clear_text(const string &text)
{
	string to_return;

	for (int i = 0; i < text.size(); ++i) {
		if (text.at(i) != '\r')
			to_return.push_back(text.at(i));
	}

	return to_return;
}

static void switch_children(DrtPred &pred)
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

void drt_collection::add_phrase(const phrase_versions &pv)
{
	//boost::mutex::scoped_lock lock(io_mutex_drt); // phrases are added concurrently
	phrase_collection_.push_back(pv);
	drt drt_tmp = pv.get_most_likely_drt();
	drt_tmp.apply_phrase_number(global_num_); // apply the global reference number to the drts
	drt_tmp.setText(pv.get_text());
	drt_tmp.setLink(link_);
	/// here you should push the link
	drt_collection_.push_back(drt_tmp);
}

static string pre_clean_text(string text)
{
	Filter filter(text);
	text = filter.str();

	int size = text.size();

	bool trigger_quote = false;

	for (int i = 0; i < size; ++i) {
		if (text.at(i) == '.' && i < size - 3 && text.at(i + 1) == '.' && text.at(i + 2) == '.') {
			text.at(i) = ' ';
			text.at(i + 1) = ' ';
			text.at(i + 2) = ' ';
			i = 0;
			continue;
		}
		if (text.at(i) == '_'
			&& ( (i > 0 && text.at(i-1) == ' ')
				|| (i > 0 && text.at(i-1) == '\r')
				|| (i > 0 && text.at(i-1) == '\n')
				|| (i > 0 && text.at(i-1) == '.')
				|| (i > 0 && text.at(i-1) == ';')
				|| (i > 0 && text.at(i-1) == ':')
				|| (i > 0 && text.at(i-1) == ',')
				|| (i > 0 && text.at(i-1) == '!')
				|| (i > 0 && text.at(i-1) == '?')
				|| (i > 0 && text.at(i-1) == ')')
				|| (i > 0 && text.at(i-1) == ']')
				|| (i > 0 && text.at(i-1) == '}')
				|| (i > 0 && text.at(i-1) == '(')
				|| (i > 0 && text.at(i-1) == '[')
				|| (i > 0 && text.at(i-1) == '{')
			     || i == 0  )
		) {
			text.at(i) = '-';
		}
		if (trigger_quote && i < size - 1 && text.at(i) == '.' && text.at(i + 1) == '\r') {
			trigger_quote = false;
			text.insert(i, "\"");
			++i;
			continue;
		}
		if (trigger_quote && i < size - 1 && text.at(i) != '.' && text.at(i) != '?' && text.at(i) != ':' && text.at(i) != ','
				&& text.at(i) != ';' && text.at(i) != '!' && text.at(i + 1) == '\n') {
			trigger_quote = false;
			text.insert(i, ".");
			++i;
			continue;
		}
		if (!trigger_quote && i < size - 1 && text.at(i) != '.' && text.at(i) != '?' && text.at(i) != ':' && text.at(i) != ','
				&& text.at(i) != ';' && text.at(i) != '!' && text.at(i + 1) == '\r') {
			trigger_quote = false;
			text.insert(i + 1, ".");
			++i;
			continue;
		}
		if (!trigger_quote && i < size - 1 && text.at(i) != '.' && text.at(i + 1) == '\n') {
			trigger_quote = false;
			text.insert(i + 1, ".");
			++i;
			continue;
		}

		if (trigger_quote && text.at(i) == '.' && i < size - 1 && text.at(i + 1) == '\"') {
			text.at(i) = '\"';
			text.at(i + 1) = '.';
		}
		if (trigger_quote && text.at(i) == ',' && i < size - 1 && text.at(i + 1) == '\"') {
			text.at(i) = '\"';
			text.at(i + 1) = ',';
		}
		if (trigger_quote && text.at(i) == '!' && i < size - 1 && text.at(i + 1) == '\"') {
			text.at(i) = '\"';
			text.at(i + 1) = '!';
		}
		if (trigger_quote && text.at(i) == '?' && i < size - 1 && text.at(i + 1) == '\"') {
			text.at(i) = '\"';
			text.at(i + 1) = '?';
		}
		if (trigger_quote && text.at(i) == '\"') {
			trigger_quote = false;
			++i;
			continue;
		}
		if (!trigger_quote && text.at(i) == '\"') {
			trigger_quote = true;
		}
		if (trigger_quote && (text.at(i) == '.' || text.at(i) == ':' || text.at(i) == ';' || text.at(i) == '?')) {
			text.insert(i, 1, '\"');
			text.insert(i + 2, 1, '\"');
			trigger_quote = false;
			++i;
		}
		if (i < size - 2 && isdigit(text.at(i)) && text.at(i + 1) == '.' && isdigit(text.at(i + 2))) {
			text.replace(i + 1, 1, "\\dot");
		}
		if (i < size - 2 && isdigit(text.at(i)) && text.at(i + 1) == ':' && isdigit(text.at(i + 2))) {
			text.replace(i + 1, 1, "\\colon");
		}
		if (i < size - 2 && isdigit(text.at(i)) && text.at(i + 1) == ',' && isdigit(text.at(i + 2))) {
			text.erase(i + 1, 1);
			size = text.size();
		}
	}
	return text;
}

static string post_clean_text(string text)
{
	int pos;
	while (true) {
		pos = text.find("\\dot");
		if (pos != string::npos)
			text.replace(pos, 4, ".");
		else
			break;
	}

	while (true) {
		pos = text.find("\\colon");
		if (pos != string::npos)
			text.replace(pos, 6, ":");
		else
			break;
	}
	return text;
}

static string erase_par(string str)
// erases everything between parenthesis
{
	int i = 0;
	bool erase_trigger = false;
	while (i < str.size()) {
		// erase everything between []
		if (str.at(i) == '[')
			erase_trigger = true;
		if (erase_trigger && str.at(i) == ']') {
			str.erase(i, 1);
			erase_trigger = false;
			continue;
		}
		if (erase_trigger) {
			str.erase(i, 1);
			continue;
		}
		++i;
	}

	i = 0;
	erase_trigger = false;
	while (i < str.size()) {
		// erase everything between ()
		if (str.at(i) == '(')
			erase_trigger = true;
		if (erase_trigger && str.at(i) == ')') {
			str.erase(i, 1);
			erase_trigger = false;
			continue;
		}
		if (erase_trigger) {
			str.erase(i, 1);
			continue;
		}
		++i;
	}

	i = 0;
	erase_trigger = false;
	while (i < str.size()) {
		// erase everything between {}
		if (str.at(i) == '{')
			erase_trigger = true;
		if (erase_trigger && str.at(i) == '}') {
			if (i < str.size() - 1 && str.at(i + 1) == '}') { // take care of {{ .. }}
				++i;
				continue;
			}
			str.erase(i, 1);
			erase_trigger = false;
			continue;
		}
		if (erase_trigger) {
			str.erase(i, 1);
			continue;
		}
		++i;
	}

	return str;
}

static string process_abbreviations(string str)
{
	map<string, string> abbr;
	abbr["No."] = "number";
	abbr["Mr."] = "Mister";
	abbr["Dr."] = "Doctor";
	abbr["Ms."] = "Ms";
	abbr["St."] = "Saint";
	abbr["Mt."] = "Mount";
	abbr["Mrs."] = "Mrs";
	abbr["i.e."] = "which means";
	abbr["e.g."] = "eg";
	abbr["vs."] = "against";
	abbr["Vs."] = "against";
	abbr["U.S."] = "USA";

	map<string, string>::iterator aiter = abbr.begin();
	map<string, string>::iterator aend = abbr.end();
	for (; aiter != aend; ++aiter) {
		bool do_continue = true;
		while (do_continue) {
			string abbr_str = aiter->first;
			int pos = str.find(abbr_str);
			if (pos == string::npos)
				do_continue = false;
			else {
				string repl_str = aiter->second;
				str.replace(pos, abbr_str.size(), repl_str);
				do_continue = true;
			}
		}
	}
	for (int i = 0; i < str.size(); ++i) {
		if (i > 1 && str.at(i) == '.' && std::isupper(str.at(i - 1)) && (str.at(i - 2) == ' ' || str.at(i - 2) == '\"'))
			str.at(i) = ' ';
	}

	// 7" -> 7min
	for (int i = 1; i < str.size(); ++i) {
		if (str.at(i) == '\"' && std::isdigit(str.at(i - 1)))
			str.replace(i, 1, "min");
	}

	return str;
}

drt_collection::~drt_collection()
{
	//     delete anaphora_;
}

static inline int max(int a, int b)
{
	return a < b ? b : a;
}
static inline int min(int a, int b)
{
	return a >= b ? b : a;
}

class ThreadCounter {
	int num_, max_num_;

public:
	ThreadCounter(int max_num) :
			max_num_(max_num), num_(0)
	{
	}
	int getCurrentNumber();
};

int ThreadCounter::getCurrentNumber()
{
	boost::mutex::scoped_lock lock(io_mutex_counter); // this method is used concurrently
	if (num_ < max_num_) {
		++num_;
		return num_ - 1;
	}
	return -1;
}

class ParserThread {

	vector<string> *input_;
	vector<phrase_versions> *output_;
	vector<Context> *context_vect_;
	ThreadCounter *counter_;
	PhraseInfo *pi_;
	WisdomInfo *wi_;

	void parseSingleSentence(const string &text, PhraseInfo *pi, WisdomInfo *wi, Context *context, phrase_versions *return_phrase);

public:
	ParserThread(vector<string> *input, vector<phrase_versions> *output, vector<Context> *context_vect, ThreadCounter *counter,
			PhraseInfo *pi, WisdomInfo *wi) :
			input_(input), output_(output), context_vect_(context_vect), counter_(counter), pi_(pi), wi_(wi)
	{
	}

	void operator()();
};

void ParserThread::operator()()
{
	int num = 0;
	while (num < input_->size() && num < output_->size()) {
		num = counter_->getCurrentNumber();

		if (debug) {
			std::cerr << "NUM::: " << num << endl;
		}

		if (num == -1)
			break;

		string in = input_->at(num);
		phrase_versions *out = &output_->at(num);
		Context *context = &context_vect_->at(num);

		this->parseSingleSentence(in, pi_, wi_, context, out);
	}
}

void ParserThread::parseSingleSentence(const string &text, PhraseInfo *pi, WisdomInfo *wi, Context *context, phrase_versions *return_phrase)
{
	try {
     	phrase_versions tmp_phrase(text, pi, context, *wi);
     	context->add(tmp_phrase.get_most_likely_drt());
     	*return_phrase = tmp_phrase;
	} catch (std::exception &e) {
     	////
	}
}

void drt_collection::init(string &text)
{
	vector<string> phrases_text;
	vector<string> punctuation;

	text = process_abbreviations(text);
	text = pre_clean_text(text);
	text = erase_par(text);

	string separators(".!?;:");
	boost::char_delimiters_separator<char> sep1(true, separators.c_str(), "\n");
	boost::tokenizer<boost::char_delimiters_separator<char> > tok(text, sep1);
	boost::tokenizer<>::iterator tok_iter = tok.begin();
	while (tok_iter != tok.end()) {
		phrases_text.push_back(*tok_iter);
		++tok_iter;
		if (tok_iter != tok.end()) {
			string punct_candidate(*tok_iter);
			if (separators.find(punct_candidate) != string::npos) {
				punctuation.push_back(punct_candidate);
				++tok_iter;
			} else {
				punctuation.push_back(".");
			}
		}
	}

	Context context(context_);

	parser *dummy_parser = parser_singleton::get_parser_instance(); // The parser and tagger data must be loaded first

	// Join together phrases and punctuation
	int num_sentences = 0;
	vector<string> text_vect;
	for (int n = 0; n < phrases_text.size() && n < punctuation.size(); ++n) {
		int size_tmp = phrases_text.at(n).size();
		if (debug) {
			cout << "PHRASES::: " << phrases_text.at(n) << endl;
		}
		if (size_tmp > 1) {
			string punct = ".";
			if (n < punctuation.size())
				punct = punctuation.at(n);
			if (punct == "!" || punct == ";" || punct == ":")
				punct = ".";
			if (debug) {
				cout << "PHRASES_PUNCT::: " << punct << endl;
			}
			phrases_text.at(n).insert(size_tmp, punct);
			text_vect.push_back(post_clean_text(phrases_text.at(n)));
			++num_sentences;
		}
	}

	// create the threads that parse the text
	Parameters *par = parameters_singleton::instance();

	int max_threads=1;
	if(commercial_version)
		max_threads = par->getNumThreads();

	int num_threads = min(num_sentences, max_threads);

	if (debug) {
		std::cerr << "NUM_THR::: " << num_threads << endl;
	}

	vector<Context> context_vect(num_sentences, context);
	vector<phrase_versions> all_phrases(num_sentences);
	boost::thread_group g;
	ThreadCounter counter(num_sentences);
	vector<ParserThread> pt_vect(num_threads, ParserThread(&text_vect, &all_phrases, &context_vect, &counter, pi_, &wi_));
	for (int t = 0; t < num_threads; ++t) {
		g.create_thread(pt_vect.at(t));
	}

	g.join_all();
	for (int t = 0; t < num_sentences; ++t) {
		if(activate_context) {
			context.add(context_vect.at(t));
		}
		this->add_phrase(all_phrases.at(t));
	}

	context_ = context;

	// Set the levels of anaphora. Example: "I don't have a pen. The
	// pen is black". The second pen is not the first one.
	drs_anaphora_levels levels;
	levels.visit(this);

	for (int n = 0; n < drt_collection_.size(); ++n) {
		// each sentence in the discourse is marked with a different number
		drt_collection_.at(n).apply_phrase_number(n);
	}

	clock_t start;
	if (debug)
		start = clock();

	// initializes the Anaphora class
	Anaphora anaphora(drt_collection_, pi_);
	vector<References> phrase_references;

	// Apply DRT and submit the references in the drts after connect_references
	phrase_references = anaphora.getReferences();
	for (int n = 0; n < drt_collection_.size(); ++n) {
		drt_collection_.at(n).set_references(phrase_references.at(n));
	}

	// Get the missed anaphoras
	phrase_references = anaphora.getUninstantiated();
	for (int n = 0; n < drt_collection_.size(); ++n) {
		drt_collection_.at(n).add_references(phrase_references.at(n));
	}
	// get the Donkey references
	phrase_references = anaphora.getDonkeyReferences();
	for (int n = 0; n < drt_collection_.size(); ++n) {
		drt_collection_.at(n).add_references(phrase_references.at(n));
	}

	if (debug) {
		clock_t end = clock();
		cout << "Mtime8::: " << (end - start) / (double) CLOCKS_PER_SEC << endl;
	}

	// Find allocution references
	connect_allocution_references();

	// Find the Levin classification of the phrase
	for (int n = 0; n < drt_collection_.size(); ++n) {
		drt_collection_.at(n).find_levin();
	}

	// Put the data into personae_
	this->compute_data();

	////  TEST
	for (int n = 0; n < drt_collection_.size(); ++n) {
		vector<DrtPred> pv = drt_collection_.at(n).predicates_with_references();
		vector<string> levins = drt_collection_.at(n).get_levin();
	}
}

drt_collection::drt_collection(string text, int global_num, Context c, PhraseInfo *pi, const string &link_str, WisdomInfo wi) :
		global_num_(global_num), context_(c), pi_(pi), link_(link_str), wi_(wi)
{
	this->init(text);
}

drt_collection::drt_collection(string text, int global_num, Context c, PhraseInfo *pi, WisdomInfo wi) :
		global_num_(global_num), context_(c), pi_(pi), wi_(wi)
{
	this->init(text);
}

void drt_collection::setLink(const string &link)
{
	link_ = link;
	for (int n = 0; n < drt_collection_.size(); ++n) {
		drt_collection_.at(n).setLink(link_);
	}
}

static string get_alloc_ref(const vector<DrtPred> &preds)
{
	vector<DrtPred>::const_iterator piter = preds.begin();
	vector<DrtPred>::const_iterator pend = preds.end();

	string ref_str = "", verb_str = "";

	vector<string> communication_verbs = get_communication_verbs();

	for (; piter != pend; ++piter) {
		string head_str = extract_header(*piter);
		if (piter->is_verb()
				&& find(communication_verbs.begin(), communication_verbs.end(), head_str) != communication_verbs.end()) {
			ref_str = extract_first_tag(*piter);
			return ref_str;
		}
		if (head_str == "@PARENT-ALLOCUTION") {
			ref_str = extract_second_tag(*piter);
			return ref_str;
		}
	}

	return ref_str;
}

static bool points_to_verb(const DrtPred &pred)
// Returns true if the predicate points to a verb
{
	string ftag = extract_first_tag(pred);
	if (ftag.find("verb") != string::npos)
		return true;
	return false;
}

static int find_pred_with_head(const DrtVect &drtvect, const string &head)
{
	int m = -1;
	for (int n = 0; n < drtvect.size(); ++n) {
		string head_str = extract_header(drtvect.at(n));
		if (head_str == head)
			return n;
	}
	return m;
}

static DrtVect eliminate_allocution(DrtVect drtvect)
{
	int m = find_pred_with_head(drtvect, "@PARENT-ALLOCUTION");
	if (m != -1) {
		string alloc_ref = extract_first_tag(drtvect.at(m));
		drtvect.erase(drtvect.begin() + m);
		m = find_element_with_string(drtvect, alloc_ref);
		if (m != -1)
			drtvect.erase(drtvect.begin() + m);
	}

	return drtvect;
}

void drt_collection::connect_allocution_references()
{
	vector<drt>::iterator drtiter = drt_collection_.begin();
	vector<drt>::iterator drtend  = drt_collection_.end();

	if (drtiter == drtend)
		return;

	// give a reference to an unassigned PARENT-ALLOCUTION, choosing
	// either from a communication verb (say, speak, write, ...) or
	// from the closest (assigned) PARENT-ALLOCUTION
	string prev_alloc_ref = "none";
	DrtVect prev_drtvect;
	int ref_sentence_position = 0;
	for (int n = 0; drtiter != drtend; ++drtiter, ++n) {
		DrtVect tmp_drt = drtiter->predicates_with_references();
		string alloc_ref = get_alloc_ref(tmp_drt);
		if (alloc_ref.size() && !points_to_verb(alloc_ref)
			&& fabs(ref_sentence_position-n) < 3 // an allocution can be 3 sentences away from the communication verb at max
				) {
			vector<pair<string, string> > refs;
			refs.push_back(make_pair(prev_alloc_ref, alloc_ref));
			drtiter->add_references(refs);

			DrtVect new_drtvect = eliminate_allocution(prev_drtvect);
			new_drtvect.insert(new_drtvect.end(), tmp_drt.begin(), tmp_drt.end());
			drtiter->setPredicates(new_drtvect);

		} else if (alloc_ref.size()) {
			prev_alloc_ref = alloc_ref;
			prev_drtvect = tmp_drt;
			ref_sentence_position = n;
		}
	}
}

static DrtVect invert_allocutions(DrtVect drtvect)
{
	for (int n = 0; n < drtvect.size(); ++n) {
		string head = extract_header(drtvect.at(n));
		if (head == "@PARENT-ALLOCUTION") {
			switch_children(drtvect.at(n));
			implant_header(drtvect.at(n), "@ALLOCUTION");
		}
	}

	return drtvect;
}

static DrtVect post_process_drtvect(DrtVect drtvect)
{
	for (int n = 0; n < drtvect.size(); ++n) {
		string head = extract_header(drtvect.at(n));
		if (drtvect.at(n).is_complement() && head == "@OWN") {
			implant_header(drtvect.at(n), "@OWNED_BY");
			switch_children(drtvect.at(n));
		}
	}

	return drtvect;
}

static bool is_included_in(const drt &lhs, const drt &rhs)
{
	DrtVect lvect = lhs.predicates_with_references();
	DrtVect rvect = rhs.predicates_with_references();
	for (int n = 0; n < lvect.size(); ++n) {
		if (lvect.at(n).is_verb() && !shortfind(rvect, lvect.at(n)))
			return false;
	}
	return true;
}
static vector<drt> erase_inclusions(const vector<drt> &drts)
{
	vector<drt> to_return;
	vector<int> to_erase;
	for (int n = 0; n < drts.size(); ++n) {
		for (int m = 0; m < drts.size(); ++m) {
			drt n_drt = drts.at(n);
			drt m_drt = drts.at(m);
			if (m != n && is_included_in(n_drt, m_drt)) {
				to_erase.push_back(n);
			}
		}
	}
	if (debug) {
		puts("TO_ERASE::");
		print_vector(to_erase);
	}
	for (int n = 0; n < drts.size(); ++n) {
		if (!shortfind(to_erase, n))
			to_return.push_back(drts.at(n));
	}

	return to_return;
}

DrtVect sign_intersection_words(const vector<DrtPred> &previous, DrtVect to_return)
{
	for(int n=0; n < to_return.size(); ++n) {
		if(shortfind(previous,to_return.at(n))) {
			to_return.at(n).set_intersection();
		}
	}

	return to_return;
}

static vector<drt> get_linked_drts_from_single_drt(const drt &d, const string &link, const string &text)
// It returns the set of drts that are connected to a specific
// verb. Conjunctions between verbs and @conditions are ignored.
{
	vector<drt> to_return;

	vector<DrtPred> drtvect = d.predicates_with_references();

	drtvect = invert_allocutions(drtvect);
	vector<DrtPred>::iterator diter = drtvect.begin();
	vector<DrtPred>::iterator dend = drtvect.end();

	int n = 0;
	vector<DrtPred> previous_elements;
	//previous_elements.push_back("@DUMMY(dummy)");
	if (debug) {
		puts("DRS_ATTACHED0");
		print_vector(drtvect);
	}
	for (; diter != dend; ++diter, ++n) {
		if (diter->is_verb() && !shortfind(previous_elements, *diter)) {
			DrtVect tmp_drtvect = find_all_attached_to_verb(drtvect, n);
			if (tmp_drtvect.size() == 1)  // not interested in lonely verbs
				continue;
			if (debug) {
				puts("DRS_ATTACHED2");
				print_vector(tmp_drtvect);
			}
			tmp_drtvect= sign_intersection_words(previous_elements, tmp_drtvect);
			drt_sort(tmp_drtvect);
			unique(tmp_drtvect.begin(), tmp_drtvect.end());
			previous_elements.insert(previous_elements.end(), tmp_drtvect.begin(), tmp_drtvect.end());
			drt tmp_drt(tmp_drtvect);
			tmp_drt.find_levin();
			tmp_drt.setLink(link);
			tmp_drt.setText(text);
			to_return.push_back(tmp_drt);
		}
	}
	to_return = erase_inclusions(to_return);

	if (debug) {
		puts("DRS_ATTACHED3:::");
		for (int n = 0; n < to_return.size(); ++n) {
			DrtVect dv = to_return.at(n).predicates_with_references();
			print_vector(dv);
		}
	}

	return to_return;
}

static vector<DrtPred> get_conjunctions_from_single_drt(const drt &d)
{
	vector<DrtPred> to_return;

	vector<string> candidates;
	candidates.push_back("@CONJUNCTION");
	candidates.push_back("@DISJUNCTION");
	candidates.push_back("@COORDINATION");

	vector<DrtPred> drtvect = d.predicates_with_references();

	vector<DrtPred>::iterator diter = drtvect.begin();
	vector<DrtPred>::iterator dend = drtvect.end();

	for (; diter != dend; ++diter) {
		string head = extract_header(*diter);
		if (find(candidates.begin(), candidates.end(), head) != candidates.end())
			to_return.push_back(*diter);
	}

	return to_return;
}

vector<drt> drt_collection::compute_data()
{
	vector<drt>::iterator drtiter = drt_collection_.begin();
	vector<drt>::iterator drtend = drt_collection_.end();

	vector<phrase_versions>::iterator phiter = phrase_collection_.begin();
	vector<phrase_versions>::iterator phend = phrase_collection_.end();

	vector<DrtVect> all_data;
	vector<drt> ret_drts;
	vector<string> all_texts;

	vector<pair<vector<drt>, string> > pres_pairs;

	clock_t start;
	if (debug)
		start = clock();

	for (; drtiter != drtend; ++drtiter, ++phiter) {
		if (phiter->num_phrases() && !phiter->get_most_likely_phrase().has_question()
				&& !phiter->get_most_likely_phrase().has_condition()
				) {
			DrtVect tmp_data(drtiter->predicates_with_references());
			tmp_data = post_process_drtvect(tmp_data);
			tmp_data = invert_allocutions(tmp_data);
			string text = clear_text(phiter->get_text()); // the original text of the phrase
			drt tmp_drt(*drtiter);
			tmp_drt.setText(text);
			tmp_drt.setPredicates(tmp_data);
			Presupposition pres(tmp_drt);
			vector<drt> pres_drtvect = pres.get();
			pres_pairs.push_back(make_pair(pres_drtvect, text));
			vector<drt> drts_tmp = get_linked_drts_from_single_drt(tmp_drt, link_, text); // It also computes the Levins within the function;

			if(debug) {
				cout << "LINK3::: " << link_ << endl;
			}

			ret_drts.insert(ret_drts.end(), drts_tmp.begin(), drts_tmp.end());
			for (int n = 0; n < drts_tmp.size(); ++n) {
				all_data.push_back(drts_tmp.at(n).predicates_with_references());
				all_texts.push_back(text);
			}
		}
	}

	if (debug) {
		clock_t end = clock();
		cout << "Mtime4::: " << (end - start) / (double) CLOCKS_PER_SEC << endl;
	}

	if (debug)
		start = clock();

	DrsPersonae personae(ret_drts, link_);
	personae.compute();
	personae_ = personae;

	if (debug) {
		clock_t end = clock();
		cout << "Mtime5::: " << (end - start) / (double) CLOCKS_PER_SEC << endl;
	}

	if (debug)
		start = clock();

	vector<drt> all_pres_drt;
	vector<string> all_pres_text;
	// add the presuppositions to the personae_;
	for (int n = 0; n < pres_pairs.size(); ++n) {
		vector<drt> drts = pres_pairs.at(n).first;
		for (int m = 0; m < drts.size(); ++m) {
			all_pres_drt.push_back(drts.at(m));
		}
	}

	DrsPersonae pres_personae(all_pres_drt, link_);
	pres_personae.compute();
	personae_.addPersonae(pres_personae);

	if (debug) {
		clock_t end = clock();
		cout << "Mtime6::: " << (end - start) / (double) CLOCKS_PER_SEC << endl;
	}

	return ret_drts;
}

vector<drt> drt_collection::extract_data()
{
	return drt_collection_;
}

vector<DrtPred> get_clause_hypothesis(const vector<DrtPred> &preds)
{
	vector<DrtPred> cons;
	string condition_to, condition_from;

	for (int n = 0; n < preds.size(); ++n) {
		string head_str = extract_header(preds.at(n));
		if (head_str == "@CONDITION") {
			condition_from = extract_first_tag(preds.at(n));
			condition_to = extract_second_tag(preds.at(n));
		}
	}
	vector<DrtPred> connected_subj, connected_obj;
	int m = find_verb_with_string(preds, condition_to);
	if (m == -1)
		return cons;
	cons = find_all_attached_to_verb(preds, m);
	drt_sort(cons);
	return cons;
}

vector<string> get_valid_verbs_attached_to_ref(const string &first_ref, const vector<DrtPred> &conjs)
/// This is to be done better: the problem is that this function now gives all the conjunctions
/// after the one with first_ref. There is no control if these conjunctions are connected
{
	vector<string> to_return;

	vector<DrtPred>::const_iterator citer = conjs.begin(), cend = conjs.end();
	bool push_trigger = false;
	for (; citer != cend; ++citer) {
		string fref = extract_first_tag(*citer);
		string sref = extract_second_tag(*citer);
		if (fref == first_ref)
			push_trigger = true;
		if (push_trigger) {
			if (find(to_return.begin(), to_return.end(), fref) == to_return.end())
				to_return.push_back(fref);
			if (find(to_return.begin(), to_return.end(), sref) == to_return.end())
				to_return.push_back(sref);
		}
	}
	return to_return;
}

vector<vector<DrtPred> > get_all_clause_hypothesis(const vector<DrtPred> &preds)
{
	vector<vector<DrtPred> > to_return;
	string condition_to, condition_from;

	for (int n = 0; n < preds.size(); ++n) {
		string head_str = extract_header(preds.at(n));
		if (head_str == "@CONDITION") {
			condition_from = extract_first_tag(preds.at(n));
			condition_to = extract_second_tag(preds.at(n));
		}
	}
	vector<DrtPred> conjs = get_conjunctions_from_single_drt(preds);
	vector<string> refs = get_valid_verbs_attached_to_ref(condition_to, conjs);
	vector<string>::iterator riter = refs.begin(), rend = refs.end();

	for (; riter != rend; ++riter) {
		vector<DrtPred> cons;
		vector<DrtPred> connected_subj, connected_obj;
		int m = find_verb_with_string(preds, *riter);
		if (m == -1)
			return to_return;
		cons = find_all_attached_to_verb(preds, m);
		drt_sort(cons);
		to_return.push_back(cons);
	}
	return to_return;;
}

DrtVect clean_consequence(DrtVect cons)
// clean the consequence from unwanted material
{
	for (int n = 0; n < cons.size(); ++n) {
		string head_str = extract_header(cons.at(n));
		if (head_str == "@CONDITION") {
			cons.erase(cons.begin() + n);
			--n;
		}
	}
	return cons;
}

vector<DrtPred> get_clause_consequence(const vector<DrtPred> &preds)
{
	vector<DrtPred> cons;
	string condition_to, condition_from;

	for (int n = 0; n < preds.size(); ++n) {
		string head_str = extract_header(preds.at(n));
		if (head_str == "@CONDITION") {
			condition_from = extract_first_tag(preds.at(n));
			condition_to = extract_second_tag(preds.at(n));
		}
	}
	vector<DrtPred> connected_subj, connected_obj;
	int m = find_verb_with_string(preds, condition_from);
	if (m == -1)
		return cons;
	cons = find_all_attached_to_verb(preds, m);
	cons = clean_consequence(cons);

	drt_sort(cons);
	return cons;
}

static bool is_valid_reference(const string &ref)
{
	if (ref.find("name") != string::npos || ref.find("ref") != string::npos || ref.find("verb") != string::npos
			|| ref.find("obj") != string::npos || ref.find("subj") != string::npos || ref.find("prev") != string::npos
			|| ref.find("next") != string::npos || ref.find("from") != string::npos || ref.find("to") != string::npos
			|| ref.find("none") != string::npos)
		return true;

	return false;
}

static vector<DrtPred> name_unifiers(vector<DrtPred> preds)
{
	for (int n = 0; n < preds.size(); ++n) {
		string head_str = extract_header(preds.at(n));
		vector<string> children = preds.at(n).extract_children();
		for (int m = 0; m < children.size(); ++m) {
			string str_tmp = children.at(m);
			if (is_valid_reference(str_tmp)) {
				str_tmp.insert(0, "_");
				children.at(m) = str_tmp;
			}
		}
		preds.at(n).implant_children(children);
	}
	return preds;
}

vector<DrtPred> name_some_unifiers(vector<DrtPred> preds, const vector<DrtPred> &hyp)
{
	// extracts all the references
	vector<string> references;
	for (int n = 0; n < hyp.size(); ++n) {
		vector<string> children = hyp.at(n).extract_children();
		for (int m = 0; m < children.size(); ++m) {
			string element = children.at(m);
			if (find(references.begin(), references.end(), element) == references.end() && is_valid_reference(element)) {
				references.push_back(element);
			}
		}
	}

	// change as unifiers the same references as in the hypothesis
	for (int n = 0; n < preds.size(); ++n) {
		vector<string> children = preds.at(n).extract_children();
		for (int m = 0; m < children.size(); ++m) {
			string element = children.at(m);
			if (find(references.begin(), references.end(), element) != references.end()) {
				string str_tmp = element;
				str_tmp.insert(0, "_");
				children.at(m) = str_tmp;
			}
		}
		preds.at(n).implant_children(children);
	}
	return preds;
}

vector<DrtPred> name_some_unifiers_from_all_hyp(vector<DrtPred> preds, const vector<vector<DrtPred> > &hyp)
{
	for (int n = 0; n < hyp.size(); ++n)
		preds = name_some_unifiers(preds, hyp.at(n));

	return preds;
}

vector<DrtPred> adjust_WP(vector<DrtPred> preds)
{
	for (int n = 0; n < preds.size(); ++n) {
		string head_str = extract_header(preds.at(n));
		if (head_str == "what") {
			implant_header(preds.at(n), "animal|thing|event");
		}
	}
	return preds;
}

static vector<DrtPred> trim_time(vector<DrtPred> preds)
// delete the present time indicators from preds
{
	vector<DrtPred> ret;
	for (int n = 0; n < preds.size(); ++n) {
		string head_str = extract_header(preds.at(n));
		if (head_str == "@TIME") {

////// This commented part is correct but it makes the matching frustrating. To be inserted when the matching is perfected!!	       
//               string tense= extract_second_tag(preds.at(n));
//               if(tense.find("present") != string::npos)
//		    continue; 
//////         

			continue;/// skipping all @TIME from questions
		}
		ret.push_back(preds.at(n));
	}
	return ret;
}

static vector<DrtPred> trim_modals(vector<DrtPred> preds)
// delete the modals!!!
{
	vector<DrtPred> ret;
	for (int n = 0; n < preds.size(); ++n) {
		string head_str = extract_header(preds.at(n));
		if (head_str == "@MODAL") {
			//string tense= extract_second_tag(preds.at(n));
			continue;
		}
		ret.push_back(preds.at(n));
	}
	return ret;
}

static vector<DrtPred> trim_auxiliaries(vector<DrtPred> preds)
// delete the modals!!!
{
	vector<DrtPred> ret;
	for (int n = 0; n < preds.size(); ++n) {
		string head_str = extract_header(preds.at(n));
		if (head_str == "@AUXILIARY") {
			//string tense= extract_second_tag(preds.at(n));
			continue;
		}
		ret.push_back(preds.at(n));
	}
	return ret;
}


static double get_clause_weight_from_cons(vector<DrtPred> *cons)
{
	double w = 1;

	vector<DrtPred>::iterator citer = cons->begin();
	vector<DrtPred>::iterator cend = cons->end();

	map<string, double> weight_map; /// Change This!!! It is completely arbitrary
	weight_map["might"] = 0.2;
	weight_map["may"] = 0.3;
	weight_map["could"] = 0.4;
	weight_map["should"] = 0.6;
	weight_map["can"] = 0.7;
	weight_map["would"] = 0.9;
	weight_map["shall"] = 1;
	weight_map["will"] = 1;
	weight_map["need"] = 1;
	weight_map["must"] = 1;

	for (; citer != cend; ++citer) {
		if (extract_header(*citer) == "@MODAL") {
			string type = extract_second_tag(*citer);
			cons->erase(citer);
			return weight_map[type];
		}
	}

	return w;
}

static DrtVect clean_duplicate_elements(const DrtVect &orig_drtvect)
{
	DrtVect drtvect;
	vector<string> already_parsed;
	for (int n = 0; n < orig_drtvect.size(); ++n) {
		std::stringstream ss;
		ss << orig_drtvect.at(n);
		string item = ss.str();
		if(debug) {
			cout << "ORIG_DUPL::: " << orig_drtvect.at(n) << " " << shortfind(already_parsed, item) << endl;
		}
		if (!shortfind(already_parsed, item) ) {
			drtvect.push_back(item);
			already_parsed.push_back(item);
		} else
			already_parsed.push_back(item);
	}

	return drtvect;
}

static vector<DrtPred> last_touch_complements_in_questions(const vector<DrtPred> &pre_drt)
// complements in questions can be ambiguous: "to" can be @MOTION_TO or @DATIVE
{
	vector<DrtPred> to_return(pre_drt);
	int size = to_return.size();

	map<string, string> map_compl;
	map_compl["@MOTION_TO"] = "@MOTION_TO|@TIME_TO|@DATIVE";
	map_compl["@DATIVE"] = "@MOTION_TO|@TIME_TO|@DATIVE";
	map_compl["@PLACE_AT"] = "@PLACE_AT|@MOTION_FROM|@MOTION_TO|@MOTION_THROUGH";
	map_compl["@TIME_AT"] = "@TIME_AT|@CLOCK_AT|@TIME_FROM|@TIME_TO|@TOPIC";
	map_compl["@CAUSED_BY"] = "@CAUSED_BY|@WITH|@DATIVE";
	map_compl["@QUANTITY"] = "@QUANTITY|@MORE_THAN|@LESS_THAN";
	map_compl["@SIZE"] = "@SIZE|@QUANTITY|@FOR";
	map_compl["@ALLOCUTION"] = "@ALLOCUTION|@TOPIC";

	map<pair<string, string>, string> map_compl_with_prep;
	map_compl_with_prep[make_pair("@MOTION_TO", "to")] = "@MOTION_TO|@TIME_TO|@DATIVE";
	map_compl_with_prep[make_pair("@DATIVE", "as")] = "@DATIVE";
	map_compl_with_prep[make_pair("@PLACE_AT", "at")] = "@PLACE_AT";
	map_compl_with_prep[make_pair("@PLACE_AT", "from")] = "@MOTION_FROM|@TIME_FROM";
	map_compl_with_prep[make_pair("@PLACE_AT", "on")] = "@PLACE_AT|@TIME_AT|@MOTION_FROM|@MOTION_TO|@MOTION_THROUGH";
	map_compl_with_prep[make_pair("@TIME_AT", "on")] = "@PLACE_AT|@TIME_AT";
	map_compl_with_prep[make_pair("@PLACE_AT", "where")] = "@PLACE_AT|@MOTION_FROM|@MOTION_TO|@MOTION_THROUGH";
	map_compl_with_prep[make_pair("@TIME_AT", "when")] = "@TIME_AT|@CLOCK_AT|@TIME_FROM|@TIME_TO|@TOPIC|@BEFORE|@AFTER";
	map_compl_with_prep[make_pair("@CAUSED_BY", "because")] = "@CAUSED_BY|@WITH|@DATIVE";
	map_compl_with_prep[make_pair("@CAUSED_BY", "why")] = "@CAUSED_BY|@WITH";
	map_compl_with_prep[make_pair("@WITH", "with")] = "@CAUSED_BY|@WITH";
	map_compl_with_prep[make_pair("@FOR", "for")] = "@CAUSED_BY|@FOR";
	map_compl_with_prep[make_pair("@TOPIC", "on")] = "@TOPIC|@TIME_AT";
	map_compl_with_prep[make_pair("@CAUSED_BY", "how")] = "@CAUSED_BY|@WITH|@DATIVE|@TOPIC";

	for (int n = 0; n < size; ++n) {
		string head_str = extract_header(to_return.at(n));
		if (to_return.at(n).is_complement()
		//&& to_return.at(n).is_question() /// Do not use! questions must be as ambiguous as possible
		) {
			string prep_str = to_return.at(n).name();
			map<pair<string, string>, string>::iterator miter = map_compl_with_prep.find(make_pair(head_str, prep_str));
			if (miter != map_compl_with_prep.end()) {
				implant_header(to_return.at(n), miter->second);
			} else {
				map<string, string>::iterator miter2 = map_compl.find(head_str);
				if (miter2 != map_compl.end()) {
					implant_header(to_return.at(n), miter2->second);
				}
			}
		}
	}
	return to_return;
}


vector<clause_vector> drt_collection::extract_clauses()
{
	vector<drt>::iterator drtiter = drt_collection_.begin();
	vector<drt>::iterator drtend = drt_collection_.end();

	vector<phrase_versions>::iterator phiter = phrase_collection_.begin();
	vector<phrase_versions>::iterator phend = phrase_collection_.end();

	vector<string> all_text;

	vector<clause_vector> ret_data;
	for (; drtiter != drtend; ++drtiter, ++phiter) {
		if (phiter->num_phrases() && phiter->get_most_likely_phrase().has_condition()) {
			DrtVect all_predicates = drtiter->predicates_with_references();
			all_predicates = post_process_drtvect(all_predicates);
			DrtVect cons = get_clause_consequence(all_predicates);

			if(debug) {
				puts("CLAUSE_CONS:::");
				print_vector(cons);
			}

			vector<vector<DrtPred> > hyp = get_all_clause_hypothesis(all_predicates);
			cons = clean_duplicate_elements(cons);
			for (int n = 0; n < hyp.size(); ++n) {
				hyp.at(n) = clean_duplicate_elements(hyp.at(n));
				hyp.at(n) = last_touch_complements_in_questions(hyp.at(n));
			}
			if (cons.size() == 0)
				continue;
			if (hyp.size() == 0) {
				DrtVect tmp_hyp = get_clause_hypothesis(all_predicates);
				tmp_hyp = last_touch_complements_in_questions(tmp_hyp);
				tmp_hyp = clean_duplicate_elements(tmp_hyp);
				if (tmp_hyp.size()) {
					hyp.push_back(tmp_hyp);
				} else
					continue;
			}
			cons = trim_time(cons);  // erase the present tense indicators
			for (int n = 0; n < hyp.size(); ++n) {
				hyp.at(n) = trim_time(hyp.at(n)); // erase the present tense indicators
				hyp.at(n) = trim_modals(hyp.at(n)); // erase the modals
				hyp.at(n) = trim_auxiliaries(hyp.at(n)); // erase the auxiliaries
			}
			double w = get_clause_weight_from_cons(&cons);
			cons = name_some_unifiers_from_all_hyp(cons, hyp);
			for (int n = 0; n < hyp.size(); ++n)
				hyp.at(n) = name_unifiers(hyp.at(n));
			string text = clear_text(phiter->get_text()); // the original text of the phrase
			clause_vector tmp_clause;
			tmp_clause.setWeigth(w);
			tmp_clause.setConsequence(cons);
			tmp_clause.setAllHypothesis(hyp);
			tmp_clause.find_levin();
			tmp_clause.setLink(link_);
			tmp_clause.setText(text);
			vector<string> consLevins = tmp_clause.getLevinCons();
			vector<string> hypLevins = tmp_clause.getLevinHyp();
			ret_data.push_back(tmp_clause);
			all_text.push_back(text);
			drtiter->set_condition();
		}
	}

	Rules rules(ret_data, all_text, link_);
	rules.compute();
	rules_ = rules;

	for (int n = 0; n < ret_data.size(); ++n) {
		if (debug) {
			cout << "EXTRACT_CLAUSE::: " << ret_data.at(n) << endl;
		}
	}

	return ret_data;
}

static string get_first_verb_ref(const DrtVect &drtvect)
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

static DrtVect add_verb_if_needed(DrtVect drtvect)
{
	DrtVect orig_drtvect(drtvect);

	// check if there are verbs
	bool has_verb= false;
	for (int n = 0; n < drtvect.size(); ++n) {
		if(drtvect.at(n).is_verb()) {
			has_verb = true;
			break;
		}
	}
	if(!has_verb && drtvect.size() >0) {
		DrtPred what_pred("[what]/NN#[pivot](_name_vless_-1)");
		DrtPred be_pred  ("be/VB(_verb_vless_-1,_name_vless_-1,dummy)");
		string fref = extract_first_tag(drtvect.at(0));
		implant_object(be_pred,fref);
		what_pred.set_question();
		what_pred.set_question_word(extract_header(drtvect.at(0)));
		drtvect.at(0).set_question(false);
		DrtVect tmp_drtvect;

		tmp_drtvect.push_back(what_pred);
		tmp_drtvect.push_back(be_pred);
		tmp_drtvect.insert(tmp_drtvect.end(),drtvect.begin(),drtvect.end() );
		drtvect = tmp_drtvect;
	}
	return drtvect;
}


pair<vector<drt>, vector<DrtPred> > drt_collection::compute_questions()
{
	vector<DrtPred> ret_conj;

	vector<drt>::iterator drtiter = drt_collection_.begin();
	vector<drt>::iterator drtend = drt_collection_.end();

	vector<phrase_versions>::iterator phiter = phrase_collection_.begin();
	vector<phrase_versions>::iterator phend = phrase_collection_.end();

	vector<drt> ret_data;
	for (; drtiter != drtend; ++drtiter, ++phiter) {
		if (phiter->num_phrases() && phiter->get_most_likely_phrase().has_question()) {

			string text = clear_text(phiter->get_text()); // the original text of the phrase
			vector<drt> drts_tmp = get_linked_drts_from_single_drt(*drtiter, link_, text); // It also computes the Levins within the function;

			drtiter->set_question();

			if (drts_tmp.size() == 0) { // if there is no verb then the question is just the predicates
				DrtVect drt_tmp = drtiter->predicates_with_references();
				drts_tmp.push_back(drt_tmp);
			}

			vector<DrtPred> conj = get_conjunctions_from_single_drt(*drtiter);
			conj = name_unifiers(conj);
			ret_conj.insert(ret_conj.end(), conj.begin(), conj.end());

			if (drts_tmp.size() > 1 && conj.size() == 0) {
				vector<string> verb_ref_list;
				for (int n = 0; n < drts_tmp.size(); ++n) {
					DrtVect drt_tmp = drts_tmp.at(n).predicates_with_references();
					string verb_ref = get_first_verb_ref(drt_tmp);
					verb_ref_list.push_back(verb_ref);
				}
				for (int n = 0; n < verb_ref_list.size() - 1; ++n) {
					string fref = verb_ref_list.at(n);
					string sref = verb_ref_list.at(n + 1);
					if (fref != "" && sref != "") {
						DrtPred conj_tmp("@CONJUNCTION(" + fref + "," + sref + ")");
						conj.push_back(conj_tmp);
					}
				}
				conj = name_unifiers(conj);
				ret_conj.insert(ret_conj.end(), conj.begin(), conj.end());
			}

			for (int m = 0; m < drts_tmp.size(); ++m) {
				DrtVect tmp_quest = drts_tmp.at(m).predicates_with_references();
				tmp_quest = post_process_drtvect(tmp_quest);
				tmp_quest = trim_time(tmp_quest); // erase the present tense indicators
				tmp_quest = trim_modals(tmp_quest); // erase the present tense indicators
				tmp_quest = trim_auxiliaries(tmp_quest); // erase the auxiliaries
				tmp_quest = name_unifiers(tmp_quest);
				tmp_quest = adjust_WP(tmp_quest);
				if(debug) {
					cout << "ADDING_IMPLICIT::: " << wi_.getImplicitVerb() << endl;
				}
				if(wi_.getImplicitVerb() ) {
					tmp_quest = add_verb_if_needed(tmp_quest);
				}
				drt tmp_drt(tmp_quest);
				tmp_drt.setText(text);
				tmp_drt.setLink(link_);
				QuestionList ql;
				ql.add(tmp_drt.predicates_with_references());
				tmp_drt.setQuestionList(ql);
				ret_data.push_back(tmp_drt);
			}
		}
	}
	questions_ = make_pair(ret_data, ret_conj);
	return questions_;

}

vector< QuestionVersions > drt_collection::compute_candidate_questions()
{
	vector< QuestionVersions > to_return;

	vector<drt>::iterator drtiter = drt_collection_.begin();
	vector<drt>::iterator drtend = drt_collection_.end();

	vector<phrase_versions>::iterator phiter = phrase_collection_.begin();
	vector<phrase_versions>::iterator phend = phrase_collection_.end();

	for (; drtiter != drtend; ++drtiter, ++phiter) {
		if (phiter->num_phrases() && phiter->get_most_likely_phrase().has_question()) {

			vector<pair<phrase, double> > phrase_and_w= phiter->get_phrases_with_weight();
			QuestionVersions question_versions;

			for(int phrase_num=0; phrase_num < phrase_and_w.size(); ++phrase_num) {

				phrase ph= phrase_and_w.at(phrase_num).first;
				int w= phrase_and_w.at(phrase_num).second;
				if(w < 0)
					continue; // the question has a grammatical mistake
				drt drt_tmp = ph.get_drt();

				if(debug) {
					puts("CANDIDATE_QUESTIONS:::");
					DrtVect drtvect = drt_tmp.predicates_with_references();
					print_vector(drtvect);
				}

				drt_tmp.apply_phrase_number(global_num_); // apply the global reference number to the drts
				drt_tmp.setText(ph.get_text());
				drt_tmp.setLink(link_);
				vector<DrtPred> ret_conj;
				vector<drt> ret_data;

				string text = clear_text(ph.get_text()); // the original text of the phrase
				vector<drt> drts_tmp = get_linked_drts_from_single_drt(drt_tmp, link_, text); // It also computes the Levins within the function;

				drtiter->set_question();

				if (drts_tmp.size() == 0) { // if there is no verb then the question is just the predicates
					DrtVect drt_tmp = ph.get_drt();
					drts_tmp.push_back(drt_tmp);
				}

				vector<DrtPred> conj = get_conjunctions_from_single_drt(drt_tmp);
				conj = name_unifiers(conj);
				ret_conj.insert(ret_conj.end(), conj.begin(), conj.end());

				if (drts_tmp.size() > 1 && conj.size() == 0) {
					vector<string> verb_ref_list;
					for (int n = 0; n < drts_tmp.size(); ++n) {
						DrtVect drt_tmp = drts_tmp.at(n).predicates_with_references();
						string verb_ref = get_first_verb_ref(drt_tmp);
						verb_ref_list.push_back(verb_ref);
					}
					for (int n = 0; n < verb_ref_list.size() - 1; ++n) {
						string fref = verb_ref_list.at(n);
						string sref = verb_ref_list.at(n + 1);
						if (fref != "" && sref != "") {
							DrtPred conj_tmp("@CONJUNCTION(" + fref + "," + sref + ")");
							conj.push_back(conj_tmp);
						}
					}
					conj = name_unifiers(conj);
					ret_conj.insert(ret_conj.end(), conj.begin(), conj.end());
				}

				for (int m = 0; m < drts_tmp.size(); ++m) {
					DrtVect tmp_quest = drts_tmp.at(m).predicates_with_references();
					tmp_quest = post_process_drtvect(tmp_quest);
					tmp_quest = trim_time(tmp_quest); // erase the present tense indicators
					tmp_quest = trim_modals(tmp_quest); // erase the present tense indicators
					tmp_quest = trim_auxiliaries(tmp_quest); // erase the auxiliaries
					tmp_quest = name_unifiers(tmp_quest);
					tmp_quest = adjust_WP(tmp_quest);
					if(debug) {
						cout << "ADDING_IMPLICIT::: " << wi_.getImplicitVerb() << endl;
					}
					if(wi_.getImplicitVerb() ) {
						tmp_quest = add_verb_if_needed(tmp_quest);
					}
					drt tmp_drt(tmp_quest);
					tmp_drt.setText(text);
					tmp_drt.setLink(link_);

					QuestionList ql;
					ql.add(tmp_drt.predicates_with_references());
					tmp_drt.setQuestionList(ql);
					ret_data.push_back(tmp_drt);
				}
				CandidateQuestion cq;
				cq.add(ret_data);
				cq.add(ret_conj);
				question_versions.push_back(cq);
			}
			if(question_versions.size())
				to_return.push_back(question_versions);
		}
	}
	//questions_ = make_pair(ret_data, ret_conj);
	candidate_questions_= to_return;
	return to_return;

}


pair<vector<drt>, vector<DrtPred> > drt_collection::extract_questions()
{
	this->compute_questions();
	return questions_;
}

vector<QuestionVersions> drt_collection::extract_candidate_questions()
{
	this->compute_candidate_questions();
	return candidate_questions_;
}

void drt_collection::setWisdomInfo(WisdomInfo wi)
{
	wi_ = wi;
}



// CandidateQuestions class

drt CandidateQuestion::getFirstCandidate() const
{
	if(candidates_.size() )
		return candidates_.at(0);
	else
		return drt();
}

vector<drt> CandidateQuestion::getCandidates() const
{
	return candidates_;
}

vector<DrtPred> CandidateQuestion::getConjunctions() const
{
	return conjunctions_;
}

vector<drt> CandidateQuestion::getAllButFirstCandidates() const
{
	if(candidates_.size() > 1) {
		vector<drt> tmp_cand= candidates_;
		tmp_cand.erase(tmp_cand.begin() );
		return tmp_cand;
	}
	else
		return vector<drt>();
}

void CandidateQuestion::add(const drt &d)
{
	candidates_.push_back(d);
}

void CandidateQuestion::add(const DrtPred &p)
{
	conjunctions_.push_back(p);
}

void CandidateQuestion::add(const vector<drt> &d)
{
	candidates_.insert(candidates_.end(),d.begin(),d.end());
}

void CandidateQuestion::add(const vector<DrtPred> &p)
{
	conjunctions_.insert(conjunctions_.end(),p.begin(),p.end() );
}


