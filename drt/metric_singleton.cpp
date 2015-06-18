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



#include"metric_singleton.hpp"


metric* metric_singleton::metric_ = 0;
     

metric *metric_singleton::get_metric_instance()
{
     if(metric_ != 0)
	  return metric_;

     Parameters *ps= parameters_singleton::instance();
     string data_dir = ps->getDir();

     metric_= new metric();

     // load the synsets for names
     metric_->load_synsets  ( (data_dir+"/synsets.txt").c_str() );
     metric_->load_hypernyms( (data_dir+"/hypernyms.txt").c_str() );

     // load the synsets for verbs
     metric_->load_verb_synsets  ( (data_dir+"/synsets_verbs.txt").c_str() );
     metric_->load_verb_hypernyms( (data_dir+"/hypernyms_verbs.txt").c_str() );

     // load the proper names for people
     metric_->load_male_names( (data_dir+"/male_names.txt").c_str() );
     metric_->load_male_names( (data_dir+"/female_names.txt").c_str() );

     // load the adjective information
     metric_->load_adj_lexnames ( (data_dir+"/lexnames_adj.txt").c_str() );
     metric_->load_adj_similar  ( (data_dir+"/similar_adj.txt").c_str() );
     metric_->load_adj_pertainym( (data_dir+"/pertainym_adj.txt").c_str() );

     // load the adverb information
     metric_->load_adv_synsets  ( (data_dir+"/synsets_adv.txt").c_str() );
     metric_->load_adv_pertainym( (data_dir+"/pertainym_adv.txt").c_str() );

     
     metric_->load_word_freq ( (data_dir+"/word_freq.txt").c_str() );
     metric_->load_noun_levin( (data_dir+"/levin_nouns").c_str() );
     metric_->load_verb_levin( (data_dir+"/levin_verbs").c_str() );

     metric_->load_nominalization( (data_dir+"/nominalization_pairs.txt").c_str() );

	// country names
     metric_->load_countries( (data_dir+"/countries.txt").c_str() );

     return metric_;
}
