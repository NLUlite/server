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



#ifndef __PARSER_SINGLETON__
#define __PARSER_SINGLETON__

#include<iostream>
#include<algorithm>
#include<vector>
#include<map>
#include<string>
#include<fstream>
#include<boost/tuple/tuple.hpp>
#include<boost/utility.hpp>
#include<boost/algorithm/string.hpp>
#include<boost/lexical_cast.hpp>
#include<boost/thread.hpp>
#include"../infero/infero.hpp"
#include"../nl/parser_include.hpp"
#include"../aux/parameters_singleton.hpp"


class parser_singleton {
     static tagger_info *info_;
     static tagger *tagger_;
     static parser_info *pinfo_;
     static parser *parser_;
     parser_singleton();
     ~parser_singleton() 
     {
	  delete info_;
	  delete tagger_;
	  delete parser_;
     }
public:
     static tagger_info* get_tagger_info_instance();
     static tagger* get_tagger_instance();
     static parser_info* get_parser_info_instance();
     static parser* get_parser_instance();
};

#endif // __PARSER_SINGLETON__
