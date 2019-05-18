#!/bin/bash
smallInstances=instances/small
mediumInstances=instances/medium
largeInstances=instances/large
./tspProblem logSmall_all_ql all none 1000 folder $smallInstances 2 ql 10 20 2 > smallDSNN.log 2>&1;
./tspProblem logMedium_all_ql all none 1000 folder $mediumInstances 2 ql 10 20 2 > mediumDSNN.log 2>&1;
./tspProblem logLarge_all_ql all none 100 folder $largeInstances 2 ql 10 20 2 > largeDSNN.log 2>&1;
./tspProblem logSmall_all_qt all none 1000 folder $smallInstances 2 qt 10 20 2 > smallNN.log 2>&1;
./tspProblem logMedium_all_qt all none 1000 folder $mediumInstances 2 qt 10 20 2 > mediumNN.log 2>&1;
./tspProblem logLarge_all_qt all none 100 folder $largeInstances 2 qt 10 20 2 > largeNN.log 2>&1;
