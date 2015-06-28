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



#include"parser.hpp"
#include"parser_aux.hpp"
#include<boost/thread.hpp>

using std::string;
using std::map;
using std::pair;
using std::make_pair;
using std::cout;
using std::endl;

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
		std::cout << (*tags_iter) << " \n";
		++tags_iter;
	}
	std::cout << std::endl;
}

template<class T>
static void print_vector_pointers(std::vector<T> &vs)
{
	typename vector<T>::iterator tags_iter = vs.begin();
	while (tags_iter < vs.end()) {
		std::cout << (*(*tags_iter)) << " \n";
		++tags_iter;
	}
	std::cout << std::endl;
}

static inline string extract_header(const Predicate &pred)
{
	return pred.pred().begin()->str;
}


static inline string extract_first_child(const Predicate &pred)
{
	return pred.pred().begin().firstChild()->str;
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

	if (find(ind_verbs.begin(), ind_verbs.end(), name) != ind_verbs.end())
		return true;

	return false;
}

string parser::find_lemma_of_verb(const string &name)
{
	vector<string> tags;
	tags.push_back("VBD");
	tags.push_back("VBN");
	tags.push_back("VBZ");

	string base = "";
	for (int n = 0; n < tags.size(); ++n) {
		base = info->get_conj(name, tags.at(n));
		if (base != "")
			break;
	}
	if (base == "")
		base = name;
	return base;
}

parser::~parser()
{
}


static double get_entropy(const vector<double> &wvect)
{
	vector<double>::const_iterator witer = wvect.begin();
	vector<double>::const_iterator wend = wvect.end();
	double wlocal = 1;
	double entropy = 0;
	for (; witer != wend; ++witer) {
		double w = *witer;
		wlocal *= w;
	}
	return wlocal;
}

static bool compare(const pair<vector<FuzzyPred>, Memory> &lhs, const pair<vector<FuzzyPred>, Memory> &rhs)
{
	double entropy_left = lhs.second.getEntropy();
	double entropy_right = rhs.second.getEntropy();
	int num_sx = lhs.first.size();
	int num_dx = rhs.first.size();

	if (num_sx != num_dx) {
		if (num_sx == 1)
			return num_sx < num_dx;
		else if (num_dx == 1)
			return num_dx < num_sx;
	}
	return entropy_left > entropy_right;
}

static void sort_data(vector<pair<vector<FuzzyPred>, Memory> > *data)
{
	sort(data->begin(), data->end(), compare);
}

static bool is_eq(const pair<vector<FuzzyPred>, Memory> lhs, const pair<vector<FuzzyPred>, Memory> rhs)
{
	std::stringstream sl, sr;
	print_vector(sl, lhs.first);
	print_vector(sr, rhs.first);
	return sl.str() == sr.str();
	//	  return lhs.second > lhs.second;
}

static void unique_data(vector<pair<vector<FuzzyPred>, Memory> > *data)
{
	for (int n = 1; n < data->size(); ++n) {
		if (is_eq(data->at(n), data->at(n - 1))) {
			data->erase(data->begin() + n);
			n = 1;
		}
	}
	// unique(data->begin(),data->end(),is_eq);
}

static inline double min(const double &a, const double &b)
{
	return (a < b) ? a : b;
}

static void insert_data(vector<pair<vector<FuzzyPred>, Memory> > *data_double, vector<pair<vector<FuzzyPred>, Memory> > data_int)
{
	vector<pair<vector<FuzzyPred>, Memory> >::iterator data_int_iter = data_int.begin();
	vector<pair<vector<FuzzyPred>, Memory> >::iterator data_int_end = data_int.end();

	int tot_int = 0;
	for (; data_int_iter != data_int_end; ++data_int_iter) {
		Memory mem_tmp(data_int_iter->second);
		//mem_tmp.add(data_int_iter->second);
		data_double->push_back(make_pair(data_int_iter->first, mem_tmp));
	}
}




static vector<pair<vector<FuzzyPred>, Memory> > launch_inference_threads(const vector<Clause> &clauses,
		const vector<FuzzyPred> &data, const Memory &mem, double noise, int num_cycles, int trace)
{
	boost::thread_group g;
	// The question
	FuzzyPred question("--");
	Inference inference0(question, data, 5, clauses, mem);
	inference0.init();
	inference0.setNoise(noise);

	inference0.makeFeetStep(num_cycles, trace);

	vector<pair<vector<FuzzyPred>, Memory> > tmpdata, retdata;
	tmpdata = inference0.getLastData();
	retdata.insert(retdata.begin(), tmpdata.begin(), tmpdata.end());

	return retdata;
}

vector<pair<vector<FuzzyPred>, Memory> > parser::apply_censor(vector<pair<vector<FuzzyPred>, Memory> > data_double)
{
	vector<pair<vector<FuzzyPred>, Memory> >::iterator diter = data_double.begin();
	vector<FuzzyPred>::iterator viter, vend;
	int d, height;

	for (; diter != data_double.end(); ++diter) {
		viter = diter->first.begin();
		vend = diter->first.end();
		for (; viter != vend; ++viter) {
			PredTree pt(viter->pred());
			if (pt.height() > 1) { /// there is a mistake in height iterator. Fix it!!!!
				PredTree::height_iterator hi(pt, 1);
				for (; hi != pt.end(); ++hi) {
					PredTree::iterator hi2(hi);
					++hi2;
					++hi2; // hi2 is at heigth 1
					while (hi2 != pt.end() && hi2.height() > 1 && hi2.firstChild() != pt.end())
						hi2 = hi2.firstChild();
					// censor a parsed tree with something like "( (VP
					// sitting in) the stadium)"
					if (hi2 != pt.end() && hi->str == "IN" && hi.parent().lastChild()->str == "IN"
							&& (hi2->str == "DT" || hi2->str == "N" || hi2->str == "EX" || hi2->str == "PRP"
									|| hi2->str == "PRP$"))
						diter->second.vanish();
					if (hi2 != pt.end() && hi->str == "TO" && hi.parent().lastChild()->str == "TO"
							&& (hi2->str == "DT" || hi2->str == "WDT" || hi2->str == "N"))
						diter->second.vanish();
					if (hi2 != pt.end() && hi->str == "DT" && hi.parent().lastChild()->str == "DT"
							&& (hi2->str == "N" || hi2->str == "J" || hi2->str == "JJS"))
						diter->second.vanish();
					if (hi2 != pt.end() && hi->str == "PRP$" && hi.parent().lastChild()->str == "PRP$"
							&& (hi2->str == "N" || hi2->str == "J" || hi2->str == "JJS"))
						diter->second.vanish();
					if (hi2 != pt.end() && hi->str == "CD" && hi.nextSibling() == pt.end() )
						diter->second.vanish();
					if (hi2 != pt.end() && hi->str == "N" && hi.nextSibling() == pt.end() && (hi2->str == "N"))
						diter->second.vanish();
				}
			}
		}
	}

	return data_double;
}

static FuzzyPred prepare_sbar(FuzzyPred pred, const FuzzyPred &trigger_pred)
{
	PredTree pt = pred.pred();
	PredTree trigger_pt = trigger_pred.pred();

	*pt.begin() = PTEl("SBAR");
	pt.appendTreeFront(pt.begin(), trigger_pt);

	return FuzzyPred(pt);
}

static FuzzyPred join_sbar(FuzzyPred root_pred, FuzzyPred sbar_pred)
// Join together a root sentence with a subordinate
{
	PredTree root_pt = root_pred.pred();
	PredTree sbar_pt = sbar_pred.pred();


	PredTree::height_iterator hi(sbar_pt, 1);
	string holder_str = hi->str;
	if (holder_str.find("-HOLDER") != string::npos) {	// If it is a "if the man calls (SBAR the dog arrives)"
		PredTree::iterator riter = sbar_pt.begin().firstChild();
		if (riter != sbar_pt.end())
			sbar_pt.erase(riter);
	}
	vector<string> orig_feet_sx = get_feet(root_pt);
	vector<string> orig_feet_dx = get_feet(sbar_pt);
	vector<string> orig_feet(orig_feet_sx);
	orig_feet.insert(orig_feet.end(),orig_feet_dx.begin(),orig_feet_dx.end());
	if (holder_str == "TO-HOLDER" || holder_str == "IN-HOLDER") {// If it is a subordinate like "he tried SBAR(to open the door)"

		PredTree::iterator riter   = root_pt.begin();
		PredTree::iterator rend    = root_pt.end();
		PredTree::iterator rattach = root_pt.end();
		++riter;
		for (; riter != rend; ++riter) {
			if (riter->str == "VP" && riter.nextSibling() == rend ) {

				PredTree::depth_iterator hi(riter);
				++hi;
				if (hi == rend)
					rattach = riter;  // attach the sbar to the last VP
			}
			if (riter->str == "-comma-" || riter->str == "SBAR" || riter->str == "PRN"
			//|| riter->str == "CC"
					)
				rattach = rend;
		}
		if (rattach != rend) {
			PredTree orig_pt= root_pt;
			root_pt.appendTree(rattach, sbar_pt);
			vector<string> new_feet = get_feet(root_pt);

			if (new_feet != orig_feet) {// check that the order of the feet is the same as the one before the corrections
				root_pt= orig_pt;
			} else {
				return FuzzyPred(root_pt);
			}
		}
	}

	root_pt.appendTree(root_pt.begin(), sbar_pt);

	return FuzzyPred(root_pt);
}

bool parser::is_sbar_trigger(const FuzzyPred &trigger_pred, const vector<FuzzyPred> &preds, vector<FuzzyPred>::iterator ipos,
		int pos)
// Check if a combination of tags triggers a subordinate
{

	if (ipos->getAttribute() == "NOSBAR")
		return false; // SBAR cannot start where nosbar_ forbids it

	if (pos == 0 || boost::next(ipos) == preds.end())
		return false;

	if ( extract_header(*ipos) == "-comma-SBAR") {
		return true;
	}

	if (boost::prior(ipos) != preds.begin() && *boost::prior(boost::prior(ipos)) == FuzzyPred("IN(since)")
			&& *boost::prior(ipos) == FuzzyPred("RB(then)")) {
		return true;
	}

	if (*ipos == FuzzyPred("IN(of)") && *boost::prior(ipos) == FuzzyPred("J(most)")) {
		return false;
	}
	if (*ipos == FuzzyPred("IN(of)")
		&& ipos != preds.begin()
		&& ipos != preds.end()
		&& boost::prior(ipos)->pred().begin()->str == "SBAR"
		&& boost::next(ipos)->pred().begin()->str == "WP"
	) {
		return true;
	}
	if (*ipos == FuzzyPred("IN(that)") && ipos != preds.end() && boost::next(ipos)->pred().begin()->str == "IN") {
		return true;
	}
	if (*ipos == FuzzyPred("IN(that)") && ipos != preds.end() && boost::next(ipos)->pred().begin()->str == "SBAR") {
		return true;
	}

	bool is_question = preds.back().pred().begin()->str == "-QM-";

	if (boost::prior(ipos)->pred().begin()->str == "V" && !is_question_
			&& (*ipos == FuzzyPred("PRP(they)") || *ipos == FuzzyPred("PRP(i)") || *ipos == FuzzyPred("PRP(he)")
					|| *ipos == FuzzyPred("PRP(she)"))) {
		return true;
	}

	if (!is_question_ && *ipos->pred().begin() == PTEl("VBN")
			&& (boost::prior(ipos)->pred().begin()->str == "N" || boost::prior(ipos)->pred().begin()->str == "J")) {
		return true;
	}
	if (*ipos == FuzzyPred("IN(if)")
			&& ( *boost::prior(ipos) == FuzzyPred("IN(that)")) ) {
		return true;
	}

	// Evidence that David is happy
	if (*ipos == FuzzyPred("IN(that)")) {
		// if there is no verb before returns false
		// check there is a verb before
		bool verb_trigger = false;
		vector<FuzzyPred>::iterator ipos2 = ipos;
		do {
			--ipos2;
			if (ipos2 != preds.begin() && boost::prior(ipos2)->pred().begin()->str == "N"
					&& ipos2->pred().begin()->str == "PRP") {
				return false;
			}
			if (ipos2->pred().begin()->str == "SBAR" || ipos2->pred().begin()->str == "PRN") {
				return false;
			}
			if (ipos2->pred().begin()->str == "V" || *ipos2->pred().begin() == PTEl("VB") // "do we want our forces to be ... ?"
			|| *ipos2->pred().begin() == PTEl("VBN") || *ipos2->pred().begin() == PTEl("VBG")
					|| *ipos2->pred().begin() == PTEl("AUX") || *ipos2->pred().begin() == PTEl("MD")) {
				verb_trigger = true;
				break;
			}
		} while (ipos2 != preds.begin());
		if (!verb_trigger)
			return false;
	}

	if (*ipos->pred().begin() == PTEl("CC")) {
		if (boost::next(ipos)->pred().begin()->str == "V"
				|| (boost::next(ipos)->pred().begin()->str == "RB" && boost::next(boost::next(ipos)) != preds.end()
						&& boost::next(boost::next(ipos))->pred().begin()->str == "V"))
			return true;
	}

	if (*ipos->pred().begin() == PTEl("CC")) {
		if (*boost::next(ipos) == FuzzyPred("PRP(she)") || *boost::next(ipos) == FuzzyPred("PRP(he)")) {
			bool verb_trigger = false;
			vector<FuzzyPred>::iterator ipos2 = ipos;
			--ipos2;
			for (; ipos2 != preds.begin(); --ipos2) {
				if (boost::prior(ipos2)->pred().begin()->str == "N" && ipos2->pred().begin()->str == "PRP") {
					return false;
				}
				if (ipos2->pred().begin()->str == "V" || *ipos2->pred().begin() == PTEl("VBN")
						|| *ipos2->pred().begin() == PTEl("VBG") || *ipos2->pred().begin() == PTEl("AUX")
						|| *ipos2->pred().begin() == PTEl("MD"))
					verb_trigger = true;
			}
			if (verb_trigger)
				return true;
		}
	}
	if (*ipos->pred().begin() == PTEl("CC")) {
		if(boost::next(ipos) != preds.end()
		   && boost::next(ipos) != preds.end()
		   && boost::next(ipos)->pred().begin()->str == "SBAR"
		   && (*boost::next(boost::next(ipos)) == FuzzyPred("PRP(she)")
		       || *boost::next(boost::next(ipos)) == FuzzyPred("PRP(he)")
		       || *boost::next(boost::next(ipos)) == FuzzyPred("PRP(they)")
		   )
		) {
				return true;
		}
	}

	if (*ipos->pred().begin() == PTEl("CC") && boost::prior(ipos) != preds.end()
			&& *boost::prior(ipos)->pred().begin() != PTEl("-comma-")
			//&& preds.size()- pos > 4
					) {
		vector<FuzzyPred>::iterator ipos2 = ipos;
		++ipos2;
		for (; ipos2 != preds.end(); ++ipos2) {
			if (*ipos2->pred().begin() == PTEl("VB"))
				return true;
			if (ipos2->pred().begin()->str != "TO")
				break;
		}
	}

	if (*ipos->pred().begin() == PTEl("CC") // "and a new government led by him came to power"
			) {
		// check there is a verb before
		bool verb_trigger = false;
		vector<FuzzyPred>::iterator ipos2 = ipos;
		do {
			--ipos2;
			if (ipos2 != preds.begin() && boost::prior(ipos2)->pred().begin()->str == "N"
					&& ipos2->pred().begin()->str == "PRP") {
				break;
			}
			if (ipos2->pred().begin()->str == "SBAR" || ipos2->pred().begin()->str == "PRN") {
				break;
			}
			if (ipos2->pred().begin()->str == "V" || *ipos2->pred().begin() == PTEl("VB") // "do we want our forces to be ... ?"
			|| *ipos2->pred().begin() == PTEl("VBN") || *ipos2->pred().begin() == PTEl("VBG")
					|| *ipos2->pred().begin() == PTEl("AUX") || *ipos2->pred().begin() == PTEl("MD")) {
				verb_trigger = true;
				break;
			}
		} while (ipos2 != preds.begin());

		if (verb_trigger) {
			// check there is a verb after
			verb_trigger = false;
			ipos2 = ipos;
			++ipos2;
			for (; ipos2 != preds.end(); ++ipos2) {
				if (boost::prior(ipos2)->pred().begin()->str == "N" && ipos2->pred().begin()->str == "PRP") {
					return false;
				}
				if (boost::prior(ipos2)->pred().begin()->str == "WDT" && ipos2->pred().begin()->str == "V") {
					return false;
				}
				if (boost::prior(ipos2)->pred().begin()->str == "TO" && ipos2->pred().begin()->str == "VB") {
					return false;
				}

				if (ipos2->pred().begin()->str == "SBAR" || ipos2->pred().begin()->str == "PRN") {
					return false;
				}
				if (ipos2->pred().begin()->str == "V" || *ipos2->pred().begin() == PTEl("VB")
						|| *ipos2->pred().begin() == PTEl("VBN") || *ipos2->pred().begin() == PTEl("VBG")
						|| *ipos2->pred().begin() == PTEl("AUX") || *ipos2->pred().begin() == PTEl("MD")) {
					if (!(*boost::prior(ipos) == FuzzyPred("J(such)"))) // "In such a case" does not trigger a SBAR
						return true;
				}
				if (ipos2->pred().begin()->str != "N" && ipos2->pred().begin()->str != "J"
						&& ipos2->pred().begin()->str != "DT" && ipos2->pred().begin()->str != "WDT"
						&& ipos2->pred().begin()->str != "WP" && ipos2->pred().begin()->str != "CD"
						&& ipos2->pred().begin()->str != "TO" && ipos2->pred().begin()->str != "CC"
						// && ipos2->pred().begin()->str != "PRN"
						// && ipos2->pred().begin()->str != "SBAR"
						&& ipos2->pred().begin()->str != "PRP" && ipos2->pred().begin()->str != "PRP$"
						// && ! (*ipos2 == FuzzyPred("IN(of)") )
						&& ipos2->pred().begin()->str != "IN")
					break;
			}
		}
	}

	if (*ipos->pred().begin() == PTEl("WRB") && ipos != preds.end() && *boost::next(ipos)->pred().begin() == PTEl("TO")
			&& (*boost::prior(ipos)->pred().begin() != PTEl("WRB"))) {
		// check there is a verb before
		bool verb_trigger = false;
		vector<FuzzyPred>::iterator ipos2 = ipos;
		do {
			--ipos2;
			if (ipos2 != preds.begin() && boost::prior(ipos2)->pred().begin()->str == "N"
					&& ipos2->pred().begin()->str == "PRP") {
				return false;
			}
			if (ipos2->pred().begin()->str == "SBAR" || ipos2->pred().begin()->str == "PRN") {
				return false;
			}
			if (ipos2->pred().begin()->str == "V" || *ipos2->pred().begin() == PTEl("VB") // "do we want our forces to be ... ?"
			|| *ipos2->pred().begin() == PTEl("VBN") || *ipos2->pred().begin() == PTEl("VBG")
					|| *ipos2->pred().begin() == PTEl("AUX") || *ipos2->pred().begin() == PTEl("MD")) {
				verb_trigger = true;
				break;
			}
		} while (ipos2 != preds.begin());
		if (verb_trigger)
			return true;
	}

	if (*ipos == FuzzyPred("TO(to)") && ipos != preds.end()
			&& ((*boost::next(ipos)->pred().begin() == PTEl("VB") || *boost::next(ipos)->pred().begin() == PTEl("VBG"))
					&& (*boost::prior(ipos)->pred().begin() != PTEl("WRB")
					))) {
		// check there is a verb before
		bool verb_trigger = false;
		vector<FuzzyPred>::iterator ipos2 = ipos;
		do {
			--ipos2;
			if (ipos2 != preds.begin() && boost::prior(ipos2)->pred().begin()->str == "N"
					&& ipos2->pred().begin()->str == "PRP") {
				return false;
			}
			if (ipos2->pred().begin()->str == "SBAR" || ipos2->pred().begin()->str == "PRN") {
				return false;
			}
			if (ipos2->pred().begin()->str == "V" || *ipos2->pred().begin() == PTEl("VB") // "do we want our forces to be ... ?"
			|| *ipos2->pred().begin() == PTEl("VBN") || *ipos2->pred().begin() == PTEl("VBG")
					|| *ipos2->pred().begin() == PTEl("AUX") || *ipos2->pred().begin() == PTEl("MD")) {
				verb_trigger = true;
				break;
			}
		} while (ipos2 != preds.begin());
		if (verb_trigger)
			return true;
	}

	if (*ipos == FuzzyPred("TO(to)")  // "... is similar to/TO what thy did ...
	&& ipos != preds.end()
			&& (*boost::next(ipos)->pred().begin() == PTEl("WP") || *boost::next(ipos)->pred().begin() == PTEl("WDT"))) {
		// check there is a verb before
		bool verb_trigger = false;
		vector<FuzzyPred>::iterator ipos2 = ipos;
		do {
			--ipos2;
			if (ipos2 != preds.begin() && boost::prior(ipos2)->pred().begin()->str == "N"
					&& ipos2->pred().begin()->str == "PRP") {
				return false;
			}
			if (ipos2->pred().begin()->str == "SBAR" || ipos2->pred().begin()->str == "PRN") {
				return false;
			}
			if (ipos2->pred().begin()->str == "V" || *ipos2->pred().begin() == PTEl("VB")
					|| *ipos2->pred().begin() == PTEl("VBN") || *ipos2->pred().begin() == PTEl("VBG")
					|| *ipos2->pred().begin() == PTEl("AUX") || *ipos2->pred().begin() == PTEl("MD")) {
				verb_trigger = true;
				break;
			}
		} while (ipos2 != preds.begin());
		if (verb_trigger)
			return true;
	}

	if (*ipos == FuzzyPred("IN(at)") && ipos != preds.end() && (*boost::next(ipos)->pred().begin() == PTEl("VBG")
			)) {
		return true;
	}

	if (pos == 1 && *boost::prior(ipos) == FuzzyPred("VBG(meaning)") // the cat is alive, meaning the decay did not happen
			) {
		return true;
	}

	if (boost::next(boost::next(ipos)) == preds.end())
		return false;

	if (*ipos->pred().begin() == PTEl("VBG") && *boost::prior(ipos)->pred().begin() == PTEl("V")) { // this justifies | abandoning
		string name = boost::prior(ipos)->pred().begin().firstChild()->str;
		/// use tagger_info!!!
		if (name != "is" && name != "be" && name != "was" && name != "am" && name != "are" && name != "were") // must not be present progressive
			return true;
	}

	// For phrases like: "the day in which there is no rain (SBAR is sunny)"
	int num_DT = 0;
	if (*ipos->pred().begin() == PTEl("V")
	//|| *ipos->pred().begin() == PTEl("VB")  // VB is taken by an auxiliary
			|| (*ipos->pred().begin() == PTEl("VBN") && !is_question_) || *ipos->pred().begin() == PTEl("MD")) {
		bool trigger_VBN = false;
		if (extract_header(*ipos) == "VBN")
			trigger_VBN = true;
		vector<FuzzyPred>::iterator ipos2 = ipos;
		--ipos2;
		for (; ipos2 != preds.begin(); --ipos2) {
			if (trigger_VBN == true && ipos2->pred().begin()->str == "VBN" && boost::next(ipos2)->pred().begin()->str != "VBN" // something like "have been | based" does not create a subordinate
					) {
				return true;
			}
			if (trigger_VBN == false
					&& (ipos2->pred().begin()->str == "V"
							|| (ipos2->pred().begin()->str == "VBN" && boost::next(ipos2)->pred().begin()->str != "VBN") // something like "have been | based" does not create a subordinate
					)) {
				return true;
			}
			if (boost::prior(ipos2)->pred().begin()->str == "N" && ipos2->pred().begin()->str == "PRP") {
				return false;
			}
			if (ipos2->pred().begin()->str != "N" && ipos2->pred().begin()->str != "J" && ipos2->pred().begin()->str != "DT"
					&& ipos2->pred().begin()->str != "PRP$" && ipos2->pred().begin()->str != "IN") {
				return false;
			}
			if (ipos2->pred().begin()->str == "DT")
				++num_DT;
			if (num_DT > 1)
				return false;
		}

		if (ipos != preds.end()
				&& (*boost::prior(ipos)->pred().begin() == PTEl("V") || *boost::prior(ipos)->pred().begin() == PTEl("VBG")
						|| (*boost::prior(ipos)->pred().begin() == PTEl("VBN") && ipos2->pred().begin()->str != "VBN" // something like "have been | based" does not create a subordinate
						)))
			return true;
	}

	if (*ipos->pred().begin() == PTEl("V"))
		if (ipos != preds.end()
				&& (*boost::prior(ipos)->pred().begin() == PTEl("V") || *boost::prior(ipos)->pred().begin() == PTEl("VBG")
						|| *boost::prior(ipos)->pred().begin() == PTEl("VBN")))
			return true;

	if (*ipos == FuzzyPred("IN(other-than)"))
		if (ipos != preds.end() && *boost::next(ipos)->pred().begin() == PTEl("TO"))
			return true;

	if ((*ipos->pred().begin() == PTEl("WDT") || *ipos->pred().begin() == PTEl("WP"))
			&& *boost::prior(ipos)->pred().begin() != PTEl("IN")) {
		vector<FuzzyPred>::iterator ipos2 = ipos;
		++ipos2;
		for (; ipos2 != preds.end(); ++ipos2) {
			if (*ipos2->pred().begin() == PTEl("MD") || *ipos2->pred().begin() == PTEl("V")
					|| *ipos2->pred().begin() == PTEl("VBP") || *ipos2->pred().begin() == PTEl("VBZ")
					|| *ipos2->pred().begin() == PTEl("VBD"))
				return true;
			if (ipos2->pred().begin()->str != "N" && ipos2->pred().begin()->str != "J" && ipos2->pred().begin()->str != "DT"
					&& ipos2->pred().begin()->str != "RB" && ipos2->pred().begin()->str != "PRP"
					&& ipos2->pred().begin()->str != "PRP$")
				return false;
		}
	}

	if (*ipos->pred().begin() == PTEl("N")) // "in which (SBAR Britain has won the medals)"
		if (ipos != preds.end() && (*boost::next(ipos)->pred().begin() == PTEl("V"))
				&& (*boost::prior(ipos)->pred().begin() == PTEl("WDT") || *boost::prior(ipos)->pred().begin() == PTEl("WP"))
				&& pos > 1
				&& (*boost::prior(boost::prior(ipos))->pred().begin() == PTEl("IN")
						|| *boost::prior(boost::prior(ipos))->pred().begin() == PTEl("TO"))
				&& boost::prior(ipos)->pred().begin().firstChild()->str == "which") {
			return true;
		}

	if (*ipos->pred().begin() == PTEl("DT") || *ipos->pred().begin() == PTEl("PRP")) // For a phrase like: if a man calls his dog the animal comes in the house.
		if (ipos != preds.end()
				&& (*boost::prior(ipos)->pred().begin() == PTEl("N") || *boost::prior(ipos)->pred().begin() == PTEl("J")
						|| *boost::prior(ipos)->pred().begin() == PTEl("CD"))) {
			// check there is a verb before
			bool verb_trigger = false;
			vector<FuzzyPred>::iterator ipos2 = ipos;
			do {
				--ipos2;
				if (ipos2 != preds.begin() && boost::prior(ipos2)->pred().begin()->str == "N"
						&& ipos2->pred().begin()->str == "PRP") {
					return false;
				}
				if (ipos2->pred().begin()->str == "SBAR" || ipos2->pred().begin()->str == "PRN") {
					return false;
				}
				if (ipos2->pred().begin()->str == "V" || *ipos2->pred().begin() == PTEl("VB") // "do we want our forces to be ... ?"
				|| *ipos2->pred().begin() == PTEl("VBN") || *ipos2->pred().begin() == PTEl("VBG")
						|| *ipos2->pred().begin() == PTEl("AUX") || *ipos2->pred().begin() == PTEl("MD")) {
					string verb_str = extract_first_child(*ipos2);
					string base = find_lemma_of_verb(verb_str);
					if (!verb_supports_indirect_obj(base) && !(*boost::prior(ipos) == FuzzyPred("J(such)"))) // "In such a case" does not trigger a SBAR
						verb_trigger = true;
					break;
				}
			} while (ipos2 != preds.begin());
			if (!verb_trigger)
				return false;

			// check there is a verb after
			ipos2 = ipos;
			++ipos2;
			for (; ipos2 != preds.end(); ++ipos2) {
				if (boost::prior(ipos2)->pred().begin()->str == "N" && ipos2->pred().begin()->str == "PRP") {
					return false;
				}
				if (boost::prior(ipos2)->pred().begin()->str == "TO" && ipos2->pred().begin()->str == "VB") {
					return false;
				}
				if (ipos2->pred().begin()->str == "V" || *ipos2->pred().begin() == PTEl("VB")
						|| *ipos2->pred().begin() == PTEl("VBN") || *ipos2->pred().begin() == PTEl("VBG")
						|| *ipos2->pred().begin() == PTEl("AUX") || *ipos2->pred().begin() == PTEl("MD")) {
					if (!(*boost::prior(ipos) == FuzzyPred("J(such)"))) // "In such a case" does not trigger a SBAR
						return true;
				}
				if (ipos2->pred().begin()->str != "N" && ipos2->pred().begin()->str != "J"
						&& ipos2->pred().begin()->str != "DT" && ipos2->pred().begin()->str != "WDT"
						&& ipos2->pred().begin()->str != "WP" && ipos2->pred().begin()->str != "CD"
						&& ipos2->pred().begin()->str != "TO" && ipos2->pred().begin()->str != "CC"
						&& ipos2->pred().begin()->str != "PRN" && ipos2->pred().begin()->str != "PRP"
						&& ipos2->pred().begin()->str != "PRP$"
						// && ! (*ipos2 == FuzzyPred("IN(of)") )
						&& ipos2->pred().begin()->str != "IN")
					break;
			}
		}

	if (*ipos == FuzzyPred("IN(that)") || *ipos == FuzzyPred("IN(if)") || *ipos == FuzzyPred("IN(because)")
			|| *ipos == FuzzyPred("IN(whether)")
			|| *ipos == FuzzyPred("IN(so)")
			|| *ipos->pred().begin() == PTEl("WRB")) {
		if (ipos != preds.end()
				&& (*boost::next(ipos)->pred().begin() == PTEl("DT") || *boost::next(ipos)->pred().begin() == PTEl("PDT")
						|| *boost::next(ipos)->pred().begin() == PTEl("CD")
						|| *boost::next(ipos)->pred().begin() == PTEl("EX")
						|| *boost::next(ipos)->pred().begin() == PTEl("PRP")
						|| *boost::next(ipos)->pred().begin() == PTEl("PRP$")
						|| *boost::next(ipos)->pred().begin() == PTEl("N")
						|| *boost::next(ipos)->pred().begin() == PTEl("WP")
						|| *boost::next(ipos)->pred().begin() == PTEl("J")
						|| *boost::next(ipos)->pred().begin() == PTEl("V")
						|| *boost::next(ipos)->pred().begin() == PTEl("VBN")
						|| *boost::next(ipos)->pred().begin() == PTEl("VBG") // when singing
						|| *boost::next(ipos)->pred().begin() == PTEl("MD")
						|| *boost::next(ipos)->pred().begin() == PTEl("RB") || *boost::next(ipos) == FuzzyPred("IN(while)")
						|| *boost::next(ipos) == FuzzyPred("IN(in)") || *boost::next(ipos) == FuzzyPred("IN(at)"))) {
			return true;
		}
	}

	if ((*ipos->pred().begin() == PTEl("CC") && *boost::prior(ipos)->pred().begin() == PTEl("-comma-"))
			|| (*ipos->pred().begin() == PTEl("CC") && !(*boost::next(ipos) == FuzzyPred("IN(of)")))
			|| (*ipos->pred().begin() == PTEl("CC") && *boost::next(ipos)->pred().begin() == PTEl("PRP$"))) {
		// check there is a verb before
		bool verb_trigger = false;
		vector<FuzzyPred>::iterator ipos2 = ipos;
		do {
			--ipos2;
			if (ipos2 != preds.begin() && boost::prior(ipos2)->pred().begin()->str == "N"
					&& ipos2->pred().begin()->str == "PRP") {
				return false;
			}
			if (ipos2->pred().begin()->str == "SBAR" || ipos2->pred().begin()->str == "PRN") {
				return false;
			}
			if (ipos2->pred().begin()->str == "V" || *ipos2->pred().begin() == PTEl("VB") // "do we want our forces to be ... ?"
			|| *ipos2->pred().begin() == PTEl("VBN") || *ipos2->pred().begin() == PTEl("VBG")
					|| *ipos2->pred().begin() == PTEl("AUX") || *ipos2->pred().begin() == PTEl("MD")) {
				verb_trigger = true;
				break;
			}
		} while (ipos2 != preds.begin());
		if (!verb_trigger)
			return false;

		// check there is a verb after
		ipos2 = ipos;
		++ipos2;
		for (; ipos2 != preds.end(); ++ipos2) {
			if (boost::prior(ipos2)->pred().begin()->str == "N" && ipos2->pred().begin()->str == "PRP") {
				return false;
			}
			if (boost::prior(ipos2)->pred().begin()->str == "TO" && ipos2->pred().begin()->str == "VB") {
				return false;
			}
			if (ipos2->pred().begin()->str == "V" || *ipos2->pred().begin() == PTEl("VB")
					|| *ipos2->pred().begin() == PTEl("VBN") || *ipos2->pred().begin() == PTEl("VBG")
					|| *ipos2->pred().begin() == PTEl("AUX") || *ipos2->pred().begin() == PTEl("MD"))
				return true;
			if (ipos2->pred().begin()->str != "N" && ipos2->pred().begin()->str != "J" && ipos2->pred().begin()->str != "DT"
					&& ipos2->pred().begin()->str != "WDT" && ipos2->pred().begin()->str != "WP"
					&& ipos2->pred().begin()->str != "CD" && ipos2->pred().begin()->str != "TO"
					&& ipos2->pred().begin()->str != "CC" && ipos2->pred().begin()->str != "RB"
					&& ipos2->pred().begin()->str != "PRN" && ipos2->pred().begin()->str != "SBAR"
					&& ipos2->pred().begin()->str != "EX" && ipos2->pred().begin()->str != "PRP"
					&& ipos2->pred().begin()->str != "PRP$"
					// && ! (*ipos2 == FuzzyPred("IN(of)") )
					&& ipos2->pred().begin()->str != "IN")
				break;
		}
	}

	if (*ipos->pred().begin() == PTEl("N") // "SBAR evidence that this will happen"
	&& *boost::next(ipos) == FuzzyPred("IN(that)") && *boost::prior(ipos)->pred().begin() == PTEl("SBAR")) {
		// check there is a verb after
		vector<FuzzyPred>::iterator ipos2 = ipos;
		ipos2 = ipos;
		++ipos2;
		for (; ipos2 != preds.end(); ++ipos2) {
			if (boost::prior(ipos2)->pred().begin()->str == "N" && ipos2->pred().begin()->str == "PRP") {
				return false;
			}
			if (ipos2->pred().begin()->str == "V" || *ipos2->pred().begin() == PTEl("VB")
					|| *ipos2->pred().begin() == PTEl("VBN") || *ipos2->pred().begin() == PTEl("VBG")
					|| *ipos2->pred().begin() == PTEl("AUX") || *ipos2->pred().begin() == PTEl("MD"))
				return true;
			if (ipos2->pred().begin()->str != "N" && ipos2->pred().begin()->str != "J" && ipos2->pred().begin()->str != "DT"
					&& ipos2->pred().begin()->str != "WDT" && ipos2->pred().begin()->str != "WP"
					&& ipos2->pred().begin()->str != "CD" && ipos2->pred().begin()->str != "TO"
					&& ipos2->pred().begin()->str != "CC" && ipos2->pred().begin()->str != "RB"
					&& ipos2->pred().begin()->str != "PRN" && ipos2->pred().begin()->str != "SBAR"
					&& ipos2->pred().begin()->str != "EX" && ipos2->pred().begin()->str != "PRP"
					&& ipos2->pred().begin()->str != "PRP$"
					// && ! (*ipos2 == FuzzyPred("IN(of)") )
					&& ipos2->pred().begin()->str != "IN")
				break;
		}
	}

	if (*ipos == FuzzyPred("IN(after)")
			&& (*boost::next(ipos) == FuzzyPred("PRP(he)") || *boost::next(ipos) == FuzzyPred("PRP(she)")
					|| *boost::next(ipos) == FuzzyPred("PRP(they)"))) {
		// these prepositions always introduce a verb
		return true;
	}

	if ((!(trigger_pred == FuzzyPred("IN-HOLDER(*)")) // He said that the warriors from Mars stopped
	|| boost::next(ipos)->pred().begin()->str == "WP"
	|| *ipos == FuzzyPred("IN(by)") || *ipos == FuzzyPred("IN(before)") || *ipos == FuzzyPred("IN(without)") || *ipos == FuzzyPred("IN(since)")
			) && trigger_pred.pred().begin()->str != "WRB" // i know why the man on the roof is unhappy
	&& !(*ipos == FuzzyPred("IN(of)") && boost::prior(ipos)->pred().begin()->str == "CD") // 1 of many
			&& !(*ipos == FuzzyPred("IN(of)") && boost::prior(ipos)->pred().begin()->str == "DT") // some of the men
			&& (extract_header(*ipos) == "IN" // any other IN can introduce a verb
			)
	) {
		bool verb_trigger = true, aux_trigger = false;

		// check there is a verb before
		//if ((trigger_pred == FuzzyPred("DummyPred")))  // if it is the main sentence
		{
			verb_trigger = false;

			vector<FuzzyPred>::iterator ipos2 = ipos;

			do {
				--ipos2;
				if (ipos2 != preds.begin() && boost::prior(ipos2)->pred().begin()->str == "N"
						&& ipos2->pred().begin()->str == "PRP") {
					return false;
				}
				if (ipos2->pred().begin()->str == "SBAR" || ipos2->pred().begin()->str == "PRN") {
					return false;
				}
				if (ipos2->pred().begin()->str == "V" || *ipos2->pred().begin() == PTEl("VBN")
						|| *ipos2->pred().begin() == PTEl("VB") || *ipos2->pred().begin() == PTEl("VBG")
						|| *ipos2->pred().begin() == PTEl("AUX") || *ipos2->pred().begin() == PTEl("MD")) {
					string verb_str = extract_first_child(*ipos2);
					if (verb_str == "am" || verb_str == "be" || verb_str == "is" || verb_str == "are" || verb_str == "was"
							|| verb_str == "were" || verb_str == "has" || verb_str == "have" || verb_str == "had") {
						aux_trigger = true;
					}
					verb_trigger = true;
					break;
				}
			} while (ipos2 != preds.begin());
			if (!verb_trigger)
				return false;
		}

		// check there is a verb after
		vector<FuzzyPred>::iterator ipos2 = ipos;
		++ipos2;
		for (; ipos2 != preds.end(); ++ipos2) {
			if (debug)
				cout << "i:::" << *ipos2 << endl;
			if (boost::prior(ipos2)->pred().begin()->str == "N" && ipos2->pred().begin()->str == "PRP") {
				return false;
			}
			if (boost::prior(ipos2)->pred().begin()->str == "TO" && ipos2->pred().begin()->str == "VB") {
				return false;
			}
			if (boost::prior(ipos2)->pred().begin()->str == "WP" && *ipos2 == FuzzyPred("WDT([prior_WP])")) {
				return false;
			}
			if (ipos2->pred().begin()->str == "V" || (!aux_trigger && *ipos2->pred().begin() == PTEl("VBN"))
					|| *ipos2->pred().begin() == PTEl("VBG") || *ipos2->pred().begin() == PTEl("AUX")
					|| *ipos2->pred().begin() == PTEl("MD")) {
				if (debug)
					cout << "i2:::" << *ipos2 << endl;
				return true;
			}
			if (ipos2->pred().begin()->str != "N" && ipos2->pred().begin()->str != "J" && ipos2->pred().begin()->str != "WP"
					&& ipos2->pred().begin()->str != "DT" && ipos2->pred().begin()->str != "CD"
					&& ipos2->pred().begin()->str != "WDT" && ipos2->pred().begin()->str != "PRN"
					&& ipos2->pred().begin()->str != "PRP" && ipos2->pred().begin()->str != "PRP$"
					&& ipos2->pred().begin()->str != "RB" && !(*ipos2 == FuzzyPred("IN(of)")))
				return false;
		}
	}

	if (*ipos->pred().begin() == PTEl("-comma-") && preds.size() - pos > 3) {
		vector<FuzzyPred>::iterator ipos2 = ipos;
		++ipos2;
		for (; ipos2 != preds.end(); ++ipos2) { // check it is not "name, name, and name"
			if (ipos2->pred().begin()->str == "CC")
				return false;
			else if (ipos2->pred().begin()->str != "N" && ipos2->pred().begin()->str != "J"
					&& ipos2->pred().begin()->str != "CD" && ipos2->pred().begin()->str != "-comma-") {
				return true;
			}
		}
		return true;
	}

	if (*ipos == FuzzyPred("IN(of)") && ipos != preds.end()) {
		if (*boost::next(ipos)->pred().begin() == PTEl("VBG")
				|| (*boost::next(ipos)->pred().begin() == PTEl("WDT") && *boost::prior(ipos)->pred().begin() != PTEl("CD") // "one of which" does not trigger SBAR
				&& *boost::prior(ipos)->pred().begin() != PTEl("J") // "biggest of which" does not trigger SBAR
				) || *boost::next(ipos)->pred().begin() == PTEl("WHNP"))
			return true;
		vector<FuzzyPred>::iterator ipos2 = ipos;
		++ipos2;
		if (*ipos2->pred().begin() == PTEl("N") && *boost::next(ipos2)->pred().begin() == PTEl("VBG"))
			return true;
	}

	if ((*ipos == FuzzyPred("IN(by)") || *ipos == FuzzyPred("IN(in)") || *ipos == FuzzyPred("IN(for)")
			|| *ipos == FuzzyPred("IN(from)") || *ipos == FuzzyPred("IN(about)")) && ipos != preds.end()) {
		if (*boost::next(ipos)->pred().begin() == PTEl("VBG") || *boost::next(ipos)->pred().begin() == PTEl("WDT")
				|| *boost::next(ipos)->pred().begin() == PTEl("WHNP"))
			return true;
		vector<FuzzyPred>::iterator ipos2 = ipos;
		++ipos2;
		if (*ipos2->pred().begin() == PTEl("N") && *boost::next(ipos2)->pred().begin() == PTEl("VBG"))
			return true;
	}

	num_DT = 0;
	if (*boost::prior(ipos)->pred().begin() == PTEl("V") // "a man sleeps / the way he wants"
	&& !is_question) {
		vector<FuzzyPred>::iterator ipos2 = ipos;
		for (; ipos2 != preds.end(); ++ipos2) {
			if (boost::prior(ipos2)->pred().begin()->str == "N" && ipos2->pred().begin()->str == "PRP") {
				return false;
			}
			if (ipos2->pred().begin()->str == "V")
				return true;
			if (ipos2->pred().begin()->str != "N" && ipos2->pred().begin()->str != "J" && ipos2->pred().begin()->str != "DT"
					&& ipos2->pred().begin()->str != "PRP" && ipos2->pred().begin()->str != "PRP$")
				return false;
			if (ipos2->pred().begin()->str == "DT")
				++num_DT;
			if (num_DT > 1)
				return false;
		}
	}

	return false;
}

static bool is_prp_sbar(const vector<FuzzyPred> &preds, vector<FuzzyPred>::iterator ipos, int pos)
{
	if (extract_header(*ipos) == "PRP")
		return true;
	return false;
}

static bool is_article_sbar(const vector<FuzzyPred> &preds, vector<FuzzyPred>::iterator ipos, int pos)
{
	if (extract_header(*ipos) == "DT")
		return true;
	return false;
}

static bool is_WP_sbar(const vector<FuzzyPred> &preds, vector<FuzzyPred>::iterator ipos, int pos)
{
	if (extract_header(*ipos) == "WP")
		return true;
	return false;
}

static bool is_IN_sbar(const vector<FuzzyPred> &preds, vector<FuzzyPred>::iterator ipos, int pos)
{
	if (extract_header(*ipos) == "IN")
		return true;
	return false;
}
static bool is_TO_sbar(const vector<FuzzyPred> &preds, vector<FuzzyPred>::iterator ipos, int pos)
{
	if (extract_header(*ipos) == "TO")
		return true;
	return false;
}
static bool is_N_sbar(const vector<FuzzyPred> &preds, vector<FuzzyPred>::iterator ipos, int pos)
{
	if (extract_header(*ipos) == "N" || extract_header(*ipos) == "CD")
		return true;
	return false;
}
static bool is_V_sbar(const vector<FuzzyPred> &preds, vector<FuzzyPred>::iterator ipos, int pos)
{
	if (extract_header(*ipos) == "V" || extract_header(*ipos) == "VBP" || extract_header(*ipos) == "VBD"
			|| extract_header(*ipos) == "VBN")
		return true;
	return false;
}

static bool is_to_insert(const vector<FuzzyPred> &preds, vector<FuzzyPred>::iterator ipos, int pos)
// Checks if the word must inserted in "subvector". For example, you
// might have a IN(*) at the end of a phrase that belongs to the
// prevector.
{
	if (is_prp_sbar(preds, ipos, pos))
		return true;
	if (is_article_sbar(preds, ipos, pos))
		return true;
	if (is_WP_sbar(preds, ipos, pos))
		return true;
	if (is_IN_sbar(preds, ipos, pos))
		return true;
	if (is_TO_sbar(preds, ipos, pos))
		return true;
	if (is_N_sbar(preds, ipos, pos))
		return true;
	if (is_V_sbar(preds, ipos, pos))
		return true;
	return false;
}

static inline int min(int a, int b)
{
	return (a < b) ? a : b;
}

static FuzzyPred add_possdt(FuzzyPred parsed)
// add a possdt dummy tag for parsed trees like (NP (NP (DT the) (N cat) (POS 's)) (N hat)) -> (NP (POSSDT (NP (DT the) (N cat) ) (POS 's)) (N hat ) )
{
	PredTree pt = parsed.pred();
	PredTree::iterator piter = pt.begin();
	PredTree::iterator pend = pt.end();
	++piter;
	for (; piter != pend; ++piter) {
		if (piter->str == "POS" && piter.parent().lastChild()->str == "POS" // POS can be also at the beginning of the subtree
		&& piter.node->parent && piter.parent()->str == "NP"
		//&& piter.nextSibling() != pend
		//&& piter.nextSibling()->str == "N"
				) {
			PredTree np_tmp(*piter.node->parent); // Extract the parent NP
			PredTree pos_tmp(*np_tmp.begin().node->lastChild); // Saving the POS('s)
			np_tmp.erase(*np_tmp.begin().node->lastChild);
			piter = piter.parent();
			pt.cut(piter);
			pt.appendTree(piter, np_tmp);
			pt.appendTree(piter, pos_tmp);
			*piter = PTEl("POSSDT");
			piter = pt.begin();
			pend = pt.end();
			++piter;
		}
	}
	return FuzzyPred(pt);
}

static FuzzyPred which_treatment(FuzzyPred parsed)
/// Unfinished
{
	PredTree pt = parsed.pred();
	PredTree::iterator piter = pt.begin();
	PredTree::iterator pend = pt.end();
	PredTree::iterator which_sbar;
	++piter;
	for (; piter != pend; ++piter) {
		if (*piter == PTEl("WDT") && piter.parent() != pt.end()
				&& (piter.parent()->str == "S" || piter.parent()->str == "SBAR")) {
			which_sbar = piter.parent();
		} else if (*piter == PTEl("WDT") && piter.parent() != pt.end() && piter.parent().parent() != pt.end()
				&& (piter.parent().parent()->str == "S" || piter.parent()->str == "SBAR")) {
			which_sbar = piter.parent().parent();
		}
	}

	return FuzzyPred(pt);
}

FuzzyPred restore_original_tags(FuzzyPred parsed, const vector<FuzzyPred> tagged)
// Substitute the simplified tags with the original one (J -> JJS or JJR or JJ)
{
	PredTree pt(parsed.pred());
	PredTree::height_iterator piter(pt, 1);
	vector<FuzzyPred>::const_iterator tagiter = tagged.begin();
	for (; piter != pt.end() && tagiter != tagged.end(); ++piter, ++tagiter) {
		if (piter->str == "CD-DATE") {
			piter->str = "CD";
			string fstr = piter.firstChild()->str;
			if (fstr.find("[date]") == string::npos) {
				piter.firstChild()->str = (string) "[date]_" + fstr;
			}
		} else if (piter->str == "-comma-SBAR") {
			piter->str = "-comma-";
		} else
			piter->str = extract_header(*tagiter);
	}

	return FuzzyPred(pt);
}

bool parser::is_closing_nested(vector<FuzzyPred>::iterator ipos, vector<FuzzyPred>::iterator iter_begin, vector<FuzzyPred> &preds,
		const FuzzyPred &open_item)
// Check if the tag in "ipos" is the closing tag of a nested subordinate
{
	if (debug)
		cout << "ISQ:::" << std::boolalpha << is_question_ << " " << *ipos << std::flush << endl;
	if (boost::next(ipos) == preds.end() )
		return false;

	if (is_question_
		&& (open_item.pred().begin()->str == "WP" || open_item.pred().begin()->str == "WDT"	)
		&& open_item == preds.front()
	)
		return false;

	if (debug)
		cout << "ISQ2:::" << std::boolalpha << is_question_ << std::flush << endl;
	if (*ipos == FuzzyPred("IN(that)") || *ipos == FuzzyPred("WDT(that)"))
		return false;
	if (*ipos == FuzzyPred("-comma-(-END-PRN-)"))
		return true;
	if (*ipos->pred().begin() == PTEl("VB") && boost::prior(ipos) != preds.end()
			&& ( *boost::next(ipos)->pred().begin() == PTEl("VB") || *boost::next(ipos)->pred().begin() == PTEl("V"))
	) {
		return true;
	}
	if (*boost::next(ipos) == FuzzyPred("-period-(-period-)") || *boost::next(ipos) == FuzzyPred("-QM-(-period-)"))
		return false;
	if ( !is_question_ && (boost::next(ipos)->pred().begin()->str == "VB" || extract_header(*ipos) == "MD" ) )
		return false;
	if ( !is_question_ && (boost::next(ipos)->pred().begin()->str == "VB" || extract_header(*ipos) == "MD" ) )
		return false;
	vector<FuzzyPred>::iterator iorig(ipos);
	if (debug)
		cout << "ORIG:::: " << open_item.pred().begin()->str << endl;

	// these PRP as next word can close a sentence
	if(boost::next(ipos) != preds.end() ) {
		string prior_tag = boost::next(ipos)->pred().begin()->str;
		string prior_head = boost::next(ipos)->pred().begin().firstChild()->str;

		if (( (prior_tag == "PRP" && prior_head == "he") || (prior_tag == "PRP" && prior_head == "she")
			|| (prior_tag == "PRP" && prior_head == "they")
			|| (prior_tag == "PRP" && prior_head == "we") )
		)
			if(open_item == FuzzyPred("IN(before)") )
				return true;
	}

	// these PRP cannot close a sentence
	string prior_tag = (ipos)->pred().begin()->str;
	string prior_head = (ipos)->pred().begin().firstChild()->str;
	if ((prior_tag == "PRP" && prior_head == "he") || (prior_tag == "PRP" && prior_head == "she")
			|| (prior_tag == "PRP" && prior_head == "they")
			|| (prior_tag == "PRP" && prior_head == "we")
	)
		return false;
	if(ipos != preds.begin() ) {
		string prior_tag = boost::prior(ipos)->pred().begin()->str;
		string prior_head = boost::prior(ipos)->pred().begin().firstChild()->str;
		string tag = (ipos)->pred().begin()->str;
		if ( tag == "RB"
			&& ( (prior_tag == "PRP" && prior_head == "he") || (prior_tag == "PRP" && prior_head == "she")
				 || (prior_tag == "PRP" && prior_head == "they")
				 || (prior_tag == "PRP" && prior_head == "we") )
		)
			return false;
	}

	ipos = iorig;
	// "to make it work" -> PRN(to make it) work/V is not acceptable
	if (open_item.pred().begin()->str == "TO" && boost::next(ipos)->pred().begin()->str == "V"
			&& ipos != preds.begin()) {
		// check it has a verb before
		bool verb_trigger = false;
		vector<FuzzyPred>::iterator ipos2 = ipos;
		do {
			--ipos2;
			if (ipos2 != preds.begin() && boost::prior(ipos2)->pred().begin()->str == "N"
					&& ipos2->pred().begin()->str == "PRP") {
				break;
			}
			if (ipos2->pred().begin()->str == "SBAR" || ipos2->pred().begin()->str == "PRN") {
				break;
			}
			if (ipos2->pred().begin()->str == "V" || *ipos2->pred().begin() == PTEl("VB") // "do we want our forces to be ... ?"
					|| *ipos2->pred().begin() == PTEl("VBN") || *ipos2->pred().begin() == PTEl("VBG")
					|| *ipos2->pred().begin() == PTEl("AUX") || *ipos2->pred().begin() == PTEl("MD")) {
				verb_trigger = true;
				break;
			}
		} while (ipos2 != preds.begin());
		if(verb_trigger && ipos2 != preds.end()) {
			string header= extract_first_child(*ipos2);
			if(header == "make"
			   || header == "let"
					) {
				return false;
			}
		}
	}

	ipos = iorig;
	bool verb_trigger = false, opening_trigger = false;
	if (*ipos == FuzzyPred("-comma-(-comma-)")) {
		if (debug)
			cout << "COMMA2::: " << open_item << endl;
		if (open_item.pred().begin()->str != "-comma-"
				&& !(open_item == FuzzyPred("IN(if)") || open_item == FuzzyPred("IN(because)")
						|| open_item == FuzzyPred("IN(after)") || open_item == FuzzyPred("IN(as)")
						|| open_item == FuzzyPred("WDT(which)") || open_item == FuzzyPred("WDT(that)")
						|| open_item == FuzzyPred("WDT([prior_WP])")))
			return false; // only a comma or a conditional phrase can close another comma

		if (boost::next(ipos) != preds.end() && *boost::next(ipos) == FuzzyPred("RB(then)")
				&& open_item == FuzzyPred("IN(if)")) {
			return true; // "If you do this, then I will do this"
		}
		if (debug)
			cout << "COMMA3::: " << open_item << endl;

		// if there is another comma after this one, and there is no
		// verb within this comma and the next one, return false
		++ipos;
		for (; ipos != preds.end(); ++ipos) {
			if (extract_header(*ipos) == "-comma-" || extract_header(*ipos) == "-period-") {
				vector<FuzzyPred>::iterator ipos2 = ipos;
				--ipos2;
				for (; ipos2 != iorig; --ipos2) {
					if (debug)
						cout << "COMMA4::: " << *ipos2 << endl;
					if (ipos2->pred().begin()->str == "V" || ipos2->pred().begin()->str == "MD"
							|| ipos2->pred().begin()->str == "VBN")
						return true;
				}
				return false;
			}
			if (extract_header(*ipos) == "WDT")
				return false; // "he did this, and then he did that, which was good"
		}

		return true;
	}

	ipos = iorig;
	if (extract_header(*ipos) == "VB" && boost::next(ipos) != preds.end()
			&& (boost::next(ipos)->pred().begin()->str == "VBG" || boost::next(ipos)->pred().begin()->str == "TO")) {
		return false;
	}

	if (boost::next(ipos) != preds.end() && *boost::next(ipos) == FuzzyPred("RB(then)") && open_item == FuzzyPred("IN(if)"))
		return true; // A conditional phrase

	if (boost::next(ipos) != preds.end() && (*ipos == FuzzyPred("DT(a)") || *ipos == FuzzyPred("DT(the)"))
			&& (boost::next(ipos)->pred().begin()->str == "V" || boost::next(ipos)->pred().begin()->str == "VBG"
					|| boost::next(ipos)->pred().begin()->str == "VBN"))
		return false;  // it makes no sense to have "(PRN what is a) part/V"

	if ((open_item == FuzzyPred("IN(if)") || open_item == FuzzyPred("IN(as)")) && extract_header(*ipos) == "N"
			&& boost::next(ipos) != preds.end()
			&& (boost::next(ipos)->pred().begin()->str == "DT" || boost::next(ipos)->pred().begin()->str == "PRP$")) {
		return true;
	}
	ipos = iorig;
	if ((open_item == FuzzyPred("IN(if)")) && boost::next(ipos) != preds.end() && boost::next(ipos)->pred().begin()->str == "PRP"
			&& ipos != preds.begin()) {
		string prp_str = boost::next(ipos)->pred().begin().firstChild()->str;
		if (prp_str == "i" || prp_str == "he" || prp_str == "she" || prp_str == "we" || prp_str == "they") {
			vector<FuzzyPred>::iterator ipos2 = ipos;
			--ipos2;
			for (; ipos2 != preds.begin(); --ipos2) {
				if (*ipos2 == *boost::next(ipos)) // "If we decide we will conclude"
					return true;
			}
		}
	}
	ipos = iorig;
	if ((open_item == FuzzyPred("IN(if)") // "if someone smiles he is happy"
	) && boost::next(ipos) != preds.end() && boost::next(ipos)->pred().begin()->str == "PRP" && ipos != preds.begin()) {
		string prp_str = boost::next(ipos)->pred().begin().firstChild()->str;
		string prior_tag = extract_header(*ipos);
		if ((prp_str == "i" || prp_str == "he" || prp_str == "she" || prp_str == "we" || prp_str == "they")
				&& (prior_tag == "V" || prior_tag == "VB" || prior_tag == "VBN")) {
			return true;
		}
	}
	ipos = iorig;
	if (boost::next(ipos) != preds.end() && *boost::next(ipos) == FuzzyPred("IN(if)")
			&& (open_item.pred().begin()->str == "N" || open_item.pred().begin()->str == "DT"
					|| open_item.pred().begin()->str == "J" || open_item.pred().begin()->str == "PRP"
					|| open_item.pred().begin()->str == "PRP$" || open_item.pred().begin()->str == "RB"))
		return true; // "I will go there to swim if I feel better"

	if (open_item.pred().begin()->str == "-comma-" && !(*ipos == FuzzyPred("-comma-(-comma-)")))
		return false; // only a comma or a conditional phrase can close another comma

	// Check if this a verb after a CC
	ipos = iorig;
	vector<FuzzyPred>::iterator ipos2 = ipos;
	--ipos2;
	bool CC_trigger = false;
	for (; ipos2 != iter_begin; --ipos2) {
		if (ipos2->pred().begin()->str == "CC") {
			CC_trigger = true;
			break;
		}
		if (ipos2->pred().begin()->str != "RB" && ipos2->pred().begin()->str != "RP" && ipos2->pred().begin()->str != "UH") {
			CC_trigger = false;
			break;
		}
	}

	// For phrases like: "the day in which there is no rain (SBAR is sunny)"
	ipos = iorig;
	++ipos;
	if (ipos == preds.end())
		return false;
	if (*ipos->pred().begin() == PTEl("MD") && boost::prior(ipos) != preds.end()
			&& *boost::prior(ipos)->pred().begin() == PTEl("VB")
	) {
		return true;
	}

	// For phrases like: "the gym that we go to is nice"
	ipos = iorig;
	++ipos;
	if (ipos == preds.end())
		return false;

	if ( (open_item.pred().begin()->str == "WDT" || open_item.pred().begin()->str == "WP" )
		&& boost::prior(ipos) != preds.end()
		&& *boost::prior(ipos)->pred().begin() == PTEl("IN")
		&& !( *boost::prior(ipos) == FuzzyPred("IN(that)") )
		&& !( *boost::prior(ipos) == FuzzyPred("IN(if)") )
		&& *ipos->pred().begin() == PTEl("V")
	) {
		return true;
	}


	/// Check that after the trigger there is a verb before the comma or "then" (for if-phrases)
	ipos = iorig;
	for (; ipos != preds.end(); ++ipos) {
		if (extract_header(*ipos) == "V" || extract_header(*ipos) == "VBG" || extract_header(*ipos) == "VB"
				|| extract_header(*ipos) == "MD")
			break;
		if (extract_header(*ipos) == "-comma-" || (open_item == FuzzyPred("IN(if)") && *ipos == FuzzyPred("RB(then)")))
			return false;
	}
	if (debug)
		cout << "Opening22::: " << open_item << ", " << *iorig << endl;
	if (extract_header(*iorig) == "CC")
		return false;

	ipos = iorig;

	if ((*boost::next(ipos)->pred().begin() == PTEl("V") || *boost::next(ipos)->pred().begin() == PTEl("VB"))
			&& *ipos->pred().begin() == PTEl("V"))
		return true;

	bool THAT_trigger = false;
	if (ipos != iter_begin
			&& (*boost::prior(ipos) == FuzzyPred("IN(that)") || *boost::prior(ipos) == FuzzyPred("WDT(that)")
					|| *boost::prior(ipos) == FuzzyPred("WDT(which)") || *ipos == FuzzyPred("IN(that)")
					|| *ipos == FuzzyPred("WDT(that)") || *ipos == FuzzyPred("WDT(which)")
					|| extract_header(*ipos) == "WP")) {
		THAT_trigger = true;
	}
	if (debug)
		cout << "Opening33::: " << open_item << ", " << *iorig << ", " << THAT_trigger << ", " << CC_trigger << endl;
	if (((*boost::next(ipos)->pred().begin() == PTEl("V")) && !CC_trigger && !THAT_trigger
			// && open_item.pred().begin()->str != "TO"
					) // a verb after a CC does not close a PRN
			|| (is_question_
					&& (open_item.pred().begin()->str == "WDT" || open_item.pred().begin()->str == "WP" )
					&& !( *boost::prior(ipos) == FuzzyPred("IN(that)") )
					&& !( *boost::prior(ipos) == FuzzyPred("IN(if)") )
					&& ( ( *ipos->pred().begin() != PTEl("TO") && *boost::next(ipos)->pred().begin() == PTEl("VB") )
							|| (  !(*ipos == FuzzyPred("IN(that)") ) && *boost::next(ipos)->pred().begin() == PTEl("MD") )
					)
			)
			|| (*boost::next(ipos)->pred().begin() == PTEl("MD") && !CC_trigger && !THAT_trigger) // a verb after a CC does not close a PRN
			|| (ipos != preds.begin()   // the man that tried to run escaped
					&& (*boost::prior(ipos)->pred().begin() == PTEl("VB") && extract_header(*ipos) != "IN" // "He thinks that it is good to speak of that"
					&& extract_header(*ipos) != "TO" && extract_header(*ipos) != "PRP$"
							&& extract_header(*ipos) != "DT" && extract_header(*ipos) != "N")
					&& boost::prior(ipos) != preds.begin()
					&& (*boost::prior(boost::prior(ipos))->pred().begin() == PTEl("TO")
							&& *boost::next(boost::prior(ipos))->pred().begin() != PTEl("TO")
							&& *boost::next(boost::prior(ipos))->pred().begin() != PTEl("IN"))
					&& (*ipos->pred().begin() != PTEl("DT") && *ipos->pred().begin() != PTEl("N")
							&& *ipos->pred().begin() != PTEl("J") && *ipos->pred().begin() != PTEl("CD")
							&& *ipos->pred().begin() != PTEl("WP") && *ipos->pred().begin() != PTEl("WDT")
							&& (*ipos->pred().begin() != PTEl("RB") && boost::next(ipos)->pred().begin()->str != "VBN"))) // You cannot use just VB because VB is the child of an auxiliary
			|| (*ipos->pred().begin() == PTEl("VBN") && boost::next(ipos)->pred().begin()->str == "V")
			|| (open_item.pred().begin()->str == "IN" && !is_question_ && !CC_trigger && boost::next(ipos) != preds.end()
					&& (*boost::next(ipos) == FuzzyPred("PRP(he)") || *boost::next(ipos) == FuzzyPred("PRP(she)")
							|| *boost::next(ipos) == FuzzyPred("PRP(i)")
							|| (*boost::next(ipos) == FuzzyPred("PRP(it)")
									&& boost::next(ipos)->pred().begin()->str != "DT") // "making it a big deal" does not trigger a PRN
					)
			)
	) {
		if (debug)
			cout << "IN-1::: " << open_item << endl;
		bool trigger_VBN = false;
		if (extract_header(*ipos) == "VBN")
			trigger_VBN = true;
		vector<FuzzyPred>::iterator ipos2 = ipos;
		--ipos2;
		for (; ipos2 != iter_begin; --ipos2) {
			if (trigger_VBN == true //the outcome of having been taken was that I could not play
			&& ipos2->pred().begin()->str == "VBN" && boost::next(ipos2) != preds.end()
					&& boost::next(ipos2)->pred().begin()->str != "VBN") {

				bool to_accept = false;
				// check there is a verb afterward
				vector<FuzzyPred>::iterator ipos3 = ipos;
				++ipos3;
				for (; ipos3 != preds.end(); ++ipos3) {
					if (boost::prior(ipos3)->pred().begin()->str == "N" && ipos3->pred().begin()->str == "PRP") {
						break;
					}
					if (boost::prior(ipos3)->pred().begin()->str == "TO" && ipos3->pred().begin()->str == "VB") {
						break;
					}
					if (ipos3->pred().begin()->str == "V" || *ipos3->pred().begin() == PTEl("VB")
							|| *ipos3->pred().begin() == PTEl("VBN") || *ipos3->pred().begin() == PTEl("VBG")
							|| *ipos3->pred().begin() == PTEl("AUX") || *ipos3->pred().begin() == PTEl("MD")) {
						to_accept = true;
						break;
					}
					if (ipos3->pred().begin()->str != "N" && ipos3->pred().begin()->str != "J" && ipos3->pred().begin()->str != "DT"
						&& ipos3->pred().begin()->str != "WDT" && ipos3->pred().begin()->str != "WP"
						&& ipos3->pred().begin()->str != "CD" && ipos3->pred().begin()->str != "TO"
						&& ipos3->pred().begin()->str != "CC" && ipos3->pred().begin()->str != "RB"
						&& ipos3->pred().begin()->str != "PRN" && ipos3->pred().begin()->str != "SBAR"
						&& ipos3->pred().begin()->str != "EX" && ipos3->pred().begin()->str != "PRP"
						&& ipos3->pred().begin()->str != "PRP$"
						&& ipos3->pred().begin()->str != "IN") {
						break;
					}
				}

				if (debug)
					cout << "IN-2::: " << *ipos2 << " " << open_item << endl;
				if(to_accept)
					return true;
			}
			if (debug)
				cout << "IN2::: " << *ipos2 << ", " << open_item << endl;
			if (trigger_VBN == false
					&& ((ipos2->pred().begin()->str == "V") || ipos2->pred().begin()->str == "MD"
							|| ipos2->pred().begin()->str == "VBN"
							|| (ipos2->pred().begin()->str == "VB" && !is_question_
									&& boost::next(ipos2)->pred().begin()->str != "VBG"
									&& boost::next(ipos2)->pred().begin()->str != "TO"
									&& boost::next(ipos2)->pred().begin()->str != "IN"
									&& boost::next(ipos2)->pred().begin()->str != "PRP")
							|| (ipos2->pred().begin()->str == "VBG"
									&& (open_item == FuzzyPred("IN(of)") || open_item == FuzzyPred("IN(for)")))
							|| (ipos2->pred().begin()->str == "VB" && open_item == FuzzyPred("TO(to)")))) {
				return true;
			}
			if (debug)
				cout << "IN3::: " << *ipos2 << endl;
			if (*ipos2 == FuzzyPred("IN(if)"))
				return false;
			if (*ipos2 == FuzzyPred("IN(that)") || *ipos2 == FuzzyPred("WDT(that)"))
				return false;
			if (ipos2->pred().begin()->str == "RB" && boost::prior(ipos2)->pred().begin()->str == "V"
					&& (boost::next(ipos2)->pred().begin()->str == "V" || boost::next(ipos2)->pred().begin()->str == "VB"))
				return false;
			if (debug)
				cout << "IN3::: " << *ipos2 << endl;
			if (ipos2->pred().begin()->str != "N" && ipos2->pred().begin()->str != "J" && ipos2->pred().begin()->str != "JJR"
					&& ipos2->pred().begin()->str != "DT" && ipos2->pred().begin()->str != "PRP"
					&& ipos2->pred().begin()->str != "PRP$"
					//&& ipos2->pred().begin()->str != "VBG"
					&& ipos2->pred().begin()->str != "IN" /// Check this
					&& ipos2->pred().begin()->str != "TO" && ipos2->pred().begin()->str != "RB"
					&& ipos2->pred().begin()->str != "POS" && ipos2->pred().begin()->str != "VB"
					&& ipos2->pred().begin()->str != "CC" ///
							//&& ipos2->pred().begin()->str != "-comma-" ///
							) {
				return false;
			}
			if (debug)
				cout << "IN4::: " << *ipos2 << endl;
		}
		if (ipos2->pred().begin()->str == "VBG" // a worker participating in a society is fine
		&& open_item == *ipos2) {
			return true;
		}
		if (ipos != preds.begin()
				&& (*boost::prior(ipos)->pred().begin() == PTEl("V") || *boost::prior(ipos)->pred().begin() == PTEl("VBG")
						|| *boost::prior(ipos)->pred().begin() == PTEl("VBN"))) {
			return true;
		}
	}

	return false;
}

vector<pair<string, vector<FuzzyPred> > > parser::extract_nested(vector<FuzzyPred> *data, int *num_prn)
// Substitute the nested sententeces in the list of tags
{
	vector<FuzzyPred> preds(*data);
	vector<pair<string, vector<FuzzyPred> > > ret_prns;
	vector<FuzzyPred>::iterator piter = preds.begin();
	vector<FuzzyPred>::iterator pend = preds.end();

	vector<FuzzyPred>::iterator start, end;
	bool trigger_sbar = false; /// there is only one nesting level!
	int num = 0, pos = 0;
	FuzzyPred open_item;
	while (piter != pend && extract_header(*piter) != "-period-" && extract_header(*piter) != "-QM-") {
		if (debug)
			cout << "ISQ0:::" << std::boolalpha << is_question_ << std::flush << endl;
		if (trigger_sbar) {
			if (is_closing_nested(piter, start, preds, open_item) && !shortfind(noprn_, pos)) {
				string sbar_str = "sbar" + boost::lexical_cast<string>(++num);
				end = piter;
				ret_prns.push_back(make_pair(sbar_str, vector<FuzzyPred>(start, end + 1)));
				string sub_name = "SBAR";
				if (extract_header(*start) == "VBG") // if it is a VBG subordinate
					sub_name = "SBAR-NP";

				piter = preds.erase(start, end + 1);

				piter = preds.insert(piter, FuzzyPred((string) sub_name + "(" + sbar_str + ")"));
				pend = preds.end();
				trigger_sbar = false;
				++piter;
				++pos;
				continue;
			}
		}
		if ( //!trigger_sbar &&
		is_opening_nested(piter, preds, open_item, is_question_) && !shortfind(noprn_, pos)) {
			//start= piter-1;
			open_item = *piter;
			start = piter;
			trigger_sbar = true;
			++piter;
			++pos;
			continue;
		}
		++piter;
		++pos;
	}
	//print_vector(preds);
	if (ret_prns.size()) {
		vector<FuzzyPred> tmp = ret_prns.front().second;
		*data = preds;
		*num_prn = num;
	}
	return ret_prns;
}

static FuzzyPred group_fragments_together(const vector<FuzzyPred> &current_layer)
{
	FuzzyPred to_return("ROOT(FRAC)");
	PredTree pt = to_return.pred();
	PredTree::iterator pi = pt.begin().firstChild();
	for (int n = 0; n < current_layer.size(); ++n) {
		pt.appendTree(pi, current_layer.at(n).pred());
	}
	to_return.pred() = pt;
	return to_return;
}

vector<pair<vector<FuzzyPred>, Memory> > parser::parse(vector<FuzzyPred> data)
{
	FuzzyPred question("--");
	/// erase the question !!!
	vector<pair<vector<FuzzyPred>, Memory> > parsed;

	// The data
	//print_vector(data);

	vector<Clause> new_feet_clauses = get_relevant_clauses_from_feet(pinfo_->get_feet_clauses_map(), data);

	// std::cout << "---- "<< new_feet_clauses.size() << std::endl;
	// print_vector_return(new_feet_clauses);

	// cycles thru the inferences steps
	int i;
	parsed.clear();
	vector<pair<vector<FuzzyPred>, Memory> > lastData;
	vector<pair<vector<FuzzyPred>, Memory> > data_list;
	Memory last_layer_memory;
	//vector<pair<vector<FuzzyPred>,Memory> > initialData;
	//initialData.push_back(make_pair(data,1));
	try {
		lastData = launch_inference_threads(new_feet_clauses, data, last_layer_memory, 0, 10, 10); /// it was 50, 5
		//lastData = launch_inference_threads(new_feet_clauses, data, last_layer_memory, 0, 100, 10); //-MAC

		if (lastData.size() == 0)
			puts("Empty!!");

		insert_data(&data_list, lastData);
		sort_data(&data_list);

		vector<FuzzyPred> current_layer;

		int n = 0;
		bool root_time;
		vector<pair<vector<FuzzyPred>, Memory> > tmp_vect, tmp_roots;
		while (data_list.size() && n++ < 15 && parsed.size() < 1) {
			current_layer = data_list.at(0).first;
			last_layer_memory = data_list.at(0).second;
			new_feet_clauses = get_exact_match(pinfo_->get_roots_clauses_map(), current_layer);
			if (new_feet_clauses.size() != 0) {
				root_time = true;
			} else {
				root_time = false;
				new_feet_clauses = get_relevant_clauses_from_feet(pinfo_->get_feet_clauses_map(), current_layer);
				vector<Clause> tmp = get_relevant_clauses_from_feet(pinfo_->get_bulk_clauses_map(), current_layer);
				new_feet_clauses.insert(new_feet_clauses.end(), tmp.begin(), tmp.end());
			}
			if (root_time) {
				Inference *inference = new Inference(question, current_layer, 1, new_feet_clauses, last_layer_memory); /// it was 20, 5
				inference->init();
				inference->makeSingleStep(new_feet_clauses.at(0));
				tmp_vect = inference->getLastData();
				delete inference;
			} else {
				//std::cout << "root size: "<< new_feet_clauses.size() << std::endl;
				tmp_vect = launch_inference_threads(new_feet_clauses, current_layer, last_layer_memory, 0, 7, 7);
				//tmp_vect = launch_inference_threads(new_feet_clauses, current_layer, last_layer_memory, 0, 100, 10); //-MAC
			}
			tmp_roots = get_roots(tmp_vect);
			if (tmp_roots.size() > 0) {
				data_list.erase(data_list.begin());
				tmp_roots.erase(tmp_roots.begin() + 1, tmp_roots.end());
				vector<pair<vector<FuzzyPred>, Memory> > parsed_tmp;
				Memory mem(last_layer_memory);
				mem.add(tmp_roots.at(0).first.size(), new_feet_clauses.at(0).getWeigth());
				insert_data(&parsed_tmp, tmp_roots);
				insert_data(&parsed, tmp_roots);
				continue;
			} else if (tmp_vect.size()) {
				data_list.erase(data_list.begin());
				tmp_vect.erase(tmp_vect.begin() + min(tmp_vect.size(), 4), tmp_vect.end());
				insert_data(&data_list, tmp_vect);
				data_list = apply_censor(data_list);
				sort_data(&data_list);
				continue;
			} else
				data_list.erase(data_list.begin());
		}
	} catch (FTree<PTEl>::TreeError &t) {
		std::cout << t.what() << std::endl;
	}
	sort_data(&parsed);
	if (parsed.size() == 0) {
		vector<FuzzyPred> current_layer = data_list.at(0).first;
		FuzzyPred tmp_pred = group_fragments_together(current_layer);
		vector<FuzzyPred> tmp_vect;
		tmp_vect.push_back(tmp_pred);
		parsed.push_back(make_pair(tmp_vect, Memory()));
	}

	if (parsed.size()) {
		//std::cout << "Solution:\n";
		int n;
		for (n = 0; n < parsed.size(); ++n) {
			parsed.at(n).first.at(0) = post_process(parsed.at(n).first.at(0));
			if (debug)
				std::cout << "PHRASE_ITERATION06" << n << endl;
			if(!is_question_)
				parsed.at(n).first.at(0) = apply_corrections(parsed.at(n).first.at(0), pinfo_->get_correction_clauses());
			if (debug)
				std::cout << "PHRASE_ITERATION07" << n << endl;
			parsed.at(n).first.at(0) = add_possdt(parsed.at(n).first.at(0));
			if (debug)
				std::cout << "PHRASE_ITERATION08" << n << endl;
		}
	}
	return parsed;
}

vector<pair<vector<FuzzyPred>, Memory> > parser::parse_all_subs(vector<FuzzyPred> data)
{
	bool has_opening_par = false, has_closing_par = false;
	FuzzyPred opar, cpar;
	if (data.front().pred().begin()->str != "DT" && data.front().pred().begin()->str != "WP"
			&& data.front().pred().begin()->str != "WDT"
			&& (data.front().pred().begin()->str != "IN" && data.front().pred().begin()->str != "TO"
					&& (is_opening_par(data.front()) || has_opening_nested(data.begin(), data)))) {
		has_opening_par = true;
		opar = data.front();
		data.erase(data.begin());
	}
	if (is_closing_par(data.back()) || has_closing_nested(data.end() - 1, data)) {
		has_closing_par = true;
		cpar = data.back();
		data.pop_back();
	}

	vector<vector<pair<vector<FuzzyPred>, Memory> > > parsed_ones;

	vector<FuzzyPred>::iterator diter = data.begin();
	vector<FuzzyPred>::iterator dend = data.end();
	vector<FuzzyPred> prevector;
	vector<FuzzyPred> subvector;
	vector<vector<FuzzyPred> > subvectors;
	vector<FuzzyPred> trigger_pred;
	bool trigger;
	int pos, orig_pos;
	trigger_pred.push_back(FuzzyPred("DummyPred"));

	do {
		pos = 0;
		prevector.clear();
		subvector.clear();
		trigger = false;
		for (; diter != dend; ++diter, ++pos) {
			if (trigger) {
				subvector.push_back(*diter);
			} else if (!is_sbar_trigger(trigger_pred.back(), data, diter, pos))
				prevector.push_back(*diter);
			if (is_sbar_trigger(trigger_pred.back(), data, diter, pos)) {
				if (!trigger && is_to_insert(data, diter, pos)) {
					subvector.push_back(*diter);
				}
				if (trigger == false) {
					if (is_prp_sbar(data, diter, pos))
						trigger_pred.push_back(FuzzyPred("PRP-HOLDER(*)"));
					else if (is_article_sbar(data, diter, pos))
						trigger_pred.push_back(FuzzyPred("DT-HOLDER(*)"));
					else if (is_WP_sbar(data, diter, pos))
						trigger_pred.push_back(FuzzyPred("WP-HOLDER(*)"));
					else if (is_IN_sbar(data, diter, pos))
						trigger_pred.push_back(FuzzyPred("IN-HOLDER(*)"));
					else if (is_TO_sbar(data, diter, pos))
						trigger_pred.push_back(FuzzyPred("TO-HOLDER(*)"));
					else if (is_N_sbar(data, diter, pos))
						trigger_pred.push_back(FuzzyPred("N-HOLDER(*)"));
					else if (is_V_sbar(data, diter, pos))
						trigger_pred.push_back(FuzzyPred("V-HOLDER(*)"));
					else
						trigger_pred.push_back(*diter);

					trigger = true;
				}
			}
		}
		orig_pos += pos;
		subvectors.push_back(prevector);
		data = subvector;
		diter = data.begin();
		dend = data.end();
	} while (subvector.size() > 0);

	//print_vector(trigger_pred);
	int j;
	for (int n = 0; n < subvectors.size(); ++n) {
		if (subvectors.at(n).size() > 0) {
			vector<pair<vector<FuzzyPred>, Memory> > tmp_parsed = parse(subvectors.at(n));
			j = 0;
			while (tmp_parsed.size() == 0 && j < 2) { // try to parse 2 times before giving up
				tmp_parsed = parse(subvectors.at(n));
				++j;
			}
			parsed_ones.push_back(tmp_parsed);
		}
	}

	for (int j = parsed_ones.size() - 1; j > 0; --j) {
		vector<pair<vector<FuzzyPred>, Memory> > tmp_vect;
		tmp_vect.clear();
		parsed_ones.at(j).erase(parsed_ones.at(j).begin() + min(parsed_ones.at(j).size(), 5), parsed_ones.at(j).end()); /// Do it in the following for cycle
		for (int n = 0; n < parsed_ones.at(j).size(); ++n) {
			int trigger_j = trigger_pred.size() - (parsed_ones.size() - j);
			if (trigger_j >= 0 && trigger_j <= trigger_pred.size())
				parsed_ones.at(j).at(n).first.at(0) = prepare_sbar(parsed_ones.at(j).at(n).first.at(0),
						trigger_pred.at(trigger_j));
			for (int m = 0; m < parsed_ones.at(j - 1).size(); ++m) {
				vector<FuzzyPred> tmp(1);
				tmp.at(0) = join_sbar(parsed_ones.at(j - 1).at(m).first.at(0), parsed_ones.at(j).at(n).first.at(0));
				Memory mem_tmp(parsed_ones.at(j - 1).at(m).second);
				mem_tmp.add(parsed_ones.at(j).at(n).second);
				tmp_vect.push_back(make_pair(tmp, mem_tmp));
			}
		}
		parsed_ones.at(j - 1) = tmp_vect;
	}

	if ( parsed_ones.size() && (has_opening_par || has_closing_par) ){
		for (int j = 0; j < parsed_ones.at(0).size(); ++j) {
			if (has_opening_par)
				parsed_ones.at(0).at(j).first.at(0).pred().appendTreeFront(parsed_ones.at(0).at(j).first.at(0).pred().begin(),
						opar.pred());
			if (has_closing_par) {
				parsed_ones.at(0).at(j).first.at(0).pred().appendTree(parsed_ones.at(0).at(j).first.at(0).pred().begin(),
						cpar.pred());
			}
		}
	}
	if(parsed_ones.size()) {
		sort_data(&parsed_ones.at(0));
		return parsed_ones.at(0);
	}
	return vector<pair<vector<FuzzyPred>, Memory> >();
}

static string get_prn_str(const FuzzyPred &parsed_prn)
// for phrases like 'he was called "the fighter"' return NP, otherwise PRN
{
	PredTree pt(parsed_prn.pred());
	PredTree::height_iterator piter(pt, 1);
	PredTree::iterator pend(pt.end());

	string prn_str = "PRN";
	return prn_str;
}

static FuzzyPred join_prn(FuzzyPred parsed, FuzzyPred parsed_prn, const string &prn_str)
{


	PredTree pt(parsed.pred());
	PredTree::height_iterator piter(pt, 1);

	string prn_string = get_prn_str(parsed_prn);

	for (; piter != pt.end(); ++piter) {
		if (piter.firstChild()->str == prn_str) {
			PredTree prn_tree = parsed_prn.pred();
			prn_tree.begin()->str = prn_string;
			piter = pt.replace(prn_tree, piter);
		}
	}
	return FuzzyPred(pt);
}

static FuzzyPred simplify_name(FuzzyPred fpred)
{
	string str = fpred.pred().begin()->str;
	string name = fpred.pred().begin().firstChild()->str;
	if (str == "VBZ" || str == "VBP" || str == "VBD")
		str = "V";
	else if (str == "NN" || str == "NNS" || str == "NNP" || str == "NNPS")
		str = "N";
	else if (str == "JJ" || str == "JJS" || str == "VBJ")
		str = "J";
	else if (str == "-period-" && name == "?")
		str = "-QM-";
	fpred.pred().begin()->str = str;

	return fpred;
}

static bool is_date(const string &str)
{
	if (str.find("[date]") != string::npos)
		return true;
	return false;
}

static bool is_place(const string &str)
{
	if (str.find("[place]") != string::npos)
		return true;
	return false;
}

static vector<FuzzyPred> simplify_tags(const vector<FuzzyPred> &tags)
{
	vector<FuzzyPred> ret_vect;
	vector<FuzzyPred>::const_iterator tags_iter = tags.begin();

	while (tags_iter != tags.end()) {
		if(tags_iter->pred().begin().firstChild() == tags_iter->pred().end() ) {
			ret_vect.push_back(*tags_iter);
			++tags_iter;
			continue;
		}
		string tag = tags_iter->pred().begin()->str;
		string word = tags_iter->pred().begin().firstChild()->str;
		if (is_date(word)) {
			ret_vect.push_back(FuzzyPred(string("CD-DATE(") + word + ")"));
		} else if (is_place(word)) {
			ret_vect.push_back(FuzzyPred(string("N-PLACE(") + word + ")"));
		}
		else
			ret_vect.push_back(simplify_name(*tags_iter));
		++tags_iter;
	}

	return ret_vect;
}

void parser::setForbiddenPRN(const vector<int> &noprn)
{
	noprn_ = noprn;
}

void parser::setForbiddenSBAR(const vector<int> &nosbar)
{
	nosbar_ = nosbar;
}

void parser::setTags(const vector<FuzzyPred> &tags)
{
	tagged_ = tags;
	if (tagged_.size() == 0) {
		tagged_.push_back(FuzzyPred("-period-(-period-)"));
	}

	tagged_simple_ = simplify_tags(tagged_);
}

static void sign_no_SBAR(vector<FuzzyPred> &data, const vector<int> &nosbar)
{
	for (int n = 0; n < data.size(); ++n) {
		if (shortfind(nosbar, n))
			data.at(n).setAttribute("NOSBAR");
	}
}

void parser::do_parse()
{
	vector<FuzzyPred> data = tagged_simple_;

	if (data.size() == 0) {
		throw(std::runtime_error("Parser: No data to parse!"));
	}

	vector<pair<string, vector<FuzzyPred> > > prn_vectors;
	vector<pair<string, vector<FuzzyPred> > > nested_sbar_vectors;

	if (data.back() == FuzzyPred("-QM-(?)")) {
		is_question_ = true;
	} else {
		is_question_ = false;
	}

	sign_no_SBAR(data, nosbar_);

	int num_prn;
	prn_vectors = extract_prn(&data, &num_prn);
	nested_sbar_vectors = extract_nested(&data, &num_prn);

	vector<pair<vector<FuzzyPred>, Memory> > parsed_ones = parse_all_subs(data);
	vector<pair<string, vector<pair<vector<FuzzyPred>, Memory> > > > parsed_prn;
	for (int m = 0; m < prn_vectors.size(); ++m) {
		string prn_str = prn_vectors.at(m).first;
		vector<FuzzyPred> prn_data = prn_vectors.at(m).second;
		parsed_prn.push_back(make_pair(prn_str, parse_all_subs(prn_data)));
	}
	for (int m = 0; m < nested_sbar_vectors.size(); ++m) {
		string prn_str = nested_sbar_vectors.at(m).first;
		vector<FuzzyPred> prn_data = nested_sbar_vectors.at(m).second;
		parsed_prn.push_back(make_pair(prn_str, parse_all_subs(prn_data)));
	}

	if (parsed_prn.size() > 0) {
		vector<pair<vector<FuzzyPred>, Memory> > parsed_with_prn;
		for (int i = 0; i < 2; ++i) { /// BAD SOLUTION. In this way you try to make all the substitutions
			for (int m = 0; m < parsed_prn.size(); ++m) {
				string prn_str = parsed_prn.at(m).first;
				vector<pair<vector<FuzzyPred>, Memory> > prn_data_and_weigth = parsed_prn.at(m).second;
				for (int n = 0; n < parsed_ones.size(); ++n) {
					FuzzyPred orig_data = parsed_ones.at(n).first.at(0);
					Memory mem_tmp(parsed_ones.at(n).second);
					for (int j = 0; j < prn_data_and_weigth.size(); ++j) {
						FuzzyPred prn_data = prn_data_and_weigth.at(j).first.at(0);
						Memory prn_mem = prn_data_and_weigth.at(j).second;
						vector<FuzzyPred> tmp(1);

						tmp.at(0) = join_prn(orig_data, prn_data, prn_str); // substitute the PRN (and nested SBAR) back in
						Memory tmp_mem2(mem_tmp);
						tmp_mem2.add(prn_mem);
						parsed_with_prn.push_back(make_pair(tmp, tmp_mem2));
					}
				}
				parsed_ones = parsed_with_prn;
				parsed_with_prn.clear();
			}
		}
	}

	bool is_question = data.back().pred().begin()->str == "-QM-";
	parsed_.clear();
	for (int n = 0; n < parsed_ones.size(); ++n) {
		FuzzyPred tmp_pred;
		tmp_pred = post_process_sbar(parsed_ones.at(n).first.at(0));
		if (is_question)
			tmp_pred = apply_corrections_questions(tmp_pred);
		tmp_pred = restore_original_tags(tmp_pred, tagged_); // Substitute the original tags with the simplified ones
		parsed_.push_back(make_pair(tmp_pred, parsed_ones.at(n).second.getEntropy()));
	}

	parsed_.erase(parsed_.begin() + min(parsed_.size(), 10), parsed_.end());

}

void parser::clear()
{
	tagged_.clear();
	tagged_simple_.clear();
	noprn_.clear();
	nosbar_.clear();
}
