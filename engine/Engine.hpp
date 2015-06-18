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



#ifndef __ENGINE__
#define __ENGINE__

#include<iostream>
#include<string>
#include<exception>
#include<vector>
#include<map>
#include<boost/algorithm/string.hpp>
#include<boost/shared_ptr.hpp>
#include"../fopl/fopl.hpp"
#include"../infero/infero.hpp"
#include"../drt/metric_singleton.hpp"
#include"../drt/DrtPred.hpp"
#include"../drt/parser_singleton.hpp"

using std::string;
using std::vector;
using std::map;
using std::cout;
using std::endl;
using std::stringstream;
using boost::shared_ptr;

class KnowledgeAnswer;

class CodePred: public Predicate {
public:
	CodePred()
	{
	}
	CodePred(const PredTree &pt) :
			Predicate(pt)
	{
	}
	CodePred(PredTree::iterator pi) :
			Predicate(pi)
	{
	}
	CodePred(const string &s);
	//CodePred(const string &s)       : Predicate(s)  {}

	void insert(const string &s, const vector<DrtPred> &d);
	void insert(const string &s, const vector<FuzzyPred> &d);
	void insert(const string &s, const Predicate &p);
	void insert(const string &s, const DrtPred &p);
	void insert(const string &s, const bool &condition);
	void insert(const string &s, const vector<KnowledgeAnswer> &condition);

	bool isTrue();
	bool isFalse();
	bool isNil();
	bool isBreak();

	void operator /(const DrtMgu &);
	void operator /(const Upg &);
};

class Knowledge;
class Engine;

class Instruction {
public:
	virtual string getWakeUpString()
	{
		return string("");
	}
	virtual PredTree transform(PredTree p, Engine *e)
	{
		return PredTree("");
	}
};

class TemplateInstruction: public Instruction {
	string wake_up_string_;
	PredTree pt_result_;
	vector<string> variables_;

public:
	TemplateInstruction(const string &wus, const PredTree &result,
			const vector<string> &variables_);

	virtual string getWakeUpString();
	virtual PredTree transform(PredTree p, Engine *e);

};

class Engine {

	Knowledge *k_;

	vector<shared_ptr<Instruction> > default_instructions_;
	map<string, shared_ptr<Instruction> > wake_up_strs_;

	//map<string,CodePred> variables_;

	//hasVariable(const string &s);
	//setVariable(const string &s, const CodePred &cp);

	string cout_engine_;

public:
	Engine(Knowledge *k);
	~Engine();

	void bind(shared_ptr<Instruction>);
	void bindTemp(shared_ptr<Instruction>);
	void bindAnyway(shared_ptr<Instruction>);

	template<class T>
	vector<T> getList(const string &s);

	template<class T>
	T getElement(const string &s);

	CodePred run(CodePred);
	void setKnowledge(Knowledge *k)
	{
		k_ = k;
	}
	Knowledge* getKnowledge()
	{
		return k_;
	}

	template<class T>
	void operator <<(T &);
	void operator >>(stringstream &ss);

	// Univeral constants for the engine
	const static PredTree pt_false, pt_true, pt_nil, pt_break;

	// Useful functions

	class IsHypernym: public Instruction {
	public:
		virtual string getWakeUpString();
		virtual PredTree transform(PredTree pt, Engine *e);
	};

	class Print: public Instruction {
	public:
		virtual string getWakeUpString();
		virtual PredTree transform(PredTree pt, Engine *e);
	};

	class Repeat: public Instruction {
	public:
		virtual string getWakeUpString();
		virtual PredTree transform(PredTree pt, Engine *e);
	};

	class If: public Instruction {
	public:
		virtual string getWakeUpString();
		virtual PredTree transform(PredTree pt, Engine *e);
	};

	class IfElse: public Instruction {
	public:
		virtual string getWakeUpString();
		virtual PredTree transform(PredTree pt, Engine *e);
	};

	class Set: public Instruction {
	public:
		virtual string getWakeUpString();
		virtual PredTree transform(PredTree pt, Engine *e);
	};

	class FindStr: public Instruction {
	public:
		virtual string getWakeUpString();
		virtual PredTree transform(PredTree pt, Engine *e);
	};
	class FindStrHeight: public Instruction {
	public:
		virtual string getWakeUpString();
		virtual PredTree transform(PredTree pt, Engine *e);
	};

	class Find: public Instruction {
	public:
		virtual string getWakeUpString();
		virtual PredTree transform(PredTree pt, Engine *e);
	};

	class GetNamesFromRef: public Instruction {
	public:
		virtual string getWakeUpString();
		virtual PredTree transform(PredTree pt, Engine *e);
	};

	class And: public Instruction {
	public:
		virtual string getWakeUpString();
		virtual PredTree transform(PredTree pt, Engine *e);
	};

	class Or: public Instruction {
	public:
		virtual string getWakeUpString();
		virtual PredTree transform(PredTree pt, Engine *e);
	};

	class ForEach: public Instruction {
	public:
		virtual string getWakeUpString();
		virtual PredTree transform(PredTree pt, Engine *e);
	};

	class ForEachTree: public Instruction {
	public:
		virtual string getWakeUpString();
		virtual PredTree transform(PredTree pt, Engine *e);
	};

	class ForEachTreeHeight: public Instruction {
	public:
		virtual string getWakeUpString();
		virtual PredTree transform(PredTree pt, Engine *e);
	};
	class Break: public Instruction {
	public:
		virtual string getWakeUpString();
		virtual PredTree transform(PredTree pt, Engine *e);
	};

	class NumChildren: public Instruction {
	public:
		virtual string getWakeUpString();
		virtual PredTree transform(PredTree pt, Engine *e);
	};

	class ConnectedToVerbs: public Instruction {
	public:
		virtual string getWakeUpString();
		virtual PredTree transform(PredTree pt, Engine *e);
	};

	class NotConnectedToVerbs: public Instruction {
	public:
		virtual string getWakeUpString();
		virtual PredTree transform(PredTree pt, Engine *e);
	};

	class Equal: public Instruction {
	public:
		virtual string getWakeUpString();
		virtual PredTree transform(PredTree pt, Engine *e);
	};

	class LessThan: public Instruction {
	public:
		virtual string getWakeUpString();
		virtual PredTree transform(PredTree pt, Engine *e);
	};

	class GreaterThan: public Instruction {
	public:
		virtual string getWakeUpString();
		virtual PredTree transform(PredTree pt, Engine *e);
	};

	class AddList: public Instruction {
	public:
		virtual string getWakeUpString();
		virtual PredTree transform(PredTree pt, Engine *e);
	};

	class ExtractFirstTag: public Instruction {
	public:
		virtual string getWakeUpString();
		virtual PredTree transform(PredTree pt, Engine *e);
	};

	class ExtractSecondTag: public Instruction {
	public:
		virtual string getWakeUpString();
		virtual PredTree transform(PredTree pt, Engine *e);
	};

	class ExtractSubject: public Instruction {
	public:
		virtual string getWakeUpString();
		virtual PredTree transform(PredTree pt, Engine *e);
	};

	class ExtractPosition: public Instruction {
	public:
		virtual string getWakeUpString();
		virtual PredTree transform(PredTree pt, Engine *e);
	};

	class SetAt: public Instruction {
	public:
		virtual string getWakeUpString();
		virtual PredTree transform(PredTree pt, Engine *e);
	};

	class InsertAt: public Instruction {
	public:
		virtual string getWakeUpString();
		virtual PredTree transform(PredTree pt, Engine *e);
	};

	class IsCandidateVerb: public Instruction {
	public:
		virtual string getWakeUpString();
		virtual PredTree transform(PredTree pt, Engine *e);
	};

	class Join: public Instruction {
	public:
		virtual string getWakeUpString();
		virtual PredTree transform(PredTree pt, Engine *e);
	};

	class Ask: public Instruction {
	public:
		virtual string getWakeUpString();
		virtual PredTree transform(PredTree pt, Engine *e);
	};

	class Quote: public Instruction {
	public:
		virtual string getWakeUpString();
		virtual PredTree transform(PredTree pt, Engine *e);
	};

	class Not: public Instruction {
	public:
		virtual string getWakeUpString();
		virtual PredTree transform(PredTree pt, Engine *e);
	};

	class FindSub: public Instruction {
	public:
		virtual string getWakeUpString();
		virtual PredTree transform(PredTree pt, Engine *e);
	};

	class FindStrSub: public Instruction {
	public:
		virtual string getWakeUpString();
		virtual PredTree transform(PredTree pt, Engine *e);
	};

	class FirstChild: public Instruction {
	public:
		virtual string getWakeUpString();
		virtual PredTree transform(PredTree pt, Engine *e);
	};

	class HasSubject: public Instruction {
	public:
		virtual string getWakeUpString();
		virtual PredTree transform(PredTree pt, Engine *e);
	};

	class HasObject: public Instruction {
	public:
		virtual string getWakeUpString();
		virtual PredTree transform(PredTree pt, Engine *e);
	};

	class IsCandidateNoun: public Instruction {
	public:
		virtual string getWakeUpString();
		virtual PredTree transform(PredTree pt, Engine *e);
	};

	class Parent: public Instruction {
	public:
		virtual string getWakeUpString();
		virtual PredTree transform(PredTree pt, Engine *e);
	};

	class Next: public Instruction {
	public:
		virtual string getWakeUpString();
		virtual PredTree transform(PredTree pt, Engine *e);
	};

	// class Prior: public Instruction {
	// public:
	// 	virtual string getWakeUpString();
	// 	virtual PredTree transform(PredTree pt, Engine *e);
	// };

	class IsCandidateAdjective: public Instruction {
	public:
		virtual string getWakeUpString();
		virtual PredTree transform(PredTree pt, Engine *e);
	};

	class IsCandidateAdverb: public Instruction {
	public:
		virtual string getWakeUpString();
		virtual PredTree transform(PredTree pt, Engine *e);
	};

	class FindStrInTree: public Instruction {
	public:
		virtual string getWakeUpString();
		virtual PredTree transform(PredTree pt, Engine *e);
	};

	class Minus: public Instruction {
	public:
		virtual string getWakeUpString();
		virtual PredTree transform(PredTree pt, Engine *e);
	};

	class Plus: public Instruction {
	public:
		virtual string getWakeUpString();
		virtual PredTree transform(PredTree pt, Engine *e);
	};

	class HasNext: public Instruction {
	public:
		virtual string getWakeUpString();
		virtual PredTree transform(PredTree pt, Engine *e);
	};

	class IsVerbLevin: public Instruction {
	public:
		virtual string getWakeUpString();
		virtual PredTree transform(PredTree pt, Engine *e);
	};

	class While: public Instruction {
	public:
		virtual string getWakeUpString();
		virtual PredTree transform(PredTree pt, Engine *e);
	};

	class Defun: public Instruction {
	public:
		virtual string getWakeUpString();
		virtual PredTree transform(PredTree pt, Engine *e);
	};

	class FindStrPos: public Instruction {
	public:
		virtual string getWakeUpString();
		virtual PredTree transform(PredTree pt, Engine *e);
	};

	class SubString: public Instruction {
	public:
		virtual string getWakeUpString();
		virtual PredTree transform(PredTree pt, Engine *e);
	};

	class LastChild: public Instruction {
	public:
		virtual string getWakeUpString();
		virtual PredTree transform(PredTree pt, Engine *e);
	};

	class MatchAndSubstitute: public Instruction {
	public:
		virtual string getWakeUpString();
		virtual PredTree transform(PredTree pt, Engine *e);
	};

	class GetLemma: public Instruction {
	public:
		virtual string getWakeUpString();
		virtual PredTree transform(PredTree pt, Engine *e);
	};

	class ReplaceAt: public Instruction {
	public:
		virtual string getWakeUpString();
		virtual PredTree transform(PredTree pt, Engine *e);
	};

	class RemoveAt: public Instruction {
	public:
		virtual string getWakeUpString();
		virtual PredTree transform(PredTree pt, Engine *e);
	};
};

#endif // __ENGINE__
