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



#include"Engine.hpp"
#include"../knowledge/Knowledge.hpp"

const bool debug = false;

const PredTree Engine::pt_false = CodePred("false").pred();
const PredTree Engine::pt_true = CodePred("true").pred();
const PredTree Engine::pt_nil = CodePred("nil").pred();
const PredTree Engine::pt_break = CodePred("break").pred();

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
static void print_vector_stream(std::stringstream &ff, const std::vector<T> &vs)
{
	typename vector<T>::const_iterator tags_iter = vs.begin();
	while (tags_iter < vs.end()) {
		tags_iter->print(ff);
		if (boost::next(tags_iter) != vs.end())
			ff << ",";
		++tags_iter;
	}
}

template<class T>
static bool shortfind(const vector<T> &vect, const T &element)
{
	if (find(vect.begin(), vect.end(), element) == vect.end())
		return false;
	return true;
}

TemplateInstruction::TemplateInstruction(const string &wus, const PredTree &result, const vector<string> &variables) :
		wake_up_string_(wus), pt_result_(result), variables_(variables)
{
}

string TemplateInstruction::getWakeUpString()
{
	return wake_up_string_;
}

PredTree TemplateInstruction::transform(PredTree p, Engine *e)
{
	int vsize = variables_.size();
	if (vsize) {
		PredTree::children_iterator pi(p.begin());
		PredTree::iterator pend = p.end();
		Upg upg;
		for (int n = 0; pi != pend && n < vsize; ++pi, ++n) {
			upg.add(variables_.at(n), PredTree(pi));
		}
		CodePred code(pt_result_);
		code / upg;
		CodePred result = e->run(code);
		return result.pred();
	}

	return pt_result_;
}

static string strip_spaces(string s)
{
	for (int i = 0; i < s.size(); ++i) {
		if (s.at(i) == ' ' || s.at(i) == '\n' || s.at(i) == '\r' || s.at(i) == '\t') {
			s.erase(i, 1);
			--i;
		}
	}
	return s;
}

CodePred::CodePred(const string &s)
{
	string input = strip_spaces(s);
	Predicate candidate_pred(input);
	PredTree pt = candidate_pred.pred();
	PredTree::iterator piter = pt.begin(), pend = pt.end();
	for (; piter != pend; ++piter) {
		string str = piter->str;
		if (str.size() && str != "\'" && str.at(0) == '\'') {
			str = str.substr(1, str.size());
			PredTree pt2 = PredTree(piter);
			pt2.begin()->str = str;
			piter->str = "\'";
			pt.cut(piter);
			pt.appendTree(piter, pt2);
			piter = pt.begin();
		}
	}

	*this = CodePred(pt);
}

void CodePred::insert(const string &s, const vector<DrtPred> &d)
{
	Upg upg;
	std::stringstream ss2;
	print_vector_stream(ss2, d);
	Predicate tmp_pred(string("list(" + ss2.str() + ")"));
	upg.add(s, tmp_pred.pred());
	(*this) / upg;
}

void CodePred::insert(const string &s, const vector<FuzzyPred> &d)
{
	Upg upg;
	std::stringstream ss2;
	print_vector_stream(ss2, d);
	Predicate tmp_pred(string("list(" + ss2.str() + ")"));
	upg.add(s, tmp_pred.pred());
	(*this) / upg;
}

void CodePred::insert(const string &s, const Predicate &p)
{
	Upg upg;
	upg.add(s, p.pred());
	(*this) / upg;
}

void CodePred::insert(const string &s, const DrtPred &p)
{
	Upg upg;
	std::stringstream ss;
	ss << p;
	upg.add(s, Predicate(ss.str()).pred());
	(*this) / upg;
}


void CodePred::insert(const string &s, const bool &condition)
{
	Upg upg;
	CodePred p;
	if (condition)
		p = CodePred(Engine::pt_true);
	else
		p = CodePred(Engine::pt_false);
	upg.add(s, p.pred());
	(*this) / upg;
}

void CodePred::insert(const string &s, const vector<KnowledgeAnswer> &condition)
{
	Upg upg;
	CodePred tmp_pred("list");
	for (int n = 0; n < condition.size(); ++n) {
		std::stringstream ss2;
		DrtVect d = condition.at(n).getPreds();
		print_vector_stream(ss2, d);
		Predicate item_pred("item");
		Predicate tmp_preds(string("preds(" + ss2.str() + ")"));
		string text, link;
		text = condition.at(n).getText();
		link = condition.at(n).getLink();
		if (text == "")
			text = "no text";
		if (link == "")
			link = "no link";
		Predicate tmp_text(string("text(" + text + ")"));
		Predicate tmp_link(string("link(" + link + ")"));
		PredTree pt_item = item_pred.pred();
		PredTree pt = tmp_pred.pred();
		pt_item.appendTree(pt_item.begin(), tmp_preds.pred());
		pt_item.appendTree(pt_item.begin(), tmp_text.pred());
		pt_item.appendTree(pt_item.begin(), tmp_link.pred());
		pt.appendTree(pt.begin(), pt_item);
		tmp_pred.pred() = pt;
	}

	upg.add(s, tmp_pred.pred());
	(*this) / upg;
}

bool CodePred::isTrue()
{
	if (this->pred() == Engine::pt_true)
		return true;
	return false;
}

bool CodePred::isFalse()
{
	if (this->pred() == Engine::pt_false)
		return true;
	return false;
}

bool CodePred::isNil()
{
	if (this->pred() == Engine::pt_nil)
		return true;
	return false;
}

bool CodePred::isBreak()
{
	if (this->pred() == Engine::pt_break)
		return true;
	return false;
}

Engine::Engine(Knowledge *k) :
		k_(k)
{
	vector<shared_ptr<Instruction> > default_instructions_;
	default_instructions_.push_back(shared_ptr<Instruction>(new IsHypernym));
	default_instructions_.push_back(shared_ptr<Instruction>(new Print));
	default_instructions_.push_back(shared_ptr<Instruction>(new Repeat));
	default_instructions_.push_back(shared_ptr<Instruction>(new If));
	default_instructions_.push_back(shared_ptr<Instruction>(new IfElse));
	default_instructions_.push_back(shared_ptr<Instruction>(new FindStr));
	default_instructions_.push_back(shared_ptr<Instruction>(new FindStrHeight));
	default_instructions_.push_back(shared_ptr<Instruction>(new Find));
	default_instructions_.push_back(shared_ptr<Instruction>(new GetNamesFromRef));
	default_instructions_.push_back(shared_ptr<Instruction>(new And));
	default_instructions_.push_back(shared_ptr<Instruction>(new Or));
	default_instructions_.push_back(shared_ptr<Instruction>(new Not));
	default_instructions_.push_back(shared_ptr<Instruction>(new ForEach));
	default_instructions_.push_back(shared_ptr<Instruction>(new ForEachTree));
	default_instructions_.push_back(shared_ptr<Instruction>(new ForEachTreeHeight));
	default_instructions_.push_back(shared_ptr<Instruction>(new Set));
	default_instructions_.push_back(shared_ptr<Instruction>(new NumChildren));
	default_instructions_.push_back(shared_ptr<Instruction>(new ConnectedToVerbs));
	default_instructions_.push_back(shared_ptr<Instruction>(new NotConnectedToVerbs));
	default_instructions_.push_back(shared_ptr<Instruction>(new Equal));
	default_instructions_.push_back(shared_ptr<Instruction>(new LessThan));
	default_instructions_.push_back(shared_ptr<Instruction>(new GreaterThan));
	default_instructions_.push_back(shared_ptr<Instruction>(new AddList));
	default_instructions_.push_back(shared_ptr<Instruction>(new ExtractFirstTag));
	default_instructions_.push_back(shared_ptr<Instruction>(new ExtractSecondTag));
	default_instructions_.push_back(shared_ptr<Instruction>(new ExtractSubject));
	default_instructions_.push_back(shared_ptr<Instruction>(new ExtractPosition));
	default_instructions_.push_back(shared_ptr<Instruction>(new SetAt));
	default_instructions_.push_back(shared_ptr<Instruction>(new InsertAt));
	default_instructions_.push_back(shared_ptr<Instruction>(new ReplaceAt));
	default_instructions_.push_back(shared_ptr<Instruction>(new Join));
	default_instructions_.push_back(shared_ptr<Instruction>(new Ask));
	default_instructions_.push_back(shared_ptr<Instruction>(new Quote));
	default_instructions_.push_back(shared_ptr<Instruction>(new FindSub));
	default_instructions_.push_back(shared_ptr<Instruction>(new FindStrSub));
	default_instructions_.push_back(shared_ptr<Instruction>(new FirstChild));
	default_instructions_.push_back(shared_ptr<Instruction>(new HasSubject));
	default_instructions_.push_back(shared_ptr<Instruction>(new HasObject));
	default_instructions_.push_back(shared_ptr<Instruction>(new HasNext));
	default_instructions_.push_back(shared_ptr<Instruction>(new While));
	default_instructions_.push_back(shared_ptr<Instruction>(new IsCandidateVerb));
	default_instructions_.push_back(shared_ptr<Instruction>(new IsCandidateNoun));
	default_instructions_.push_back(shared_ptr<Instruction>(new IsCandidateAdjective));
	default_instructions_.push_back(shared_ptr<Instruction>(new IsCandidateAdverb));
	default_instructions_.push_back(shared_ptr<Instruction>(new Parent));
	default_instructions_.push_back(shared_ptr<Instruction>(new Next));
	//default_instructions_.push_back(shared_ptr<Instruction>(new Prior));
	default_instructions_.push_back(shared_ptr<Instruction>(new FindStrInTree));
	default_instructions_.push_back(shared_ptr<Instruction>(new Plus));
	default_instructions_.push_back(shared_ptr<Instruction>(new Minus));
	default_instructions_.push_back(shared_ptr<Instruction>(new IsVerbLevin));
	default_instructions_.push_back(shared_ptr<Instruction>(new Defun));
	default_instructions_.push_back(shared_ptr<Instruction>(new FindStrPos));
	default_instructions_.push_back(shared_ptr<Instruction>(new SubString));
	default_instructions_.push_back(shared_ptr<Instruction>(new LastChild));
	default_instructions_.push_back(shared_ptr<Instruction>(new MatchAndSubstitute));
	default_instructions_.push_back(shared_ptr<Instruction>(new GetLemma));
	default_instructions_.push_back(shared_ptr<Instruction>(new RemoveAt));

	for (int n = 0; n < default_instructions_.size(); ++n) {
		this->bind(default_instructions_.at(n));
	}
}

Engine::~Engine()
{
	// for (int n = 0; n < default_instructions_.size(); ++n) {
	//      delete default_instructions_.at(n);
	// }
}

template<class T>
vector<T> Engine::getList(const string &list_str)
{
	vector<T> to_return;
	map<string, shared_ptr<Instruction> >::iterator miter = wake_up_strs_.find(list_str);
	if (miter != wake_up_strs_.end()) {
		CodePred code(list_str);
		CodePred result = this->run(code);
		PredTree ptree = result.pred();
		PredTree::children_iterator pi = ptree.begin();
		for (; pi != ptree.end(); ++pi) {
			stringstream ss;
			ss << CodePred(pi);
			T tmp_pred(ss.str());
			to_return.push_back(tmp_pred);
		}
		return to_return;
	}
	return to_return;
}

template vector<FuzzyPred> Engine::getList<FuzzyPred>(const string &s);
template vector<DrtPred> Engine::getList<DrtPred>(const string &s);

template<> vector<int> Engine::getList<int>(const string &list_str)
{
	vector<int> to_return;
	map<string, shared_ptr<Instruction> >::iterator miter = wake_up_strs_.find(list_str);
	if (miter != wake_up_strs_.end()) {
		CodePred code(list_str);
		CodePred result = this->run(code);
		PredTree ptree = result.pred();
		PredTree::children_iterator pi = ptree.begin();
		for (; pi != ptree.end(); ++pi) {
			stringstream ss;
			ss << CodePred(pi);
			int pos = -1;
			try {
				pos = boost::lexical_cast<int>(ss.str());
			} catch (std::exception &e) {
			}
			to_return.push_back(pos);
		}
		return to_return;
	} else
		throw(std::runtime_error(string("Engine: The list with string ") + list_str + " has not been found."));
}

template<class T>
T Engine::getElement(const string &list_str)
{
	map<string, shared_ptr<Instruction> >::iterator miter = wake_up_strs_.find(list_str);
	if (miter != wake_up_strs_.end()) {
		CodePred code(list_str);
		CodePred result = this->run(code);
		return result;
	} else
		throw(std::runtime_error(string("Engine: The list with string ") + list_str + " has not been found."));
	return FuzzyPred(Engine::pt_false);
}

template FuzzyPred Engine::getElement<FuzzyPred>(const string &s);

template<> vector<KnowledgeAnswer> Engine::getList<KnowledgeAnswer>(const string &list_str)
{
	vector<KnowledgeAnswer> to_return;
	map<string, shared_ptr<Instruction> >::iterator miter = wake_up_strs_.find(list_str);
	if (miter != wake_up_strs_.end()) {
		CodePred code(list_str);
		CodePred result = this->run(code);
		PredTree ptree = result.pred();
		PredTree::children_iterator sitem = ptree.begin();
		vector<KnowledgeAnswer> kav;
		for (; sitem != ptree.end(); ++sitem) {
			KnowledgeAnswer ka;
			PredTree ptree_item = PredTree(sitem);
			PredTree::children_iterator si = ptree_item.begin();
			bool to_insert = false;
			for (; si != ptree.end(); ++si) {
				if (si->str == "preds") {
					PredTree ptree2 = PredTree(si);
					PredTree::children_iterator si2 = ptree2.begin();
					std::stringstream ss;
					for (; si2 != ptree2.end(); ++si2) {
						ss << Predicate(PredTree(si2));
						if (si2.nextSibling() != ptree.end())
							ss << ",";
					}
					DrtVect orig_drt = create_drtvect(ss.str());
					ka.setPreds(orig_drt);
				} else if (si->str == "text") {
					to_insert = true;
					PredTree ptree2 = PredTree(si);
					string text = ptree2.begin().firstChild()->str;
					ka.setText(text);
				} else if (si->str == "link") {
					PredTree ptree2 = PredTree(si);
					string link = ptree2.begin().firstChild()->str;
					ka.setLink(link);
				}
			}
			if (to_insert) {
				ka.setWeigth(0.5); ///
				kav.push_back(ka);
			}
		}
		return kav;
	} else
		throw(std::runtime_error(string("Engine: The list with string ") + list_str + " has not been found."));

}

void Engine::operator >>(stringstream &ss)
{
	ss << cout_engine_;
}

template<class T>
void Engine::operator <<(T &rhs)
{
	stringstream ss;
	ss << rhs;
	cout_engine_ += ss.str();
}

void Engine::bind(shared_ptr<Instruction> instruction)
{
	string wu_str = instruction->getWakeUpString();

	map<string, shared_ptr<Instruction> >::iterator miter = wake_up_strs_.find(wu_str);
	if (miter == wake_up_strs_.end()) {
		wake_up_strs_[wu_str] = instruction;
	} else
		throw(std::runtime_error(string("Engine: The wake up string ") + wu_str + " is already bound to an instruction."));
}

void Engine::bindAnyway(shared_ptr<Instruction> instruction)
// Bind the new instruction even if the wake up string already exists
{
	string wu_str = instruction->getWakeUpString();
	wake_up_strs_[wu_str] = instruction;
}

static PredTree::iterator find_proper_element(const PredTree &pt, const vector<string> &strs)
{
	int n = 1;
	bool found_element;
	for (; n <= pt.height() && n < strs.size(); ++n) {
		PredTree::depth_iterator di(pt, n - 1);
		found_element = false;
		for (; di != pt.end(); ++di) {
			string di_str = di->str;
			if (di_str == strs.at(n)) {
				if (n == strs.size() - 1)
					return di;
				found_element = true;
				break;
			}
		}
		if (!found_element)
			return pt.begin();
	}
	return pt.begin();
}

CodePred Engine::run(CodePred code)
{
	PredTree pt = code.pred();
	PredTree::iterator pi = pt.begin();

	string pstr = pi->str;
	vector<string> strs;
	bool has_period = false;
	// Check if there is a '.'
	map<string, shared_ptr<Instruction> >::iterator miter = wake_up_strs_.find(pstr);
	if (miter != wake_up_strs_.end()) {
		PredTree subtree(pi);
		shared_ptr<Instruction> instruction = miter->second;

		clock_t start;
		if (debug )
			start = clock();
		subtree = instruction->transform(subtree, this);

		pt = subtree;
		if (has_period && strs.size() > 1 && strs.at(0) == pt.begin()->str)
			strs.insert(strs.begin(), strs.at(0));
	}

	return CodePred(pt);
}

string Engine::IsHypernym::getWakeUpString()
{
	return "is-hypernym";
}

PredTree Engine::IsHypernym::transform(PredTree pt, Engine *e)
{
	PredTree::iterator pi = pt.begin();
	if (pi.num_children() != 2)
		return Engine::pt_false;
	string fstr = pi.firstChild()->str;
	string sstr = pi.lastChild()->str;

	metric *d = metric_singleton::get_metric_instance();

	if (fstr == "" && sstr == "")
		return Engine::pt_false;
	clock_t start;
	if (debug )
		start = clock();
	double dist = d->hypernym_dist(fstr, sstr, 6);
	if (dist > 0.2)
		return Engine::pt_true;
	return Engine::pt_false;
}

string Engine::IsVerbLevin::getWakeUpString()
{
	return "is-verb-levin";
}

PredTree Engine::IsVerbLevin::transform(PredTree pt, Engine *e)
{
	PredTree::iterator pi = pt.begin();
	if (pi.num_children() != 2)
		return Engine::pt_false;
	string fstr = pi.firstChild()->str;
	string sstr = pi.lastChild()->str;
	string verb_str = fstr.substr(0, fstr.find("/"));
	metric *d = metric_singleton::get_metric_instance();
	string vlevin = d->get_levin_verb(verb_str);
	if (vlevin == sstr)
		return Engine::pt_true;
	return Engine::pt_false;
}

string Engine::Print::getWakeUpString()
{
	return "print";
}

PredTree Engine::Print::transform(PredTree pt, Engine *e)
{
	PredTree::iterator pi = pt.begin();
	if (pi.num_children() != 1)
		return Engine::pt_false;
	string fstr = pi.firstChild()->str;
	CodePred result = e->run(CodePred(pi.lastChild()));
	(*e) << result;
	(*e) << "\n";
	return Engine::pt_true;
}

string Engine::Repeat::getWakeUpString()
{
	return "repeat";
}

PredTree Engine::Repeat::transform(PredTree pt, Engine *e)
{
	PredTree::iterator pi = pt.begin();
	if (pi.num_children() != 2)
		return Engine::pt_false;
	string fstr = pi.firstChild()->str;
	int num;
	try {
		num = boost::lexical_cast<int>(fstr);
	} catch (std::exception &e) {
		return Engine::pt_false;
	}
	for (int n = 0; n < num; ++n)
		e->run(CodePred(pi.lastChild()));

	return Engine::pt_true;
}

string Engine::If::getWakeUpString()
{
	return "if";
}

PredTree Engine::If::transform(PredTree pt, Engine *e)
{
	PredTree::iterator pi = pt.begin();
	if (pi.num_children() != 2)
		return Engine::pt_false;
	CodePred result = e->run(CodePred(pi.firstChild()));
	if (result.pred() == Engine::pt_true) {
		result = e->run(CodePred(pi.lastChild()));
		PredTree subtree = result.pred();
		pt.replace(subtree, pi.lastChild());
		return result.pred();
	} else
		return Engine::pt_false;
}

string Engine::IfElse::getWakeUpString()
{
	return "if-else";
}

PredTree Engine::IfElse::transform(PredTree pt, Engine *e)
{
	PredTree::iterator pi = pt.begin();
	if (pi.num_children() != 3)
		return Engine::pt_false;
	CodePred result = e->run(CodePred(pi.firstChild()));
	if (result.pred() == Engine::pt_true) {
		result = e->run(CodePred(pi.firstChild().nextSibling()));
		PredTree subtree = result.pred();
		pt.replace(subtree, pi.firstChild().nextSibling());
		return result.pred();
	} else {
		result = e->run(CodePred(pi.lastChild()));
		PredTree subtree = result.pred();
		pt.replace(subtree, pi.lastChild());
		return result.pred();
	}
}

string Engine::FindStr::getWakeUpString()
{
	return "find-str";
}

PredTree Engine::FindStr::transform(PredTree pt, Engine *e)
{
	PredTree::iterator pi = pt.begin();
	if (pi.num_children() != 2)
		return Engine::pt_false;
	CodePred result = e->run(CodePred(pi.firstChild()));
	string fstr = result.pred().begin()->str;
	string sstr = pi.lastChild()->str;
	if (fstr.find(sstr) != string::npos)
		return Engine::pt_true;
	return Engine::pt_false;
}

string Engine::FindStrPos::getWakeUpString()
{
	return "find-str-pos";
}

PredTree Engine::FindStrPos::transform(PredTree pt, Engine *e)
{
	PredTree::iterator pi = pt.begin();
	if (pi.num_children() != 2)
		return Engine::pt_false;
	CodePred result = e->run(CodePred(pi.firstChild()));
	string fstr = result.pred().begin()->str;
	string sstr = pi.lastChild()->str;
	int pos = fstr.find(sstr);
	if (pos != string::npos)
		return boost::lexical_cast<string>(pos);
	return boost::lexical_cast<string>(-1);
}

string Engine::FindStrInTree::getWakeUpString()
{
	return "find-str-in-tree";
}

PredTree Engine::FindStrInTree::transform(PredTree pt, Engine *e)
{
	PredTree::iterator pi = pt.begin();
	if (pi.num_children() != 2)
		return Engine::pt_false;
	CodePred fresult = e->run(CodePred(pi.firstChild()));
	PredTree sx_tree = fresult.pred();
	CodePred sresult = e->run(CodePred(pi.lastChild()));
	PredTree dx_tree = sresult.pred();

	PredTree::iterator si = sx_tree.begin();
	for (; si != sx_tree.end(); ++si) {
		if (si->str.find(dx_tree.begin()->str) != string::npos)
			return Engine::pt_true;
	}
	return Engine::pt_false;
}

string Engine::FindStrSub::getWakeUpString()
{
	return "find-str-sub";
}

PredTree Engine::FindStrSub::transform(PredTree pt, Engine *e)
{
	PredTree::iterator pi = pt.begin();
	if (pi.num_children() != 2)
		return Engine::pt_false;
	CodePred fresult = e->run(CodePred(pi.firstChild()));
	PredTree sx_tree = fresult.pred();
	CodePred sresult = e->run(CodePred(pi.lastChild()));
	PredTree dx_tree = sresult.pred();

	PredTree::iterator si(sx_tree.begin());
	++si;
	for (; si != sx_tree.end(); ++si) {
		if (si->str == dx_tree.begin()->str)
			return Engine::pt_true;
	}
	return Engine::pt_false;
}

string Engine::FindStrHeight::getWakeUpString()
{
	return "find-str-height";
}

PredTree Engine::FindStrHeight::transform(PredTree pt, Engine *e)
{
	PredTree::iterator pi = pt.begin();
	if (pi.num_children() != 3)
		return Engine::pt_false;
	PredTree ptree(pi.firstChild());
	string sstr = pi.firstChild().nextSibling()->str;
	int height = 0;
	try {
		height = boost::lexical_cast<int>(sstr);
	} catch (std::exception &e) {
	}
	string tstr = pi.lastChild()->str;
	PredTree::height_iterator hi(ptree, height);
	for (; hi != pt.end(); ++hi) {
		if (hi->str.find(tstr) != string::npos)
			return Engine::pt_true;
	}
	return Engine::pt_false;
}

string Engine::Find::getWakeUpString()
{
	return "find";
}

PredTree Engine::Find::transform(PredTree pt, Engine *e)
// parse all the element of a tree searching for a specific tree
{
	PredTree::iterator pi = pt.begin();
	if (pi.num_children() != 2)
		return Engine::pt_false;
	CodePred fresult = e->run(CodePred(pi.firstChild()));
	PredTree sx_tree = fresult.pred();
	CodePred sresult = e->run(CodePred(pi.lastChild()));
	PredTree dx_tree = sresult.pred();

	PredTree::iterator si(sx_tree.begin());
	for (; si != sx_tree.end(); ++si) {
		if (PredTree(si) == dx_tree)
			return Engine::pt_true;
	}
	return Engine::pt_false;
}

string Engine::FindSub::getWakeUpString()
{
	return "find-sub";
}

PredTree Engine::FindSub::transform(PredTree pt, Engine *e)
// parse all the element of a tree (except the root element) searching for a specific tree
{
	PredTree::iterator pi = pt.begin();
	if (pi.num_children() != 2)
		return Engine::pt_false;
	CodePred fresult = e->run(CodePred(pi.firstChild()));
	PredTree sx_tree = fresult.pred();
	CodePred sresult = e->run(CodePred(pi.lastChild()));
	PredTree dx_tree = sresult.pred();

	PredTree::iterator si(sx_tree.begin());
	++si;
	for (; si != sx_tree.end(); ++si) {
		if (PredTree(si) == dx_tree)
			return Engine::pt_true;
	}
	return Engine::pt_false;
}

string Engine::GetNamesFromRef::getWakeUpString()
{
	return "get-names-from-ref";
}

PredTree Engine::GetNamesFromRef::transform(PredTree pt, Engine *e)
{
	PredTree::iterator pi = pt.begin();
	if (pi.num_children() != 1)
		return CodePred("").pred();
	string fstr = pi.firstChild()->str;
	Knowledge *k = e->getKnowledge();
	string ret_str = "";
	vector<string> names,tempnames;
	names = k->getNamesFromRef(fstr);
	tempnames = k->getTempNamesFromRef(fstr);

	names.insert(names.end(), tempnames.begin(), tempnames.end());
	int size = names.size();
	for (int n = 0; n < size; ++n) {
		if (names.at(n).find("[") != string::npos)
			continue;
		ret_str += names.at(n);
		if (n != size - 1)
			ret_str += "|";
	}

	return CodePred(ret_str).pred();
}

string Engine::And::getWakeUpString()
{
	return "and";
}

PredTree Engine::And::transform(PredTree pt, Engine *e)
{
	PredTree::iterator pi = pt.begin();
	PredTree::children_iterator si(pi);
	for (; si != pt.end(); ++si) {
		CodePred fresult = e->run(CodePred(si));
		if (!(fresult.pred() == Engine::pt_true))
			return Engine::pt_false;
	}
	return Engine::pt_true;
}

string Engine::Not::getWakeUpString()
{
	return "not";
}

PredTree Engine::Not::transform(PredTree pt, Engine *e)
{
	PredTree::iterator pi = pt.begin();
	if (pi.num_children() != 1)
		return Engine::pt_false;
	CodePred fresult = e->run(CodePred(pi.firstChild()));
	if (fresult.pred() == Engine::pt_true) {
		return Engine::pt_false;
	}
	return Engine::pt_true;
}

string Engine::Join::getWakeUpString()
{
	return "join";
}

PredTree Engine::Join::transform(PredTree pt, Engine *e)
{
	PredTree::iterator pi = pt.begin();
	PredTree::children_iterator si(pi);
	CodePred result;
	for (; si != pt.end(); ++si) {
		result = e->run(CodePred(si));
	}
	return result.pred();
}

string Engine::Or::getWakeUpString()
{
	return "or";
}

PredTree Engine::Or::transform(PredTree pt, Engine *e)
{
	PredTree::iterator pi = pt.begin();
	PredTree::children_iterator si(pi);
	for (; si != pt.end(); ++si) {
		CodePred fresult = e->run(CodePred(si));
		if (fresult.pred() == Engine::pt_true)
			return Engine::pt_true;
	}
	return Engine::pt_false;
}

string Engine::ForEach::getWakeUpString()
{
	return "for-each";
}

PredTree Engine::ForEach::transform(PredTree pt, Engine *e)
{
	CodePred result;
	PredTree::iterator pi = pt.begin();
	if (pi.num_children() != 3)
		return Engine::pt_false;
	string fstr = pi.firstChild()->str;
	PredTree list_pt(pi.firstChild().nextSibling());
	bool list_pred_trigger = false;
	string sstr;
	result = e->run(list_pt);
	list_pt = result.pred();
	if (list_pt.begin().height() == 0) {
		sstr = list_pt.begin()->str;
		if (debug)
			cout << sstr << endl;
	} else {
		list_pred_trigger = true;
		sstr = result.pred().begin()->str;
	}
	vector<PredTree> preds;
	int size;
	if (sstr == "nil")
		return Engine::pt_false;
	if (list_pred_trigger) {
		PredTree::children_iterator si = list_pt.begin();
		for (; si != list_pt.end(); ++si) {
			preds.push_back(Predicate(si).pred());
		}
		size = list_pt.begin().num_children();
	} else {
		vector<string> names;
		boost::split(names, sstr, boost::is_any_of("|"));
		size = names.size();
		for (int n = 0; n < size; ++n) {
			preds.push_back(Predicate(names.at(n)).pred());
		}
	}
	for (int n = 0; n < size; ++n) {
		Upg upg;
		upg.add(fstr, preds.at(n));
		CodePred subpred = CodePred(pi.lastChild());
		PredTree spt = subpred.pred();
		if (spt.findData(PTEl("for-each"), spt.begin()) // no nested counters !!
		== spt.end()) {
			upg.add(PTEl("_counter"), boost::lexical_cast<string>(n));
		}

		if (n < size - 1)
			upg.add(PTEl((string) "_next[" + fstr + "]"), preds.at(n + 1));
		else
			upg.add(PTEl((string) "_next[" + fstr + "]"), Engine::pt_nil);
		if (n > 0)
			upg.add(PTEl((string) "_prior[" + fstr + "]"), preds.at(n - 1));
		else
			upg.add(PTEl((string) "_prior[" + fstr + "]"), Engine::pt_nil);

		subpred / upg;
		result = e->run(subpred);
		if (result.pred() == Engine::pt_break) {
			result = CodePred(Engine::pt_true);
			break;
		}
	}
	return result.pred();
}

string Engine::While::getWakeUpString()
{
	return "while";
}

PredTree Engine::While::transform(PredTree pt, Engine *e)
{
	CodePred result;
	PredTree::iterator pi = pt.begin();
	if (pi.num_children() != 2)
		return Engine::pt_false;
	PredTree first_pt(pi.firstChild());
	PredTree second_pt(pi.lastChild());

	CodePred result_first, result_second;
	do {
		result_second = e->run(second_pt);
		result_first = e->run(first_pt);
	} while (!(result_first.pred() == Engine::pt_false));

	return result_second.pred();
}

string Engine::ForEachTreeHeight::getWakeUpString()
{
	return "for-each-tree-height";
}

PredTree Engine::ForEachTreeHeight::transform(PredTree pt, Engine *e)
{
	PredTree::iterator pi = pt.begin();
	if (pi.num_children() != 4)
		return Engine::pt_false;
	string fstr = pi.firstChild()->str;
	string sstr = pi.firstChild().nextSibling()->str;
	PredTree list_pt(pi.firstChild().nextSibling().nextSibling());
	int height = 0;
	PredTree::height_iterator hi;
	try {
		height = boost::lexical_cast<int>(sstr);
		hi = PredTree::height_iterator(list_pt, height);
	} catch (std::exception &e) {
		if (debug)
			std::cerr << e.what() << std::flush << endl;
	}
	CodePred result;
	for (int n = 0; hi != list_pt.end(); ++hi, ++n) {
		CodePred cp(hi);
		Upg upg;
		upg.add(fstr, cp.pred());
		upg.add(PTEl("_counter"), boost::lexical_cast<string>(n));
		if (hi.parent() != list_pt.end())
			upg.add(PTEl((string) "_parent[" + fstr + "]"), PredTree(hi.parent()));
		else
			upg.add(PTEl((string) "_parent[" + fstr + "]"), Engine::pt_false);
		CodePred subpred = CodePred(pi.lastChild());
		subpred / upg;
		result = e->run(subpred);
	}
	return result.pred();
}

string Engine::ForEachTree::getWakeUpString()
{
	return "for-each-tree";
}

PredTree Engine::ForEachTree::transform(PredTree pt, Engine *e)
// Parse each subtree and apply an action to the subtree elements
{
	CodePred result;
	PredTree::iterator pi = pt.begin();
	if (pi.num_children() != 3)
		return Engine::pt_false;
	string fstr = pi.firstChild()->str;
	CodePred result_second = e->run(CodePred(pi.firstChild().nextSibling()));
	PredTree list_pt(result_second.pred());
	PredTree::iterator si = list_pt.begin();
	for (; si != list_pt.end(); ++si) {
		PredTree pred(si);
		Upg upg;
		upg.add(fstr, pred);
		if (si.parent() != list_pt.end())
			upg.add(PTEl((string) "_parent[" + fstr + "]"), PredTree(si.parent()));
		else
			upg.add(PTEl((string) "_parent[" + fstr + "]"), Engine::pt_false);
		CodePred subpred = CodePred(pi.lastChild());
		subpred / upg;
		result = e->run(subpred);
		if (result.pred() == Engine::pt_break) {
			result = CodePred(Engine::pt_true);
			break;
		}
	}
	return result.pred();
}

string Engine::Defun::getWakeUpString()
{
	return "defun";
}
PredTree Engine::Defun::transform(PredTree pt, Engine *e)
{
	PredTree::iterator pi = pt.begin();
	if (pi.num_children() == 3) {
		string fstr = pi.firstChild()->str;
		vector<string> variables;
		PredTree::children_iterator si(pi.firstChild());
		PredTree::iterator send = pt.end();
		for (; si != send; ++si) {
			variables.push_back(si->str);
		}
		PredTree sx_tree = Predicate(pi.firstChild().nextSibling()).pred();
		CodePred result = e->run(CodePred(sx_tree));
		sx_tree = result.pred();
		shared_ptr<TemplateInstruction> ti(new TemplateInstruction(fstr, sx_tree, variables));
		e->bindAnyway(ti);

		CodePred subpred = CodePred(pi.lastChild());
		result = e->run(subpred);
		return result.pred();
	} else if (pi.num_children() == 2) {
		string fstr = pi.firstChild()->str;
		vector<string> variables;
		PredTree::children_iterator si(pi.firstChild());
		PredTree::iterator send = pt.end();
		for (; si != send; ++si) {
			variables.push_back(si->str);
		}

		PredTree sx_tree = Predicate(pi.firstChild().nextSibling()).pred();
		CodePred result = e->run(CodePred(sx_tree));
		sx_tree = result.pred();

		shared_ptr<TemplateInstruction> ti(new TemplateInstruction(fstr, sx_tree, variables));
		e->bindAnyway(ti);

		return Engine::pt_true;
	}
	return Engine::pt_false;
}

string Engine::Set::getWakeUpString()
{
	return "set";
}

PredTree Engine::Set::transform(PredTree pt, Engine *e)
{
	PredTree::iterator pi = pt.begin();
	if (pi.num_children() == 3) {
		string fstr = pi.firstChild()->str;
		vector<string> variables;
		PredTree sx_tree = Predicate(pi.firstChild().nextSibling()).pred();
		CodePred result = e->run(CodePred(sx_tree));
		sx_tree = result.pred();
		shared_ptr<TemplateInstruction> ti(new TemplateInstruction(fstr, sx_tree, variables));
		e->bindAnyway(ti);

		CodePred subpred = CodePred(pi.lastChild());
		result = e->run(subpred);
		return result.pred();
	} else if (pi.num_children() == 2) {
		string fstr = pi.firstChild()->str;
		vector<string> variables;
		PredTree sx_tree = Predicate(pi.firstChild().nextSibling()).pred();
		CodePred result = e->run(CodePred(sx_tree));
		sx_tree = result.pred();

		shared_ptr<TemplateInstruction> ti(new TemplateInstruction(fstr, sx_tree, variables));
		e->bindAnyway(ti);

		return Engine::pt_true;
	}
	return Engine::pt_false;
}

string Engine::Break::getWakeUpString()
{
	return "break";
}

PredTree Engine::Break::transform(PredTree pt, Engine *e)
{
	return CodePred("break").pred();
}

string Engine::NumChildren::getWakeUpString()
{
	return "num-children";
}

PredTree Engine::NumChildren::transform(PredTree pt, Engine *e)
{
	PredTree::iterator pi = pt.begin();
	if (pi.num_children() != 1)
		return Engine::pt_false;
	CodePred result = e->run(CodePred(pi.firstChild()));
	PredTree pt2 = result.pred();
	pi = pt2.begin();
	string fstr = pi->str;
//     if(fstr != "list")
//	  return Engine::pt_nil;
	int num_children = pi.num_children();
	CodePred to_return(boost::lexical_cast<string>(num_children));
	return to_return.pred();
}

string Engine::ConnectedToVerbs::getWakeUpString()
{
	return "connected-to-verbs";
}

PredTree Engine::ConnectedToVerbs::transform(PredTree pt, Engine *e)
{
	PredTree::iterator pi = pt.begin();
	if (pi.num_children() != 1)
		return Engine::pt_false;
	string fstr = pi.firstChild()->str;
	int num_children = pi.firstChild().num_children();
	if (num_children == 0)
		return Engine::pt_nil;

	std::stringstream ss;
	PredTree::children_iterator si = pi.firstChild();
	for (; si != pt.end(); ++si) {
		ss << Predicate(PredTree(si));
		if (si.nextSibling() != pt.end())
			ss << ",";
	}
	DrtVect orig_drt = create_drtvect(ss.str());
	vector<DrtVect> all_drts = get_linked_drtvect_from_single_drtvect(orig_drt);

	string ret_str = "list(";
	for (int n = 0; n < all_drts.size(); ++n) {
		std::stringstream ss2;
		ss2 << "list(";
		print_vector_stream(ss2, all_drts.at(n));
		ss2 << ")";
		ret_str += ss2.str();
		if (n != all_drts.size() - 1)
			ret_str += ",";
	}
	ret_str += ")";
	CodePred to_return(ret_str);
	return to_return.pred();
}

string Engine::NotConnectedToVerbs::getWakeUpString()
{
	return "not-connected-to-verbs";
}

static vector<DrtPred> get_not_connected(const DrtVect &orig_drt, const vector<DrtVect> &connected)
{
	DrtVect not_connected, connected_list;

	for (int m = 0; m < connected.size(); ++m) {
		connected_list.insert(connected_list.end(), connected.at(m).begin(), connected.at(m).end());
	}
	for (int n = 0; n < orig_drt.size(); ++n) {
		if (!shortfind(connected_list, orig_drt.at(n)) && !orig_drt.at(n).is_complement()) {
			not_connected.push_back(orig_drt.at(n));
		}
	}

	return not_connected;
}

PredTree Engine::NotConnectedToVerbs::transform(PredTree pt, Engine *e)
{
	PredTree::iterator pi = pt.begin();
	if (pi.num_children() != 1)
		return Engine::pt_false;
	string fstr = pi.firstChild()->str;
	int num_children = pi.firstChild().num_children();
	if (num_children == 0)
		return Engine::pt_nil;

	std::stringstream ss;
	PredTree::children_iterator si = pi.firstChild();
	for (; si != pt.end(); ++si) {
		ss << Predicate(PredTree(si));
		if (si.nextSibling() != pt.end())
			ss << ",";
	}

	DrtVect orig_drt = create_drtvect(ss.str());
	vector<DrtVect> connected = get_linked_drtvect_from_single_drtvect(orig_drt);
	vector<DrtPred> not_connected = get_not_connected(orig_drt, connected);

	if (not_connected.size() == 0)
		return Engine::pt_nil;

	string ret_str = "list(";
	std::stringstream ss2;
	print_vector_stream(ss2, not_connected);
	ret_str += ss2.str();
	ret_str += ")";

	CodePred to_return(ret_str);
	return to_return.pred();
}

string Engine::Equal::getWakeUpString()
{
	return "equal";
}

PredTree Engine::Equal::transform(PredTree pt, Engine *e)
{
	PredTree::iterator pi = pt.begin();
	if (pi.num_children() != 2)
		return Engine::pt_false;
	CodePred result1 = e->run(CodePred(pi.firstChild()));
	CodePred result2 = e->run(CodePred(pi.lastChild()));
	if (result1.pred() != Engine::pt_nil && result2.pred() != Engine::pt_nil && result1 == result2) {
		return Engine::pt_true;
	} else
		return Engine::pt_false;
}

string Engine::LessThan::getWakeUpString()
{
	return "less-than";
}

PredTree Engine::LessThan::transform(PredTree pt, Engine *e)
{
	PredTree::iterator pi = pt.begin();
	if (pi.num_children() != 2)
		return Engine::pt_false;
	CodePred result1 = e->run(CodePred(pi.firstChild()));
	CodePred result2 = e->run(CodePred(pi.lastChild()));

	try {
		double w1 = boost::lexical_cast<double>(result1.pred().begin()->str);
		double w2 = boost::lexical_cast<double>(result2.pred().begin()->str);

		if (w1 < w2) {
			return Engine::pt_true;
		} else
			return Engine::pt_false;
	} catch (std::exception &e) {
		return Engine::pt_false;
	}
}

string Engine::GreaterThan::getWakeUpString()
{
	return "greater-than";
}

PredTree Engine::GreaterThan::transform(PredTree pt, Engine *e)
{
	PredTree::iterator pi = pt.begin();
	if (pi.num_children() != 2)
		return Engine::pt_false;
	CodePred result1 = e->run(CodePred(pi.firstChild()));
	CodePred result2 = e->run(CodePred(pi.lastChild()));

	try {
		double w1 = boost::lexical_cast<double>(result1.pred().begin()->str);
		double w2 = boost::lexical_cast<double>(result2.pred().begin()->str);

		if (w1 > w2) {
			return Engine::pt_true;
		} else
			return Engine::pt_false;
	} catch (std::exception &e) {
		return Engine::pt_false;
	}
}

string Engine::AddList::getWakeUpString()
{
	return "add-list";
}

PredTree Engine::AddList::transform(PredTree pt, Engine *e)
{
	PredTree::iterator pi = pt.begin();
	if (pi.num_children() != 2)
		return Engine::pt_false;
	CodePred result1 = e->run(CodePred(pi.firstChild()));
	PredTree sx_tree = result1.pred();
	CodePred result2 = e->run(CodePred(pi.lastChild()));
	PredTree dx_tree = result2.pred();

	sx_tree.appendTree(sx_tree.begin(), dx_tree);
	return sx_tree;
}

string Engine::ExtractFirstTag::getWakeUpString()
{
	return "extract-first-tag";
}

PredTree Engine::ExtractFirstTag::transform(PredTree pt, Engine *e)
{
	PredTree::iterator pi = pt.begin();
	if (pi.num_children() != 1)
		return Engine::pt_false;
	CodePred result1 = e->run(CodePred(pi.firstChild()));
	PredTree sx_tree = result1.pred();
	string ret_str = extract_first_tag(Predicate(sx_tree));
	return Predicate(ret_str).pred();
}

string Engine::ExtractSecondTag::getWakeUpString()
{
	return "extract-second-tag";
}

PredTree Engine::ExtractSecondTag::transform(PredTree pt, Engine *e)
{
	PredTree::iterator pi = pt.begin();
	if (pi.num_children() != 1)
		return Engine::pt_false;
	CodePred result1 = e->run(CodePred(pi.firstChild()));
	PredTree sx_tree = result1.pred();
	string ret_str = extract_second_tag(Predicate(sx_tree));
	return Predicate(ret_str).pred();
}

string Engine::ExtractSubject::getWakeUpString()
{
	return "extract-subject";
}

PredTree Engine::ExtractSubject::transform(PredTree pt, Engine *e)
{
	PredTree::iterator pi = pt.begin();
	if (pi.num_children() != 1)
		return Engine::pt_false;
	CodePred result1 = e->run(CodePred(pi.firstChild()));
	PredTree sx_tree = result1.pred();
	string ret_str = extract_subject(Predicate(sx_tree));
	return Predicate(ret_str).pred();
}


string Engine::ExtractPosition::getWakeUpString()
{
	return "extract-position";
}

PredTree Engine::ExtractPosition::transform(PredTree pt, Engine *e)
{
	PredTree::iterator pi = pt.begin();
	if (pi.num_children() != 1)
		return Engine::pt_false;
	CodePred result1 = e->run(CodePred(pi.firstChild()));
	PredTree sx_tree = result1.pred();
	string ref_str = Predicate(sx_tree).pred().begin()->str;
	double pos = get_single_distance(ref_str);
	string pos_str;
	try {
		pos_str = boost::lexical_cast<string>(pos);
	} catch (std::exception &e) {
	}

	return Predicate(pos_str).pred();
}

string Engine::SetAt::getWakeUpString()
{
	return "set-at";
}

PredTree Engine::SetAt::transform(PredTree pt, Engine *e)
{
	PredTree::iterator pi = pt.begin();
	if (pi.num_children() != 3)
		return Engine::pt_false;

	CodePred result1 = e->run(CodePred(pi.firstChild()));
	PredTree sx_tree = result1.pred();
	CodePred result2 = e->run(CodePred(pi.firstChild().nextSibling()));
	PredTree md_tree = result2.pred();
	CodePred result3 = e->run(CodePred(pi.lastChild()));
	PredTree dx_tree = result3.pred();

	int size = sx_tree.begin().num_children();
	int pos = 0;
	try {
		pos = boost::lexical_cast<int>(md_tree.begin()->str);
	} catch (std::exception &e) {
		return Engine::pt_false;
	}
	if (pos >= size)
		return Engine::pt_false;

	vector<CodePred> preds;
	PredTree::children_iterator si(sx_tree);
	int pos_at = 0;
	for (; si != sx_tree.end(); ++si, ++pos_at) {
		if (pos_at == pos)
			si->str = dx_tree.begin()->str;
	}

	return sx_tree;
}

string Engine::InsertAt::getWakeUpString()
{
	return "insert-at";
}

PredTree Engine::InsertAt::transform(PredTree pt, Engine *e)
// insert a tree on the left of an element of a list
{
	PredTree::iterator pi = pt.begin();
	if (pi.num_children() != 3)
		return Engine::pt_false;

	CodePred result1 = e->run(CodePred(pi.firstChild()));
	PredTree sx_tree = result1.pred();
	CodePred result2 = e->run(CodePred(pi.firstChild().nextSibling()));
	PredTree md_tree = result2.pred();
	CodePred result3 = e->run(CodePred(pi.lastChild()));
	PredTree dx_tree = result3.pred();

	int size = sx_tree.begin().num_children();
	int pos = 0;
	try {
		pos = boost::lexical_cast<int>(md_tree.begin()->str);
	} catch (std::exception &e) {
		return Engine::pt_false;
	}
	if (pos >= size)
		return Engine::pt_false;

	vector<CodePred> preds;
	PredTree::children_iterator si(sx_tree);
	int pos_at = 0;
	for (; si != sx_tree.end(); ++si, ++pos_at) {
		if (pos_at == pos - 1) {
			sx_tree.insert(dx_tree, si);
		}
	}

	return sx_tree;
}

string Engine::ReplaceAt::getWakeUpString()
{
	return "replace-at";
}

PredTree Engine::ReplaceAt::transform(PredTree pt, Engine *e)
// insert a tree on the left of an element of a list
{
	PredTree::iterator pi = pt.begin();
	if (pi.num_children() != 3)
		return Engine::pt_false;

	CodePred result1 = e->run(CodePred(pi.firstChild()));
	PredTree sx_tree = result1.pred();
	CodePred result2 = e->run(CodePred(pi.firstChild().nextSibling()));
	PredTree md_tree = result2.pred();
	CodePred result3 = e->run(CodePred(pi.lastChild()));
	PredTree dx_tree = result3.pred();

	int size = sx_tree.begin().num_children();
	int pos = 0;
	try {
		pos = boost::lexical_cast<int>(md_tree.begin()->str);
	} catch (std::exception &e) {
		return Engine::pt_false;
	}
	if (pos >= size)
		return Engine::pt_false;

	vector<CodePred> preds;
	PredTree::children_iterator si(sx_tree);
	int pos_at = 0;
	for (; si != sx_tree.end(); ++si, ++pos_at) {
		if (pos_at == pos) {
			sx_tree.replace(dx_tree, si);
		}
	}

	return sx_tree;
}

string Engine::RemoveAt::getWakeUpString()
{
	return "remove-at";
}

PredTree Engine::RemoveAt::transform(PredTree pt, Engine *e)
// insert a tree on the left of an element of a list
{
	PredTree::iterator pi = pt.begin();
	if (pi.num_children() != 2)
		return Engine::pt_false;

	CodePred result1 = e->run(CodePred(pi.firstChild()));
	PredTree sx_tree = result1.pred();
	CodePred result2 = e->run(CodePred(pi.firstChild().nextSibling()));
	PredTree md_tree = result2.pred();
	CodePred result3 = e->run(CodePred(pi.lastChild()));
	PredTree dx_tree = result3.pred();

	int size = sx_tree.begin().num_children();
	int pos = 0;
	try {
		pos = boost::lexical_cast<int>(md_tree.begin()->str);
	} catch (std::exception &e) {
		return Engine::pt_false;
	}
	if (pos >= size)
		return Engine::pt_false;

	vector<CodePred> preds;
	PredTree::children_iterator si(sx_tree);
	int pos_at = 0;
	for (; si != sx_tree.end(); ++si, ++pos_at) {
		if (pos_at == pos) {
			sx_tree.erase(si);
			break;
		}
	}
	return sx_tree;
}



string Engine::IsCandidateVerb::getWakeUpString()
{
	return "is-candidate-verb";
}

PredTree Engine::IsCandidateVerb::transform(PredTree pt, Engine *e)
{
	PredTree::iterator pi = pt.begin();
	if (pi.num_children() != 2)
		return Engine::pt_false;
	CodePred result = e->run(CodePred(pi.firstChild()));
	PredTree sx_tree = result.pred();
	string noun_str = sx_tree.begin()->str;
	CodePred result3 = e->run(CodePred(pi.lastChild()));
	PredTree dx_tree = result3.pred();
	string tag_str = dx_tree.begin()->str;
	noun_str = noun_str.substr(0, noun_str.find("/"));
	tagger *t = parser_singleton::get_tagger_instance();
	tagger_info *info = t->get_info();
	string base = info->get_conj(noun_str, tag_str);
	if (base == "" && (tag_str == "VB" || tag_str == "VBP" || tag_str == "NN" || tag_str == "NNP"))
		base = noun_str;
	if (info->is_candidate_verb(base))
		return Engine::pt_true;
	return Engine::pt_false;
}

string Engine::GetLemma::getWakeUpString()
{
	return "get-lemma";
}

PredTree Engine::GetLemma::transform(PredTree pt, Engine *e)
{
	PredTree::iterator pi = pt.begin();
	if (pi.num_children() == 2) {
		CodePred result = e->run(CodePred(pi.firstChild()));
		PredTree sx_tree = result.pred();
		string noun_str = sx_tree.begin()->str;
		noun_str = noun_str.substr(0, noun_str.find("/"));
		noun_str = noun_str.substr(0, noun_str.find(":"));
		CodePred result3 = e->run(CodePred(pi.lastChild()));
		PredTree dx_tree = result3.pred();
		string tag_str = dx_tree.begin()->str;
		tagger *t = parser_singleton::get_tagger_instance();
		tagger_info *info = t->get_info();
		string base = info->get_conj(noun_str, tag_str);
		if (base == "")
			base = noun_str;
		return CodePred(base).pred();
	} else if (pi.num_children() == 1) {
		CodePred result = e->run(CodePred(pi.firstChild()));
		PredTree sx_tree = result.pred();
		string noun_str = sx_tree.begin()->str;
		string tag_str = noun_str.substr(noun_str.find("/")+1,noun_str.size());
		noun_str = noun_str.substr(0, noun_str.find("/"));
		noun_str = noun_str.substr(0, noun_str.find(":"));
		tagger *t = parser_singleton::get_tagger_instance();
		tagger_info *info = t->get_info();
		string base = info->get_conj(noun_str, tag_str);
		if (base == "")
			base = noun_str;
		return CodePred(base).pred();
	}
	return Engine::pt_false;
}

string Engine::IsCandidateAdjective::getWakeUpString()
{
	return "is-candidate-adjective";
}

PredTree Engine::IsCandidateAdjective::transform(PredTree pt, Engine *e)
{
	PredTree::iterator pi = pt.begin();
	if (pi.num_children() == 1) {
		CodePred result = e->run(CodePred(pi.firstChild()));
		PredTree sx_tree = result.pred();
		string noun_str = sx_tree.begin()->str;
		noun_str = noun_str.substr(0, noun_str.find("/"));
		metric *d = metric_singleton::get_metric_instance();
		if (d->is_adjective(noun_str))
			return Engine::pt_true;
	} else if (pi.num_children() == 2) {
		CodePred result = e->run(CodePred(pi.firstChild()));
		PredTree sx_tree = result.pred();
		string noun_str = sx_tree.begin()->str;
		noun_str = noun_str.substr(0, noun_str.find("/"));
		CodePred result3 = e->run(CodePred(pi.lastChild()));
		PredTree dx_tree = result3.pred();
		string tag_str = dx_tree.begin()->str;
		tagger *t = parser_singleton::get_tagger_instance();
		tagger_info *info = t->get_info();
		string base = info->conjugate(noun_str, tag_str);
		metric *d = metric_singleton::get_metric_instance();
		if (d->is_adjective(base))
			return Engine::pt_true;
	}

	return Engine::pt_false;

}

string Engine::IsCandidateAdverb::getWakeUpString()
{
	return "is-candidate-adverb";
}

PredTree Engine::IsCandidateAdverb::transform(PredTree pt, Engine *e)
{
	PredTree::iterator pi = pt.begin();
	if (pi.num_children() != 1)
		return Engine::pt_false;
	CodePred result = e->run(CodePred(pi.firstChild()));
	PredTree sx_tree = result.pred();
	string noun_str = sx_tree.begin()->str;
	noun_str = noun_str.substr(0, noun_str.find("/"));
	metric *d = metric_singleton::get_metric_instance();
	if (d->is_adverb(noun_str))
		return Engine::pt_true;
	return Engine::pt_false;
}

string Engine::Ask::getWakeUpString()
{
	return "ask";
}

PredTree Engine::Ask::transform(PredTree pt, Engine *e)
{
	PredTree::iterator pi = pt.begin();
	if (pi.num_children() != 1)
		return Engine::pt_nil;

	CodePred result = e->run(CodePred(pi.firstChild()));
	PredTree sx_tree = result.pred();
	std::stringstream ss;
	PredTree::children_iterator si(sx_tree.begin());
	for (; si != pt.end(); ++si) {
		ss << Predicate(PredTree(si));
		if (si.nextSibling() != pt.end())
			ss << ",";
	}
	DrtVect orig_drt = create_drtvect(ss.str());
	vector<DrtVect> all_drts = get_linked_drtvect_from_single_drtvect(orig_drt);


	if (all_drts.size() == 0)
		return Engine::pt_nil;

	Knowledge *k = e->getKnowledge();
	vector<KnowledgeAnswer> kav = k->getAnswers(all_drts.at(0));
	if (kav.size() > 0) {
		CodePred tmp_pred("list");
		for (int n = 0; n < kav.size(); ++n) {
			std::stringstream ss2;
			DrtVect d = kav.at(n).getPreds();
			print_vector_stream(ss2, d);
			Predicate item_pred("item");
			Predicate tmp_preds(string("preds(" + ss2.str() + ")"));
			string text, link;
			text = kav.at(n).getText();
			link = kav.at(n).getLink();
			if (text == "")
				text = "no text";
			if (link == "")
				link = "no link";
			Predicate tmp_text(string("text(" + text + ")"));
			Predicate tmp_link(string("link(" + link + ")"));
			PredTree pt_item = item_pred.pred();
			pt_item.appendTree(pt_item.begin(), tmp_preds.pred());
			pt_item.appendTree(pt_item.begin(), tmp_text.pred());
			pt_item.appendTree(pt_item.begin(), tmp_link.pred());
			PredTree pt = tmp_pred.pred();
			pt.appendTree(pt.begin(), pt_item);
			tmp_pred.pred() = pt;
		}
		return tmp_pred.pred();
	}
	return Engine::pt_nil;
}

string Engine::Quote::getWakeUpString()
{
	return "\'";
}

PredTree Engine::Quote::transform(PredTree pt, Engine *e)
{
	PredTree::iterator pi = pt.begin();
	if (pi.num_children() != 1 || pi.firstChild() == pt.end())
		return Engine::pt_false;
	CodePred result = CodePred(pi.firstChild());
	return result.pred();
}

string Engine::FirstChild::getWakeUpString()
{
	return "first-child";
}

PredTree Engine::FirstChild::transform(PredTree pt, Engine *e)
{
	PredTree::iterator pi = pt.begin();


	CodePred result = e->run(CodePred(pi.firstChild()));
	if (result.pred().height() > 1) {
		result = CodePred(result.pred().begin().firstChild());
		return result.pred();
	} else
		return Engine::pt_false;
}

string Engine::LastChild::getWakeUpString()
{
	return "last-child";
}

PredTree Engine::LastChild::transform(PredTree pt, Engine *e)
{
	PredTree::iterator pi = pt.begin();

	CodePred result = e->run(CodePred(pi.firstChild()));
	if (result.pred().height() > 1) {
		result = CodePred(result.pred().begin().lastChild());
		return result.pred();
	} else
		return Engine::pt_false;
}

static bool has_next(const PredTree &pred)
{
	string subj = extract_subject(Predicate(pred));
	if (subj.find("next") != string::npos)
		return true;
	return false;
}

static bool has_subject(const PredTree &pred)
{
	string subj = extract_subject(Predicate(pred));
	if (subj.find("subj") == string::npos)
		return true;
	return false;
}

static bool has_object(const PredTree &pred)
{
	string obj = extract_object(Predicate(pred));
	if (obj.find("obj") == string::npos && obj.find("none") == string::npos && obj.find("subj") == string::npos // The passive verbs invert the tags
			)
		return true;
	return false;
}

string Engine::HasNext::getWakeUpString()
{
	return "has-next";
}

PredTree Engine::HasNext::transform(PredTree pt, Engine *e)
{
	PredTree::iterator pi = pt.begin();
	if (pi.num_children() != 1)
		return Engine::pt_false;
	CodePred result = e->run(CodePred(pi.firstChild()));
	PredTree sx_tree = result.pred();
	if (has_next(sx_tree))
		return Engine::pt_true;
	return Engine::pt_false;
}

string Engine::HasSubject::getWakeUpString()
{
	return "has-subject";
}

PredTree Engine::HasSubject::transform(PredTree pt, Engine *e)
{
	PredTree::iterator pi = pt.begin();
	if (pi.num_children() != 1)
		return Engine::pt_false;
	CodePred result = e->run(CodePred(pi.firstChild()));
	PredTree sx_tree = result.pred();
	if (has_subject(sx_tree))
		return Engine::pt_true;
	return Engine::pt_false;
}

string Engine::HasObject::getWakeUpString()
{
	return "has-object";
}

PredTree Engine::HasObject::transform(PredTree pt, Engine *e)
{
	PredTree::iterator pi = pt.begin();
	if (pi.num_children() != 1)
		return Engine::pt_false;
	CodePred result = e->run(CodePred(pi.firstChild()));
	PredTree sx_tree = result.pred();
	if (has_object(sx_tree))
		return Engine::pt_true;
	return Engine::pt_false;
}

string Engine::IsCandidateNoun::getWakeUpString()
{
	return "is-candidate-noun";
}

PredTree Engine::IsCandidateNoun::transform(PredTree pt, Engine *e)
{
	PredTree::iterator pi = pt.begin();
	if (pi.num_children() != 2)
		return Engine::pt_false;
	CodePred result = e->run(CodePred(pi.firstChild()));
	PredTree sx_tree = result.pred();
	string candidate_str = sx_tree.begin()->str;
	candidate_str = candidate_str.substr(0, candidate_str.find("/"));
	candidate_str = candidate_str.substr(0, candidate_str.find(":"));
	CodePred result3 = e->run(CodePred(pi.lastChild()));
	PredTree dx_tree = result3.pred();
	string tag_str = dx_tree.begin()->str;

	tagger *t = parser_singleton::get_tagger_instance();
	tagger_info *info = t->get_info();
	string base = info->get_conj(candidate_str, tag_str);
	if (base == "")
		base = candidate_str;
	if (base == "" && (tag_str == "VB" || tag_str == "VBP" || tag_str == "NN" || tag_str == "NNP"))
		base = candidate_str;
	if (info->is_candidate_name(base))
		return Engine::pt_true;

	return Engine::pt_false;
}

string Engine::Parent::getWakeUpString()
{
	return "parent";
}

PredTree Engine::Parent::transform(PredTree pt, Engine *e)
{
	PredTree::iterator pi = pt.begin();
	if (pi.num_children() != 2)
		return Engine::pt_false;
	CodePred result = e->run(CodePred(pi.firstChild()));
	PredTree sx_tree = result.pred();
	CodePred result3 = e->run(CodePred(pi.lastChild()));
	PredTree dx_tree = result3.pred();
	PredTree::iterator fi = dx_tree.findSubtree(sx_tree);
	if (fi != dx_tree.end() && fi.parent() != dx_tree.end()) {
		CodePred return_pred = CodePred(fi.parent());
		return return_pred.pred();
	}
	return Engine::pt_false;
}

string Engine::Next::getWakeUpString()
{
	return "next";
}

PredTree Engine::Next::transform(PredTree pt, Engine *e)
{
	PredTree::iterator pi = pt.begin();
	if (pi.num_children() != 2)
		return Engine::pt_false;
	CodePred result = e->run(CodePred(pi.firstChild()));
	PredTree sx_tree = result.pred();
	CodePred result3 = e->run(CodePred(pi.lastChild()));
	PredTree dx_tree = result3.pred();
	PredTree::iterator fi = dx_tree.findSubtree(sx_tree);
	if (fi != dx_tree.end() && fi.nextSibling() != dx_tree.end()) {
		CodePred return_pred = CodePred(fi.nextSibling());
		return return_pred.pred();
	}
	return Engine::pt_false;
}

string Engine::Plus::getWakeUpString()
{
	return "plus";
}

PredTree Engine::Plus::transform(PredTree pt, Engine *e)
{
	PredTree::iterator pi = pt.begin();
	if (pi.num_children() != 2)
		return Engine::pt_false;
	CodePred result = e->run(CodePred(pi.firstChild()));
	PredTree sx_tree = result.pred();
	CodePred result3 = e->run(CodePred(pi.lastChild()));
	PredTree dx_tree = result3.pred();
	string sstr = sx_tree.begin()->str;
	string dstr = dx_tree.begin()->str;

	try {
		double snum = boost::lexical_cast<double>(sstr);
		double dnum = boost::lexical_cast<double>(dstr);
		double result = snum + dnum;
		CodePred return_pred = CodePred(boost::lexical_cast<string>(result));
		return return_pred.pred();
	} catch (...) {
	}
	return Engine::pt_false;
}

string Engine::Minus::getWakeUpString()
{
	return "minus";
}

PredTree Engine::Minus::transform(PredTree pt, Engine *e)
{
	PredTree::iterator pi = pt.begin();
	if (pi.num_children() != 2)
		return Engine::pt_false;
	CodePred result = e->run(CodePred(pi.firstChild()));
	PredTree sx_tree = result.pred();
	CodePred result3 = e->run(CodePred(pi.lastChild()));
	PredTree dx_tree = result3.pred();
	string sstr = sx_tree.begin()->str;
	string dstr = dx_tree.begin()->str;

	try {
		double snum = boost::lexical_cast<double>(sstr);
		double dnum = boost::lexical_cast<double>(dstr);
		double result = snum - dnum;
		CodePred return_pred = CodePred(boost::lexical_cast<string>(result));
		return return_pred.pred();
	} catch (...) {
	}
	return Engine::pt_false;
}

string Engine::SubString::getWakeUpString()
{
	return "sub-string";
}

PredTree Engine::SubString::transform(PredTree pt, Engine *e)
{
	PredTree::iterator pi = pt.begin();
	if (pi.num_children() != 3)
		return Engine::pt_false;
	CodePred result = e->run(CodePred(pi.firstChild()));
	PredTree sx_tree = result.pred();
	CodePred result2 = e->run(CodePred(pi.firstChild().nextSibling()));
	PredTree start_tree = result2.pred();
	CodePred result3 = e->run(CodePred(pi.lastChild()));
	PredTree end_tree = result3.pred();
	string start_str = start_tree.begin()->str;
	string end_str = end_tree.begin()->str;
	string text = sx_tree.begin()->str;

	try {
		int start_pos = boost::lexical_cast<int>(start_str);
		int end_pos = boost::lexical_cast<int>(end_str);
		CodePred return_pred = CodePred(boost::lexical_cast<string>(text.substr(start_pos, end_pos - start_pos)));
		return return_pred.pred();
	} catch (...) {
	}
	return Engine::pt_false;
}

string Engine::MatchAndSubstitute::getWakeUpString()
{
	return "match-and-substitute";
}

PredTree Engine::MatchAndSubstitute::transform(PredTree pt, Engine *e)
{
	PredTree::iterator pi = pt.begin();
	if (pi.num_children() != 4)
		return Engine::pt_false;

	CodePred result = e->run(CodePred(pi.firstChild()));
	PredTree subtree = result.pred();
	CodePred result2 = e->run(CodePred(pi.firstChild().nextSibling()));
	PredTree tree = result2.pred();
	CodePred result3 = e->run(CodePred(pi.firstChild().nextSibling().nextSibling()));
	PredTree from_pt = result3.pred();
	CodePred result4 = e->run(CodePred(pi.lastChild()));
	PredTree to_pt = result4.pred();

	PredTree::iterator pi2 = tree.findSubtree(subtree, tree.begin());
	if (pi2 != tree.end()) {
		Predicate tmp_pred(pi2);
		Predicate from_pred(from_pt);

		Upg upg;
		bool has_unified = tmp_pred.unify(from_pred, &upg);
		if (!has_unified)
			return Engine::pt_false;
		to_pt / upg;
		tree.replace(to_pt, pi2);
		return CodePred(tree).pred();
	}

	return Engine::pt_false;
}


void CodePred::operator /(const Upg &mgu)
{
	Predicate::operator/(mgu);
}

void CodePred::operator /(const DrtMgu &mgu)
{
	PredTree pt = this->pred();
	PredTree::iterator pBegin = this->pred().begin();
	DrtMgu::const_iterator ui = mgu.begin(), uend = mgu.end();
	while (ui != uend) {
		PTEl first(ui->first);
		PTEl second(ui->second);
		pt.replaceEachData(first, second, pBegin);
		++ui;
	}
}
