#!/bin/bash
smallInstances=instances/small
mediumInstances=instances/medium
largeInstances=instances/large
./tspProblem smallNN NN both 1 folder $smallInstances > smallNN.log 2>&1;
./tspProblem mediumNN NN both 1 folder $mediumInstances > mediumNN.log 2>&1;
./tspProblem largeNN NN both 1 folder $largeInstances > largeNN.log 2>&1;
./tspProblem smallDSNN DSNN both 1 folder $smallInstances > smallDSNN.log 2>&1;
./tspProblem mediumDSNN DSNN both 1 folder $mediumInstances > mediumDSNN.log 2>&1;
./tspProblem largeDSNN DSNN both 1 folder $largeInstances > largeDSNN.log 2>&1;
