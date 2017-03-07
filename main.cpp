#include "Utils.h"
#include <vector>
#include "Generator.h"
#include "Simulator.h"
#include <iostream>
#include <vector>
#include <fstream>
#include <string>
using namespace std;
#define OUTPUT
int main(int argc, char** argv) {

	int seed = stoi(argv[1]);
	int involved = stoi(argv[2]);
	double max_time = stod(argv[3]);
	string graph_filename = argv[4];
	Generator<double> gen(seed);
	int multiplier = -1;
	Graph<double> *graph = gen.generate_graph_from_file(graph_filename);
	involved = std::min(involved, graph->NodesSize());
	Simulator sim(&gen, involved);
	sim.perform_simulation(max_time);
	vector<std::pair<double, double>> delays = sim.collect_delays();
#ifdef OUTPUT
	ofstream delays_file;
	delays_file.open("sim_result.sr");
#endif
	//std::cout << delays.size() << "\n";
	double mean = 0;
	for (unsigned int i = 0; i < delays.size(); i++) {
		//std::cout << mean << "\n";
		mean += delays[i].first;
#ifdef OUTPUT
		delays_file << delays[i].first << " " << delays[i].second << endl;
#endif
	}
#ifdef OUTPUT
	delays_file.close();
#endif
#ifdef PRINT
	cout << "File done!\n";
#endif
	cout << mean / delays.size() << endl;
	return 0;
}
