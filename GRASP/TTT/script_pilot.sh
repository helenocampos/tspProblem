!#!/bin/bash
dataPoints=10;
eil51=433;
berlin52=7842;
pr144=58862;
pr299=49613;
u574=38916;
rat783=9250;
vm1084=254490;
pcb1173=60421;
d1291=52963;
rl1304=272037;
vm1748=353289;
u1817=60433;

./tspProblem GRASP_TTT_eil51 RDSNN first $dataPoints file instances/small/eil51.tsp 0 ql 3 $eil51 25;
./tspProblem GRASP_TTT_berlin52 RDSNN first $dataPoints file instances/small/berlin52.tsp 0 ql 3 $berlin52 25;
