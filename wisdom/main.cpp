#include<iostream>
#include<algorithm>
#include<vector>
#include<map>
#include<string>
#include"../drt/drt_collection.hpp"
#include"../knowledge/Knowledge.hpp"
#include"Wisdom.hpp"
#include<boost/algorithm/string/split.hpp>
#include<boost/program_options.hpp>
#include<stdexcept>
#include<fstream>

using std::string;
using std::vector;
using std::map;
using boost::tuple;
using boost::make_tuple;
using std::pair;
using std::make_pair;


const bool debug= false;
const bool commercial_version = true;


template <class T>
static void print_vector(std::vector<T> &vs)
{
     if(vs.size()) {
	  typename vector<T>::iterator tags_iter= vs.begin();
	  while ( tags_iter != vs.end() ) {
	       std::cout << (*tags_iter) << " ";
	       ++ tags_iter;
	  }
	  std::cout << std::endl;
     }
}

template <class T>
static void print_vector_return(std::vector<T> &vs)
{
     typename vector<T>::iterator tags_iter= vs.begin();
     while ( tags_iter != vs.end() ) {
	  std::cout << (*tags_iter) << endl;
	  ++ tags_iter;
     }
     std::cout << std::endl;
}


static string get_link(string line)
{
	line.erase(0, 2);
	int pos_end = line.find("%]");
	line.erase(pos_end, 2);
     
	string to_return = line;

	return to_return;
}

static vector<drt_collection> load_discourses(const char *filename, int global_num, PhraseInfo *pi, Wisdom *w)
{
     std::ifstream file;
     file.open(filename); /// Check the filename exists !!!

	vector<drt_collection> to_return;
	string data = "";
	string link, old_link;
	char c_line[50000];


	bool first = true;
	while (file.good()) {
		file.getline(c_line, 50000);
		string line(c_line);
		if (line.find("[%") != string::npos
				&& line.rfind("%]") != string::npos) {
			link = get_link(line);
			if (debug) {
				cout << "DESCRIPTION::: " << link << endl;
			}
			if (data.size()) {
			     drt_collection *dc;
			     if (link.size()) {
				  dc = new drt_collection(data, global_num++, Context(), pi, old_link);
			     } else {
				  dc= new drt_collection(data, global_num++, Context(), pi);
			     }
			     w->addDiscourse(*dc);
			     delete dc;
			     data = "";
			     old_link = link;
			}
		} else {
			data.append(line + "\n");
			old_link = link;
		}
	}
	if (data.size()) {
	     drt_collection dc(data, global_num, Context(), pi, link);
		if (debug) {
			cout << "DESCRIPTION2::: " << link << endl;
		}
		w->addDiscourse(dc);
		//to_return.push_back(dc);
	}

	file.close();

	return to_return;
}


int main(int argc, char **argv)
{     
	namespace po = boost::program_options;
	po::options_description description("Options");

	std::cerr << "Starting NLUlite off-line wisdomizer version 0.1.12" << endl;
	if( !commercial_version ) {
		std::cerr << "This is the non commercial (single threaded) version." 
				<< endl;
	}
	std::cerr << endl;
     	 
     string input = "";
     string output = "";
     int num= 1;

     // The home directory
     string home_dir_ = getenv("HOME");
     
     Parameters *ps = parameters_singleton::instance();
     string dir = home_dir_ + "/NLUlite/data";
	int nthreads = 1;


	description.add_options()
		("help,h", "Print this help")
		("nthreads,n", po::value<int>(&nthreads),  "The number of threads for the server")
		("dir,d",      po::value<string>(&dir),    "The directory where to find the data")
		("input,i",    po::value<string>(&input),  "The input file")
		("output,o",   po::value<string>(&output), "The output file")
		("num,n",      po::value<int>(&num),	   "The starting number of wisdom labels")
		;

	po::variables_map vm;
	try {
		po::store(po::parse_command_line(argc, argv, description), vm);
		if (vm.count("help")) {
			std::cerr << "NLUlite v0.0.7 command line arguments:"
					<< std::endl << std::endl << description << std::endl;
			return 0;
		}
		if (vm.count("dir")) {
			dir = vm["dir"].as<string>();
			std::cerr << "Reading data files from " << dir << std::endl
					<< std::endl;
		}
		if (vm.count("input")) {
			input = vm["input"].as<string>();
			std::cerr << "The input file is " << input << std::endl
					<< std::endl;
		}
		if (vm.count("output")) {
			output = vm["output"].as<string>();
			std::cerr << "The output file is " << output << std::endl
					<< std::endl;
		}
		if (vm.count("nthreads")) {
			nthreads = vm["nthreads"].as<int>();
			if(nthreads <= 0 ) nthreads = 1;
			if( !commercial_version) {
				nthreads = 1;
				std::cerr << "The number of threads is still 1, please buy the commercial version for mutithreading." << std::endl
						<< std::endl;
			} else {
				std::cerr << "The number of threads is " << nthreads << std::endl
						<< std::endl;
			}
		}
		if (vm.count("num")) {
			num = vm["num"].as<int>();
			std::cerr << "The starting number for Wisdom labels " << num << std::endl
					<< std::endl;
		}
		po::notify(vm);
	} catch (po::error& e) {
		std::cerr << "Error: " << e.what() << std::endl << std::endl;
		std::cerr << description << std::endl;
		return -1;
	}


     if(input == "" ) {
		std::cerr << "Input file not specified" << endl;
		return -1;
     }
     if(output == "") {
		std::cerr << "Output file not specified" << endl;
		return -1;
     }

	std::ifstream ifile;
	ifile.open(input.c_str());
	if(!ifile) {
		std::cerr << "Cannot open the input file \'" << input << "\'." << endl << endl;
		return -1;
	}
	ifile.close();


     ps->setDir(dir);
     ps->setNumThreads(nthreads); // the default number of threads

     PhraseInfo pi;    

     Wisdom w;
     w.setPhraseInfo(&pi);
     load_discourses( input.c_str() , num, &pi, &w);
     w.writeFile( output.c_str() );
     
     return 0;
}
