/*
 * Simulator.h
 *
 *  Created on: Feb 17, 2017
 *      Author: vadim
 */

#pragma once

#include "Generator.h"
#include "Events.h"
#include "ASNode.h"
#include <queue>
#include <vector>
#include <map>

using std::priority_queue;
using std::vector;
using std::map;

class EventCompare {
public:
	bool operator()(Event* f, Event* s);
};

class Simulator {
public:
	Simulator(Generator<double>* gen, int involved);

	void perform_simulation(double max_time, int max_events = -1);

	vector<std::pair<double, double>> collect_delays();

	virtual ~Simulator();
private:

	double world_time;
	Generator<double>* generator;

	map<int, ASNode*> entites_map;

	priority_queue<Event*, vector<Event*>, EventCompare> event_queue;

	void push_vector(vector<Event*> events);
};
