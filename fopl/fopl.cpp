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



#include"fopl.hpp"


const bool debug = false;

std::ostream & operator <<(std::ostream &out, const Predicate& pt)
{
	printLikePred(pt.pred(), "", out, false);
	return out;
}
std::ostream & operator <<(std::ostream &out, const Upg& upg)
{
	Upg::const_iterator ui = upg.begin(), uend = upg.end();

	for (; ui != uend; ++ui) {
		out << ui->first;
		out << " -> ";
		out << ui->second << std::endl;
	}
	return out;
}
std::ostream & operator <<(std::ostream &out, const genUpg& genupg)
{
	genUpg::const_iterator ui = genupg.begin(), uend = genupg.end();

	for (; ui != uend; ++ui) {
		out << ui->header << "(" << ui->first << ")";
		out << " -> ";
		out << ui->second << std::endl;
	}
	return out;
}
std::ostream &operator <<(std::ostream &out, const PTEl &ptel)
{
	out << ptel.str;
	if (debug)
		if (ptel.uniQuant())
			out << "{" << ptel.uniVal << "}";
	return out;
}

void operator /(std::vector<Predicate> &predVect, const Upg &upg)
{
	std::vector<Predicate>::iterator predIter;
	std::vector<Predicate>::iterator endPred = predVect.end();

	for (predIter = predVect.begin(); predIter != endPred; predIter++) {
		(*predIter) / upg;
		predIter->print();
	}
}

static inline std::string::size_type matchFirstOpeningPar(const std::string &str, std::string::size_type strIter)
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
static inline void recursInternals(std::string str, PredTree &pt, const PredTree::iterator &ptIter)
//Process predicate's (pred1, pred2, ....) internals
{
	std::string::size_type strIter = 0, commaIter, oParIter, cParIter;		  // Opening and closing parenthesis index.
	PredTree::iterator childIter;

	do {
		commaIter = str.find(',', strIter);
		oParIter = str.find('(', strIter);

		if (commaIter <= oParIter)
		// It works even if commaIter=oParIter=npos.
				{
			pt.appendChild(ptIter, str.substr(strIter, commaIter - strIter));
		} else {
			cParIter = matchFirstOpeningPar(str, str.find('(', strIter));
			childIter = pt.appendChild(ptIter, str.substr(strIter, oParIter - strIter));
			recursInternals(str.substr(oParIter + 1, cParIter - oParIter - 1), pt, childIter);
			strIter = cParIter;
		}
	} while ((strIter = str.find(',', strIter + 1) + 1)
	// Not using commaIter: commaIter could be a nested comma.
			!= std::string::npos + 1);
}

void printLikeTree(const PredTree &pred, const std::string &str)
{
	PredTree::iterator i = pred.begin();

	std::cout << str << std::endl;
	while (i != pred.end()) {
		std::cout << "- " << std::string(2 * (i.depth()), ' ') << "`" << *i << "`" << std::endl;
		++i;
	}
}
void printLikePred(const PredTree &pred, const std::string &str, std::ostream &out, const bool &endLine)
{
	PredTree::iterator i = pred.begin();

	int d = i.depth();
	int diff;
	out << str;
	while (i != pred.end()) {
		if (i.depth() > d) {
			++d;
			out << '(';
		}
		if ((diff = d - i.depth()) > 0) {
			d -= diff;
			for (; diff > 0; --diff)
				out << ')';
		}
		if(i.parent() != pred.end() && i.parent().firstChild() != i)
			out << ',';
		out << *i;
		++i;
	}
	while (((--d) - pred.begin().depth() + 1) > 0)
		out << ')';
	if (endLine)
		out << std::endl;
}

// PTEl struct:
PTEl::~PTEl()
{
}
PTEl::PTEl(const std::string &s)
{
	if (s.size() > 1 && s.at(0) == '_') {
		if (s.size() > 2 && s.at(1) == '_')
			uniVal = -1;
		else
			uniVal = 1;
	} else
		uniVal = 0;
	str = s;
}
PTEl::PTEl(const char *arg)
{
	std::string s(arg);
	if (s.size() > 1 && s.at(0) == '_') {
		if (s.size() > 2 && s.at(1) == '_')
			uniVal = -1;
		else
			uniVal = 1;
	} else
		uniVal = 0;
	str = s;
}

// PredTree Class::
bool PredTree::operator <(const PredTree &rhs) const
{
	PredTree::iterator i = this->begin();
	PredTree::iterator r = rhs.begin();
	PredTree::iterator end = this->end();
	PredTree::iterator rend = rhs.end();
	for (; i != end && r != rend; ++i, ++r)
		if (!(*i < *r))
			return false;
	if (i == end && r == end)
		return true;
	return false;
}

void PredTree::operator /(const Upg &upg)
{
	PredTree::iterator pBegin = begin();
	Upg::const_iterator ui = upg.begin(), uend = upg.end();
	while (ui != uend) {
		replaceEachData(ui->first, ui->second, pBegin);
		++ui;
	}
}
void PredTree::operator /(const genUpg &genupg)
{
	PredTree::iterator pBegin;
	PredTree::iterator pEnd = end();
	PredTree::iterator pIter;
	genUpg::const_iterator ui = genupg.begin(), uend = genupg.end();

	while (ui != uend) {
		pBegin = begin();
		while ((pIter = findData(ui->first, pBegin)) != pEnd) {
			if (pIter.node->parent && pIter.node->parent->data == ui->header)
				pIter = replace(ui->second, *pIter.node->parent);
			pBegin = ++pIter;
			if (pBegin == pEnd)
				break;
		}
		++ui;
	}
}
inline void PredTree::operator /(const Link &link)
{
	replaceEachData(link.first, link.second, begin());
}
inline void PredTree::substFrom(const Link &link, const PredTree::iterator &from)
{
	replaceEachData(link.first, link.second, from);
}
void PredTree::uniVal(const int &value)
{
	PredTree::iterator end = this->end();
	for (PredTree::iterator i = this->begin(); i != end; ++i)
		if (i->uniQuant())
			i->uniVal = value;
}


// Upg Class:

void Upg::uniVal(int num)
{
	Upg::iterator ui = this->begin(), end = this->end();
	while (ui != end) {
		ui->first.uniVal = num;
		++ui;
	}
}

void Upg::operator /(const Link &link)
{
	Upg::iterator ui = this->begin(), end = this->end();
	while (ui != end) {
		ui->second.replaceEachData(link.first, link.second, ui->second.begin());
		++ui;
	}
}

void Upg::operator /(const Upg &rhs)
// Simple unification within the most general unifier
{
	Upg::iterator ui, end = this->end();
	Upg::const_iterator ui_rhs = rhs.begin(), end_rhs = rhs.end();
	while (ui_rhs != end_rhs) {
		ui = this->begin();
		while (ui != end) {
			if (*ui->second.begin() == ui_rhs->first)
				ui->second = ui_rhs->second;
			if (ui->first == ui_rhs->first)
				ui->first = *ui_rhs->second.begin();
			++ui;
		}
		++ui_rhs;
	}
}

bool Upg::operator <(const Upg &rhs) const
{
	bool toReturn = true;

	Upg::const_iterator end = this->end();
	Upg::const_iterator rend = rhs.end();
	Upg::const_iterator i = this->begin();
	Upg::const_iterator r = rhs.begin();
	for (; i != end && r != rend; ++i) {
		toReturn = toReturn && (*i < *r);
	}
	return toReturn;
}

void Upg::add(const Upg &rhs)
{
	(*this) / rhs; // Substitute the new Upg first
	std::list<Link>::const_iterator riter = rhs.begin(), rend = rhs.end();
	for (; riter != rend; ++riter) {
		this->add(riter->first, riter->second);
	}
}

void Upg::addWithoutUnification(const Upg &rhs)
{
	std::list<Link>::const_iterator riter = rhs.begin(), rend = rhs.end();
	for (; riter != rend; ++riter) {
		this->add(riter->first, riter->second);
	}
}

void Upg::addReverse(const Upg &rhs)
{
	std::list<Link>::const_iterator riter = rhs.begin(), rend = rhs.end();
	for (; riter != rend; ++riter) {
		this->add(*riter->second.begin(), riter->first);
	}
}

// generalized Upg class

bool genUpg::operator <(const genUpg &rhs) const
{
	bool toReturn = true;

	genUpg::const_iterator end = this->end();
	genUpg::const_iterator rend = rhs.end();
	genUpg::const_iterator i = this->begin();
	genUpg::const_iterator r = rhs.begin();
	for (; i != end && r != rend; ++i) {
		toReturn = toReturn && (*i < *r);
		//      std::cout << "Upg "<< std::boolalpha << toReturn;
	}
	return toReturn;
}

// Predicate Class:
Predicate::Predicate()
{

}
Predicate::Predicate(const std::string &str)
{
	readLiteral(str);
}
Predicate::Predicate(const PredTree &pt)
{
	predicate = pt;
}
Predicate::~Predicate()
{
}
PredTree Predicate::readLiteral(const std::string &literal)
// Reads a literal/predicate
{
	PredTree pt;

	std::string str(literal);

	int d = 0;
	for (int n = 0; n < str.size(); ++n) {
		if (str.at(n) == '(')
			++d;
		if (str.at(n) == ')')
			--d;
	}
	if (d > 0)
		throw(std::runtime_error("String " + str + ", " + boost::lexical_cast<std::string>(d) + " cannot be parsed to a tree!"));

	if (str.size() != 0) {
		std::string::size_type oParIter, cParIter;
		//oParIter= MIN(str.find('('),str.find('.'));
		oParIter = str.find('(');
		cParIter = str.rfind(')');
		pt.rename(pt.begin(), str.substr(static_cast<std::string::size_type>(0), oParIter));
		if (oParIter != -1)
			recursInternals(str.substr(oParIter + 1, cParIter - oParIter - 1), pt, pt.begin());
		predicate = pt;
	}

	// Verify if this PredTree is made of all valid non 0-lenght strings
	int incr = 0;
	for (PredTree::iterator ptIter = pt.begin(); ptIter != pt.end(); ++ptIter, ++incr)
		if (ptIter->size() == 0) {
			if (debug) {
				std::cerr << str << " (" << incr << ") : not a valid predicate: missing term.\nSetting to nil.\n";
				predicate = PredTree("nil");
			}
			break;
		}
	return pt;
}
void Predicate::print() const
{
	std::cout << "Predicate " << ": ";
	printLikePred(predicate);
}

bool Predicate::equalAtoms(const PTEl &lhs, const PTEl &rhs) const
{
	if (lhs == rhs)
		return true;
	return false;
}

bool Predicate::unify(const Predicate &rhs, Upg *retUpg) const
// Only Universal quantifier'_*' are substituted in unification.
{
	if (rhs.pred().begin() == rhs.pred().end()) {
		return false;
	}
	PredTree sxTree = predicate;
	PredTree dxTree = rhs.predicate;

	PredTree::iterator sxIter = sxTree.begin(), end = PredTree::iterator(), dxIter = dxTree.begin();
	Link uni;

	if (sxIter->uniQuant()) {
		if (retUpg)
			retUpg->add(*predicate.begin(), rhs.predicate);
		return true;
	}
	if (dxIter->uniQuant()) {
		if (retUpg)
			retUpg->add(*rhs.predicate.begin(), predicate);
		return true;
	}
	if (++sxIter == end || ++dxIter == end) {
		if (predicate == rhs.predicate)
			return true;
		return false;
	}

	sxIter = sxTree.begin();
	dxIter = dxTree.begin();

	while (sxIter != end && dxIter != end) {
		if (!equalAtoms(*sxIter, *dxIter)) { //&& !(sxIter->uniQuant() && dxIter->uniQuant()) ) {
			if (sxIter->uniQuant()) {
				uni.first = *sxIter;
				uni.second = static_cast<PredTree>(dxTree.subTree(dxIter));
				sxIter = sxTree.replace(dxTree.subTree(dxIter), sxIter);
			} else if (dxIter->uniQuant()) {
				uni.first = *dxIter;
				uni.second = static_cast<PredTree>(sxTree.subTree(sxIter));
				dxIter = dxTree.replace(sxTree.subTree(sxIter), dxIter);
			} else
				return false;
			if (uni.second.findData(uni.first, uni.second.begin()) != end) {
				throw std::runtime_error("No Unification: Recursion in unifier!\n");
			}
			if (retUpg) {
				*retUpg / uni;
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
bool Predicate::genUnify(const Predicate &rhs, genUpg *retUpg) const
// Just Universal quantificator '_*' are substituted in unification. 
{
	PredTree sxTree = predicate;
	PredTree dxTree = rhs.predicate;

	if (sxTree.header()->data != dxTree.header()->data)
		return false;
	if (!sxTree.header()->firstChild && !dxTree.header()->firstChild)
		return true; // Only head trees
	if (!sxTree.header()->firstChild && dxTree.header()->firstChild)
		return false;
	if (sxTree.header()->firstChild && !dxTree.header()->firstChild)
		return false;
	if (sxTree.header()->firstChild->data.uniVal >= 0) {
		if (sxTree.header()->firstChild->data == dxTree.header()->firstChild->data)
			return true;
		else
			return false;
	}
	if (retUpg)
		retUpg->add(sxTree.header()->firstChild->data, dxTree.header()->data, dxTree);
	return true;
}

inline bool Predicate::unify(const Predicate &rhs)
{
	Upg upg;
	bool ret = unify(rhs, &upg);
	(*this) / upg;
	return ret;
}
inline bool Predicate::canUnify(const Predicate &rhs) const
{
	return unify(rhs, NULL);
}
inline bool Predicate::genUnify(const Predicate &rhs)
{
	genUpg genupg;
	bool ret = genUnify(rhs, &genupg);
	(*this) / genupg;
	return ret;
}
inline bool Predicate::canGenUnify(const Predicate &rhs) const
{
	return genUnify(rhs, NULL);
}

#define MAX(a,b) ((a>b)?a:b)
int Predicate::unique(const int &startAt)
{
	int max = 1;
	for (PredTree::iterator i = predicate.begin(); i != predicate.end(); ++i)
		if (i->uniQuant()) {
			i->uniVal += startAt;
			max = MAX(i->uniVal, max);
		}
	return startAt + max;
}

Predicate Predicate::deUnique() const
{
	Predicate retPred(*this);
	PredTree::iterator end = retPred.predicate.end();
	for (PredTree::iterator i = retPred.predicate.begin(); i != end; ++i)
		if (i->uniQuant())
			i->uniVal = 1;

	return retPred;
}
bool Predicate::hasEtiquette() const
{
	PredTree::iterator end = predicate.end();
	for (PredTree::iterator i = predicate.begin(); i != end; ++i)
		if (i->uniQuant() || i->genUniQuant())
			return true;
	return false;
}
void Predicate::readLikeLisp(std::string str)
{
	// erase all non alphanum characters
	int i, size = str.size();
	for (i = 0; i < size; ++i) {
		if (static_cast<int>(str.at(i)) < 32 || static_cast<int>(str.at(i)) > 128) {
			str.erase(i, 1);
			size = str.size();
		}
	}
	// erase spurious spaces
	for (i = 0; str.at(i) != '(';)
		if (str.at(i) == ' ')
			str.erase(i, 1);
	size = str.size();
	for (i = 0; i < size - 1; ++i) {
		if (str.at(i + 1) == ' ') {
			if (str.at(i) == ' ') {
				str.erase(i, 1);
				--i;
				size = str.size();
			}
		}
	}
	for (i = 0; i < size; ++i) {
		if (str.at(i) == ' ') {
			if (str.at(i - 1) == ')' && str.at(i + 1) != '(') {
				str.erase(i, 1);
				size = str.size();
			}
		}
	}
	// substitute all commas with '\254' and all points with '\253'
	for (i = 0; i < size; ++i) {
		if (str.at(i) == ',')
			str.at(i) = '\254';
		if (str.at(i) == '.')
			str.at(i) = '\253';
	}
	// invert "(word" as "word("
	char tmp;
	for (i = 0; i < size; ++i) {
		if (str.at(i) == '(' && str.at(i + 1) != ')') {
			if (str.at(i + 1) != ' ') {
				tmp = str.at(i + 1);
				str.at(i + 1) = '(';
				str.at(i) = tmp;
			} else {
				str.erase(i + 1, 1);
				size = str.size();
			}
		}
	}
	// substitute all spaces with commas
	for (i = 0; i < size; ++i) {
		if (str.at(i) == ' ')
			str.at(i) = ',';
	}
	readLiteral(str);
	// substitute back the commas and points
	PredTree::iterator end = predicate.end();
	for (PredTree::iterator i = predicate.begin(); i != end; ++i) {
		if (i->str == "\254")
			i->str = ",";
		if (i->str == "\253")
			i->str = ".";
	}
}

void Predicate::operator /(const Upg &upg)
{
	predicate / upg;
}
