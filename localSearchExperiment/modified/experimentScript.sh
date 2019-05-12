#!/bin/bash
smallInstances=instances/small
mediumInstances=instances/medium
largeInstances=instances/large
./tspProblem smallNN_first_2 NN first_2 1000 folder $smallInstances 0 0 100 > smallNN_first_2.log 2>&1;
./tspProblem mediumNN_first_2 NN first_2 1000 folder $mediumInstances 0 0 100 > mediumNN_first_2.log 2>&1;
./tspProblem largeNN_first_2 NN first_2 1000 folder $largeInstances 0 0 100 > largeNN_first_2.log 2>&1;
./tspProblem smallNN_best_2 NN best_2 1000 folder $smallInstances 0 0 100 > smallNN_best_2.log 2>&1;
./tspProblem mediumNN_best_2 NN best_2 1000 folder $mediumInstances 0 0 100 > mediumNN_best_2.log 2>&1;
./tspProblem largeNN_best_2 NN best_2 1000 folder $largeInstances 0 0 100 > largeNN_best_2.log 2>&1;
./tspProblem smallDSNN_first_2 DSNN first_2 1000 folder $smallInstances 0 0 100 > smallDSNN_first_2.log 2>&1;
./tspProblem mediumDSNN_first_2 DSNN first_2 1000 folder $mediumInstances 0 0 100 > mediumDSNN_first_2.log 2>&1;
./tspProblem largeDSNN_first_2 DSNN first_2 1000 folder $largeInstances 0 0 100 > largeDSNN_first_2.log 2>&1;
./tspProblem smallDSNN_best_2 DSNN best_2 1000 folder $smallInstances 0 0 100 > smallDSNN_best_2.log 2>&1;
./tspProblem mediumDSNN_best_2 DSNN best_2 1000 folder $mediumInstances 0 0 100 > mediumDSNN_best_2.log 2>&1;
./tspProblem largeDSNN_best_2 DSNN best_2 1000 folder $largeInstances 0 0 100 > largeDSNN_best_2.log 2>&1;
