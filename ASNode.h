/*
 * ASNode.h
 *
 *  Created on: Feb 17, 2017
 *      Author: vadim
 */

#pragma once

#include "Generator.h"
#include "Events.h"
#include <map>
#include <vector>
#include <set>

using std::map;
using std::vector;
using std::set;

class ASNode {
public:
	ASNode(int _id, Generator<double>* _gen, bool _involved);

	void SetInvolvedNodes(set<int> nodes) {
		involved_nodes = nodes;
	}

	vector<Event*> process_event(Event* evnt, double world_time);

	vector<Event*> init_event(double world_time);

	vector<std::pair<double, double>>& get_delays() {
		return delays;
	}

	int delays_size() {
		return delays.size();
	}

	virtual ~ASNode();

private:

	void build_routing(bool build_weighted=true, bool build_payload=true);
	void get_direct_connections();
	double get_bandwidth(TransitEvent* evnt);

	vector<Event*> process_send_event(SendEvent* evnt, double world_time);
	vector<Event*> process_transit_event(TransitEvent* evnt, double world_time);
	vector<Event*> process_quality_event(QualityRefreshEvent* evnt, double world_time);
	vector<Event*> process_trading_event(TradingQualityEvent* evnt, double world_time);

	int id;

	vector<vector<int>> routing_table;
	vector<vector<int>> trading_routing_table;
	set<int> involved_nodes;

	Generator<double>* generator;

	double process_mean;
	double process_std;

	double sending_mean;
	double sending_std;

	double qos_refresh_lambda;

	double qos_mean;
	double qos_std;

	double trading_refresh_lambda;

	double trading_mean;
	double trading_std;

	double trading_qos_mean;
	double trading_qos_std;

	double local_time;

	bool involved;

	vector<std::pair<double, double>> delays;
	set<int> direct_connections;
	map<int, double> avg_delay_to_direct;
	map<int, int> packets_sent;
	map<int, double> to_port_local_time;
};
