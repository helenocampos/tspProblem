#!/bin/bash
./tspProblem truncatedAnalysis RDSNN first 1 file instances/small/pr439.tsp 0 ql 4 1 300 20 | grep "#" > pr439_data.txt;
./tspProblem truncatedAnalysis RDSNN first 1 file instances/small/d657.tsp 0 ql 4 1 300 20 | grep "#" > d657_data.txt;
./tspProblem truncatedAnalysis RDSNN first 1 file instances/medium/rl1889.tsp 0 ql 4 1 300 20 | grep "#" > rl1889_data.txt;
./tspProblem truncatedAnalysis RDSNN first 1 file instances/medium/d2103.tsp 0 ql 4 1 300 20 | grep "#" > d2103.txt;
./tspProblem truncatedAnalysis RDSNN first 1 file instances/large/d15112.tsp 0 ql 4 1 300 20 | grep "#" > d15112.txt;
