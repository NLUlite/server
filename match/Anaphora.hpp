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



/* 
 * File:   Anaphora.hpp
 * Author: alce
 *
 * Created on 12 August 2013, 1:14 PM
 */

#ifndef ANAPHORA
#define	ANAPHORA

#include<boost/algorithm/string/classification.hpp>
#include<boost/algorithm/string/erase.hpp>
#include<boost/algorithm/string/split.hpp>
#include"Match.hpp"
#include"../knowledge/Knowledge.hpp"
#include"../drt/DrtPred.hpp"
#include"../drt/phrase_info.hpp"
#include"../sense/Sense.hpp"

typedef vector<pair<string,string> > References;

class Anaphora {
     
     Knowledge *k_;
     PhraseInfo *pi_;
     Sense *sense_;
     vector<drt> drt_collection_;
     References internal_refs_;

     vector<pair<string,string> > getInstantiationWithPreds(const drt &drt_sample, const drt &drt_from, vector<DrtPred> &ref, vector<double> &);
     double getLikeliness(const drt &drt_sample, const drt &drt_from, const string &ref_to,  const string &ref_from, bool is_plural);

public:
     Anaphora(const vector<drt> dc, PhraseInfo *pi);
     ~Anaphora();
     
     vector< References > getReferences();
     vector< References > getDonkeyReferences();
     vector< References > getUninstantiated();
};

#endif	// ANAPHORA

