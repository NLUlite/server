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



#include"Writer.hpp"
//#include"../fixed_allocator/fixed_allocator.hpp"

const bool debug = false;


template<class T>
static bool shortfind(const vector<T> &vect, const T &element)
{
	if (find(vect.begin(), vect.end(), element) == vect.end())
		return false;
	return true;
}

template<class T>
static void print_vector(std::vector<T> &vs)
{
	if (vs.size()) {
		typename vector<T>::iterator tags_iter = vs.begin();
		while (tags_iter != vs.end()) {
			std::cout << (*tags_iter) << " ";
			++tags_iter;
		}
		std::cout << std::endl;
	}
}

static vector<DrtPred> sign_adjectives(vector<DrtPred> pred_list, Knowledge *k_)
{
	metric *d = metric_singleton::get_metric_instance();

	for (int n = 0; n < pred_list.size(); ++n) {
		DrtPred pred(pred_list.at(n));
		if (pred.is_name() && !pred.is_adjective()) {
			string head = extract_header(pred);
			// check that the Proper Name is not a nationality
			//double is_nationality = get_string_vector_distance(d, head, "inhabitant", k_, 6);
			bool is_nationality = d->hypernym(head, "inhabitant", 6) != 6;
			//double is_nationality= 0;
			if (is_nationality)
				pred_list.at(n).setTag("JJ");
		}
	}

	return pred_list;
}

static vector<DrtPred> eliminate_broken_complements(vector<DrtPred> speech)
// only to be used right before writing the words, when the complements and specifications are well-ordered
{
	vector<DrtPred>::iterator diter = speech.begin();
	vector<DrtPred>::iterator dend = speech.end();

	vector<DrtPred> already_parsed;
	vector<string> parsed_verb_ref;
	vector<string> parsed_name_ref;
	for (; diter != dend; ++diter) {
		string ref = extract_first_tag(*diter);
		if (boost::next(diter) != speech.end()) {
			DrtPred next_pred = *boost::next(diter);
			string header = extract_header(*diter);
			string sref1 = extract_second_tag(*diter);
			string fref2 = extract_first_tag(*boost::next(diter));
			if (diter->is_complement() && header.find(":DELETE") == string::npos && sref1 != fref2) {
				add_header(*diter, ":DELETE");
			}
		}
		if (boost::next(diter) == speech.end()) {
			if (diter->is_complement()) {
				add_header(*diter, ":DELETE");
			}
		}
	}
	return speech;
}

static string eliminate_par(string str)
{
	int i = 0;
	bool erase_trigger = false;
	while (i < str.size()) {
		// erase everything between []
		if (str.at(i) == '[')
			erase_trigger = true;
		if (str.at(i) == ']') {
			str.erase(i, 1);
			erase_trigger = false;
		}
		if (erase_trigger) {
			str.erase(i, 1);
			continue;
		}
		++i;
	}
	return str;
}

static bool compare_names(string lhs, string rhs)
{
	lhs = eliminate_par(lhs);
	rhs = eliminate_par(rhs);

	metric *d = metric_singleton::get_metric_instance();

	// put WP in here!
	vector<string> WPs;
	WPs.push_back("who");
	WPs.push_back("where");
	WPs.push_back("what");
	WPs.push_back("how");
	WPs.push_back("why");
	WPs.push_back("person");
	WPs.push_back("something");
	WPs.push_back("*something*");

	if (find(WPs.begin(), WPs.end(), lhs) != WPs.end() && find(WPs.begin(), WPs.end(), rhs) == WPs.end())
		return false;
	if (find(WPs.begin(), WPs.end(), lhs) == WPs.end() && find(WPs.begin(), WPs.end(), rhs) != WPs.end())
		return true;

	if (d->pertains_to_name(lhs, rhs, 8) < 0.5)
		return false;
	if (d->pertains_to_name(rhs, lhs, 8) > 0.5)
		return false;

	if (d->has_synset(lhs) && !d->has_synset(rhs))
		return true;
	if (!d->has_synset(lhs) && d->has_synset(rhs))
		return false;
	if (!d->has_synset(lhs) && !d->has_synset(rhs))
		return lhs.size() > rhs.size();

	if (d->hypernym(rhs, lhs, 8) == 8)
		return true;

	return false;
}

static bool compare_pred_names(const DrtPred &plhs, const DrtPred &prhs)
{
	string lhs = extract_header(plhs);
	string rhs = extract_header(prhs);
	lhs = eliminate_par(lhs);
	rhs = eliminate_par(rhs);

	string ltag = plhs.tag();
	string rtag = prhs.tag();

	bool lpivot = plhs.is_pivot();
	bool rpivot = prhs.is_pivot();

	metric *d = metric_singleton::get_metric_instance();

	// put WP in here!
	vector<string> WPs;
	WPs.push_back("person");
	WPs.push_back("color");
	WPs.push_back("something");
	WPs.push_back("who");
	WPs.push_back("whom");
	WPs.push_back("which");
	WPs.push_back("where");
	WPs.push_back("what");
	WPs.push_back("how");
	WPs.push_back("why");
	WPs.push_back("whose");
	WPs.push_back("*something*");
	WPs.push_back("[number]");
	WPs.push_back("[JJ]");
	WPs.push_back("[NNP]");
	WPs.push_back("[any]");
	WPs.push_back("[*]");
	WPs.push_back("");

	//cout << "LTAG:: " << ltag << ", " << rtag << endl;
	//cout << "LNAME:: "<< lhs << ", " << rhs << endl;
	//cout << "LPIVOT:: "<< lpivot << ", " << rpivot << endl;

	if (find(WPs.begin(), WPs.end(), lhs) != WPs.end() && find(WPs.begin(), WPs.end(), rhs) != WPs.end()) {
		vector<string>::iterator left = find(WPs.begin(), WPs.end(), lhs);
		vector<string>::iterator right = find(WPs.begin(), WPs.end(), rhs);
		int ldist = std::distance(WPs.begin(), left);
		int rdist = std::distance(WPs.begin(), right);
		return ldist < rdist;
	}

	if (find(WPs.begin(), WPs.end(), lhs) != WPs.end() && find(WPs.begin(), WPs.end(), rhs) == WPs.end())
		return false;
	if (find(WPs.begin(), WPs.end(), lhs) == WPs.end() && find(WPs.begin(), WPs.end(), rhs) != WPs.end())
		return true;

	if (lhs.find("#") != string::npos && rhs.find("#") == string::npos)
		return false;
	if (lhs.find("#") == string::npos && rhs.find("#") != string::npos)
		return false;

	if (!lpivot && rpivot)
		return false;
	if (lpivot && !rpivot)
		return true;

	if ((ltag == "JJ" || ltag == "JJS" || ltag == "JJR" || ltag == "PRP")
			&& (rtag == "NN" || rtag == "NNP" || rtag == "NNS" || rtag == "NNPS"))
		return false;
	if ((rtag == "JJ" || rtag == "JJS" || rtag == "JJR" || rtag == "PRP")
			&& (ltag == "NN" || ltag == "NNP" || ltag == "NNS" || ltag == "NNPS"))
		return true;

	if (d->pertains_to_name(lhs, rhs, 8) < 0.5)
		return false;
	if (d->pertains_to_name(rhs, lhs, 8) > 0.5)
		return false;

	if (d->has_synset(lhs) && !d->has_synset(rhs))
		return true;
	if (!d->has_synset(lhs) && d->has_synset(rhs))
		return false;
	if (!d->has_synset(lhs) && !d->has_synset(rhs))
		return lhs.size() > rhs.size();

	if (d->hypernym(rhs, lhs, 8) == 8)
		return true;

	return false;
}

bool ComparePredNames::operator()(const DrtPred &plhs, const DrtPred &prhs)
{
	string lhs = extract_header(plhs);
	string rhs = extract_header(prhs);
	lhs = eliminate_par(lhs);
	rhs = eliminate_par(rhs);

	string ltag = plhs.tag();
	string rtag = prhs.tag();

	bool lpivot = plhs.is_pivot();
	bool rpivot = prhs.is_pivot();

	metric *d = metric_singleton::get_metric_instance();

	// put WP in here!
	vector<string> WPs;
	WPs.push_back("person");
	WPs.push_back("color");
	WPs.push_back("number");
	WPs.push_back("something");
	WPs.push_back("who");
	WPs.push_back("whom");
	WPs.push_back("which");
	WPs.push_back("where");
	WPs.push_back("what");
	WPs.push_back("how");
	WPs.push_back("why");
	WPs.push_back("whose");
	WPs.push_back("*something*");
	WPs.push_back("[number]");
	WPs.push_back("[JJ]");
	WPs.push_back("[NNP]");
	WPs.push_back("[any]");
	WPs.push_back("[*]");
	WPs.push_back("");

	if (find(WPs.begin(), WPs.end(), lhs) != WPs.end() && find(WPs.begin(), WPs.end(), rhs) != WPs.end()) {
		vector<string>::iterator left = find(WPs.begin(), WPs.end(), lhs);
		vector<string>::iterator right = find(WPs.begin(), WPs.end(), rhs);
		int ldist = std::distance(WPs.begin(), left);
		int rdist = std::distance(WPs.begin(), right);
		return ldist < rdist;
	}

	if (find(WPs.begin(), WPs.end(), lhs) != WPs.end() && find(WPs.begin(), WPs.end(), rhs) == WPs.end())
		return false;
	if (find(WPs.begin(), WPs.end(), lhs) == WPs.end() && find(WPs.begin(), WPs.end(), rhs) != WPs.end())
		return true;

	if (lhs.find("#") != string::npos && rhs.find("#") == string::npos)
		return false;
	if (lhs.find("#") == string::npos && rhs.find("#") != string::npos)
		return false;

	if (!lpivot && rpivot)
		return false;
	if (lpivot && !rpivot)
		return true;

	if ((prhs.is_adjective() && d->pertains_to_name(rhs, question_str_, 9) > 0.5 && !plhs.is_adjective()))
		return false;
	if ((plhs.is_adjective() && d->pertains_to_name(lhs, question_str_, 9) > 0.5 && !prhs.is_adjective()))
		return true;

	if (d->hypernym(rhs, question_str_, 9) != 9 && d->hypernym(lhs, question_str_, 9) == 9)
		return false;
	if (d->hypernym(rhs, question_str_, 9) == 9 && d->hypernym(lhs, question_str_, 9) != 9)
		return true;

	if (priors_.hasPrior(lhs) && !priors_.hasPrior(rhs))
		return false;
	if (!priors_.hasPrior(lhs) && priors_.hasPrior(rhs))
		return true;

	if ((ltag == "JJ" || ltag == "JJS" || ltag == "JJR" || ltag == "PRP")
			&& (rtag == "NN" || rtag == "NNP" || rtag == "NNS" || rtag == "NNPS"))
		return false;
	if ((rtag == "JJ" || rtag == "JJS" || rtag == "JJR" || rtag == "PRP")
			&& (ltag == "NN" || ltag == "NNP" || ltag == "NNS" || ltag == "NNPS"))
		return true;

	if (d->pertains_to_name(lhs, rhs, 8) < 0.5)
		return false;
	if (d->pertains_to_name(rhs, lhs, 8) > 0.5)
		return false;

	if (d->has_synset(lhs) && !d->has_synset(rhs))
		return true;
	if (!d->has_synset(lhs) && d->has_synset(rhs))
		return false;
	if (!d->has_synset(lhs) && !d->has_synset(rhs))
		return lhs.size() > rhs.size();

	if (d->hypernym(rhs, lhs, 8) == 8)
		return true;

	return false;
}

static bool verb_is_singular(DrtPred pred, vector<DrtPred> speech, Knowledge *k)
{
	if (pred.is_verb()) {
		string subj_ref = extract_subject(pred);
		try { // try to see if the reference is in the data
			vector<DrtPred> pred_names = k->getPredsFromRef(subj_ref);
			speech.insert(speech.end(), pred_names.begin(), pred_names.end());
		} catch (std::runtime_error &exc) {
		}
		for (int n = 0; n < speech.size(); ++n) {
			if (speech.at(n).is_name()) {
				string fref = extract_first_tag(speech.at(n));
				if (fref == subj_ref && speech.at(n).is_plural()) {
					return false;
				}
			}
			if (speech.at(n).is_PRP()) {
				string fref = extract_first_tag(speech.at(n));
				string header = extract_header(speech.at(n));
				if (fref == subj_ref && (header == "i" || header == "we")) {
					return false;
				}
			}
			if (speech.at(n).is_complement()) {
				string fref = extract_first_tag(speech.at(n));
				string head = extract_header(speech.at(n));
				if (fref == subj_ref && (head == "@AND" || head == "@OR"))
					return false;
			}
		}
	}

	return true;
}

static string extract_conj(DrtPred pred, vector<DrtPred> speech, Knowledge *k)
{
	if(pred.tag() == "VBN")
		return "VBN";

	string tag = "V";

	if (pred.is_verb()) {
		string vref = extract_first_tag(pred);
		for (int n = 0; n < speech.size(); ++n) {
			if (speech.at(n).is_complement()) {
				string fref = extract_first_tag(speech.at(n));
				string sref = extract_second_tag(speech.at(n));
				string head = extract_header(speech.at(n));
				if (fref == vref && head == "@TIME") {
					if (sref.find("past") != string::npos)
						tag = "VBD";
					else if (sref.find("present") != string::npos) {
						if (verb_is_singular(pred, speech, k))
							tag = "VBZ";
						else
							tag = "VBP";
					}
					break;
				}
			}
		}
		if (tag == "V") { // No tense found, use present
			if (verb_is_singular(pred, speech, k))
				tag = "VBZ";
			else
				tag = "VBP";
		}
	}

	return tag;
}

static bool has_negation(DrtPred pred, vector<DrtPred> speech)
{
	if (pred.is_verb()) {
		string vref = extract_first_tag(pred);
		for (int n = 0; n < speech.size(); ++n) {
			if (speech.at(n).is_adverb()) {
				string fref = extract_first_tag(speech.at(n));
				string head = extract_header(speech.at(n));
				if (fref == vref && head == "not")
					return true;
			}
		}
	}
	return false;
}

string Writer::getNumberFromRef(const string ref)
{
	string number = "";
	try { // try to see if the reference is in the data
		vector<DrtVect> specifications = k_.getSpecificationsFromRef(ref);
		for (int n = 0; n < specifications.size(); ++n) {
			if (specifications.at(n).size() < 2)
				continue;
			DrtPred pred = specifications.at(n).at(0);
			string head = extract_header(pred);
			if (head == "@QUANTITY") {
				pred = specifications.at(n).at(1);
				number = extract_header(pred);
			}
		}
	} catch (std::runtime_error &exc) {
	}

	return number;
}

static bool has_verbatim(const DrtVect &drtvect)
{
	for (int n = 0; n < drtvect.size(); ++n) {
		if (drtvect.at(n).is_verbatim())
			return true;
	}

	return false;
}

vector<DrtVect> Writer::getJoinedNounsFromRef(const string nref)
{
	vector<DrtVect> to_return;

	vector<string> all_refs;
	all_refs.push_back(nref);
	for (int m = 0; m < all_refs.size(); ++m) {
		string ref = all_refs.at(m);
		try { // try to see if the reference is in the data
			vector<DrtVect> specifications = k_.getSpecificationsFromRef(ref);

			for (int n = 0; n < specifications.size(); ++n) {
				if (specifications.at(n).size() < 2)
					continue;
				DrtPred pred = specifications.at(n).at(0);
				string head = extract_header(pred);
				string sref = extract_second_tag(pred);
				if ((head == "@AND" || head == "@OR") && !has_verbatim(specifications.at(n))) {
					to_return.push_back(specifications.at(n));
					//all_refs.push_back(sref);
				}
			}
		} catch (std::runtime_error &exc) {
		}
	}

	return to_return;
}

static DrtPred clean_header(DrtPred dpred)
// some predicates can be PRED(A|B) -> PRED(A)
{
	string head = extract_header(dpred);
	int pos = head.rfind("|");
	if (pos != string::npos) {
		head = head.substr(pos, head.size());
		implant_header(dpred, head);
		dpred.name() = head;
	}
	return dpred;
}

static string clean_string(string head)
// some predicates can be PRED(A|B) -> PRED(A)
{
	int pos = head.rfind("|");
	if (pos != string::npos && pos < head.size() - 1) {
		head = head.substr(pos + 1, head.size());
	}
	return head;
}

static bool has_vowel(const string &str)
{
	vector<char> vowels;
	vowels.push_back('a');
	vowels.push_back('e');
	vowels.push_back('i');
	vowels.push_back('o');
	vowels.push_back('u');
	vowels.push_back('y');

	int i;
	for (i = 0; i < str.size() && str.at(i) == ' '; ++i)
		;

	char first_char = str.at(i);
	if (shortfind(vowels, first_char))
		return true;

	return false;
}

static bool noun_supports_article(const string &str)
{
	if (str == "this" || str == "these" || str == "that" || str == "those" || str == "something" || str == "anything" || str == "all")
		return false;
	return true;
}

static inline double max(double a, double b) {return a<b?b:a; }

vector<DrtPred> select_answer_to_question(const vector<DrtPred> &preds, const DrtPred &question)
{
	vector<DrtPred> to_return;
	DrtPred first_pred;
	string question_str= extract_header(question);
	int first_pred_pos= -1;
	double pertayn_w=-1, hypernym_w = -1, w= 0.2;
	metric *d = metric_singleton::get_metric_instance();
	for(int n=0; n < preds.size(); ++n) {
		string header = extract_header(preds.at(n));
		pertayn_w  = d->pertains_to_name(header, question_str, 9);
		hypernym_w = d->hypernym_dist   (header, question_str, 9);
		if(question_str == "material") {
			double hypernym_w_metal= d->hypernym_dist(header, "metal", 9);
			hypernym_w= max(hypernym_w,hypernym_w_metal);
		}
		string tag = preds.at(n).tag();
		if(header != question_str && header != "[number]" && tag != "PRP" && pertayn_w > hypernym_w && pertayn_w > w) {
			w= pertayn_w;
			first_pred_pos= n;
		}
		if(header != question_str && header != "[number]" && tag != "PRP" && hypernym_w > pertayn_w && hypernym_w > w) {
			w= hypernym_w;
			first_pred_pos=n;
		}
	}
	if(first_pred_pos != -1) {
		to_return.push_back(preds.at(first_pred_pos) );
		to_return.insert(to_return.end(), preds.begin(),preds.end() );
	} else {
		to_return= preds;
	}
	return to_return;
}

static string avoid_embarassing_names(const DrtPred &pred, const string &name)
{
	string to_return(name);

	string question_str = extract_header(pred);
	if(question_str == "what" && name == "what")
		to_return= "something";
	if(question_str == "[what]" && name == "[what]")
		to_return= "something";

	return to_return;
}

static bool verb_has_object(const DrtPred &pred)
{
	string obj = extract_object(pred);
	if (debug)
		cout << "OBJ::: " << obj << endl;
	if (obj.find("obj") == string::npos && obj.find("none") == string::npos && obj.find("subj") == string::npos // The passive verbs invert the tags
	    )
		return true;
	return false;
}


static string avoid_embarassing_verb_names(const DrtPred &pred, const string &name, string *tag)
{
	string to_return(name);

	string question_str = extract_header(pred);
	if(question_str == "what-happens" && name == "what-happens" && *tag != "VBZ") {
		if(verb_has_object(pred) ) {
			to_return= "do something about";
		} else {
			to_return= "do something";
		}
		*tag = "VB"; // no need to conjugate afterwards
	}
	if(question_str == "what-happens" && name == "what-happens" && *tag == "VBZ") {
		if(verb_has_object(pred) ) {
			to_return= "does something about";
		} else {
			to_return= "does something";
		}
		*tag = "VB"; // no need to conjugate afterwards
	}
	if(question_str == "do-something" && name == "do-something" && *tag != "VBZ") {
		if(verb_has_object(pred) ) {
			to_return= "do something about";
		} else {
			to_return= "do something";
		}
		*tag = "VB"; // no need to conjugate afterwards
	}
	if(question_str == "do-something" && name == "do-something" && *tag == "VBZ") {
		if(verb_has_object(pred) ) {
			to_return= "does something about";
		} else {
			to_return= "does something";
		}
		*tag = "VB"; // no need to conjugate afterwards
	}

	return to_return;
}

static string get_appropriate_complement_name(const DrtVect &speech, const DrtPred &pred)
{
	string ret_str;
	string header= extract_header(pred);
	map<string, string> all_complements = get_all_complement_conversion();
	map<string, string>::iterator aiter = all_complements.find(header);
	if (aiter != all_complements.end())
		ret_str = aiter->second;

	string sref= extract_second_tag(pred);
	int m= find_name_with_string(speech,sref);

	if ( m != -1 ) {
		if(header == "@TIME_AT" && speech.at(m).is_date() )
			ret_str = "on";
		if(header == "@TIME_AT" && !ref_is_ref(extract_first_tag(speech.at(m))) )
			ret_str = "on"; // on an occasion, at the occasion
	}

	return ret_str;
}

static vector<DrtPred> eliminate_impossible(const vector<DrtPred> &preds)
{
	vector<DrtPred> to_return;
	if(preds.size() == 1)
		return preds;

	for(int n=0; n < preds.size(); ++n) {
		if (!preds.at(n).is_PRP() ) // they is not a valid name to print from the data
			to_return.push_back(preds.at(n));
	}

	return to_return;
}

static int get_key_from_header(const string &header)
{
	int to_return = -1;

	int pos= header.find(":Q");
	if( pos != string::npos) {
		try {
			string num_str = header.substr(pos+2,header.size() );
			int num        = boost::lexical_cast<int>(num_str);
			to_return      = num;
		} catch(...) {
			;// Q cannot be converted to an integer
		}
	}

	return to_return;
}

pair<string, DrtPred> Writer::getStringFromPred(DrtPred pred, vector<DrtPred> speech, bool conjugate, Priors *priors, DrtPred *printed_pred)
{
	tagger_info *info = parser_singleton::get_tagger_info_instance();
	metric *d = metric_singleton::get_metric_instance();

	string head = extract_header(pred);


	string ret_str = "";
	DrtPred pred_str(pred);
	if (pred.is_name()) {
		string article = "", number = "";
		string ref_str = extract_first_tag(pred);

		clock_t start;
		vector<DrtPred> pred_names, pred_rulenames;
		pred_names = k_.getPredsFromRef(ref_str);

		int key = get_key_from_header( extract_header(pred) );
		if(key != -1) {
			string new_header= info->get_wikidata_name(key);
			if(new_header != "")
				implant_header(pred,new_header);
		}
		string tmp_header= extract_header(pred);
		if(tmp_header.find(":Q[") != string::npos) {
			int pos = tmp_header.find(":Q");
			string str= tmp_header.substr(pos+2,tmp_header.size());
			implant_header(pred,str);
		}

		pred_names.push_back(pred);

		pred_names = eliminate_impossible(pred_names);

		if (pred_names.size()) {
			pred_names = sign_adjectives(pred_names, &k_);

			if(pred.is_question()) {
				ComparePredNames compare(*priors, extract_header(pred));
				sort(pred_names.begin(), pred_names.end(), compare);
				pred_names= select_answer_to_question(pred_names, pred);
			} else {
				ComparePredNames compare(*priors, "");
				sort(pred_names.begin(), pred_names.end(), compare);
			}
			string word = "";
			DrtPred pred_name;
			// take the first name that is not ""
			for (int n = 0; word.size() == 0 && n < pred_names.size() && n < 10; ++n) {
				pred_name = pred_names.at(n);
				if(pred_names.size() > 1 && n==0 && extract_header(pred_name) == "do-something")
					continue;
				word = extract_header(pred_name);
			}
			for (int n = 0; n < pred_names.size() && n < 10; ++n) {
				string header= extract_header(pred_names.at(n));
				if(header == "[act]") {// this has the precedence over all names
					pred_name = pred_names.at(n);
					word = extract_header(pred_name);
					break;
				}
			}
			if (!pred_name.is_adjective() && d->is_country(word))
				pred_name.setTag("NNP");
			ret_str = extract_header(pred_name);
			implant_header(pred_str,extract_header(pred_name) );
			pred_str.setTag(pred_name.tag() );
			*printed_pred = pred_name;
			ret_str= avoid_embarassing_names(pred, ret_str);
			priors->addNoun(ret_str);
			if (ret_str.size() == 0) {
				ret_str = "something";
				implant_header(pred_str,"something");
			}
			string name_tag = pred_name.tag();
			ret_str = info->conjugate(ret_str, name_tag);
			if (pred_name.is_verbatim()) {
				ret_str = ret_str.substr(11, ret_str.size());
				ret_str = string("\"") + ret_str + "\"";
			}
			if (pred_name.is_proper_name() && !pred_name.is_verbatim()) {
				ret_str.at(0) = std::toupper(ret_str.at(0));
			}
			if (!pred_name.is_proper_name() && !pred_name.is_PRP() && !pred_name.is_plural() && !pred_name.is_number()
					&& !pred_name.is_adjective() && !pred_name.is_place() && !pred_name.is_date()
					&& noun_supports_article(ret_str)) {
				if (ref_str.find("name") != string::npos)
					article = "a";
				else if (ref_str.find("ref") != string::npos)
					article = "the";
			}
		} else
			ret_str = head;
		if (ret_str.find("[*]") != string::npos)
			ret_str = "something";
		if (ret_str == "[act]" )
			ret_str = "act";
		number = getNumberFromRef(ref_str);
		bool has_number= false;
		if(number != "")
			has_number= true;
		ret_str = number + " " + clean_string(ret_str);
		if(!has_number) {
			if (has_vowel(ret_str) && article == "a")
				ret_str = article + "n " + clean_string(ret_str);
			else
				ret_str = article + " " + clean_string(ret_str);
		}

	} else if (pred.is_verb()) {
		string ref_str = extract_first_tag(pred);
		string verb_tag;
		if (pred.tag() == "VBG")
			verb_tag = "VBG";
		else
			verb_tag = extract_conj(pred, speech, &k_);
		//vector<string> verb_names = ps.getVerbNames(ref_str);
		vector<string> verb_names = k_.getVerbNamesFromRef(ref_str);

		//Rules rs = k_.getRulesPersonae();

		verb_names.push_back(extract_header(pred));
		string RP_str = "";
		if (verb_names.size()) {
			ret_str = verb_names.at(0);
			ret_str= avoid_embarassing_verb_names(pred,ret_str,&verb_tag);
			if (ret_str.find("_") != string::npos) {
				vector<string> strs;
				boost::split(strs, ret_str, boost::is_any_of("_"));
				if (strs.size() > 1) {
					ret_str = strs.at(0);
					RP_str = strs.at(1);
				}
			}
			if (has_negation(pred, speech)) {
				if(verb_tag == "VBN") {
					ret_str = "not " + info->conjugate(ret_str, "VBN");
				} else if (conjugate) {
					string do_str = info->conjugate("do", verb_tag);
					if (do_str == "don")
						do_str = "do";
					ret_str = do_str + " not " + ret_str;
				} else {
					ret_str = string("not ") + ret_str;
				}
			} else {
				if (conjugate) {
					ret_str = info->conjugate(ret_str, verb_tag);

					if (ret_str == "haven")
						ret_str = "have";
					if (ret_str == "don")
						ret_str = "do";
				}
			}
		} else
			ret_str = head;
		if (RP_str != "")
			ret_str += string(" ") + RP_str;
	} else if (head.size() && head.at(0) == '@') {
		if (head.find("|")) {
			vector<string> strings;
			boost::split(strings, head, boost::is_any_of("|"));
			head = strings.at(0);
			implant_header(pred,head);
		}
		string compl_str = get_appropriate_complement_name(speech,pred);
		return make_pair(compl_str,pred_str);
	} else if (pred.is_adverb()) {
		ret_str = "";
	} else {
		ret_str = head;
	}

	return make_pair(ret_str,pred_str);
}

static vector<string> get_missing_references(const vector<DrtPred> &drs)
// Find the references that do not have a predicate in drs. 
// Example: he(name0) make(verb1,name0,name2) coffee(name2) @PLACE_AT(verb1,name3)
//          => name3 is missing
{
	vector<string> missing_refs;
	vector<DrtPred>::const_iterator diter = drs.begin();
	vector<DrtPred>::const_iterator dend = drs.end();
	vector<string> present_refs;

	// finds the references that are actually present
	for (; diter != dend; ++diter) {
		vector<string> children = diter->extract_children();
		if (children.size() == 1)
			present_refs.push_back(children.at(0));
	}
	// finds the missing references
	diter = drs.begin();
	for (; diter != dend; ++diter) {
		vector<string> children = diter->extract_children();
		int n = 0;
		if (diter->is_WP_pos())
			++n; // for WP$ skip the the first ref
		for (; n < children.size(); ++n) {
			string ref = children.at(n);
			if ((ref.find("name") != string::npos || ref.find("ref") != string::npos
					|| (n != 0 && ref.find("verb") != string::npos) // there can be verbs as objects (keeping trained, ...)
			) && find(present_refs.begin(), present_refs.end(), ref) == present_refs.end()) {
				if (ref.find("verb") != string::npos)
					missing_refs.push_back(ref + ":verb");
				else
					missing_refs.push_back(ref);
			}
		}
	}
	return missing_refs;
}

static vector<DrtPred> create_preds_with_refs(const vector<string> &refs, Knowledge *k)
{
	vector<DrtPred> to_return;

	vector<string>::const_iterator riter = refs.begin();
	vector<string>::const_iterator rend = refs.end();
	for (; riter != rend; ++riter) {
		if (riter->find(":verb") != string::npos) {
			int pos = riter->find(":");
			string verb_ref = riter->substr(0, pos);
			DrtPred tmp_pred;
			vector<DrtPred> vpreds = k->getVerbPredsFromRef(verb_ref);
			if (vpreds.size() == 0)
				tmp_pred = DrtPred(string("do") + "(" + verb_ref + ")");
			else
				tmp_pred = vpreds.at(0);
			tmp_pred.setTag("VBP");
			to_return.push_back(tmp_pred);
		} else {
			DrtPred tmp_pred(string("something/NN") + "(" + *riter + ")");
			try { // try to see if the reference is in the data
				vector<DrtPred> pred_names = k->getPredsFromRef(*riter);
				if (pred_names.size())
					tmp_pred = pred_names.at(0);
			} catch (std::runtime_error &exc) {
			}
			to_return.push_back(tmp_pred);
		}
	}
	return to_return;
}

static bool compare_drt(const DrtPred &drt_sx, const DrtPred &drt_dx)
{
	int dist_sx, dist_dx;
	string str_sx, str_dx;
	string non_verb_str, subj_str, obj_str;

	// The subject goes before the verb and the object after
	if (drt_sx.is_verb() && drt_dx.is_name()) {
		subj_str = extract_subject(drt_sx);
		obj_str = extract_object(drt_sx);
		non_verb_str = extract_first_tag(drt_dx);
		if (subj_str == non_verb_str)
			return false;
		else if (obj_str == non_verb_str)
			return true;
		else
			return false;
	} else if (drt_sx.is_name() && drt_dx.is_verb()) {
		subj_str = extract_subject(drt_dx);
		obj_str = extract_object(drt_dx);
		non_verb_str = extract_first_tag(drt_sx);
		if (obj_str == non_verb_str)
			return false;
		else if (subj_str == non_verb_str)
			return true;
		else
			return false;
	}

	// Complements marked with '@' go before the object they point to
	string name_sx, name_dx;
	string first_ref, second_ref, name_ref;
	name_sx = extract_header(drt_sx);
	name_dx = extract_header(drt_dx);
	if (name_sx.size() && name_dx.size() && name_sx.at(0) == '@' && name_dx.at(0) != '@') {
		second_ref = extract_second_tag(drt_sx);
		name_ref = extract_first_tag(drt_dx);
		if (second_ref == name_ref)
			return true;
		return false;
	} else if (name_sx.size() && name_dx.size() && name_sx.at(0) != '@' && name_dx.at(0) == '@') {
		second_ref = extract_second_tag(drt_dx);
		name_ref = extract_first_tag(drt_sx);
		if (second_ref == name_ref)
			return false;
		return true;
	} else if (name_sx.size() && name_dx.size() && name_dx.at(0) == '@' && name_sx.at(0) == '@') // if both @-names order alphabetically
		return name_sx < name_dx;

	// If two words share the same reference do nothing
	str_sx = extract_first_tag(drt_sx);
	str_dx = extract_first_tag(drt_dx);

	if (str_sx != str_dx)
		return false;

	//return dist_sx < dist_dx;
	return true;
}

static void gnomesort(vector<DrtPred>::iterator start, vector<DrtPred>::iterator end)
{
	vector<DrtPred> phrase_old(start, end);
	vector<DrtPred> phrase_new;
	vector<DrtPred>::iterator iter = start;

	int n = 0; // the cycle has to stop
	while (phrase_new != phrase_old && ++n < 20) {
		phrase_old = phrase_new;
		//print_vector(phrase_new);
		iter = start;
		while (boost::next(iter) != end) {
			if (compare_drt(*iter, *boost::next(iter)))
				++iter;
			else {
				DrtPred tmp = *iter;
				*iter = *boost::next(iter);
				++iter;
				*iter = tmp;
			}
		}
		phrase_new = vector<DrtPred>(start, end);
	}
}

static void writer_sort(vector<DrtPred> &pre_drt)
{
	gnomesort(pre_drt.begin(), pre_drt.end());
}

vector<DrtVect> Writer::get_subordinates(const vector<DrtPred> &drs)
{
	vector<DrtVect> to_return;
	vector<string> already_found;
	DrtVect old_drs(drs), new_drs(drs);

	bool first_trigger = true;
	while (first_trigger || old_drs != new_drs) {
		first_trigger = false;
		old_drs = new_drs;
		vector<DrtPred>::iterator diter = new_drs.begin();
		vector<DrtPred>::iterator dend = new_drs.end();
		// finds the subordinates
		for (; diter != dend; ++diter) {
			if (diter->is_verb()) {
				string verb_ref = extract_first_tag(*diter);
				if (shortfind(already_found, verb_ref))
					continue;
				try {
					//cout << "VREF:::" << verb_ref << endl;
					vector<pair<DrtPred, Action> > sub_actions = k_.getSubordinatesFromRef(verb_ref);
					already_found.push_back(verb_ref);
					for (int n = 0; n < sub_actions.size(); ++n) {
						DrtVect tmp_drtvect;
						tmp_drtvect.push_back(sub_actions.at(n).first);
						DrtVect drs = sub_actions.at(n).second.getDrs();
						tmp_drtvect.insert(tmp_drtvect.end(), drs.begin(), drs.end());
						to_return.push_back(tmp_drtvect);
						//cout << "DITER::: " << *diter << endl;
						new_drs.insert(new_drs.end(), drs.begin(), drs.end());
						diter = new_drs.begin();
						dend = new_drs.end();
					}
					//return to_return;
				} catch (std::runtime_error &e) {
					//cout << e.what() << endl;
					//return to_return;
				}
			}
		}
	}

	return to_return;
}

vector<vector<DrtPred> > Writer::get_missing_complement_preds(const vector<DrtPred> &drs)
{
	vector<vector<DrtPred> > to_return;

	vector<DrtPred>::const_iterator diter = drs.begin();
	vector<DrtPred>::const_iterator dend = drs.end();
	// finds the subordinates
	for (; diter != dend; ++diter) {
		if (diter->is_verb()) {
			string subj_ref = extract_subject(*diter);
			string verb_ref = extract_first_tag(*diter);
			Action action = k_.getActionFromVerbRef(subj_ref, verb_ref);
			vector<DrtVect> complements = action.getComplements();
			to_return.insert(to_return.end(), complements.begin(), complements.end());
		}
	}
	for (diter = drs.begin(); diter != dend; ++diter) {
		if (diter->is_verb()) {
			string subj_ref = extract_subject(*diter);
			string verb_ref = extract_first_tag(*diter);
			ConditionalAction action = k_.getConditionalActionFromVerbRef(subj_ref, verb_ref);
			vector<DrtVect> complements = action.getComplements();
			to_return.insert(to_return.end(), complements.begin(), complements.end());
		}
	}

	return to_return;
}

static vector<vector<DrtPred> > filter_valid_specifications(const vector<vector<DrtPred> > &specs)
{
	vector<vector<DrtPred> > to_return;

	vector<vector<DrtPred> >::const_iterator diter = specs.begin();
	vector<vector<DrtPred> >::const_iterator dend = specs.end();

	for (; diter != dend; ++diter) {
		DrtVect tmp_drt = *diter;

		string head = extract_header(diter->at(0));
		if (head == "@GENITIVE" || head == "@MORE_THAN" || head == "@LESS_THAN" || head == "@COMPARED_TO")
			to_return.push_back(*diter);
	}
	return to_return;
}

vector<vector<DrtPred> > Writer::get_missing_specification_preds(const vector<DrtPred> &drs)
{
	vector<vector<DrtPred> > to_return;

	vector<DrtPred>::const_iterator diter = drs.begin();
	vector<DrtPred>::const_iterator dend = drs.end();
	// finds the subordinates
	for (; diter != dend; ++diter) {
		if (diter->is_name()) {
			string subj_ref = extract_first_tag(*diter);
			vector<vector<DrtPred> > specifications;
			try {
				specifications = filter_valid_specifications(k_.getSpecificationsFromRef(subj_ref));
				to_return.insert(to_return.end(), specifications.begin(), specifications.end());
				return to_return;
			} catch (std::runtime_error &e) {
				continue;
			}
		}
	}

	return to_return;
}

vector<DrtPred> eliminate_subject(const vector<DrtPred> &preds)
{
	vector<DrtPred> to_return(preds);

	for (int n = 0; n < to_return.size(); ++n) {
		if (to_return.at(n).is_name() && points_to_subject(preds, to_return.at(n))) {
			to_return.erase(to_return.begin() + n);
		}
	}

	return to_return;
}

static bool is_valid_element(const DrtPred &pred)
// Return true if the element is an actual reference.
{
	string head = extract_header(pred);
	if (head.find(":DELETE") != string::npos)
		return false;

	string name_str = extract_first_tag(pred);
	if (name_str.find("obj") == string::npos && name_str.find("subj") == string::npos && name_str.find("next") == string::npos
			&& name_str.find("prev") == string::npos && name_str.find("to") == string::npos
			&& name_str.find("from") == string::npos)
		return true;
	return false;
}

vector<DrtVect> add_sign_to_names(vector<DrtVect> complements)
// the names of a complement must be signed differently from the names of a subject or object
{
	vector<DrtVect>::iterator citer = complements.begin();
	vector<DrtVect>::iterator cend = complements.end();

	for (; citer != cend; ++citer) {
//          if(citer->size() == 0) 
//               continue;
		vector<DrtPred>::iterator c2iter = citer->begin();
		vector<DrtPred>::iterator c2end = citer->end();
		string cname = "";
		for (; c2iter != c2end; ++c2iter) {
			if (c2iter->is_complement() && cname == "")
				cname = extract_header(*c2iter);
			if (c2iter->is_name() && cname != "") {
				string head = extract_header(*c2iter);
				implant_header(*c2iter, head + "[" + cname + "]");
				c2iter->name() = head + "[" + cname + "]";
			}
		}
	}
	return complements;
}

vector<DrtPred> Writer::writer_graph_sort(const SingleMatchGraph &graph, const MatchWrite &mgraph)
{
	vector<DrtPred> ordered;

	vector<DrtPred> subj, obj, adv;
	vector<vector<DrtPred> > subj_specs, obj_specs, complements, compl_specs;
	DrtPred verb;

	subj = graph.getSubject();
	if (subj.size()) {
		subj_specs = mgraph.getSpecifications(subj.at(0));
		subj_specs = filter_valid_specifications(subj_specs); // eliminate @OWN-type complements
	}
	ordered.insert(ordered.end(), subj.begin(), subj.end());

	if (!graph.hasVerb()) // If there is no verb end here
		return ordered;

	obj = graph.getObject();
	if (obj.size()) {
		obj_specs = mgraph.getSpecifications(obj.at(0));
		obj_specs = filter_valid_specifications(obj_specs); // eliminate @OWN-type complements
		//obj_specs = add_sign_to_names(obj_specs);
	}
	verb = graph.getVerb();
	complements = graph.getComplements();
	for (int m = 0; m < complements.size(); ++m) {
		for (int n = 0; n < complements.at(m).size(); ++n) {
			vector<DrtVect> tmp_compl;
			tmp_compl = mgraph.getSpecifications(complements.at(m).at(n));
			subj_specs = filter_valid_specifications(subj_specs);
			if (tmp_compl.size()) {
				compl_specs.insert(compl_specs.end(), tmp_compl.begin(), tmp_compl.end());
			}
			//complements = filter_valid_specifications(subj_specs); // eliminate @OWN-type complements
		}
	}
	//compl_specs = add_sign_to_names(compl_specs);

	adv = graph.getAdverb();
	ordered.insert(ordered.end(), adv.begin(), adv.end());

	ordered.push_back(verb);
	ordered.insert(ordered.end(), obj.begin(), obj.end());

	for (int n = 0; n < complements.size(); ++n) {
		ordered.insert(ordered.end(), complements.at(n).begin(), complements.at(n).end());
	}
	ordered = eliminateRedundant(ordered);

	return ordered;
}

static vector<DrtPred>::iterator writer_find(vector<DrtPred>::iterator first, vector<DrtPred>::iterator last, const DrtPred& val)
{
	while (first != last) {
		string first_ref = extract_first_tag(*first);
		string second_ref = extract_second_tag(*first);
		string third_ref = extract_third_tag(*first);
		string first_ref2 = extract_first_tag(val);
		string second_ref2 = extract_second_tag(val);
		string third_ref2 = extract_third_tag(val);

		if (first_ref == first_ref2 && second_ref == second_ref2 && third_ref == third_ref2)
			return first;
		++first;
	}
	return last;
}

void Writer::writer_unique(vector<DrtPred> &preds)
{
	vector<DrtPred> already_present;

	for (int n = 0; n < preds.size(); ++n) {
		if (writer_find(already_present.begin(), already_present.end(), preds.at(n)) == already_present.end()) {
			already_present.push_back(preds.at(n));
		} else {
			preds.erase(preds.begin() + n);
			--n;
		}
	}
}

vector<DrtPred> get_drtvect_from_sentence_list(const vector<pair<string, vector<DrtPred> > > &sentence_list,
		const string &from_str)
{
	vector<DrtPred> to_return;
	for (int n = 0; n < sentence_list.size(); ++n) {
		if (sentence_list.at(n).first == from_str) {
			to_return = sentence_list.at(n).second;
			break;
		}
	}
	return to_return;
}

static SentenceTree build_sentence_tree_from_strings(const FTree<string>::iterator &i,
		const vector<pair<string, vector<DrtPred> > > &sentence_list, map<string, DrtPred> map_vtos)
{
	Node<string> *run = i.node;
	vector<DrtPred> drtvect = get_drtvect_from_sentence_list(sentence_list, run->data);
	DrtPred dummyhead("@HEAD(dummy,dummy)");
	SentenceTree to_return(make_pair(dummyhead, drtvect));
	SentenceTree::iterator inTree(to_return.begin());

	if (run->firstChild) {
		run = run->firstChild;
		drtvect = get_drtvect_from_sentence_list(sentence_list, run->data);
		string tmpstr = run->data;
		DrtPred pointer = map_vtos[tmpstr];
		inTree = to_return.appendChild(inTree, make_pair(pointer, drtvect));
		while (true) {
			if (run->parent == i.node && !run->firstChild && !run->nextSibling)
				break;
			else if (run->firstChild) {
				run = run->firstChild;
				drtvect = get_drtvect_from_sentence_list(sentence_list, run->data);
				DrtPred pointer = map_vtos[run->data];
				inTree = to_return.appendChild(inTree, make_pair(pointer, drtvect));
			} else if (run->nextSibling) {
				run = run->nextSibling;
				drtvect = get_drtvect_from_sentence_list(sentence_list, run->data);
				DrtPred pointer = map_vtos[run->data];
				inTree = to_return.appendChild(inTree.parent(), make_pair(pointer, drtvect));
			} else {
				while (run->parent && !run->nextSibling && run->parent != i.node) {
					run = run->parent;
					inTree = inTree.parent();
				}
				if (run->nextSibling) {
					run = run->nextSibling;
					drtvect = get_drtvect_from_sentence_list(sentence_list, run->data);
					DrtPred pointer = map_vtos[run->data];
					inTree = to_return.appendChild(inTree.parent(), make_pair(pointer, drtvect));
				} else
					break;
			}
		}
	}

	return to_return;
}

SentenceTree Writer::getSentenceTree(vector<DrtPred> speech)
{
	SentenceTree to_return;

	speech = eliminateRedundant(speech); // if the same ref appears twice one of the two names is deleted

	MatchWrite mgraph(speech);

	vector<SingleMatchGraph> sentences = mgraph.getSentences();

	vector<SingleMatchGraph>::iterator siter = sentences.begin();
	vector<SingleMatchGraph>::iterator send = sentences.end();

	vector<pair<string, vector<DrtPred> > > sentence_list;
	vector<pair<string, DrtPred> > sentence_preps;
	// pair with the verb reference and the drtvect

	for (; siter != send; ++siter) {
		string verb_ref = extract_first_tag(siter->getVerb());
		DrtVect drs = writer_graph_sort(*siter, mgraph);
		sentence_list.push_back(make_pair(verb_ref, drs));
	}

	// put the sentences on a tree according to the subordinate structure

	vector<DrtPred> subs = mgraph.getSubs();
	string from_str, to_str;
	// create a tree of verb references
	FTree<string> str_tree;
	map<string, DrtPred> map_vtos;
	if (subs.size()) {
		FTree<string>::iterator titer;
		from_str = extract_first_tag(subs.at(0));
		*str_tree.begin() = from_str;
		for (int n = 0; n < subs.size(); ++n) {
			from_str = extract_first_tag(subs.at(n));
			to_str = extract_second_tag(subs.at(n));
			map_vtos[to_str] = subs.at(n);
			titer = str_tree.findData(from_str, str_tree.begin());
			if (titer != str_tree.end()) {
				str_tree.appendChild(titer, to_str);
				//cout << "TREE:: " << to_str << endl;
			}
		}
	} else {
		// if there is only one sentence then the tree contains only that
		// sentence
		string verb_ref = extract_first_tag(sentences.begin()->getVerb());
		*str_tree.begin() = verb_ref;
	}

	to_return = build_sentence_tree_from_strings(str_tree.begin(), sentence_list, map_vtos);

	return to_return;
}

static DrtVect order_subordinate_elements(DrtVect speech)
{
	for (int n = 0; n < speech.size(); ++n) {
		if (speech.at(n).is_complement()) {
			string head = extract_header(speech.at(n));
			if (head == "@QUANTITY") {
				string fref = extract_first_tag(speech.at(n));
				string sref = extract_second_tag(speech.at(n));
				int q = find_name_with_string(speech, sref);
				for (int m = q; m > 0; --m) {
					string fref2 = extract_first_tag(speech.at(m));
					if (fref2 == fref) {
						speech.insert(speech.begin() + m - 1, speech.at(q));
						add_header(speech.at(q + 1), ":DELETE");
						break;
					}
				}
			}
		}
	}
	return speech;
}

static DrtVect separate_composed_verbs(DrtVect speech)
// Some verbs like put_up, take_place, are composed. This function separate the elements
{
	DrtVect::iterator siter = speech.begin(), send = speech.end();
	for (; siter != send; ++siter) {
		if (siter->is_verb()) {
			string head = extract_header(*siter);
			vector<string> strs;
			boost::split(strs, head, boost::is_any_of("_"));
			if (strs.size() > 1) {
				string head1 = strs.at(0);
				string head2 = strs.at(1);
				implant_header(*siter, head1);
				siter->name() = head1;
				DrtPred RP_pred(*siter);
				implant_header(RP_pred, head2);
				RP_pred.name() = head2;
				RP_pred.setTag("RP");
				siter = speech.insert(siter + 1, RP_pred);
				send = speech.end();
			}
		}
	}
	return speech;
}

DrtVect Writer::insertJoinedNouns(DrtVect speech)
{
	vector<string> already_done;
	for (int m = 0; m < speech.size(); ++m) {
		if (!speech.at(m).is_name())
			continue;
		string ref = extract_first_tag(speech.at(m));
		if (shortfind(already_done, ref))
			continue;
		vector<DrtVect> specs = getJoinedNounsFromRef(ref);
		//cout << "J:::" << specs.size() << endl;
		for (int n = 0; n < specs.size(); ++n) {
			speech.insert(speech.begin() + m + 1, specs.at(n).begin(), specs.at(n).end());
			already_done.push_back(ref);
		}
	}
	return speech;
}

static bool spec_is_already_present(const DrtVect &speech, const DrtVect &spec)
{
	for (int n = 0; n < spec.size(); ++n) {
		if (shortfind(speech, spec.at(n)))
			return true;
	}
	return false;
}

static DrtVect process_missing_elements(DrtVect drtvect, Knowledge *k)
{
	vector<string> missing_refs = get_missing_references(drtvect);
	vector<string> already_parsed;
	for (int n; n < drtvect.size(); ++n) {
		if (drtvect.at(n).is_verb()) {
			string oref = extract_object(drtvect.at(n));
			string vref = extract_first_tag(drtvect.at(n));
			if (shortfind(missing_refs, oref) && !shortfind(already_parsed, oref)) {
				DrtPred tmp_pred(string("something/NN") + "(" + oref + ")");
				drtvect.insert(drtvect.begin() + n + 1, tmp_pred);
				already_parsed.push_back(oref);
				drtvect.at(n).setTag("VBG");
				++n;
			}
		}
	}
	return drtvect;
}

DrtVect Writer::insertSpecifications(DrtVect speech)
{
	vector<DrtVect> to_return;

	vector<string> already_done;
	for (int m = 0; m < speech.size(); ++m) {
		if (!speech.at(m).is_name())
			continue;
		if (speech.at(m).tag() == "PRP")
			continue;
		string ref = extract_first_tag(speech.at(m));
		if (shortfind(already_done, ref))
			continue;
		// try to see if the reference is in the data
		vector<DrtVect> new_specs = k_.getSpecificationsFromRef(ref);
		vector<DrtVect> specs = filter_valid_specifications(new_specs);


		for (int n = 0; n < specs.size(); ++n) {
			if (spec_is_already_present(speech, specs.at(n)))
				continue;
			if (specs.at(n).size() && extract_header(specs.at(n).at(0)) == "@QUANTITY")
				continue;
			if (specs.at(n).size() && extract_header(specs.at(n).at(0)) == "@AND")
				continue;
			if (specs.at(n).size() && extract_header(specs.at(n).at(0)) == "@OR")
				continue;

			DrtVect to_insert = specs.at(n);
			to_insert = process_missing_elements(to_insert, &k_);

			speech.insert(speech.begin() + m + 1, to_insert.begin(), to_insert.end());
			already_done.push_back(ref);
		}
	}
	// if the specification introduces a verb, create the new subordinate
	for (int m = 0; m < speech.size(); ++m) {
		string fref = extract_first_tag(speech.at(m));
		string sref = extract_second_tag(speech.at(m));
		string header = extract_header(speech.at(m));
		if (m < speech.size() - 1 && !speech.at(m).is_complement() && !ref_is_verb(fref) && ref_is_verb(sref)) {
			string next_verb_ref = extract_first_tag(speech.at(m + 1));
			if (next_verb_ref != sref) {
				string tag = "VB";
				DrtPred new_verb("do/"+tag+"(" + next_verb_ref + ",none,none" + ")");
				speech.insert(speech.begin() + m + 1, new_verb);
			}
		}
	}

	return speech;
}

DrtVect Writer::insertSpecificationsLocally(DrtVect speech, int pos)
{
	vector<DrtVect> to_return;

	vector<string> already_done;
	for (int m = 0; m < speech.size(); ++m) {
		if ( m != pos )
			continue;
		if (!speech.at(m).is_name())
			continue;
		if (speech.at(m).tag() == "PRP")
			continue;
		string ref = extract_first_tag(speech.at(m));
		if (shortfind(already_done, ref))
			continue;
		// try to see if the reference is in the data
		vector<DrtVect> new_specs = k_.getSpecificationsFromRef(ref);
		vector<DrtVect> specs = filter_valid_specifications(new_specs);


		for (int n = 0; n < specs.size(); ++n) {
			if (spec_is_already_present(speech, specs.at(n)))
				continue;
			if (specs.at(n).size() && extract_header(specs.at(n).at(0)) == "@QUANTITY")
				continue;
			if (specs.at(n).size() && extract_header(specs.at(n).at(0)) == "@AND")
				continue;
			if (specs.at(n).size() && extract_header(specs.at(n).at(0)) == "@OR")
				continue;

			DrtVect to_insert = specs.at(n);
			to_insert = process_missing_elements(to_insert, &k_);

			speech.insert(speech.begin() + m + 1, to_insert.begin(), to_insert.end());
			already_done.push_back(ref);
		}
	}
	// if the specification introduces a verb, create the new subordinate
	for (int m = 0; m < speech.size(); ++m) {
		string fref = extract_first_tag(speech.at(m));
		string sref = extract_second_tag(speech.at(m));
		if (m < speech.size() - 1 && !speech.at(m).is_complement() && !ref_is_verb(fref) && ref_is_verb(sref)) {
			string next_verb_ref = extract_first_tag(speech.at(m + 1));
			if (next_verb_ref != sref) {
				DrtPred new_verb("do/VB(" + next_verb_ref + ",none,none" + ")");
				speech.insert(speech.begin() + m + 1, new_verb);
			}
		}
	}

	return speech;
}


DrtVect Writer::insertAdverbs(DrtVect speech)
{
	for (int m = 0; m < speech.size(); ++m) {
		if (!speech.at(m).is_verb())
			continue;
		string ref = extract_first_tag(speech.at(m));
		vector<DrtPred> advs = k_.getAdverbsFromRef(ref);
		speech.insert(speech.begin() + m + 1, advs.begin(), advs.end());
	}
	return speech;
}

static DrtPred clean_references(DrtPred dpred)
// some predicates can be PRED(A|B) -> PRED(A)
{
	string ref = extract_first_tag(dpred);
	int pos = ref.find("|");
	if (pos != string::npos) {
		ref = ref.substr(0, pos);
		implant_first(dpred, ref);
	}
	return dpred;
}

static bool has_object(const DrtPred &pred)
{
	string obj = extract_object(pred);
	if (debug)
		cout << "OBJ::: " << obj << endl;
	if (obj.find("obj") == string::npos && obj.find("none") == string::npos && obj.find("subj") == string::npos // The passive verbs invert the tags
			)
		return true;
	return false;
}

static bool has_subject(const DrtPred &pred)
{
	string subj = extract_subject(pred);
	if (subj.find("obj") == string::npos && subj.find("none") == string::npos && subj.find("subj") == string::npos )
		return true;
	return false;
}

static void switch_subj_obj(DrtPred &pred)
{
	string fstr = extract_subject(pred);
	string sstr = extract_object(pred);
	string str_tmp;
	str_tmp = fstr;
	fstr = sstr;
	sstr = str_tmp;
	implant_subject(pred, fstr);
	implant_object(pred, sstr);
}

static DrtVect implement_passive_form(DrtVect speech)
{
	speech.push_back(DrtPred("dummy:DELETE(dummy0)") );
	for (int n = 0; n < speech.size(); ++n) {
		if(speech.at(n).is_verb() && !has_subject(speech.at(n)) && has_object(speech.at(n)) ) {
			string oref= extract_object(speech.at(n));
			DrtPred new_verb(speech.at(n));
			new_verb.setTag("VBN");
			switch_subj_obj(new_verb);
			add_header(speech.at(n),":DELETE");
			for (int m = 0; m < speech.size()-1; ++m) {
				string fref= extract_first_tag(speech.at(m));
				string next_ref= extract_first_tag(speech.at(m+1));
				if(m == speech.size() -2)
					next_ref = "";
				if(fref == oref && next_ref != oref) {
					DrtPred be_pred(new_verb);
					implant_header(be_pred,"be");
					implant_first(be_pred,"dummyverb");
					be_pred.setTag("VBP");
					speech.insert(speech.begin()+m+1,new_verb);
					speech.insert(speech.begin()+m+1,be_pred);
				}
			}
		}
	}
	return speech;
}

static DrtVect collapse_dummy_complements(DrtVect speech)
{
	vector<string> name_candidates; // names or RB that triggers an implicit @MOTION_TO
	name_candidates.push_back("always");
	name_candidates.push_back("never");
	name_candidates.push_back("somewhere");
	name_candidates.push_back("anywhere");
	name_candidates.push_back("everywhere");
	name_candidates.push_back("nowhere");
	name_candidates.push_back("sometime");
	name_candidates.push_back("sometimes");
	name_candidates.push_back("there");
	name_candidates.push_back("no-more");
	name_candidates.push_back("close");
	name_candidates.push_back("a_lot");
	for (int n = 1; n < speech.size(); ++n) {
		string header = extract_header(speech.at(n));
		string ref = extract_first_tag(speech.at(n));
		string prev_ref = extract_second_tag(speech.at(n - 1));
		if (shortfind(name_candidates, header) && speech.at(n - 1).is_complement() && ref == prev_ref) {
			add_header(speech.at(n - 1), ":DELETE");
			speech.at(n).setTag("RB");
		}
	}

	return speech;
}

static DrtVect assign_VBG(DrtVect speech)
{
	for (int n = 1; n < speech.size(); ++n) {
		string header = extract_header(speech.at(n));
		string fref = extract_first_tag(speech.at(n));
		string sref = extract_second_tag(speech.at(n));
		if( speech.at(n).is_complement() && header == "@GENITIVE" && ref_is_verb(sref) ) {
			int m= find_verb_with_string(speech,sref);
			if(m != -1) {
				speech.at(m).setTag("VBG");
			}
		}
		if( speech.at(n).is_complement() && header == "@TIME_AT" && ref_is_verb(sref) ) {
			int m= find_verb_with_string(speech,sref);
			if(m != -1 && !has_subject(speech.at(m)) ) {
				speech.at(m).setTag("VBG");
			}
		}
	}

	return speech;
}





string Writer::writeSingleSentence(DrtPred complement, vector<DrtPred> speech)
{
// list of complements that can introduce a subordinate.
// If there is another complement "to" is used instead.
	map<string, string> map_compl_to_str;
	map_compl_to_str["@PLACE_AT"] = "where";
	map_compl_to_str["@MOTION_TO"] = "into";
	map_compl_to_str["@TIME_AT"] = "when";
	map_compl_to_str["@TIME_FROM"] = "since";
	map_compl_to_str["@TIME_TO"] = "until";
	map_compl_to_str["@BEFORE"] = "before";
	map_compl_to_str["@AFTER"] = "after";
	map_compl_to_str["@CAUSED_BY"] = "because";
	map_compl_to_str["@ALLOCUTION"] = "that";
	map_compl_to_str["@EXCLUDING"] = "without";
	map_compl_to_str["@CONJUNCTION"] = "and";
	map_compl_to_str["@DISJUNCTION"] = "but";
	map_compl_to_str["@COORDINATION"] = "or";
	map_compl_to_str["@CONDITION"] = "if";
	map_compl_to_str["@HEAD"] = "";
	map_compl_to_str["@PAR"] = "and";
	map_compl_to_str["@SUBORD_DIFFERENT"] = "that";

	string str = "";
	string head = extract_header(complement);
	if (head.find("|")) {
		vector<string> strings;
		boost::split(strings, head, boost::is_any_of("|"));
		if (strings.size())
			head = strings.at(0);
		else
			head = "thing";
	}
	map<string, string>::iterator miter = map_compl_to_str.find(head);
	map<string, string> all_complements = get_all_complement_conversion();
	map<string, string>::iterator miter2 = all_complements.find(head);
	bool conjugate = true;
	if (miter != map_compl_to_str.end()) {
		str += miter->second + " ";
	} else if (miter2 != all_complements.end()) {
		str += miter2->second + " ";
	} else {
		str += "to ";
		conjugate = false;
		speech = eliminate_subject(speech);
	}

	writer_sort(speech);
	speech = implement_passive_form(speech);
	speech = insertJoinedNouns(speech);
	speech = insertSpecifications(speech);
	speech = insertAdverbs(speech);
	speech = eliminateRedundant(speech);
	speech = eliminateCompeting(speech);
	speech = addMissing(speech);
	speech = collapse_dummy_complements(speech);
	speech = eliminate_broken_complements(speech);
	speech = assign_VBG(speech);

//speech= order_subordinate_elements(speech);

	vector<DrtPred>::iterator diter = speech.begin();
	vector<DrtPred>::iterator dend = speech.end();

	Priors priors(priors_);

	clock_t start;
	int speech_pos = 0;
	for (; diter != dend && speech_pos < speech.size(); ++diter, ++speech_pos) {
		if (!is_valid_element(*diter))
			continue;
		DrtPred printed_pred;
		pair<string,DrtPred> sp_pair = getStringFromPred(clean_references(*diter), speech, conjugate, &priors, &printed_pred);
		str += sp_pair.first;
		str += " ";
		speech.at(speech_pos) = sp_pair.second;
		if ( printed_pred.is_name() ) {
			string fref1 = extract_first_tag(*diter);
			string fref2 = extract_first_tag(printed_pred);
			if (fref1 != fref2) {
				implant_first(*diter,fref2);
				speech = insertSpecificationsLocally(speech,speech_pos);
				speech = eliminateRedundant(speech);
				speech = eliminateCompeting(speech);
				speech = collapse_dummy_complements(speech);
				speech = eliminate_broken_complements(speech);
				speech = assign_VBG(speech);
				diter = speech.begin() + speech_pos;
				dend = speech.end();
			}
		}
	}


	return str;
}

vector<DrtPred> Writer::addMissing(vector<DrtPred> speech)
// @GENITIVE(A,B) @...(C) -> @GENITIVE(A,B) himself(B) @...(C)
{
	vector<DrtPred>::iterator diter = speech.begin();
	vector<DrtPred>::iterator dend = speech.end();

	vector<DrtPred> already_parsed;
	vector<string> parsed_verb_ref;
	vector<string> parsed_name_ref;
	for (; diter != dend; ++diter) {
		string header = extract_header(*diter);
		string sref = extract_second_tag(*diter);
		if (diter->is_complement() && boost::next(diter) != dend && header != "@TIME" && ref_is_name(sref)) {
			string nref = extract_first_tag(*boost::next(diter));
			if (sref != nref) {
				implant_header(*diter, header + ":DELETE");
			}
		}
	}

	return speech;
}

vector<DrtPred> Writer::eliminateCompeting(vector<DrtPred> speech)
// eliminate competing complements: @TIME_AT(A,B) @AFTER(A,B) -> @AFTER(A,B)
{
	vector<DrtPred> to_return(speech);

	vector<string> priority;
	priority.push_back("how");

	priority.push_back("HOWTO:DELETE");
	priority.push_back("@BEFORE");
	priority.push_back("@AFTER");
	priority.push_back("@TIME_AT|@CLOCK_AT|@TIME_FROM|@TIME_TO|@TOPIC");
	priority.push_back("@TIME_AT");
	priority.push_back("@PLACE_AT");
	priority.push_back("@MOTION_TO");
	priority.push_back("@GENITIVE");
	priority.push_back("@PAR");
	priority.push_back("@SUBORD");
	priority.push_back("@SUB-OBJ");
	priority.push_back("@DISJUNCTION");
	priority.push_back("@CONJUNCTION");
	priority.push_back("@COORDINATION");
	priority.push_back("@AND");
	priority.push_back("@OR");

	vector<pair<string, string> > tag_pairs;
	map<pair<string, string>, int> map_pairs;

	for (int n = 0; n < to_return.size(); ++n) {
		string head0 = extract_header(to_return.at(n));
		if (((to_return.at(n).is_complement() || head0 == "how" /// temporary: assign a complement to these two
		|| head0 == "why") && head0.find(":DELETE") == string::npos) || head0 == "HOWTO:DELETE") {
			string fref = extract_first_tag(to_return.at(n));
			string sref = extract_second_tag(to_return.at(n));
			pair<string, string> tagpair = make_pair(fref, sref);
			vector<pair<string, string> >::iterator prev_pair = find(tag_pairs.begin(), tag_pairs.end(), tagpair);
			map<pair<string, string>, int>::iterator map_iter = map_pairs.find(tagpair);

			if (prev_pair != tag_pairs.end() && map_iter != map_pairs.end()) {
				// If there already is a complement pointing to the same elements
				string head = extract_header(to_return.at(n));
				int pos_prior_int = map_pairs[tagpair];
				string head_prior = extract_header(to_return.at(pos_prior_int));

				vector<string>::iterator pos_now = find(priority.begin(), priority.end(), head);
				vector<string>::iterator pos_prior = find(priority.begin(), priority.end(), head_prior);

				if (std::distance(priority.begin(), pos_now) > std::distance(priority.begin(), pos_prior)) {
					// If the previous pair has a higher priority, then erase the current element
					add_header(to_return.at(n), ":DELETE");
					to_return.at(n).name() += ":DELETE";
				} else {
					map_pairs[tagpair] = n;
					add_header(to_return.at(pos_prior_int), ":DELETE");
					to_return.at(pos_prior_int).name() += ":DELETE";
				}
			} else {
				tag_pairs.push_back(tagpair);
				map_pairs[tagpair] = n;
			}
		}
	}
	return to_return;
}

static bool is_delete(const DrtPred &pred)
{
	string header = extract_header(pred);
	if(header.find(":DELETE") == string::npos)
		return false;
	return true;
}

vector<DrtPred> Writer::eliminateRedundant(vector<DrtPred> speech)
{
	vector<DrtPred>::iterator diter = speech.begin();
	vector<DrtPred>::iterator dend = speech.end();

	vector<DrtPred> already_parsed;
	vector<string> parsed_verb_ref;
	vector<string> parsed_name_ref;
	for (; diter != dend; ++diter) {
		string ref = extract_first_tag(*diter);
		if (shortfind(already_parsed, *diter)) {
			add_header(*diter, ":DELETE");
		} else
			already_parsed.push_back(*diter);
		if (diter->is_verb() && !is_delete(*diter) && ref_is_verb(ref)) {
			if (shortfind(parsed_verb_ref, ref)) {
				add_header(*diter, ":DELETE");
			} else {
				parsed_verb_ref.push_back(ref);
			}
		}
		if (diter->is_name()) { // only one name with the same reference
			if (shortfind(parsed_name_ref, ref)) {
				add_header(*diter, ":DELETE");
			} else {
				parsed_name_ref.push_back(ref);
			}
		}
	}

	vector<DrtPred> to_return;
	for (diter = speech.begin(); diter != dend; ++diter) {
		if (extract_header(*diter).find(":DELETE") == string::npos) {
			to_return.push_back(*diter);
		}
	}

	return to_return;
}

static string last_touch(string str)
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
	while (i < str.size()) {
// erase two continuous spaces
		if (str.at(i) == '_') {
			str.at(i) = ' ';
			continue;
		}
		++i;
	}

	i = 0;
	while (i < str.size()) {
// erase two continuous spaces
		if (str.at(i) == ' ' && i < str.size() - 1 && str.at(i + 1) == ' ') {
			str.erase(i + 1, 1);
			continue;
		}
		++i;
	}

// erase an eventual initial space
	if (str.size() && str.at(0) == ' ')
		str.erase(0, 1);

	return str;
}

static DrtVect sign_subordinates_with_different_subj(DrtVect speech)
{

	for (int n = 0; n < speech.size(); ++n) {
		string header = extract_header(speech.at(n));
		if (speech.at(n).is_complement() && header == "@SUBORD") {
			string fref = extract_first_tag(speech.at(n));
			string sref = extract_second_tag(speech.at(n));
			int m1 = find_verb_with_string(speech, fref);
			int m2 = find_verb_with_string(speech, sref);
			if (m1 == -1 || m2 == -1)
				continue;
			string obj_ref1 = extract_object(speech.at(m1));
			string subj_ref2 = extract_subject(speech.at(m2));
			if (obj_ref1 != subj_ref2)
				implant_header(speech.at(n), "@SUBORD_DIFFERENT");
		}
	}
	return speech;
}

string Writer::write(const vector<DrtPred> &drs, Priors priors)
{
	priors_ = priors;
	string ret_str = "";

	vector<DrtPred> speech(drs);

	writer_sort(speech);

// Includes missing predicates (like subjects or objects)
	vector<string> missing_refs = get_missing_references(speech);
	vector<DrtPred> missing_preds = create_preds_with_refs(missing_refs, &k_);
	speech.insert(speech.end(), missing_preds.begin(), missing_preds.end());
	missing_refs = get_missing_references(missing_preds);
	missing_preds = create_preds_with_refs(missing_refs, &k_);
	speech.insert(speech.end(), missing_preds.begin(), missing_preds.end());


// Includes missing complements
	vector<vector<DrtPred> > complements_preds = get_missing_complement_preds(speech);
	for (int n = 0; n < complements_preds.size(); ++n) {
		speech.insert(speech.end(), complements_preds.at(n).begin(), complements_preds.at(n).end());
	}

	vector<DrtVect> subs = this->get_subordinates(speech);
//puts("SUBORDINATES:::");
	for (int n = 0; n < subs.size(); ++n) {
//print_vector(subs.at(n));
		speech.insert(speech.end(), subs.at(n).begin(), subs.at(n).end());
	}

	speech = sign_subordinates_with_different_subj(speech);
	speech = assign_VBG(speech);
	SentenceTree st = this->getSentenceTree(speech);

	SentenceTree::iterator siter = st.begin();
	SentenceTree::iterator send = st.end();

	ret_str += this->writeSingleSentence(siter->first, siter->second);
	++siter;
	for (; siter != send; ++siter) {
		ret_str += this->writeSingleSentence(siter->first, siter->second);
	}

	ret_str = last_touch(ret_str);

	return ret_str;
}

string Writer::trivialSpeech(const DrtPred &d)
{
	string speech = "something";

	string header = extract_header(d);
	if (header.find("what") != string::npos) {
		speech = "something";
	}

	return speech;
}

static bool is_quantifier(const string &str)
{
	if (str.size() && str.at(0) == '_')
		return true;
	return false;
}

static bool is_trivial_speech(const DrtPred &d)
{
	string fref = extract_first_tag(d);
	string header = extract_header(d);
	if (is_quantifier(fref) && header.find(":Q") == string::npos) {
		return true;
	}

	return false;
}

static DrtPred create_proper_predicate(const DrtPred &pred)
{
	DrtPred to_return;

	if(pred.is_WRB()) {
		string sref   = extract_second_tag(pred);
		if(!ref_is_verb(sref) ) {
			to_return = DrtPred( (string) "something/NN(" + sref+ ")" );
		} else
			to_return = pred;
	} else {
		to_return = pred;
	}

	if(extract_first_tag(to_return).find("_prev") != string::npos) {
		implant_first(to_return,"_name_dummy");
	}

	return to_return;
}

string Writer::write(const DrtPred &d, Priors priors)
{
	if (is_trivial_speech(d)) {
		return this->trivialSpeech(d);
	}
	priors_ = priors;
	vector<DrtPred> drtvect;
	drtvect.push_back( create_proper_predicate(d) );
	return this->write(drtvect, priors_);
}

string Writer::write(const clause_vector &clause)
{
	string ret_str = "";

	vector<DrtPred> hyp = clause.getHypothesis();
	vector<DrtPred> cons = clause.getConsequence();

	ret_str += this->write(cons);
	ret_str += " <-- ";
	ret_str += this->write(hyp);

	return ret_str;
}

static DrtVect negate_drtvect(DrtVect drtvect)
{
	for (int n = 0; n < drtvect.size(); ++n) {
		if (drtvect.at(n).is_verb()) {
			string ref = extract_first_tag(drtvect.at(n));
			DrtPred pred(string("not/RB(" + ref + ")"));
			drtvect.push_back(pred);
			break;
		}
	}
	return drtvect;
}

string Writer::write(const vector<KnowledgeAnswer> &kav, drt &dquest)
{
	string comment;

// Check if it is a yes/no question
	bool yn_question;
	vector<DrtPred> qlist = dquest.getQuestionList().get();
	if (qlist.size() == 0)
		yn_question = true;
	else
		yn_question = false;

	DrtVect question = dquest.predicates();
	DrtVect negated_question = negate_drtvect(question);

	DrtVect yes_drs = create_drtvect("yes/NNP(name-2)");
	DrtVect no_drs = create_drtvect("no/NNP(name-3)");
	DrtVect dunno_drs = create_drtvect("nlulite/NNP(name_-4),know/V(verb_-4,name_-4,none),not/RB(verb_-4)");
	DrtVect list_answer_drs = create_drtvect("answer/NN(ref_-5),be/V(verb_-5,ref_-5,name_-6),list/NN(name_-6)");

	CodePred answer("join(set(kav,_KAV),"
			"     set(yes,_YES_DRS),"
			"     set(no,_NO_DRS),"
			"     set(dunno,_DUNNO_DRS),"
			"     set(list-answer,_LIST_ANSWER_DRS),"
			"     set(yn-question,_YN_QUESTION),"
			"     set(question,_QUESTION),"
			"     set(negated-question,_NEGATED_QUESTION),"
			"     if-else(yn-question,"
			"             if-else(greater-than(num-children(kav),0),"
			"                     set(answer,yes),"
			"                     join(set(negated-kav,ask(negated-question)),"
			"                          if-else(greater-than(num-children(negated-kav),0),"
			"                                  set(answer,no),"
			"                                  set(answer,dunno)"
			"                                 )"
			"                         )"
			"                    ),"
			"             if-else(greater-than(num-children(kav),0),"
			"                     set(answer,list-answer),"
			"                     set(answer,dunno)"
			"                    )"
			"            )"
			"    )");

	answer.insert("_KAV", kav);
	answer.insert("_QUESTION", question);
	answer.insert("_NEGATED_QUESTION", negated_question);
	answer.insert("_YES_DRS", yes_drs);
	answer.insert("_NO_DRS", no_drs);
	answer.insert("_DUNNO_DRS", dunno_drs);
	answer.insert("_LIST_ANSWER_DRS", list_answer_drs);
	answer.insert("_YN_QUESTION", yn_question);

	Engine engine(&k_);
	CodePred result = engine.run(answer);
	DrtVect answer_drs = engine.getList<DrtPred>("answer");
	comment = this->write(answer_drs);

	return comment;
}
