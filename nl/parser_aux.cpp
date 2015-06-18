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



#include"parser_aux.hpp"

const bool debug = false;



static inline string extract_header(const Predicate &pred)
{
	return pred.pred().begin()->str;
}


static inline string extract_first_child(const Predicate &pred)
{
	return pred.pred().begin().firstChild()->str;
}



// auxiliary functions for selecting the clauses to match
string extract_hyp(const string &clause)
{
	vector<string> strs, hyp;
	string ret_string;

	boost::split(strs, clause, boost::is_any_of(":"));
	if (strs.size() != 2)
		throw(std::invalid_argument("Badly formed clause."));
	strs.at(1).erase(0, 1); // erase the '-'
	boost::split(hyp, strs.at(1), boost::is_any_of(","));
	//ret_string= hyp.at(0);
	int n = 0;
	if (hyp.size() > 0) {
		ret_string += hyp.at(0);
		++n;
	}
	if (hyp.size() > 1) {
		ret_string += hyp.at(1);
		++n;
	}
	// if(hyp.size() > 2) {
	//  	  ret_string+= hyp.at(2);
	//   	  ++n;
	// }
	//for(; n< hyp.size()-1; ++n)
	for (; n < hyp.size(); ++n)
		//ret_string += string(" X");
		ret_string += hyp.at(n);
	//ret_string += hyp.at(hyp.size()-1);
	return ret_string;
}

string extract_cons(const string &clause)
{
	vector<string> strs;

	boost::split(strs, clause, boost::is_any_of(":"));
	if (strs.size() != 2)
		throw(std::invalid_argument("Badly formed clause."));
	return Predicate(strs.at(0)).pred().begin()->str;
}

vector<string> get_feet_from_clause(const Clause &clause)
{
	vector<string> hyp_tags;
	vector<FuzzyPred> hyp = clause.getHypothesis();
	vector<FuzzyPred>::iterator hyp_iter = hyp.begin();

	for (; hyp_iter != hyp.end(); ++hyp_iter)
		hyp_tags.push_back(hyp_iter->getOrder().first);
	return hyp_tags;
}

bool is_relevant(const Clause &clause, const vector<FuzzyPred> &data)
{

	vector<string> hyp_tags;
	vector<string> data_tags;
	vector<string>::iterator hyp_tags_iter;
	vector<string>::iterator siter;
	vector<FuzzyPred> hyp = clause.getHypothesis();
	vector<FuzzyPred>::iterator hyp_iter = hyp.begin();
	vector<FuzzyPred>::const_iterator data_iter = data.begin();

	for (; data_iter != data.end(); ++data_iter)
		data_tags.push_back(data_iter->getOrder().first); /// The name is enough!! Do not ask for the number of children
	for (; hyp_iter != hyp.end(); ++hyp_iter)
		hyp_tags.push_back(hyp_iter->getOrder().first);
	hyp_tags_iter = hyp_tags.begin();
	for (; hyp_tags_iter != hyp_tags.end(); ++hyp_tags_iter) {
		siter = find(data_tags.begin(), data_tags.end(), *hyp_tags_iter);
		if (siter == data_tags.end())
			return false;
	}
	return true;
}

bool is_valid_start(vector<string> &tags, vector<string>::iterator start)
// some subset of tags are not valid: "a short man is on the table" cannot be understood as "a short (man is on the table)"
{
	if (start != tags.begin() && (*start == "N" || *start == "J" || *start == "CD")
			&& (*boost::prior(start) == "N" || *boost::prior(start) == "J" || *boost::prior(start) == "DT"
					|| *boost::prior(start) == "WP" || *boost::prior(start) == "CD" || *boost::prior(start) == "JJR"
					|| *boost::prior(start) == "JJS" || *boost::prior(start) == "WP$" || *boost::prior(start) == "PRP$"))
		return false;

	if (start != tags.begin() && (*start == "N" || *start == "J" || *start == "CD")
			&& (*boost::prior(start) == "N" || *boost::prior(start) == "J" || *boost::prior(start) == "DT"
					|| *boost::prior(start) == "WP" || *boost::prior(start) == "CD" || *boost::prior(start) == "JJR"
					|| *boost::prior(start) == "JJS" || *boost::prior(start) == "WP$" || *boost::prior(start) == "PRP$"))
		return false;

	if (start != tags.begin() && boost::prior(start) != tags.begin() && (*start == "N" || *start == "J" || *start == "CD")
			&& *boost::prior(start) == "CC"
			&& (*boost::prior(boost::prior(start)) == "N" || *boost::prior(boost::prior(start)) == "J"
					|| *boost::prior(boost::prior(start)) == "DT" || *boost::prior(boost::prior(start)) == "WP"
					|| *boost::prior(boost::prior(start)) == "CD" || *boost::prior(boost::prior(start)) == "JJR"
					|| *boost::prior(boost::prior(start)) == "JJS" || *boost::prior(boost::prior(start)) == "WP$"
					|| *boost::prior(boost::prior(start)) == "PRP$"))
		return false;

	if (start != tags.begin() && (*start == "RB") && ( *boost::prior(start) == "WP" || *boost::prior(start) == "WDT" ) && boost::prior(start) != tags.begin()
			&& (*boost::prior(boost::prior(start)) == "IN" || *boost::prior(boost::prior(start)) == "TO"))
		return false;

	if (start != tags.begin() && (*start == "RB") && (*boost::prior(start) == "RBR" || *boost::prior(start) == "JJR"))
		return false;

	if (start != tags.begin() && (*start == "SBAR") //(NP VP (SBAR ....)) is forbidden
			)
		return false;

	if (start != tags.begin() && (*start == "$") && (*boost::prior(start) == "CD" || *boost::prior(start) == "DT"))
		return false;

	if (start != tags.begin() && (*start == "N" || *start == "J")
			&& (*boost::prior(start) == "POS" || *boost::prior(start) == "J" || *boost::prior(start) == "DT"
					|| *boost::prior(start) == "WP" || *boost::prior(start) == "WRB" || *boost::prior(start) == "CD"
					|| *boost::prior(start) == "RB"))
		return false;

	if (start != tags.begin() && (*start == "J") && (*boost::prior(start) == "PRP$"))
		return false;

	if (start != tags.begin() && (*start == "CD-DATE") && (*boost::prior(start) == "IN"))
		return false;

	if (start != tags.begin() && (*start == "N-PLACE") && (*boost::prior(start) == "IN"))
		return false;

	if (start != tags.begin() && (*start == "RBS") && (*boost::prior(start) == "POS" || *boost::prior(start) == "DT"))
		return false;

	if (start != tags.begin() && (*start == "RB") && (*boost::prior(start) == "IN" || *boost::prior(start) == "TO"))
		return false;

	if (start != tags.begin() && (*start == "WP") && (*boost::prior(start) == "IN" || *boost::prior(start) == "TO"))
		return false;

	if (start != tags.begin() && (*start == "PRP$") && (*boost::prior(start) == "DT" || *boost::prior(start) == "PDT"))
		return false;

	if (start != tags.begin() && (*start == "CD") && *boost::prior(start) == "IN" && boost::prior(start) != tags.begin()
			&& *boost::prior(boost::prior(start)) == "JJR")
		return false;

	// "Albert - with whom David was a friend - what happy"
	// N PRN V JJ -/>  N S(PRN,V,JJ)
	if (start != tags.begin() && (*start == "PRN")
			&& (*boost::prior(start) == "J" || *boost::prior(start) == "DT" || *boost::prior(start) == "N"
					|| *boost::prior(start) == "CD"))
		return false;

	/// You mush check there is a verb before! "As/IN | a/DT large convoy sailed..."
	if (start != tags.begin() && (*start == "DT" || *start == "N" || *start == "PRP$") && (*boost::prior(start) == "IN")) {
		vector<string>::iterator ipos = boost::prior(start);
		bool name_trigger = false;
		for (; ipos != tags.begin(); --ipos) {
			if (*ipos == "V")
				return true;
			if (*ipos == "N" || *ipos == "J" || *ipos == "CD" || *ipos == "DT" || *ipos == "WP") {
				name_trigger = true;
				break;
			}
		}
		if (name_trigger)
			return false;

//          if(name_trigger) {
//               for (; ipos != tags.begin(); --ipos) {
//                    cout << "MIL2::: " << *ipos << endl;
//                    if (*ipos == "IN")
//                         return true;
//               }
//               return false;
//          }
	}

//     if(start != tags.begin()
//        && (*start == "V")
//        && ( *boost::prior(start) == "WP"
//             )
//       )
//          return false;
//     if(start != tags.begin()
//        && (*start == "WP")
//        && ( *boost::prior(start) == "IN")
//       )
//          return false;
	return true;
}

bool is_valid_end(vector<string> &tags, vector<string>::iterator end)
// some subset of tags are not valid: "a short man is on the table" cannot be understood as "a short (man is on the table)"
{
	if (end != tags.end() && boost::next(end) != tags.end()
			&& (*end == "DT" || *end == "N" || *end == "J" || *end == "JJR" || *end == "JJS" || *end == "CD")
			&& (*boost::next(end) == "N" || *boost::next(end) == "J" || *boost::next(end) == "CD" // "the first"
			|| *boost::next(end) == "CC" // conjunctions that introduce phrases are not present at this level
			|| *boost::next(end) == "POS" // (team's) leading scorer
			)) {
		return false;
	}

//     if(end != tags.end() && boost::next(end) != tags.end()
//        && (*end == "WP" )
//        && ( *boost::next(end) == "N" || *boost::next(end) == "J"
//             || *boost::next(end) == "CD"
//             )
//       ) {
//          return false;
//     } /// forbidden by is_compatible()

	if (end != tags.end() && boost::next(end) != tags.end() && (*end == "CD" || *end == "DT") && (*boost::next(end) == "$" // 2$
	)) {
		return false;
	}
	if (end != tags.end() && boost::next(end) != tags.end() && (*end == "POS")
			&& (*boost::next(end) == "N" || *boost::next(end) == "J" || *boost::next(end) == "DT" || *boost::next(end) == "JJR"
					|| *boost::next(end) == "JJS" || *boost::next(end) == "RB" || *boost::next(end) == "RBR"
					|| *boost::next(end) == "RBS"))
		return false;

	if (end != tags.end() && boost::next(end) != tags.end() && (*end == "RBR" || *end == "RBS" || *end == "RB")
			&& (*boost::next(end) == "N" || *boost::next(end) == "J" || *boost::next(end) == "RB"))
		return false;

	if (end != tags.end() && boost::next(end) != tags.end() && (*end == "IN" || *end == "TO")
			&& (*boost::next(end) == "WP" || *boost::next(end) == "WDT"
					|| *boost::next(end) == "DT" || *boost::next(end) == "CD" || *boost::next(end) == "N"
					|| *boost::next(end) == "RB" || *boost::next(end) == "RBS" || *boost::next(end) == "RBR"
					|| *boost::next(end) == "PRP$" || *boost::next(end) == "PRP"))
		return false;

	if (end != tags.end() && boost::next(end) != tags.end() && (*end == "V" || *end == "VB")
			&& (*boost::next(end) == "N" || *boost::next(end) == "J" || *boost::next(end) == "PRP"))
		return false;

	if (end != tags.begin() && (*end == "PRP")
			&& (*boost::prior(end) == "N" || *boost::prior(end) == "J" || *boost::prior(end) == "PRP"))
		return false;

	if (end != tags.end() && boost::next(end) != tags.end() && (*end == "V" || *end == "VBN") && (*boost::next(end) == "RP"))
		return false;

	if (end != tags.end() && boost::next(end) != tags.end() && (*end == "IN") && (*boost::next(end) == "CD-DATE"))
		return false;

	if (end != tags.end() && boost::next(end) != tags.end() && (*end == "IN") && (*boost::next(end) == "N-PLACE"))
		return false;

	if (end != tags.end() && boost::next(end) != tags.end() && (*end == "DT") && (*boost::next(end) == "PRP$"))
		return false;

	if (end != tags.end() && boost::next(end) != tags.end() && (*end == "IN") && (*boost::next(end) == "VBG"))
		return false;

	if (end != tags.end() && boost::next(end) != tags.end() && (*end == "EX") && (*boost::next(end) == "V"))
		return false;

	if (end != tags.end() && boost::next(end) != tags.end() && (*end == "EX") && (*boost::next(end) == "VP"))
		return false;

//     if(end != tags.end() && boost::next(end) != tags.end()
//        && (*end == "WP")
//        && ( *boost::next(end) == "V")
//             )  // "... that the strike was imminent"
//          return false;
//     if(end != tags.end() && boost::next(end) != tags.end()
//        && (*end == "IN")
//        && ( *boost::next(end) == "WP")
//             )  // "... however in what was his most dovish remark ..."
//          return false;

	return true;
}

bool is_compatible(vector<string> &tags, vector<string>::iterator start, vector<string>::iterator end)
{
	if (end != tags.end() && boost::next(end) != tags.end() && (*start == "IN")
			&& (*boost::next(end) == "V" || *boost::next(end) == "VBG") && (*end != "SBAR"))
		return false;

	if (end != tags.end() && boost::next(end) != tags.end() && *start == "IN"
			&& (*boost::next(start) == "PRP" || *boost::next(start) == "CD" || *boost::next(start) == "PRP$"
					|| *boost::next(start) == "N") && (*boost::next(end) == "VP") && (*end != "SBAR"))
		return false;

	if (end != tags.end() && boost::next(end) != tags.end() && start != tags.begin()
			&& (*boost::prior(start) == "IN" && *start != "PRP" && *start != "CD" && *start != "DT") && (*end == "VP"))
		return false;

	if (end != tags.end() && boost::next(start) != tags.end() && end != tags.begin() && *start == "IN"
			&& *boost::next(start) == "WP" && (*end == "N" || *end == "J"))
		return false;

	if (end != tags.end() && boost::next(start) != tags.end() && end != tags.begin() && *start == "IN" && *end == "EX")
		return false;

	return true;
}

vector<Clause> get_relevant_clauses_from_feet(
		const clauses_map &matching_feet, vector<FuzzyPred> data)
{
	vector<Clause>::iterator citer;
	vector<Clause> ret_vect;
	vector<string> data_tags;
	vector<FuzzyPred>::const_iterator data_iter = data.begin();

	for (; data_iter != data.end(); ++data_iter)
		data_tags.push_back(data_iter->getOrder().first); /// The name is enough!! Do not ask for the number of children

	vector<string>::iterator tags_iter;
	vector<string>::iterator tags_iter_begin;
	vector<string>::iterator tags_iter_end;
	tags_iter_begin = data_tags.begin();
	//for(; boost::next(tags_iter_begin) != data_tags.end(); ++tags_iter_begin) {
	for (; tags_iter_begin != data_tags.end(); ++tags_iter_begin) {
		tags_iter_end = data_tags.end();
		for (; tags_iter_end != tags_iter_begin; --tags_iter_end) {
			vector<string> tmp_vect(tags_iter_begin, tags_iter_end);
			if (matching_feet.find(tmp_vect) != matching_feet.end() && is_valid_start(data_tags, tags_iter_begin)
					&& is_valid_end(data_tags, boost::prior(tags_iter_end))
					&& is_compatible(data_tags, tags_iter_begin, boost::prior(tags_iter_end))) {
				vector<ClauseContainer> to_insert(matching_feet.find(tmp_vect)->second);
				for (int n = 0; n < to_insert.size(); ++n) {
					ret_vect.push_back(to_insert.at(n).getClause());
					//cout << to_insert.at(n).getClause() << endl;
				}
//                    vector<Clause> to_insert(matching_feet.find(tmp_vect)->second);
//                    ret_vect.insert(ret_vect.end(), to_insert.begin(), to_insert.end());
			}
		}
	}
	// std::cout << ">>> ";
	// print_vector_return(ret_vect);
	return ret_vect;
}

vector<Clause> get_exact_match(const clauses_map &matching_feet,
		vector<FuzzyPred> data)
{
	vector<Clause>::iterator citer;
	vector<Clause> ret_vect;
	vector<string> data_tags;
	vector<FuzzyPred>::const_iterator data_iter = data.begin();

	for (; data_iter != data.end(); ++data_iter)
		data_tags.push_back(data_iter->getOrder().first); /// The name is enough!! Do not ask for the number of children

	vector<string>::iterator tags_iter_begin;
	vector<string>::iterator tags_iter_end;
	tags_iter_begin = data_tags.begin();
	tags_iter_end = data_tags.end();
	vector<string> tmp_vect(tags_iter_begin, tags_iter_end);
	if (matching_feet.find(tmp_vect) != matching_feet.end()) {
		vector<ClauseContainer> to_insert(matching_feet.find(tmp_vect)->second);
		for (int n = 0; n < to_insert.size(); ++n)
			ret_vect.push_back(to_insert.at(n).getClause());
//	  vector<Clause> to_insert( matching_feet.find(tmp_vect)->second );
//	  ret_vect.insert(ret_vect.end(), to_insert.begin(), to_insert.end() );
	}
	//print_vector_return(ret_vect);
	return ret_vect;
}

class compare_pred {
public:
	bool operator ()(const vector<FuzzyPred> lhs, const vector<FuzzyPred> rhs) const
	{
		//if(lhs.size() > rhs.size()) return false;
		std::stringstream sl, sr;
		print_vector(sl, lhs);
		print_vector(sr, rhs);
		return sl.str() < sr.str();
	}
};


bool is_root(const vector<FuzzyPred> &clauses)
{
	if (clauses.size() && clauses.at(0).pred().begin()->str == "ROOT")
		return true;
	return false;
}

vector<pair<vector<FuzzyPred>, Memory> > get_roots(const vector<pair<vector<FuzzyPred>, Memory> > data)
{
	vector<pair<vector<FuzzyPred>, Memory> > ret_vect;
	vector<pair<vector<FuzzyPred>, Memory> >::const_iterator viter = data.begin();
	while (viter != data.end()) {
		if (is_root(viter->first))
			ret_vect.push_back(*viter);
		++viter;
	}
	return ret_vect;
}



// auxiliary functions for post-processing the parsing

PredTree correct_period(PredTree to_return)
{
	// put the period at the level of the ROOT. It gives a mistake if
	// you do it after the bynarization: it can give you three
	// children below root.

	PredTree::iterator pi(to_return);
	++pi;
	for (; pi != to_return.end(); ++pi) {
		int size = pi->str.size();
		if (pi->str.find("-period-") != string::npos) {
			PredTree subtree(pi);
			to_return.erase(pi);
			to_return.appendTree(to_return.begin(), subtree);
			break;
		}
	}

	return to_return;
}

bool has_verb(PredTree::iterator pi, const PredTree &pt)
{
	for (; pi != pt.end(); ++pi) {
		if (pi->str == "V" || pi->str == "VB" || pi->str == "VBN" || pi->str == "VBG" || pi->str == "MD")
			return true;
	}
	return false;
}

bool has_WRB(PredTree::iterator pi, const PredTree &pt)
{
	for (; pi != pt.end(); ++pi) {
		if (pi->str == "WRB" )
			return true;
	}
	return false;
}


PredTree name_subordinates(PredTree pt)
// Change the name to subordinates to join them correctly in post_process_sbar()
{
	PredTree::iterator pi = pt.begin();
	++pi;
	for (; pi != pt.end(); ++pi) {
		PredTree::iterator to_the_foot(pi);
		for (; to_the_foot.firstChild() != pt.end(); to_the_foot = to_the_foot.firstChild())
			;
		string curr_foot = to_the_foot->str;
		if (pi->str == "SBAR" && curr_foot == "if") {
			pi->str = "SBAR-IF";
		}
		if ( (pi->str == "SBAR" || pi->str == "S") && pi.firstChild()->str == "PP") {
			if (debug) {
				puts("PP_SBAR:::");
			}
			pi->str = "SBAR-PP";
		}
		if (pi->str == "SBAR" && pi.firstChild().firstChild() != pt.end() && pi.firstChild().firstChild()->str == "PP") {
			pi->str = "SBAR-PP";
		}
		if (pi->str == "SBAR" && (pi.firstChild()->str == "WP" || pi.firstChild()->str == "WDT")) {
			pi->str = "SBAR-WP";
		}
		if ( pi->str == "SBAR" && pi.firstChild()->str == "WRB") {
			pi->str = "SBAR-WRB";
		}
		if ( (pi->str == "SBAR"  || pi->str == "S") && pi.firstChild()->str == "WHNP"
			 && !(pi.parent() != pt.end()
					 && (pi.parent()->str == "S"
					 	|| pi.parent()->str.find("PRN") != string::npos
					 	||  pi.parent()->str.find("SBAR") != string::npos ) )
				) {
			pi->str = "SBAR-WHNP";
		}
		if ((pi->str == "SBAR" || pi->str == "SQ" || pi->str == "S")
				&& (pi.firstChild()->str == "CC" || pi.firstChild()->str == "-comma-"
						|| (pi.firstChild().firstChild() != pt.end() && pi.firstChild().firstChild()->str == "CC"))) {
			if (has_verb(pi, pt) || has_WRB(pi,pt))
				pi->str = "SBAR-CC";
			else
				pi->str = "SBAR-CC-NOVERB";
		}
		if (pi->str == "SBAR" && !has_verb(pi, pt)) {
			pi->str = "SBAR-NP";
		}
		if (pi->str == "SBAR" && pi.firstChild()->str == "VBG") {
			pi->str = "SBAR-VP";
		}
		if (pi->str == "PRN" && pi.firstChild()->str == "VP" && pi.firstChild().firstChild() != pt.end()
				&& pi.firstChild().firstChild()->str == "TO") {
			pi->str = "PRN-TO";
		}
		if (pi->str == "SBAR" && pi.firstChild()->str == "VP" && pi.firstChild().firstChild() != pt.end()
				&& pi.firstChild().firstChild()->str == "TO") {
			pi->str = "SBAR-TO";
		}
//          if( pi->str == "S"
//	      && pi.firstChild()->str == "PRN" ) {
//	       pi->str = "SBAR-PRN";
//	  }
		if (pi->str == "PRN" && pi.firstChild()->str == "PP") {
			pi->str = "PRN-PP";
		}
		if (pi->str == "PRN" && pi.firstChild()->str == "-LBR-") {
			pi->str = "PRN-CC";
		}
		if (pi->str == "PRN"
				&& (pi.firstChild()->str == "WP" || pi.firstChild()->str == "WDT"
						|| (pi.firstChild().firstChild() != pt.end()
								&& (pi.firstChild().firstChild()->str == "WP"
										|| pi.firstChild().firstChild()->str == "WDT"))
						|| (pi.firstChild().firstChild() != pt.end()
								&& pi.firstChild().firstChild().firstChild() != pt.end()
								&& (pi.firstChild().firstChild().firstChild()->str == "WP"
										|| pi.firstChild().firstChild().firstChild()->str == "WDT"))
						|| (pi.firstChild().firstChild() != pt.end()
								&& pi.firstChild().firstChild().firstChild() != pt.end()
								&& pi.firstChild().firstChild().firstChild().firstChild() != pt.end()
								&& (pi.firstChild().firstChild().firstChild().firstChild()->str == "WP"
										|| pi.firstChild().firstChild().firstChild().firstChild()->str == "WDT")))) {
			pi->str = "PRN-WP";
		}
	}
	return pt;
}

PredTree clean_subordinates(PredTree pt)
// Restore the name to subordinates after joining them in post_process_sbar()
{
	PredTree::iterator pi = pt.begin();
	++pi;
	for (; pi != pt.end(); ++pi) {
		if (pi.num_children() == 1 && (pi->str == "SBAR-PP") ) { // || pi->str == "PRN-PP")) {
			PredTree tmp(pi.firstChild()); // eliminate single parents as: SBAR-PP(PP(..)) -> PP() /// NOT FOR PRNs!!!
			pt.replace(tmp, pi);
			pi = pt.begin();
			//++pi;
			continue;
		}
		// All the remaining SBAR-PP must be converted back
		if (pi->str == "SBAR-IF") {
			pi->str = "SBAR";
		}
		if (pi->str == "SBAR-PP") {
			pi->str = "SBAR";
		}
		if (pi->str == "SBAR-CC") {
			pi->str = "SBAR";
		}
		if (pi->str == "SBAR-CC-NOVERB") {
			pi->str = "NP";
		}
		if (pi->str == "SBAR-NP") {
			pi->str = "NP";
		}
		if (pi->str == "SBAR-WP") {
			pi->str = "SBAR";
		}
		if (pi->str == "SBAR-VP") {
			pi->str = "VP";
		}
		if (pi->str == "PRN-TO") {
			pi->str = "PRN";
		}
		if (pi->str == "SBAR-TO") {
			pi->str = "SBAR";
		}
		if (pi->str == "PRN-WP") {
			pi->str = "PRN";
		}
		if (pi->str == "SBAR-WRB") {
			pi->str = "SBAR";
		}
		if (pi->str == "SBAR-WHNP") {
			pi->str = "SBAR";
		}
		if (pi->str == "PRN-PP") {
			pi->str = "PRN"; // It cannot become just "PP" because a PRN subordinate has no access to the main sentence verbs
		}
		if (pi->str == "PRN-CC") {
			pi->str = "PRN";
		}
	}
	return pt;
}

vector<string> get_feet(const PredTree &pt)
{
	vector<string> to_return;
	PredTree::height_iterator piter(pt, 0);

	for (; piter != pt.end(); ++piter) {
		to_return.push_back(piter->str);
	}
	return to_return;
}

PredTree move_period_out(const PredTree &pt)
{
	PredTree to_return(pt);
	PredTree::height_iterator piter(to_return, 1);
	PredTree::iterator last_iter;
	last_iter = pt.end();

	for (; piter != pt.end(); ++piter)
		last_iter = piter; // go to the last element

	if (last_iter != pt.end() && last_iter->str == "-period-") {
		if(debug) {
			cout << "PERIOD::: " << endl;
		}
		to_return.erase(last_iter);
		to_return.appendTree(to_return.begin(), Predicate("-period-(-period-)").pred());
	}

	return to_return;
}

bool is_question(PredTree::iterator pi, PredTree::iterator pend)
{
	for (; pi.parent() != pend; pi = pi.parent()) {
		if (pi->str == "SQ")
			return true;
	}

	return false;
}


FuzzyPred post_process_sbar(const Predicate &pred)
{
	PredTree pt(pred.pred());

	if (debug) {
		cout << "SBAR-PP0::: " << Predicate(pt) << endl;
	}
	pt = move_period_out(pt);
	pt = name_subordinates(pt);
	PredTree orig_pt(pt);
	vector<string> orig_feet = get_feet(pt);

	// Simplify the children-parent
	PredTree::iterator pi = pt.begin();
	++pi;
	for (; pi != pt.end(); ++pi) {
		if (pi.num_children() == 1 && (pi->str == "S" || pi->str == "X") && (pi.firstChild()->str == "PRN")) {
			PredTree tmp(pi.firstChild()); // eliminate single parents as: S(NP(NP(...))) -> NP(...)
			pt.replace(tmp, pi);
			pi = pt.begin();
			//++pi;
			continue;
		}
	}

	// Correct the subordinates
	pi = pt.begin();
	++pi;
	PredTree::iterator last_hook = pt.end(), last_hook_of = pt.end();
	string str_tag;
	int depth_tag = 0, depth_old = 0;
	int safe = 0;
	string curr_foot; // The foot in the parsed tree right down the element "pi"
	if (debug) {
		cout << "SBAR-PP::: " << Predicate(pt) << endl;
	}
	for (; pi != pt.end() && safe < 100; ++pi, ++safe) { // corrects PP stray elements

		PredTree::iterator to_the_foot(pi);
		for (; to_the_foot.firstChild() != pt.end(); to_the_foot = to_the_foot.firstChild())
			;
		curr_foot = to_the_foot->str;

		str_tag = pi->str;
		depth_tag = pi.depth();
//
		// decides the depth of the trigger for moving PP subtrees
		// "IN(of)" is always at depth+1
		int depth_trigger;
		if (is_question(pi, pt.end()) || curr_foot == "of" || curr_foot == "that") {
			depth_trigger = depth_tag + 1;
		} else
			depth_trigger = depth_tag + 2;

		if (depth_trigger < depth_old && last_hook != pt.end()) {
			if (str_tag == "SBAR-PP" || str_tag == "PRN-PP") {
				if (curr_foot == "of")
					last_hook = last_hook_of;
				if (pi.lastChild()->str != "-period-") {
					PredTree tmp_tree(pi);
					pt.erase(pi);
					pi = pt.appendTree(last_hook, tmp_tree);
					pi = last_hook;
//					pi = pt.begin();
//					++pi;
					last_hook = pt.end();
					continue;
				} else {
					PredTree tmp_tree(pi);
					tmp_tree.erase(tmp_tree.begin().lastChild());
					PredTree period_tree(pi.lastChild());
					pt.replace(period_tree, pi);
					pi = pt.appendTree(last_hook, tmp_tree);
					pi = last_hook;
//					pi = pt.begin();
//					++pi;
					last_hook = pt.end();
					continue;
				}
			}
		}
		if ((str_tag == "NP" && pi.parent()->str != "PP" // he is on the roof because it is weary
		&& pi.parent()->str != "VP" // they ended the dinasty and adopted republic
		) || str_tag == "VP"
		//|| str_tag == "S"
		//|| (str_tag == "PP" && pi.firstChild()->str == "IN")
				) {
			last_hook = pi;
			last_hook_of = pi;
		}
		if (str_tag == "NP")
			last_hook_of = pi;
		if (str_tag == "SBAR" || str_tag == "ADJP" || str_tag == "ADVP" || str_tag == "PP"
		//    || str_tag == "PRN"
				|| str_tag == "POS" || str_tag == "-RBR-" || str_tag == "-comma-" || str_tag == "-LBR-"
				//|| str_tag == "IN"
				|| str_tag == "CC")
			last_hook = pt.end();
		if (str_tag == "CD-DATE") {
			last_hook = pt.end();
			last_hook_of = pt.end();
		}
		depth_old = depth_tag;
	}

	vector<string> new_feet = get_feet(pt);
	if (new_feet != orig_feet) // check that the order of the feet is the same as the one before the corrections
		pt = orig_pt;
	else
		orig_pt = pt;

	// "after episodes where the state had ..."
	pi = pt.begin();
	++pi;
	last_hook = pt.end();
	depth_tag = 0, depth_old = 0;
	safe = 0;
	for (; pi != pt.end() && safe < 100; ++pi, ++safe) { // corrects PP stray elements
		str_tag = pi->str;
		depth_tag = pi.depth();
		int depth_trigger = depth_tag + 1;

		if (depth_trigger < depth_old && last_hook != pt.end()) {
			if (str_tag == "SBAR-WRB") {
				if (pi.lastChild()->str != "-period-") {
					PredTree tmp_tree(pi);
					pt.erase(pi);
					pi = pt.appendTree(last_hook, tmp_tree);
					pi = last_hook;
//					pi = pt.begin();
//					++pi;
					last_hook = pt.end();
					continue;
				} else {
					PredTree tmp_tree(pi);
					tmp_tree.erase(tmp_tree.begin().lastChild());
					PredTree period_tree(pi.lastChild());
					pt.replace(period_tree, pi);
					pi = pt.appendTree(last_hook, tmp_tree);
					pi = last_hook;
//					pi = pt.begin();
//					++pi;
					last_hook = pt.end();
					continue;
				}
			}
		}
		if ((str_tag == "NP" && pi.parent()->str != "VP") || str_tag == "VP"
		//|| str_tag == "S"
		//|| (str_tag == "PP" && pi.firstChild()->str == "IN")
				)
			last_hook = pi;
		else if (str_tag == "SBAR" || str_tag == "ADJP" || str_tag == "ADVP" || str_tag == "PP"
		//    || str_tag == "PRN"
				|| str_tag == "POS" || str_tag == "-RBR-" || str_tag == "-comma-" || str_tag == "-LBR-"
				//|| str_tag == "IN"
				|| str_tag == "CC")
			last_hook = pt.end();
		depth_old = depth_tag;
	}

	if (debug) {
		cout << "SBAR-PP2::: " << Predicate(pt) << endl;
	}

	new_feet = get_feet(pt);
	if (new_feet != orig_feet) // check that the order of the feet is the same as the one before the corrections
		pt = orig_pt;
	else
		orig_pt = pt;

	pi = pt.begin();
	++pi;
	last_hook = pt.end();
	depth_tag = 0;
	depth_old = 0;
	safe = 0;
	for (; pi != pt.end() && safe < 100; ++pi, ++safe) { // corrects PP stray elements

		str_tag = pi->str;
		depth_tag = pi.depth();

		int depth_trigger = depth_tag + 3;

		if (depth_trigger < depth_old && last_hook != pt.end()) {
			if (str_tag == "SBAR-CC") {
				if (pi.lastChild()->str != "-period-") {
					PredTree tmp_tree(pi);
					pt.erase(pi);
					pi = pt.appendTree(last_hook, tmp_tree);
					pi = last_hook;
//					pi = pt.begin();
//					++pi;
					last_hook = pt.end();
					continue;
				} else {
					PredTree tmp_tree(pi);
					tmp_tree.erase(tmp_tree.begin().lastChild());
					PredTree period_tree(pi.lastChild());
					pt.replace(period_tree, pi);
					pi = pt.appendTree(last_hook, tmp_tree);
					pi = last_hook;
//					pi = pt.begin();
//					++pi;
					last_hook = pt.end();
					continue;
				}
			}
		}
		if ((str_tag == "NP" && pi.parent()->str != "PP" && pi.parent()->str != "S" && pi.parent()->str != "VP"
				&& pi.parent()->str != "ADJP" && pi.parent()->str != "ADVP") // it must not be a complement
		|| str_tag == "VP")
			last_hook = pi;
		else if (str_tag == "SBAR"
		//|| str_tag == "ADJP"
		//|| str_tag == "ADVP"
		//|| str_tag == "PP"
		//    || str_tag == "PRN"
				|| str_tag == "POS" || str_tag == "-RBR-" || str_tag == "-comma-" || str_tag == "-LBR-"
				//|| str_tag == "IN"
						)
			last_hook = pt.end();
		depth_old = depth_tag;
	}

	new_feet = get_feet(pt);
	if (new_feet != orig_feet) // check that the order of the feet is the same as the one before the corrections
		pt = orig_pt;
	else
		orig_pt = pt;

	pi = pt.begin();
	++pi;
	last_hook = pt.end();
	depth_tag = 0;
	depth_old = 0;
	safe = 0;
	for (; pi != pt.end() && safe < 100; ++pi, ++safe) { // corrects PP stray elements

		str_tag = pi->str;
		depth_tag = pi.depth();

		int depth_trigger = depth_tag + 2;

		if (depth_trigger < depth_old && last_hook != pt.end()) {
			if (str_tag == "SBAR-CC-NOVERB") {
				if (pi.lastChild()->str != "-period-") {
					PredTree tmp_tree(pi);
					pt.erase(pi);
					pi = pt.appendTree(last_hook, tmp_tree);
					pi = last_hook;
//					pi = pt.begin();
//					++pi;
					last_hook = pt.end();
					continue;
				} else {
					PredTree tmp_tree(pi);
					tmp_tree.erase(tmp_tree.begin().lastChild());
					PredTree period_tree(pi.lastChild());
					pt.replace(period_tree, pi);
					pi = pt.appendTree(last_hook, tmp_tree);
					pi = last_hook;
//					pi = pt.begin();
//					++pi;
					last_hook = pt.end();
					continue;
				}
			}
		}
		if (str_tag == "NP" || str_tag == "VP")
			last_hook = pi;
		else if (str_tag == "SBAR"
		//|| str_tag == "ADJP"
		//|| str_tag == "ADVP"
		//|| str_tag == "PP"
		//    || str_tag == "PRN"
				|| str_tag == "POS" || str_tag == "-RBR-" || str_tag == "-comma-" || str_tag == "-LBR-"
				//|| str_tag == "IN"
						)
			last_hook = pt.end();
		depth_old = depth_tag;
	}

	new_feet = get_feet(pt);
	if (new_feet != orig_feet) // check that the order of the feet is the same as the one before the corrections
		pt = orig_pt;
	else
		orig_pt = pt;

	pi = pt.begin();
	++pi;
	last_hook = pt.end();
	depth_tag = 0;
	depth_old = 0;
	safe = 0;
	curr_foot = "";
	for (; pi != pt.end() && safe < 100; ++pi, ++safe) { // corrects PP stray elements

		PredTree::iterator to_the_foot(pi);
		for (; to_the_foot.firstChild() != pt.end(); to_the_foot = to_the_foot.firstChild())
			;
		PredTree::height_iterator hi(to_the_foot);
		++hi;
		if (hi != pt.end()) {
			curr_foot = hi->str;
		}

		str_tag = pi->str;
		depth_tag = pi.depth();

		// decides SBAR-CC starting with "also" always join the previous NP or VP
		int depth_trigger;
		bool do_trigger = false;
		if (curr_foot == "also" || str_tag == "PRN-CC") {
			do_trigger = true;
		}
		depth_trigger = depth_tag + 2;

		if (do_trigger && depth_trigger < depth_old && last_hook != pt.end()) {
			if (str_tag == "SBAR-CC" || str_tag == "PRN-CC"
			//|| str_tag == "SBAR-PRN"
					) {
				if (pi.lastChild()->str != "-period-") {
					PredTree tmp_tree(pi);
					pt.erase(pi);
					pi = pt.appendTree(last_hook, tmp_tree);
					pi = last_hook;
//					pi = pt.begin();
//					++pi;
					last_hook = pt.end();
					continue;
				} else {
					PredTree tmp_tree(pi);
					tmp_tree.erase(tmp_tree.begin().lastChild());
					PredTree period_tree(pi.lastChild());
					pt.replace(period_tree, pi);
					pi = pt.appendTree(last_hook, tmp_tree);
					pi = last_hook;
//					pi = pt.begin();
//					++pi;
					last_hook = pt.end();
					continue;
				}
			}
		}
		if (str_tag == "NP" || str_tag == "VP")
			last_hook = pi;
		else if (str_tag == "SBAR" || str_tag == "ADJP" || str_tag == "ADVP"
		//|| str_tag == "PP"
		//    || str_tag == "PRN"
				|| str_tag == "POS" || str_tag == "-RBR-" || str_tag == "-comma-" || str_tag == "-LBR-"
				//|| str_tag == "IN"
						)
			last_hook = pt.end();
		depth_old = depth_tag;
	}

	new_feet = get_feet(pt);
	if (new_feet != orig_feet) // check that the order of the feet is the same as the one before the corrections
		pt = orig_pt;
	else
		orig_pt = pt;

	pi = pt.begin();
	++pi;
	last_hook = pt.end();
	depth_tag = 0;
	depth_old = 0;
	safe = 0;
	for (; pi != pt.end() && safe < 100; ++pi, ++safe) { // corrects NP stray elements
		PredTree::iterator to_the_foot(pi);
		for (; to_the_foot.firstChild() != pt.end(); to_the_foot = to_the_foot.firstChild())
			;
		curr_foot = to_the_foot.parent()->str;

		str_tag = pi->str;
		depth_tag = pi.depth();
		int depth_trigger = depth_tag + 2;
		if (depth_trigger < depth_old && last_hook != pt.end()) {
			if (str_tag == "SBAR-NP" // "Is this story a troll?"
			|| str_tag == "SBAR-WP" || str_tag == "PRN-WP") {
				if (pi.lastChild()->str != "-period-") {
					PredTree tmp_tree(pi);
					pt.erase(pi);
					pi = pt.appendTree(last_hook, tmp_tree);
					pi = last_hook;
//					pi = pt.begin();
//					++pi;
					last_hook = pt.end();
					continue;
				} else {
					PredTree tmp_tree(pi);
					tmp_tree.erase(tmp_tree.begin().lastChild());
					PredTree period_tree(pi.lastChild());
					pt.replace(period_tree, pi);
					pi = pt.appendTree(last_hook, tmp_tree);
					pi = last_hook;
//					pi = pt.begin();
//					++pi;
					last_hook = pt.end();
					continue;
				}
			}
		}
		if ((str_tag == "NP" && pi.parent()->str != "PP" && curr_foot != "DT") // he is on the roof because it is weary
		|| (str_tag == "VP")) {
			last_hook = pi;
		} else if (str_tag == "SBAR" || str_tag == "ADJP" || str_tag == "ADVP" || str_tag == "POS" || str_tag == "-RBR-"
				|| str_tag == "-LBR-") {
			last_hook = pt.end();
		}
		depth_old = depth_tag;
	}

	if (debug) {
		cout << "PRN-WP::: " << Predicate(pt) << endl;
	}

	new_feet = get_feet(pt);
	if (new_feet != orig_feet) // check that the order of the feet is the same as the one before the corrections
		pt = orig_pt;
	else
		orig_pt = pt;

	pi = pt.begin();
	++pi;
	last_hook = pt.end();
	depth_tag = 0;
	depth_old = 0;
	safe = 0;
	for (; pi != pt.end() && safe < 200; ++pi, ++safe) { // corrects NP stray elements
		PredTree::iterator to_the_foot(pi);
		for (; to_the_foot.firstChild() != pt.end(); to_the_foot = to_the_foot.firstChild())
			;
		curr_foot = to_the_foot.parent()->str;

		str_tag = pi->str;
		depth_tag = pi.depth();
		int depth_trigger = depth_tag + 2;

		if(debug)
			cout << "SBAR-WHNP:: "
				<< str_tag << " "
				<< depth_trigger << " " << depth_old << endl;

		if (depth_trigger < depth_old && last_hook != pt.end()) {
			if (str_tag == "SBAR-WHNP") {
				if (pi.lastChild()->str != "-period-") {
					PredTree tmp_tree(pi);
					pt.erase(pi);
					pi = pt.appendTree(last_hook, tmp_tree);
					pi = last_hook;
					last_hook = pt.end();
					continue;
				} else {
					PredTree tmp_tree(pi);
					tmp_tree.erase(tmp_tree.begin().lastChild());
					PredTree period_tree(pi.lastChild());
					pt.replace(period_tree, pi);
					pi = pt.appendTree(last_hook, tmp_tree);
					pi = last_hook;
					//					pi = pt.begin();
					//					++pi;
					last_hook = pt.end();
					continue;
				}
			}
		}
		if ((str_tag == "NP" ) // he is on the roof because it is weary
				|| (str_tag == "VP")) {
			last_hook = pi;
		} else if (str_tag == "SBAR" || str_tag == "ADJP" || str_tag == "ADVP" || str_tag == "POS" || str_tag == "-RBR-"
				|| str_tag == "-LBR-") {
			last_hook = pt.end();
		}
		depth_old = depth_tag;
	}

	if (debug) {
		cout << "PRN-WP2::: " << Predicate(pt) << endl;
	}

	new_feet = get_feet(pt);
	if (new_feet != orig_feet) // check that the order of the feet is the same as the one before the corrections
		pt = orig_pt;
	else
		orig_pt = pt;

	pi = pt.begin();
	++pi;
	last_hook = pt.end();
	depth_tag = 0;
	depth_old = 0;
	safe = 0;
	for (; pi != pt.end() && safe < 100; ++pi, ++safe) { // corrects NP stray elements
		PredTree::iterator to_the_foot(pi);
		for (; to_the_foot.firstChild() != pt.end(); to_the_foot = to_the_foot.firstChild())
			;
		curr_foot = to_the_foot.parent()->str;

		str_tag = pi->str;
		depth_tag = pi.depth();
		int depth_trigger = depth_tag + 2;
		if (depth_trigger < depth_old && last_hook != pt.end()) {
			if (str_tag == "PRN-WP") {
				if (pi.lastChild()->str != "-period-") {
					PredTree tmp_tree(pi);
					pt.erase(pi);
					pi = pt.appendTree(last_hook, tmp_tree);
					pi = last_hook;
//					pi = pt.begin();
//					++pi;
					last_hook = pt.end();
					continue;
				} else {
					PredTree tmp_tree(pi);
					tmp_tree.erase(tmp_tree.begin().lastChild());
					PredTree period_tree(pi.lastChild());
					pt.replace(period_tree, pi);
					pi = pt.appendTree(last_hook, tmp_tree);
					pi = last_hook;
//					pi = pt.begin();
//					++pi;
					last_hook = pt.end();
					continue;
				}
			}
		}
		if (str_tag == "WHNP") {
			last_hook = pi;
		} else if (str_tag == "SBAR" || str_tag == "ADJP" || str_tag == "ADVP" || str_tag == "POS" || str_tag == "-RBR-"
				|| str_tag == "-comma-" || str_tag == "-LBR-" || str_tag == "CC")
			last_hook = pt.end();
		depth_old = depth_tag;
	}

	new_feet = get_feet(pt);
	if (new_feet != orig_feet) // check that the order of the feet is the same as the one before the corrections
		pt = orig_pt;
	else
		orig_pt = pt;

	pi = pt.begin();
	++pi;
	last_hook = pt.end();
	depth_tag = 0;
	depth_old = 0;
	safe = 0;
	for (; pi != pt.end() && safe < 100; ++pi, ++safe) { // corrects VP stray elements
		str_tag = pi->str;
		depth_tag = pi.depth();
		int depth_trigger = depth_tag + 2;
		if (depth_trigger < depth_old && last_hook != pt.end()) {
			if (str_tag == "SBAR-VP") {
				if (pi.lastChild()->str != "-period-") {
					PredTree tmp_tree(pi);
					pt.erase(pi);
					pi = pt.appendTree(last_hook, tmp_tree);
					pi = last_hook;
//					pi = pt.begin();
//					++pi;
					last_hook = pt.end();
					continue;
				} else {
					PredTree tmp_tree(pi);
					tmp_tree.erase(tmp_tree.begin().lastChild());
					PredTree period_tree(pi.lastChild());
					pt.replace(period_tree, pi);
					pi = pt.appendTree(last_hook, tmp_tree);
					pi = last_hook;
//					pi = pt.begin();
//					++pi;
					last_hook = pt.end();
					continue;
				}
			}
		}

		if (str_tag == "VP") {
			last_hook = pi;
		} else if (str_tag == "SBAR" || str_tag == "CC" || str_tag == "PRN" || str_tag == "IN" || str_tag == "-comma-"
				|| str_tag == "-RBR-" || str_tag == "-LBR-")
			last_hook = pt.end();
		depth_old = depth_tag;
	}

	new_feet = get_feet(pt);
	if (new_feet != orig_feet) // check that the order of the feet is the same as the one before the corrections
		pt = orig_pt;
	else
		orig_pt = pt;

	// the only section of the track (PRN to come above the ground) is ...
	pi = pt.begin();
	++pi;
	last_hook = pt.end();
	depth_tag = 0;
	depth_old = 0;
	safe = 0;
	for (; pi != pt.end() && safe < 100; ++pi, ++safe) { // corrects VP stray elements
		str_tag = pi->str;
		depth_tag = pi.depth();
		int depth_trigger = depth_tag + 2;
		if (depth_trigger < depth_old && last_hook != pt.end()) {
			if (str_tag == "PRN-TO" || str_tag == "SBAR-TO") {
				if (pi.lastChild()->str != "-period-") {
					PredTree tmp_tree(pi);
					pt.erase(pi);
					pi = pt.appendTree(last_hook, tmp_tree);
					pi = last_hook;
//					pi = pt.begin();
//					++pi;
					last_hook = pt.end();
					continue;
				} else {
					PredTree tmp_tree(pi);
					tmp_tree.erase(tmp_tree.begin().lastChild());
					PredTree period_tree(pi.lastChild());
					pt.replace(period_tree, pi);
					pi = pt.appendTree(last_hook, tmp_tree);
					pi = last_hook;
//					pi = pt.begin();
//					++pi;
					last_hook = pt.end();
					continue;
				}
			}
		}

		if ((str_tag == "NP" && pi.parent()->str != "PP" && pi.parent()->str != "VP") || str_tag == "VP") {
			last_hook = pi;
		} else if (str_tag == "SBAR" || str_tag == "CC" || str_tag == "PRN" || str_tag == "-comma-" || str_tag == "-RBR-"
				|| str_tag == "-LBR-")
			last_hook = pt.end();
		depth_old = depth_tag;
	}

	new_feet = get_feet(pt);
	if (new_feet != orig_feet) // check that the order of the feet is the same as the one before the corrections
		pt = orig_pt;
	else
		orig_pt = pt;

	// Some verbs join to the next subordinate (let, make, ...)
	pi = pt.begin();
	++pi;
	last_hook = pt.end();
	depth_tag = 0;
	depth_old = 0;
	safe = 0;
	for (; pi != pt.end() && safe < 100; ++pi, ++safe) { // corrects NP stray elements
		PredTree::iterator to_the_foot(pi);
		for (; to_the_foot.firstChild() != pt.end(); to_the_foot = to_the_foot.firstChild())
			;
		curr_foot = to_the_foot.parent()->str;

		str_tag = pi->str;
		depth_tag = pi.depth();
		int depth_trigger = depth_tag + 2;
		if (depth_trigger < depth_old && last_hook != pt.end()) {
			if (str_tag == "SBAR") { // "if Britain let this energy revolution pass ..."
				if (pi.lastChild()->str != "-period-") {
					PredTree tmp_tree(pi);
					pt.erase(pi);
					pi = pt.appendTree(last_hook, tmp_tree);
					pi = last_hook;
//					pi = pt.begin();
//					++pi;
					last_hook = pt.end();
					continue;
				} else {
					PredTree tmp_tree(pi);
					tmp_tree.erase(tmp_tree.begin().lastChild());
					PredTree period_tree(pi.lastChild());
					pt.replace(period_tree, pi);
					pi = pt.appendTree(last_hook, tmp_tree);
					pi = last_hook;
//					pi = pt.begin();
//					++pi;
					last_hook = pt.end();
					continue;
				}
			}
		}
		if (str_tag == "VP" && pi.firstChild()->str == "V" && (pi.firstChild().firstChild()->str == "let" // Linking verbs
		|| pi.firstChild().firstChild()->str == "make" || pi.firstChild().firstChild()->str == "made")) {
			last_hook = pi;
		} else if (str_tag == "SBAR" || str_tag == "ADJP" || str_tag == "ADVP" || str_tag == "POS" || str_tag == "-RBR-"
				|| str_tag == "-comma-" || str_tag == "-LBR-" || str_tag == "CC")
			last_hook = pt.end();
		depth_old = depth_tag;
	}

	new_feet = get_feet(pt);
	if (new_feet != orig_feet) // check that the order of the feet is the same as the one before the corrections
		pt = orig_pt;
	else
		orig_pt = pt;


	// sometimes there is a case like "NP())),-comma-(-comma-),NP(...) )" -> "NP(,-comma-(-comma-),NP(...) ) )))
	pi = pt.begin();
	++pi;
	last_hook = pt.end();
	depth_tag = 0;
	depth_old = 0;
	safe = 0;
	for (; pi != pt.end() && safe < 100; ++pi, ++safe) { // corrects PP stray elements

		str_tag = pi->str;
		depth_tag = pi.depth();

		int depth_trigger = depth_tag + 3;

		if(debug) {
			cout << "SBAR_-comma-:::" << str_tag << endl;
		}

		if (depth_trigger < depth_old && last_hook != pt.end() ) {
			if (str_tag == "-comma-" && pi.parent()->str != "NP" ) {
				if (pi.nextSibling() != pt.end() && pi.nextSibling()->str == "NP") {
					PredTree tmp_tree(pi);
					PredTree tmp_tree2(pi.nextSibling());
					pt.erase(pi.nextSibling() );
					pt.erase(pi);
					pi = pt.appendTree(last_hook, tmp_tree );
					pi = pt.appendTree(last_hook, tmp_tree2);
					pi = last_hook;
					last_hook = pt.end();
					continue;
				}
			}
		}
		if (str_tag == "NP" )
			last_hook = pi;
		else if (str_tag == "SBAR"
				|| str_tag == "POS" || str_tag == "-RBR-"
				|| str_tag == "-LBR-"
		)
			last_hook = pt.end();
		depth_old = depth_tag;
	}

	new_feet = get_feet(pt);
	if (new_feet != orig_feet) // check that the order of the feet is the same as the one before the corrections
		pt = orig_pt;
	else
		orig_pt = pt;


	pt = clean_subordinates(pt);

	if (debug) {
		cout << "SBAR-PP3::: " << Predicate(pt) << endl;
	}

	return pt;
}

FuzzyPred apply_corrections_questions(FuzzyPred pred)
{
	vector<std::pair<FuzzyPred, FuzzyPred> > candidates;
	candidates.push_back(make_pair(FuzzyPred("ROOT(SBARQ(WHNP(WP(what)),SQ(S(NP(_A),_B),-QM-(?))))"),
							 FuzzyPred("ROOT(SBARQ(WHNP(WP(what),NP(_A)),SQ(_B),-QM-(?))))")));
	candidates.push_back(make_pair(FuzzyPred("ROOT(SBARQ(_A,VP(V(_B),SQ(WHNP(_C),NP(_D)),_E)))"),
							 FuzzyPred("ROOT(SBARQ(_A,SQ(VP(V(_B),NP(_C,_D),_E))))")));
	candidates.push_back(make_pair(FuzzyPred("ROOT(SBARQ(_A,VP(V(_B),SQ(WHNP(_C),NP(_D1,_D2)),_E)))"),
							 FuzzyPred("ROOT(SBARQ(_A,SQ(VP(V(_B),NP(_C,_D1,_D2),_E))))")));
	candidates.push_back(make_pair(FuzzyPred("ROOT(SBARQ(_A,VP(V(_B),SQ(WHNP(_C),NP(_D1,_D2,_D3)),_E)))"),
							 FuzzyPred("ROOT(SBARQ(_A,SQ(VP(V(_B),NP(_C,_D1,_D2,_D3),_E))))")));
	candidates.push_back(make_pair(FuzzyPred("ROOT(SBARQ(_A,VP(V(_B),SQ(WHNP(_C),NP(_D)),_E)))"),
							 FuzzyPred("ROOT(SBARQ(_A,SQ(VP(V(_B),NP(_C,_D),_E))))")));
	candidates.push_back(make_pair(FuzzyPred("ROOT(S(VP(_A,ADJP(N(_B),J(_C))),-QM-(?)))"), FuzzyPred("ROOT(SQ(VP(_A,NP(N(_B)),ADJP(J(_C))),-QM-(?)))")));
	candidates.push_back(make_pair(FuzzyPred("ROOT(S(VP(_A,ADJP(N(_B),J(_C))),-QM-(?)))"), FuzzyPred("ROOT(SQ(VP(_A,NP(N(_B)),ADJP(J(_C))),-QM-(?)))")));
	candidates.push_back(make_pair(FuzzyPred("ROOT(S(PRN(WHNP(_A),_B,VP(_C,_D)),_E,-QM-(?)))"), FuzzyPred("ROOT(SQ(SBARQ(WHNP(_A,_B)),VP(_C,_D,_E),-QM-(?)))")));
	candidates.push_back(make_pair(FuzzyPred("ROOT(S(PRN(WHNP(_A),_B,VP(_C,_D)),_E,-QM-(?)))"), FuzzyPred("ROOT(SQ(SBARQ(WHNP(_A,_B)),VP(_C,_D,_E),-QM-(?)))")));
	candidates.push_back(make_pair(FuzzyPred("ROOT(S(PRN(S(WHNP(_A),S(_B,VP(_C,_D)))),VP(_E,_F),-QM-(?))))"),FuzzyPred("ROOT(SQ(SBARQ(WHNP(_A,_B)),VP(_C,_D,VP(_E,_F)),-QM-(?)))")));
	candidates.push_back(make_pair(FuzzyPred("ROOT(VP(_A,NP(_B,NP(_C,-QM-(?)))))"), FuzzyPred("ROOT(VP(_A,NP(_B),NP(_C),-QM-(?)))")));
	candidates.push_back(make_pair(FuzzyPred("ROOT(VP(_A,NP(_B,NP(S(_C,-QM-(?))))))"), FuzzyPred("ROOT(VP(_A,NP(_B),NP(_C),-QM-(?)))")));
	candidates.push_back(make_pair(FuzzyPred("ROOT(S(PRN(SBARQ(_A,SQ(VP(_B,NP(_C))))),VP(_D),-QM-(?)))"),FuzzyPred("ROOT(SBARQ(_A,SQ(VP(_B,NP(_C),VP(_D)),-QM-(?))))")));
	candidates.push_back(make_pair(FuzzyPred("ROOT(VP(_A,NP(_B,VB(_C)),_D))"), FuzzyPred("ROOT(SQ(VP(_A,NP(_B),VP(_C),_D)))")));
	candidates.push_back(make_pair(FuzzyPred("ROOT(S(PP(WRB(_A),S(NP(_B),_C)),-QM-(?)))"), FuzzyPred("ROOT(SBARQ(WHNP(WRB(_A),NP(_B)),SQ(_C),-QM-(?)))")));
	candidates.push_back(make_pair(FuzzyPred("ROOT(SBARQ(SBARQ(WHADVP(WRB(_A)),SQ(_B)),SQ(_C,-QM-(?))))"),FuzzyPred("ROOT(SBARQ(WHADVP(WRB(_A),SQ(VP(_B,VP(_C)),-QM-(?)))))")));
	candidates.push_back(make_pair(FuzzyPred("ROOT(VP(_A,NP(_B,VB(_C)),_D))"), FuzzyPred("ROOT(SQ(VP(_A,NP(_B),VP(_C),_D)))")));
	candidates.push_back(make_pair(FuzzyPred("ROOT(SBARQ(_A,VP(_B,ADJP(N(_C),J(_D))),_E))"), FuzzyPred("ROOT(SBARQ(_A,SQ(VP(_B),NP(N(_C)),NP(J(_D))),_E))")));
	candidates.push_back(make_pair(FuzzyPred("ROOT(SBARQ(SBARQ(WHADVP(WRB(_B)),VP(_C,_D)),SQ(VP(VBN(_E)),_F)))"),FuzzyPred("ROOT(SBARQ(WHADVP(WRB(_B)),SQ(VP(_C),NP(_D),VP(VBN(_E)),_F)))")));
	candidates.push_back(make_pair(FuzzyPred("ROOT(PP(WHADVP(WRB(_A)),S(NP(_B),VP(_C),-QM-(_D))))"),FuzzyPred("ROOT(SBARQ(WHADVP(WRB(_A),NP(_B)),SQ(VP(_C)),-QM-(_D)))")));
	candidates.push_back(make_pair(FuzzyPred("ROOT(PP(WHADVP(WRB(_A)),S(NP(_B1,_B2),VP(_C),-QM-(_D))))"),FuzzyPred("ROOT(SBARQ(WHADVP(WRB(_A),NP(_B1,_B2)),SQ(VP(_C)),-QM-(_D)))")));
	candidates.push_back(make_pair(FuzzyPred("ROOT(PP(WHADVP(WRB(_A)),S(NP(_B1,_B2,_B3),VP(_C),-QM-(_D))))"),FuzzyPred("ROOT(SBARQ(WHADVP(WRB(_A),NP(_B1,_B2,_B3)),SQ(VP(_C)),-QM-(_D)))")));
	candidates.push_back(make_pair(FuzzyPred("ROOT(PP(WHADVP(WRB(_A)),S(NP(_B),_C)))"), FuzzyPred("ROOT(SBARQ(WHADVP(WRB(_A),NP(_B)),SBARQ(VP(_C))))")));
	candidates.push_back(make_pair(FuzzyPred("ROOT(PP(WHADVP(WRB(_A)),S(NP(_B,_D),_C)))"), FuzzyPred("ROOT(SBARQ(WHADVP(WRB(_A),NP(_B,_D)),SBARQ(VP(_C))))")));
	candidates.push_back(make_pair(FuzzyPred("ROOT(PP(WHADVP(WRB(_A)),S(NP(_B,_D,_E),_C)))"),FuzzyPred("ROOT(SBARQ(WHADVP(WRB(_A),NP(_B,_D,_E)),SBARQ(VP(_C))))")));
	candidates.push_back(make_pair(FuzzyPred("ROOT(SBARQ(_F,VP(V(_A),NP(DT(_B),_C),NP(_D)),-QM-(_E)))"),FuzzyPred("ROOT(SBARQ(_F,VP(V(_A),NP(DT(_B),_C,_D)),-QM-(_E)))")));
	candidates.push_back(make_pair(FuzzyPred("ROOT(SBARQ(WHNP(_D),SQ(VP(V(_A),_B,_C,IN(of)),-QM-(?))))"),FuzzyPred("ROOT(SBARQ(WHNP(_D),SQ(VP(V(_A),_B,NP(_C,IN(of))),-QM-(?))))")));
	candidates.push_back(make_pair(FuzzyPred("ROOT(SBARQ(WHNP(_A,_B),SQ(VP(_C,NP(_D,_E),NP(_F)),_G)))"),FuzzyPred("ROOT(SBARQ(WHNP(_A,_B),SQ(VP(_C,NP(_D,_E,_F)),_G)))")));
	candidates.push_back(make_pair(FuzzyPred("ROOT(SBARQ(WHNP(_A,_B),SQ(VP(_C,NP(_D,_E1,_E2),NP(_F)),_G)))"),FuzzyPred("ROOT(SBARQ(WHNP(_A,_B),SQ(VP(_C,NP(_D,_E1,_E2,_F)),_G)))")));
	candidates.push_back(make_pair(FuzzyPred("ROOT(SBARQ(_A),SBARQ(_B),_C)"),FuzzyPred("ROOT(SBARQ(_A,_B),_C)")));
	candidates.push_back(make_pair(FuzzyPred("ROOT(SBARQ(SBARQ(_C,VP(_B,_D)),PP(_A),-QM-(?)))"),FuzzyPred("ROOT(SBARQ(_C,SQ(VP(VP(_B,_D),PP(_A)),-QM-(?))))")));
	candidates.push_back(make_pair(FuzzyPred("ROOT(SBARQ(SBARQ(_C,VP(_B,_D,_E)),PP(_A),-QM-(?)))"),FuzzyPred("ROOT(SBARQ(_C,SQ(VP(VP(_B,_D,_E),PP(_A)),-QM-(?))))")));
	candidates.push_back(make_pair(FuzzyPred("ROOT(SQ(VP(_A1,NP(_B,PP(_C,_D))),-QM-(?)))"),FuzzyPred("ROOT(SQ(VP(_A1,NP(_B),PP(_C,_D)),-QM-(?)))")));
	candidates.push_back(make_pair(FuzzyPred("ROOT(SBARQ(WHNP(_F),SQ(VP(_E,NP(_D,_C,_B),NP(_A))),-QM-(?)))"),FuzzyPred("ROOT(SBARQ(WHNP(_F),SQ(VP(_E,NP(_D,_C,_B,_A))),-QM-(?)))")));
	candidates.push_back(make_pair(FuzzyPred("ROOT(SBARQ(WHNP(_F),VP(_E,NP(_D,_C,_B),NP(_A)),-QM-(?)))"),FuzzyPred("ROOT(SBARQ(WHNP(_F),VP(_E,NP(_D,_C,_B,_A)),-QM-(?)))")));
	candidates.push_back(make_pair(FuzzyPred("ROOT(SBARQ(SBARQ(WHNP(_A),SQ(_B,_C,VP(_D))),PP(TO(to)),-QM-(?)))"),FuzzyPred("ROOT(SBARQ(WHNP(_A),SQ(_B,_C,VP(VP(_D),PP(TO(to))))),-QM-(?))")));
	candidates.push_back(make_pair(FuzzyPred("ROOT(SBARQ(SBARQ(WHNP(_A),SQ(_B,_C,VP(_D))),PP(TO(to)),-QM-(?)))"),FuzzyPred("ROOT(SBARQ(WHNP(_A),SQ(_B,_C,VP(VP(_D),PP(TO(to))))),-QM-(?))")));
	candidates.push_back(make_pair(FuzzyPred("ROOT(FRAC(SBARQ(WHNP(_A),VP(_B,_C,PRN(_D))),_E))"),FuzzyPred("ROOT(SBARQ(WHNP(_A),SQ(VP(_B,_C,PRN(_D),_E))))")));

	//ROOT:h(SBAR:h(WHNP:c(WRB:a(how-many),NNS:a(dogs)),S:h(VP:h(VBP:h(are),NP:c(EX:c(there)),SBAR:c(PP:h(IN:h(in),NP:c(DT:a(the),JJ:c(united),NNS:h(states))),-period-:a(?))))))
	candidates.push_back(make_pair(FuzzyPred("ROOT(SBAR(WHNP(_A,_B),S(VP(_C,_D,SBAR(_E,-QM-(?))))))"),
			FuzzyPred("ROOT(SBARQ(WHNP(_A,_B),SQ(VP(_C,_D,SBAR(_E,-QM-(?))))))")));
//	ROOT:h(SBARQ:h(WHNP:h(WHADJP:h(WRB:h(how-many),JJ:c(pet)),NNS:a(dogs)),SQ:c))
	candidates.push_back(make_pair(FuzzyPred("ROOT(SBARQ(WHNP(WHADJP(WRB(_A),_B),_C),_D))"),FuzzyPred("ROOT(SBARQ(WHNP(WRB(_A),_B,_C),_D))")));
// ROOT(NP(NP(DT(_A1),NN(_A2)),WHPP(IN(_A3),WHNP(WP(_A4))),S(NP(NN(_A5)),VP(VBZ(is),VP(VBN(striped))))),-period-(?))
	candidates.push_back(make_pair(FuzzyPred("ROOT(NP(NP(DT(_A1),N(_A2)),WHPP(IN(_A3),WHNP(WP(_A4))),S(NP(_A5),_A6)),_A7)"),
				FuzzyPred("ROOT(NP(NP(DT(_A1),NN(_A2)),WHPP(IN(_A3),WHNP(WP(_A4),_A5)),S(_A6)),_A7)")));


	vector<pair<FuzzyPred, FuzzyPred> >::iterator citer = candidates.begin();
	vector<pair<FuzzyPred, FuzzyPred> >::iterator cend = candidates.end();
	for(int ncycle=0; ncycle < 2; ++ncycle) { // apply all the rules twice
		for (; citer != cend; ++citer) {
			if (debug) {
				cout << pred << endl;
				cout << citer->first << endl;
			}
			Upg upg;
			//if(citer->first.unify(pred, &upg)) {
			if (pred.unify(citer->first, &upg)) {
				pred = citer->second;
				pred / upg;
				if (debug) {
					cout << upg << endl;
				}
				break;
			}
		}
	}
	return pred;
}

FuzzyPred apply_corrections(FuzzyPred pred, const vector<Clause> &sub_candidates)
{
	vector<pair<FuzzyPred, FuzzyPred> > candidates;
	candidates.push_back(make_pair(FuzzyPred("ROOT(S(PP(_A,NP(_B)),S(VP(_C,_D),_G)))"),FuzzyPred("ROOT(S(PP(_A,S(NP(_B),VP(_C),_D))))")));
	candidates.push_back(make_pair(FuzzyPred("ROOT(S(PP(_A,NP(_B,_E)),S(VP(_C,_D),_G)))"),FuzzyPred("ROOT(S(PP(_A,S(NP(_B,_E),VP(_C),_D))))")));
	candidates.push_back(make_pair(FuzzyPred("ROOT(S(PP(_A,NP(_B,_E,_F)),S(VP(_C,_D),_G)))"),FuzzyPred("ROOT(S(PP(_A,S(NP(_B,_E,_F),VP(_C,_D),_G))))")));
	candidates.push_back(make_pair(FuzzyPred("ROOT(S(NP(NP(_A),VP(_B),_C),_D))"),FuzzyPred("ROOT(S(NP(NP(_A),VP(VP(_B),_C)),_D))")));
	candidates.push_back(make_pair(FuzzyPred("ROOT(S(NP(NP(_A),VP(_B),_C),_D))"),FuzzyPred("ROOT(S(NP(NP(_A),VP(VP(_B),_C)),_D))")));

	vector<pair<FuzzyPred, FuzzyPred> >::iterator citer = candidates.begin();
	vector<pair<FuzzyPred, FuzzyPred> >::iterator cend = candidates.end();
	for (; citer != cend; ++citer) {
		Upg upg;
		if (pred.unify(citer->first, &upg)) {
			if (debug) {
				cout << "CORRECTIONS::: " << citer->second << endl;
			}
			pred = citer->second;
			pred / upg;
			break;
		}
	}

	// replace each data
	vector<Clause>::const_iterator citer2 = sub_candidates.begin();
	vector<Clause>::const_iterator cend2 = sub_candidates.end();
	PredTree tree = pred.pred();
	PredTree orig_pt(tree);
	vector<string> orig_feet = get_feet(tree);
	const int safe_max = 50;
	for (; citer2 != cend2; ++citer2) {
		PredTree from_pred(citer2->getHypothesis().at(0).pred() );
		PredTree to_pred(citer2->getConsequence().pred() );
		if (debug) {
			cout << "CORR:: " << Predicate(tree) << endl;
			cout << Predicate(to_pred) << " :- " << Predicate(from_pred) << endl;
		}
		PredTree::iterator pi = tree.begin();
		int safe = 0;
		for (; pi != tree.end() && safe < safe_max; ++pi, ++safe) {
			FuzzyPred tmp_pred(pi);
			Upg upg;
			bool has_unified = tmp_pred.unify(from_pred, &upg);
			if (debug) {
				cout << tmp_pred << endl;
				cout << Predicate(from_pred) << endl;
				cout << "UPG::: " << upg << " " << has_unified << endl;
			}
			if (!has_unified) {
				continue;
			}
			if (debug) {
				cout << "CORR2001:: " << Predicate(pred) << endl;
			}
			to_pred / upg;
			if (debug) {
				cout << "CORR2002:: " << Predicate(pred) << endl;
			}
			if (debug) {
				cout << "CORR201:: " << Predicate(pred) << endl;
			}
			pi = tree.replace(to_pred, pi);
			if (debug) {
				cout << "CORR202:: " << Predicate(pred) << endl;
			}
		}
		if (debug) {
			cout << "CORR21:: " << Predicate(pred) << endl;
		}
		vector<string> new_feet = get_feet(tree);
		if (debug) {
			cout << "CORR22:: " << Predicate(pred) << endl;
		}
		if (new_feet != orig_feet) // check that the order of the feet is the same as the one before the corrections
			tree = orig_pt;
		else
			orig_pt = tree;
	}
	pred.pred() = tree;

	if (debug) {
		cout << "CORR3:: " << Predicate(pred) << endl;
	}

	return pred;
}

PredTree correct_tree(PredTree &pt)
{
	PredTree::iterator pi = pt.begin();
	++pi;
	for (; pi != pt.end(); ++pi) {
		if (pi.num_children() == 1 && (pi->str == "S" || pi->str == "SQ" || pi->str == "NP" || pi->str == "X")
				&& (pi.firstChild()->str == "NP" || pi.firstChild()->str == "S" || pi.firstChild()->str == "ADJP"
						|| pi.firstChild()->str == "VP" || pi.firstChild()->str == "PP" || pi.firstChild()->str == "POS"
						|| pi.firstChild()->str == "CC") && pi.parent()->str != "ROOT") {
			PredTree tmp(pi.firstChild()); // eliminate single parents as: S(NP(NP(...))) -> NP(...)
			pt.replace(tmp, pi);
			pi = pt.begin();
			//++pi;
			continue;
		}
		if (pi.num_children() == 1 && pi->str == pi.firstChild()->str && pi->str == "ADJP") {
			PredTree tmp(pi.firstChild()); // eliminate single parents as: S(NP(NP(...))) -> NP(...)
			pt.replace(tmp, pi);
			pi = pt.begin();
			//++pi;
			continue;
		}
		if (pi->str == "NP" && pi.num_children() == 2 && pi.height() > 0 && pi.firstChild()->str == "N"
				&& pi.lastChild()->str == "VP") {
			PredTree first_child(pi.firstChild());
			PredTree NPtree("NP");
			NPtree.appendTree(NPtree.begin(), first_child);
			pt.replace(NPtree, pi.firstChild());
			pi = pt.begin();
			//++pi;
			continue;
		}
		if (pi->str == "PP" && pi.num_children() == 2 && pi.height() > 0 && pi.firstChild()->str == "IN"
				&& pi.lastChild()->str == "NP" && pi.nextSibling() != pt.end() && pi.nextSibling().num_children() == 1
				&& pi.nextSibling()->str == "VP") {
			// repair cases like PP( *PP(IN(...),NP(...)), VP(...) )
			PredTree NP_child(pi.lastChild());
			PredTree VP_child(pi.nextSibling());
			PredTree Stree("S");
			Stree.appendTree(Stree.begin(), NP_child);
			Stree.appendTree(Stree.begin(), VP_child);
			pt.replace(Stree, pi.lastChild());
			pt.erase(pi.nextSibling());
			pi = pt.begin();
			//++pi;
			continue;
		}

		if (pi.num_children() == 2 && !(pi.firstChild()->str == "NP" && pi.lastChild()->str == "VP") && (pi->str == "S")
				&& (pi.parent()->str != "PP") // it must not be a subordinate
				&& (pi.parent()->str != "WHPP") // it must not be a subordinate
				&& (pi.parent()->str != "S") // it must not be a subordinate
				&& !(pi.parent()->str == "VP" && pi.parent().firstChild()->str == "VBG") // "assuming he am correct" (VBG subordinate)
				&& pi.parent()->str != "ADVP" // "Then he needs this"
				&& (pi.parent()->str != "ROOT") // it must not be a subordinate
				&& pi == pi.parent().lastChild()
				&& pi.firstChild()->str != "CC"
				//&& pi.lastChild()->str != "-period-"
				//&& pi.lastChild()->str != "-QM-"
						) { // for cases like: ROOT(..., S( NP(...), ADJP(...) ) -> ROOT(..., NP(...), ADJP(...) )
			PredTree::iterator parent = pi.parent();
			PredTree child1(pi.firstChild());
			PredTree child2(pi.lastChild());
			pt.erase(pi);
			pt.appendTree(parent, child1);
			pt.appendTree(parent, child2);
			pi = pt.begin();
			//++pi;
			continue;
		}
		if (pi.num_children() == 2 && (pi->str == "NP") && pi == pi.parent().lastChild() && pi.parent()->str != "PP"
				&& pi.parent()->str != "WHPP" && pi.parent()->str != "WHNP" && pi.firstChild()->str == "NP"
				&& pi.lastChild()->str == "VP") {
			PredTree::iterator parent = pi.parent();
			PredTree child1(pi.firstChild());
			PredTree child2(pi.lastChild());
			pt.erase(pi);
			pt.appendTree(parent, child1);
			pt.appendTree(parent, child2);
			pi = pt.begin();
			//++pi;
			continue;
		}
		if (pi.num_children() == 2 && pi == pi.parent().lastChild() && pi->str == "S" && pi.firstChild()->str == "PP"
				&& pi.lastChild()->str == "VP") { // for cases like: ROOT(S( PP(...), VP(...) ) -> ROOT(PP(...), VP(...) )
			PredTree::iterator parent = pi.parent();
			PredTree child1(pi.firstChild());
			PredTree child2(pi.lastChild());
			pt.erase(pi);
			pt.appendTree(parent, child1);
			pt.appendTree(parent, child2);
			pi = pt.begin();
			//++pi;
			continue;
		}
		if (pi.num_children() == 2 && pi->str == "VP" && pi.firstChild()->str == "V"
				&& pi.firstChild().nextSibling()->str == "ADJP") {
			pi.firstChild().nextSibling()->str = "NP";
			continue;
		}
		if (pi.num_children() == 3
				//&& pi == pi.parent().lastChild()
				&& pi->str == "NP" && pi.firstChild()->str == "NP" && pi.firstChild().nextSibling()->str == "VP"
				&& pi.lastChild()->str == "PP") { // for cases like: ROOT(S( PP(...), VP(...) ) -> ROOT(PP(...), VP(...) )
			pi->str = "S";
			pi = pt.begin();
			//++pi;
			continue;
		}
		if (pi.num_children() == 2 && pi == pi.parent().lastChild() && pi->str == "S"
				&& (pi.firstChild()->str == "ADJP" || pi.firstChild()->str == "PP" || pi.firstChild()->str == "NP")
				&& pi.lastChild()->str == "-period-") {
			PredTree::iterator parent = pi.parent();
			PredTree child1(pi.firstChild());
			PredTree child2(pi.lastChild());
			pt.erase(pi);
			pt.appendTree(parent, child1);
			pt.appendTree(parent, child2);
			pi = pt.begin();
			//++pi;
			continue;
		}
		if (pi.num_children() == 2 && pi == pi.parent().lastChild() && pi->str == "S" && pi.parent()->str == "VP" && pi.firstChild()->str == "NP"
				&& pi.lastChild()->str == "VP") { // for cases like: VP(V,S( NP(...), VP(...) ) ->  VP(V,NP(...), VP(...))
			PredTree::iterator parent = pi.parent();
			PredTree child1(pi.firstChild());
			PredTree child2(pi.lastChild());
			pt.erase(pi);
			pt.appendTree(parent, child1);
			pt.appendTree(parent, child2);
			pi = pt.begin();
			//++pi;
			continue;
		}
		if (pi.num_children() == 2 && pi == pi.parent().lastChild() && pi->str == "VP" && pi.parent()->str == "VP" && pi.firstChild()->str == "NP"
				&& pi.lastChild()->str == "NP") { // for cases like: VP(V,VP( NP(...), NP(...) ) ->  VP(V,NP(...), NP(...))
			PredTree::iterator parent = pi.parent();
			PredTree child1(pi.firstChild());
			PredTree child2(pi.lastChild());
			pt.erase(pi);
			pt.appendTree(parent, child1);
			pt.appendTree(parent, child2);
			pi = pt.begin();
			//++pi;
			continue;
		}
		if (pi.num_children() == 2 && pi == pi.parent().lastChild() && pi->str == "S" && pi.firstChild()->str == "NP"
				&& pi.lastChild()->str == "-QM-") { // for cases like: ROOT(S( PP(...), VP(...) ) -> ROOT(PP(...), VP(...) )
			PredTree::iterator parent = pi.parent();
			PredTree child1(pi.firstChild());
			PredTree child2(pi.lastChild());
			pt.erase(pi);
			pt.appendTree(parent, child1);
			pt.appendTree(parent, child2);
			pi = pt.begin();
			//++pi;
			continue;
		}
		if (pi.num_children() == 2 && pi == pi.parent().lastChild() && pi->str == "S" && pi.firstChild()->str == "ADJP"
				&& pi.lastChild()->str == "S" && pi.lastChild().firstChild() != pt.end()
				&& pi.lastChild().firstChild()->str == "VP") { // for cases like: ROOT(S( ADJP(...), S(VP(...)) ) -> ROOT(S( NP(...), VP(VP(...)) )
			pi.firstChild()->str = "NP";
			pi.lastChild()->str = "VP";
			//++pi;
			continue;
		}
//          if( pi.num_children() == 2
//              && pi == pi.parent().lastChild()
//	      && pi->str == "S"
//	      && pi.firstChild()->str == "VP"
//	      && pi.lastChild()->str == "-period-"
//	      )  { // for cases like: ROOT(S( PP(...), VP(...) ) -> ROOT(PP(...), VP(...) )
//	       PredTree::iterator parent= pi.parent();
//	       PredTree child1(pi.firstChild());
//	       PredTree child2(pi.lastChild());
//	       pt.erase(pi);
//	       pt.appendTree(parent, child1);
//	       pt.appendTree(parent, child2);
//	       pi= pt.begin();
//	       //++pi;
//	       continue;
//	  }
		if (pi.num_children() == 2 && pi->str == "NP" && pi == pi.parent().lastChild()
				&& (pi.parent()->str == "PP" || pi.parent()->str == "WHNP") && pi.firstChild()->str == "NP"
				&& pi.lastChild()->str == "VP") {
			pi->str = "S";
			continue;
		}
		if (pi->str == "PP" && pi.firstChild()->str == "VBG" && pi.parent()->str == "VP") {
			pi->str = "VP";
			continue;
		}
		//cout << "PRNTREE::: " << PredTree(pi) << endl;
		if (pi.num_children() == 3 && pi.firstChild()->str == "NP"
				&& (pi.firstChild().nextSibling()->str == "PRN" || pi.firstChild().nextSibling()->str == "SBAR")
				&& pi.firstChild().nextSibling().nextSibling()->str == "VP") {
			PredTree NP_child(pi.firstChild());
			PredTree PRN_child(pi.firstChild().nextSibling());
			PredTree::iterator next_iter = pi.firstChild().nextSibling();
			PredTree NPtree("NP");
			NPtree.appendTree(NPtree.begin(), NP_child);
			NPtree.appendTree(NPtree.begin(), PRN_child);
			pt.replace(NPtree, pi.firstChild());
			pt.erase(next_iter);
			pi = pt.begin();
			continue;
		}
	}

	return pt;
}

FuzzyPred post_process(const Predicate &pred)
{
	PredTree pt(pred.pred());

	if(debug) {
		cout << "POST_PROC::: " << pred << endl;
	}

	pt = move_period_out(pt);
	pt = correct_tree(pt);
	if(debug) {
		cout << "POST_PROC2::: " << Predicate(pt) << endl;
	}

	PredTree orig_pt(pt);
	vector<string> orig_feet = get_feet(pt);

	PredTree::iterator pi = pt.begin();
	++pi;
	PredTree::iterator last_hook = pt.end();
	string str_tag;
	int depth_tag = 0, depth_old = 0;
	int safe = 0;
	string curr_foot; // The foot in the parsed tree right down the element "pi"
	for (; pi != pt.end() && safe < 100; ++pi, ++safe) { // corrects NP stray elements ( ( this (seemed)) (a mistake) )
		PredTree::iterator to_the_foot(pi);
		for (; to_the_foot.firstChild() != pt.end(); to_the_foot = to_the_foot.firstChild())
			;
		curr_foot = to_the_foot.parent()->str;

		str_tag = pi->str;
		depth_tag = pi.depth();
		if (depth_tag > 0 && depth_tag + 1 < depth_old && last_hook != pt.end()) {
			if ((str_tag == "NP" || str_tag == "WHNP" || str_tag == "N" || str_tag == "J"
					|| (!is_question(pi, pt.end()) && (str_tag == "ADJP" || str_tag == "ADVP")))
			//&& pi == pi.parent().lastChild()
			) {
				if (pi.lastChild()->str != "-period-") {
					PredTree tmp_tree(pi);
					pt.erase(pi);
					pi = pt.appendTree(last_hook, tmp_tree);
					pi = pt.begin();
					++pi;
					last_hook = pt.end();
					continue;
				} else {
					PredTree tmp_tree(pi);
					tmp_tree.erase(tmp_tree.begin().lastChild());
					PredTree period_tree(pi.lastChild());
					pt.replace(period_tree, pi);
					pi = pt.appendTree(last_hook, tmp_tree);
					pi = pt.begin();
					++pi;
					last_hook = pt.end();
					continue;
				}
			}
		}
		if (str_tag == "VP" || (str_tag == "NP"
		//&& pi.parent()->str != "PP"
				&& pi.parent()->str != "VP"   /// check this!!
				&& (curr_foot != "DT") // "Is this story a troll?"
		) || (str_tag == "PP" && pi.firstChild()->str == "IN" && pi.firstChild().nextSibling() == pt.end())
		//|| str_tag == "ADVP"
				)
			last_hook = pi;
		if (str_tag == "SBAR"
				//|| str_tag == "PP"
				|| str_tag == "PRN" //|| str_tag == "PRP"
				|| str_tag == "RB" || str_tag == "CC" || str_tag == "CD-DATE" || str_tag == "N-PLACE" || str_tag == "VBG"
				|| str_tag == "-comma-" || str_tag == "CONJP" || str_tag == "ADVP" || str_tag == "ADJP" || str_tag == "WDT"
				|| str_tag == "WHPP" || str_tag == "WHNP" || str_tag == "JJR" || str_tag == "POS" || str_tag == "-RBR-"
				|| str_tag == "-LBR-")
			last_hook = pt.end();
		depth_old = depth_tag;
	}

	vector<string> new_feet = get_feet(pt);
	if (new_feet != orig_feet) // check that the order of the feet is the same as the one before the corrections
		pt = orig_pt;
	else
		orig_pt = pt;

	// the following is to ensure indirect object are parsed correctly
	pi = pt.begin();
	++pi;
	last_hook = pt.end();
	depth_tag = 0;
	depth_old = 0;
	safe = 0;
	for (; pi != pt.end() && safe < 100; ++pi, ++safe) { // corrects PP stray elements
		PredTree::iterator to_the_foot(pi);
		for (; to_the_foot.firstChild() != pt.end(); to_the_foot = to_the_foot.firstChild())
			;
		curr_foot = to_the_foot->str;

		str_tag = pi->str;
		depth_tag = pi.depth();

		// decides the depth of the trigger for moving PP subtrees
		// "IN(of)" is always at depth+1
		int depth_trigger;
		depth_trigger = depth_tag + 2;

		if (depth_trigger < depth_old && last_hook != pt.end()) {
			if ((str_tag == "NP" && (curr_foot == "a" || curr_foot == "the" || curr_foot == "that" ///
			))) {
				if (pi.lastChild()->str != "-period-") {
					PredTree tmp_tree(pi);
					pt.erase(pi);
					pi = pt.appendTree(last_hook, tmp_tree);
					pi = last_hook;
//					pi = pt.begin();
//					++pi;
					last_hook = pt.end();
					continue;
				} else {
					PredTree tmp_tree(pi);
					tmp_tree.erase(tmp_tree.begin().lastChild());
					PredTree period_tree(pi.lastChild());
					pt.replace(period_tree, pi);
					pi = pt.appendTree(last_hook, tmp_tree);
					pi = last_hook;
					//pi = pt.begin();
					//++pi;
					last_hook = pt.end();
					continue;
				}
			}
		}
		if (str_tag == "VP")
			last_hook = pi;
		if (str_tag == "SBAR"
				//|| str_tag == "PP"
				|| str_tag == "PRN" || str_tag == "PRP"
				//|| str_tag == "RB"
				|| str_tag == "CC" || str_tag == "CD-DATE" || str_tag == "N-PLACE" || str_tag == "VBG" || str_tag == "-comma-"
				|| str_tag == "CONJP" || str_tag == "ADVP" || str_tag == "ADJP"
				//|| str_tag == "WDT"
				|| str_tag == "WHPP" || str_tag == "WHNP"
				//|| str_tag == "JJR"
				//|| str_tag == "POS"
				|| str_tag == "-RBR-" || str_tag == "-LBR-")
			last_hook = pt.end();
		depth_old = depth_tag;
	}

	new_feet = get_feet(pt);
	if (new_feet != orig_feet) // check that the order of the feet is the same as the one before the corrections
		pt = orig_pt;
	else
		orig_pt = pt;

	pi = pt.begin();
	++pi;
	last_hook = pt.end();
	depth_tag = 0;
	depth_old = 0;
	safe = 0;
	int height_tag= -1, height_old = -1;
	bool is_the_article = false;
	for (; pi != pt.end() && safe < 100; ++pi, ++safe) { // corrects PP stray elements
		PredTree::iterator to_the_foot(pi);
		for (; to_the_foot.firstChild() != pt.end(); to_the_foot = to_the_foot.firstChild())
			;
		curr_foot = to_the_foot->str;

		str_tag = pi->str;
		depth_tag = pi.depth();
		height_tag = pi.height();
		// decides the depth of the trigger for moving PP subtrees
		// "IN(of)" is always at depth+1
		int depth_trigger, height_trigger;
		if (is_question(pi, pt.end()) || curr_foot == "of" || curr_foot == "on" || curr_foot == "over" || curr_foot == "than"
				|| curr_foot == "by" || is_the_article) {
			depth_trigger = depth_tag + 1;
			height_trigger = height_tag + 1;
		} else {
			depth_trigger = depth_tag + 2;
			height_trigger = height_tag + 2;
		}

          if(debug) {
               cout << "DEPTH_TR::: " << depth_trigger << ", " << depth_old
               	<< ", " << height_trigger << ", " << height_old
                    << ", " << curr_foot
                    << ", " << str_tag
                    << ", " << (pi.firstChild() != pt.end() ? pi.firstChild()->str : "END")
                    << ", " << (pi.firstChild() != pt.end() ? pi.num_children() : -1)
                    << ", " << (last_hook != pt.end() ? "VALID" : "NOT VALID")
                    << endl;
          }

		if ( (depth_trigger < depth_old
			  || (curr_foot == "of" && depth_trigger == depth_old && height_trigger > height_old) )
				&& last_hook != pt.end()) {
			if(debug) {
				cout << "DEPTH_TR2::: " << endl;
			}
			if ((pi.num_children() > 1 && (str_tag == "PP" || str_tag == "WHPP")) // the num_children() >1 is due to final PP(IN) in questions
			|| (curr_foot == "of" && (str_tag == "NP" || str_tag == "IN"))) {
				if(debug) {
					cout << "DEPTH_TR3::: " << endl;
				}
				is_the_article = false;
				if (pi.lastChild()->str != "-period-") {
					PredTree tmp_tree(pi);
					pt.erase(pi);
					pi = pt.appendTree(last_hook, tmp_tree);
					if(debug) {
						cout << "PT1::: " << Predicate(pt) << endl;
						cout << "PT11::: " << Predicate(last_hook) << endl;
					}
					pi = last_hook;
//					pi = pt.begin();
//					++pi;
					last_hook = pt.end();

					continue;
				} else {
					if (debug) {
						puts("OF:::");
					}
					PredTree tmp_tree(pi);
					tmp_tree.erase(tmp_tree.begin().lastChild());
					PredTree period_tree(pi.lastChild());
					pt.replace(period_tree, pi);
					pi = pt.appendTree(last_hook, tmp_tree);
					pi = last_hook;
//					pi = pt.begin();
//					++pi;
					last_hook = pt.end();
					continue;
				}
			}
		}
		if ( //( pi.num_children() > 1 && (str_tag == "NP" || str_tag == "WHNP") ) // the num_children() >1 is due to final PP(IN) in questions
				(str_tag == "VP")
				|| (str_tag == "ADJP" && pi.firstChild()->str == "JJR")
				|| (str_tag == "ADJP" && pi.firstChild()->str == "RBR")
				|| (str_tag == "ADJP" && pi.firstChild().nextSibling() != pt.end()
						&& pi.firstChild().nextSibling()->str == "RBR")) {
			last_hook = pi;
			is_the_article = false;
		}
		if ((str_tag == "NP" || str_tag == "WHNP" || str_tag == "ADJP") && pi.parent()->str != "NP") {
			if (curr_foot == "the")
				is_the_article = true;
			last_hook = pi;
		}
		if (str_tag == "PP" && (pi.firstChild()->str == "IN" || pi.firstChild()->str == "TO" || pi.firstChild()->str == "PP")
				&& pi.parent()->str != "PP") {
			last_hook = pi;
		}

		else if (str_tag == "SBAR"
				|| ( (str_tag == "NP" || str_tag == "WHNP") && pi.parent()->str == "NP" )
		//|| str_tag == "ADJP"
		//|| str_tag == "ADVP"
		//    || str_tag == "PRN"
		//|| str_tag == "POS"
				|| str_tag == "CD-DATE" || str_tag == "-comma-" || str_tag == "-RBR-" || str_tag == "-LBR-"
				//|| str_tag == "IN"
						)
			last_hook = pt.end();
		depth_old = depth_tag;
		height_old = height_tag;
	}


	if(debug) {
		cout << "PT::: " << Predicate(pt) << endl;
	}

	new_feet = get_feet(pt);
	if (new_feet != orig_feet) // check that the order of the feet is the same as the one before the corrections
		pt = orig_pt;
	else
		orig_pt = pt;

	pi = pt.begin();
	++pi;
	last_hook = pt.end(); // VP can attach to ROOT
	depth_tag = 0;
	depth_old = 0;
	safe = 0;
	for (; pi != pt.end() && safe < 100; ++pi, ++safe) { // corrects VP stray elements
		str_tag = pi->str;
		depth_tag = pi.depth();
		if (depth_tag + 2 < depth_old && last_hook != pt.end()) {
			if ((str_tag == "VP" && pi.parent()->str != "VP") || str_tag == "PRT") {
				if (pi.lastChild()->str != "-period-") {
					PredTree tmp_tree(pi);
					pt.erase(pi);
					pi = pt.appendTree(last_hook, tmp_tree);
					pi = last_hook;
//					pi = pt.begin();
//					++pi;
					last_hook = pt.end();
					continue;
				} else {
					PredTree tmp_tree(pi);
					tmp_tree.erase(tmp_tree.begin().lastChild());
					PredTree period_tree(pi.lastChild());
					pt.replace(period_tree, pi);
					pi = pt.appendTree(last_hook, tmp_tree);
					pi = last_hook;
//					pi = pt.begin();
//					++pi;
					last_hook = pt.end();
					continue;
				}
			}
		}
		if (str_tag == "VP" //&& pi.parent()->str != "NP"
				)
			last_hook = pi;
		if (str_tag == "SBAR" || str_tag == "CC" || str_tag == "PRN" || str_tag == "IN" || str_tag == "-comma-"
				|| str_tag == "-RBR-" || str_tag == "-LBR-")
			last_hook = pt.end();
		depth_old = depth_tag;
	}

	pt = correct_tree(pt);     // apply correction again

	new_feet = get_feet(pt);
	if (new_feet != orig_feet) // check that the order of the feet is the same as the one before the corrections
		pt = orig_pt;
	else
		orig_pt = pt;

	if(debug) {
		cout << "AFTER_POST_PROC::: " << Predicate(pt) << endl;
	}

	return FuzzyPred(pt);
}



// auxiliary functions for finding PRN subordinates 

bool is_opening_par(const FuzzyPred &pred)
{
	if (pred == FuzzyPred("-LBR-(-LBR-)") || pred == FuzzyPred("\"(\")"))
		return true;
	return false;
}

bool is_closing_par(const FuzzyPred &pred)
{
	if (pred == FuzzyPred("-RBR-(-RBR-)") || pred == FuzzyPred("\"(\")"))
		return true;
	return false;
}

vector<pair<string, vector<FuzzyPred> > > extract_prn(vector<FuzzyPred> *data, int *num_prn)
{
	vector<FuzzyPred> preds(*data);
	vector<pair<string, vector<FuzzyPred> > > ret_prns;
	vector<FuzzyPred>::iterator piter = preds.begin();
	vector<FuzzyPred>::iterator pend = preds.end();

	vector<FuzzyPred>::iterator start, end;
	bool trigger_prn = false; /// there is only one nesting level!
	int num = 0;
	while (piter != pend) {
		if (trigger_prn && is_closing_par(*piter)) {
			string prn_str = "prn" + boost::lexical_cast<string>(++num);
			end = piter;
			ret_prns.push_back(make_pair(prn_str, vector<FuzzyPred>(start, end + 1)));
			piter = preds.erase(start, end + 1);
			preds.insert(piter, FuzzyPred("PRN(" + prn_str + ")"));
			piter = preds.begin();
			pend = preds.end();
			trigger_prn = false;
			continue;
		}
		if (is_opening_par(*piter)) {
			start = piter;
			trigger_prn = true;
		}
		++piter;
	}
	//print_vector(preds);
	if (ret_prns.size()) {
		vector<FuzzyPred> tmp = ret_prns.front().second;
		*data = preds;
		*num_prn = num;
	}
	return ret_prns;
}

bool has_opening_nested(vector<FuzzyPred>::iterator ipred, vector<FuzzyPred> &data)
{
	vector<FuzzyPred>::iterator iorig(ipred);
	bool verb_trigger = false, closing_trigger = false;

	if (*ipred == FuzzyPred("-comma-(-comma-)")) {
		while (ipred != data.end()) {
			string head_str = extract_header(*ipred);
			if (head_str == "V")
				verb_trigger = true;
			else if (head_str == "-comma-")
				closing_trigger = true;
			else if (verb_trigger && closing_trigger)
				return true;
			++ipred;
		}
	}
	ipred = iorig;
	string pred_str = extract_header(*ipred);
	if (pred_str == "WH" || (pred_str == "IN" && boost::next(ipred)->pred().begin()->str == "WDT")
			|| (pred_str == "TO" && boost::next(ipred)->pred().begin()->str == "WP") || pred_str == "WDT" || pred_str == "WH$"
			//|| pred_str == "WRB"
					) {
		while (ipred != data.end()) {
			string head_str = extract_header(*ipred);
			if (head_str == "V") {
				return true;
			}
			++ipred;
		}
	}
	return false;
}

bool has_closing_nested(vector<FuzzyPred>::iterator ipred, vector<FuzzyPred> &data)
{
	bool verb_trigger = false, opening_trigger = false;
	if (*ipred == FuzzyPred("-comma-(-comma-)")) {
		return true;
	}
	return false;
}

bool is_opening_nested(vector<FuzzyPred>::iterator ipred, vector<FuzzyPred> &data, const FuzzyPred &open_item, bool is_question_)
{
	// if(ipred == data.begin())
	// 	  return false;
	vector<FuzzyPred>::iterator iorig(ipred);
	string pred_str = extract_header(*ipred);

	if(debug && ipred != data.begin() && boost::next(ipred) != data.end()) {
		cout << "AND_IN:::"
			<< " " << pred_str
			<< " " << boost::prior(ipred)->pred().begin()->str
			<< " " << boost::next(ipred)->pred().begin()->str
			<< endl;
	}


	if (pred_str == "IN" && ipred != data.begin()
		&& boost::prior(ipred)->pred().begin()->str == "CC"
		&& boost::next(ipred) != data.end()
		&& boost::next(ipred)->pred().begin()->str == "PRP"
		//&& boost::next(ipred)->pred().begin().firstChild()->str == "they"
	) {
		if(debug) {
			cout << "AND_IN:::" << endl;
		}
		return true;
	}


	// The man going to the cinema is happy
	ipred = iorig;
	if (ipred == data.begin() && pred_str == "DT" ) {
		while (ipred != data.end()) {
			++ipred;
			if (ipred == data.end())
				break;
			string head_str = extract_header(*ipred);
			if (head_str == "VBG")
				return true;
			if (head_str != "N" && head_str != "J" && head_str != "DT" && head_str != "PRP" && head_str != "PRP$"
					&& head_str != "WDT"
					&& head_str != "CC" //
					&& head_str != "IN" /// check this!
			) {
				return false;
			}

		}
	}

	ipred = iorig;
	if (extract_header(*ipred) == "IN" && ipred != data.begin() && boost::prior(ipred)->pred().begin()->str == "V")
		return false;

	if (extract_header(*ipred) == "PRP")
		return false;

	if (*ipred == FuzzyPred("IN(if)")
		&& !(ipred == data.begin()
		     || ( ipred != data.begin() && *boost::prior(ipred) == FuzzyPred("IN(that)") )
		     || ( ipred != data.begin() && *boost::prior(ipred) == FuzzyPred("-comma-(-comma-)") )
		)
	)
		return false;

	if ( (*ipred == FuzzyPred("IN(that)") || *ipred == FuzzyPred("WDT(that)") )
		&& (ipred != data.begin()
		    && (boost::prior(ipred)->pred().begin()->str == "CC"
		        || boost::prior(ipred)->pred().begin()->str == "-comma-")
		)
	)
		return false;

	if (*ipred == FuzzyPred("IN(because)") && ipred != data.begin())
		return false;
	if (pred_str == "-comma-" && boost::next(ipred) != data.end() && *boost::next(ipred) == FuzzyPred("IN(if)")
			&& ipred != data.begin())
		return false;
	if (extract_header(*ipred) == "IN" && open_item.pred().begin()->str == "VBG")
		return false; // a worker participating in the society is fine

	bool verb_trigger = false, closing_trigger = false;

	if (*ipred == FuzzyPred("-comma-(-comma-)") && boost::next(ipred) != data.end()
			&& !(*boost::next(ipred) == FuzzyPred("CC(and)"))) {
		while (ipred != data.end()) {
			string head_str = extract_header(*ipred);
			if (head_str == "V" || head_str == "VBN")
				verb_trigger = true;
			else if (head_str == "-comma-")
				closing_trigger = true;
			else if (verb_trigger && closing_trigger)
				return true;
			++ipred;
		}
	}
	if (debug) {
		cout << "WHILE_OPENING:::" << *iorig << endl;
	}

	ipred = iorig;
	if (pred_str == "IN" && *ipred == FuzzyPred("IN(for)") && ipred != data.begin()
			&& boost::prior(ipred)->pred().begin()->str == "N" && boost::next(ipred) != data.end()
			&& boost::next(ipred)->pred().begin()->str == "VBG") {
		return true;
	}


	if (pred_str == "TO" && boost::next(ipred) != data.end() && boost::next(ipred)->pred().begin()->str == "VB"
			&& !(open_item == FuzzyPred("IN(if)")
					|| open_item == FuzzyPred("IN(because)")
					|| open_item.pred().begin()->str == "TO"
					|| open_item.pred().begin()->str == "WP"
					|| open_item.pred().begin()->str == "WDT"
			)
	) {
		return true;
	}

	ipred = iorig;
	if (ipred == data.begin()) { // "I go there to swim if I feel better"
		while (ipred != data.end()) {
			if (*ipred == FuzzyPred("IN(if)") || *ipred == FuzzyPred("IN(because)") || *ipred == FuzzyPred("IN(after)")
					|| *ipred == FuzzyPred("IN(as)")
					)
				return true;
			++ipred;
		}
	}



	ipred = iorig;
	if ((ipred != data.begin() && pred_str == "WDT" && boost::prior(ipred)->pred().begin()->str != "IN"
			&& !(open_item == FuzzyPred("IN(if)")) // "if he does not take the steps the people will doubt him."
			&& !(open_item == FuzzyPred("IN(because)")))
			|| (pred_str == "IN" && boost::next(ipred) != data.end() && boost::next(ipred)->pred().begin()->str == "WDT")
			|| (pred_str == "IN" && !(*ipred == FuzzyPred("IN(of)") || *ipred == FuzzyPred("IN(into)"))
					&& !(open_item == FuzzyPred("IN(of)") || open_item == FuzzyPred("IN(into)")
							|| open_item == FuzzyPred("IN(if)") || open_item == FuzzyPred("IN(because)")
							|| open_item == FuzzyPred("IN(for)") || open_item == FuzzyPred("IN(that)")
							|| open_item == FuzzyPred("TO(to)") || open_item == FuzzyPred("WDT(that)")
							|| open_item == FuzzyPred("WDT(which)")
					)
			)
			//|| open_item == FuzzyPred("WDT(that)")
			|| (pred_str == "TO" && boost::next(ipred) != data.end() && boost::next(ipred)->pred().begin()->str == "WP")
			|| pred_str == "WH"
			//|| ( *ipred == FuzzyPred("VBG(including)") )
			|| pred_str == "WH$"
			//|| pred_str == "WRB"
			|| ( pred_str == "WP" && !(is_question_ && ipred == data.begin()) ) // a question can start with "what"
			|| (pred_str == "VBG" && ipred != data.begin() && boost::prior(ipred)->pred().begin()->str == "N")
			|| (pred_str == "VBG" && ipred != data.begin() && boost::prior(ipred)->pred().begin()->str == "RB")
			|| (pred_str == "VBG" && ipred == data.begin()) // wearing a mask is illegal
			) {
		if (debug) {
			if (ipred != data.begin())
				cout << "VBG:: " << *ipred << ", " << boost::prior(ipred)->pred().begin()->str << endl;
			else
				cout << "VBG:: " << *ipred << ", " << extract_header(*ipred) << endl;
		}
		while (ipred != data.end()) {
			++ipred;
			if (ipred == data.end())
				break;
			string head_str = extract_header(*ipred);
			string next_str = "";
			string next_next_str = "";
			if (boost::next(ipred) != data.end())
				next_str = boost::next(ipred)->pred().begin()->str;
			if (boost::next(ipred) != data.end() && boost::next(boost::next(ipred)) != data.end())
				next_next_str = boost::next(boost::next(ipred))->pred().begin()->str;
			if (debug) {
				cout << "VBG3::" << head_str << " " << next_str << " " << next_next_str << endl;
			}
			if (head_str == "V"
					|| (head_str == "VB" && pred_str == "WDT") // that be
					|| (head_str == "MD" && next_str == "VB")
					|| (head_str == "MD" && next_str == "RB" && next_next_str == "VB"))
				return true;
			if (head_str != "N" && head_str != "J" && head_str != "DT" && head_str != "PRP" && head_str != "PRP$"
					&& head_str != "WDT"
					&& head_str != "CC" //
					&& head_str != "IN" /// check this!
							) {
				break;
				//return false;
			}

		}
	}



	//cout << "Opening::: "<< *ipred << endl;
	ipred = iorig;
	pred_str = extract_header(*ipred);
	if (pred_str == "IN" && *ipred == FuzzyPred("IN(of)")) {
		while (ipred != data.end()) {
			string head_str = extract_header(*ipred);
			if (head_str == "VBG") {
				return true;
			}
			if (head_str != "N" && head_str != "J" && head_str != "DT" && head_str != "PRP" && head_str != "PRP$"
					&& head_str != "IN" // check this!
							) {
				return false;
			}
			++ipred;
		}
	}
	return false;
}
