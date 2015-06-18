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



#include"parameters.hpp"

Parameters::Parameters()
{
     data_dir_= "./data/";
     is_wikidata_proxy_= false;
}

void Parameters::setDir(string d) 
{
     data_dir_= d;
}

string Parameters::getDir() 
{
     return data_dir_;
}

void Parameters::setNumThreads(int n) 
{
     num_threads_= n;
}

int Parameters::getNumThreads() 
{
     return num_threads_;
}

void Parameters::setWikidataProxy(bool b)
{
	is_wikidata_proxy_= b;
}

bool Parameters::getWikidataProxy()
{
	return is_wikidata_proxy_;
}
