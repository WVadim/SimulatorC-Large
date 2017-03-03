/*
 * Generator.h
 *
 *  Created on: Feb 10, 2017
 *      Author: vadim
 */

#define NODELETE
#undef MEASURE

#include "Utils.h"

#include <random>
#include <iostream>
#include <string>
#include <stdlib.h>
#include <fstream>
#include <iostream>
#include <set>
#include <vector>
#ifdef MEASURE
#include <ctime>
#endif

using std::string;
using std::to_string;
using std::ifstream;
using std::cout;
using std::endl;
using std::stoi;
using std::set;
using std::vector;
using std::pair;

#ifdef MEASURE
using std::clock_t;
using std::clock;
#endif

#pragma once

using std::default_random_engine;
using std::normal_distribution;
using std::weibull_distribution;
using std::exponential_distribution;
using std::uniform_real_distribution;
using std::uniform_int_distribution;
using std::string;

template<typename PayloadType>
class Generator {
public:
	Generator(int _seed);

	double generate_normal(double mean = 0, double std = 1);
	double generate_weibull(double mean = 0, double std = 1);
	double generate_exponential(double mean = 1);
	double generate_uniform(double low, double high);
	int generate_uniform_int(int low, int high);
	//Fast interfaces, basically deprecated
	Graph<PayloadType>* generate_graph_newmann(int size, int min_connectivity, double edge_probability);
	Graph<PayloadType>* generate_graph_powerlaw(int size, int random_edges, double triangle_probability);
	Graph<PayloadType>* generate_graph_simple_loop(int size);
	Graph<PayloadType>* generate_graph_from_file(string filename);
	// Possible types :
	// newmann for Newmann-Watts-Strogatz graph model
	// power for Power-Law graph
	// loop for lcycle graph
	// Binded for new models
	set<int> generate_involved(int involved_size);
	Graph<PayloadType>* generate_graph(string type, int size, int int_param, double double_param);

	Graph<PayloadType>* GetGraph() {
		return graph;
	}

	virtual ~Generator();
private:

	void execute_command(string command);

	default_random_engine engine;
	int seed;
	Graph<PayloadType>* graph;
};


/*
 * Generator.cpp
 *
 *  Created on: Feb 10, 2017
 *      Author: vadim
 */

template<typename PayloadType>
Generator<PayloadType>::Generator(int _seed) :
	engine(_seed),
	seed(_seed),
	graph(NULL) {
	// TODO Auto-generated constructor stub
		engine.seed(seed);
	}

template<typename PayloadType>
double Generator<PayloadType>::generate_normal(double mean, double std) {
	return normal_distribution<double>(mean, std)(engine);
}

template<typename PayloadType>
double Generator<PayloadType>::generate_weibull(double mean, double std) {
	return weibull_distribution<double>(mean, std)(engine);
}

template<typename PayloadType>
double Generator<PayloadType>::generate_exponential(double mean) {
	return exponential_distribution<double>(mean)(engine);
}

template<typename PayloadType>
double Generator<PayloadType>::generate_uniform(double low, double high) {
	return uniform_real_distribution<double>(low, high)(engine);
}

template<typename PayloadType>
int Generator<PayloadType>::generate_uniform_int(int low, int high) {
	return uniform_int_distribution<int>(low, high-1)(engine);
}
//python graph_generator.py -t=power -n=100 -s=190 -m=3 -p=0.01 -f=test

template<typename PayloadType>
Graph<PayloadType>* Generator<PayloadType>::generate_graph_newmann(int size, int min_connectivity, double edge_probability) {
	string call_command = string("python graph_generator.py -t=newmann") + string(" -n=") + to_string(size) +
			string(" -s=") + to_string(seed) + string(" -m=") + to_string(min_connectivity) +
			string(" -p=") + to_string(edge_probability) + string(" -f=tmp_grph_file.grph");
	execute_command(call_command);
	Graph<PayloadType>* result = generate_graph_from_file("tmp_grph_file.grph");
#ifndef NODELETE
	execute_command("rm -f tmp_grph_file.grph");
#endif
	return result;
}

template<typename PayloadType>
Graph<PayloadType>* Generator<PayloadType>::generate_graph_powerlaw(int size, int random_edges, double triangle_probability) {
	string call_command = string("python graph_generator.py -t=power") + string(" -n=") + to_string(size) +
		string(" -s=") + to_string(seed) + string(" -m=") + to_string(random_edges) +
		string(" -p=") + to_string(triangle_probability) + string(" -f=tmp_grph_file.grph");
	execute_command(call_command);
#ifdef MEASURE
	clock_t start = clock();
#endif
	Graph<PayloadType>* result = generate_graph_from_file("tmp_grph_file.grph");
#ifdef MEASURE
	clock_t end = clock();
	cout << double(end - start) / CLOCKS_PER_SEC << endl;
#endif

#ifndef NODELETE
	execute_command("rm -f tmp_grph_file.grph");
#endif
	return result;
}

template<typename PayloadType>
Graph<PayloadType>* Generator<PayloadType>::generate_graph_simple_loop(int size) {
	string call_command = string("python graph_generator.py -t=loop") + string(" -n=") + to_string(size) +
			string(" -s=") + to_string(seed) + string(" -m=") + string(" -f=tmp_grph_file.grph");
	execute_command(call_command);
	Graph<PayloadType>* result = generate_graph_from_file("tmp_grph_file.grph");
#ifndef NODELETE
	execute_command("rm -f tmp_grph_file.grph");
#endif
	return result;
}

template<typename PayloadType>
Graph<PayloadType>* Generator<PayloadType>::generate_graph(string type, int size, int int_param, double double_param) {
	string call_command = string("python graph_generator.py -t=") + type + string(" -n=") + to_string(size) +
			string(" -s=") + to_string(seed) + string(" -m=") + to_string(int_param) +
			string(" -p=") + to_string(double_param) + string(" -f=tmp_grph_file.grph");
	cout << call_command << endl;
#ifdef MEASURE
	{
	clock_t start = clock();
#endif
	execute_command(call_command);
#ifdef MEASURE
	clock_t end = clock();
	cout << double(end - start) / CLOCKS_PER_SEC << endl;
	}
#endif
#ifdef MEASURE
	clock_t start = clock();
#endif
	cout << "Reading from file...\n";
	Graph<PayloadType>* result = generate_graph_from_file("tmp_grph_file.grph");
#ifdef MEASURE
	clock_t end = clock();
	cout << double(end - start) / CLOCKS_PER_SEC << endl;
#endif
	cout << "Done!\n";
#ifndef NODELETE
	execute_command("rm -f tmp_grph_file.grph");
#endif
	return result;
}

template<typename PayloadType>
void Generator<PayloadType>::execute_command(string command) {
	int status = std::system(command.c_str());
	assert(!status);

}

template<typename PayloadType>
Graph<PayloadType>* Generator<PayloadType>::generate_graph_from_file(string filename) {
	ifstream graph_file(filename);
	set<int> nodes;
	vector<pair<int, int>> edges;
	if (graph_file.is_open()) {
		string line;
		int edge = 0;
		pair<int, int> new_edge;
		while ( getline(graph_file, line) ) {
			int new_node = stoi(line);
			nodes.insert(new_node);
			if (!edge) {
				new_edge.first = new_node;
			} else {
				new_edge.second = new_node;
			}
			if (edge) {
				edges.push_back(new_edge);
			}
			edge ^= 1;
		}
	} else {
		assert(false);
	}
	graph_file.close();
	Graph<PayloadType> *graph = new Graph<PayloadType>(nodes.size(), true, true);
	for (pair<int, int> edge : edges) {
		graph->AddEdge(edge.first, edge.second, 1, 1);
		graph->AddEdge(edge.second, edge.first, 1, 1);
	}
	this->graph = graph;
	return graph;
}

template<typename PayloadType>
set<int> Generator<PayloadType>::generate_involved(int involved_size) {
	vector<int> graph_nodes;
	for (int i = 0; i < graph->NodesSize(); i++) {
		graph_nodes.push_back(i);
	}
	set<int> nodes_to_involve;
	for (int i = 0; i < involved_size; i++) {
		int index = generate_uniform_int(0, graph_nodes.size());
		nodes_to_involve.insert(graph_nodes[index]);
		graph_nodes.erase(graph_nodes.begin() + index);
	}
	//return set<int>();
	return nodes_to_involve;
}

template<typename PayloadType>
Generator<PayloadType>::~Generator() {
	// TODO Auto-generated destructor stub
}
