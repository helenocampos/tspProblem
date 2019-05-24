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
./tspProblem GRASP_TTT_pr144 RDSNN first $dataPoints file instances/small/pr144.tsp 0 ql 3 $pr144 25;
./tspProblem GRASP_TTT_pr299 RDSNN first $dataPoints file instances/small/pr299.tsp 0 ql 3 $pr299 25;
./tspProblem GRASP_TTT_u574 RDSNN first $dataPoints file instances/small/u574.tsp 0 ql 3 $u574 25;
./tspProblem GRASP_TTT_rat783 RDSNN first $dataPoints file instances/small/rat783.tsp 0 ql 3 $rat783 25;
./tspProblem GRASP_TTT_vm1084 RDSNN first $dataPoints file instances/medium/vm1084.tsp 0 ql 3 $vm1084 25;
./tspProblem GRASP_TTT_pcb1173 RDSNN first $dataPoints file instances/medium/pcb1173.tsp 0 ql 3 $pcb1173 25;
./tspProblem GRASP_TTT_d1291 RDSNN first $dataPoints file instances/medium/d1291.tsp 0 ql 3 $d1291 25;
./tspProblem GRASP_TTT_rl1304 RDSNN first $dataPoints file instances/medium/rl1304.tsp 0 ql 3 $rl1304 25;
./tspProblem GRASP_TTT_vm1748 RDSNN first $dataPoints file instances/medium/vm1748.tsp 0 ql 3 $vm1748 25;
./tspProblem GRASP_TTT_u1817 RDSNN first $dataPoints file instances/medium/u1817.tsp 0 ql 3 $u1817 25;
