from __future__ import print_function
from os import system
import sys
import progressbar
from subprocess import Popen, PIPE
import numpy as np
import matplotlib.pyplot as plt
from threading import Thread
from time import sleep, time
from math import trunc


def run_test_once(seed, involved_size, time, graph, result):
    p = Popen(['./Release/TradinSimulator', str(seed), str(involved_size), str(time), graph], stdout=PIPE)
    #system("./Release/TradingSimulator " + str(seed) + " " +
    #          str(involved_size) + " " +
    #          str(time) + " " +
    #          graph)
    #lines = ""#sys.stdin.readlines()
    output = p.stdout.read()
    if involved_size not in result.keys():
        result[involved_size] = []
    result[involved_size].append(float(output))


def run_for_graph(type, n, m, p, seed, run_time, experiments, threads_num):
    filename = type + "_" + str(n) + "_" + str(m) + "_" + str(p) + "_" + str(seed)
    command = ['python', 'graph_generator.py', '-t='+type, '-n='+str(n), '-s='+str(seed), '-m='+str(m), '-p='+str(p),
           '-f='+filename]
    system(' '.join(command))
    data = {}
    counter = 0
    pb = progressbar.ProgressBar()
    start = time()
    while counter < experiments:
        threads = [None] * threads_num
        for i in range(threads_num):
            estimated = 0
            executed = counter
            if executed != 0:
                estimated = int(1.0 * (time() - start) * (experiments - executed) / (executed))
            elspsed = int(time() - start)
            print_string = 'Progress : ' + str(1 + executed) + ' Out of ' + str(experiments) \
                           + ' Estimated time :' + ' ' +\
                           str(trunc(estimated / 3600)) + 'h ' + str(trunc((estimated % 3600) / 60)) + 'm ' +\
                           str(estimated % 60) + 's' + ' Elapsed time :' + ' ' +\
                           str(trunc(elspsed/ 3600)) + 'h ' + str(trunc((elspsed % 3600) / 60)) + 'm ' +\
                           str(elspsed % 60) + 's'
            print(print_string, end='\n')
            inv = counter % (n + 1)
            counter += 1
            #run_test_once(seed + counter, inv, run_time, filename, data)
            threads[i] = Thread(target=run_test_once, args=(seed + counter, inv, run_time, filename, data))
        for i in range(threads_num):
            threads[i].start()
        #sleep(1)
        for i in range(threads_num):
            threads[i].join()
    print('\nDone', end='\n')
    #system('rm ' + filename)
    return data

def run_for_bench(types, sizes, connects, probs, seed, run_time, exp_multiplier, threads):
    total_data = []
    total = len(types) * len(sizes) * len(connects) * len(probs)
    start = time()
    steps = 0
    for type in types:
        for size in sizes:
            for connect in connects:
                for prob in probs:
                    steps += 1
                    total_data.append((type, size, connect, run_for_graph(type, size, connect, prob, seed,
                                                                          run_time, size * exp_multiplier, threads)))
                    steps += 1
                    diff = (time() - start)
                    h = int(diff / 3600)
                    m = int(h / 60)
                    s = m % 60
#                    print "Done :", steps, "out of", total, 'spent', h, 'h', m, 'm', s, 's'
    return total_data

graph_size = 500
result = run_for_graph('power', graph_size, 4, 0.15, 335, 0.05, graph_size * 10, 10)
#import yaml
#data = run_for_bench(['power', 'newmann'], range(500, 2001, 100), range(2,6), [0.05, 0.06, 0.07, 0.08, 0.09, 0.1], 123, 0.05, 10, 4)
#data = run_for_bench(['power'], range(100, 201, 10), range(2,6), [0.095], 123, 0.05, 10, 4)
#with open('data.yml', 'w') as outfile:
#    yaml.dump(data, outfile, default_flow_style=False)
#exit(0)
x_data = []
y_data = []
for key in result.keys():
    x_data += [key] * len(result[key])
    y_data += result[key]
#compilated = []
#for key in result.keys():
#    compilated.append(np.mean(result[key]))
#mx = max(compilated)
#ml = min(compilated)
#compilated = [(mx - item) / (mx - ml) for item in compilated]

plt.plot(x_data, y_data, 'ro')
plt.show()