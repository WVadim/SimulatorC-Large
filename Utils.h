/*
 * Utils.h
 *
 *  Created on: Jan 30, 2017
 *      Author: vadim
 */
#pragma once
#include <vector> // For vector
#include <cassert> // For assert
#include <cstdlib> // For NULL
//#define MEASURE
#ifdef MEASURE
#include <ctime>
#include <iostream>
#endif

#ifdef MEASURE
using std::clock_t;
using std::clock;
#endif

//#define DEBUG
#ifdef DEBUG
#include <iostream>
using std::cout;
using std::endl;
#endif

using std::vector;

class Node {
public:
	Node(int _ID):
		sID(_ID)
	{}

	int ID() {
		return sID;
	}
private:
	int sID;
};

template <typename T>
class Edge {
public:
	Edge():
		start(NULL),
		end(NULL)
	{}

	void SetPayload(T data) {
		payload = data;
	}

	T GetPayload() {
		return payload;
	}

	void SetStart(Node* node) {
		assert(node);
		start = node;
	}

	void SetEnd(Node* node) {
		assert(node);
		end = node;
	}

	Node* GetStart() {
		return start;
	}

	int GetStartID() {
		assert(start);
		return start->ID();
	}

	int GetEndID() {
		assert(end);
		return end->ID();
	}

	Node* GetEnd() {
		return end;
	}
private:
	T payload;
	Node* start;
	Node* end;
};

template <typename EdgePayload>
class Graph {
public:
	Graph(int _size, bool _directed=false, bool _weighted=false):
		directed(_directed),
		weighted(_weighted),
		size(_size)
	{
		incident_matrix = new double*[size];
		for (int i = 0; i < size; i++) {
			incident_matrix[i] = new double[size];
			for (int j = 0; j < size; j++) {
				incident_matrix[i][j] = 0;
			}
		}
		for(int i = 0; i < size; i++) {
			Node* node = new Node(i);
			nodes.push_back(node); //This is Eclipse fail, not mine
		}
#ifdef DEBUG
		cout << "Graph";
		if (directed)
			cout << " directed";
		if (weighted)
			cout << " weighted";
		cout << " with " << size << " nodes created" << endl;
#endif
	}

	int NodesSize() {
		return nodes.size();
	}

	int EdgesSize() {
		return edges.size();
	}

	vector<Node*> Nodes() {
		return nodes;
	}

	vector<Edge<EdgePayload> *> Edges() {
		return edges;
	}

	void SetIncomingWeight(int node, double weight) {
		for (int i = 0; i < size; i++) {
			if (incident_matrix[node][i] != 0)
				GetEdge(node, i)->SetPayload(weight);
				//incident_matrix[node][i] = weight;
		}
	}

	void AddEdge(int from, int to, EdgePayload payload, float weight=1) {
		assert((from < size) && (to < size));
		if (!weighted) {
			weight = 1;
		}
		incident_matrix[from][to] = weight;
		if (!directed) {
			incident_matrix[to][from] = weight;
		}
		Edge<EdgePayload>* edge = new Edge<EdgePayload>();
		edge->SetStart(nodes[from]);
		edge->SetEnd(nodes[to]);
		edge->SetPayload(payload);
		edges.push_back(edge);
	}

	Edge<EdgePayload>* GetEdge(int from, int to) {
		assert((from < size) && (to < size));
		if (incident_matrix[from][to] == 0) {
			return NULL;
		}
		for (int i = 0; i < edges.size(); i++) {
			if (edges[i]->GetStartID() == from && edges[i]->GetEndID() == to) {
				return edges[i];
			}
			if (!directed) {
				if (edges[i]->GetStartID() == to && edges[i]->GetEndID() == from) {
					return edges[i];
				}
			}
		}
		assert(false);
		return NULL;
	}

	void SetWeight(int from, int to, float weight) {
		assert(weighted);
		assert((from < size) && (to < size));
		assert(incident_matrix[from][to] != 0);
		incident_matrix[from][to] = weight;
		if (!directed) {
			incident_matrix[to][from] = weight;
		}
	}

	vector<vector<int>> ShortestPath(int source, bool use_weight, bool use_payload=false) {
#ifdef MEASURE
		clock_t start = clock();
#endif
		assert(source < size);
		vector<float> cost;
		vector<vector<int>> paths;
		vector<int> unvisited;
		for (int i = 0; i < size; i++) {
			cost.push_back(100000000);
			unvisited.push_back(i);
			paths.push_back(vector<int>());
		}
		cost[source] = 0;
#ifdef DEBUG
		/*cout << "Incident matrix :" << endl;
		for (int i = 0; i < size; i++) {
			for (int j = 0; j < size; j++) {
				cout << incident_matrix[i][j] << " ";
			}
			cout << endl;
		}*/
#endif
		while (unvisited.size() != 0) {
			int min_cost = 0;
			for (unsigned int i = 0; i < unvisited.size(); i++) {
				int index = unvisited[i];
				if (cost[index] <= cost[unvisited[min_cost]] && cost[index] >= 0) {
					min_cost = i;
				}
			}
			int delete_val = min_cost;
			min_cost = unvisited[min_cost];
			unvisited.erase(unvisited.begin() + delete_val);
			for (unsigned int i = 0; i < unvisited.size(); i++) {
				int index = unvisited[i];
				double edge_cost = 0;
				if (use_weight || use_payload) {
					if (use_weight && !use_payload)
						edge_cost = incident_matrix[min_cost][index];
					if (use_payload && !use_weight)
						edge_cost = incident_matrix[min_cost][index] ? GetEdge(min_cost, index)->GetPayload() : 0;
					if (use_payload && use_weight) {
						double pay_cost = incident_matrix[min_cost][index] ? GetEdge(min_cost, index)->GetPayload() : 0;
						double weight_cost = incident_matrix[min_cost][index];
						edge_cost = std::min(pay_cost, weight_cost);
					}
				}
				else
					edge_cost = incident_matrix[min_cost][index] ? 1 : 0;
				if (edge_cost != 0) {
					if (cost[index] > cost[min_cost] + edge_cost ) {
						cost[index] = cost[min_cost] + edge_cost ;
						paths[index] = paths[min_cost];
						paths[index].push_back(index);
					}
				}
			}
		}
#ifdef DEBUG
		if (use_weight && !use_payload) {
		cout << "Costs : ";
		for (int i = 0; i < size; i++) {
			cout << cost[i] << " ";
		}
		cout << endl;
		}
#endif
#ifdef MEASURE
		clock_t end = clock();
		std::cout << "Path measurements : " << double(end - start) / CLOCKS_PER_SEC << "\n";
#endif

		return paths;
	}

private:
	vector<Node *> nodes;
	vector<Edge<EdgePayload> *> edges;
	double** incident_matrix;
	bool directed;
	bool weighted;
	int size;
};
