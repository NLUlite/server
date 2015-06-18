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



#include"phrase.hpp"
#include"tools.hpp"

using std::cout;
using std::endl;

const bool debug = false;
const bool measure_time = false;

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

vector<FuzzyPred> phrase::get_tag_preds() const
{
	vector<FuzzyPred> to_return;
	for (int n = 0; n < tags_.size(); ++n) {
		FuzzyPred tmppred(tags_.at(n) + "(" + names_.at(n) + ")");
		to_return.push_back(tmppred);
	}
	return to_return;
}

bool phrase::operator ==(const phrase &rhs) const
{
	PredTree lhs_pt = this->parsed;
	PredTree rhs_pt = rhs.parsed;
	PredTree::iterator liter = lhs_pt.begin(), lend = lhs_pt.end();
	PredTree::iterator riter = rhs_pt.begin(), rend = rhs_pt.end();

	for (; liter != lend && riter != rend; ++liter, ++riter) {
		if (*liter != *riter )
			return false;
	}
	return true;
}

static bool has_hashtag(const string &str)
{
	if (str.find("#") != string::npos)
		return true;
	return false;
}


void phrase::compute_names(const FuzzyPred &pred)
{
	PredTree pt(pred.pred());
	PredTree::height_iterator li(pt, 0);

	tagger *tagg = parser_singleton::get_tagger_instance();

	names_.clear();
	base_text_ = "";
	for (; li != pred.pred().end(); ++li) {
		string tag = li.parent()->str;
		string name = li->str;
		string tmp = tagg->get_info()->get_conj(name, tag);
		if (tmp == "") {
			if(has_hashtag(name) ) {
				string noun = name.substr(0,name.find("#"));
				string hash = name.substr(name.find("#"),name.size() );
				tmp = tagg->get_info()->get_conj(noun, tag);
				if(tmp != "")
					tmp += "#" + hash;
				else
					tmp = name;
			} else
				tmp = name;
		}

		base_text_ += name + " ";
		// check if it is a comma introducing a PRN
		if (li->str == "-comma-") {
			PredTree::height_iterator to_the_head = li.parent();
			while (to_the_head.parent() != pt.end()) {
				if (to_the_head->str.find("[ignore]") != string::npos)
					break;
				if (to_the_head->str.find("PRN") != string::npos) {
					tmp += "[prn]";
					break;
				}
				to_the_head = to_the_head.parent();
			}
		}
		names_.push_back(tmp);
	}
}

void phrase::compute_prn_depths(const FuzzyPred &pred)
{
	PredTree pt(pred.pred());
	PredTree::height_iterator li(pt, 0);

	prn_depths_.clear();

	for (; li != pred.pred().end(); ++li) {
		bool is_under_prn= false;
		PredTree::height_iterator to_the_head = li.parent();
		while (to_the_head.parent() != pt.end()) {
			if (to_the_head->str.find("PRN") != string::npos) {
				is_under_prn= true;
				break;
			}
			to_the_head = to_the_head.parent();
		}
		if(is_under_prn)
			prn_depths_.push_back(1);
		else
			prn_depths_.push_back(0);
	}
}


void phrase::compute_tags(const FuzzyPred &pred)
{
	PredTree pt(pred.pred());
	PredTree::height_iterator li(pt, 1);

	tags_.clear();
	for (; li != pred.pred().end(); ++li) {
		//std::cout << (*li)<< std::endl;
		tags_.push_back(li->str);
	}
}

void phrase::restore_names(const FuzzyPred &pred)
{
	PredTree pt(pred.pred());
	PredTree::height_iterator li(pt, 0);

	names_.clear();
	for (; li != pred.pred().end(); ++li) {
		names_.push_back(li->str);
	}
}

void phrase::compute_constit(const composition &comp)
{
	FTree<constituents> pt(comp.tree());
	FTree<constituents>::height_iterator li(pt, 1);

	constit_.clear();
	for (; li != pt.end(); ++li) {
		//std::cout << (*li)<< std::endl;
		constit_.push_back(*li);
	}
}

static void mark_heads(PredTree &parsed, PhraseInfo *pinfo)
{
	// Following the instructions in CGIbank manual

	// The first term is the parent, the second is the candidate head under the head
	vector<tuple<string, string, direction> > head_tags;
	head_tags = pinfo->getHeadTags();

	int max_depth = parsed.height();

	int n;
	bool stop;
	for (n = 0; n < max_depth - 1; ++n) {
		PredTree::depth_iterator di(parsed, n);
		for (; di != parsed.end(); ++di) {
			vector<tuple<string, string, direction> >::iterator hi = head_tags.begin();
			stop = false;
			//std::cout << (*di) << std::endl;
			for (; hi != head_tags.end() && !stop; ++hi) { // Search for heads in the siblings of di
				string trigger = hi->get<0>();
				if (di->str.substr(0, trigger.size()) == trigger && !stop) {
					if (hi->get<2>() == right) {
						PredTree::children_iterator si_r(di);
						for (; si_r != parsed.end(); ++si_r) {
							if (hi->get<1>() == si_r->str) {
								//std::cout << "r--" << (*si_r) << std::endl;
								si_r->str += ":h"; // It is a head!!
								stop = true;
								break;
							}
						}
					} else if (hi->get<2>() == left) {
						PredTree::children_iterator si_l(di);
						vector<PredTree::children_iterator> reversed_items;
						for (; si_l != parsed.end(); ++si_l)
						     reversed_items.push_back(si_l); // vector with the list of sibligs
						// this vector is parsed from right to left
						for (int sn=reversed_items.size()-1; sn >= 0 ; --sn) { 
						     PredTree::children_iterator iter= reversed_items.at(sn);
						     if (hi->get<1>() == iter->str) {
							  //std::cout << "l--" <<(*si_l) << std::endl;
							  iter->str += ":h"; // It is a head!!
							  stop = true;
							  break;
						     }
						}
					}
				}
			}
		}
	}
	if (parsed.height() > 1) {
		PredTree::iterator pi = parsed.begin();
		pi->str += ":h";
	}
}

static void mark_complements_or_adjunct(PredTree &parsed, PhraseInfo *pinfo)
{
	// Following the instructions in CGIbank manual

	// The first term is the parent, the second is the candidate complement under the head
	vector<pair<string, string> > compl_tags;
	compl_tags = pinfo->getComplementTags();

	int max_depth = parsed.height();

	int n;
	for (n = 0; n < max_depth; ++n) {
		PredTree::depth_iterator di(parsed, n);
		for (; di != parsed.end(); ++di) {
			vector<pair<string, string> >::iterator ci = compl_tags.begin();
			for (; ci != compl_tags.end(); ++ci) { // Search for complements and adj in the siblings of di
				string trigger = ci->first;
				if (di->str.substr(0, trigger.size()) == trigger) {
					PredTree::children_iterator si(di);
					for (; si != parsed.end(); ++si) {
						int size = si->str.size();
						if (si.height() > 0 && si->str.find(":") == std::string::npos) {
							if (ci->second == si->str) {
								si->str += ":c"; // It is a complement!!
							}
						}
					}
				}
			}
		}
	}

	PredTree::iterator pi(parsed);
	++pi;
	for (; pi != parsed.end(); ++pi) {
		int size = pi->str.size();
		if (pi.height() > 0 && pi->str.find(":") == std::string::npos)
			pi->str += ":a"; // It is an adjunct!!
		else if (pi->str.find("RB:") == 0)
			pi->str = "RB:a"; // for situations like: VP(V:h(did),RB:a(not))
		else if (pi->str.find("DT:") != std::string::npos && pi.parent().num_children() == 2
				&& pi.parent()->str.find("NP:") != std::string::npos && pi.nextSibling() != parsed.end()
				&& pi.nextSibling()->str.find("NP:") != std::string::npos)
			pi->str = "DT:c"; // for situations like: NP(DT:a(did),NP:h(not))
	}
}

static PredTree get_constituent_types(const PredTree &parsed, PhraseInfo *pinfo)
{
	PredTree to_return(parsed);
	PredTree::iterator pi = to_return.begin();

	mark_heads(to_return, pinfo);
	mark_complements_or_adjunct(to_return, pinfo);
	return to_return;
}

static PredTree get_binary(const PredTree &constituents)
{
	PredTree to_return(constituents);

	PredTree::iterator pi = to_return.begin();

	for (; pi != to_return.end(); ++pi) {
		if (pi.num_children() > 2) {
			PredTree tmp_tree(pi);
			string node_string = pi.node->firstChild->data.str;
			string right_node_string = pi.node->firstChild->data.str;
			int size = node_string.size();
			if (node_string.at(size - 1) == 'h' // if the first child is a head then goes left
					) {
				PredTree right_tree(*pi.node->lastChild);
				tmp_tree.erase(*tmp_tree.begin().node->lastChild);
				to_return.cut(pi);
				PredTree::iterator pi_left = to_return.appendChild(pi, PTEl(pi->str));
				to_return.appendTree(pi_left, tmp_tree);
				to_return.appendTree(pi, right_tree);
				pi = to_return.begin();
			} else { // if the first child is not a head then goes right
				PredTree left_tree(*pi.node->firstChild);
				tmp_tree.erase(*tmp_tree.begin().node->firstChild);
				to_return.cut(pi);
				to_return.appendTree(pi, left_tree);
				PredTree::iterator pi_right = to_return.appendChild(pi, PTEl(pi->str));
				to_return.appendTree(pi_right, tmp_tree);
				pi = to_return.begin();
			}
		}
	}
	return to_return;
}

static PredTree process_prn(const PredTree &constituents)
{
	PredTree to_return(constituents);
	PredTree::iterator pi = to_return.begin();
	pi = to_return.begin();
	for (; pi != to_return.end(); ++pi) { // The last comma of a PRN is signed as neutral
		if (pi.height() > 0 && pi.lastChild() != to_return.end() && pi->str.find("PRN") != string::npos
				&& (pi.lastChild()->str.find("-comma-") != string::npos || pi.lastChild()->str.find("\"") != string::npos)) {
			pi.lastChild()->str = "-comma[ignore]-:a";
		}
	}
	return to_return;
}

static bool is_question(PredTree::iterator pi, PredTree::iterator pend)
{
	for (; pi.parent() != pend; pi = pi.parent()) {
		if (pi->str.find("SQ") != string::npos)
			return true;
	}

	return false;
}

static PredTree get_corrected(const PredTree &constituents)
// All the previous passages can create small mistakes. Here we
// correct them
{
	PredTree to_return(constituents);

	PredTree::iterator pi = to_return.begin();

	pi = to_return.begin();
	for (; pi != to_return.end(); ++pi) { // Add dummy node for commas or CC
		if (pi.node->firstChild //&& pi.height() > 0
				&& (pi.node->firstChild->data.str.find("-comma-") != std::string::npos
						|| pi.node->firstChild->data.str.find("CC") != std::string::npos)) {
			PredTree tmp_child(pi);
			tmp_child.erase(*tmp_child.begin().node->firstChild);
			string str = pi->str;
			int pos;
			pos = str.find(":");
			if (pos != std::string::npos) {
				//str.replace(pos+3,1,"c");
				str.insert(pos, "[conj]");
			} else
				str += "[conj]";
			PredTree tmp_head(str);
			tmp_head.appendChild(tmp_head.begin(), string("-conj-:h"));
			tmp_head.appendChild(*tmp_head.begin().node->firstChild, string("-conj-"));
			string child_str = tmp_child.begin()->str;
			if (child_str.find(":a") != string::npos || child_str.find(":h") != string::npos) {
				child_str.at(child_str.size() - 1) = 'c'; // the last child in a conjunction is a complement
				tmp_child.begin()->str = child_str;
			}
			tmp_head.appendTree(tmp_head.begin(), tmp_child);
			to_return.replace(tmp_head, *pi.node);
			pi = to_return.begin();
		} else if (pi->str == "QP:a" && pi.node->lastChild && pi.node->lastChild->data.str == "QP:a") {
			pi.node->lastChild->data.str = "QP:c";
		}
	}
	int size;
	pi = to_return.begin();
	++pi;
	for (; pi != to_return.end(); ++pi) {
		size = pi->str.size();
		if (size == 0)
			continue;
		// eliminate single children in the middle of the tree
		if (PredTree::iterator(*pi.node->parent).num_children() == 1 && pi->str == pi.node->parent->data.str) {
			PredTree tmp(pi);
			to_return.replace(tmp, *pi.node->parent);
			pi = to_return.begin();
			++pi;
		} else if (pi->str == "ROOT:h") // Sometimes ROOT is binarized and becomes a subtree
			pi->str = "S:h";
		else if (pi->str.find("NP:") != string::npos && pi.firstChild() != to_return.end()
				&& pi.firstChild()->str.find("PRN:") != string::npos) {
			pi->str = pi.firstChild()->str;
		} else if (pi->str.find("SBAR:") != string::npos && pi.firstChild() != to_return.end()
				&& pi.firstChild()->str.find("VP:") != string::npos) {
			pi->str = pi.firstChild()->str;
		} else if (pi->str.find(":") == std::string::npos && pi->str.find("V") != std::string::npos)
			// some verbs might be missed, think of: VP(V(makes),CC(and),V:h(distributes))
			pi->str += ":h"; // It is an adjunct!!
		else if (pi->str == pi.parent()->str && pi->str.at(size - 1) == 'c' && pi.num_children()
				&& pi.parent().lastChild()->str.find(":a") != string::npos) {
			pi->str.at(size - 1) = 'h'; // for cases like (SBAR:c (SBAR:c -period-))
		} else if (pi->str == "NP:c"
				&& (pi.parent().lastChild()->str.find("SBAR:c") != string::npos
						|| pi.parent().lastChild()->str.find("S:c") != string::npos
						|| pi.parent().lastChild()->str.find("SBARQ:c") != string::npos
						|| pi.parent().lastChild()->str.find("SQ:c") != string::npos)) {
			int size2 = pi.parent().lastChild()->str.size();
			pi.parent().lastChild()->str.at(size2 - 1) = 'h'; // for cases like (SBAR:c (NP:c SBAR:c))
		} else if (pi->str.find("NP") != string::npos && pi.num_children() == 2
				&& (pi.firstChild()->str.find(":c") != string::npos && pi.lastChild()->str.find(":c") != string::npos)) {
			int size1 = pi.firstChild()->str.size();
			int size2 = pi.lastChild()->str.size();
			pi.firstChild()->str.at(size1 - 1) = 'a';
			pi.lastChild()->str.at(size2 - 1) = 'c'; // for cases like (NP (NN:c NN:c))
		} else if (pi->str.find("VP") != string::npos && pi.num_children() == 2
				&& (pi.firstChild()->str.find("VP:a") != string::npos && pi.lastChild()->str.find("NP:c") == 0)) {
			int size1 = pi.firstChild()->str.size();
			int size2 = pi.lastChild()->str.size();
			pi.firstChild()->str.at(size1 - 1) = 'h';
			pi.lastChild()->str.at(size2 - 1) = 'c';
		} else if (pi.num_children() == 2
				&& (pi.firstChild()->str.find("WDT:a") != string::npos && pi.lastChild()->str.find("VP:h") == 0)) {
			int size1 = pi.firstChild()->str.size();
			int size2 = pi.lastChild()->str.size();
			pi.firstChild()->str.at(size1 - 1) = 'c';
			pi.lastChild()->str.at(size2 - 1) = 'h';
		} else if (pi.num_children() == 2
				&& (pi.firstChild()->str.find("WRB:a") != string::npos && pi.lastChild()->str.find("NN:a") == 0)) {
			int size1 = pi.firstChild()->str.size();
			int size2 = pi.lastChild()->str.size();
			pi.firstChild()->str.at(size1 - 1) = 'h';
			pi.lastChild()->str.at(size2 - 1) = 'a'; // for cases like (NP (NN:c NN:c))
		} else if (pi->str.find("ADVP") != string::npos && pi.num_children() == 2
				&& (pi.firstChild()->str.find("NP:c") != string::npos && pi.lastChild()->str.find("VP:a") == 0)) {
			int size1 = pi.firstChild()->str.size();
			int size2 = pi.lastChild()->str.size();
			pi.firstChild()->str.at(size1 - 1) = 'c';
			pi.lastChild()->str.at(size2 - 1) = 'h'; // for cases like (NP (NN:c NN:c))
		} else if (pi.num_children() == 2
				&& (pi.firstChild()->str.find("NAC:a") != string::npos && pi.lastChild()->str.find("S:h") == string::npos)) {
			int size1 = pi.firstChild()->str.size();
			int size2 = pi.lastChild()->str.size();
			pi.firstChild()->str.at(size1 - 1) = 'c';
			pi.lastChild()->str.at(size2 - 1) = 'h'; // for cases like (NP (NN:c NN:c))
		} else if (pi.num_children() == 2
				&& (pi.firstChild()->str.find("NAC:a") != string::npos && pi.lastChild()->str.find("VP:h") == string::npos)) {
			int size1 = pi.firstChild()->str.size();
			int size2 = pi.lastChild()->str.size();
			pi.firstChild()->str.at(size1 - 1) = 'c';
			pi.lastChild()->str.at(size2 - 1) = 'h'; // for cases like (NP (NN:c NN:c))
		} else if ( pi.num_children() == 2
				&& (pi.firstChild()->str.find("VP:h") != string::npos && pi.lastChild()->str.find("PRN:h") != string::npos)) {
			int size1 = pi.firstChild()->str.size();
			int size2 = pi.lastChild()->str.size();
			pi.firstChild()->str.at(size1 - 1) = 'h';
			pi.lastChild()->str.at(size2 - 1) = 'c';
		}
		else if (pi->str.find("SQ") != string::npos && pi.num_children() == 2
				&& (pi.firstChild()->str.find(":c") != string::npos && pi.lastChild()->str.find(":c") != string::npos)) {
			int size1 = pi.firstChild()->str.size();
			int size2 = pi.lastChild()->str.size();
			pi.firstChild()->str.at(size1 - 1) = 'h';
			pi.lastChild()->str.at(size2 - 1) = 'c'; // for questions (SQ (SQ VP)
		} else if ( //pi->str.find("PRN") != string::npos
				  //&&
		pi.num_children() == 2
				&& (pi.firstChild()->str.find("PRN:c") != string::npos && pi.lastChild()->str.find("VP:h") != string::npos)) {
			int size1 = pi.firstChild()->str.size();
			int size2 = pi.lastChild()->str.size();
			pi.firstChild()->str.at(size1 - 1) = 'a';
			pi.lastChild()->str.at(size2 - 1) = 'h';
		} else if (pi.num_children() == 2
				&& (pi.firstChild()->str.find("RB") != string::npos && pi.lastChild()->str.find("S") != string::npos)) {
			int size1 = pi.firstChild()->str.size();
			int size2 = pi.lastChild()->str.size();
			pi.firstChild()->str.at(size1 - 1) = 'a';
			pi.lastChild()->str.at(size2 - 1) = 'h';
		} else if (pi->str.find("ADVP") != string::npos && pi.num_children() == 2
				&& (pi.firstChild()->str.find("RB") != string::npos && pi.lastChild()->str.find("ADVP") != string::npos)) {
			int size1 = pi.firstChild()->str.size();
			int size2 = pi.lastChild()->str.size();
			pi.firstChild()->str.at(size1 - 1) = 'h';
			pi.lastChild()->str.at(size2 - 1) = 'a';
		} else if (pi->str.find("NP") != string::npos && pi.num_children() == 2
				&& (pi.firstChild()->str.find("CD") != string::npos && pi.lastChild()->str.find("SBAR") != string::npos)) {
			int size1 = pi.firstChild()->str.size();
			int size2 = pi.lastChild()->str.size();
			pi.firstChild()->str.at(size1 - 1) = 'h';
			pi.lastChild()->str.at(size2 - 1) = 'a';
		} else if (pi->str.find("NP") != string::npos && pi.num_children() == 2
				&& (pi.firstChild()->str.find("NNP") != string::npos && pi.lastChild()->str.find("SBAR") != string::npos)) {
			int size1 = pi.firstChild()->str.size();
			int size2 = pi.lastChild()->str.size();
			pi.firstChild()->str.at(size1 - 1) = 'h';
			pi.lastChild()->str.at(size2 - 1) = 'a';
		} else if (pi.num_children() == 2
				&& (pi.firstChild()->str.find("WP") != string::npos && pi.lastChild()->str.find("SBAR") != string::npos)) {
			int size1 = pi.firstChild()->str.size();
			int size2 = pi.lastChild()->str.size();
			pi.firstChild()->str.at(size1 - 1) = 'c';
			pi.lastChild()->str.at(size2 - 1) = 'h';
		} else if (pi.num_children() == 2
				&& (pi.firstChild()->str.find("JJR") != string::npos && pi.lastChild()->str.find("NP") != string::npos)) {
			int size1 = pi.firstChild()->str.size();
			int size2 = pi.lastChild()->str.size();
			pi.firstChild()->str.at(size1 - 1) = 'a';
			pi.lastChild()->str.at(size2 - 1) = 'h';
		} else if (pi->str.find("PP") != string::npos && pi.num_children() == 2
				&& (pi.firstChild()->str.find("TO:h") != string::npos && pi.lastChild()->str.find("ADJP:a") != string::npos)) {
			int size1 = pi.firstChild()->str.size();
			int size2 = pi.lastChild()->str.size();
			pi.firstChild()->str.at(size1 - 1) = 'h';
			pi.lastChild()->str.at(size2 - 1) = 'c';
		} else if (pi->str.find("PP") != string::npos && pi.num_children() == 2
				&& (pi.firstChild()->str.find("IN:h") != string::npos && pi.lastChild()->str.find("ADJP:a") != string::npos)) {
			int size1 = pi.firstChild()->str.size();
			int size2 = pi.lastChild()->str.size();
			pi.firstChild()->str.at(size1 - 1) = 'h';
			pi.lastChild()->str.at(size2 - 1) = 'c';
		} else if (pi->str.find("NP") != string::npos && pi.num_children() == 2
				&& (pi.firstChild()->str.find("ADJP:a") != string::npos && pi.lastChild()->str.find("VP:h") != string::npos)) {
			int size1 = pi.firstChild()->str.size();
			int size2 = pi.lastChild()->str.size();
			pi.firstChild()->str.at(size1 - 1) = 'c';
			pi.lastChild()->str.at(size2 - 1) = 'h';
		} else if (pi->str.find("NP:a") != string::npos && pi.num_children() == 2
				&& (pi.firstChild()->str.find("PP:a") != string::npos && pi.lastChild()->str.find("SBAR:h") != string::npos)) {
			int size1 = pi.firstChild()->str.size();
			int size2 = pi.lastChild()->str.size();
			pi.firstChild()->str.at(size1 - 1) = 'h';
			pi.lastChild()->str.at(size2 - 1) = 'c';
		} else if (pi->str.find("X") != string::npos && pi.num_children() == 2
				&& (pi.firstChild()->str.find("NP:a") != string::npos && pi.lastChild()->str.find("X:a") != string::npos)) {
			int size1 = pi.firstChild()->str.size();
			int size2 = pi.lastChild()->str.size();
			pi.firstChild()->str.at(size1 - 1) = 'c';
			pi.lastChild()->str.at(size2 - 1) = 'h';
		} else if (pi->str.find("S") != string::npos && pi.num_children() == 2
				&& (pi.firstChild()->str.find("NAC:") != string::npos && pi.lastChild()->str.find("VP:h") != string::npos)) {
			int size1 = pi.firstChild()->str.size();
			int size2 = pi.lastChild()->str.size();
			pi.firstChild()->str.at(size1 - 1) = 'c';
			pi.lastChild()->str.at(size2 - 1) = 'h';
		} else if (pi->str.find("S") != string::npos && pi.num_children() == 2
				&& (pi.firstChild()->str.find("PP:a") != string::npos && pi.lastChild()->str.find("S:h") != string::npos)) {
			int size1 = pi.firstChild()->str.size();
			int size2 = pi.lastChild()->str.size();
			pi.firstChild()->str.at(size1 - 1) = 'c';
			pi.lastChild()->str.at(size2 - 1) = 'h';
		}
		else if (pi->str.find("PRN") != string::npos && pi.num_children() == 2
				&& (pi.firstChild()->str.find("WHNP:a") != string::npos && pi.lastChild()->str.find("PRN:c") != string::npos)) {
			int size1 = pi.firstChild()->str.size();
			int size2 = pi.lastChild()->str.size();
			pi.firstChild()->str.at(size1 - 1) = 'c';
			pi.lastChild()->str.at(size2 - 1) = 'h'; // for cases like (NP (NN:c NN:c))
		} else if (pi->str.find("NP") != string::npos && pi.num_children() == 2
				&& (pi.firstChild()->str.find("CD:") != string::npos && pi.lastChild()->str.find("NP:") == 0)) {
			int size1 = pi.firstChild()->str.size();
			int size2 = pi.lastChild()->str.size();
			pi.firstChild()->str.at(size1 - 1) = 'a';
			pi.lastChild()->str.at(size2 - 1) = 'h';
		} else if (pi->str.find("ADJP") != string::npos && pi.num_children() == 2
				&& (pi.firstChild()->str.find("JJR:h") != string::npos && pi.lastChild()->str.find("PP:c") != string::npos)) {
			int size1 = pi.firstChild()->str.size();
			int size2 = pi.lastChild()->str.size();
			pi.firstChild()->str.at(size1 - 1) = 'h';
			pi.lastChild()->str.at(size2 - 1) = 'a'; // for cases like (NP (NN:c NN:c))
		} else if (pi->str.find("ADJP") != string::npos && pi.num_children() == 2
				&& (pi.firstChild()->str.find("JJ:h") != string::npos && pi.lastChild()->str.find("PP:c") != string::npos)) {
			int size1 = pi.firstChild()->str.size();
			int size2 = pi.lastChild()->str.size();
			pi.firstChild()->str.at(size1 - 1) = 'h';
			pi.lastChild()->str.at(size2 - 1) = 'a'; // for cases like (NP (NN:c NN:c))
		} else if (pi->str.find("UCP") != string::npos && pi.num_children() == 2
				&& (pi.firstChild()->str.find(":c") != string::npos && pi.lastChild()->str.find("UCP:c") != string::npos)) {
			int size1 = pi.firstChild()->str.size();
			int size2 = pi.lastChild()->str.size();
			pi.firstChild()->str.at(size1 - 1) = 'c';
			pi.lastChild()->str.at(size2 - 1) = 'a'; // for cases like (NP (NN:c NN:c))
		} else if (pi->str.find("PP") != string::npos && pi.num_children() == 2
				&& (pi.firstChild()->str.find("IN") != string::npos && pi.lastChild()->str.find("UCP") != string::npos)) {
			int size1 = pi.firstChild()->str.size();
			int size2 = pi.lastChild()->str.size();
			pi.firstChild()->str.at(size1 - 1) = 'h';
			pi.lastChild()->str.at(size2 - 1) = 'c'; // for cases like (NP (NN:c NN:c))
		} else if (pi->str.find("ADJP") != string::npos && pi.num_children() == 2
				&& (pi.firstChild()->str.find("RB:a") != string::npos && pi.lastChild()->str.find("ADJP:c") != string::npos)) {
			int size1 = pi.firstChild()->str.size();
			int size2 = pi.lastChild()->str.size();
			pi.firstChild()->str.at(size1 - 1) = 'a';
			pi.lastChild()->str.at(size2 - 1) = 'h';
		} else if (pi->str.find("VP") != string::npos && pi.num_children() == 2
				&& (pi.firstChild()->str.find("ADVP:c") != string::npos && pi.lastChild()->str.find("VP:h") != string::npos)) {
			int size1 = pi.firstChild()->str.size();
			int size2 = pi.lastChild()->str.size();
			pi.firstChild()->str.at(size1 - 1) = 'a';
			pi.lastChild()->str.at(size2 - 1) = 'h';
		} else if (pi->str.find("PP") != string::npos && pi.num_children() == 2
				&& (pi.firstChild()->str.find("PP:a") != string::npos && pi.lastChild()->str.find("PP:a") != string::npos)) {
			int size1 = pi.firstChild()->str.size();
			int size2 = pi.lastChild()->str.size();
			pi.firstChild()->str.at(size1 - 1) = 'c';
			pi.lastChild()->str.at(size2 - 1) = 'a';
		} else if (pi->str.find("WHADVP") != string::npos && pi.num_children() == 2
				&& (pi.firstChild()->str.find("WRB:a") != string::npos && pi.lastChild()->str.find("WHADVP:a") != string::npos)) {
			int size1 = pi.firstChild()->str.size();
			int size2 = pi.lastChild()->str.size();
			pi.firstChild()->str.at(size1 - 1) = 'a';
			pi.lastChild()->str.at(size2 - 1) = 'c'; // for cases like (WHADVP (WRB:a WHADVP:a))
		} else if (pi->str.find("QP") != string::npos && pi.num_children() == 2
				&& (pi.firstChild()->str.find("JJR:a") != string::npos && pi.lastChild()->str.find("QP:h") == 0)) {
			int size1 = pi.firstChild()->str.size();
			int size2 = pi.lastChild()->str.size();
			pi.firstChild()->str.at(size1 - 1) = 'h';
			pi.lastChild()->str.at(size2 - 1) = 'a';
		} else if (pi->str.find("NP") != string::npos && pi.num_children() == 2 && (( //pi.firstChild()->str.find("CD:a") != string::npos
																	   // ||
		pi.firstChild()->str.find("PRP$:a") != string::npos) && pi.lastChild()->str.find("NP:c") == 0)) {
			int size1 = pi.firstChild()->str.size();
			int size2 = pi.lastChild()->str.size();
			pi.firstChild()->str.at(size1 - 1) = 'a';
			pi.lastChild()->str.at(size2 - 1) = 'h';
		} else if (pi->str.find("NP") != string::npos && pi.num_children() == 2
				&& (pi.firstChild()->str.find("PRP:c") != string::npos && pi.lastChild()->str.find("SBAR:h") == 0)) {
			int size1 = pi.firstChild()->str.size();
			int size2 = pi.lastChild()->str.size();
			pi.firstChild()->str.at(size1 - 1) = 'h';
			pi.lastChild()->str.at(size2 - 1) = 'a';
		} else if (pi->str.find("NP") != string::npos && pi.num_children() == 2
				&& (pi.firstChild()->str.find("NN:h") != string::npos && pi.lastChild()->str.find("NN:c") == 0)) {
			int size1 = pi.firstChild()->str.size();
			int size2 = pi.lastChild()->str.size();
			pi.firstChild()->str.at(size1 - 1) = 'a';
			pi.lastChild()->str.at(size2 - 1) = 'h';
		} else if (pi->str.find("NP") != string::npos && pi.num_children() == 2
				&& (pi.firstChild()->str.find("NN:c") != string::npos && pi.lastChild()->str.find("NNP:h") == 0)) {
			int size1 = pi.firstChild()->str.size();
			int size2 = pi.lastChild()->str.size();
			pi.firstChild()->str.at(size1 - 1) = 'a';
			pi.lastChild()->str.at(size2 - 1) = 'h';
		} else if (pi->str.find("NP") != string::npos && pi.num_children() == 2
				&& (pi.firstChild()->str.find("JJ:c") != string::npos && pi.lastChild()->str.find("NNP:h") == 0)) {
			int size1 = pi.firstChild()->str.size();
			int size2 = pi.lastChild()->str.size();
			pi.firstChild()->str.at(size1 - 1) = 'a';
			pi.lastChild()->str.at(size2 - 1) = 'h';
		} else if (pi->str.find("NP") != string::npos && pi.num_children() == 2
				&& (pi.firstChild()->str.find("NNP:c") != string::npos && pi.lastChild()->str.find("NNS:h") == 0)) {
			int size1 = pi.firstChild()->str.size();
			int size2 = pi.lastChild()->str.size();
			pi.firstChild()->str.at(size1 - 1) = 'a';
			pi.lastChild()->str.at(size2 - 1) = 'h';
		} else if (pi->str.find("NP") != string::npos && pi.num_children() == 2
				&& (pi.firstChild()->str.find("NNPS:c") != string::npos && pi.lastChild()->str.find("NNS:h") == 0)) {
			int size1 = pi.firstChild()->str.size();
			int size2 = pi.lastChild()->str.size();
			pi.firstChild()->str.at(size1 - 1) = 'a';
			pi.lastChild()->str.at(size2 - 1) = 'h';
		} else if (pi->str.find("NP") != string::npos && pi.num_children() == 2
				&& (pi.firstChild()->str.find("CD:c") != string::npos
						&& (pi.lastChild()->str.find("NN:h") == 0 || pi.lastChild()->str.find("NNS:h") == 0
								|| pi.lastChild()->str.find("NNP:h") == 0 || pi.lastChild()->str.find("NNPS:h") == 0))) {
			int size1 = pi.firstChild()->str.size();
			int size2 = pi.lastChild()->str.size();
			pi.firstChild()->str.at(size1 - 1) = 'a';
			pi.lastChild()->str.at(size2 - 1) = 'h';
		} else if (pi->str.find("NP") != string::npos && pi.num_children() == 2
				&& (pi.firstChild()->str.find("JJS:c") != string::npos && pi.lastChild()->str.find("NN:h") == 0)) {
			int size1 = pi.firstChild()->str.size();
			int size2 = pi.lastChild()->str.size();
			pi.firstChild()->str.at(size1 - 1) = 'a';
			pi.lastChild()->str.at(size2 - 1) = 'h';
		} else if (pi->str.find("NP") != string::npos && pi.num_children() == 2
				&& (pi.firstChild()->str.find("NNP:c") != string::npos && pi.lastChild()->str.find("NN:h") == 0)) {
			int size1 = pi.firstChild()->str.size();
			int size2 = pi.lastChild()->str.size();
			pi.firstChild()->str.at(size1 - 1) = 'a';
			pi.lastChild()->str.at(size2 - 1) = 'h';
		} else if (pi->str.find("NP") != string::npos && pi.num_children() == 2
				&& (pi.firstChild()->str.find("NNPS:c") != string::npos && pi.lastChild()->str.find("NN:h") == 0)) {
			int size1 = pi.firstChild()->str.size();
			int size2 = pi.lastChild()->str.size();
			pi.firstChild()->str.at(size1 - 1) = 'a';
			pi.lastChild()->str.at(size2 - 1) = 'h';
		} else if (pi->str.find("ADJP") != string::npos && pi.num_children() == 2 && (( //pi.firstChild()->str.find("CD:a") != string::npos
																		// ||
		pi.firstChild()->str.find("ADJP:c") != string::npos) && pi.lastChild()->str.find("PP:c") != string::npos)) {
			int size1 = pi.firstChild()->str.size();
			int size2 = pi.lastChild()->str.size();
			pi.firstChild()->str.at(size1 - 1) = 'c';
			pi.lastChild()->str.at(size2 - 1) = 'a'; // for cases like (NP (NN:c NN:c))
		} else if (pi->str.find("NP") != string::npos && pi.num_children() == 2
				&& pi.firstChild()->str.find("NN") != string::npos && pi.firstChild()->str.find(":c") != string::npos
				&& pi.lastChild()->str.find("PRN:h") != string::npos) {
			int size1 = pi.firstChild()->str.size();
			int size2 = pi.lastChild()->str.size();
			pi.firstChild()->str.at(size1 - 1) = 'c';
			pi.lastChild()->str.at(size2 - 1) = 'a'; // for cases like (NP (NNP:c PRN:h))
		} else if (pi.num_children() == 2 && pi.firstChild()->str.find(":c") != string::npos
				&& pi.lastChild()->str.find("[conj]:h") != string::npos) {
			int size1 = pi.firstChild()->str.size();
			int size2 = pi.lastChild()->str.size();
			pi.firstChild()->str.at(size1 - 1) = 'h';
			pi.lastChild()->str.at(size2 - 1) = 'c';
		} else if (pi.num_children() == 2 && pi.firstChild()->str.find("NN:h") != string::npos
				&& pi.lastChild()->str.find("SBAR:c") != string::npos) {
			int size1 = pi.firstChild()->str.size();
			int size2 = pi.lastChild()->str.size();
			pi.firstChild()->str.at(size1 - 1) = 'h';
			pi.lastChild()->str.at(size2 - 1) = 'a'; // for cases like (NP (NN:h SBAR:c))
		} else if (pi.num_children() == 2 && pi.firstChild()->str.find("NNS:h") != string::npos
				&& pi.lastChild()->str.find("SBAR:c") != string::npos) {
			int size1 = pi.firstChild()->str.size();
			int size2 = pi.lastChild()->str.size();
			pi.firstChild()->str.at(size1 - 1) = 'h';
			pi.lastChild()->str.at(size2 - 1) = 'a'; // for cases like (NP (NN:h SBAR:c))
		} else if (pi.num_children() == 2 && pi.firstChild()->str.find("VP:c") != string::npos
				&& pi.lastChild()->str.find("NP:c") == 0) {
			int size1 = pi.firstChild()->str.size();
			int size2 = pi.lastChild()->str.size();
			pi.firstChild()->str.at(size1 - 1) = 'h';
			pi.lastChild()->str.at(size2 - 1) = 'c'; // for cases like (NP (NN:h SBAR:c))
		} else if (pi->str.find("S") != string::npos && pi.num_children() == 2
				&& (pi.firstChild()->str.find(":a") != string::npos && pi.lastChild()->str.find("S:a") != string::npos)) {
			int size1 = pi.firstChild()->str.size();
			int size2 = pi.lastChild()->str.size();
			pi.firstChild()->str.at(size1 - 1) = 'a';
			pi.lastChild()->str.at(size2 - 1) = 'c'; // for cases like (S:a (PP:c S:a))
		} else if (pi->str.find("S") != string::npos && pi.num_children() == 2
				&& (pi.firstChild()->str.find("IN") != string::npos && pi.lastChild()->str.find("S") != string::npos)) {
			int size1 = pi.firstChild()->str.size();
			int size2 = pi.lastChild()->str.size();
			pi.firstChild()->str.at(size1 - 1) = 'h';
			pi.lastChild()->str.at(size2 - 1) = 'c'; // for cases like (S (IN S))
		} else if (pi.num_children() == 2
				&& (pi.firstChild()->str.find("WHNP:c") != string::npos && pi.lastChild()->str.find("S:h") == 0)) {
			int size1 = pi.firstChild()->str.size();
			int size2 = pi.lastChild()->str.size();
			pi.firstChild()->str.at(size1 - 1) = 'a';
			pi.lastChild()->str.at(size2 - 1) = 'h';
		} else if (pi.num_children() == 2 && pi.firstChild()->str.find("VP:c") != string::npos
				&& pi.lastChild()->str.find("SBAR[conj]:c") != string::npos) {
			int size1 = pi.firstChild()->str.size();
			int size2 = pi.lastChild()->str.size();
			pi.firstChild()->str.at(size1 - 1) = 'h';
			pi.lastChild()->str.at(size2 - 1) = 'c';
		}
		else if (pi->str.find("NP:c") == 0 && pi.num_children() == 2
				&& ((pi.firstChild()->str.find("JJ:c") != string::npos || pi.firstChild()->str.find("VBJ:c") != string::npos)
						&& pi.lastChild()->str.find("NP:h") == 0)) {
			int size1 = pi.firstChild()->str.size();
			int size2 = pi.lastChild()->str.size();
			pi.firstChild()->str.at(size1 - 1) = 'c';
			pi.lastChild()->str.at(size2 - 1) = 'a';
		} else if (pi->str.find("NP:h") != string::npos && pi.num_children() == 2
				&& ((pi.firstChild()->str.find("JJ:c") != string::npos || pi.firstChild()->str.find("VBJ:c") != string::npos)
						&& pi.lastChild()->str.find("NP:h") == 0)) {
			int size1 = pi.firstChild()->str.size();
			int size2 = pi.lastChild()->str.size();
			pi.firstChild()->str.at(size1 - 1) = 'a';
			pi.lastChild()->str.at(size2 - 1) = 'h';
		} else if (pi->str.find("SBAR:a") != string::npos && pi.num_children() == 2
				&& (pi.firstChild()->str.find("WP:a") != string::npos && pi.lastChild()->str.find("SBAR:a") != string::npos)) {
			int size1 = pi.firstChild()->str.size();
			int size2 = pi.lastChild()->str.size();
			pi.firstChild()->str.at(size1 - 1) = 'c';
			pi.lastChild()->str.at(size2 - 1) = 'h';
		} else if (pi.num_children() == 2 && (pi.firstChild()->str.find("NP") == 0 && pi.lastChild()->str.find("SBAR") != string::npos)) {
			int size1 = pi.firstChild()->str.size();
			int size2 = pi.lastChild()->str.size();
			pi.firstChild()->str.at(size1 - 1) = 'c';
			pi.lastChild()->str.at(size2 - 1) = 'h';
		} else if (pi->str.find("PRN") != string::npos && pi.num_children() == 2
				&& (pi.firstChild()->str.find("NP:h") == 0 && pi.lastChild()->str.find("VP:a") != string::npos)) {
			int size1 = pi.firstChild()->str.size();
			int size2 = pi.lastChild()->str.size();
			pi.firstChild()->str.at(size1 - 1) = 'c';
			pi.lastChild()->str.at(size2 - 1) = 'h'; // for cases like (PRN (NP:h VP:a))
		} else if (pi->str.find("NP:h") == 0 && pi.num_children() == 2
				&& (pi.firstChild()->str.find("JJS:c") != string::npos && pi.lastChild()->str.find("NP:h") == 0)) {
			int size1 = pi.firstChild()->str.size();
			int size2 = pi.lastChild()->str.size();
			pi.firstChild()->str.at(size1 - 1) = 'a';
			pi.lastChild()->str.at(size2 - 1) = 'h'; // for cases like (NP:h (JJS:h NP:h))
		} else if (pi->str.find("NP:") != string::npos && pi.num_children() == 2
				&& (pi.firstChild()->str.find("NP:") == 0 && pi.lastChild()->str.find("PRN:") != string::npos)) {
			int size1 = pi.firstChild()->str.size();
			int size2 = pi.lastChild()->str.size();
			pi.firstChild()->str.at(size1 - 1) = 'c';
			pi.lastChild()->str.at(size2 - 1) = 'a'; // for cases like (NP (NP:c PRP:h))
			// When there is a situation like (NP PRN) NP must be "head" and PRN must be "c"
			// a worker (PRN participating in the society) is fine
		} else if (pi->str.find("PRN") != string::npos && pi.num_children() == 2
				&& (pi.firstChild()->str.find("PRN:h") != string::npos && pi.lastChild()->str.find("VP:h") != string::npos)) {
			int size1 = pi.firstChild()->str.size();
			int size2 = pi.lastChild()->str.size();
			pi.firstChild()->str.at(size1 - 1) = 'h';
			pi.lastChild()->str.at(size2 - 1) = 'c'; // for cases like (NP (NP:c PRP:h))
			// When there is a situation like (NP PRN) NP must be "head" and PRN must be "adjunct"
		} else if (pi->str.find("NP") != string::npos && pi.num_children() == 2
				&& (pi.firstChild()->str.find("NN") != string::npos && pi.firstChild()->str.find(":a") != string::npos
						&& pi.lastChild()->str.find("NP:c") == 0)) {
			int size1 = pi.firstChild()->str.size();
			int size2 = pi.lastChild()->str.size();
			pi.firstChild()->str.at(size1 - 1) = 'h';
			pi.lastChild()->str.at(size2 - 1) = 'a'; // for cases like (NP:c (NN:a NP:c)). Fundamental for conjunctions!!
		} else if (pi->str.find("NP:h") == 0 && pi.num_children() == 2
				&& (pi.firstChild()->str.find("NP:h") == 0 && pi.lastChild()->str.find("NP:c") == 0)) {
			int size1 = pi.firstChild()->str.size();
			int size2 = pi.lastChild()->str.size();
			pi.firstChild()->str.at(size1 - 1) = 'h';
			pi.lastChild()->str.at(size2 - 1) = 'a';
		} else if (pi->str.find("NP") != string::npos && pi.num_children() == 2
				&& (pi.firstChild()->str.find("NN") != string::npos && pi.firstChild()->str.find(":c") != string::npos
						&& pi.lastChild()->str.find("NP:h") == 0)) {
			int size1 = pi.firstChild()->str.size();
			int size2 = pi.lastChild()->str.size();
			pi.firstChild()->str.at(size1 - 1) = 'h';
			pi.lastChild()->str.at(size2 - 1) = 'a'; // for cases like (NP:h (NN:c NP:h)).
		} else if (pi->str.find("NP") != string::npos && pi.num_children() == 2
				&& (pi.firstChild()->str.find("NP") == 0 && pi.lastChild()->str.find("NN") != string::npos)) {
			int size1 = pi.firstChild()->str.size();
			int size2 = pi.lastChild()->str.size();
			pi.firstChild()->str.at(size1 - 1) = 'a';
			pi.lastChild()->str.at(size2 - 1) = 'h';
		} else if (pi->str.find("NP") != string::npos && pi.num_children() == 2
				&& ((pi.firstChild()->str.find("JJ") != string::npos || pi.firstChild()->str.find("VBJ") != string::npos)
						&& pi.firstChild()->str.find(":c") != string::npos && pi.lastChild()->str.find("NN") != string::npos)
				&& pi.lastChild()->str.find(":h") != string::npos) {
			int size1 = pi.firstChild()->str.size();
			int size2 = pi.lastChild()->str.size();
			pi.firstChild()->str.at(size1 - 1) = 'a';
			pi.lastChild()->str.at(size2 - 1) = 'h'; // for cases like (NP:c (JJ:a NN:c)). /// It must be changed, this is the result of making JJ complement
		} else if (pi->str.find("NP:") != string::npos && pi.num_children()
				&& (pi.firstChild()->str == "NP:c" && pi.lastChild()->str == "NP:c")) {
			if (pi.lastChild().firstChild()->str == "VP:h") {
				int size1 = pi.firstChild()->str.size();
				int size2 = pi.lastChild()->str.size();
				pi.firstChild()->str.at(size1 - 1) = 'c';
				pi.lastChild()->str.at(size2 - 1) = 'h'; // for cases like (NP:c (NP:c (NP:c (VP:h ) ) ))
			}
		} else if (is_question(pi, to_return.end()) && pi->str.find("VP:") != string::npos && pi.num_children()
				&& (pi.firstChild()->str == "VP:c" && pi.lastChild()->str == "VP:h")) {
			int size1 = pi.firstChild()->str.size();
			int size2 = pi.lastChild()->str.size();
			pi.firstChild()->str.at(size1 - 1) = 'h';
			pi.lastChild()->str.at(size2 - 1) = 'c';
			// For cases like (VP:h (VP:c (VP:h ) ). the order of c and h must be
			// reversed for questions
		} else if (!is_question(pi, to_return.end()) && pi.num_children() == 2
				&& (pi.firstChild()->str == "NP:h" && pi.lastChild()->str == "VP:c")) {
			int size1 = pi.firstChild()->str.size();
			int size2 = pi.lastChild()->str.size();
			pi.firstChild()->str.at(size1 - 1) = 'c';
			pi.lastChild()->str.at(size2 - 1) = 'h';
			// For cases like (VP:h (VP:c (VP:h ) ). the order of c and h must be
			// this for declarative phrases
		} else if (is_question(pi, to_return.end()) && pi->str.find("NP:") != string::npos && pi.num_children()
				&& (pi.firstChild()->str == "NP:c" && pi.lastChild()->str == "VP:h")) {
			int size1 = pi.firstChild()->str.size();
			int size2 = pi.lastChild()->str.size();
			pi.firstChild()->str.at(size1 - 1) = 'h';
			pi.lastChild()->str.at(size2 - 1) = 'c';
			// For cases like (NP:h (NP:c (VP:h ) ). the order of c and h must be
			// reversed for questions
		} else if (pi->str.find("NP:") != string::npos && pi.num_children()
				&& (pi.firstChild()->str == "NP:c" && pi.lastChild()->str == "NP:h")) {
			int size1 = pi.firstChild()->str.size();
			int size2 = pi.lastChild()->str.size();
			pi.firstChild()->str.at(size1 - 1) = 'h';
			pi.lastChild()->str.at(size2 - 1) = 'c';
			// For cases like (NP:h (NP:c (NP:h ) ).
		} else if (pi->str.find("VP:") != string::npos && pi.num_children()
				&& (pi.firstChild()->str == "VP:c" && pi.lastChild()->str == "VP:c")) {
			int size1 = pi.firstChild()->str.size();
			int size2 = pi.lastChild()->str.size();
			pi.firstChild()->str.at(size1 - 1) = 'h';
			pi.lastChild()->str.at(size2 - 1) = 'c';
			// For cases like (VP:h (VP:c (VP:c ) ).
		} else if (pi->str == "VP:" && pi.num_children() && pi.firstChild()->str == "VP:c" && pi.lastChild()->str == "VP:h") {
			int size1 = pi.firstChild()->str.size();
			int size2 = pi.lastChild()->str.size();
			pi.firstChild()->str.at(size1 - 1) = 'h';
			pi.lastChild()->str.at(size2 - 1) = 'c';
			// For cases like (VP:h (VP:c (VP:h ) ). the order of c and h must be
			// reversed
		}

		else if (pi->str.find("TO") != string::npos && pi.nextSibling() != to_return.end() && pi.nextSibling()->str == "WHNP"
				&& (pi.firstChild()->str == "NP:c" && pi.lastChild()->str == "NP:c")) {
			if (pi.lastChild().firstChild()->str == "VP:h") {
				int size1 = pi.firstChild()->str.size();
				int size2 = pi.lastChild()->str.size();
				pi.firstChild()->str.at(size1 - 1) = 'c';
				pi.lastChild()->str.at(size2 - 1) = 'h'; // for cases like (TO(to) WHNP(WDT(whom)))
			}
		} else if (pi->str.find("S:h") != string::npos && pi.num_children() == 2
				&& (pi.firstChild()->str == "S:c" && pi.lastChild()->str == "S:h")) {
			int size1 = pi.firstChild()->str.size();
			int size2 = pi.lastChild()->str.size();
			pi.firstChild()->str.at(size1 - 1) = 'h';
			pi.lastChild()->str.at(size2 - 1) = 'c';
		} else if (pi->str.find("FRAC:") != string::npos && pi.num_children() == 2
				&& (pi.firstChild()->str == "S:a" && pi.lastChild()->str == "VP:a")) {
			int size1 = pi.firstChild()->str.size();
			int size2 = pi.lastChild()->str.size();
			pi.firstChild()->str.at(size1 - 1) = 'c';
			pi.lastChild()->str.at(size2 - 1) = 'h';
		}
		else if (pi->str.find("NP") != string::npos && pi.num_children() == 2
				&& (pi.firstChild()->str == "NP:h" && pi.lastChild()->str == "PRN:a")) {
			int size1 = pi.firstChild()->str.size();
			int size2 = pi.lastChild()->str.size();
			pi.firstChild()->str.at(size1 - 1) = 'c';
			pi.lastChild()->str.at(size2 - 1) = 'h';
		}
		// ]
	}

	// It must go before (A)
	pi = to_return.begin();
	++pi;
	for (; pi != to_return.end(); ++pi) {
		int size = pi->str.size();
		if (pi->str.find("NP:") != string::npos && pi.num_children()
				&& (pi.firstChild()->str.find("VB") != string::npos && pi.firstChild()->str.find("VBJ") == string::npos /// VBJ definitely does not seem a good name
				)) {
			pi->str = "VP:h"; // for cases like (NP:h (VBN ...))
		}
	}
	// (A)
	pi = to_return.begin();
	++pi;
	for (; pi != to_return.end(); ++pi) {
		int size = pi->str.size();
		if (pi->str.find("NP:") != string::npos && pi.num_children() && pi.firstChild()->str.find("VP:h") != string::npos) {
			pi->str = "VP:h"; // for cases like (NP:c (VP:h ...))
			if (pi.parent().firstChild()->str.find(":h") != string::npos) {
				int size1 = pi.parent().firstChild()->str.size();
				pi.parent().firstChild()->str.at(size1 - 1) = 'c';
			}
		}
	}

	pi = to_return.begin();
	++pi;
	for (; pi != to_return.end(); ++pi) {
		int size = pi->str.size();
		if (pi.num_children() && (pi.firstChild()->str.find("VB") != string::npos
		//&& pi.firstChild()->str.find(":h") != string::npos
				&& pi.lastChild()->str == "VP:h")) {
			int size1 = pi.firstChild()->str.size();
			int size2 = pi.lastChild()->str.size();
			pi.firstChild()->str.at(size1 - 1) = 'h';
			pi.lastChild()->str.at(size2 - 1) = 'c';
			// this is for auxiliaries, as in (VP:h (VBZ:h VP:h))
		} else if (pi.num_children() == 2 && pi.firstChild()->str.find("IN:h") != string::npos
				&& pi.lastChild()->str.find("VP:h") != string::npos) {
			int size1 = pi.firstChild()->str.size();
			int size2 = pi.lastChild()->str.size();
			pi.firstChild()->str.at(size1 - 1) = 'h';
			pi.lastChild()->str.at(size2 - 1) = 'c'; // for cases like (NP (NN:h SBAR:c))
		} else if (pi->str.find("NP") != string::npos ////
		&& pi.num_children() == 2
				&& (pi.firstChild()->str.find("NNP:h") != string::npos && pi.lastChild()->str.find("NP:a") == 0)) {
			int size1 = pi.firstChild()->str.size();
			int size2 = pi.lastChild()->str.size();
			pi.firstChild()->str.at(size1 - 1) = 'c';
			pi.lastChild()->str.at(size2 - 1) = 'a';
		} else if (pi->str.find("VP:h") != string::npos && pi.num_children() == 2
				&& (pi.firstChild()->str.find("VP:h") != string::npos && pi.lastChild()->str.find("VP:h") != string::npos)) {
			int size1 = pi.firstChild()->str.size();
			int size2 = pi.lastChild()->str.size();
			pi.firstChild()->str.at(size1 - 1) = 'h';
			pi.lastChild()->str.at(size2 - 1) = 'c'; // for cases like (VP (VP:h VP:h))
		} else if (pi.num_children() == 2
				&& (pi.firstChild()->str.find("VP:c") != string::npos && pi.lastChild()->str.find("VP:h") != string::npos)) {
			int size1 = pi.firstChild()->str.size();
			int size2 = pi.lastChild()->str.size();
			pi.firstChild()->str.at(size1 - 1) = 'h';
			pi.lastChild()->str.at(size2 - 1) = 'c'; // for cases like (VP (VP:h VP:h))
		} else if (pi.num_children() == 2
				&& (pi.firstChild()->str.find(":h") != string::npos && pi.lastChild()->str.find(":h") != string::npos)) {
			int size1 = pi.firstChild()->str.size();
			int size2 = pi.lastChild()->str.size();
			pi.firstChild()->str.at(size1 - 1) = 'h';
			pi.lastChild()->str.at(size2 - 1) = 'c'; // for cases like (VP (VP:h VP:h))
		} else if (pi->str.find("PP") != string::npos && pi.num_children() == 2
				&& (pi.firstChild()->str.find("IN:h") != string::npos && pi.lastChild()->str.find("JJ:a") != string::npos)) {
			int size1 = pi.firstChild()->str.size();
			int size2 = pi.lastChild()->str.size();
			pi.firstChild()->str.at(size1 - 1) = 'h';
			pi.lastChild()->str.at(size2 - 1) = 'c'; // described as painted/JJ
		} else if (pi->str.find("NP") != string::npos && pi.num_children() == 2
				&& (pi.firstChild()->str.find("NN") != string::npos && pi.lastChild()->str.find("NP:") != string::npos)) {
			int size1 = pi.firstChild()->str.size();
			int size2 = pi.lastChild()->str.size();
			pi.firstChild()->str.at(size1 - 1) = 'c';
			pi.lastChild()->str.at(size2 - 1) = 'a'; // described as painted/JJ
		} else if (pi->str.find("NP") != string::npos && pi.num_children() == 2
				&& (pi.firstChild()->str.find("JJ:a") != string::npos && pi.lastChild()->str.find("NP:c") != string::npos)) {
			int size1 = pi.firstChild()->str.size();
			int size2 = pi.lastChild()->str.size();
			pi.firstChild()->str.at(size1 - 1) = 'h';
			pi.lastChild()->str.at(size2 - 1) = 'a';
		} else if (pi->str.find("SBARQ:") != string::npos && pi.num_children() == 2
				&& (pi.firstChild()->str.find("NP:h") != string::npos && pi.lastChild()->str.find("SQ:c") != string::npos)) {
			int size1 = pi.firstChild()->str.size();
			int size2 = pi.lastChild()->str.size();
			pi.firstChild()->str.at(size1 - 1) = 'c';
			pi.lastChild()->str.at(size2 - 1) = 'h'; // for cases like (NP (NN:c NN:c))
		} else if (pi->str.find("SQ:") != string::npos && pi.num_children() == 2
				&& (pi.firstChild()->str.find("SQ:h") != string::npos && pi.lastChild()->str.find("ADVP:a") != string::npos)) {
			int size1 = pi.firstChild()->str.size();
			int size2 = pi.lastChild()->str.size();
			pi.firstChild()->str.at(size1 - 1) = 'c';
			pi.lastChild()->str.at(size2 - 1) = 'a'; // for cases like (NP (NN:c NN:c))
		} else if (pi->str.find("VP:") != string::npos && pi.num_children() == 2
				&& (pi.firstChild()->str.find("VP:c") != string::npos && pi.lastChild()->str.find("ADJP:c") != string::npos)) {
			int size1 = pi.firstChild()->str.size();
			int size2 = pi.lastChild()->str.size();
			pi.firstChild()->str.at(size1 - 1) = 'h';
			pi.lastChild()->str.at(size2 - 1) = 'c'; // for cases like (NP (NN:c NN:c))
		} else if (pi.num_children() == 2
				&& (pi.firstChild()->str.find("WRB:a") != string::npos && pi.lastChild()->str.find("NNS:h") == 0)) {
			int size1 = pi.firstChild()->str.size();
			int size2 = pi.lastChild()->str.size();
			pi.firstChild()->str.at(size1 - 1) = 'h';
			pi.lastChild()->str.at(size2 - 1) = 'a';
		} else if (pi->str.find("ADVP") != string::npos && pi.num_children() == 2
				&& pi.firstChild()->str.find("ADVP") != string::npos && pi.lastChild()->str.find("VP") == 0) {
			int size1 = pi.firstChild()->str.size();
			int size2 = pi.lastChild()->str.size();
			pi.firstChild()->str.at(size1 - 1) = 'c';
			pi.lastChild()->str.at(size2 - 1) = 'h';
		} else if (pi->str.find("ADVP") != string::npos && pi.num_children() == 2
				&& pi.firstChild()->str.find("ADVP") != string::npos && pi.lastChild()->str.find("NP") == 0) {
			int size1 = pi.firstChild()->str.size();
			int size2 = pi.lastChild()->str.size();
			pi.firstChild()->str.at(size1 - 1) = 'a';
			pi.lastChild()->str.at(size2 - 1) = 'h';
		} else if (pi->str.find("NP") != string::npos && pi.num_children() == 2
				&& pi.firstChild()->str.find("ADJP") != string::npos && pi.lastChild()->str.find("PP") == 0) {
			int size1 = pi.firstChild()->str.size();
			int size2 = pi.lastChild()->str.size();
			pi.firstChild()->str.at(size1 - 1) = 'c';
			pi.lastChild()->str.at(size2 - 1) = 'a';
		} else if (pi->str.find("ADJP") != string::npos && pi.num_children() == 2
				&& pi.firstChild()->str.find("RB") != string::npos && pi.lastChild()->str.find("ADJP") == 0) {
			int size1 = pi.firstChild()->str.size();
			int size2 = pi.lastChild()->str.size();
			pi.firstChild()->str.at(size1 - 1) = 'a';
			pi.lastChild()->str.at(size2 - 1) = 'h';
		} else if (pi->str.find("NP") != string::npos && pi.num_children() == 2
				&& (pi.firstChild()->str.find("NP:h") != string::npos && pi.lastChild()->str.find("NP:c") == 0)
				&& pi.lastChild().firstChild() != to_return.end()
				&& pi.lastChild().firstChild()->str.find("VP:") != string::npos) {
			int size1 = pi.firstChild()->str.size();
			int size2 = pi.lastChild()->str.size();
			pi.firstChild()->str = "NP:c";
			pi.lastChild()->str = "VP:h";
		} else if (pi.num_children() == 2 && pi.firstChild()->str.find("WHPP:c") != string::npos
				&& pi.lastChild()->str.find("S:h") != string::npos) {
			int size1 = pi.firstChild()->str.size();
			int size2 = pi.lastChild()->str.size();
			pi.firstChild()->str.at(size1 - 1) = 'a';
			pi.lastChild()->str.at(size2 - 1) = 'h';
		} else if (pi->str.find("NP") != string::npos && pi.num_children() == 2
				&& (pi.firstChild()->str == "NP:a" && pi.lastChild()->str == "NP:c")) {
			int size1 = pi.firstChild()->str.size();
			int size2 = pi.lastChild()->str.size();
			pi.firstChild()->str.at(size1 - 1) = 'c';
			pi.lastChild()->str.at(size2 - 1) = 'a';
		} else if (pi.num_children() == 2 && pi.firstChild()->str == "VP:c" && pi.lastChild()->str == "SBAR:c") {
			int size1 = pi.firstChild()->str.size();
			int size2 = pi.lastChild()->str.size();
			pi.firstChild()->str.at(size1 - 1) = 'h';
			pi.lastChild()->str.at(size2 - 1) = 'c';
		} else if (pi->str.find("WHNP") != string::npos && pi.num_children() == 2
				&& (pi.firstChild()->str == "IN:h" && pi.lastChild()->str == "WDT:a")) {
			int size1 = pi.firstChild()->str.size();
			int size2 = pi.lastChild()->str.size();
			pi.firstChild()->str.at(size1 - 1) = 'h';
			pi.lastChild()->str.at(size2 - 1) = 'c';
		} else if (pi->str.find("NP") != string::npos && pi.num_children() == 2
				&& (pi.firstChild()->str.find("JJ") != string::npos
						&& pi.lastChild()->str.find("NP") != string::npos )) {
			int size1 = pi.firstChild()->str.size();
			int size2 = pi.lastChild()->str.size();
			pi.firstChild()->str.at(size1 - 1) = 'a';
			pi.lastChild()->str.at(size2 - 1) = 'h';
		} else if (pi->str.find("NP") != string::npos && pi.num_children() == 2
				&& (pi.firstChild()->str.find("RB") != string::npos
				    && pi.lastChild()->str.find("CD") != string::npos )) {
			int size1 = pi.firstChild()->str.size();
			int size2 = pi.lastChild()->str.size();
			pi.firstChild()->str.at(size1 - 1) = 'a';
			pi.lastChild()->str.at(size2 - 1) = 'h';
		} else if (pi->str.find("NP") != string::npos && pi.num_children() == 2
				&& (pi.firstChild()->str.find("NP:c") != string::npos && pi.lastChild()->str.find("VP:h") == 0)) {
			int size3 = pi->str.size();
			if (size3 > 1)
				pi->str = (string) "S:" + pi->str.at(size3 - 1);
		} else if (pi->str.find("WHNP") != string::npos && pi.num_children() == 2
				&& (pi.firstChild()->str.find("WRB:a") != string::npos
						&& (pi.lastChild()->str.find("NNS:h") != string::npos
								|| pi.lastChild()->str.find("NN:h") != string::npos
								|| pi.lastChild()->str.find("NNP:h") != string::npos
								|| pi.lastChild()->str.find("NNPS:h") != string::npos))) {
			int size1 = pi.firstChild()->str.size();
			int size2 = pi.lastChild()->str.size();
			pi.firstChild()->str.at(size1 - 1) = 'h';
			pi.lastChild()->str.at(size2 - 1) = 'a'; // for cases like (WHADVP (WRB:a WHADVP:a))
		}
	}

	return to_return;
}

static int get_num_NP_parent(constituents con)
{
	FTree<pair<string, int> > contree(con.tree());
	FTree<pair<string, int> >::iterator ci(contree);
	if (ci.height() == 1)
		return ci.firstChild()->second;
	while (ci != contree.end() && ci.height() > 0 // && ci.node->firstChild->firstChild
	)
		ci = ci.firstChild();
	if (ci.node->nextSibling)
		return ci.nextSibling()->second;
	else
		return ci->second;
}

static constituents get_complement_name(PredTree binary_subtree, int num, int num_NP_parent = 0, string parent = "",
		constituents constit_head = constituents())
{
	PredTree::iterator pi(binary_subtree);

	string trigger, atom;
	constituents to_return;
	bool trigger_NP = false;
	bool w_trigger = false;

	if (constit_head.tree().begin()->first == "S[wq]")
		w_trigger = true;
	if (constit_head.tree().begin().num_children() > 0 && constit_head.tree().begin().firstChild()->first == "S[wq]")
		w_trigger = true;

	for (; pi != binary_subtree.end(); ++pi) {
		atom = pi->str;

		if ((pi.num_children() == 1 && pi.firstChild()->str.find("EX") != std::string::npos)
				|| atom.find("EX") != std::string::npos) {
			return constituents(string("NP[ex]"), num);
		}
		if (atom.find("SBAR") != std::string::npos && parent.find("VP") != string::npos
				&& pi.firstChild()->str.find("TO") != string::npos) {
			return constituents(string("VP[to]"), num);
		}
		if (atom.find("SINV") != std::string::npos) {
			return constituents(string("S[inv]"), num);
		}
		if (atom.find("PRN") != std::string::npos) {
			return constituents(string("S[dcl]"), num);
		}
		if (atom.find("WRB") != std::string::npos) {
			w_trigger = true;
		}
		if (atom.find(":c") != std::string::npos) {
			if (atom == "S:c" || atom == "SBAR:c") {
				return constituents(string("S[dcl]"), num);
			}
			if (atom == "SQ:c") {
				if (parent.find("SBAR") != string::npos) {
					if (w_trigger)
						return constituents(string("S[wq]"), num);
					else
						return constituents(string("S[dcl]"), num);
				}
				return constituents(string("S[q]"), num);
			}
			if (atom == "NP:c") {
				return constituents(string("NP"), num);
			}
			if (atom == "VP:c" || atom == "VB:c" || atom == "VBP:c" || atom == "VBZ:c" || atom == "VBD:c" || atom == "VBD:c"
					|| atom == "MD:c") {
				return constituents(string("VP"), num);

			}
			if (atom.find("PP") != std::string::npos) {
				return constituents(string("PP"), num);
			}
		}
		if (atom == "to")
			trigger = "TO";
		if (atom.find("VBN") != std::string::npos || atom.find("VBG") != std::string::npos) {
			to_return = constituents(string("S[ng]"), num).add_left(string("NP"), num_NP_parent);
			return to_return;
		}
		if (trigger == "TO" && atom.find("VB") != std::string::npos) {
			to_return = constituents(string("S[to]"), num).add_left(string("NP"), num_NP_parent);
			return to_return;
		}
	}
	return constituents(string("NP"), num); // default
}

static constituents function_composition(constituents constit)
// simplify the composition tree by using function composition
// (explained in the CCGbank manual)
{
	FTree<pair<string, int> > ctree = constit.tree();
	constituents check1("\\(_A,_A)");
	constituents_mgu dummy_mgu;
	if (check1.unify(constit, &dummy_mgu)) {
		ctree = FTree<pair<string, int> >(ctree.begin().firstChild());
	}
	constituents check2("/(\\(S[ng],_A),S[ng])");
	if (check2.unify(constit, &dummy_mgu)) {
		constituents pred("_A");
		pred / dummy_mgu;
		ctree = pred.tree();
	}
	return constituents(ctree);
}

static PredTree correct_period(PredTree to_return)
{
	// put the period at the level of the ROOT. It gives a mistake if
	// you do it after the bynarization: it can give you three
	// children below root.

	PredTree::iterator pi = to_return.begin();
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

static PredTree sign_names(PredTree to_sign)
{
	PredTree::height_iterator piter(to_sign, 1);
	PredTree::iterator pend(to_sign.end());

	string str;

	for (; piter != pend; ++piter) {
		str = piter->str;
		if (str.find("DT") != string::npos || str.find("NN") != string::npos || str.find("NNS") != string::npos
				|| str.find("NNP") != string::npos || str.find("NNPS") != string::npos || str.find("PRP") != string::npos
				|| str.find("PRP$") != string::npos || str.find("JJ") != string::npos || str.find("VBJ") != string::npos
				|| str.find("JJR") != string::npos || str.find("JJS") != string::npos || str.find("CD") != string::npos
				|| str.find("$") != string::npos
				//|| str.find("RB") != string::npos /// check again!
						) {
			piter.firstChild()->str += "[Nish]";
		}
		if (str.find("WP") != string::npos
		//|| str.find("WDT") != string::npos
				) {
			piter.firstChild()->str += "[WP][Nish]";
		}
//	  if( str.find("VBG") != string::npos) {
//	       piter.firstChild()->str += "[VBG][Nish]";
//	  }
	}

	return to_sign;
}

static composition get_corrected_composition(composition comp)
// correct mistakes in the composition tree
{
	composition_tree ctree = comp.tree();
	composition_tree::height_iterator citer(ctree, 1);
	composition_tree::iterator cend = ctree.end();

	map<int, int> subst;
	int sx_int, new_int;

	for (; citer != cend; ++citer) {
		FTree<pair<string, int> > constit_tree = citer->tree();
		if (constit_tree.height() == 2) {
			pair<string, int> slash = *constit_tree.begin();
			pair<string, int> sx = *constit_tree.begin().firstChild();
			pair<string, int> dx = *constit_tree.begin().lastChild();
			if ( (slash.first == "/" || slash.first == "\\")
				&& sx.first == "NP" && sx == dx) { // search for candidates like NP_i/NP_i in the lowest row and transform them into NP_i
				sx_int = constit_tree.begin().firstChild()->second;
				FTree<pair<string, int> > tmp_tree(make_pair("NP", sx_int));
				citer->tree() = tmp_tree;
			}
			if ( (slash.first == "/" || slash.first == "\\")
				&& sx.first == "S[dcl]" && sx == dx) { // cases like (S[dcl]_0 (S[dcl]_0/S[dcl]_0 ... ) )
				FTree<pair<string, int> > parent_tree = citer.parent()->tree();
				sx_int = constit_tree.begin().firstChild()->second;
				if (parent_tree.height() == 1 && parent_tree.begin()->first == "S[dcl]"
						&& parent_tree.begin()->second == sx_int) {
					FTree<pair<string, int> > tmp_tree(make_pair("/", sx_int));
					if (sx_int == 0)
						new_int = sx_int + 1; // the current sentence is connected to the following one (sx_int == 0)
					else
						new_int = sx_int - 1; // the current sentence is connected to the previous one
					tmp_tree.appendChild(tmp_tree.begin(), make_pair("S[dcl]", new_int));
					tmp_tree.appendChild(tmp_tree.begin(), make_pair("S[dcl]", sx_int));
					citer->tree() = tmp_tree;
				}
			}

		}
	}

	return composition(ctree);
}

static bool is_tough_move(PredTree pt)
{
	PredTree::height_iterator pi(pt, 0);
	bool verb_be_trigger = false, adj_trigger = false;
	metric *d = metric_singleton::get_metric_instance();
	string str;
	vector<string> trigger_strings;
	trigger_strings.push_back("easy");
	trigger_strings.push_back("simple");
	trigger_strings.push_back("tough");
	trigger_strings.push_back("hard");

	for (; pi != pt.end(); ++pi) {
		str = pi->str;
		if (str == "am" || str == "is" || str == "are" || str == "was" || str == "were") {
			verb_be_trigger = true;
		}
		if (verb_be_trigger
				&& find(trigger_strings.begin(), trigger_strings.end(), str) != trigger_strings.end()) {
			adj_trigger = true;
		}
		if (verb_be_trigger && adj_trigger && str == "to")
			return true;
	}

	return false;
}

static constituents get_top_name(PredTree binary)
{
	PredTree::iterator pi(binary);

	string trigger, atom;
	bool sbar_trigger = false;
	constituents to_return;

	atom = pi->str;
	if (atom == "VP:h") {
		return constituents(string("VP"));
	}
	for (; pi != binary.end(); ++pi) {
		atom = pi->str;
		if (atom == "?" && trigger != "q") {
			trigger = "q";
			pi = binary.begin();
		}
		if (atom.find("VP") != std::string::npos // for tough move
		&& pi.lastChild() != binary.end() && pi.lastChild()->str.find("ADJP") != string::npos && sbar_trigger == false
				&& is_tough_move(pi.parent())) {
			return constituents(string("S[adj]"), 0).add_left(string("NP"), 0);
			//return constituents(string("S[adj]"), 0);
			/// Check that the subject is a lonely DT!!
		}
		if (atom.find("SBAR") != std::string::npos) {
			sbar_trigger = true;
		} else if (trigger == "q" && atom.find("WH") != std::string::npos) {
			return constituents(string("S[wq]"), 0);
		}
	}

	if (trigger == "q")
		return constituents(string("S[q]"), 0);
	return constituents(string("S[dcl]"), 0);
}

static bool has_S_parent(const constituents &constit)
{
	if (constit.tree().begin()->first.find("S[") != string::npos)
		return true;
	FTree<constit_element> ctree = constit.tree();
	FTree<constit_element>::iterator citer = ctree.begin();
	FTree<constit_element>::iterator cend = ctree.end();
	++citer;
	for (; citer != cend; ++citer) {
		if (citer->first.find("S[") != string::npos)
			return true;
	}

	return false;
}

static composition_tree get_composition_tree(PredTree binary)
{
	PredTree::iterator pi = binary.begin();
	composition_tree comptree(binary.begin()); // creates a void composition tree

	constituents top = get_top_name(binary);
	*comptree.begin() = top;

	composition_tree::iterator ci = comptree.begin();

	vector<constituents> already_used;

	int num = 0;
	for (; pi != binary.end(); ++pi, ++ci) {
		if (pi.height() > 1 && pi.num_children() == 2) {

			PredTree::iterator left_iter = PredTree::iterator(*pi.node->firstChild);
			PredTree::iterator right_iter = PredTree::iterator(*pi.node->lastChild);

			constituents complement;
			constituents constit_head = *ci;
			string left_string = left_iter->str;
			string right_string = right_iter->str;
			int left_size = left_string.size();
			int right_size = right_string.size();

			composition_tree::iterator constit_left_iter = composition_tree::iterator(*ci.node->firstChild);
			composition_tree::iterator constit_right_iter = composition_tree::iterator(*ci.node->lastChild);
			constituents constit_left = *constit_left_iter, constit_right = *constit_right_iter;

			// It is N, JJ, PRP, or DT
			if (pi->str.find("NP:h") != std::string::npos || pi->str.find("NP:c") != std::string::npos
					|| pi->str.find("NP:a") != std::string::npos) { // It must not be "NP[conj]:" !!
				if ((left_string.find(":a") != string::npos
						|| (left_string.find(":h") != string::npos && right_string.find(":c") != string::npos))
						&& pi.firstChild().height() == 1
						&& (left_string.find("NN") != string::npos || left_string.at(0) == 'J'
								|| left_string.find("CD:") != std::string::npos
								|| left_string.find("DT:") != std::string::npos
								|| left_string.find("PRP:") != std::string::npos
								|| left_string.find("PRP$:") != std::string::npos)
						&& (right_string.find("NN") != string::npos || right_string.at(0) == 'J'
								|| right_string.find("CD:") != std::string::npos
								|| right_string.find("DT:") != std::string::npos
								|| right_string.find("PRP:") != std::string::npos
								|| right_string.find("PRP$:") != std::string::npos)) {
					int num_parent = get_num_NP_parent(constit_head);
					//constit_right= constituents("NP",num_parent);
					constit_right = constituents(constit_head, num_parent);
					constit_left = constituents("N", num).add_right(constit_head);
					++num;
					if (left_string.find("[ignore]") == string::npos)
						*constit_left_iter = constit_left;
					if (right_string.find("[ignore]") == string::npos)
						*constit_right_iter = constit_right;
					continue;
				} else if ((right_string.find(":a") != string::npos
						|| (right_string.find(":h") != string::npos && left_string.find(":c") != string::npos))
						&& pi.lastChild().height() == 1
						&& (right_string.find("NN") != string::npos || right_string.at(0) == 'J'
								|| right_string.find("CD:") != std::string::npos
								|| right_string.find("DT:") != std::string::npos
								|| right_string.find("PRP:") != std::string::npos
								|| right_string.find("PRP$:") != std::string::npos)) {
					int num_parent = get_num_NP_parent(constit_head);
					constit_left = constituents(constit_head, num_parent);
					constit_right = constituents("N", num).add_left(constit_head);
					++num;
					if (left_string.find("[ignore]") == string::npos)
						*constit_left_iter = constit_left;
					if (right_string.find("[ignore]") == string::npos)
						*constit_right_iter = constit_right;
					continue;
				}
			}
			if (left_string.at(left_size - 1) == 'h') { // head is on the left (index num is increased)
				PredTree subtree(right_iter);
				if (right_string.find("[conj]") != std::string::npos) { // the right is a conjuntion
					constit_right = constit_head;
					constit_left = constit_head;
				} else if (right_string.at(right_size - 1) == 'a') { // the right is an adjunct
					if (pi->str.find(":h") == string::npos
							&& (left_string == "S:h" || left_string == "SBAR:h" || left_string == "ROOT:h"
									|| left_string == "SQ:h" || left_string == "SBARQ:h" || left_string == "PRN:h")) {
						int num_parent = constit_head.tree().begin()->second;
						constit_left = constituents(get_top_name(binary).tree().begin()->first, num_parent);
					} else
						constit_left = constit_head;
					constit_right = function_composition(
							function_composition(constit_head).add_left(function_composition(constit_left), num));
				} else if (right_string.at(right_size - 1) == 'c') { // the right is a complement
					int num_parent = max(num + 1, get_num_NP_parent(constit_head) + 1);
					complement = get_complement_name(subtree, num + 1, num_parent, pi->str, constit_head);
					complement.tree().begin()->second = num_parent; /// Do constituents.topNumber()
					if (shortfind(already_used, complement)) {
						complement.tree().begin()->second += 100; ///
					} else
						already_used.push_back(complement);
					constit_left = constituents(constit_head).add_right(complement);
					constit_right = constituents(complement);
					num += 2;
				}
			} else if (right_string.at(right_size - 1) == 'h') { // if head is on the right (index num is increased)

				PredTree subtree(left_iter);
				if (left_string.find("-conj-") != std::string::npos) { // the left is a conjuntion
					constit_left = constit_head;
					constit_right = constit_head;
				} else if (left_string.at(left_size - 1) == 'a') { // the left is an adjunct
					if ((pi.parent() != binary.end() && pi.parent()->str.find(":h") == string::npos)
							&& (right_string == "S:h" || right_string == "SBAR:h" || right_string == "ROOT:h"
									|| right_string == "SQ:h" || right_string == "SBARQ:h" || right_string == "PRN:h")
							) {
						++num;
						constit_right = constituents(get_top_name(binary).tree().begin()->first, num);
					}
					else {
						constit_right = constit_head;
						num = constit_right.tree().begin()->second;
						++num;
					}
					constit_left = constit_right;
					constit_left.tree().begin()->second = num;
					constit_left = function_composition(constit_left.add_right(function_composition(constit_head))); /// h/a
					++num;
				} else if (left_string.at(left_size - 1) == 'c') { // the left is a complement
					int num_parent = max(num + 1, get_num_NP_parent(constit_head) + 1);
					complement = get_complement_name(subtree, num + 1, num_parent, pi->str, constit_head);
					if (shortfind(already_used, complement)) {
						complement.tree().begin()->second += 100; ///
					} else
						already_used.push_back(complement);
					constituents pivot_constit;
					if (right_string == "VP:h" && !has_S_parent(constit_head)) {
						// A new VP is created if the head is not a S[...]
						pivot_constit = constituents("VP", get_num_NP_parent(constit_head));
						constit_right = constituents(pivot_constit).add_left(constit_head);
						constit_left = constituents(constit_head);
					} else {
						pivot_constit = constit_head;
						constit_right = constituents(pivot_constit).add_left(complement);
						constit_left = constituents(complement);
					}
					if (constit_left.tree().begin()->first.find("NP") == string::npos)
						++num;
				}
			} else if (right_string.at(right_size - 1) == 'c') {
				if (right_string.find("[conj]") != std::string::npos) { // the right is a conjuntion
					constit_left = constit_head;
					constit_right = constit_head;
				} else {
					PredTree subtree(right_iter);
					// same as with the head
					complement = get_complement_name(subtree, num + 1);
					if (shortfind(already_used, complement)) {
						complement.tree().begin()->second += 100; ///
					} else
						already_used.push_back(complement);
					constit_left = constituents(complement).add_right(constit_head);
					constit_right = constituents(constit_head);
					num += 2;
				}
			} else if (right_string.at(right_size - 1) == 'a') {
				// same as previous but do not increase the index
				PredTree subtree(left_iter);
				if (left_string.find("-conj-") != std::string::npos) { // the left is a conjuntion
					constit_left = constit_head;
					constit_right = constit_head;
				} else {
					int num_parent = max(num + 1, get_num_NP_parent(constit_head) + 1);
					complement = get_complement_name(subtree, num + 1, num_parent, pi->str, constit_head);
					if (shortfind(already_used, complement)) {
						complement.tree().begin()->second += 100; ///
					} else
						already_used.push_back(complement);
					complement.tree().begin()->second = num_parent;
					constit_right = complement.add_left(constit_head, num);
					constit_left = constituents(constit_head, num);

					if (constit_left.tree().begin()->first.find("NP") == string::npos)
						++num;
				}
			}
			if (left_string.find("[ignore]") == string::npos)
				*constit_left_iter = constit_left;
			if (right_string.find("[ignore]") == string::npos)
				*constit_right_iter = constit_right;
		} else if (pi.height() > 1 && pi.num_children() == 1) { // if there is only one child
			constituents constit_head = *ci;
			PredTree::iterator left_iter = PredTree::iterator(*pi.node->firstChild);
			string left_string = left_iter->str;
			int left_size = left_string.size();
			composition_tree::iterator constit_left_iter = composition_tree::iterator(*ci.node->firstChild);
			constituents constit_left = *constit_left_iter;

			if (left_string.find("RB") != std::string::npos) {
				constit_left = constit_head;
				++num;
			}
			// if there is a lonely :h
			else if (left_string.find(":h") != std::string::npos) {
				constit_left = constit_head;
				++num;
			} else { //default (it must be optimized!!!)
				constit_left = constit_head;
				++num;
			}
			*constit_left_iter = constit_left;
		}
	}

	return comptree;
}

PredTree pre_correct(PredTree parsed)
{
	return parsed;
}

phrase::phrase(const FuzzyPred &pred, PhraseInfo *pi) :
		parsed(pred.pred()), has_question_(false), has_condition_(false), phrase_info_(pi)
{
	clock_t start;
	if (debug || measure_time)
		start = clock();

	this->compute_names(pred);
	this->compute_tags(pred);
	this->compute_prn_depths(pred);

	//PredTree parse_corrected= pre_correct(parsed);
	PredTree constit = get_constituent_types(parsed, phrase_info_);
	constit = process_prn(constit); // pre-process PRN sub-sentences

	if (debug)
		Predicate(constit).print();

	constit = sign_names(constit); // The names and articles are signed by adding [Nish]

	constit = correct_period(constit);

	PredTree binary = get_binary(constit);
	binary = get_corrected(binary); // corrects small mistakes and implements exceptions
	string str;
	if (debug) {
		printLikeTree(binary, str);
		std::cout << str << std::endl;
	}

	if (debug||measure_time) {
		clock_t end = clock();
		cout << "Mtime_phrase1::: " << (end - start) / (double) CLOCKS_PER_SEC << endl;
	}
	if (debug || measure_time)
		start = clock();

	composition_ = composition(get_composition_tree(binary));
	composition_ = get_corrected_composition(composition_);
	if (debug)
		composition_.print_like_tree(std::cout);

	connections_ = composition_.get_connections(binary);
	for (int n = 0; debug && n < connections_.size(); ++n) {
		int pos1 = connections_.at(n).first.first, pos2 = connections_.at(n).first.second;
		constituents unif = connections_.at(n).second;
		string arrow;
		if (pos1 > pos2)
			arrow = " <- ";
		else
			arrow = " -> ";
		std::cout << names_.at(pos1) << arrow << names_.at(pos2) << " (" << unif << ")" << std::endl;
	}
	if (debug)
		std::cout << "------------" << std::endl;
	compute_constit(composition_);

	if (debug||measure_time) {
		clock_t end = clock();
		cout << "Mtime_phrase2::: " << (end - start) / (double) CLOCKS_PER_SEC << endl;
	}

	if (debug || measure_time)
		start = clock();

	drt_builder builder(this);
	tuple<DrtVect, DrtVect, DrtPred> tuple_drt = builder.get_drt_form();
	drt_form_      = tuple_drt.get<0>();
	orig_drt_form_ = tuple_drt.get<1>();
	error_         = tuple_drt.get<2>();

	this->restore_names(pred);

	if (debug||measure_time) {
		clock_t end = clock();
		cout << "Mtime_phrase3::: " << (end - start) / (double) CLOCKS_PER_SEC << endl;
	}
}

vector<int> phrase::get_prn_depths() const
{
	return prn_depths_;
}

int phrase::get_num_elements() const
{
	if (drt_form_.size() == 0)
		return -1;

	int num = 0;
	int num_elements = 0;

	vector<DrtPred>::const_iterator diter = drt_form_.begin();
	vector<DrtPred>::const_iterator dend = drt_form_.end();
	vector<string> already_parsed;
	string prev_str;

	for (; diter != dend; ++diter) {
		string header = extract_header(*diter);
		if(diter->is_verb()) {
			if(!has_subject(*diter)) {
				cout << "HAS_NOT_SUBJ::" << *diter << endl;
				num -= 1;
			}
		}
	}

	return num;
}

PhraseInfo* phrase::getInfo()
{
	return phrase_info_;
}

DrtPred phrase::getError() const
{
	return error_;
}
