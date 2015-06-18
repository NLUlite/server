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



#ifndef __REPLICATOR__
#define __REPLICATOR__

#include<iostream>
#include<algorithm>
#include<vector>
#include<cmath>
#include<map>
#include<string>
#include<fstream>
#include<boost/tuple/tuple.hpp>
#include<boost/utility.hpp>
#include<boost/algorithm/string.hpp>
#include<boost/lexical_cast.hpp>
#include<boost/asio.hpp>
#include<boost/thread/thread.hpp>
#include"../infero/infero.hpp"
#include"../infero_vector/infero_vector.hpp"
#include"../match/Match.hpp"
#include"../pragmatics/Presupposition.hpp"
#include"../context/Context.hpp"
#include"../nl/Filter.hpp"
#include"../drt/drt_collection.hpp"
#include"../writer/Writer.hpp"
#include"../engine/Engine.hpp"
#include"../wisdom/WisdomInfo.hpp"
#include"../wisdom/WikidataSingleton.hpp"


struct Answer {
     DrtVect drs_answer_;
     vector<vector<clause_vector> > clause_history_;
     drt drt_answer_;
     double weigth_;
     vector<pair<string,string> > WP_answers_;
     vector<drt> clause_answer_; // the final data matching the solver algorithm
     
public:

     void setDrsAnswer(const DrtVect &d) {drs_answer_= d;}
     void setDrtAnswer(const drt &d) {drt_answer_= d;}
     void setWeigth(double w) {weigth_= w;}
     void setClauseHistory(const vector<vector<clause_vector> > &c) {clause_history_= c;}
     void setClauseAnswer(const vector<drt> &c) {clause_answer_= c;}
     void setWPAnswer(const vector<pair<string,string> > &wpa) {WP_answers_= wpa;}

     DrtVect getDrsAnswer() const {return drs_answer_;}     
     drt getDrtAnswer() const {return drt_answer_;}
     double getWeigth() const {return weigth_;}
     vector<vector<clause_vector> > getClauseHistory() const {return clause_history_;}     
     vector<drt> getClauseAnswer() const {return clause_answer_;}
     vector<pair<string,string> > getWPAnswer() const {return WP_answers_;}
};


class ArbiterItem {
     vector<KnowledgeAnswer> kav_;
     string comment_;
     string status_;

public:
     void setKav(const vector<KnowledgeAnswer> &kav) {kav_= kav;}
     void setComment(const string &comment) {comment_= comment;}
     void setStatus(const string &status) {status_= status;}

     vector<KnowledgeAnswer> getKav() const {return kav_;}
     string getComment() const {return comment_;}
     string getStatus()  const {return status_;}
};


typedef struct {
	bool approximate_answer;
} StepInfo;

class ArbiterAnswer {
     vector<Answer> answer_;
     string comment_;
     string status_;

public:
     void setAnswer(const vector<Answer> &answer) {answer_= answer;}
     void setComment(const string &comment) {comment_= comment;}
     void setStatus(const string &status) {status_= status;}

     vector<Answer> getAnswers() const {return answer_;}
     string getComment() const {return comment_;}
     string getStatus()  const {return status_;}
};


class Arbiter {

     Knowledge *k_;
     WisdomInfo *wi_;

     ArbiterItem processComment(vector<KnowledgeAnswer> kav, vector<drt> &dquest_vect, StepInfo sinfo);
     vector<KnowledgeAnswer> arbiterStep(drt question, StepInfo &sinfo);
     ArbiterAnswer getEmptyAnswer();

     vector<KnowledgeAnswer> arbiterWikidataStep(drt question, StepInfo &sinfo);

     
public:
     Arbiter(Knowledge *k, WisdomInfo *wi) { k_= k; wi_=wi; }
     void setKnowledge(Knowledge *k) { k_= k; }

     ArbiterAnswer processAllQuestions(vector<drt> questions);

     ArbiterAnswer processQuestion(const pair<vector<drt>, vector<DrtPred> > &question_conj);
     ArbiterAnswer processQuestion(const vector<QuestionVersions > &question_conj);

     void processQuestionForThread(const pair<vector<drt>, vector<DrtPred> > &question_conj, ArbiterAnswer *answer);
     void processQuestionVersionsForThread(const vector<QuestionVersions > &question_conj, ArbiterAnswer *answer);

     ArbiterAnswer processWikidataQuestion(const pair<vector<drt>, vector<DrtPred> > &question_conj);
     ArbiterAnswer processWikidataQuestion(const vector<QuestionVersions > &question_conj);

};

#endif //__REPLICATOR__
