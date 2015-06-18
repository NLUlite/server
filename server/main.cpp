#include<iostream>
#include<algorithm>
#include<vector>
#include<map>
#include<string>
#include<cstdlib>
#include"../drt/drt_collection.hpp"
#include"../knowledge/Knowledge.hpp"
#include"../wisdom/Wisdom.hpp"
#include"WisdomServer.hpp"
#include<boost/algorithm/string/split.hpp>
#include<boost/program_options.hpp>
#include<stdexcept>
#include<fcntl.h>

using std::string;
using std::vector;
using std::map;
using boost::tuple;
using boost::make_tuple;
using std::pair;
using std::make_pair;

const bool commercial_version = true;

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

int main(int argc, char **argv)
{
	namespace po = boost::program_options;
	po::options_description description("Options");

	std::cerr << "Starting NLUlite server version 0.1.12" << endl;
	if( !commercial_version ) {
		std::cerr << "This is the non commercial (single threaded) version." 
				<< endl;
	}
	std::cerr << endl;
	    

	int port = 4001;
	string home_dir = getenv("HOME");
	string dir = home_dir + "/NLUlite/data/";
	int nthreads = 1;
	bool wikidata= false;

	description.add_options()
		("help,h", "Print this help")
		("port,p",     po::value<int>(&port),     	"The port where the server listens"   )
		("nthreads,n", po::value<int>(&nthreads), 	"The number of threads for the server")
		("dir,d",      po::value<string>(&dir),	  	"The directory where to find the data")
		("wikidata,w", 						"Run the server as a wikidata proxy")
		;

	po::variables_map vm;
	try {
		po::store(po::parse_command_line(argc, argv, description), vm);
		if (vm.count("help")) {
			std::cerr << "NLUlite v0.1.12 command line arguments:"
					<< std::endl << std::endl << description << std::endl;
			return 0;
		}
		if (vm.count("port")) {
			port = vm["port"].as<int>();
			std::cerr << "Running NLUlite from port " << port << std::endl
					<< std::endl;
		}
		if (vm.count("dir")) {
			dir = vm["dir"].as<string>();
			std::cerr << "Reading data files from " << dir << std::endl
					<< std::endl;
		}
		if (vm.count("wikidata")) {
			std::cerr << "Running the server as a Wikidata proxy (requires an internet connection)" << std::endl
					<< std::endl;
			wikidata= true;
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

		po::notify(vm);
	} catch (po::error& e) {
		std::cerr << "Error: " << e.what() << std::endl << std::endl;
		std::cerr << description << std::endl;
		return -1;
	}

////
// fork into a deamon
//	pid_t pid, sid;
//
//	//Fork the Parent Process
//	pid = fork();
//
//	if (pid < 0) {
//		exit(EXIT_FAILURE);
//	}
//
//	//We got a good pid, Close the Parent Process
//	if (pid > 0) {
//		exit(EXIT_SUCCESS);
//	}
//
//	std::cerr << "\n\n\n *** The server is now a deamon. You can stop it by writing \'killall -v NLUlite-server\'. ***\n\n\n" << endl;
////

	// start the server
	WisdomServer wserver(port, dir, nthreads, wikidata);
	wserver.run();

	return 0;
}
