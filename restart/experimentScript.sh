#!/bin/bash
#restart every X iterations without improvements
./tspProblem pcb1173_restart_15_i RDSNN first 200 file instances/medium/pcb1173.tsp 0 ql 5 63487 150 15 25;
./tspProblem pcb1173_restart_30_i RDSNN first 200 file instances/medium/pcb1173.tsp 0 ql 5 63487 150 30 25;
./tspProblem pcb1173_restart_45_i RDSNN first 200 file instances/medium/pcb1173.tsp 0 ql 5 63487 150 45 25;
./tspProblem pcb1173_restart_60_i RDSNN first 200 file instances/medium/pcb1173.tsp 0 ql 5 63487 150 60 25;
./tspProblem pcb1173_restart_75_i RDSNN first 200 file instances/medium/pcb1173.tsp 0 ql 5 63487 150 75 25;
./tspProblem pcb1173_restart_100_i RDSNN first 200 file instances/medium/pcb1173.tsp 0 ql 5 63487 150 100 25;
./tspProblem u1817_restart_15_i RDSNN first 200 file instances/medium/u1817.tsp 0 ql 5 65518 150 15 25;
./tspProblem u1817_restart_30_i RDSNN first 200 file instances/medium/u1817.tsp 0 ql 5 65518 150 30 25;
./tspProblem u1817_restart_45_i RDSNN first 200 file instances/medium/u1817.tsp 0 ql 5 65518 150 45 25;
./tspProblem u1817_restart_60_i RDSNN first 200 file instances/medium/u1817.tsp 0 ql 5 65518 150 60 25;
./tspProblem u1817_restart_75_i RDSNN first 200 file instances/medium/u1817.tsp 0 ql 5 65518 150 75 25;
./tspProblem u1817_restart_100_i RDSNN first 200 file instances/medium/u1817.tsp 0 ql 5 65518 150 100 25;
#restart every X seconds without improvements
./tspProblem pcb1173_restart_15_t RDSNN first 200 file instances/medium/pcb1173.tsp 0 ql 7 63487 150 10 25;
./tspProblem pcb1173_restart_30_t RDSNN first 200 file instances/medium/pcb1173.tsp 0 ql 7 63487 150 20 25;
./tspProblem pcb1173_restart_45_t RDSNN first 200 file instances/medium/pcb1173.tsp 0 ql 7 63487 150 30 25;
./tspProblem pcb1173_restart_60_t RDSNN first 200 file instances/medium/pcb1173.tsp 0 ql 7 63487 150 40 25;
./tspProblem pcb1173_restart_75_t RDSNN first 200 file instances/medium/pcb1173.tsp 0 ql 7 63487 150 50 25;
./tspProblem u1817_restart_15_t RDSNN first 200 file instances/medium/u1817.tsp 0 ql 7 65518 150 10 25;
./tspProblem u1817_restart_30_t RDSNN first 200 file instances/medium/u1817.tsp 0 ql 7 65518 150 20 25;
./tspProblem u1817_restart_45_t RDSNN first 200 file instances/medium/u1817.tsp 0 ql 7 65518 150 30 25;
./tspProblem u1817_restart_60_t RDSNN first 200 file instances/medium/u1817.tsp 0 ql 7 65518 150 40 25;
./tspProblem u1817_restart_75_t RDSNN first 200 file instances/medium/u1817.tsp 0 ql 7 65518 150 50 25;
