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



#include"parser_singleton.hpp"


tagger_info* parser_singleton::info_   = 0;
tagger*      parser_singleton::tagger_ = 0;
parser_info* parser_singleton::pinfo_  = 0;
parser*      parser_singleton::parser_ = 0;

     
tagger_info *parser_singleton::get_tagger_info_instance()
{
     if(info_ != 0)
	  return info_;

     Parameters *ps= parameters_singleton::instance();
     string data_dir = ps->getDir();

     init_ran(time(0) );
     info_= new tagger_info;
     info_->load_substitutions( (data_dir+"/substitutions.txt").c_str() );
     info_->load_tag_frequencies( (data_dir+"/tag_freq.txt").c_str() );
     info_->load_tag_frequencies_back( (data_dir+"/tag_freq_back.txt").c_str() );
     info_->load_tag_frequencies_prior( (data_dir+"/tag_freq_prior.txt").c_str() );
     info_->load_frequencies( (data_dir+"/freq.txt").c_str() );
     info_->load_frequencies_back( (data_dir+"/freq_back.txt").c_str() );
     info_->load_frequencies_prior( (data_dir+"/freq_prior.txt").c_str() );
     info_->load_conjugations( (data_dir+"/conjugations.txt").c_str() );

	///- Wikidata
     if(ps->getWikidataProxy()) {
     	info_->load_wikidata_qs( (data_dir+"wikidata_Qs.txt").c_str() );
     	info_->load_wikidata_names( (data_dir+"wikidata_Qs.txt").c_str() );
     }


     return info_;
}

tagger *parser_singleton::get_tagger_instance()
{
     if(tagger_ != 0)
	  return tagger_;

     Parameters *ps= parameters_singleton::instance();
     string data_dir = ps->getDir();

     init_ran(time(0) );
     if(info_ == 0) {
	  info_= new tagger_info;
	  info_->load_substitutions( (data_dir+"/substitutions.txt").c_str() );
	  info_->load_tag_frequencies( (data_dir+"/tag_freq.txt").c_str() );
	  info_->load_tag_frequencies_back( (data_dir+"/tag_freq_back.txt").c_str() );
	  info_->load_tag_frequencies_prior( (data_dir+"/tag_freq_prior.txt").c_str() );
	  info_->load_frequencies( (data_dir+"/freq.txt").c_str() );
	  info_->load_frequencies_back( (data_dir+"/freq_back.txt").c_str() );
	  info_->load_frequencies_prior( (data_dir+"/freq_prior.txt").c_str() );
	  info_->load_conjugations( (data_dir+"/conjugations.txt").c_str() );

	  ///- Wikidata
	  if(ps->getWikidataProxy()) {
		  info_->load_wikidata_qs( (data_dir+"wikidata_Qs.txt").c_str() );
		  info_->load_wikidata_names( (data_dir+"wikidata_Qs.txt").c_str() );
	  }
     }
     tagger_= new tagger(info_);

     return tagger_;
}

parser_info *parser_singleton::get_parser_info_instance()
{
     if(pinfo_ != 0)
	  return pinfo_;

     Parameters *ps= parameters_singleton::instance();
     string data_dir = ps->getDir();

     init_ran(time(0) );

     pinfo_ = new parser_info;
     pinfo_->load_feet_clauses ( (data_dir+"/feet.dat").c_str() );
     pinfo_->load_bulk_clauses ( (data_dir+"/bulk.dat").c_str() );
     pinfo_->load_roots_clauses( (data_dir+"/roots.dat").c_str() );
     
     return pinfo_;
}

parser *parser_singleton::get_parser_instance()
{
     if(parser_ != 0)
	  return parser_;

     Parameters *ps= parameters_singleton::instance();
     string data_dir = ps->getDir();

     if(tagger_ == 0 ) {
	  if(info_ == 0) {
	       info_= new tagger_info;;
	       info_->load_substitutions( (data_dir+"/substitutions.txt").c_str() );
	       info_->load_tag_frequencies( (data_dir+"/tag_freq.txt").c_str() );
	       info_->load_tag_frequencies_back( (data_dir+"/tag_freq_back.txt").c_str() );
	       info_->load_tag_frequencies_prior( (data_dir+"/tag_freq_prior.txt").c_str() );
	       info_->load_frequencies( (data_dir+"/freq.txt").c_str() );
	       info_->load_frequencies_back( (data_dir+"/freq_back.txt").c_str() );
	       info_->load_frequencies_prior( (data_dir+"/freq_prior.txt").c_str() );
	       info_->load_conjugations( (data_dir+"/conjugations.txt").c_str() );

		  ///- Wikidata
	       if(ps->getWikidataProxy()) {
	     	  info_->load_wikidata_qs( (data_dir+"wikidata_Qs.txt").c_str() );
	     	  info_->load_wikidata_names( (data_dir+"wikidata_Qs.txt").c_str() );
	       }
	  }
	  tagger_= new tagger(info_);
     }

     if(pinfo_ == 0) {
     	pinfo_ = new parser_info;
     	pinfo_->load_feet_clauses ( (data_dir+"/feet.dat").c_str() );
     	pinfo_->load_bulk_clauses ( (data_dir+"/bulk.dat").c_str() );
     	pinfo_->load_roots_clauses( (data_dir+"/roots.dat").c_str() );
     	pinfo_->load_matching_clauses( (data_dir+"/corrections.dat").c_str() );
     }


     parser_= new parser(info_,pinfo_);
     init_ran(time(0) );

     std::cerr << "Loading complete!" << std::endl;

     return parser_;
}

