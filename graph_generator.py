import networkx as nx
import argparse

parser = argparse.ArgumentParser(description='Graph generator')

parser.add_argument("-t", "--type", type=str, help="Type of graph to be generated : \"newmann\" for Newmann-Watts graph; \"power\" for Power Cluster graph; \"loop\" for simple loop graph")
parser.add_argument("-n", "--nodes", type=int, help="Amount of nodes in graph")
parser.add_argument("-s", "--seed", type=int, help="Initial seed")
parser.add_argument("-m", "--min_connect_edges", type=int, help="minimal connectivity for newmann or amount of edges for power")
parser.add_argument("-p", "--addition_edge", type=float, help="edge probability for newmann or triangle for power")
parser.add_argument("-f", "--filename", type=str, help="output filename")

args = parser.parse_args()
graph = None
type = args.type
#print 'Generation...'
if type == "newmann":
	graph = nx.newman_watts_strogatz_graph(args.nodes, args.min_connect_edges, args.addition_edge, args.seed)
if type == "power":
	graph = nx.powerlaw_cluster_graph(args.nodes, args.min_connect_edges, args.addition_edge, args.seed)
if type == "loop":
	graph = nx.cycle_graph(args.nodes)
#print '...Done'
#import time
#start = time.time()
#print nx.average_shortest_path_length(graph)
#print 'Elapsed :', time.time() - start

output = open(args.filename, 'w')
for edge in graph.edges():
	output.write(str(edge[0]) + "\n" + str(edge[1]) + "\n")
output.close()
