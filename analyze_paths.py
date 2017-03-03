import networkx as nx

filenames = ['path_ways_' + str(i) for i in range(100)]

def get_used_paths(node, filelist):
    file = filelist[node]
    data_file = open(file, 'r')
    lines = data_file.readlines()
    return [[int(item) for item in line.split(' ')] for line in lines]

graph = nx.Graph()

graph_file = open('power_100_4_0.15_335', 'r')
lines = graph_file.readlines()
edges = []
nodes = []
counter = 0
while counter < len(lines):
    f = int(lines[counter])
    counter += 1
    s = int(lines[counter])
    if f not in nodes:
        nodes.append(f)
    if s not in nodes:
        nodes.append(s)
    counter += 1
    edges.append([f, s])

graph.add_nodes_from(nodes)
for edge in edges:
    graph.add_edge(edge[0], edge[1])

diffs = []

for i in range(100):
    used = get_used_paths(i, filenames)
    real =  nx.single_source_shortest_path(graph, i)
    for key in real.keys():
        diffs.append(len(used[key]) - len(real[key]))

import numpy as np
import matplotlib.pyplot as plt
print np.mean(diffs)
print diffs

plt.hist(diffs, bins=50)
plt.show()
