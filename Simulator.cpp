/*
 * Simulator.cpp
 *
 *  Created on: Feb 17, 2017
 *      Author: vadim
 */

#include "Simulator.h"
#include "Generator.h"
#include "Events.h"
#include "ASNode.h"
#include "Utils.h"
#include <queue>
#include <vector>
#include <map>
#include <iostream>
#include <iomanip>
#include <ctime>
#include <chrono>
#include <set>

using std::priority_queue;
using std::vector;
using std::map;
using std::clock_t;
using std::clock;
using std::set;

bool EventCompare::operator()(Event* f, Event* s) {
	return f->scheduled_time > s->scheduled_time;
}

Simulator::Simulator(Generator<double>* gen, int involved) :
	world_time(0),
	generator(gen)
{
#ifdef PRINT
	std::cout << "Entities creation\n";
	std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
#endif
	Graph<double> *graph = generator->GetGraph();
	set<int> involved_nodes = generator->generate_involved(involved);
	unsigned int total = graph->NodesSize();
	for (unsigned int i = 0; i < total; i++) {
#ifdef PRINT
		std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
		int time_left = ((std::chrono::duration_cast<std::chrono::seconds>(end - start).count())) *
				(total - i - 1) / (i + 1);
		int min = time_left / 60;
		int sec = time_left % 60;
		float perc_val = int(10000.0 * (i + 1) / total) / 100.0;
		std::cout << "\rEntity "
				<< std::setw(3) << i + 1 << " out of "
				<< std::setw(6) << total
				<< "("
				<< std::setw(5) << perc_val << "%)" << " creation"
				<< " Estimated time left :"
				<< std::setw(3) << min << " min"
				<< std::setw(3) <<sec << " sec";
#endif
		bool involve = involved_nodes.find(i) != involved_nodes.end();
		ASNode* new_node = new ASNode(i, generator, involve);
		if (involve) {
			new_node->SetInvolvedNodes(involved_nodes);
		}
		entites_map.insert(std::pair<int, ASNode*>(i, new_node));
		push_vector(new_node->init_event(world_time));
	}
#ifdef PRINT
	std::cout << "\nDone!\n";
#endif
}

void Simulator::perform_simulation(double max_time, int max_events) {
#ifdef PRINT
	double one_percent = max_time / 10000;
	int current_percentage = 0;
	std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
#endif
	int events_total = 0;
	while (world_time < max_time && (max_events < 0 || events_total < max_events)) {
		Event* gathered_event = event_queue.top();
		events_total++;
		event_queue.pop();
		ASNode* executor =	entites_map[gathered_event->executor];
		if (gathered_event->scheduled_time < world_time) {
			assert(false);
		}
		world_time = gathered_event->scheduled_time;
#ifdef PRINT
		std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
		//if (world_time - last_value > one_percent) {
		double events_percentage = 0;
		if (max_events > 0) {
			events_percentage =	100 * events_total / max_events;
		}
		current_percentage = std::max(world_time / one_percent / 100, events_percentage);
		string domination = "Events";
		if (events_percentage < current_percentage) {
			domination = "Time";
		};
		int time_spent = std::chrono::duration_cast<std::chrono::seconds>(end - begin).count();
		int min = time_spent / 60;
		int sec = time_spent % 60;
		int time_left = 0;
		if (current_percentage != 0) {
			time_left = time_spent * (100 - current_percentage) / current_percentage;
		}
		int min_left = time_left / 60;
		int sec_left = time_left % 60;
		std::cout << "\r" << "Executed : "
			<< std::setw(3) << current_percentage << "%"
			<< " Queue length : "
			<< std::setw(5) << event_queue.size()
			<< " Events total : "
			<< std::setw(10) << events_total
			<< " SimTime : "
			<< std::setw(5) << int(world_time * 1000) / 1000.0
			<< " Time Spent/Left :"
				<< std::setw(3) << min << "min"
				<< std::setw(3) << sec << "sec"
				<< " /"
				<< std::setw(3) << min_left << "min"
				<< std::setw(3) << sec_left << "sec";
#endif
		vector<Event*> reply = executor->process_event(gathered_event, world_time);
		push_vector(reply);
	}
#ifdef PRINT
	std::cout << "\nFinished!\n";
#endif
}

void Simulator::push_vector(vector<Event*> events) {
	for (unsigned int i = 0; i < events.size(); i++) {
		Event* new_event = events[i];
		event_queue.push(new_event);
	}
}

vector<std::pair<double, double>> Simulator::collect_delays() {
	vector<std::pair<double, double>> result;
	unsigned long int total_space = 0;
	for (auto& entity : entites_map) {
		ASNode* node = entity.second;
		total_space +=	node->delays_size();
	}
	result.reserve(total_space);
	for (auto& entity : entites_map) {
		ASNode* node = entity.second;
		vector<std::pair<double, double>> &delays = node->get_delays();
		result.insert(result.end(), delays.begin(), delays.end());
	}
	return result;
}

Simulator::~Simulator() {
	// TODO Auto-generated destructor stub
}

