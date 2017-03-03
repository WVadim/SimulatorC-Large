/*
 * ASNode.cpp
 *
 *  Created on: Feb 17, 2017
 *      Author: vadim
 */

#include "ASNode.h"
#include "Utils.h"
#include "Events.h"

#include <vector>
#include <cmath>

using std::vector;
using std::pair;
using std::abs;

static map<int, int> build_table(vector<vector<int>> paths, int me) {
	map<int, int> table;
	for (unsigned int i = 0; i < paths.size(); i++) {
		vector<int> path = paths[i];
		if (path.size() == 0) {
			assert(me == i);
			table.insert(pair<int, int>(me, me));
			continue;
		}
		int dst = path[path.size() - 1];
		int next_hop = path[0];
		table.insert(pair<int, int>(dst, next_hop));
	}
	return table;
}

ASNode::ASNode(int _id, Generator<double>* _gen, bool _involved):
	id(_id),
	generator(_gen),
	process_mean(1024000),
	process_std(102400),
	sending_mean(1.0/100),
	sending_std(1.0/100),
	qos_refresh_lambda(1),
	qos_mean(1024000),
	qos_std(102400),
	trading_refresh_lambda(0.1),
	trading_mean(1024000 * 2),
	trading_std(102400 * 2),
	trading_qos_mean(1024000 * 2),
	trading_qos_std(102400 * 2),
	local_time(0),
	involved(_involved) {
	build_routing();
	get_direct_connections();
}

void ASNode::get_direct_connections() {
	auto graph = generator->GetGraph();
	vector<vector<int>> paths = graph->ShortestPath(id, false);
	for (unsigned int i = 0; i < paths.size(); i++) {
		if (paths[i].size() == 1) {
			direct_connections.insert(paths[i][0]);
			packets_sent.insert(pair<int, int>(i, 0));
			avg_delay_to_direct.insert(pair<int, double>(i, 0));
			to_port_local_time.insert(pair<int, double>(i, 0));
		}
	}
}

void ASNode::build_routing(bool build_weighted, bool build_payload) {
	auto graph = generator->GetGraph();
	if (build_payload) {
		vector<vector<int>> uw_paths = graph->ShortestPath(id, false, true);
		routing_table = uw_paths;//build_table(uw_paths, id);
	}
	if (involved and build_weighted) {
		vector<vector<int>> w_paths = graph->ShortestPath(id, true, true);
		trading_routing_table = w_paths;//build_table(w_paths, id);
	}
}

vector<Event*> ASNode::init_event(double world_time) {
	vector<Event*> reply;
	int dst = id;
	while (dst == id)
		dst = generator->generate_uniform_int(0, generator->GetGraph()->NodesSize());
	SendEvent* init_event = new SendEvent(0, id, dst, 1);
	reply.push_back(init_event);
	QualityRefreshEvent* qos_event = new QualityRefreshEvent(world_time, id);
	reply.push_back(qos_event);
	TradingQualityEvent* transit_qos_event = new TradingQualityEvent(world_time, id);
	reply.push_back(transit_qos_event);
	return reply;
}

vector<Event*> ASNode::process_event(Event* evnt, double world_time) {
	TransitEvent* transit = dynamic_cast<TransitEvent*>(evnt);
	if (transit) {
		return process_transit_event(transit, world_time);
	}
	SendEvent* send = dynamic_cast<SendEvent*>(evnt);
	if (send) {
		return process_send_event(send, world_time);
	}
	QualityRefreshEvent* qos = dynamic_cast<QualityRefreshEvent*>(evnt);
	if (qos) {
		return process_quality_event(qos, world_time);
	}
	TradingQualityEvent* trading = dynamic_cast<TradingQualityEvent*>(evnt);
	if (trading) {
		return process_trading_event(trading, world_time);
	}
	return vector<Event*>();
}

vector<Event*> ASNode::process_send_event(SendEvent* evnt, double world_time) {
	vector<Event*> reply;
	TransitEvent* transit = new TransitEvent(evnt->scheduled_time, evnt->src, evnt->dst, evnt->size);
	vector<int> next_hop;
	if (involved and involved_nodes.find(evnt->dst) != involved_nodes.end()) {
		next_hop = trading_routing_table[evnt->dst];
	} else {
		next_hop = routing_table[evnt->dst];
	}
	transit->executor = next_hop[0];
	transit->routing = next_hop;
	transit->scheduled_time += transit->size / get_bandwidth(transit);
	reply.push_back(transit);
	double next_traffic_send = std::min(abs(generator->generate_weibull(sending_mean, sending_std)),
			1.5 * sending_mean);
	next_traffic_send = std::max(next_traffic_send, sending_mean / 2);
	next_traffic_send += world_time;
	int packet_size = 64;
	int target = id;
	while (target == id)
		target = generator->generate_uniform_int(0, generator->GetGraph()->NodesSize());
	*evnt = SendEvent(next_traffic_send, id, target, packet_size);
	reply.push_back(evnt);
	return reply;
}

vector<Event*> ASNode::process_transit_event(TransitEvent* evnt, double world_time) {
	vector<Event*> reply;
	if (evnt->dst == id) {
		if (evnt->src != id) {
			if (direct_connections.find(evnt->src) != direct_connections.end()) {
				if (avg_delay_to_direct.count(evnt->src) == 0) {
					avg_delay_to_direct.insert(pair<int, double>(evnt->src, 0));
					packets_sent.insert(pair<int, int>(evnt->src, 0));
				}
				double current_delay = avg_delay_to_direct.at(evnt->src);
				int events_checked = packets_sent.at(evnt->src);
				current_delay =
						(current_delay * double(events_checked) + world_time - evnt->creation_time)
								/ (events_checked + 1);
				auto current_it = avg_delay_to_direct.find(evnt->src);
				current_it->second = current_delay;
				auto packets_it = packets_sent.find(evnt->src);
				packets_it->second = events_checked + 1;
			}
			if (world_time > 0.001) {
				double process_time = 0;//evnt->size / get_bandwidth(evnt);
				double delay = world_time + process_time - evnt->creation_time;
				delays.push_back(std::pair<double, double>(delay, world_time));
				if (direct_connections.find(evnt->src) != direct_connections.end()) {
					auto packets_iter = packets_sent.find(evnt->src);
					int packets = packets_iter->second;
					auto local_delay_iter = avg_delay_to_direct.find(evnt->src);
					double local_delay = local_delay_iter->second;
					if (packets)
						local_delay = (local_delay + delay) * double(packets) / (packets + 1);
					else
						local_delay = delay;
					packets_iter->second++;
					local_delay_iter->second =local_delay;
				}
			}
		}
		delete evnt;
		return reply;
	}
	int next_hop = evnt->routing[0];
	double bandwidth = get_bandwidth(evnt);
	evnt->executor = next_hop;
	evnt->routing.erase(evnt->routing.begin());
	double execution_time = evnt->size / (1 + bandwidth);
	//local_time = std::max(world_time, local_time) + execution_time;
	auto hop_data = to_port_local_time.find(next_hop);
	evnt->scheduled_time = std::max(hop_data->second, world_time) + execution_time;
	hop_data->second = evnt->scheduled_time;
	reply.push_back(evnt);
	return reply;
}

double ASNode::get_bandwidth(TransitEvent* evnt) {
	double bandwidth = 1;
	if (involved and involved_nodes.find(evnt->dst) != involved_nodes.end()
			and involved_nodes.find(evnt->src) != involved_nodes.end()) {
		bandwidth = std::max(abs(generator->generate_normal(trading_mean, trading_std)),
				trading_mean/2);
		bandwidth = std::min(bandwidth, trading_mean * 1.5);
	} else {
		bandwidth = std::max(abs(generator->generate_normal(process_mean, process_std)),
					process_mean/2);
		bandwidth = std::min(bandwidth, process_mean * 1.5);
	}
	return bandwidth;
}

vector<Event*> ASNode::process_quality_event(QualityRefreshEvent* evnt, double world_time) {
	double new_process_mean = std::max(abs(generator->generate_normal(qos_mean, qos_std)), qos_mean / 2);
	double new_process_std = std::max(abs(generator->generate_normal(qos_std, qos_std)), qos_std / 2);
	process_mean = new_process_mean;
	process_std = new_process_std;
	evnt->scheduled_time = world_time + generator->generate_exponential(qos_refresh_lambda);
	vector<Event*> reply;
	reply.push_back(evnt);
	for (int i : direct_connections) {
		double payload = avg_delay_to_direct.find(i)->second;
		generator->GetGraph()->GetEdge(id, i)->SetPayload(payload);
	}
	return reply;
}

vector<Event*> ASNode::process_trading_event(TradingQualityEvent* evnt, double world_time) {
	double new_trading_mean = std::max(abs(generator->generate_normal(trading_qos_mean, trading_qos_std)), trading_qos_mean / 2);
	double new_trading_std = std::max(abs(generator->generate_normal(trading_qos_std, trading_qos_std)), trading_qos_std / 2);
	trading_mean = new_trading_mean;
	trading_std = new_trading_std;
	evnt->scheduled_time = world_time + generator->generate_exponential(trading_refresh_lambda);
	vector<Event*> reply;
	reply.push_back(evnt);
	generator->GetGraph()->SetIncomingWeight(id, 1.0/trading_mean);
	build_routing(false, true);
	return reply;
}

ASNode::~ASNode() {
	// TODO Auto-generated destructor stub
}

