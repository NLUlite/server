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



#include"WikidataSingleton.hpp"


WikidataInfo* WikidataSingleton::wdi_ = 0;


WikidataInfo* WikidataSingleton::instance()
{
     if(wdi_ != 0)
	  return wdi_;
     
     wdi_= new WikidataInfo();

	Parameters *ps= parameters_singleton::instance();
     string data_dir = ps->getDir();

     // load the wikidata info
     wdi_->loadRules( (data_dir+"/wikidata_rules.txt").c_str() );
     wdi_->loadForbidden( (data_dir+"/wikidata_forbidden.txt").c_str() );
     wdi_->loadTypes( (data_dir+"/wikidata_types.txt").c_str() );

     return wdi_;
}


