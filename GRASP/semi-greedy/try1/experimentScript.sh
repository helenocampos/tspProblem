#!/bin/bash
smallInstances=instances/small
mediumInstances=instances/medium
largeInstances=instances/large
./tspProblem logSmall_RDSNN RDSNN none 10 folder $smallInstances 2 ql 10 20 2 > smallDSNN.log 2>&1;  
./tspProblem logMedium_RDSNN RDSNN none 10 folder $mediumInstances 2 ql 10 20 2 > mediumDSNN.log 2>&1; 
./tspProblem logLarge_RDSNN RDSNN none 10 folder $largeInstances 2 ql 10 20 2 > largeDSNN.log 2>&1;
./tspProblem logSmall_RNN RNN none 10 folder $smallInstances 2 ql 10 20 2 > smallNN.log 2>&1;
./tspProblem logMedium_RNN RNN none 10 folder $mediumInstances 2 ql 10 20 2 > mediumNN.log 2>&1;
./tspProblem logLarge_RNN RNN none 10 folder $largeInstances 2 ql 10 20 2 > largeNN.log 2>&1;