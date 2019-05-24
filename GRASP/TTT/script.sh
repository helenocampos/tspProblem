!#!/bin/bash
dataPoints=10;
eil51=461;
berlin52=8146;
pr144=63220;
pr299=52047;
u574=39858;
rat783=9511;
vm1084=258441;
pcb1173=61444;
d1291=54866;
rl1304=273184;
vm1748=363481;
u1817=61778;

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
