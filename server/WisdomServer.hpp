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



#ifndef __WISDOMSERVER__
#define __WISDOMSERVER__

#include<iostream>
#include<algorithm>
#include<vector>
#include<map>
#include<string>
#include<ctime>
#include<cstdlib>
#include<boost/asio.hpp>
#include<boost/thread/thread.hpp>
#include<boost/thread/mutex.hpp>
#include"../wisdom/Wisdom.hpp"
#include"../drt/drt_collection.hpp"
#include"../knowledge/Knowledge.hpp"
#include"../writer/Writer.hpp"
#include"../aux/parameters_singleton.hpp"

using boost::asio::ip::tcp;
using std::string;
using std::vector;
using std::map;
//using boost::tuple;
//using boost::make_tuple;
using std::pair;
using std::make_pair;

class WisdomServer {
     vector<string> id_list_;
     vector<string> writer_id_list_;
     map<string,Wisdom> w_;
     map<string,Writer*> writers_;
     map<string,string> keys_;
     map<string,string> password_;
     Writer *writer_;
     PhraseInfo *pi_;
     int global_num_;
     long int wisdom_last_num_;
     int port_;
     string home_dir_, data_dir_;
     vector<boost::tuple<int,int,string> > timer_;
     bool *server_is_available_;

     string get_string_answer_with_clauses(const vector<Answer> &answers);
     string get_string_answer(const ArbiterAnswer &ra, const string &qID);
     string process_input(string input);
     string process_input_for_scheduling(string input);
     void create_discourses_from_string(string str, int global_num=0, Wisdom *w = 0);
          
     string get_new_id();
     string get_new_writer_id();
     void   eraseExpired();
     
     void handleConnection(boost::asio::io_service *ioservice,tcp::acceptor *acceptor);

public:
     WisdomServer(int port=4001, string dir="", int nthreads = 1, bool wikidata_proxy= false);
     ~WisdomServer();

     void run();
};


#endif // __WISDOMSERVER__
