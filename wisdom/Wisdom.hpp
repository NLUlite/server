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



#ifndef __WISDOM__
#define __WISDOM__

#include<iostream>
#include<algorithm>
#include<vector>
//#include<forward_list>
#include<map>
#include<string>
#include "boost/date_time/posix_time/posix_time.hpp"
//#include<boost/chrono.hpp>
#include"../drt/drt_collection.hpp"
#include"../knowledge/Knowledge.hpp"
#include"../writer/Writer.hpp"
#include"../arbiter/Arbiter.hpp"
#include"WisdomInfo.hpp"


using std::string;
using std::vector;
using std::map;
//using boost::tuple;
//using boost::make_tuple;
using std::pair;
using std::make_pair;

//typedef tuple<vector<DrtPred>,pair<vector<vector<clause_vector> >,drt>, double> AnswerTuple;


class Wisdom {
     std::deque<drt>              data_;
     std::deque<drt>              questions_;
     std::deque<clause_vector>    clauses_;
     //pair<vector<drt>, vector<DrtPred> > questions_;
     //DrsPersonae personae_, q_personae_;
     //Rules rules_;
     Knowledge k_, k_questions_;
     Context context_;
     PhraseInfo *pi_;
     map<string,string> map_comments_;
     int load_num_;

     WisdomInfo wi_;

     ArbiterAnswer getAnswersFromAllQuestions(vector<drt> questions, Knowledge &k, WisdomInfo &wi);
     ArbiterAnswer getAnswersFromQuestionPair(pair<vector<drt>, DrtVect > q, Knowledge &, WisdomInfo &wi);
     ArbiterAnswer getAnswersFromCandidateQuestion(vector<QuestionVersions> qv, Knowledge &k, WisdomInfo &wi);
     
public:
     Wisdom();
     Wisdom(drt_collection &dc);
     Wisdom(vector<drt_collection> &dc);
     
     void addDiscourse(drt_collection &dc);
     void addDiscourse(vector<drt_collection> &dc);
     void setKnowledge(const Knowledge &k);
     ArbiterAnswer ask(const string &str);
     ArbiterAnswer ask(const string &str, const string &qID);
     ArbiterAnswer askWikidata(const string &str, const string &qID);
     ArbiterAnswer match(const string &str, const string &qID);
     ArbiterAnswer matchDrsWithText(const drt &drs, const string &str, const string &qID);
     
     ArbiterAnswer ask(const std::deque<drt> &questions, const string &qID);

     void loadFile(const string &str);
     void loadString(const string &str);
     void writeFile(const string &str);
     string writeString();
     string writeStringRDF();

     Knowledge& getKnowledge() {return k_;}
     
     void setPhraseInfo(PhraseInfo *pi) {pi_=pi;}
     
     string getComment(const string &qID);

     void setWisdomInfo(const WisdomInfo &wi);

     std::deque<drt> getQuestions() {return questions_;}
};


#endif // __WISDOM__
