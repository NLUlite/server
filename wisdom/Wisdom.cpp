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



#include"Wisdom.hpp"

const bool debug = false;
const bool measure_time = false;
const bool activate_context = false;
const bool commercial_version = true;
boost::mutex io_mutex_wisdom_counter;

const int max_size = 100;

template<class T>
static void print_vector(std::vector<T> &vs)
{
	if (vs.size()) {
		typename vector<T>::iterator tags_iter = vs.begin();
		while (tags_iter != vs.end()) {
			std::cout << (*tags_iter) << " ";
			++tags_iter;
		}
		std::cout << std::endl;
	}
}

static inline int max(int a, int b)
{
	return a < b ? b : a;
}
static inline int min(int a, int b)
{
	return a >= b ? b : a;
}

template<class T>
static void print_vector_return(std::vector<T> &vs)
{
	typename vector<T>::iterator tags_iter = vs.begin();
	while (tags_iter != vs.end()) {
		std::cout << (*tags_iter) << endl;
		++tags_iter;
	}
	std::cout << std::endl;
}

static void print_vector_file(std::ofstream &ff, const std::vector<DrtPred> &vs)
{
	vector<DrtPred>::const_iterator tags_iter = vs.begin();
	while (tags_iter < vs.end()) {
		tags_iter->print(ff);
		if (boost::next(tags_iter) != vs.end())
			ff << ",";
		++tags_iter;
	}
}

static void switch_children(DrtPred &pred)
{
	string fstr = extract_first_tag(pred);
	string sstr = extract_second_tag(pred);
	string str_tmp;
	str_tmp = fstr;
	fstr = sstr;
	sstr = str_tmp;
	implant_first(pred, fstr);
	implant_second(pred, sstr);
}


static void print_vector_stream(std::stringstream &ff, const std::vector<DrtPred> &vs)
{
	const int max_length= 49000;
	int start_size = ff.str().size();
	vector<DrtPred>::const_iterator tags_iter = vs.begin();
	while (tags_iter < vs.end()) {
		int end_size = ff.str().size();
		if(end_size-start_size > max_length)
			break;
		tags_iter->print(ff);
		if (boost::next(tags_iter) != vs.end())
			ff << ", ";
		++tags_iter;
	}
}

static string substitute_string(string str, const string& orig, const string& replace)
{
	int pos = 0;
	while ((pos = str.find(orig, pos)) != std::string::npos) {
		str.replace(pos, orig.size(), replace);
		pos += replace.size();
	}
	return str;
}



static string xml_substitutions(string text)
{
	text = substitute_string(text,"&","&amp;");
	text = substitute_string(text,"\"","&quot;");
	text = substitute_string(text,"\'","&apos;");
	text = substitute_string(text,"<","&lt;");
	text = substitute_string(text,">","&gt;");
	return text;
}


static string xml_substitutions_backward(string text)
{
	text = substitute_string(text,"&amp;","&");
	text = substitute_string(text,"&quot;","\"");
	text = substitute_string(text,"&apos;","\'");
	text = substitute_string(text,"&lt;","<");
	text = substitute_string(text,"&gt;",">");
	return text;
}


static vector<Predicate> get_additional_hypernyms()
{
	vector<Predicate> to_return;
	//to_return.push_back(Predicate("place(earth(europe(italy(rome,milan,venice),germany(berlin,munich,hamburg),belgium),usa|us|america(new_york)))"));
	to_return.push_back(Predicate("time(week,day,month,year)"));
	to_return.push_back(
			Predicate(
					"place|country|nation(world|earth(africa(nigeria(abuja),ethiopia(addis_ababa),egypt(cairo),democratic_republic_of_the_congo|congo(brazzaville),south_africa(cape_town,pretoria),tanzania(dodoma),kenya(nairobi),algeria(algiers),uganda(kampala),sudan(khartoum),morocco(rabat),ghana(accra),mozambique(maputo),ivory_coast,madagascar(antananarivo),angola(luanda),cameroon(yaoundé),niger(niamey),burkina_faso(ouagadougou),mali(bamako),malawi(lilongwe),zambia(lusaka),senegal(dakar),zimbabwe(harare),chad(n_djamena),guinea(conakry),tunisia(tunis),rwanda(kigali),south_sudan(juba),benin(porto-novo),somalia(mogadishu),burundi(bujumbura),togo(lomé),libya(tripoli),sierra_leone(freetown),central_african_republic(bangui),eritrea(asmara),republic_of_the_congo,liberia(monrovia),mauritania(Nouakchott),gabon(libreville),namibia(windhoek),botswana(gaborone),lesotho(maseru),equatorial_guinea(malabo),gambia(Banjul),guinea-bissau(bissau),mauritius(port_louis),swaziland(mbabane),djibouti,réunion,comoros(moroni),western_sahara,cape_verde(praia),mayotte,são_tomé_and_príncipe,seychelles(victoria),saint_helena,ascension_and_tristan_da_cunha),europe(russia(moscow),germany(berlin,heidelberg,hamburg),united_kingdom|uk|great_britain(london,nottingham,cambridge,oxford),france(paris),italy(rome,milan,florence,venice,palermo),spain(barcelona,madrid),ukraine(Kiev,chernobyl),poland(poznan,krakow,warsaw),romania(bucharest),netherlands(amsterdam),belgium(brussels),greece(athens),portugal(lisbon),czech_republic(Prague),hungary(budapest),sweden(umea,lund,malmo,uppsala,stockholm,gothenburg),belarus(mensk),austria(vienna),switzerland(bern),bulgaria(sofia),serbia(belgrade),denmark(copenhagen),finland(helsinki),slovakia(bratislava),norway(oslo),ireland(dublin),croatia(zagreb),bosnia_and_herzegovina,moldova(chisinau),lithuania(vilnius),albania(tirana),macedonia(skopje),slovenia(ljubljana),latvia(riga),kosovo(pristina),estonia(tallinn),montenegro(podgorica),luxembourg,malta(valletta),iceland(reykjavik),jersey,isle_of_man,andorra,guernsey,faroe_islands,liechtenstein(vaduz),monaco,san_marino,gibraltar,åland_islands,svalbard_and_jan_mayen,vatican_city),asia(afghanistan(Kabul,Kandahar,Herat,Mazar-i-Sharif,Kunduz,Taloqan,Jalalabad,Puli_Khumri,Charikar,Sheberghan,Ghazni,Sar-e_Pol,Khost,Chaghcharan,Mihtarlam,Farah,Pul-i-Alam,Samangan,Lashkar_Gah),armenia(yerevan),azerbaijan(baku),bahrain(al-manámah),bangladesh(dhaka),bhutan(thimphu),brunei(bandar_seri_begawan),burma,cambodia(phnom_penh),china(beijing,shangai,hong_kong),east_timor(dili),georgia,hong_kong,india(new_delhi),indonesia(jakarta),iran(tehran),iraq(baghdad),japan(tokyo),jordan(amman),kazakhstan(astana),north_korea(pyongyang),south_korea,kuwait,kyrgyzstan(bishkek),laos(vientiane),lebanon(beirut),macau,malaysia(kuala_lumpur),maldives(malé),mongolia(ulaan_Baatar),nepal(kathmandu),oman(muscat),pakistan(islamabad),papua_new_guinea(port_moresby),philippines(manila),qatar(doha),republic_of_china|taiwan(taipei),russia(moscow),saude_arabia,singapore,sri_lanka(colombo),syria(damascus),tajikistan(dushanbe),thailand(bangkok),turkmenistan(ashgabat),united_arab_emirates(abu_dhabi),uzbekistan(tashkent),vietnam(hanoi),yemen(sanaá)),north_america(us|usa|united_states|america(washington,new_york,los_angeles,san_francisco,seattle),canada(ottawa,toronto)),south_america(brazil(brasília,sao_paulo,rio_de_janeiro),colombia(santafé_de_Bogotá),argentina(buenos_aires),peru(lima),venezuela(caracas),chile(santiago),ecuador(quito),bolivia(sucre),paraguay(asunción),uruguay(montevideo),guyana(georgetown),suriname(paramaribo),french_guiana,falkland_islands),antartica,australasia(australia(canberra),new_zealand(auckland,wellington,hamilton,queenstown))))"));

	//to_return.push_back(Predicate("place(continent(europe,australia(western_australia)))"));
	//to_return.push_back(Predicate("nationality(afghan,albanian,algerian,american,andorran,angolan,argentine,armenian,aromanian,aruban,australian,austrian,azeri,bahamian,bahraini,bangladeshi,barbadian,belarusian,belgian,belizean,bermudian,boer,bosnian,brazilian,breton,british,british_virgin_islander,bulgarian,burkinabè,burundian,cambodian,cameroonian,canadian,catalan,cape_verdean,chadian,chilean,chinese,comorian,congolese,croatian,cuban,cypriot,turkish_cypriot,czech,dane,dominican,dominican_(republic),dominican_(commonwealth),dutch,east_timorese,ecuadorian,egyptian,emirati,english,eritrean,estonian,ethiopian,faroese,finn,finnish_swedish,fijian,filipino,french_citizen,georgian,german,baltic_german,ghanaian,gibraltar,greek,grenadian,guatemalan,guianese (french),guinean,guinea-bissau national,guyanese,haitian,honduran,hungarian,icelander,indian,indonesian,iranian,iraqi,irish,israeli,italian,ivoirian,jamaican,japanese,jordanian,kazakh,kenyan,korean,kosovo_albanian,kuwaiti,lao,latvian,lebanese,liberian,libyan,liechtensteiner,lithuanian,luxembourger,macedonian,malawian,maldivian,malian,maltese,manx,mauritian,mexican,moldovan,moroccan,mongolian,montenegrin,namibian,nepalese,new_zealander,nicaraguan,nigerien,nigerian,norwegian,pakistani,palauan,palestinian,panamanian,papua_new_guinean,paraguayan,peruvian,pole,portuguese,puerto_rican,quebecer,réunionnai,romanian,russian,baltic_russian,rwandan,salvadoran,são_tome_and_príncipe,saudi,scot,senegalese,serb,sierra_leonean,sikh,singaporean,slovak,slovene,somali,south_african,spaniard,sri_lankan,st_lucian,sudanese,surinamese,swede,swis,syrian,taiwanese,tanzanian,thai,tibetan,tobagonian,trinidadian,turk,tuvaluan,ugandan,ukrainian,uruguayan,uzbek,vanuatuan,venezuelan,vietnamese,welsh,yemeni,zambian,zimbabwean,swedish)"));
	return to_return;
}

static DrsPersonae get_additional_personae()
{
	vector<drt> drts;
	string link = "(from WordNet 3.1)";

	{
		string drs_str = "[any]/!WP([data]nameA1), be/V([data]verbC1,[data]nameA1,[data]nameB1), [any]/!WP([data]nameB1)";
		CodePred code(
				"for-each(_a,get-names-from-ref([data]nameA1),if(for-each(_b,get-names-from-ref([data]nameB1),if(is-hypernym(_a,_b),break)),break))");
		string text = "WordNet";

		DrtVect drtvect = create_drtvect(drs_str);
		drt tmp_drt(drtvect);
		tmp_drt.setCode(code);
		tmp_drt.setText(text);

		drts.push_back(tmp_drt);
	}

	{
		string drs_str =
				"[any]/!WP([data]nameA2), be/V([data]verbC2,[data]nameA2,[data]nameB2), not/RB([data]verbC2), [any]/!WP([data]nameB2)";
		CodePred code(
				"for-each(_a,get-names-from-ref([data]nameA2),if(for-each(_b,get-names-from-ref([data]nameB2),if(and(is-candidate-noun(_a,NN),is-candidate-noun(_b,NN),not(is-hypernym(_a,_b))),break)),break))");
		string text = "WordNet";

		DrtVect drtvect = create_drtvect(drs_str);
		drt tmp_drt(drtvect);
		tmp_drt.setCode(code);
		tmp_drt.setText(text);

		drts.push_back(tmp_drt);
	}

	DrsPersonae personae(drts, link);
	personae.compute();

	return personae;
}

static boost::tuple<vector<clause_vector>, vector<string>, string> get_additional_rules()
{
	vector<clause_vector> all_rules;
	vector<string> all_texts;

	clause_vector rule;
	string text;
	CodePred code;

	// rule= clause_vector(string("[*]/NN(_nameA_r1), [*]/V(_verbE_r1,_nameA_r1,_nameE_r1),[*]/NN(_nameE_r1)")
	//                            + ":- [*]/NN(_nameA_r1), be/VBP(_verbC_r1,_nameA_r1,_nameB_r1), [*]/NN(_nameB_r1) & [*]/NN(_nameB_r1), [*]/V(_verbE_r1,_nameB_r1,_nameE_r1),[*]/NN(_nameE_r1)");
	// text= "if A is B and B does something, then A does the same thing.";
	// rule.setText(text);
	// all_rules.push_back(rule);
	// all_texts.push_back(text);

	// rule= clause_vector(string("[*]/NN(_nameA_r2), [*]/V(_verbE_r2,_nameE_r2,_nameA_r2)")
	//                            + ":- [*]/NN(_nameA_r2), be/VBP(_verbC_r2,_nameA_r2,_nameB_r2), [*]/NN(_nameB_r2) & [*]/NN(_nameB_r2), [*]/V(_verbE_r2,_nameE_r2,_nameB_r2)");
	// text= "if A is B and B does something, then A does the same thing.";
	// rule.setText(text);
	// all_rules.push_back(rule);
	// all_texts.push_back(text);

	// rule= clause_vector(string("[*]/NN(_nameA_r3), [*]/V(_verbE_r3,_nameA_r3,_nameE_r3),@MOTION_TO(_verbE_r3,_nameF_r3),[*]/NN(_nameF_r3) ")
	// 			 + ":- [*]/NN(_nameA_r3), be/VBP(_verbC_r3,_nameA_r3,_nameB_r3), [*]/NN(_nameB_r3) & [*]/NN(_nameB_r3), [*]/V(_verbE_r3,_nameB_r3,_nameE_r3),@MOTION_TO(_verbE_r3,_nameF_r3),[*]/NN(_nameF_r3)");
	// text= "if A is B and B does something, then A does the same thing.";
	// rule.setText(text);
	// all_rules.push_back(rule);
	// all_texts.push_back(text);

	// rule= clause_vector(string("[*]/NN(_nameA_r4), [*]/V(_verbE_r4,_nameE_r4,_nameA_r4),@TOPIC(_verbE_r4,_nameF_r4),[*]/NN(_nameF_r4)")
	//                            + ":- [*]/NN(_nameA_r4), be/VBP(_verbC_r4,_nameA_r4,_nameB_r4), [*]/NN(_nameB_r4) & [*]/NN(_nameB_r4), [*]/V(_verbE_r4,_nameE_r4,_nameB_r4),@TOPIC(_verbE_r4,_nameF_r4),[*]/NN(_nameF_r4)");
	// text= "if A is B and B does something, then A does the same thing.";
	// rule.setText(text);
	// all_rules.push_back(rule);
	// all_texts.push_back(text);

	rule =
			clause_vector(
					"[any]/NN(_nameD1), be/V(verbE1,_nameD1,none3), @PLACE_AT(verbE1,_nameG1), [any]/NN(_nameG1) :- [*]/NN(_nameD1), be/V(_verbH1,_nameD1,_none1), @PLACE_AT(_verbH1,_nameC1), [*]/NN(_nameC1), @TIME(_verbH1,_present|past|future|continuative) & [*]/NN(_nameF1), be/V(_verbI1,_nameF1,_none2), @PLACE_AT(_verbI1,_nameG1), [*]/NN(_nameG1)");
	code =
			CodePred(
					"for-each(_a,get-names-from-ref(_nameC1),if(for-each(_b,get-names-from-ref(_nameF1),if(equal(_a,_b),break)),break))");
	rule.setCode(code);
	code =
			CodePred(
					"or(for-each(_a,get-names-from-ref(_nameC1),if(for-each(_b,get-names-from-ref(_nameF1),if(equal(_a,_b),break)),break)),find-str(_nameF1,_name))");
	rule.setMatchCode(code);

	text = "if A is in B and B is in C, then A is in C.";
	rule.setText(text);

	all_rules.push_back(rule);
	all_texts.push_back(text);

	rule =
			clause_vector(
					"[any]/NN(_nameD2), be/V(verbE2,_nameD2,none), @PLACE_AT(verbE2,_nameG2), [any]/NN(_nameG2) :- [*]/NN(_nameD2), be/V(_verbH2,_nameD2,_none), @PLACE_AT(_verbH2,_nameC2), [*]/NN(_nameC2), @PLACE_AT(_nameC2,_nameG2), [*]/NN(_nameG2)");
	text = "if A is in B in C, then A is in C.";
	rule.setText(text);

	all_rules.push_back(rule);
	all_texts.push_back(text);

	rule =
			clause_vector(
					"[any]/NN(_nameD3), be/V(verbE3,_nameD3,none), @PLACE_AT(verbE3,_nameG3), [any]/NN(_nameG3) :- [*]/NN(_nameD3), be/V(_verbH3,_nameD3,_none), @PLACE_AT(_verbH3,_nameC3), [*]/NN(_nameC3), @PLACE_AT(_nameC3,_nameL3), [*]/NN(_nameL3), @PLACE_AT(_nameL3,_nameG3), [*]/NN(_nameG3)");
	text = "if A is in B in C, then A is in C.";
	rule.setText(text);
	all_rules.push_back(rule);
	all_texts.push_back(text);

     rule= clause_vector("be/VB(verbE11,_nameE3,_nameE5), [what]/NN#[pivot](_nameE1), [what]/NN#[pivot](_nameE3), [what]/NN#[pivot](_nameE5), @GENITIVE(_nameE5,_nameE1) :- [what]/NN#[pivot](_nameE1), have/VB(_verbE2,_nameE1,_nameE5), [what]/NN#[pivot](_nameE3), [what]/NN#[pivot](_nameE5), @GENITIVE(_nameE5,_nameE3)"); 
     text= "if something has a property of X then the propery of something is X";
     rule.setText(text);
     all_rules.push_back(rule);
     all_texts.push_back(text);

//     rule= clause_vector("be/VB(verbF11,_nameF3,_nameF5), [what]/NN#[pivot](_nameF3), atomic_number/NN#[pivot](_nameF5), [what]/NN#[pivot](_nameF1), @GENITIVE(_nameF5,_nameF1) :- [what]/NN#[pivot](_nameF1), be/VB(_verbF2,_nameF1,_nameF5), [what]/NN#[pivot](_nameF3), element/NN#[pivot](_nameF5), @QUANTITY(_nameF5,_nameF3)");
//     text= "if something is element X then the atomic number of something is X";
//     rule.setText(text);
//     all_rules.push_back(rule);
//     all_texts.push_back(text);



	string link = "Common sense";
	return boost::make_tuple(all_rules, all_texts, link);
}

Wisdom::Wisdom()
{
	load_num_ = 0;
	// default no answer (Reduntant: It should be defined in the Arbiter class)
	map_comments_["no_answer"] = "The answer is not found in the data";

	vector<Predicate> hyps = get_additional_hypernyms();
	k_.addHypernym(hyps);

	boost::tuple<vector<clause_vector>, vector<string>, string> rules_tuple = get_additional_rules();
	Rules additional_rules(rules_tuple.get<0>(), rules_tuple.get<1>(), rules_tuple.get<2>());
	additional_rules.compute();
	k_.addRules(additional_rules);
	DrsPersonae dp = get_additional_personae();
	//personae_.addPersonae(dp);
	k_.addPersonae(dp);
}

Wisdom::Wisdom(drt_collection &dc)
{
	load_num_ = 0;
	// default no answer (Reduntant: It should be defined in the Arbiter class)
	map_comments_["no_answer"] = "The answer is not found in the data";

	this->addDiscourse(dc);

	vector<Predicate> hyps = get_additional_hypernyms();
	k_.addHypernym(hyps);

	boost::tuple<vector<clause_vector>, vector<string>, string> rules_tuple = get_additional_rules();
	Rules additional_rules(rules_tuple.get<0>(), rules_tuple.get<1>(), rules_tuple.get<2>());
	additional_rules.compute();
	k_.addRules(additional_rules);
	DrsPersonae dp = get_additional_personae();
	k_.addPersonae(dp);
}

Wisdom::Wisdom(vector<drt_collection> &dc)
{
	load_num_ = 0;
	// default no answer (Reduntant: It should be defined in the Arbiter class)
	map_comments_["no_answer"] = "The answer is not found in the data";

	vector<drt_collection>::iterator diter = dc.begin();
	vector<drt_collection>::iterator dend = dc.end();
	for (; diter != dend; ++diter) {
		addDiscourse(*diter);
	}

	vector<Predicate> hyps = get_additional_hypernyms();
	k_.addHypernym(hyps);

	boost::tuple<vector<clause_vector>, vector<string>, string> rules_tuple = get_additional_rules();
	Rules additional_rules(rules_tuple.get<0>(), rules_tuple.get<1>(), rules_tuple.get<2>());
	additional_rules.compute();
	k_.addRules(additional_rules);
	DrsPersonae dp = get_additional_personae();
	k_.addPersonae(dp);
}

void Wisdom::addDiscourse(vector<drt_collection> &dc)
{
	vector<drt_collection>::iterator diter = dc.begin();
	vector<drt_collection>::iterator dend = dc.end();
	for (; diter != dend; ++diter) {
		addDiscourse(*diter);
	}
}

static vector<DrtPred> unname_unifiers(vector<DrtPred> preds)
{
	for (int n = 0; n < preds.size(); ++n) {
		string head_str = extract_header(preds.at(n));
		vector<string> children = preds.at(n).extract_children();
		for (int m = 0; m < children.size(); ++m) {
			string str_tmp = children.at(m);
			if (str_tmp.size() && str_tmp.at(0) == '_') {
				str_tmp.erase(0, 1);
				children.at(m) = str_tmp;
			}
		}
		preds.at(n).implant_children(children);
	}
	return preds;
}

static vector<drt> get_data_from_questions(const vector<drt> &qdrt)
{
	vector<drt> to_return;

	for (int n = 0; n < qdrt.size(); ++n) {
		DrtVect tmp_quest = qdrt.at(n).predicates_with_references();
		tmp_quest = unname_unifiers(tmp_quest);
		if (debug) {
			cout << "QUESTIONS::: ";
			print_vector(tmp_quest);
		}
		drt tmp_drt(tmp_quest);
		tmp_drt.setText(qdrt.at(n).getText());
		to_return.push_back(tmp_drt);
	}
	return to_return;
}

static vector<drt> only_declarations(const vector<drt> &data)
{
	vector<drt> to_return;
	for(int n=0; n < data.size(); ++n) {
		if(!data.at(n).has_question() && !data.at(n).has_condition() ) {
			to_return.push_back(data.at(n));
		}
	}
	return to_return;
}

static vector<drt> sort_all(vector<drt> &data)
{
	for(int n=0; n < data.size(); ++n) {
		DrtVect tmp_drtvect= data.at(n).predicates_with_references();
		drt_sort(tmp_drtvect);
		data.at(n).setPredicates(tmp_drtvect);
	}
	return data;
}


static DrtVect invert_allocutions(DrtVect drtvect)
{
	for (int n = 0; n < drtvect.size(); ++n) {
		string head = extract_header(drtvect.at(n));
		if (head == "@PARENT-ALLOCUTION") {
			switch_children(drtvect.at(n));
			implant_header(drtvect.at(n), "@ALLOCUTION");
		}
	}

	return drtvect;
}

static DrtVect post_process_drtvect(DrtVect drtvect)
{
	for (int n = 0; n < drtvect.size(); ++n) {
		string head = extract_header(drtvect.at(n));
		if (drtvect.at(n).is_complement() && head == "@OWN") {
			implant_header(drtvect.at(n), "@OWNED_BY");
			switch_children(drtvect.at(n));
		}
	}

	return drtvect;
}


static vector<drt> process_single_preds(vector<drt> &data)
{
	for(int n=0; n < data.size(); ++n) {
		DrtVect tmp_drtvect= data.at(n).predicates_with_references();
		tmp_drtvect = invert_allocutions(tmp_drtvect);
		tmp_drtvect = post_process_drtvect(tmp_drtvect);
		data.at(n).setPredicates(tmp_drtvect);
	}
	return data;
}



void Wisdom::addDiscourse(drt_collection &dc)
{
	dc.setWisdomInfo(wi_);

	vector<clause_vector> cv = dc.extract_clauses();
	clauses_.insert(clauses_.end(), cv.begin(), cv.end());

	pair<vector<drt>, vector<DrtPred> > q = dc.extract_questions();
	questions_.insert(questions_.end(), q.first.begin(), q.first.end());

	// add questions as data
	vector<drt> additional_data = get_data_from_questions(q.first);
	DrsPersonae dp_questions(additional_data, dc.getLink());
	dp_questions.compute();

	DrsPersonae dc_ds(dc.getPersonae());
	/// LOAD-TAG
	vector<drt> data = dc.extract_data();
	data = only_declarations(data);
	data = sort_all(data);
	data = process_single_preds(data);
	data_.insert(data_.end(), data.begin(), data.end());
	///

	k_.addPersonae(dc_ds);
	k_questions_.addPersonae(dp_questions);
	Rules dc_rules(dc.getRules());
	k_.addRules(dc_rules);

	if (activate_context) {
		context_.add(dc.getContext());
	}
}

ArbiterAnswer Wisdom::getAnswersFromAllQuestions(vector<drt> questions, Knowledge &k, WisdomInfo &wi)
{
	int timeout = 5;

	Arbiter arbiter(&k,&wi);

	ArbiterAnswer repl_answer;

	repl_answer = arbiter.processAllQuestions(questions);


//	ArbiterAnswer repl_answer;
//	boost::thread rthread(boost::bind(&Arbiter::processQuestionForThread,&arbiter,q,&repl_answer));
	//boost::posix_time::ptime deadline = boost::posix_time::microsec_clock::local_time() + boost::posix_time::seconds(timeout);
	//boost::chrono::system_clock::time_point deadline = boost::chrono::system_clock::now() + boost::chrono::seconds(timeout);
	//rthread.timed_join(deadline);
//	rthread.timed_join(boost::posix_time::seconds(timeout) );

	return repl_answer;
}


ArbiterAnswer Wisdom::getAnswersFromQuestionPair(pair<vector<drt>, DrtVect> q, Knowledge &k, WisdomInfo &wi)
{
	int timeout = 5;

	vector<drt> questions = q.first;
	vector<DrtPred> conjs = q.second;

	Arbiter arbiter(&k,&wi);

	ArbiterAnswer repl_answer;
	//Parameters *ps = parameters_singleton::instance();
	//if(ps->getWikidataProxy()) {
	if(wi_.isWikidata() ) {
		repl_answer= arbiter.processWikidataQuestion(q);
	} else 
		repl_answer = arbiter.processQuestion(q);


//	ArbiterAnswer repl_answer;
//	boost::thread rthread(boost::bind(&Arbiter::processQuestionForThread,&arbiter,q,&repl_answer));
	//boost::posix_time::ptime deadline = boost::posix_time::microsec_clock::local_time() + boost::posix_time::seconds(timeout);
	//boost::chrono::system_clock::time_point deadline = boost::chrono::system_clock::now() + boost::chrono::seconds(timeout);
	//rthread.timed_join(deadline);
//	rthread.timed_join(boost::posix_time::seconds(timeout) );

	return repl_answer;
}

ArbiterAnswer Wisdom::getAnswersFromCandidateQuestion(vector<QuestionVersions> qv, Knowledge &k, WisdomInfo &wi)
{
	int timeout = 5;

	clock_t start;
	if(debug || measure_time) {
		start = clock();
	}

	k.setWisdomInfo(wi);
	Arbiter arbiter(&k,&wi);

	ArbiterAnswer repl_answer;
	//Parameters *ps = parameters_singleton::instance();
	//if(ps->getWikidataProxy()) {
	if(wi_.isWikidata() ) {
		repl_answer= arbiter.processWikidataQuestion(qv);
	} else 
		repl_answer = arbiter.processQuestion(qv);

//	Arbiter arbiter(&k,&wi);
//	ArbiterAnswer repl_answer;
//	boost::thread rthread(boost::bind(&Arbiter::processQuestionVersionsForThread,&arbiter,qv,&repl_answer));
	//boost::posix_time::ptime deadline = boost::posix_time::microsec_clock::local_time() + boost::posix_time::seconds(timeout);
	//boost::chrono::system_clock::time_point deadline = boost::chrono::system_clock::now() + boost::chrono::seconds(timeout);
	//rthread.timed_join(deadline);
//	rthread.timed_join(boost::posix_time::seconds(timeout) );


	if (debug || measure_time) {
		clock_t end = clock();
		cout << "Mtime_arbiter::: " << (end - start) / (double) CLOCKS_PER_SEC << endl;
	}

	return repl_answer;
}

static vector<DrtPred> adjust_WP(vector<DrtPred> preds)
{
	for (int n = 0; n < preds.size(); ++n) {
		string head_str = extract_header(preds.at(n));
		if (head_str == "what") {
			implant_header(preds.at(n), "animal|thing|event");
		}
	}
	return preds;
}

static vector<DrtPred> trim_time(vector<DrtPred> preds)
// delete the present time indicators from preds
{
	vector<DrtPred> ret;
	for (int n = 0; n < preds.size(); ++n) {
		string head_str = extract_header(preds.at(n));
		if (head_str == "@TIME") {

////// This commented part is correct but it makes the matching frustrating. To be inserted when the matching is perfected!!	       
//               string tense= extract_second_tag(preds.at(n));
//               if(tense.find("present") != string::npos)
//		    continue; 
//////         

			continue;/// skipping all @TIME from questions
		}
		ret.push_back(preds.at(n));
	}
	return ret;
}

static vector<DrtPred> trim_modals(vector<DrtPred> preds)
// delete the modals!!!
{
	vector<DrtPred> ret;
	for (int n = 0; n < preds.size(); ++n) {
		string head_str = extract_header(preds.at(n));
		if (head_str == "@MODAL") {
			//string tense= extract_second_tag(preds.at(n));
			continue;
		}
		ret.push_back(preds.at(n));
	}
	return ret;
}

static bool is_valid_reference(const string &ref)
{
	if (ref.find("name") != string::npos || ref.find("ref") != string::npos || ref.find("verb") != string::npos
			|| ref.find("obj") != string::npos || ref.find("subj") != string::npos || ref.find("prev") != string::npos
			|| ref.find("next") != string::npos || ref.find("from") != string::npos || ref.find("to") != string::npos
			|| ref.find("none") != string::npos)
		return true;

	return false;
}

static vector<DrtPred> name_unifiers(vector<DrtPred> preds)
{
	for (int n = 0; n < preds.size(); ++n) {
		string head_str = extract_header(preds.at(n));
		vector<string> children = preds.at(n).extract_children();
		for (int m = 0; m < children.size(); ++m) {
			string str_tmp = children.at(m);
			if (is_valid_reference(str_tmp)) {
				str_tmp.insert(0, "_");
				children.at(m) = str_tmp;
			}
		}
		preds.at(n).implant_children(children);
	}
	return preds;
}

static vector<drt> create_questions_from_declarations(const vector<drt> &drts_tmp)
{
	vector<drt> ret_data;
	for (int m = 0; m < drts_tmp.size(); ++m) {
		DrtVect tmp_quest = drts_tmp.at(m).predicates_with_references();
		string text = drts_tmp.at(m).getText();
		tmp_quest = trim_time(tmp_quest); // erase the present tense indicators
		tmp_quest = trim_modals(tmp_quest); // erase the present tense indicators
		tmp_quest = name_unifiers(tmp_quest);
		tmp_quest = adjust_WP(tmp_quest);
		QuestionList ql;
		drt tmp_drt(tmp_quest);
		tmp_drt.setText(text);
		ql.add(tmp_drt.predicates_with_references());
		tmp_drt.setQuestionList(ql);
		ret_data.push_back(tmp_drt);
	}
	return ret_data;
}

static bool compare_answers(const Answer &lhs, const Answer &rhs)
{
	return lhs.getWeigth() > rhs.getWeigth();
}

ArbiterAnswer Wisdom::match(const string &str, const string &qID)
{
	drt_collection dc(str, -1, context_, pi_);
	dc.setWisdomInfo(wi_);

	pair<vector<drt>, DrtVect> q = dc.extract_questions();
	if (debug) {
		cout << "MATCH_WISDOM:::" << q.first.size() << endl;
	}
	if (q.first.size() == 0) {
		vector<drt> data = dc.extract_data();
		q.first = create_questions_from_declarations(data);
	}
	ArbiterAnswer repl_answer = getAnswersFromQuestionPair(q, k_, wi_);
	ArbiterAnswer repl_answer_from_question = getAnswersFromQuestionPair(q, k_questions_, wi_);

	vector<Answer> answers = repl_answer.getAnswers();
	vector<Answer> qanswers = repl_answer_from_question.getAnswers();
	if (answers.size() == 0)
		repl_answer.setComment(repl_answer_from_question.getComment());

	if (debug) {
		cout << "MATCH_WISDOM2:::" << answers.size() << endl;
		cout << "MATCH_WISDOM3:::" << qanswers.size() << endl;
	}

	answers.insert(answers.end(), qanswers.begin(), qanswers.end());
	sort(answers.begin(), answers.end(), compare_answers);
	repl_answer.setAnswer(answers);

	if (q.first.size() != 0) {
		map_comments_[qID] = repl_answer.getComment();
	}
	return repl_answer;
}

ArbiterAnswer Wisdom::matchDrsWithText(const drt &drs, const string &str, const string &qID)
{
	drt_collection dc(str, -1, context_, pi_);
	pair<vector<drt>, DrtVect> q = dc.extract_questions();
	if (q.first.size() == 0) {
		vector<drt> data = dc.extract_data();
		q.first = create_questions_from_declarations(data);
	}
	if (debug) {
		cout << "MATCH_DRS_WISDOM:::" << q.first.size() << endl;
	}
	Match match(&k_);
	MatchSubstitutions msubs;
	DrtVect tmp_pred = drs.predicates_with_references();
	vector<Answer> answers;
	for (int n = 0; n < q.first.size(); ++n) {
		DrtVect question = q.first.at(n).predicates_with_references();

		double w = match.singlePhraseMatch(tmp_pred, question, &msubs);
		if (w != 0) {
			Answer answ;
			answ.setWeigth(w);
			drt tmp_drt(tmp_pred);
			answ.setDrtAnswer(tmp_drt);
			answers.push_back(answ);
			break;
		}
	}

	ArbiterAnswer ra;
	if (answers.size()) {
		ra.setAnswer(answers);
		ra.setComment("Yes");
		ra.setStatus("yes");
		if (q.first.size() != 0) {
			map_comments_[qID] = "Yes";
		}
	}
	return ra;
}

ArbiterAnswer Wisdom::ask(const std::deque<drt> &questions, const string &qID)
{
	vector<drt> qs;
	std::deque<drt>::const_iterator qiter = questions.begin();
	std::deque<drt>::const_iterator qend  = questions.end();

	for(;qiter != qend; ++qiter) {
		qs.push_back(*qiter);
		if(debug) {
			std::cout << "QPAIR::: ";
			DrtVect tmp_drt= qiter->predicates_with_references();
			print_vector(tmp_drt);
		}
	}
	ArbiterAnswer repl_answer = getAnswersFromAllQuestions(qs, k_, wi_);
	if (qs.size() != 0) {
		map_comments_[qID] = repl_answer.getComment();
	}

	return repl_answer;
}


ArbiterAnswer Wisdom::ask(const string &str, const string &qID)
{
	drt_collection dc(str, -1, context_, pi_);
	dc.setWisdomInfo(wi_);
	vector<QuestionVersions> cq = dc.extract_candidate_questions();
	ArbiterAnswer repl_answer = getAnswersFromCandidateQuestion(cq, k_, wi_);
	if (cq.size() != 0) {
		map_comments_[qID] = repl_answer.getComment();
	}

	return repl_answer;
}

ArbiterAnswer Wisdom::askWikidata(const string &str, const string &qID)
{
	wi_.setWikidata("true");
	wi_.setNumAnswers(100);
	drt_collection dc(str, -1, context_, pi_, wi_);
	dc.setWisdomInfo(wi_);
	vector<QuestionVersions> cq = dc.extract_candidate_questions();
	ArbiterAnswer repl_answer = getAnswersFromCandidateQuestion(cq, k_, wi_);
	if (cq.size() != 0) {
		map_comments_[qID] = repl_answer.getComment();
	}

	return repl_answer;
}


ArbiterAnswer Wisdom::ask(const string &str)
{
	drt_collection dc(str, -1, context_, pi_);
	pair<vector<drt>, DrtVect> q = dc.extract_questions();
	ArbiterAnswer repl_answer = getAnswersFromQuestionPair(q, k_, wi_);
	return repl_answer;
}

string Wisdom::getComment(const string &qID)
{
	string to_return;
	map<string, string>::iterator miter = map_comments_.find(qID);
	if (miter != map_comments_.end()) {
		to_return = miter->second;
		return to_return;
	} else
		throw(std::runtime_error("Wisdom: no such question ID."));
	return to_return;
}

vector<DrtPred> string_to_drs_cast(const string &s)
{
	string str = s;
	boost::erase_all(str, string(" ")); // strip all the spaces from the string

	// saving the consequence
	int p, size = str.size();
	int depth = 0;
	int p1 = 0, p2;
	// the string
	vector<string> strs;
	for (p = 0; p < size; ++p) {
		if (str.at(p) == '(')
			++depth;
		if (str.at(p) == ')')
			--depth;
		if (depth < 0)
			throw(std::invalid_argument("Badly formed drs."));
		if (str.at(p) == ',' && depth == 0) {
			p2 = p;
			strs.push_back(str.substr(p1, p2 - p1));
			p1 = p2 + 1;
		}
	}
	strs.push_back(str.substr(p1, p - p1));

	vector<string>::iterator si = strs.begin();
	vector<string>::iterator se = strs.end();
	vector<DrtPred> to_return;
	to_return.resize(strs.size());
	vector<DrtPred>::iterator hi = to_return.begin();
	for (; si != se; ++si, ++hi) {
		*hi = DrtPred(*si);
	}

	return to_return;
}

void Wisdom::loadFile(const string &str)
{
	std::ifstream file;
	file.open(str.c_str());
	file.seekg(0, std::ios::end);
	int length = file.tellg();
	file.seekg(0, std::ios::beg);
	char *buffer = new char[length];
	file.read(buffer, length);
	file.close();
	this->loadString(buffer);
	delete buffer;
}

static DrtVect update_references(const DrtVect &tmp_drts, int load_num )
{
	DrtVect to_return(tmp_drts);

	for(int n=0; n < to_return.size(); ++n) {
//		if (load_num == 0) // All loaded wisdom must have a new label, because they could be confused with the added text
//			continue;
		vector<string> children = extract_children(to_return.at(n));
		for(int j=0; j < children.size(); ++j) {
			if( children.at(j).find("_") != string::npos ) // it is not an attribute like "most", "all", "present" ...
				children.at(j) += (string) "_"+ boost::lexical_cast<string>(load_num);
		}
		implant_children(to_return.at(n),children);
	}

	return to_return;
}

static clause_vector update_references(const clause_vector &clause, int load_num )
{
	DrtVect cons(clause.getConsequence());
	vector<DrtVect> all_hyp(clause.getAllHypothesis());

	clause_vector return_clause;

	for(int n=0; n < cons.size(); ++n) {
//		if (load_num == 0) // All loaded wisdom must have a new label, because they could be confused with the added text
//			continue;
		vector<string> children = extract_children(cons.at(n));
		for(int j=0; j < children.size(); ++j) {
			if( children.at(j).find("_") != string::npos ) // it is not an attribute like "most", "all", "present" ...
				children.at(j) += (string) "_"+ boost::lexical_cast<string>(load_num);
		}
		implant_children(cons.at(n),children);
	}

	return_clause.setConsequence(cons);

	for(int m=0; m < all_hyp.size(); ++m) {
		DrtVect hyp = all_hyp.at(m);
		for(int n=0; n < hyp.size(); ++n) {
//			if (load_num == 0)
//				continue;
			vector<string> children = extract_children(hyp.at(n));
			for(int j=0; j < children.size(); ++j) {
				if( children.at(j).find("_") != string::npos ) // it is not an attribute like "most", "all", "present" ...
					children.at(j) += (string) "_"+ boost::lexical_cast<string>(load_num);
			}
			implant_children(hyp.at(n),children);
		}
		all_hyp.at(m) = hyp;
	}
	return_clause.setAllHypothesis(all_hyp);

	return return_clause;
}

class WisdomThreadCounter {
	int num_, max_num_;

public:
	WisdomThreadCounter(int max_num) :
			max_num_(max_num), num_(0)
	{
	}
	int getCurrentNumber();
};

int WisdomThreadCounter::getCurrentNumber()
{
	boost::mutex::scoped_lock lock(io_mutex_wisdom_counter); // this method is used concurrently
	if (num_ < max_num_) {
		++num_;
		return num_ - 1;
	}
	return -1;
}

class PersonWThread {

	vector<DrsPersonae> *person_vect_;
	vector<string> *data_, *data_link_, *data_text_;
	WisdomThreadCounter *counter_;
	int load_num_;

	void launchMatchingThread(DrsPersonae *person, string &data, string &data_link, string &data_text);

public:
	PersonWThread(vector<DrsPersonae> *person_vect, vector<string> *data, vector<string> *data_link, vector<string> *data_text, int load_num, WisdomThreadCounter *counter) :
		person_vect_(person_vect), data_(data), data_link_(data_link), data_text_(data_text), load_num_(load_num), counter_(counter)
	{
	}

	void operator()();
};

void PersonWThread::operator()()
{
	int num = 0;
	while (num < data_->size() && num < data_link_->size() && num < data_text_->size() ) {
		num = counter_->getCurrentNumber();

		if (num == -1)
			break;

		DrsPersonae *out  = &person_vect_->at(num);
		string in = data_->at(num);
		string in_link = data_link_->at(num);
		string in_text = data_text_->at(num);

		this->launchMatchingThread(out, in, in_link, in_text);
	}
}


void PersonWThread::launchMatchingThread(DrsPersonae *personae, string &data, string &data_link, string &data_text)
{
	vector<DrtPred> tmp_drs = string_to_drs_cast(data);
	tmp_drs = update_references(tmp_drs, load_num_ );

	drt tmp_drt(tmp_drs);
	string link = xml_substitutions_backward( data_link );
	string text = xml_substitutions_backward( data_text );
	vector<DrtVect> all_data;
	all_data.push_back(tmp_drs);
	vector<string> texts;
	texts.push_back(text);
	DrsPersonae tmp_persona(all_data, texts, link);
	tmp_persona.compute();
	personae->addPersonae(tmp_persona);

	// Add the presuppositions
	// Note: presuppositions are not added to the data_
	Presupposition pres(tmp_drt);
	vector<drt> pres_drt = pres.get();
	for (int m = 0; m < pres_drt.size(); ++m) {
		DrtVect pres_pred = pres_drt.at(m).predicates_with_references();
		vector<DrtVect> all_pres_data;
		all_pres_data.push_back(pres_pred);
		DrsPersonae pres_persona(all_pres_data, texts, link);
		pres_persona.compute();
		personae->addPersonae(pres_persona);
	}
}



void Wisdom::loadString(const string &str)
{
	std::stringstream file(str);

	const int max_line_length= 500000;
	char c_line[max_line_length];
	vector<string> data;
	vector<string> data_links;
	vector<string> data_texts;
	vector<string> clauses;
	vector<string> clauses_links;
	vector<string> clauses_texts;
	vector<double> clauses_weigths;
	vector<string> questions;
	vector<string> questions_links;
	vector<string> questions_texts;

	bool data_trigger = false, clauses_trigger = false, questions_trigger = false, preds_trigger = false, link_trigger = false,
			weigth_trigger = false, text_trigger = false;

	file.seekg(0, std::ios_base::end);
	int eof_pos= file.tellg();
	file.seekg(0, std::ios_base::beg);

	if(debug) {
		cout << "FILE_LENGTH0::: " << eof_pos << endl;
	}

	if (!file.bad()) {
		// std::cerr << "Loading wisdom from client." << std::endl;
		int length = 0;
		while (length < eof_pos) {
			length = file.tellg();

			if(debug) {
				cout << "FILE_LENGTH::: " << length << endl;
			}

			file.getline(c_line, max_line_length);
			string line(c_line);
			if (line.find("<data>") != string::npos) {
				data_trigger = true;
				file.getline(c_line, max_line_length);
				line = c_line;
			}
			if (line.find("</data>") != string::npos) {
				data_trigger = false;
				file.getline(c_line, max_line_length);
				line = c_line;
			}
			if (line.find("<clauses>") != string::npos) {
				clauses_trigger = true;
				file.getline(c_line, max_line_length);
				line = c_line;
			}
			if (line.find("</clauses>") != string::npos) {
				clauses_trigger = false;
				file.getline(c_line, max_line_length);
				line = c_line;
			}
			if (line.find("<questions>") != string::npos) {
				questions_trigger = true;
				file.getline(c_line, max_line_length);
				line = c_line;
			}
			if (line.find("</questions>") != string::npos) {
				questions_trigger = false;
				file.getline(c_line, max_line_length);
				line = c_line;
			}
			if (line.find("<predicates>") != string::npos) {
				preds_trigger = true;
				file.getline(c_line, max_line_length);
				line = c_line;
			}
			if (line.find("</predicates>") != string::npos) {
				preds_trigger = false;
				file.getline(c_line, max_line_length);
				line = c_line;
			}
			if (line.find("<link>") != string::npos) {
				link_trigger = true;
				file.getline(c_line, max_line_length);
				line = c_line;
			}
			if (line.find("</link>") != string::npos) {
				link_trigger = false;
				file.getline(c_line, max_line_length);
				line = c_line;
			}
			if (line.find("<text>") != string::npos) {
				text_trigger = true;
				file.getline(c_line, max_line_length);
				line = c_line;
			}
			if (line.find("</text>") != string::npos) {
				text_trigger = false;
				file.getline(c_line, max_line_length);
				line = c_line;
			}
			if (line.find("<weight>") != string::npos) {
				weigth_trigger = true;
				file.getline(c_line, max_line_length);
				line = c_line;
			}
			if (line.find("</weight>") != string::npos) {
				weigth_trigger = false;
				file.getline(c_line, max_line_length);
				line = c_line;
			}
			if (data_trigger && preds_trigger)
				data.push_back(line);
			if (data_trigger && link_trigger)
				data_links.push_back(line);
			if (data_trigger && text_trigger)
				data_texts.push_back(line);
			if (clauses_trigger && preds_trigger)
				clauses.push_back(line);
			if (clauses_trigger && link_trigger)
				clauses_links.push_back(line);
			if (clauses_trigger && text_trigger)
				clauses_texts.push_back(line);
			if (clauses_trigger && weigth_trigger)
				clauses_weigths.push_back(boost::lexical_cast<double>(line));
			if (questions_trigger && preds_trigger)
				questions.push_back(line);
			if (questions_trigger && text_trigger)
				questions_texts.push_back(line);
			if (questions_trigger && link_trigger)
				questions_links.push_back(line);
		}
	} else
		throw std::length_error(std::string("Wisdom file") + str + " finished unexpectedly.");

	Rules rules;
	//DrsPersonae clause_personae;
	// Load the clauses into the Wisdom class

	//clauses_.clear();  /// You cannot do this!!! Otherwise you cannot load multiple files.

	if(debug)
		std::cerr << "LOADING_CLAUSE::: " << std::boolalpha << wi_.getLoadClauses() << endl;

	for (int n = 0; n < clauses.size() && wi_.getLoadClauses(); ++n) {
		string clause_string = clauses.at(n);
		clause_string = substitute_string(clause_string, "\\and", "&");
		clause_vector tmp_clause;
		try {
			clause_vector tmp_clause_build(clause_string);
			tmp_clause = tmp_clause_build;
		} catch (...) {
			continue;
		}
		tmp_clause = update_references(tmp_clause, load_num_ );
		if (debug) {
			cout << "LOADING_CLAUSE::: " << tmp_clause << endl;
			cout << "CLAUSE_STRING::: " << clause_string << endl;
		}

		string link = xml_substitutions_backward( clauses_links.at(n) );
		string text = xml_substitutions_backward( clauses_texts.at(n) );
		tmp_clause.setLink(link);
		tmp_clause.setText(text);
		tmp_clause.setWeigth(clauses_weigths.at(n));
		tmp_clause.find_levin();
		vector<string> levins = tmp_clause.getLevinCons();
		clauses_.push_back(tmp_clause);
		vector<clause_vector> cvect;
		cvect.push_back(tmp_clause);
		vector<string> texts;
		texts.push_back(text);
		Rules tmp_rules(cvect, texts, link);
		//DrsPersonae tmp_persona(tmp_clause.getConsequence(), "" , link);
		tmp_rules.compute();
		rules.addRules(tmp_rules);
	}

	////
//	tuple<vector<clause_vector>, vector<string>, string> rules_tuple = get_additional_rules();
//	Rules additional_rules(rules_tuple.get<0>(), rules_tuple.get<1>(), rules_tuple.get<2>());
//	additional_rules.compute();
//	rules.addRules(additional_rules);
	////

	k_.addRules(rules);
	if (debug) {
		std::cout << "PUTTING_INTO_KNOWLEDGE4:::" << std::flush << endl;
	}

	// Load the data into the Wisdom class

	//data_.clear();   /// You cannot do this!!! Otherwise you cannot load multiple files.

	//vector<DrtPred> all_data;


//	int max_threads = 1;
//	Parameters *par = parameters_singleton::instance();
//	if(commercial_version)
//		max_threads = par->getNumThreads();
//	int num_results= data.size();
//	int num_threads = min(num_results, max_threads);
//	vector<DrsPersonae> person_vect(num_results);
//	WisdomThreadCounter counter0(num_results);
//	vector<PersonWThread> pt0_vect(num_threads, PersonWThread(&person_vect, &data, &data_links, &data_texts, load_num_, &counter0) );
//	boost::thread_group g0;
//	for (int t = 0; t < num_threads; ++t) {
//		g0.create_thread( pt0_vect.at(t) );
//	}
//	g0.join_all();
//
//	for (int n=0; n < num_results; ++n) {
//		person_vect.at(n).sort();
//		k_.addPersonae(person_vect.at(n));
//	}

		//data_.clear();   /// You cannot do this!!! Otherwise you cannot load multiple files.

	//vector<DrtPred> all_data;
	string old_link= "", link = "";
	vector<DrtVect> all_data;
	vector<DrtVect> all_pres_data;
	vector<string> texts;
	vector<string> pres_texts;
	vector<DrsPersonae> all_personae;
	for (int n = 0; n < data.size(); ++n) {
		old_link= link;
		vector<DrtPred> tmp_drs;
		try {
			tmp_drs = string_to_drs_cast(data.at(n));
		} catch (...) {
			continue;
		}

		if(tmp_drs.size() > max_size)
			continue;
		////
		tmp_drs = update_references(tmp_drs, load_num_ );
		////

//		if (debug) {
//			std::cerr << "LOADING_DRS::: " << n << endl;
//			print_vector(tmp_drs);
//			std::cerr << "DRS_STRING::: " << data.at(n) << endl;
//		}

		drt tmp_drt(tmp_drs);
		link = xml_substitutions_backward( data_links.at(n) );
		if(link != old_link || n == data.size()-1 ) {
			DrsPersonae personae;
			DrsPersonae tmp_persona(all_data, texts, old_link);
			tmp_persona.compute();
			personae.addPersonae(tmp_persona);
			DrsPersonae pres_persona(all_pres_data, pres_texts, old_link);
			pres_persona.compute();
			personae.addPersonae(pres_persona);
			all_data.clear();
			all_pres_data.clear();
			texts.clear();
			pres_texts.clear();

			all_personae.push_back(personae);
		}
		string text = xml_substitutions_backward( data_texts.at(n) );
		all_data.push_back(tmp_drs);
		texts.push_back(text);
		if(wi_.getAddData() ) {
			data_.push_back(tmp_drt);
		}

		// Add the presuppositions
		// Note: presuppositions are not added to the data_
		Presupposition pres(tmp_drt);
		vector<drt> pres_drt = pres.get();
		for (int m = 0; m < pres_drt.size(); ++m) {
			DrtVect pres_pred = pres_drt.at(m).predicates_with_references();
			all_pres_data.push_back(pres_pred);
			pres_texts.push_back(text);
		}
	}
	

	// Insert the last data
	{
		DrsPersonae personae;
		DrsPersonae tmp_persona(all_data, texts, link);
		tmp_persona.compute();
		personae.addPersonae(tmp_persona);
		DrsPersonae pres_persona(all_pres_data, pres_texts, link);
		pres_persona.compute();
		personae.addPersonae(pres_persona);
		all_data.clear();
		all_pres_data.clear();
		texts.clear();
		pres_texts.clear();

		all_personae.push_back(personae);
	}


	for(int n=0; n < all_personae.size(); ++n) {
		//DrsPersonae personae = all_personae.at(n);
		//personae.sort();
		k_.addPersonae(all_personae.at(n));
	}
	k_.clearNameRefs();

	
	// Load the questions into the Wisdom class
	vector<drt> questions_drt;
	//questions_.clear();
	for (int n = 0; n < questions.size(); ++n) {
		vector<DrtPred> tmp_drs;
		try {
			tmp_drs = string_to_drs_cast(questions.at(n));
		} catch (...) {
			continue;
		}
		drt tmp_drt(tmp_drs);
		string link = xml_substitutions_backward( questions_links.at(n) );
		string text = xml_substitutions_backward( questions_texts.at(n) );
		tmp_drt.setLink(link);
		tmp_drt.setText(text);
		questions_drt.push_back(tmp_drt);
		questions_.push_back(tmp_drt);
	}
	DrsPersonae dp_questions(questions_drt, "");
	dp_questions.compute();
	dp_questions.sort();
//   q_personae_.addPersonae(dp_questions);
	k_questions_.addPersonae(dp_questions);


	// increase the label of loaded wisdom
	++load_num_;

	//dp_questions.clear();
	//personae.clear();
	/// There should be a rules.clear();

	//puts("LOADED:::");
}

void Wisdom::writeFile(const string &str)
{
	std::ofstream file;
	file.open(str.c_str());
	file << this->writeString();
	file.close();
}

string Wisdom::writeString()
{
	std::stringstream file;

	file << "<?xml version=\"1.0\"?>" << endl;
	file << "<?nlulite version=\"0.1.12\"?>" << endl;
	file << "<wisdom>" << endl;

	//vector<drt> data = k_.getDrtList();
	std::deque<drt>::iterator diter= data_.begin();
	std::deque<drt>::iterator dend = data_.end();

	std::deque<drt> questions = questions_;
	std::deque<drt>::iterator qiter = questions_.begin();
	std::deque<drt>::iterator qend = questions_.end();

	std::deque<clause_vector>::iterator citer = clauses_.begin();
	std::deque<clause_vector>::iterator cend = clauses_.end();

	// write the data
	file << "<data>" << endl;
	for (; diter != dend; ++diter) {
		file << "<item>" << endl;
		file << "<predicates>" << endl;
		print_vector_stream(file, diter->predicates_with_references());
		file << endl;
		file << "</predicates>" << endl;
		file << "<link>" << endl;
		string link_str = diter->getLink();
		link_str = xml_substitutions(link_str);
		file << link_str << endl;
		file << "</link>" << endl;
		file << "<text>" << endl;
		string text_str = diter->getText();
		text_str = xml_substitutions(text_str);
		file << text_str << endl;
		file << "</text>" << endl;
		file << "</item>" << endl;
	}
	file << "</data>" << endl;

	// write the clauses
	file << "<clauses>" << endl;
	for (; citer != cend; ++citer) {
		std::stringstream ss;
		ss << *citer;
		string clause_string = ss.str();
		clause_string = substitute_string(clause_string, "&", "\\and");
		file << "<item>" << endl;
		file << "<predicates>" << endl;
		file << clause_string << endl;
		file << "</predicates>" << endl;
		file << "<link>" << endl;
		string link_str = citer->getLink();
		link_str = xml_substitutions(link_str);
		file << link_str << endl;
		file << "</link>" << endl;
		file << "<text>" << endl;
		string text_str = citer->getText();
		text_str = xml_substitutions(text_str);
		file << text_str << endl;
		file << "</text>" << endl;
		file << "<weight>" << endl;
		file << citer->getWeigth() << endl;
		file << "</weight>" << endl;
		file << "</item>" << endl;
	}
	file << "</clauses>" << endl;

	// write the questions
	file << "<questions>" << endl;
	for (; qiter != qend; ++qiter) {
		file << "<item>" << endl;
		file << "<predicates>" << endl;
		print_vector_stream(file, qiter->predicates_with_references());
		file << endl;
		file << "</predicates>" << endl;
		file << "<link>" << endl;
		string link_str = qiter->getLink();
		link_str = xml_substitutions(link_str);
		file << link_str << endl;
		file << "</link>" << endl;
		file << "<text>" << endl;
		string text_str = qiter->getText();
		text_str = xml_substitutions(text_str);
		file << text_str << endl;
		file << "</text>" << endl;
		file << "</item>" << endl;
	}
	file << "</questions>" << endl;

	file << "</wisdom>" << endl;

	return file.str();
}

typedef boost::tuple<string, string, string> RDFTriplet;

class RDFGraph {

	vector<RDFTriplet> asserted_bys_;
	vector<RDFTriplet> triplets_;

	map<string, string> compl_names_;

	string getComplementName(const string &str);

public:

	RDFGraph();

	void addDrs(const DrtVect &drtvect);
	vector<RDFTriplet> getAllTriplets() const;
};

RDFGraph::RDFGraph()
{
	asserted_bys_.push_back(boost::make_tuple((string) ":" + "none", "swp:assertedBy", (string) "_:" + "none"));

	compl_names_["@MOTION_TO"] = ":motionTo";
	compl_names_["@MOTION_FROM"] = ":motionFrom";
	compl_names_["@PLACE_AT"] = ":placeIn";
	compl_names_["@TIME_TO"] = ":timeTo";
	compl_names_["@TIME_FROM"] = ":timeFrom";
	compl_names_["@TIME_AT"] = ":timeAt";
	compl_names_["@TIME_DURATION"] = ":timeDuration";
	compl_names_["@DATIVE"] = ":to";
	compl_names_["@CAUSED_BY"] = ":by";
	compl_names_["@FOR"] = ":for";
	compl_names_["@GENITIVE"] = ":of";
	compl_names_["@TOPIC"] = ":about";
	compl_names_["@WITH"] = ":with";
	compl_names_["@EXCLUDING"] = ":without";
	compl_names_["@AFTER"] = ":after";
	compl_names_["@BEFORE"] = ":before";
	compl_names_["@MOTION_THROUGH"] = ":across";
	compl_names_["@MOTION_AGAINST"] = ":against";
	compl_names_["@TIME_THROUGH"] = ":across";
	compl_names_["@CLOCK_AT"] = ":timeAt";
	compl_names_["@OWNED_BY"] = ":of";
	compl_names_["@QUANTITY"] = ":ofQuantity";
	compl_names_["@QUANTIFIER"] = ":ofQuantity";
	compl_names_["@ALLOCUTION"] = ":speech";
	compl_names_["@SIZE"] = ":ofSize";
	compl_names_["@TIMES"] = ":ofTimes";
	compl_names_["@TIME"] = ":tense";
	compl_names_["@COMPARED_TO"] = ":than";
	compl_names_["@MORE"] = ":more";
	compl_names_["@LESS"] = ":less";
	compl_names_["@MORE_THAN"] = ":moreThan";
	compl_names_["@LESS_THAN"] = ":lessThan";
	compl_names_["@AND"] = ":and";
	compl_names_["@OR"] = ":or";
	compl_names_["@CONJUNCTION"] = ":and";
	compl_names_["@DISJUNCTION"] = ":but";
	compl_names_["@COORDINATION"] = ":or";
	compl_names_["@ACCORDING_TO"] = ":accordingTo";
	compl_names_["@AGE"] = ":ofAge";
}

string RDFGraph::getComplementName(const string &str)
{
	map<string, string>::iterator miter = compl_names_.find(str);
	if (miter != compl_names_.end()) {
		return miter->second;
	}
	return "of";
}

static bool has_subject(const DrtPred &pred)
{
	string subj = extract_subject(pred);
	if (subj.find("subj") == string::npos && subj.find("none") == string::npos && subj.find("subj") == string::npos // The passive verbs invert the tags
			)
		return true;
	return false;
}

static bool has_object(const DrtPred &pred)
{
	string obj = extract_object(pred);
	if (obj.find("obj") == string::npos && obj.find("none") == string::npos && obj.find("subj") == string::npos // The passive verbs invert the tags
			)
		return true;
	return false;
}

void RDFGraph::addDrs(const DrtVect &drtvect)
{
	// extract the references and put them into "asserted_bys_"
	for (int n = 0; n < drtvect.size(); ++n) {
		if (drtvect.at(n).is_name() || drtvect.at(n).is_PRP() || drtvect.at(n).is_verb()) {
			string name, ref;
			name = extract_header(drtvect.at(n));
			ref = extract_first_tag(drtvect.at(n));
			asserted_bys_.push_back(boost::make_tuple((string) ":" + name, "swp:assertedBy", (string) "_:" + ref));
		}
	}

	// extract the semantic information and translate into triplets
	for (int n = 0; n < drtvect.size(); ++n) {
		if (drtvect.at(n).is_verb()) {
			string vref, subj, obj;
			vref = extract_first_tag(drtvect.at(n));
			subj = extract_subject(drtvect.at(n));
			obj = extract_object(drtvect.at(n));
			if (!has_subject(drtvect.at(n)))
				subj = "none";
			if (!has_object(drtvect.at(n)))
				obj = "none";
			triplets_.push_back(boost::make_tuple((string) "_:" + subj, (string) "_:" + vref, (string) "_:" + obj));
		}
		if (drtvect.at(n).is_complement()) {
			string header, fref, sref, prefix;

			prefix = "_";
			header = extract_header(drtvect.at(n));

			if (header == "@TIME" || header == "@MODAL" || header == "@QUANTIFIER")
				prefix = "";

			fref = extract_first_tag(drtvect.at(n));
			sref = extract_second_tag(drtvect.at(n));

			string compl_name = this->getComplementName(header);

			triplets_.push_back(boost::make_tuple((string) "_:" + fref, compl_name, prefix + ":" + sref));
		}

	}
}

vector<RDFTriplet> RDFGraph::getAllTriplets() const
{
	vector<RDFTriplet> to_return;
	to_return.insert(to_return.end(), asserted_bys_.begin(), asserted_bys_.end());
	to_return.insert(to_return.end(), triplets_.begin(), triplets_.end());
	return to_return;
}

string Wisdom::writeStringRDF()
{
	std::stringstream file;

	file << "@prefix rdf: <http://www.w3.org/1999/02/22-rdf-syntax-ns#> ." << endl;
	file << "@prefix xsd: <http://www.w3.org/2001/XMLSchema#> ." << endl;
	file << "@prefix swp: <http://www.w3.org/2004/03/trix/swp-1/> ." << endl;
	file << "@prefix dc: <http://purl.org/dc/elements/1.1/> ." << endl;
	file << "@prefix ex: <http://www.example.org/vocabulary#> ." << endl;
	file << endl;

	map<string, RDFGraph> graph_map;
	vector<drt> data = k_.getDrtList();


	// write the data
	for (int n = 0; n < data.size(); ++n) {
		string link = data.at(n).getLink();
		DrtVect drtvect = data.at(n).predicates_with_references();
		graph_map[link].addDrs(drtvect);
	}

	map<string, RDFGraph>::const_iterator giter = graph_map.begin(), gend = graph_map.end();
	int m = 0;
	for (; giter != gend; ++giter, ++m) {
		string graph_name = ":G" + boost::lexical_cast<string>(m);
		vector<RDFTriplet> triplets = giter->second.getAllTriplets();
		file << graph_name << " {" << endl;
		for (int n = 0; n < triplets.size(); ++n) {
			file << "\t" << triplets.at(n).get<0>() << "\t" << triplets.at(n).get<1>() << "\t" << triplets.at(n).get<2>()
					<< " " << "." << endl;
		}
		file << "}" << endl;
	}
	giter = graph_map.begin(), gend = graph_map.end();

	file << endl;
	file << ":links {" << endl;
	for (m = 0; giter != gend; ++giter, ++m) {
		string graph_name = ":G" + boost::lexical_cast<string>(m);
		string triplets_link = giter->first;
		if (triplets_link == "")
			triplets_link = "http://";
		file << "\t" << graph_name << "\t" << "swp:link" << "\t" << triplets_link << " " << "." << endl;
	}
	file << "}" << endl;

	return file.str();
}

void Wisdom::setKnowledge(const Knowledge &k)
{
	k_ = k;
}


void Wisdom::setWisdomInfo(const WisdomInfo &wi)
{
	wi_= wi;
	k_.setWisdomInfo(wi);
}
