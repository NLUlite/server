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



#include"constituents.hpp"

const bool debug = false;

// constituent class

std::ostream & operator <<(std::ostream &out, const constituents & ct)
{
	ct.print(out);
	return out;
}

void constituents::print_like_pred(std::ostream &out) const
{
	FTree<std::pair<string, int> >::iterator i = tree_.begin();

	int d = i.depth();
	int diff;
	while (i != tree_.end()) {
		if (i.depth() > d) {
			++d;
			out << '(';
		}
		if ((diff = d - i.depth()) > 0) {
			d -= diff;
			for (; diff > 0; --diff)
				out << ')';
		}
		if (i.parent() != tree_.end() && i.parent().firstChild() != i)
			out << ',';
		out << i->first << "_" << i->second;
		++i;
	}
	while (((--d) - tree_.begin().depth() + 1) > 0)
		out << ')';
}

static inline string::size_type matchFirstOpeningPar(const string &str, int strIter)
{
	int counter = 1;
	while (counter != 0)
		switch (str[++strIter]) {
		case '(':
			++counter;
			break;
		case ')':
			--counter;
			break;
		default:
			break;
		}
	return strIter;
}

static inline int min(int a, int b)
{
	return (a < b ? a : b);
}

static inline pair<string, int> get_constitutent_pair_from_string(string str)
{
	bool is_unifier = false;
	if (str.at(0) == '_') {
		is_unifier = true;
		str = str.substr(1, str.size() - 1);
	}
	vector<string> pairs;
	boost::split(pairs, str, boost::is_any_of("_"));
	str = pairs.at(0);
	int number = 0;
	if (pairs.size() > 1)
		number = boost::lexical_cast<int>(pairs.at(1));
	if (is_unifier)
		str.insert(0, "_");
	return make_pair(str, number);
}

static inline void recursInternals(std::string str, FTree<pair<string, int> > &pt,
		const FTree<pair<string, int> >::iterator &ptIter)
//Process predicate's (pred1, pred2, ....) internals
{
	string::size_type strIter = 0, commaIter, oParIter, cParIter; // Opening and closing parenthesis
													  // index.
	FTree<pair<string, int> >::iterator childIter;

	do {
		commaIter = str.find(',', strIter);
		oParIter = str.find('(', strIter);

		if (commaIter <= oParIter) { // It works even if commaIter=oParIter=npos.
			pair<string, int> element = get_constitutent_pair_from_string(str.substr(strIter, commaIter - strIter));
			pt.appendChild(ptIter, element);
		} else {
			cParIter = matchFirstOpeningPar(str, str.find('(', strIter));
			childIter = pt.appendChild(ptIter, make_pair(str.substr(strIter, oParIter - strIter), 0));
			recursInternals(str.substr(oParIter + 1, cParIter - oParIter - 1), pt, childIter);
			strIter = cParIter;
		}
	} while ((strIter = str.find(',', strIter + 1) + 1) != std::string::npos + 1);
	// Not using commaIter: commaIter could be a nested comma.
}

constituents::constituents(const string &literal)
// Reads a literal/predicate
// ..well, just reading up to the first '(', recursInternals does it all.
{
	FTree<pair<string, int> > pt;

	std::string str(literal);
	//  strip(str);

	int d = 0;
	for (int n = 0; n < str.size(); ++n) {
		if (str.at(n) == '(')
			++d;
		if (str.at(n) == ')')
			--d;
	}
	if (d > 0)
		throw(std::runtime_error("String " + str + " cannot be parsed to a Constituents tree!"));

	if (str.size() != 0) {
		std::string::size_type oParIter, cParIter;
		oParIter = str.find('(');
		cParIter = str.rfind(')');
		pt.rename(pt.begin(), make_pair(str.substr(static_cast<std::string::size_type>(0), oParIter), 0));
		if (oParIter != -1)
			recursInternals(str.substr(oParIter + 1, cParIter - oParIter - 1), pt, pt.begin());
		tree_ = pt;
	}
}

constituents::constituents() :
		tree_(make_pair(string(""), 0))
{
}

constituents::constituents(string str, int n) :
		tree_(make_pair(str, n))
{
}
constituents::constituents(const constituents &c, int n) :
		tree_(c.tree_)
{
}

constituents constituents::add_left(string str, int n)
{
	std::pair<string, int> tmp_pair(make_pair(string("\\"), n));
	std::pair<string, int> to_add(make_pair(str, n));
	FTree<std::pair<string, int> > tmp(tmp_pair);
	tmp.appendTree(tmp.begin(), tree_);
	tmp.appendChild(tmp.begin(), to_add);
	tree_ = tmp;
	return *this;
}
constituents constituents::add_right(string str, int n)
{
	std::pair<string, int> tmp_pair(make_pair("/", n));
	std::pair<string, int> to_add(make_pair(str, n));
	FTree<std::pair<string, int> > tmp(tmp_pair);
	tmp.appendTree(tmp.begin(), tree_);
	tmp.appendChild(tmp.begin(), to_add);
	tree_ = tmp;
	return *this;
}
constituents constituents::add_left(const constituents &c, int n)
{
	std::pair<string, int> tmp_pair(make_pair(string("\\"), n));
	FTree<std::pair<string, int> > tmp(tmp_pair);
	tmp.appendTree(tmp.begin(), tree_);
	tmp.appendTree(tmp.begin(), c.tree_);
	tree_ = tmp;
	return *this;
}
constituents constituents::add_right(const constituents &c, int n)
{
	std::pair<string, int> tmp_pair(make_pair("/", n));
	FTree<std::pair<string, int> > tmp(tmp_pair);
	tmp.appendTree(tmp.begin(), tree_);
	tmp.appendTree(tmp.begin(), c.tree_);
	tree_ = tmp;
	return *this;
}
constituents constituents::add_left(const constituents &c)
{
	int n = c.tree_.begin()->second;
	std::pair<string, int> tmp_pair(make_pair(string("\\"), n));
	FTree<std::pair<string, int> > tmp(tmp_pair);
	tmp.appendTree(tmp.begin(), tree_);
	tmp.appendTree(tmp.begin(), c.tree_);
	tree_ = tmp;
	return *this;
}
constituents constituents::add_right(const constituents &c)
{
	int n = c.tree_.begin()->second;
	std::pair<string, int> tmp_pair(make_pair("/", n));
	FTree<std::pair<string, int> > tmp(tmp_pair);
	tmp.appendTree(tmp.begin(), tree_);
	tmp.appendTree(tmp.begin(), c.tree_);
	tree_ = tmp;
	return *this;
}

void constituents::print(std::ostream &out) const
{
	FTree<std::pair<string, int> >::iterator i = tree_.begin();
	FTree<std::pair<string, int> >::iterator i2;
	if (tree_.height() == 1) {
		out << i->first << "_" << i->second;
		;
		return;
	}
	int d = i.depth(), depth; //, height;
	int diff;
	while (i != tree_.end()) {
		depth = i.depth();
		//height= i.height();
		if (depth > d) {
			++d;
			out << '(';
		}
		if ((diff = d - depth) > 0) {
			d -= diff;
			// if(i2.node->parent != i.node->parent)
			// 	    i2=i;
			for (; diff > 0; --diff) {
				out << ')';
				//out << '_'<< i.node->parent->data.second ;
				i2 = *i2.node->parent;
				out << '_' << i2->second;
				//out << '-'<< i2->first ;
			}
		}
		if (i.parent() != tree_.end() && i.parent().firstChild() != i)
			out << i.node->parent->data.first;
		if (i->first != "\\" && i->first != "/") {
			out << i->first << "_" << i->second;
		}
		i2 = i; // for the indices;
		++i;
	}
	i = *tree_.begin().node->lastChild;
	while (i.node->lastChild)
		i = *i.node->lastChild;
	if (i.node->parent)
		i = *i.node->parent;
	while (((--d) - tree_.begin().depth() + 1) > 0) {
		out << ')';
		if (i.node  // && i.node->parent->parent
		) {
			out << '_' << i->second; //i.node->parent->data.second ;
			//out << '-'<< i->first ;
			i = *i.node->parent;
		}
	}
}

static bool is_unifier(const pair<string, int> &element)
{
	if (element.first.size() && element.first.at(0) == '_')
		return true;
	return false;
}

static bool is_pair_equal(const pair<string, int> &lhs, const pair<string, int> &rhs)
{
	if ((lhs.first.at(0) == '_' && rhs.first.at(0) == '_') || (lhs.first.at(0) == 'S' && rhs.first.at(0) == 'S')
			|| (lhs.first.at(0) == 'V' && rhs.first.at(0) == 'V') || (lhs.first.at(0) == '\\' && rhs.first.at(0) == '\\')
			|| (lhs.first.at(0) == '/' && rhs.first.at(0) == '/'))
		return lhs.first == rhs.first;
	return lhs == rhs;
}

bool constituents::can_unify(const constituents &rhs) const
// Just Universal quantifiers '_*' are substituted in unification. 
{
	if (rhs.tree().begin() == rhs.tree().end()) {
		return false;
	}
	FTree<pair<string, int> > sxTree = tree_;
	FTree<pair<string, int> > dxTree = rhs.tree_;

	FTree<pair<string, int> >::iterator sxIter = sxTree.begin(), end = FTree<pair<string, int> >::iterator(), dxIter =
			dxTree.begin();

	if (is_unifier(*sxIter)) {
		return true;
	}
	if (is_unifier(*dxIter)) {
		return true;
	}
	if (++sxIter == end || ++dxIter == end) {
		if (tree_ == rhs.tree_)
			return true;
		return false;
	}

	sxIter = sxTree.begin();
	dxIter = dxTree.begin();

	while (sxIter != end && dxIter != end) {
		if (!is_pair_equal(*sxIter, *dxIter) && !(is_unifier(*sxIter) && is_unifier(*dxIter))) {
			if (is_unifier(*sxIter)) {
				FTree<pair<string, int> > tmp_tree = dxTree.subTree(dxIter);
				sxIter = sxTree.replace(tmp_tree, sxIter);
			} else if (is_unifier(*dxIter)) {
				FTree<pair<string, int> > tmp_tree = sxTree.subTree(sxIter);
				dxIter = dxTree.replace(tmp_tree, dxIter);
			} else
				return false;

			sxIter.node = sxIter.firstLeafLeft();
			dxIter.node = dxIter.firstLeafLeft();
		} else {
			++sxIter;
			++dxIter;
		}
	}

	if (sxIter != dxIter)
		return false;
	return true;
}

bool constituents::unify(const constituents &rhs, constituents_mgu *retUpg) const
// Just Universal quantificator '_*' are substituted in unification. 
{
	if (rhs.tree().begin() == rhs.tree().end()) {
		return false;
	}
	FTree<pair<string, int> > sxTree = tree_;
	FTree<pair<string, int> > dxTree = rhs.tree_;

	FTree<pair<string, int> >::iterator sxIter = sxTree.begin(), end = FTree<pair<string, int> >::iterator(), dxIter =
			dxTree.begin();

	if (is_unifier(*sxIter)) {
		if (retUpg)
			retUpg->add(*sxTree.begin(), dxTree);
		return true;
	}
	if (is_unifier(*dxIter)) {
		if (retUpg)
			retUpg->add(*dxTree.begin(), sxTree);
		return true;
	}
	if (++sxIter == end || ++dxIter == end) {
		if (tree_ == rhs.tree_)
			return true;
		return false;
	}

	clink uni;

	sxIter = sxTree.begin();
	dxIter = dxTree.begin();

	while (sxIter != end && dxIter != end) {
		if (!is_pair_equal(*sxIter, *dxIter) && !(is_unifier(*sxIter) && is_unifier(*dxIter))) {
			if (is_unifier(*sxIter)) {
				uni.first = *sxIter;
				uni.second = dxTree.subTree(dxIter);
				sxIter = sxTree.replace(dxTree.subTree(dxIter), sxIter);
			} else if (is_unifier(*dxIter)) {
				uni.first = *dxIter;
				uni.second = sxTree.subTree(sxIter);
				dxIter = dxTree.replace(sxTree.subTree(sxIter), dxIter);
			} else
				return false;

			if (uni.second.findData(uni.first, uni.second.begin()) != end) {
				throw std::runtime_error("No Unification: Recursion in unifier!\n");
				//std::cerr << "No Unification: Recursion in unifier!\n";
				//return false;
			}
			if (retUpg) {
				//*retUpg/uni; /// This must be implemented !!!
				retUpg->push_back(uni);
			}

			sxIter.node = sxIter.firstLeafLeft();
			dxIter.node = dxIter.firstLeafLeft();
		} else {
			++sxIter;
			++dxIter;
		}
	}
	if (sxIter != dxIter)
		return false;
	return true;
}

void constituents::operator /(const constituents_mgu &upg)
{
	FTree<pair<string, int> >::iterator pBegin = tree_.begin();
	constituents_mgu::const_iterator ui = upg.begin(), uend = upg.end();
	while (ui != uend) {
		tree_.replaceEachData(ui->first, ui->second, pBegin);
		++ui;
	}
}

static bool is_valid_subset(const FTree<std::pair<string, int> > &lhs, const FTree<std::pair<string, int> > &rhs)
{
	bool bool_return = false;

	FTree<std::pair<string, int> >::iterator i1;
	for (i1 = lhs.begin(); i1 != lhs.end(); ++i1) {
		FTree<std::pair<string, int> > subtree1(i1);
		if (rhs == subtree1)
			bool_return = true;
		if (i1->first == "N") {
			bool_return = false;
		}
	}
	return bool_return;
}

bool constituents::can_co_unify(const constituents &rhs, int pos1, int pos2, constituents *constit, // The result of the unification
		bool left_nish, bool right_nish)
// 
{
	FTree<std::pair<string, int> >::iterator i1, i2;
	FTree<std::pair<string, int> > rhs_tree(rhs.tree_);

	int safe = 0, safe_max = 20000;

	for (i1 = tree_.begin(); i1 != tree_.end(); ++i1) {
		for (i2 = rhs_tree.begin(); i2 != rhs_tree.end(); ++i2, ++safe) {

			if (debug) {
				cout << "SAFE2::: " << safe << endl;
			}

			if (safe > safe_max)
				throw std::runtime_error("Constituents unification exceeded the safety limit!");

			bool break_trigger = false;
			FTree<std::pair<string, int> > subtree1(i1);
			FTree<std::pair<string, int> > subtree2(i2);
			if (i1.hasParent()) {
				FTree<std::pair<string, int> > subtree1_pl(i1.parent().lastChild());
				FTree<std::pair<string, int> > subtree1_pf(i1.parent().firstChild());
				if (i1.parent()->first == "\\" && subtree1_pl == subtree1 && pos2 < pos1)
					break_trigger = true;
				if (i1.parent()->first == "/" && subtree1_pf == subtree1 && pos2 < pos1)
					break_trigger = true;
			}
			if (break_trigger && i2.hasParent()) {
				FTree<std::pair<string, int> > subtree2_pl(i2.parent().lastChild());
				FTree<std::pair<string, int> > subtree2_pf(i2.parent().firstChild());
				if (i2.parent()->first == "/" && subtree2_pf == subtree2 && pos1 < pos2)
					continue;
				if (i2.parent()->first == "\\" && subtree2_pl == subtree2 && pos1 < pos2)
					continue;
			}

			if (subtree1 == subtree2 && (subtree1.begin()->first.find("NP") == string::npos || left_nish)
					&& (subtree2.begin()->first.find("NP") == string::npos /// Do you need this?
					|| right_nish)) { // Unification with NP must be exact (this is for the rest).
				*constit = constituents(subtree1);
				return true;
			} else if (subtree1 == rhs_tree || is_valid_subset(subtree1, rhs_tree)) { // This is about unification with NP
				*constit = constituents(rhs_tree);
				return true;
			} else if (subtree2 == tree_ || is_valid_subset(tree_, subtree2)) { // This is about unification with NP
				*constit = constituents(subtree2);
				return true;
			}
		}
	}
	return false;
}

// composition_tree class

composition_tree::composition_tree()
{
	//head= new Node<constituents>(constituents("S[dcl]", 0) );
	head->data = constituents("S[dcl]", 0);
}

composition_tree::~composition_tree()
{
	//delete head;
}

composition_tree::composition_tree(const PredTree::iterator &i)
{
	Node<PTEl> *run = i.node;
	head->data = constituents(run->data.str, 0);
	iterator inTree(*head);

	if (run == NULL)
		throw TreeError((std::string) "Parsing a void node in the composition tree");
	if (run->firstChild) {
		run = run->firstChild;
		inTree = appendChild(inTree, constituents(run->data.str, 0));
		while (true) {
			if (run->parent == i.node && !run->firstChild && !run->nextSibling)
				break;
			else if (run->firstChild) {
				run = run->firstChild;
				inTree = appendChild(inTree, constituents(run->data.str, 0));
			} else if (run->nextSibling) {
				run = run->nextSibling;
				inTree = appendChild(iterator(*inTree.node->parent), constituents(run->data.str, 0));
			} else {
				while (run->parent && !run->nextSibling && run->parent != i.node) {
					run = run->parent;
					inTree = iterator(*inTree.node->parent);
				}
				if (run->nextSibling) {
					run = run->nextSibling;
					inTree = appendChild(iterator(*inTree.node->parent), constituents(run->data.str, 0));
				} else
					break;
			}
		}
	}
}

// composition class

void composition::print(std::ostream &out) const
{
	FTree<constituents>::iterator i = ctree_.begin();

	int d = i.depth(), depth;
	int diff;
	while (i != ctree_.end()) {
		depth = i.depth();
		if (depth > d) {
			++d;
			out << '{';
		}
		if ((diff = d - depth) > 0) {
			d -= diff;
			for (; diff > 0; --diff)
				out << '}';
		}
		if (i.parent() != ctree_.end() && i.parent().firstChild() != i)
			out << ",";
		out << *i;
		++i;
	}
	while (((--d) - ctree_.begin().depth() + 1) > 0)
		out << '}';
}

void composition::print_like_tree(std::ostream &out) const
{
	FTree<constituents>::iterator i = ctree_.begin();
	while (i != ctree_.end()) {
		std::cout << "+ " << std::string(2 * (i.depth()), ' ') << "" << *i << "" << std::endl;
		++i;
	}
}


FTree<constituents>::iterator composition::find_head(FTree<constituents>::iterator &unif_iter) const
{
	static int test = 0;

	constituents candidate_head1, candidate_head2, candidate_head3, candidate_non_head1, candidate_non_head2, parent, left,
			right;
	FTree<constituents>::iterator to_the_head = unif_iter;
	while (to_the_head.node->firstChild) { /// This is horrible. Use composition tree instead
		if (to_the_head.num_children() == 1) {
			to_the_head = *to_the_head.node->firstChild;
		}
		else if (to_the_head.node->firstChild && to_the_head.node->lastChild) {
			left = to_the_head.node->firstChild->data;
			right = to_the_head.node->lastChild->data;
			parent = *to_the_head;
			candidate_head1 = parent;
			candidate_head2 = parent.add_left(left);
			candidate_head3 = parent.add_right(right);
			candidate_non_head1 = constituents(right).add_right(right);
			candidate_non_head2 = constituents(left).add_left(left);

			if (left == candidate_head1 || left == candidate_head3)
				to_the_head = *to_the_head.node->firstChild;
			else if (right == candidate_head1 || right == candidate_head2)
				to_the_head = *to_the_head.node->lastChild;
			else if (right == candidate_non_head2)
				to_the_head = *to_the_head.node->firstChild;
			else if (left == candidate_non_head1)
				to_the_head = *to_the_head.node->lastChild;
			else
				to_the_head = *to_the_head.node->firstChild;
		}
	}
	return to_the_head;
}

int composition::count_from_start(FTree<constituents>::iterator &unif_iter) const
{
	FTree<constituents>::iterator end = unif_iter;
	FTree<constituents>::height_iterator transverse(ctree_, 0);

	int steps = 0;
	while (transverse != end) {
		++transverse;
		++steps;
	}
	return steps;
}

int composition::find_head(const constituents &constit) const
{
	int pos = -1;
	FTree<constituents>::iterator citer = ctree_.begin();

	for (; citer != ctree_.end(); ++citer) {
		if (*citer == constit)
			break;
	}
	if (citer != ctree_.end()) {
		FTree<constituents>::iterator unif_iter;
		unif_iter = find_head(citer);
		pos = count_from_start(unif_iter);
	}

	return pos;
}

vector<pair<pair<int, int>, constituents> > composition::get_connections(const PredTree &binary)
{
	vector<pair<pair<int, int>, constituents> > connections;
	FTree<constituents>::height_iterator hi1(ctree_, 1);
	FTree<constituents>::iterator unif_iter;

	int pos1, pos2;
	int safe = 0, safe_max = 20000;
	for (pos1 = 0; hi1 != ctree_.end(); ++hi1, ++pos1) {
		FTree<constituents>::height_iterator hi2(ctree_, 1);
		//pi= binary.begin();
		for (pos2 = 0; hi2 != ctree_.end(); ++hi2, ++pos2, ++safe) {
			constituents constit;  // The result of the unification
			if (debug) {
				cout << "SAFE:::" << safe << endl;
			}

			bool left_nish = false;
			bool right_nish = false;
			if (hi1.firstChild()->tree().begin()->first.find("[Nish]") != string::npos)
				left_nish = true;
			if (hi2.firstChild()->tree().begin()->first.find("[Nish]") != string::npos)
				right_nish = true;

			// Special rules apply to WP (and VBG): if there is a WP, then
			// the component NP_i can co-unify freely
			if (hi1.firstChild()->tree().begin()->first.find("[WP]") != string::npos
					|| hi1.firstChild()->tree().begin()->first.find("[VBG]") != string::npos)
				right_nish = true;
			if (hi2.firstChild()->tree().begin()->first.find("[WP]") != string::npos
					|| hi2.firstChild()->tree().begin()->first.find("[VBG]") != string::npos)
				left_nish = true;

			if (pos1 != pos2
				&& hi1.node->firstChild->data.tree().begin()->first.find("[ignore]") == std::string::npos
				&& hi2.node->firstChild->data.tree().begin()->first.find("[ignore]") == std::string::npos
				&& hi1.node->firstChild->data.tree().begin()->first.find("-period-") == std::string::npos
				&& hi2.node->firstChild->data.tree().begin()->first.find("-period-") == std::string::npos
				&& hi1.node->firstChild->data.tree().begin()->first.find("?") == std::string::npos
				&& hi2.node->firstChild->data.tree().begin()->first.find("?") == std::string::npos

				&& hi1->can_co_unify(*hi2, pos1, pos2, &constit, left_nish, right_nish)
			) {
				if (find(connections.begin(), connections.end(), make_pair(make_pair(pos1, pos2), constit))
				                 == connections.end()
				&& find(connections.begin(), connections.end(), make_pair(make_pair(pos2, pos1), constit))
				                 == connections.end()
				) {
					connections.push_back(make_pair(make_pair(pos1, pos2), constit));
				}
			}
		}
	}

	if (debug) {
		cout << "FINISHED::: " << endl;
	}

	return connections;
}

std::ostream & operator <<(std::ostream &out, const composition & comp)
{
	comp.print(out);
	return out;
}
