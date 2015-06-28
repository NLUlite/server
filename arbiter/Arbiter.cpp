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



#include"Arbiter.hpp"

const bool debug = false;
const bool measure_time = false;
const bool commercial_version = true;
boost::mutex io_mutex_wcounter;

const int standard_max_level = 5;

//const int standard_max_level = 0;

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

template<class T>
static bool shortfind(const vector<T> &vect, const T &element)
{
	if (find(vect.begin(), vect.end(), element) == vect.end())
		return false;
	return true;
}

static inline int max(int a, int b)
{
	return a < b ? b : a;
}
static inline int min(int a, int b)
{
	return a >= b ? b : a;
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


vector<KnowledgeAnswer> knowledge_unique(const vector<KnowledgeAnswer> &preds)
{
	vector<KnowledgeAnswer> to_return;

	for (int n = 0; n < preds.size(); ++n) {
		if (preds.at(n).getWeigth() < 0.01) {
			to_return.push_back( preds.at(n) );
		}
	}
	return to_return;
}

static bool compare_kanswers(const KnowledgeAnswer &lhs, const KnowledgeAnswer &rhs)
{
	return lhs.getWeigth() > rhs.getWeigth();
}

static string create_list_comment(Writer &writer, vector<KnowledgeAnswer> &kav)
{
	string to_return;

	sort(kav.begin(), kav.end(), compare_kanswers);

	vector<string> sentences;

	const int max_comment_length = 5;

	for (int n = 0; n < kav.size() && n < max_comment_length; ++n) {
		if (kav.at(n).getWeigth() > 0.01) {
			vector<DrtPred> tmp_drt = kav.at(n).getPreds();
			string sentence;
			if(kav.at(n).getText() == "" || kav.at(n).getText() == "From Wikidata")
				sentence =  writer.write( tmp_drt );
			else
				sentence = kav.at(n).getText();

			sentences.push_back( sentence );
		}
	}

	sort(sentences.begin(), sentences.end());
	sentences.erase(std::unique(sentences.begin(), sentences.end()), sentences.end());

	for(int n=0; n < sentences.size(); ++n) {
		string tmp_str = sentences.at(n);
		if(tmp_str.size() && tmp_str.at(tmp_str.size()-1) == ' ')
			tmp_str.resize(tmp_str.size()-1);
		if(tmp_str.size() && tmp_str.at(0) != ' ')
			tmp_str.at(0) = std::toupper(tmp_str.at(0));
		else if(tmp_str.size() > 1 && tmp_str.at(1) != ' ')
			tmp_str.at(1) = std::toupper(tmp_str.at(1));
		to_return += tmp_str;
		if(tmp_str.at(tmp_str.size()-1) != '.'
		   && tmp_str.at(tmp_str.size()-1) != ';'
		   && tmp_str.at(tmp_str.size()-1) != ':'
		   && tmp_str.at(tmp_str.size()-1) != '!'
		   && tmp_str.at(tmp_str.size()-1) != '?'
		)
			to_return += ". ";
	}

	return to_return;
}


ArbiterItem Arbiter::processComment(vector<KnowledgeAnswer> kav, vector<drt> &dquest_vect, StepInfo ainfo)
{
//	kav = knowledge_unique(kav); // Do not put! It makes the comment string unpredictable

	// Check if it is a yes/no question
	bool yn_question = true;
	vector<DrtPred> qlist;
	if (dquest_vect.size() == 0)
		return ArbiterItem();

	for(int n=0; n < dquest_vect.size(); ++n) {
		drt dquest = dquest_vect.at(n);
		qlist = dquest.getQuestionList().get();
		if (qlist.size() != 0) {
			yn_question = false;
			break;
		}
	}


	drt dquest = dquest_vect.at(0);
	DrtVect question = dquest.predicates();
	DrtVect negated_question = negate_drtvect(question);

	DrtVect yes_drs = create_drtvect("yes/NNP(name-2)");
	DrtVect no_drs = create_drtvect("no/NNP(name-3)");
	DrtVect dunno_drs = create_drtvect("answer/NN(ref_-5),not/RB(verb_-4),find/V(verb_-4,none,ref_-5),@PLACE_AT(verb_-4,ref_-4),data/NN(ref_-4)");
	DrtVect list_answer_drs = create_drtvect("answer/NN(ref_-5),be/V(verb_-5,ref_-5,name_-6),list/NN(name_-6)");
	DrtVect approx_answer_drs = create_drtvect("answer/NN(ref_-5),be/V(verb_-8,ref_-5,name_-8),approximate/JJ(name_-8)");

	CodePred answer("join(set(kav,_KAV),"
			"     set(yes,_YES_DRS),"
			"     set(no,_NO_DRS),"
			"     set(dunno,_DUNNO_DRS),"
			"     set(list-answer,_LIST_ANSWER_DRS),"
			"     set(yn-question,_YN_QUESTION),"
			"     set(question,_QUESTION),"
			"     set(negated-question,_NEGATED_QUESTION),"
			"     set(approx-answer-drs,_APPROX_ANSWER_DRS),"
			"     set(approx-answer,_APPROX_ANSWER),"
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
			"                )"
			"            ),"
			"     if(approx-answer, set(answer,approx-answer-drs)),"
			"     print(neg-kav),"
			"     print(negated-kav)"
			"    )");

	answer.insert("_KAV", kav);
	answer.insert("_QUESTION", question);
	answer.insert("_NEGATED_QUESTION", negated_question);

	clock_t start;

	answer.insert("_YES_DRS", yes_drs);
	answer.insert("_NO_DRS", no_drs);
	answer.insert("_DUNNO_DRS", dunno_drs);
	answer.insert("_LIST_ANSWER_DRS", list_answer_drs);
	answer.insert("_YN_QUESTION", yn_question);
	answer.insert("_APPROX_ANSWER", ainfo.approximate_answer);
	answer.insert("_APPROX_ANSWER_DRS", list_answer_drs);

	Engine engine(k_);
	CodePred result = engine.run(answer);


	DrtVect comment_drt = engine.getList<DrtPred>("answer");
	vector<KnowledgeAnswer> new_kav;
	try {
		new_kav = engine.getList<KnowledgeAnswer>("negated-kav");
	} catch (std::exception &e) {
		///
	}

	// Fill the answer structure
	ArbiterItem ri;
	ri.setKav(new_kav);
	Writer writer(*k_);
	string comment = writer.write(comment_drt);
	string status = "list";
	if (comment.find("Yes") != string::npos)
		status = "yes";
	if (comment.find("No") != string::npos)
		status = "no";
	if (comment.find("not") != string::npos)
		status = "unknown";

	if(status == "list") {
		comment = create_list_comment(writer, kav);
	}
	ri.setComment(comment);

	ri.setStatus(status);
	return ri;
}

static bool compare_answers(const Answer &lhs, const Answer &rhs)
{
	return lhs.getWeigth() > rhs.getWeigth();
}
static vector<Answer>::iterator wisdom_find(vector<Answer>::iterator iter, vector<Answer>::iterator last, const Answer& val)
{
	while (iter != last) {
		string lstr = iter->getDrtAnswer().getText();
		string rstr = val.getDrtAnswer().getText();
		vector<pair<string,string> > lpairs= iter->getWPAnswer();
		vector<pair<string,string> > rpairs= val.getWPAnswer();
		if (lstr == rstr && lpairs == rpairs )
			return iter;

		++iter;
	}
	return last;
}

static vector<Answer>::iterator wisdom_find_not_empty(vector<Answer>::iterator iter, vector<Answer>::iterator last, const Answer& val)
{
	while (iter != last) {
		string lstr = iter->getDrtAnswer().getText();
		string rstr = val.getDrtAnswer().getText();
		vector<pair<string,string> > lpairs= iter->getWPAnswer();
		vector<pair<string,string> > rpairs= val.getWPAnswer();
		if (lstr == rstr && lpairs.size() != 0 )
			return iter;
		++iter;
	}
	return last;
}


void pre_wisdom_unique(vector<Answer> &preds)
{
	vector<Answer> already_present;

	for (int n = 0; n < preds.size(); ++n) {
		if (preds.at(n).getWeigth() < 0.01) {
			preds.erase(preds.begin() + n);
			--n;
		}
	}
}


void wisdom_unique(vector<Answer> &preds)
{
	vector<Answer> originals(preds);

	vector<Answer> already_present;

	for (int n = 0; n < preds.size(); ++n) {
		if (wisdom_find(already_present.begin(), already_present.end(), preds.at(n)) == already_present.end()) {
			already_present.push_back(preds.at(n));
		} else {
			preds.erase(preds.begin() + n);
			--n;
		}
	}

	for (int n = 0; n < preds.size(); ++n) {
		if (preds.at(n).getWeigth() < 0.01) {
			preds.erase(preds.begin() + n);
			--n;
		}
	}

	// erase empty answers if there are answers with preds
	for (int n = 0; n < preds.size(); ++n) {
		vector<pair<string,string> > answers = preds.at(n).getWPAnswer();
		if(answers.size() == 0) {
			if (wisdom_find_not_empty(preds.begin(), preds.end(), preds.at(n)) != preds.end()) {
				preds.erase(preds.begin() + n);
				--n;
			};
		}
	}
	if(preds.size() == 0)
		preds = originals;
}


class WThreadCounter {
	int num_, max_num_;

public:
	WThreadCounter(int max_num) :
			max_num_(max_num), num_(0)
	{
	}
	int getCurrentNumber();
};

int WThreadCounter::getCurrentNumber()
{
	boost::mutex::scoped_lock lock(io_mutex_wcounter); // this method is used concurrently
	if (num_ < max_num_) {
		++num_;
		return num_ - 1;
	}
	return -1;
}


class StringWThread {

	Writer *writer_;
	vector< vector<Answer> > *results_;
	DrtVect question_;
	vector<AnswerContainer> *final_answers_;
	WThreadCounter *counter_;

	void launchStringThread(vector<Answer> *results, AnswerContainer final_answer);

public:
	StringWThread(vector< vector<Answer> > *results, Writer *writer, vector<AnswerContainer> *final_answers, WThreadCounter *counter) :
		results_(results), writer_(writer), final_answers_(final_answers), counter_(counter)
	{
	}

	void operator()();
};

void StringWThread::operator()()
{
	int num = 0;
	while (num < final_answers_->size() && num < results_->size()) {
		num = counter_->getCurrentNumber();
		if (num == -1)
			break;
		vector<Answer> *out = &results_->at(num);
		AnswerContainer in = final_answers_->at(num);
		this->launchStringThread(out, in);
	}
}

void StringWThread::launchStringThread(vector<Answer> *results, AnswerContainer final_answer)
{
	vector<KnowledgeAnswer> contained_answers = final_answer.getAnswers();
	for (int n = 0; n < contained_answers.size(); ++n) {
		vector<DrtPred> preds = contained_answers.at(n).getPreds();
		vector<DrtPred> qlist = contained_answers.at(n).getQuestionList().get();
		Priors priors = contained_answers.at(n).getQuestionList().getPriors();

		vector<pair<string, string> > str_answers;
		for (int m = 0; m < qlist.size(); ++m) {
			string qstr = writer_->write(qlist.at(m), priors);
			string str_answer = extract_header(qlist.at(m));
			int pos = str_answer.find(":");
			if(pos != string::npos)
				str_answer = str_answer.substr(0,pos);
			if(str_answer == "[what]")
				str_answer = "what";
			str_answers.push_back(make_pair(str_answer, qstr));
		}

		// Put the answers in the return structure
		string link = contained_answers.at(n).getLink();
		Answer tmp_answer;
		tmp_answer.setWPAnswer(str_answers);
		tmp_answer.setDrsAnswer(preds);
		drt dummy_drt(preds);
		dummy_drt.setLink(link);
		string text = contained_answers.at(n).getText();
		dummy_drt.setText(text);
		tmp_answer.setDrtAnswer(dummy_drt);
		path_memory mem = contained_answers.at(n).getMemory();
		tmp_answer.setClauseHistory(mem.get_clause_history());
		tmp_answer.setClauseAnswer(mem.getDrt());
		tmp_answer.setWeigth(contained_answers.at(n).getWeigth());
		results->push_back(tmp_answer);
	}
}



static vector<Answer> get_wisdom_answers_from_answer_container(vector<AnswerContainer> &final_answers, Knowledge *k, int max_size)
{
	clock_t start;

	Writer writer(*k);	
	int max_threads = 1;
	Parameters *par = parameters_singleton::instance();
	if(commercial_version)
		max_threads = par->getNumThreads();
	int num_results= final_answers.size();
	num_results = min(num_results,max_size);
	vector< vector<Answer> > results(num_results);
	WThreadCounter counter(num_results);
	int num_threads = min(num_results, max_threads);
	vector<StringWThread> pt_vect(num_threads, StringWThread(&results, &writer, &final_answers, &counter) );

	boost::thread_group g;
	for (int t = 0; t < num_threads; ++t) {
		g.create_thread( pt_vect.at(t) );
	}
	g.join_all();

	vector<Answer> wisdom_answers;
	for(int n = 0; n < results.size(); ++n) {
		vector<Answer> answers_tmp= results.at(n);
		wisdom_answers.insert(wisdom_answers.end(), answers_tmp.begin(), answers_tmp.end() );
	}

	return wisdom_answers;
}

static vector<KnowledgeAnswer> get_knowledge_answers_from_answer_container(const vector<AnswerContainer> &final_answers, int max_size)
{
	vector<KnowledgeAnswer> kanswers;
	for (int k = 0; k < final_answers.size() && k < max_size; ++k) {
		vector<KnowledgeAnswer> contained_answers = final_answers.at(k).getAnswers();
		for(int n=0; n < contained_answers.size(); ++n) {
			if(contained_answers.at(n).getWeigth() > 0.01) {
				kanswers.push_back(contained_answers.at(n));
			}
		}
	}
	return kanswers;
}

static inline string get_first_verb_ref(const vector<DrtPred> &drtvect)
{
	string vref;
	for (int n = 0; n < drtvect.size(); ++n) {
		if (drtvect.at(n).is_verb()) {
			vref = extract_first_tag(drtvect.at(n));
			break;
		}
	}
	return vref;
}

static vector<boost::tuple<string, int, int> > get_conj_relations(const vector<drt> &questions, const vector<DrtPred> &conjs)
{
	vector<boost::tuple<string, int, int> > to_return;

	vector<string> refs;

	for (int n = 0; n < questions.size(); ++n) {
		vector<DrtPred> drtvect = questions.at(n).predicates_with_references();
		string vref = get_first_verb_ref(drtvect);
		refs.push_back(vref);
	}

	for (int n = 0; n < conjs.size(); ++n) {
		string fref = extract_first_tag(conjs.at(n));
		string sref = extract_second_tag(conjs.at(n));

		vector<string>::iterator fiter = find(refs.begin(), refs.end(), fref);
		vector<string>::iterator siter = find(refs.begin(), refs.end(), sref);

		if (fiter != refs.end() && siter != refs.end()) {
			int fpos = std::distance(refs.begin(), fiter);
			int spos = std::distance(refs.begin(), siter);

			string head = extract_header(conjs.at(n));

			to_return.push_back(boost::make_tuple(head, fpos, spos));
		}
	}

	return to_return;
}

static string get_first_subject_ref(DrtVect &drtvect)
{
	string to_return = "";

	for (int n = 0; n < drtvect.size(); ++n) {
		if (drtvect.at(n).is_verb()) {
			to_return = extract_subject(drtvect.at(n));
			break;
		}
	}
	return to_return;
}

static string get_first_object_ref(DrtVect &drtvect)
{
	string to_return = "";

	for (int n = 0; n < drtvect.size(); ++n) {
		if (drtvect.at(n).is_verb()) {
			to_return = extract_object(drtvect.at(n));
			break;
		}
	}
	return to_return;
}

static bool question_intersection(const QuestionList &lhs, const QuestionList &rhs, Knowledge *k)
// intersects the two question looking at the header of the predicates
{
	vector<DrtPred> lpreds = lhs.get();
	vector<DrtPred> rpreds = rhs.get();

	for(int lnum= 0; lnum < lpreds.size(); ++lnum) {
		for(int rnum= 0; rnum < rpreds.size(); ++rnum) {
			if (lpreds.at(lnum) == rpreds.at(rnum)) {
				continue;
			}

			// if the two preds are different, then try to see if they correspond to the same names
			string lfref = extract_first_tag(lpreds.at(lnum));
			string rfref = extract_first_tag(rpreds.at(rnum));

			vector<DrtPred> lpreds= k->getPredsFromRef(lfref);
			vector<DrtPred> rpreds= k->getPredsFromRef(rfref);
			bool have_common_name = false;
			for(int ri=0; ri < rpreds.size(); ++ri) {
				string rstring= extract_header(rpreds.at(ri));
				if(rpreds.at(ri).is_PRP()
					|| !rpreds.at(ri).is_pivot()
				)
					continue;
				string rfref2 = extract_first_tag(rpreds.at(ri));
				if(rfref2 != rfref)
					continue;

				for(int li=0; li < lpreds.size(); ++li) {
					string lfref2 = extract_first_tag(lpreds.at(li));
					if(lfref2 != lfref)
						continue;

					string lstring= extract_header(lpreds.at(li));
					if(lpreds.at(li).is_PRP()
					   || !rpreds.at(ri).is_pivot()
					)
						continue;
					if (lstring == rstring ) {
						have_common_name = true;
						break;
					}
				}
				if (have_common_name)
					break;
			}

			//// have common reference

			if( !have_common_name)
				return false; // the two predicates have nothing in common, return false
		}
	}

	return true;
}

static bool word_intersection(const DrtVect &lpreds, const DrtVect &rpreds, Knowledge *k)
// intersects the two question looking at the names associated to the predicates
{
	for(int lnum= 0; lnum < lpreds.size(); ++lnum) {
		for(int rnum= 0; rnum < rpreds.size(); ++rnum) {
			if(!lpreds.at(lnum).is_intersection() && !rpreds.at(rnum).is_intersection() )
				continue;

			if (lpreds.at(lnum) == rpreds.at(rnum)) {
				return true;
			}
			// if the two preds are different, then try to see if they correspond to the same names
			string lfref = extract_first_tag(lpreds.at(lnum));
			string rfref = extract_first_tag(rpreds.at(rnum));

			vector<string> lstrings= k->getNamesFromRef(lfref);
			vector<string> rstrings= k->getNamesFromRef(rfref);
			bool have_common_name = false;
			for(int ri=0; ri < rstrings.size(); ++ri) {
				if (shortfind(lstrings,rstrings.at(ri))) {
					have_common_name = true;
					break;
				}
			}

			//// have common reference

			if( have_common_name)
				return true; // the two predicates have nothing in common, return false
		}
	}

	return false;
}


static vector<AnswerContainer> get_answer_intersection(const vector<AnswerContainer> &lhs, const vector<KnowledgeAnswer> &rhs, Knowledge *k, WisdomInfo *wi)
{
	vector<AnswerContainer> to_return;

	// exact intersection with references
	for (int li = 0; li < lhs.size(); ++li) {
		for (int ri = 0; ri < rhs.size(); ++ri) {
			DrtVect dl = lhs.at(li).at(0).getPreds();
			DrtVect dr = rhs.at(ri).getPreds();

			string lsubj_ref = get_first_subject_ref(dl);
			string rsubj_ref = get_first_subject_ref(dr);

			string lobj_ref = get_first_object_ref(dl);
			string robj_ref = get_first_object_ref(dr);

			vector<DrtPred> ql = lhs.at(li).at(0).getQuestionList().get();
			vector<DrtPred> ql2 = rhs.at(ri).getQuestionList().get();
			if ( lhs.at(li).at(0).getQuestionList().get() == rhs.at(ri).getQuestionList().get()
				) {
				AnswerContainer tmp_answer(lhs.at(li));
				tmp_answer.add(rhs.at(ri));
				to_return.push_back(tmp_answer);
			}
		}
	}


	if (to_return.size())
		return to_return;

	// sloppy intersection with_words
	if(wi->getWordIntersection()) {
		for (int li = 0; li < lhs.size(); ++li) {
			for (int ri = 0; ri < rhs.size(); ++ri) {
				DrtVect dl = lhs.at(li).at(0).getPreds();
				DrtVect dr = rhs.at(ri).getPreds();

				string lsubj_ref = get_first_subject_ref(dl);
				string rsubj_ref = get_first_subject_ref(dr);

				string lobj_ref = get_first_object_ref(dl);
				string robj_ref = get_first_object_ref(dr);

				vector<DrtPred> ql = lhs.at(li).at(0).getQuestionList().get();
				vector<DrtPred> ql2 = rhs.at(ri).getQuestionList().get();
				if ( question_intersection(lhs.at(li).at(0).getQuestionList(), rhs.at(ri).getQuestionList(), k )
						|| word_intersection(dl, dr, k )
						|| lsubj_ref == rsubj_ref
						|| lobj_ref == robj_ref
						|| lobj_ref == rsubj_ref
						|| robj_ref == lsubj_ref) {
					AnswerContainer tmp_answer(lhs.at(li));
					tmp_answer.add(rhs.at(ri));
					to_return.push_back(tmp_answer);
				}
			}
		}
	} else {
		for (int li = 0; li < lhs.size(); ++li) {
			for (int ri = 0; ri < rhs.size(); ++ri) {
				DrtVect dl = lhs.at(li).at(0).getPreds();
				DrtVect dr = rhs.at(ri).getPreds();

				string lsubj_ref = get_first_subject_ref(dl);
				string rsubj_ref = get_first_subject_ref(dr);

				string lobj_ref = get_first_object_ref(dl);
				string robj_ref = get_first_object_ref(dr);

				vector<DrtPred> ql = lhs.at(li).at(0).getQuestionList().get();
				vector<DrtPred> ql2 = rhs.at(ri).getQuestionList().get();
				if ( lsubj_ref == rsubj_ref
						|| lobj_ref == robj_ref
						|| lobj_ref == rsubj_ref
						|| robj_ref == lsubj_ref) {
					AnswerContainer tmp_answer(lhs.at(li));
					tmp_answer.add(rhs.at(ri));
					to_return.push_back(tmp_answer);
				}
			}
		}
	}

	return to_return;
}

static vector<Answer> transform_kav_into_wisdom_answers(vector<KnowledgeAnswer> &kav, Knowledge *k)
{
	vector<Answer> wisdom_answers;
	Writer writer(*k);
	for (int n = 0; n < kav.size(); ++n) {
		double w= kav.at(n).getWeigth();
		vector<DrtPred> preds = kav.at(n).getPreds();
		vector<DrtPred> qlist = kav.at(n).getQuestionList().get();
		Priors priors = kav.at(n).getQuestionList().getPriors();
		vector<pair<string, string> > str_answers;

		clock_t start;
		for (int m = 0; m < qlist.size(); ++m) {
			string qstr = writer.write(qlist.at(m), priors );
			string str_answer = extract_header(qlist.at(m));
			int pos = str_answer.find(":");
			if(pos != string::npos)
				str_answer = str_answer.substr(0,pos);
			if(str_answer == "[what]")
				str_answer = "what";

			str_answers.push_back(make_pair(str_answer, qstr));
		}

		// Put the answers in the return structure
		string link = kav.at(n).getLink();
		Answer tmp_answer;
		tmp_answer.setWPAnswer(str_answers);
		tmp_answer.setDrsAnswer(preds);
		drt dummy_drt(preds);
		dummy_drt.setLink(link);
		string text = kav.at(n).getText();
		dummy_drt.setText(text);
		tmp_answer.setDrtAnswer(dummy_drt);
		path_memory mem = kav.at(n).getMemory();
		tmp_answer.setClauseHistory(mem.get_clause_history());
		tmp_answer.setClauseAnswer(mem.getDrt());
		//cout << "CH:::" << mem.get_clause_history().size() << endl;
		tmp_answer.setWeigth(kav.at(n).getWeigth());
		wisdom_answers.push_back(tmp_answer);
	}

	return wisdom_answers;
}

static pair<DrtVect, DrtMgu> break_off_specifications(DrtVect drtvect)
{
	DrtMgu mgu;
	for (int n = 0; n < drtvect.size(); ++n) {
		string fref   = extract_first_tag(drtvect.at(n));
		string header = extract_header   (drtvect.at(n));
		if (drtvect.at(n).is_complement() && ref_is_name(fref)
			&& header.find("@QUANTITY") == string::npos // these are properties of the predicate
			&& header.find("@GENITIVE") == string::npos
		) {
			string old_ref = fref;
			string new_ref = string("_[broken_off]_") + boost::lexical_cast<string>(n) + "_" + fref;
			implant_first(drtvect.at(n), new_ref);
			mgu.add(old_ref, new_ref);
		}
	}
	return make_pair(drtvect, mgu);
}

static pair<DrtVect, DrtMgu> break_off_complements(DrtVect drtvect)
{
	DrtMgu mgu;
	for (int n = 0; n < drtvect.size(); ++n) {
		string fref   = extract_first_tag(drtvect.at(n));
		string header = extract_header   (drtvect.at(n));
		if (drtvect.at(n).is_complement() && ref_is_verb(fref)) {
			string old_ref = fref;
			string new_ref = string("_[broken_off]_") + boost::lexical_cast<string>(n) + "_" + fref;
			implant_first(drtvect.at(n), new_ref);
			if( header.find("@SIZE") != string::npos) {
				DrtPred new_pred("meter|foot|m|ft|cm|inch|mm|km|yard/NN#[pivot](" + new_ref + ")");
				drtvect.insert(drtvect.begin()+n,new_pred);
				++n;
			}
			mgu.add(old_ref, new_ref);
		}
	}
	return make_pair(drtvect, mgu);
}

static pair<DrtVect, DrtMgu> break_off_objects(DrtVect drtvect)
{
	DrtMgu mgu;
	for (int n = 0; n < drtvect.size(); ++n) {
		if (drtvect.at(n).is_verb()) {
			string fref = extract_object(drtvect.at(n));
			string old_ref = fref;
			string new_ref = string("_[broken_off]_") + boost::lexical_cast<string>(n) + "_" + fref;
			implant_object(drtvect.at(n), new_ref);
			mgu.add(old_ref, new_ref);
		}
	}
	return make_pair(drtvect, mgu);
}

static pair<DrtVect, DrtMgu> break_off_subjects(DrtVect drtvect)
{
	DrtMgu mgu;
	for (int n = 0; n < drtvect.size(); ++n) {
		if (drtvect.at(n).is_verb()) {
			string fref = extract_subject(drtvect.at(n));
			string old_ref = fref;
			string new_ref = string("_[broken_off]_") + boost::lexical_cast<string>(n) + "_" + fref;
			implant_subject(drtvect.at(n), new_ref);
			mgu.add(old_ref, new_ref);
		}
	}
	return make_pair(drtvect, mgu);
}

static pair<DrtVect, DrtMgu> break_off_everything(DrtVect drtvect)
{
	DrtMgu mgu;
	for (int n = 0; n < drtvect.size(); ++n) {
		string fref = extract_first_tag(drtvect.at(n));
		string old_ref = fref;
		string new_ref = string("_[broken_off]_") + boost::lexical_cast<string>(n) + "_" + fref;
		implant_first(drtvect.at(n), new_ref);
		mgu.add(old_ref, new_ref);
	}
	return make_pair(drtvect, mgu);
}

static pair<drt, DrtMgu> relax_conditions_in_question_at_level(drt question, DrtMgu mgu, int level)
{
	DrtVect drtvect(question.predicates_with_references());
	pair<DrtVect, DrtMgu> answer_pair;
	// drtvect / mgu;
	switch (level) {
	case 0:
		answer_pair = make_pair(drtvect, mgu);
		break;
	case 1:
		answer_pair = break_off_specifications(drtvect);
		break;
	case 2:
		answer_pair = break_off_complements(drtvect);
		break;
	case 3:
		answer_pair = break_off_objects(drtvect);
		break;
	case 4:
		answer_pair = break_off_subjects(drtvect);
		break;
	case 5:
		answer_pair = break_off_everything(drtvect);
		break;
	default:
		break;
	}

	question.setPredicates(answer_pair.first);
	mgu.add(answer_pair.second);
	return make_pair(question, mgu);
}

static drt clean_cause(drt question)
{
	DrtVect to_return = question.predicates_with_references();

	for (int n = 0; n < to_return.size(); ++n) {
		string fref = extract_first_tag(to_return.at(n));
		string head = extract_header(to_return.at(n));
		string name = to_return.at(n).name();

		if (to_return.at(n).is_question()
			&& head.find("@CAUSED_BY") != -1
			&& (name == "how" || name == "why")
		) {
			add_header( to_return.at(n), ":DELETE");
		}
	}

	DrtVect to_return2;
	for (int n = 0; n < to_return.size(); ++n) {
		if(!to_return.at(n).is_delete()) {
			to_return2.push_back(to_return.at(n));
		}
	}

	question.setPredicates(to_return2);

	return question;
}

vector<KnowledgeAnswer> Arbiter::arbiterStep(drt question, StepInfo &sinfo)
{
	vector<KnowledgeAnswer> to_return;

	sinfo.approximate_answer = false;

	int n = 0;
	DrtMgu mgu;
	bool yn_question;
	vector<DrtPred> qlist = question.getQuestionList().get();
	if (qlist.size() == 0)
		yn_question = true;
	else
		yn_question = false;

	int max_level = min(standard_max_level,wi_->getAccuracyLevel());


	int approx_level = 5;
	if (yn_question)
		max_level = min(max_level,4); // the 5th level is only word association, it does not work well with yes/no questions
	drt old_question;

	vector<int> approximations_with_same_candidates; //C++11// = {0,1,2};
	approximations_with_same_candidates.push_back(0);
	approximations_with_same_candidates.push_back(1);
	approximations_with_same_candidates.push_back(2);

	k_->setWisdomInfo(*wi_);

	vector<int> skip_level; //C++11// = {};
	vector<Candidate> candidates;
	do {
		if(shortfind(skip_level,n) )
			continue;

		if(wi_->getTimeout() < 0)
			break;


		pair<drt, DrtMgu> answer_pair = relax_conditions_in_question_at_level(question, mgu, n);
		question = answer_pair.first;
		mgu = answer_pair.second;


		// Finds the answers by searching in the knowledge
		clock_t start;

		vector<KnowledgeAnswer> tmp_answers;
		if(n == 0) {
			candidates = k_->getAnswerCandidates(question);
		}
		if(shortfind(approximations_with_same_candidates,n) ) {
			tmp_answers = k_->getAnswers(question,candidates);
		} else {
			tmp_answers = k_->getAnswers(question);
		}


		to_return.insert(to_return.end(), tmp_answers.begin(), tmp_answers.end());



		// Finds the answers by using the solver
		if (n == 0
			//n <= 2
			&& !wi_->getSkipSolver()) { // no inference for broken questions
			solver s(k_,wi_);
			if (to_return.size() == 0 // Questions already answered by the Knowledge are not solved
			    || wi_->getDoSolver() // do the solver anyway
			) {
				drt dquest = question;

				dquest = clean_cause(dquest);

				s.set_question(dquest);
				s.do_solve();
				vector<KnowledgeAnswer> solved = s.get_solved();
				for (int n = 0; n < solved.size(); ++n) {
					DrtVect preds = solved.at(n).getPreds();
					path_memory mem = solved.at(n).getMemory();
					double w = mem.get_total_weigth();
					vector<vector<clause_vector> > clause_history = mem.get_clause_history();
					to_return.insert(to_return.end(), solved.begin(), solved.end());
				}
			}
		}

		// Ask presupposed questions
		bool skip_presuppositions = false;
		if( to_return.size() != 0 && wi_->getSkipPresuppositions() )
			skip_presuppositions = true;
		if (n == 0 && !skip_presuppositions) {

			// Adds the answers to the presupposed questions
			vector<drt> presupp_questions;
			question.set_question();
			Presupposition pres(question); /// CORRECT: it considers only one question!!
			vector<drt> pres_drtvect = pres.get();

			if(wi_->getTimeout() < 0)
				break;

			presupp_questions.insert(presupp_questions.end(), pres_drtvect.begin(), pres_drtvect.end());

			// Finds the answers to the presupposed questions by searching in the knowledge
			vector<vector<KnowledgeAnswer> > presupp_answers;
			for (int qnum = 0; qnum < presupp_questions.size(); ++qnum) {
				if(wi_->getTimeout() < 0)
					break;
				vector<KnowledgeAnswer> tmp_answers = k_->getAnswers(presupp_questions.at(qnum));
				presupp_answers.push_back(tmp_answers);
			}

			for (int q = 0; q < presupp_answers.size(); ++q)
				to_return.insert(to_return.end(), presupp_answers.at(q).begin(), presupp_answers.at(q).end());
		}


		if (n >= approx_level && to_return.size()) {
			sinfo.approximate_answer = true;
		}

	} while (to_return.size() == 0 && n++ < max_level);

	use_mgu_prior(to_return, mgu);

	return to_return;
}

static drt specialize_question(drt question, const vector<KnowledgeAnswer> &answer, Knowledge *k)
{
	map<string,string > ref_and_name;

	for(int n=0; n < answer.size(); ++n) {
		DrtMgu mgu= answer.at(n).getDrtMgu();
		for(int m=0; m < mgu.size(); ++m) {
			string first = mgu.at(m).first;
			string second= mgu.at(m).second;

			vector<string> names= k->getNamesFromRef(second);
			// assemble the names into a single string like name1|name2|name3...
			string new_name;
			for(int j=0; j < names.size(); ++j) {
				new_name += names.at(j);
				if(j != names.size()-1)
					new_name += "|";
			}
			ref_and_name[first] = new_name;
		}
	}

	DrtVect qvect= question.predicates_with_references();

	for(int n=0; n < qvect.size(); ++n) {
		if(!qvect.at(n).is_question() )
			continue;
		string first= extract_first_tag(qvect.at(n));
		map<string,string>::iterator miter = ref_and_name.find(first);
		if(miter != ref_and_name.end()) {
			implant_header(qvect.at(n),miter->second);
		}
	}
	question.setPredicates(qvect);

	return question;
}

ArbiterAnswer Arbiter::processAllQuestions(vector<drt> questions)
{
	if (questions.size() == 0)
		return ArbiterAnswer(); // Always a positive number of questions


	vector<vector<KnowledgeAnswer> > answers;
	answers.resize(questions.size());

	vector<StepInfo> ainfo(questions.size());
	for (int qnum = 0; qnum < questions.size(); ++qnum) {
		answers.at(qnum) = this->arbiterStep(questions.at(qnum), ainfo.at(0));

		if(qnum < questions.size()-1)
			questions.at(qnum+1) = specialize_question(questions.at(qnum+1),answers.at(qnum), k_ );
	}

	// Manage the conjunctions
	vector<AnswerContainer> final_answers;
	if (answers.size()) {
		vector<int> already_answered;
		int fpos, spos;
		fpos = 0;
		already_answered.push_back(fpos);
		for(int m=0; m < answers.size(); ++m) {
			int size = answers.at(m).size();
			for (int n = 0; n < size; ++n) {
				KnowledgeAnswer ka = answers.at(m).at(n);
				AnswerContainer tmp_container(ka);
				final_answers.push_back(tmp_container);
			}
		}
	}

	int answer_size= final_answers.size();
	int max_size= wi_->getNumAnswers();
	max_size = min(answer_size,max_size);

	vector<Answer> wisdom_answers = get_wisdom_answers_from_answer_container(final_answers, k_, max_size);
	pre_wisdom_unique(wisdom_answers);
	sort(wisdom_answers.begin(), wisdom_answers.end(), compare_answers);


	// Fill the answer structure
	ArbiterAnswer ra;
	ArbiterItem ri = this->processComment(get_knowledge_answers_from_answer_container(final_answers, max_size), questions, ainfo.at(0));
	ra.setComment(ri.getComment());
	vector<KnowledgeAnswer> new_kav = ri.getKav();



	vector<Answer> new_w_answers = transform_kav_into_wisdom_answers(new_kav, k_);
	wisdom_answers.insert(wisdom_answers.end(), new_w_answers.begin(), new_w_answers.end());



	wisdom_unique(wisdom_answers);
	ra.setAnswer(wisdom_answers);
	ra.setStatus(ri.getStatus());
	return ra;
}

ArbiterAnswer Arbiter::processQuestion(const pair<vector<drt>, vector<DrtPred> > &question_conj)
{
	double max_time = wi_->getTimeout();

	vector<drt> questions = question_conj.first;
	vector<DrtPred> conjs = question_conj.second;

	if (questions.size() == 0)
		return ArbiterAnswer(); // Always a positive number of questions

	vector<boost::tuple<string, int, int> > conj_rel = get_conj_relations(questions, conjs);

	vector<vector<KnowledgeAnswer> > answers;
	answers.resize(questions.size());

	vector<StepInfo> ainfo(questions.size());
	for (int qnum = 0; qnum < questions.size(); ++qnum) {
		answers.at(qnum) = this->arbiterStep(questions.at(qnum), ainfo.at(0));

		if(qnum < questions.size()-1)
			questions.at(qnum+1) = specialize_question(questions.at(qnum+1),answers.at(qnum), k_ );
	}

	// Manage the conjunctions
	vector<AnswerContainer> final_answers;

	if (answers.size()) {
		vector<int> already_answered;
		int fpos, spos;
		fpos = 0;
		already_answered.push_back(fpos);
		int size = answers.at(0).size();
		for (int n = 0; n < size; ++n) {
			KnowledgeAnswer ka = answers.at(0).at(n);
			AnswerContainer tmp_container(ka);
			final_answers.push_back(tmp_container);
		}

		for (int cnum = 0; cnum < conj_rel.size(); ++cnum) {
			fpos = conj_rel.at(cnum).get<1>();
			spos = conj_rel.at(cnum).get<2>();
			string head = conj_rel.at(cnum).get<0>();
			if (find(already_answered.begin(), already_answered.end(), fpos) != already_answered.end()
			// The binary operator combines to the answer in spos
					) {
				already_answered.push_back(spos);
				if (head == "@CONJUNCTION" || head == "@DISJUNCTION") {
					final_answers = get_answer_intersection(final_answers, answers.at(spos), k_, wi_);
					continue;
				}
				if (head == "@COORDINATION") {
					final_answers.insert(final_answers.end(), answers.at(spos).begin(), answers.at(spos).end());
				}
			}
		}
	}

	int answer_size= final_answers.size();
	int max_size= wi_->getNumAnswers();
	max_size = min(answer_size,max_size);

	vector<Answer> wisdom_answers = get_wisdom_answers_from_answer_container(final_answers, k_, max_size);
	pre_wisdom_unique(wisdom_answers);
	sort(wisdom_answers.begin(), wisdom_answers.end(), compare_answers);


	// Fill the answer structure
	ArbiterAnswer ra;
	ArbiterItem ri = this->processComment(get_knowledge_answers_from_answer_container(final_answers, max_size), questions, ainfo.at(0));
	ra.setComment(ri.getComment());
	vector<KnowledgeAnswer> new_kav = ri.getKav();



	vector<Answer> new_w_answers = transform_kav_into_wisdom_answers(new_kav, k_);
	wisdom_answers.insert(wisdom_answers.end(), new_w_answers.begin(), new_w_answers.end());



	wisdom_unique(wisdom_answers);
	ra.setAnswer(wisdom_answers);
	ra.setStatus(ri.getStatus());
	return ra;
}

ArbiterAnswer Arbiter::processQuestion(const vector<QuestionVersions> &qvect)
{
	clock_t start_time = clock();

	wi_->startTime();
	double max_time = wi_->getTimeout();

	ArbiterAnswer ra;
	for(int candidate_num=0; candidate_num < qvect.size()
		&& candidate_num < 1; /// only one question in the alpha phase
	++candidate_num) {
		QuestionVersions qv= qvect.at(candidate_num);
		for(int qnum=0; qnum < qv.size(); ++qnum ) {
			CandidateQuestion cq= qv.at(qnum);
			vector<drt> questions = cq.getCandidates();
			vector<DrtPred> conjs = cq.getConjunctions();
			ra= this->processQuestion(make_pair(questions, conjs) );
			clock_t current_time = clock();
			double elapsed = (current_time - start_time) / (double) CLOCKS_PER_SEC;
			//if(ra.getAnswers().size() || elapsed > max_time)
			if(ra.getAnswers().size() || wi_->getTimeout() < 0 )
				break;
		}
	}

	return ra;
}

ArbiterAnswer Arbiter::getEmptyAnswer()
{
	ArbiterItem ri;
	string comment = "the answer is not found in the data";
	ri.setComment(comment);
	string status = "unknown";
	ri.setStatus(status);

	ArbiterAnswer ra;
	ra.setComment(ri.getComment());
	ra.setStatus(ri.getStatus());
	return ra;
}

void Arbiter::processQuestionForThread(const pair<vector<drt>, vector<DrtPred> > &question_conj, ArbiterAnswer *answer)
{
	*answer = this->getEmptyAnswer(); // if the thread is interrupted before step2, the answer is empty
	*answer = this->processQuestion(question_conj); // step2
}

void Arbiter::processQuestionVersionsForThread(const vector<QuestionVersions > &question_conj, ArbiterAnswer *answer)
{
	*answer = this->getEmptyAnswer(); // if the thread is interrupted before step2, the answer is empty
	*answer = this->processQuestion(question_conj); // step2
}


// Wikidata methods and functions

ArbiterAnswer Arbiter::processWikidataQuestion(const vector<QuestionVersions> &qvect)
{
	clock_t start_time = clock();

	wi_->startTime();
	double max_time = wi_->getTimeout();


	ArbiterAnswer ra;
	for(int candidate_num=0; candidate_num < qvect.size()
		&& candidate_num < 1; /// only one question in the alpha phase
	++candidate_num) {
		QuestionVersions qv= qvect.at(candidate_num);
		for(int qnum=0; qnum < qv.size(); ++qnum ) {
			CandidateQuestion cq= qv.at(qnum);
			vector<drt> questions = cq.getCandidates();
			vector<DrtPred> conjs = cq.getConjunctions();
			ra= this->processWikidataQuestion(make_pair(questions, conjs) );
			clock_t current_time = clock();
			double elapsed = (current_time - start_time) / (double) CLOCKS_PER_SEC;
			//if(ra.getAnswers().size() || elapsed > max_time)
			if(ra.getAnswers().size() || wi_->getTimeout() < 0 )
				break;
		}
	}

	return ra;
}


ArbiterAnswer Arbiter::processWikidataQuestion(const pair<vector<drt>, vector<DrtPred> > &question_conj)
{
	double max_time = wi_->getTimeout();

	vector<drt> questions = question_conj.first;
	vector<DrtPred> conjs = question_conj.second;

	if (questions.size() == 0)
		return ArbiterAnswer(); // Always a positive number of questions

	vector<boost::tuple<string, int, int> > conj_rel = get_conj_relations(questions, conjs);

	vector<vector<KnowledgeAnswer> > answers;
	answers.resize(questions.size());

	vector<StepInfo> ainfo(questions.size());
	for (int qnum = 0; qnum < questions.size(); ++qnum) {
		/// WikiData - start
		answers.at(qnum) = this->arbiterWikidataStep(questions.at(qnum), ainfo.at(0));
		/// WikiData - end
		if(qnum < questions.size()-1)
			questions.at(qnum+1) = specialize_question(questions.at(qnum+1),answers.at(qnum), k_ );
	}

	// Manage the conjunctions
	vector<AnswerContainer> final_answers;
	if (answers.size()) {
		vector<int> already_answered;
		int fpos, spos;
		fpos = 0;
		already_answered.push_back(fpos);
		int size = answers.at(0).size();
		for (int n = 0; n < size; ++n) {
			KnowledgeAnswer ka = answers.at(0).at(n);
			AnswerContainer tmp_container(ka);
			final_answers.push_back(tmp_container);
		}

		for (int cnum = 0; cnum < conj_rel.size(); ++cnum) {
			fpos = conj_rel.at(cnum).get<1>();
			spos = conj_rel.at(cnum).get<2>();
			string head = conj_rel.at(cnum).get<0>();
			if (find(already_answered.begin(), already_answered.end(), fpos) != already_answered.end()
			// The binary operator combines to the answer in spos
					) {
				already_answered.push_back(spos);
				if (head == "@CONJUNCTION" || head == "@DISJUNCTION") {
					//cout << "CONJ::: " << fpos << ", " << spos << ", " << head << endl;
					final_answers = get_answer_intersection(final_answers, answers.at(spos), k_, wi_);
					continue;
				}
				if (head == "@COORDINATION") {
					final_answers.insert(final_answers.end(), answers.at(spos).begin(), answers.at(spos).end());
				}
			}
		}
	}

	int answer_size= final_answers.size();
	int max_size= wi_->getNumAnswers();
	max_size = min(answer_size,max_size);

	vector<Answer> wisdom_answers = get_wisdom_answers_from_answer_container(final_answers, k_, max_size);
	pre_wisdom_unique(wisdom_answers);
	sort(wisdom_answers.begin(), wisdom_answers.end(), compare_answers);


	// Fill the answer structure
	ArbiterAnswer ra;
	ArbiterItem ri = this->processComment(get_knowledge_answers_from_answer_container(final_answers, max_size), questions, ainfo.at(0));
	ra.setComment(ri.getComment());
	vector<KnowledgeAnswer> new_kav = ri.getKav();



	vector<Answer> new_w_answers = transform_kav_into_wisdom_answers(new_kav, k_);
	wisdom_answers.insert(wisdom_answers.end(), new_w_answers.begin(), new_w_answers.end());



	wisdom_unique(wisdom_answers);
	ra.setAnswer(wisdom_answers);
	ra.setStatus(ri.getStatus());
	return ra;
}


static string clean_date(const string &header)
{
	string to_return;

	int pos = 0;
	if(header.find("[date]_") != string::npos) {
		pos = 7;
	}
	to_return = header.substr(pos,header.size() );
	return to_return;
}

static DrtVect get_wikidata_question_from_single_drt(DrtVect drtvect)
{
	DrtVect to_return;

	// create a pseudo-drt with the wikidata properties
	vector<pair<DrtVect,DrtVect> > rules;

	WikidataInfo *wdi = WikidataSingleton::instance();

	rules = wdi->getRules();

	Knowledge k;
	Match match(&k);
	for(int n=0; n < rules.size(); ++n) {
		drtvect = match.substituteAllWithRule(drtvect,rules.at(n) );
	}

	return drtvect;
}

static vector<int> find_all_compl_with_first_tag(vector<DrtPred> &pre_drt, string from_str, string head ="")
{
	vector<int> to_return;
	for (int n = 0; n < pre_drt.size(); ++n) {
		if (!pre_drt.at(n).is_complement())
			continue;
		string head_tmp = extract_header(pre_drt.at(n));
		if (head != "" && head_tmp.find(head) == string::npos )
			continue;
		string ftag = extract_first_tag(pre_drt.at(n));
		if (ftag == from_str)
			to_return.push_back(n);
	}
	return to_return;
}

struct Options {
	string props_;
	string type_;
	vector<string> qs;
};


static string get_wikidata_string_from_drt(DrtVect drtvect, Options *options)
{
	// find the first question-word
	int qpos= -1;
	for(int n=0; n < drtvect.size(); ++n) {
		if(drtvect.at(n).is_question() ) {
			qpos = n;
			break;
		}
	}

	map<string,vector<string> > forbidden_map;

	WikidataInfo *wdi = WikidataSingleton::instance();
	forbidden_map = wdi->getForbidden();


	vector<string> prev_properties;

	bool add_type= true; // what award -> TREE[397647][][31]

	// create the wikidata queries for the properties attached to the question
	vector<string> wikidata_rules;
	bool type_is_time= false;
	map<string,string> types = wdi->getTypes();
	string prop_str = "";
	if(qpos != -1) {
		string fref = extract_first_tag(drtvect.at(qpos) );
		vector<int> poz = find_all_compl_with_first_tag(drtvect,fref);
		for(int n=0; n < poz.size(); ++n) {
			int pos = poz.at(n);
			string sref = extract_second_tag(drtvect.at(pos) );
			int pos2 = find_name_with_string(drtvect,sref);
			if(pos2 == -1)
				continue;
			string cheader = extract_header(drtvect.at(pos));
			string nheader = extract_header(drtvect.at(pos2));
			if(cheader.find("@P-") == string::npos)
				continue;
			int start_pos = 0;
			if(nheader.find(":") != string::npos)
				start_pos= nheader.find(':')+2;
			if(nheader.find(":") == nheader.size()+1 )
				continue;
			cheader = cheader.substr(3,cheader.size() );

			nheader = nheader.substr(start_pos,nheader.size() );
			if(nheader == "0")
				continue;

			string wikidata_rule;

			vector<string> forbidden = forbidden_map[cheader];

			bool proceed= true;
			for(int m=0; m < forbidden.size(); ++m) {
				if(shortfind(prev_properties,forbidden.at(m)) ) {
					proceed = false;
				}
			}

			if(!proceed)
				continue;
			if(cheader == "166")
				add_type= false;
			prev_properties.push_back(cheader);

			if(cheader == "AFTER") {
				nheader= clean_date(nheader);
				wikidata_rule= (string) "BETWEEN[585,"  + nheader + "]";
			} else if(cheader == "BEFORE") {
				nheader= clean_date(nheader);
				wikidata_rule= (string) "BETWEEN[585,," + nheader + "]";
			} else if(cheader == "WHERE") {
				wikidata_rule= (string) "TREE[" + nheader + "][150][17,131]";
			} else if(cheader == "TIME_AT_PERSON") {
				nheader= clean_date(nheader);
				string nheader_to= nheader + "-13";
				wikidata_rule= (string) "BETWEEN[569," + nheader + "," + nheader_to + "]";
			} else {
				if(nheader.find("[date]") != string::npos) {
					nheader= clean_date(nheader); // if nheader is a [date] it is converted to the wikidata format
					string nheader_to= nheader + "-13";
					wikidata_rule= (string) "BETWEEN[" + cheader + "," + nheader + "," + nheader_to + "]";
				}
				else
					wikidata_rule= (string) "TREE[" + nheader + "][" +cheader+ "][" + cheader + "]";
			}
			map<string,string>::iterator miter =types.find(cheader);
			if(miter != types.end() ) {
				string type = miter->second;
				if(type == "time") {
					type_is_time = true;
					prop_str = cheader;
					options->type_ = type;
				}
			}

			wikidata_rules.push_back(wikidata_rule);
			options->qs.push_back(nheader);
		}
	}

	for(int n=0; add_type && n < drtvect.size(); ++n) {
		if(drtvect.at(n).is_question()) {
			string header = extract_header(drtvect.at(n) );
			int start = header.find(":Q");
			if(start != string::npos) {
				string nheader = header.substr(start+2,header.size());
				string wikidata_rule= (string) "TREE[" + nheader + "][][31]";
				wikidata_rules.push_back(wikidata_rule);
			}
		}
	}

	string query_str;
	for(int n=0; n < wikidata_rules.size(); ++n) {
		if(debug)
			cout << "WRULE::: " << wikidata_rules.at(n) << endl;
		query_str += wikidata_rules.at(n);
		if(n != wikidata_rules.size()-1)
			query_str += "%20AND%20";
	}

	if(type_is_time) {
		query_str+="&props="+prop_str;
		options->props_ = prop_str;
	}



	return query_str;
}

string convert_time_from_nlulite_to_wiki(const string &wdate)
{
	vector<string> months;
	months.push_back("january");
	months.push_back("february");
	months.push_back("march");
	months.push_back("april");
	months.push_back("may");
	months.push_back("june");
	months.push_back("july");
	months.push_back("august");
	months.push_back("september");
	months.push_back("october");
	months.push_back("november");
	months.push_back("december");

	string to_return,date;
	date = wdate.substr(wdate.find("+")+1,wdate.find("T")-1);
	vector<string> strs;
	boost::split(strs,date,boost::is_any_of("-"));
	int year  = boost::lexical_cast<int>(strs.at(0));
	int month = boost::lexical_cast<int>(strs.at(1));
	int day   = boost::lexical_cast<int>(strs.at(2));

	string month_str = months.at(month);

	to_return += "[date]";
	to_return += "_"+boost::lexical_cast<string>(day);
	to_return += "_"+month_str;
	to_return += "_"+boost::lexical_cast<string>(year);


	return to_return;
}

static vector<string> post_to_wikidata(string wikidata_query, Options options)
{
	vector<string> to_return;

	string host = "wdq.wmflabs.org";
	string port = "80";

	clock_t start;
	if(debug || measure_time )
		start = clock();

	try {
		boost::asio::ip::tcp::iostream request_stream;
		request_stream.connect(host,port);
		if(!request_stream) {
			return to_return;
		}

		request_stream << "GET /api?q=" << wikidata_query << " HTTP/1.1\r\n"
				<< "Connection: Keep-Alive\r\n"
				<< "Host: " << host << "\r\n"
				<< "User-Agent: NLUlite Wikidata query" << "\r\n"
				//<< "User-Agent: Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Ubuntu Chromium/37.0.2062.94 Chrome/37.0.2062.94 Safari/537.36" << "\r\n"

				<< "Accept-Encoding: none" << "\r\n"
				<< "Accept-Language: en-US,en;q=0.8,sv;q=0.6,it;q=0.4,de;q=0.2,es;q=0.2,fr;q=0.2" << "\r\n"
				<< "\r\n";

		std::string line1;

		std::getline(request_stream,line1);
		if (!request_stream) {
			if(debug)
				std::cout << "REPLY1::: " << std::endl;
			return to_return;
		}
		std::stringstream response_stream(line1);
		std::string http_version;
		response_stream >> http_version;
		unsigned int status_code;
		response_stream >> status_code;
		std::string status_message;
		std::getline(response_stream,status_message);

		if (!response_stream||http_version.substr(0,5)!="HTTP/") {
			if(debug)
				std::cout << "REPLY2::: " << std::endl;
			return to_return;
		}
		if (status_code != 200 ) {
			if(debug)
				std::cout << "REPLY3::: " << status_code << std::endl;
			return to_return;
		}
		vector<string> headers;
		std::string header;
		while (std::getline(request_stream, header)
			&& header.find("items\":[") == string::npos
			//&& header != "\r" // NO! The last line must be the return data
		) {
			headers.push_back(header);
		}

		if(debug)
			std::cout << "REPLY::: " << header << std::endl;

		string return_str = header;

		string type = options.type_;
		if(type == "") { // no props in this request
			return_str = return_str.substr(return_str.find("items\":[")+ string("items\":[").size(), return_str.size() ) ;
			int end= return_str.find("]");
			if(end != string::npos)
				return_str = return_str.substr(0, end );
		} else { // the answer is a propr
			string str_to_search = (string)"\"time\",\"";
			string wikireply = return_str;
			return_str = "";
			int pos= wikireply.find(str_to_search);
			while (pos != string::npos) {
				string result;
				result = wikireply.substr(pos + str_to_search.size(), wikireply.size() );
				int end= result.find("\"");
				result = result.substr(0, end );
				string date_str= convert_time_from_nlulite_to_wiki(result);
				return_str += date_str + ',';
				pos    = wikireply.find(str_to_search,pos+str_to_search.size());
			}
			if(return_str.size() && return_str.at(return_str.size()-1) == ',') {
				return_str = return_str.substr(0,return_str.size()-1);
			}
		}
		if(return_str != "")
			boost::split(to_return,return_str,boost::is_any_of(",") );

	} catch (std::exception &e) {
		if(debug)
			std::cout << "REPLY4::: " << e.what() << std::endl;
	}


	return to_return;
}

static DrtVect substitute_question_predicate(DrtVect drtvect, const string &qanswer)
{
	for(int n=0; n < drtvect.size(); ++n) {
		if(drtvect.at(n).is_question() ) {
			string new_ref = qanswer;
			string header = extract_header(drtvect.at(n));
			if(header.find(":Q") != string::npos)
				header = header.substr(0,header.find(":") );
			implant_header(drtvect.at(n), header+":Q" + new_ref);
			break;
		}
	}

	return drtvect;
}

static DrtVect clean_after_colon(DrtVect drtvect, const string &qanswer)
{
	for(int n=0; n < drtvect.size(); ++n) {
		string header = extract_header(drtvect.at(n) );
		if(header.find(":") != string::npos && header.find(":Q") == string::npos) {
			string new_header = header.substr(0,header.find(":") );
			implant_header(drtvect.at(n), new_header);
		}
		if(header.find(":") != string::npos && header.find("|") != string::npos) {
			string new_header = header.substr(0,header.find(":") );
			implant_header(drtvect.at(n), new_header);
		}
	}

	return drtvect;
}

static DrtVect take_away_question_unifiers(DrtVect drtvect, const string &qanswer)
{
	for(int n=0; n < drtvect.size(); ++n) {
		string fref = extract_first_tag(drtvect.at(n) );
		if(fref.size() && fref.at(0) == '_') {
			string new_ref= fref.substr(1,fref.size() );
			implant_first(drtvect.at(n), new_ref);
		}
	}

	return drtvect;
}

static QuestionList get_question_list(DrtVect drtvect, const string &qanswer)
{
	QuestionList ql;
	for(int n=0; n < drtvect.size(); ++n) {
		if(drtvect.at(n).is_question() ) {
			ql.add(drtvect.at(n));
			break;
		}
	}

	return ql;
}


DrtVect add_elements_to_WRB(DrtVect to_return)
{
	for(int n=0; n < to_return.size(); ++n) {
		if(to_return.at(n).is_question() && to_return.at(n).is_WRB()) {
			DrtPred dummy("dummy/NN#[pivot](dummy)");
			string new_ref = "_name_dummy";
			implant_first(dummy,new_ref);
			implant_second(to_return.at(n),new_ref);
			implant_header(dummy,to_return.at(n).name());
			dummy.set_question(true);
			dummy.set_question_word( to_return.at(n).get_question_word() );
			to_return.at(n).set_question(false);
			to_return.push_back(dummy);
		}
	}

	return to_return;
}

static KnowledgeAnswer create_kanswer_from_wikianswer(const string &qanswer,const drt &question)
{
	KnowledgeAnswer single_answer;

	bool to_add = true;

	vector<DrtPred> tmp_drs(question.predicates_with_references() );
	tmp_drs         = add_elements_to_WRB(tmp_drs);
	tmp_drs         = substitute_question_predicate(tmp_drs, qanswer);
	tmp_drs         = clean_after_colon(tmp_drs, qanswer);
	//tmp_drs         = take_away_question_unifiers(tmp_drs, qanswer);
	QuestionList ql = get_question_list(tmp_drs,qanswer);

	double w    = 1;
	string link = "http://wikidata.org/wiki/Q"+qanswer;
	string text = "From Wikidata";

	single_answer.setPreds(tmp_drs);
	single_answer.setWeigth(w);
	single_answer.setLink(link);
	single_answer.setText(text);
	single_answer.setQuestionList(ql);

	return single_answer;
}

vector<DrtVect> get_all_word_combinations(const DrtVect &question)
{
	vector<DrtVect> to_return;

	vector<DrtVect> tmp_solutions;

	tmp_solutions.push_back(question);
	int num_solutions=1;
	for(int m=0; m < question.size(); ++m) {
		DrtPred pred(question.at(m));
		string header = extract_header(pred);
		vector<string> strs;
		boost::split(strs,header,boost::is_any_of(":") );
		if(strs.size() != 2) {
			continue;
		}
		string orig_header = strs.at(0);
						string qs_str = strs.at(1);
		boost::split(strs,qs_str,boost::is_any_of("|") );
		int max= strs.size();
		vector<string> qs = strs;
		if(qs.size() == 0) {
			continue;
		}
		vector<DrtVect> new_solutions;
		for (int n=0; n < qs.size(); ++n) {
			for(int i=0; i < tmp_solutions.size(); ++i) {
				DrtVect tmp_drt = tmp_solutions.at(i);
				string new_header = orig_header + ":" + qs.at(n);
				implant_header(tmp_drt.at(m),new_header);
				new_solutions.push_back(tmp_drt);
			}
		}
		tmp_solutions = new_solutions;
	}
	to_return= tmp_solutions;


	return to_return;
}

vector<string> filter_valid_answers(const vector<string> &answers, Options options)
{
	vector<string> to_return;

	for(int n=0; n < answers.size(); ++n) {
		string qstr= answers.at(n);
		if(qstr != "0" && !shortfind(options.qs,qstr) )
			to_return.push_back(qstr);
	}

	return to_return;
}

vector<KnowledgeAnswer> Arbiter::arbiterWikidataStep(drt question, StepInfo &sinfo)
{
	vector<KnowledgeAnswer> to_return;

	DrtVect drt_question = question.predicates_with_references();

	clock_t start;

	vector<string> q_answer;
	Options options;
	drt_question          = add_elements_to_WRB(drt_question);


	vector<DrtVect> questions = get_all_word_combinations(drt_question);
	vector<string> already_asked;
	for(int n=0; n < questions.size(); ++n) {

		if(wi_->getTimeout() < 0)
			break;


		drt_question                = get_wikidata_question_from_single_drt(questions.at(n));
		string wikidata_query       = get_wikidata_string_from_drt(drt_question,&options);
		if(shortfind(already_asked,wikidata_query))
			continue;
		vector<string> tmp_q_answer = post_to_wikidata(wikidata_query,options);
		already_asked.push_back(wikidata_query);
		tmp_q_answer                = filter_valid_answers(tmp_q_answer,options);
		q_answer.insert(q_answer.end(),tmp_q_answer.begin(),tmp_q_answer.end());
		if(q_answer.size() > 10)
			break; // it just makes it faster
	}


	for(int n=0; n < q_answer.size(); ++n) {
		KnowledgeAnswer ka= create_kanswer_from_wikianswer(q_answer.at(n),question);
		to_return.push_back(ka);
	}

	return to_return;
}

