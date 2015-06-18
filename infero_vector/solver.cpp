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



#include"solver.hpp"

const bool debug = false;
const bool measure_time = false;

const int max_backtracking= 121;
const int standard_max_level = 2;


static inline int max(int a, int b)
{
	return a < b ? b : a;
}
static inline int min(int a, int b)
{
	return a >= b ? b : a;
}

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
	typename vector<T>::iterator tags_iter = vs.begin();
	while (tags_iter < vs.end()) {
		std::cout << (*tags_iter) << " ";
		++tags_iter;
	}
	std::cout << std::endl;
}

template<class T>
static void print_vector_return(std::vector<T> &vs)
{
	typename vector<T>::iterator tags_iter = vs.begin();
	while (tags_iter < vs.end()) {
		std::cout << (*tags_iter) << endl;
		++tags_iter;
	}
	std::cout << std::endl;
}

static vector<string> get_feet_from_clause(const clause_vector &clause)
{
	vector<string> hyp_tags;
	vector<DrtPred> hyp = clause.getHypothesis();
	vector<DrtPred>::iterator hyp_iter = hyp.begin();

	for (; hyp_iter != hyp.end(); ++hyp_iter)
		hyp_tags.push_back(hyp_iter->extract_header());
	return hyp_tags;
}

static inline vector<int> get_feet_positions(int pos_first_feet, const vector<int> &space_between_feet)
{
	int size = space_between_feet.size() + 1;
	vector<int> ret_vect(size);
	ret_vect.at(0) = pos_first_feet;
	for (int n = 1; n < size; ++n)
		ret_vect.at(n) = ret_vect.at(n - 1) + space_between_feet.at(n - 1);
	return ret_vect;
}

static inline vector<string> get_feet_on_positions(const vector<DrtPred> &data, vector<int> positions)
{
	vector<string> to_ret;
	int size = positions.size();
	for (int n = 0; n < size; ++n)
		to_ret.push_back(data.at(positions.at(n)).extract_header());
	return to_ret;
}

static bool compare_clauses(const clause_vector &lhs, const clause_vector &rhs)
{
	std::stringstream sl, sr;
	sl << lhs;
	sr << rhs;
	return sl.str() < sr.str();
}

static bool is_same_clause(const clause_vector &lhs, const clause_vector &rhs)
{
	std::stringstream sl, sr;
	sl << lhs;
	sr << rhs;
	return sl.str() == sr.str();
}

static string extrapolate_levin_description(const vector<DrtPred> &preds)
{
	string description;
	metric *d = metric_singleton::get_metric_instance();

	vector<DrtPred>::const_iterator piter = preds.begin();
	vector<DrtPred>::const_iterator pend = preds.end();
	string lev_tmp;

	for (; piter != pend; ++piter) {
		string head_str = extract_header(*piter);
		if (piter->is_verb()) {
			lev_tmp = d->get_levin_verb(head_str);
			if (lev_tmp.size()) {
				description += lev_tmp;
				description += "-";
			}
		} else {
			lev_tmp = d->get_levin_noun(head_str);
			if (lev_tmp.size()) {
				description += lev_tmp;
				description += "-";
			}
		}
	}
	description.erase(description.size() - 1);

	return description;
}

vector<vector<clause_vector> > solver::get_relevant_clauses(vector<DrtVect> &predicates)
{
	vector<vector<clause_vector> > to_return;
	clause_vector id_clause;
	for (int n = 0; n < predicates.size(); ++n) {
		vector<clause_vector> clauses;
		DrtVect drtvect = predicates.at(n);
		vector<KnowledgeAnswer> answers = k_->getRules(drtvect);
		if (answers.size()) {
			for (int n = 0; n < answers.size(); ++n) {
				double w = answers.at(n).getWeigth();
				if (w > 0.4) {
					clause_vector clause = answers.at(n).getClause();
					string link = answers.at(n).getLink();
					clause.setLink(link);
					clauses.push_back(clause);
				} else
					clauses.push_back(id_clause); // the void clauses is the Identity clause
			}
			to_return.push_back(clauses);
			clauses.clear();
		} else {
			vector<clause_vector> clauses;
			clauses.push_back(id_clause); // the void clauses is the Identity clause
			to_return.push_back(clauses);
		}
	}
	if(to_return.size() == 1 && to_return.at(0).size() == 1 && to_return.at(0).at(0) == id_clause)
		to_return.clear(); // only one ID clause is meaningless
	return to_return;
}

static vector<Level> launch_inference_threads(const vector<vector<clause_vector> > &clauses, const vector<DrtVect> &data,
		const path_memory &mem, Knowledge *k, double noise, int num_cycles, int trace, int num_level)
{
	// The question
	DrtPred question("--");
	inference_vector inference0(question, data, 1, num_level, clauses, mem, k);
	inference0.init();
	inference0.setNoise(noise);
	inference0.makeFeetStep(num_cycles, trace);
	vector<Level> tmpdata, retdata;

	tmpdata = inference0.getLastData();

	retdata.insert(retdata.end(), tmpdata.begin(), tmpdata.end());

	return retdata;
}

static inline vector<int> get_predicates_positions(int pos_first_feet, const vector<int> &space_between_feet)
{
	int size = space_between_feet.size() + 1;
	vector<int> ret_vect(size);
	ret_vect.at(0) = pos_first_feet;
	for (int n = 1; n < size; ++n)
		ret_vect.at(n) = ret_vect.at(n - 1) + space_between_feet.at(n - 1);
	return ret_vect;
}

static inline vector<DrtPred> get_predicates_on_positions(const vector<DrtPred> &data, vector<int> positions)
{
	vector<DrtPred> to_ret;
	int size = positions.size();
	for (int n = 0; n < size; ++n)
		to_ret.push_back(data.at(positions.at(n)));
	return to_ret;
}

static vector<drt> get_drts_from_back_pairs(vector<tuple<vector<KnowledgeAnswer>, int, double, DrtMgu> > &back_pairs)
{
	vector<drt> to_return;

	for (int n = 0; n < back_pairs.size(); ++n) {
		int pos = back_pairs.at(n).get<1>();
		if (n != back_pairs.size() - 1) // all the elements but the last point to the n+1 position
			--pos;
		vector<KnowledgeAnswer> all_answers = back_pairs.at(n).get<0>();
		if (all_answers.size() == 0)
			throw(std::runtime_error("Solver: Getting drts: no such answer."));
		KnowledgeAnswer answer = all_answers.at(pos);
		DrtVect drtvect = answer.getPreds();
		drt tmp_drt(drtvect);
		tmp_drt.setLink(answer.getLink());
		tmp_drt.setText(answer.getText());
		to_return.push_back(tmp_drt);
	}

	return to_return;
}

static vector<pair<CodePred, int> > get_all_levels_codes(path_memory &mem)
{
	vector<pair<CodePred, int> > to_return;

	vector<tuple<vector<clause_vector>, DrtMgu, double> > all_memory = mem.get_memory();
	if (all_memory.size()) {
		for (int m = 0; m < all_memory.size(); ++m) {
			vector<clause_vector> last_clauses = all_memory.at(m).get<0>();
			for (int n = 0; n < last_clauses.size(); ++n) {
				if (last_clauses.at(n).hasMatchCode())
					to_return.push_back(make_pair(last_clauses.at(n).getMatchCode(), m));
			}
		}
	}

	return to_return;
}

static DrtMgu get_all_levels_upg(path_memory &mem)
{
	DrtMgu to_return;

	vector<tuple<vector<clause_vector>, DrtMgu, double> > all_memory = mem.get_memory();
	if (all_memory.size()) {
		for (int m = 0; m < all_memory.size(); ++m) {
			DrtMgu upg = all_memory.at(m).get<1>();
			//to_return.addReverse(upg);
			to_return.add(upg);
		}
	}
	return to_return;
}

static pair<DrtVect, DrtMgu> break_off_specifications(DrtVect drtvect)
{
	DrtMgu mgu;
	for (int n = 0; n < drtvect.size(); ++n) {
		string fref = extract_first_tag(drtvect.at(n));
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

vector<KnowledgeAnswer> solver::arbiterStep(drt question)
{
	vector<KnowledgeAnswer> to_return;

	int n = 0;
	DrtMgu mgu;
	bool yn_question;
	vector<DrtPred> qlist = question.getQuestionList().get();
	if (qlist.size() == 0)
		yn_question = true;
	else
		yn_question = false;

	int max_level = min(standard_max_level,wi_->getAccuracyLevel());

	if(debug) {
		cout << "MAX_LEVEL:::" << max_level << endl;
	}

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


		if(debug) {
			cout << "STIMEOUT03::: " << wi_->getTimeout() << endl;
		}

		pair<drt, DrtMgu> answer_pair = relax_conditions_in_question_at_level(question, mgu, n);
		question = answer_pair.first;
		mgu = answer_pair.second;

		if (debug) {
			cout << "SOLVER_QUESTION::: " << n << endl;
			DrtVect tmppreds = question.predicates_with_references();
			print_vector(tmppreds);
			cout << mgu << endl;
		}

		// Finds the answers by searching in the knowledge
		clock_t start;
		if(debug || measure_time) {
			start = clock();
		}

		vector<KnowledgeAnswer> tmp_answers;
		if(n == 0) {
			candidates = k_->getAnswerCandidates(question);
		}
		if(shortfind(approximations_with_same_candidates,n) ) {
			tmp_answers = k_->getAnswers(question,candidates);
		} else {
			tmp_answers = k_->getAnswers(question);
		}

		if (debug || measure_time ) {
			clock_t end = clock();
			cout << "Mtime_arbiter_knowledge::: " << (end - start) / (double) CLOCKS_PER_SEC << endl;
		}

		to_return.insert(to_return.end(), tmp_answers.begin(), tmp_answers.end());

		bool skip_presuppositions = true; // Skip them for now, it adds too much complexity ( questions can be truncated)
		//bool skip_presuppositions = false;
		if( to_return.size() == 0 && wi_->getSkipPresuppositions() )
			skip_presuppositions = true;
		if (n == 0 && !skip_presuppositions) {

			// Adds the answers to the presupposed questions
			vector<drt> presupp_questions;
			question.set_question();
			Presupposition pres(question); /// CORRECT: it considers only one question!!
			vector<drt> pres_drtvect = pres.get();
			if (debug)
				cout << "PRESUPPOSITION:::" << pres_drtvect.size() << endl;
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


	} while (to_return.size() == 0 && n++ < max_level);

	if (debug)
		puts("END_SOLVER:::");

	if (debug)
		cout << "ADDITIONAL_MGU:::" << mgu;

	use_mgu_prior(to_return, mgu);

	return to_return;
}



double solver::is_match(const vector<vector<DrtPred> > &hyps, DrtMgu *retDrtMgu, vector<drt> *drt_to_return, path_memory &mem)
{
/// Backtracking is not implemented correctly!

	int num_backtracking = 0;

	vector<tuple<vector<KnowledgeAnswer>, int, double, DrtMgu> > back_pairs;
	int hpos, hsize;
	int depth = mem.getDepth();

	hsize = hyps.size();
	back_pairs.resize(hsize);

	vector<pair<CodePred, int> > level_codes = get_all_levels_codes(mem);
	DrtMgu level_upg = get_all_levels_upg(mem);

	for (hpos = 0; hpos != hsize; ++hpos) {
		// Save the answers in the backtracking vector
		back_pairs.at(hpos).get<1>() = 0; // The backtracked position
		back_pairs.at(hpos).get<2>() = 0; // The backtracked weigth
	}
	hpos = 0;
	Engine engine(k_); /// This is dangerous: elements defined in the backtracking can last

	while (hpos < hsize && hpos >= 0) {

		if(debug) {
			cout << "STIMEOUT::: " << wi_->getTimeout() << endl;
		}
		if(wi_->getTimeout() < 0)
			break;

		if(num_backtracking <= max_backtracking) {
			++num_backtracking;
		} else {
			return 0;
		}

		DrtVect question = hyps.at(hpos);
		DrtMgu prev_upg;
		double prev_w = 1;
		vector<KnowledgeAnswer> answers;
		if (back_pairs.at(hpos).get<0>().size()) {
			// If there are answers at this level
			if (hpos > 0) {
				prev_w = back_pairs.at(hpos - 1).get<2>();
				prev_upg = back_pairs.at(hpos - 1).get<3>();
			}
			question / prev_upg;
			answers = back_pairs.at(hpos).get<0>();
		} else {
			// If this level is unexplored
			if (hpos > 0) {
				prev_w = back_pairs.at(hpos - 1).get<2>();
				prev_upg = back_pairs.at(hpos - 1).get<3>();
			}
			question / prev_upg;
			answers = this->arbiterStep(question);
			//answers = k_->getAnswers(question);

			if(debug) {
				cout << "SOLVER_ANSWERS::: " << answers.size() << endl;
			}

			back_pairs.at(hpos).get<0>() = answers;
		}
		if (answers.size() == 0) {
			if (hpos == 0)
				break; // nothing is found
			--hpos;
			continue;
		}
		int n = back_pairs.at(hpos).get<1>();
		int nmax = answers.size();
		if (n >= nmax) {
			if (hpos == 0)
				break; // nothing is found
			--hpos;
			continue;
		}
		vector<DrtPred> drtvect = answers.at(n).getPreds();
		double w = answers.at(n).getWeigth();
		DrtMgu upg = prev_upg;
		upg.add(answers.at(n).getDrtMgu());

		if (w > 0.4) {
			bool code_agrees = true;
			for (int ncode = 0; ncode < level_codes.size(); ++ncode) {
				CodePred code = level_codes.at(ncode).first;
				int level_depth = level_codes.at(ncode).second;
				//upg.uniVal(1);
				DrtMgu upg_tmp = upg;
				//code.pred().uniVal(level_depth+2);
				code / level_upg;
				code / upg_tmp;
				engine.setKnowledge(k_);
				//code.pred().uniVal(1);
				CodePred result = engine.run(code);
				if (result.pred() != Engine::pt_true) {
					code_agrees = false;
					break;
				}
			}

			// Check if the matching code agrees with the current choice
			if (hpos == hsize - 1 && code_agrees) {
				try {
					*drt_to_return = get_drts_from_back_pairs(back_pairs);
				} catch (std::exception &e) {
					return 0;
				}
				*retDrtMgu = upg;
				return w;
			}
			// Save the data into the backtracking structure
			back_pairs.at(hpos).get<1>() = n + 1;
			back_pairs.at(hpos).get<2>() = w * prev_w;
			back_pairs.at(hpos).get<3>() = upg;
			if (code_agrees)
				++hpos;
		} else
			--hpos;
	}
	return 0;
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
class compare_pred {
public:
	bool operator ()(const vector<DrtPred> lhs, const vector<DrtPred> rhs) const
	{
		//if(lhs.size() > rhs.size()) return false;
		std::stringstream sl, sr;
		print_vector(sl, lhs);
		print_vector(sr, rhs);
		return sl.str() < sr.str();
	}
};

static inline bool compare(const Level &lhs, const Level &rhs)
{
	double w_lhs = lhs.getMemory().get_total_weigth();
	double w_rhs = rhs.getMemory().get_total_weigth();

	return w_lhs > w_rhs;
}

static inline void sort_data(vector<Level> *data)
{
	sort(data->begin(), data->end(), compare);
}

static inline bool compare_pair(const pair<vector<DrtPred>, path_memory> &lhs, const pair<vector<DrtPred>, path_memory> &rhs)
{
	double w_lhs = lhs.second.get_total_weigth();
	double w_rhs = rhs.second.get_total_weigth();

	return w_lhs > w_rhs;
}

static inline void sort_data(vector<pair<vector<DrtPred>, path_memory> > *data)
{
	sort(data->begin(), data->end(), compare_pair);
}

static inline double min(const double &a, const double &b)
{
	return (a < b) ? a : b;
}
static void insert_data(vector<pair<vector<DrtPred>, double> > *data_double, vector<pair<vector<DrtPred>, int> > data_int,
		const double &weight)
{
	vector<pair<vector<DrtPred>, int> >::iterator data_int_iter = data_int.begin();
	vector<pair<vector<DrtPred>, int> >::iterator data_int_end = data_int.end();
	map<vector<DrtPred>, int, compare_pred> map_data_int;

	int tot_int = 0;
	for (; data_int_iter != data_int_end; ++data_int_iter) {
		map_data_int[data_int_iter->first] += data_int_iter->second;
		tot_int += data_int_iter->second;
	}
	map<vector<DrtPred>, int>::iterator map_data_iter = map_data_int.begin();
	map<vector<DrtPred>, int>::iterator map_data_end = map_data_int.end();
	for (; map_data_iter != map_data_end; ++map_data_iter) {
		//double wh=  min( weight, (double)map_data_iter->second/tot_int );
		double wh = weight * (double) map_data_iter->second / tot_int;
		data_double->push_back(make_pair(map_data_iter->first, wh));
	}
}

bool solver::areValidRules(const path_memory &pm)
{
	DrtMgu upg = pm.get_first_upg();
	vector<vector<clause_vector> > cv = pm.get_clause_history();
	for (int m = 0; m < cv.size(); ++m) {
		vector<clause_vector> rules = cv.at(m);
		for (int n = 0; n < rules.size(); ++n) {
			clause_vector tmp_rule = rules.at(n);
			Engine engine(k_);
			if (tmp_rule.hasCode()) {
				int depth = m + 2;
				CodePred code = tmp_rule.getCode();
				//code.pred().uniVal(depth);
				//upg.uniVal(1);
				vector<DrtVect> hyps = tmp_rule.getAllHypothesis();
				for (int j = 0; j < hyps.size(); ++j) {
					DrtVect hyp = hyps.at(j);
					hyp / upg;
					k_->addTemporary(hyp);
				}
				code / upg;
				engine.setKnowledge(k_);
				//code.pred().uniVal(1);
				CodePred result = engine.run(code);

				if (result == Engine::pt_false)
					return false;
			}
		}
	}
	k_->clearAllTemporary();
	return true;
}

vector<DrtVect> solver::updateCurrentLayer(const Level &current_layer, DrtVect question)
{
	path_memory mem_tmp = current_layer.getMemory();
	vector<DrtVect> qvect = current_layer.getData();

	DrtMgu upg = get_all_levels_upg(mem_tmp);

	qvect / upg;
	question /upg;
	k_->addTemporary(question);

	for (int m = 0; m < qvect.size(); ++m) {
		DrtVect drtvect = qvect.at(m);
		for (int n = 0; n < drtvect.size(); ++n) {
			string fref = extract_first_tag(drtvect.at(n));
			string head = extract_header(drtvect.at(n));
			vector<string> all_names;
			if (drtvect.at(n).is_name())
				all_names = k_->getNounNames(fref);
			else if (drtvect.at(n).is_verb())
				all_names = k_->getVerbNames(fref);
			if (all_names.size() == 0)
				continue;
			if (head == "[*]" || head == "[what]" || head == "person" || head == "place"
					//|| head == "time"
			) {
				string new_name = all_names.at(0);
				if(debug) {
					cout << "SOLVER_ALL_NAMES::: ";
					print_vector(all_names);
					print_vector(question);
					print_vector(drtvect);
					cout << "UPG:::" << upg ;
				}

				if (new_name == "[any]")
					new_name = "[*]";
				implant_header(drtvect.at(n), new_name);
			}
		}
		qvect.at(m) = drtvect;
	}
	k_->clearAllTemporary();
	return qvect;
}

static vector<vector<clause_vector> > filter_simple_inference(const vector<vector<clause_vector> > &feet_clauses)
{
	vector<vector<clause_vector> > to_return(feet_clauses.size() );
	for(int n = 0; n < feet_clauses.size(); ++n) {
		for(int m = 0; m < feet_clauses.at(n).size(); ++m) {
			if(feet_clauses.at(n).at(m).getAllHypothesis().size() == 1)
				to_return.at(n).push_back(feet_clauses.at(n).at(m));
		}
	}
	return to_return;
}


void solver::do_solve()
{
	//DrtPred question("--");
	/// erase the question !!!
	string solver_options= wi_->getSolverOptions();

	vector<Level> solved;

	vector<DrtVect> question_vect;
	question_vect.push_back(question_);
	vector<vector<clause_vector> > new_feet_clauses = get_relevant_clauses(question_vect);
	if(solver_options.find("simple") != string::npos) {
		new_feet_clauses= filter_simple_inference(new_feet_clauses);
	}
	if (new_feet_clauses.size() == 0
		|| (new_feet_clauses.size() != 0 && new_feet_clauses.at(0).size() == 0)
	) {
		if (debug)
			puts("Empty Clauses!!");
		return;
	}
	if (debug) {
		cout << "CLAUSES_NUM::: " << new_feet_clauses.size()  << endl;
	}

	// cycles through the inferences steps
	int i;
	solved.clear();
	vector<Level> lastData, data_list;

	if (debug)
		puts("Solving...");
	path_memory mem;
	int n = 0;
	//set_clauses_unival(&new_feet_clauses, n+2);
	if (debug)
		print_vector(question_);
	lastData = launch_inference_threads(new_feet_clauses, question_vect, mem, k_, 1, 10, 1, n + 2);
	if (lastData.size() == 0) {
		if (debug)
			puts("Empty!!");
		return;
	}

	DrtMgu upg;
	vector<path_memory> mem_vect;
	data_list.insert(data_list.end(), lastData.begin(), lastData.end());

	sort_data(&data_list);

	Level current_layer;
	double last_layer_weight = 1;

	/// The following is necessary so that the clauses order in the first layer does not count!!!
	for (int m = 0; m < data_list.size(); ++m) {
		current_layer = data_list.at(m);
		vector<DrtVect> qvect = this->updateCurrentLayer(current_layer, question_);
		if (debug) {
			puts("LAYERS:::");
			cout << n << endl;
			for (int j = 0; j < qvect.size(); ++j) {
				print_vector(qvect.at(j));
			}
		}
		path_memory mem_tmp = current_layer.getMemory();
		upg.clear();
		vector<drt> tmp_drts;

		clock_t start;
		if (debug || measure_time) {
			start = clock();
		}
		if(debug) {
			cout << "STIMEOUT01::: " << wi_->getTimeout() << endl;
		}
		double w = is_match(qvect, &upg, &tmp_drts, mem_tmp);
		if(debug) {
			cout << "STIMEOUT02::: " << wi_->getTimeout() << endl;
		}
		if(wi_->getTimeout() < 0 )
			break;
		if (debug || measure_time) {
			clock_t end = clock();
			cout << "Mtime_is_match::: " << (end - start) / (double) CLOCKS_PER_SEC << endl;
		}

		if (debug)
			cout << "SOLVER22:::" << w << endl;
		if (w != 0) {
			mem_tmp.last_upg(upg);
			mem_tmp.last_weigth(w);
			mem_tmp.setDrt(tmp_drts);
			mem_tmp.close();
			if (this->areValidRules(mem_tmp))
				mem_vect.push_back(mem_tmp);
		}
	}

	if(debug) {
		cout << "STIMEOUT1::: " << wi_->getTimeout() << endl;
	}

	/// MULTIPLE LEVELS IN THE NEXT VERSION!!! (NOW IT IS TOO SLOW TO DEPLOY)
	// vector<Level> tmp_vect;
	// while ( data_list.size() && n < 3 ) {
	// 	  current_layer= data_list.front();
	// 	  upg.clear();
	// 	  vector<drt> tmp_drts;
	//      vector<DrtVect> qvect= this->updateCurrentLayer(current_layer,question_);
	//      //vector<DrtVect> qvect= current_layer.getData();
	//      if (debug) {
	//           puts("LAYERS:::");
	//           cout << n << endl;
	//           for (int j = 0; j < qvect.size(); ++j) {
	//                print_vector(qvect.at(j));
	//           }
	//      }
	//      path_memory mem_tmp = current_layer.getMemory();
	//      double w=0;
	//      if(mem_tmp.getDepth() > 0)
	//           w = is_match(qvect, &upg, &tmp_drts, mem_tmp);
	//      if (debug) {
	//           cout << "L_W:::" << w << endl;
	//      }
	//      //bool add_new_clauses= true;
	// 	  if( w != 0 ) {
	// 	       mem_tmp.last_upg(upg);
	// 	       mem_tmp.last_weigth(w);
	// 	       mem_tmp.setDrt(tmp_drts);
	// 	       mem_tmp.close();
	//           //if (this->areValidRules(mem_tmp)) {
	//                //if(true)
	//           //add_new_clauses = false;
	//           mem_vect.push_back(mem_tmp);
	//           //}
	//      }
	// 	  //else {
	//      ///if(add_new_clauses) {
	//      new_feet_clauses = get_relevant_clauses(qvect);
	//      //new_feet_clauses= clauses_;
	//      //}
	//      ++n;
	// 	  set_clauses_unival(&new_feet_clauses, n+2);
	// 	  tmp_vect= launch_inference_threads(new_feet_clauses, qvect, current_layer.getMemory(), k_, 1, 10, 1, n+2);
	// 	  if( tmp_vect.size() ) {
	// 	       data_list.erase( data_list.begin() );
	// 	       tmp_vect.erase(tmp_vect.begin()+1, tmp_vect.end());
	// 	       data_list.insert(data_list.begin(), tmp_vect.begin(), tmp_vect.end() );
	// 	       sort_data(&data_list);
	// 	  }
	// 	  else {
	// 	       data_list.erase( data_list.begin() );
	// 	  }
	// }
	////// NEXT VERSION!!!

	solved_.clear();

	if (mem_vect.size()) {
		int n;
		for (n = 0; n < mem_vect.size(); ++n) {
			DrtMgu upg_tmp = mem_vect.at(n).get_first_upg();
			vector<DrtPred> solved_tmp(question_);
			solved_tmp / upg_tmp;
			double w = mem_vect.at(n).get_total_weigth();
			path_memory mem_tmp = mem_vect.at(n);
			KnowledgeAnswer ka;
			ka.setPreds(solved_tmp);
			ka.setWeigth(w);
			ka.setLink("");
			ka.setText("");
			ka.setMemory(mem_tmp);

			// Update the Question list
			QuestionList tmp_qlist(qlist_);
			tmp_qlist / upg_tmp;
			ka.setQuestionList(tmp_qlist);

			solved_.push_back(ka);
		}
	}
	//sort_data(&solved_);
}
