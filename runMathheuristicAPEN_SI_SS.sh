#!/bin/bash

PS="I_15_2_S_1-9_1
I_15_2_S_1-49_1
I_15_2_S_1-99_1
I_15_2_S_1-124_1
I_15_3_S_1-9_1
I_15_3_S_1-49_1
I_15_3_S_1-99_1
I_15_3_S_1-124_1
I_15_4_S_1-9_1
I_15_4_S_1-49_1
I_15_4_S_1-99_1
I_15_4_S_1-124_1
I_15_5_S_1-9_1
I_15_5_S_1-49_1
I_15_5_S_1-99_1
I_15_5_S_1-124_1
"

TIMELIMIT="20"

for timeLimit in $TIMELIMIT
  do
	for probl in $PS 
	do 
   		echo "Resolvendo o problema $probl with time limit $timeLimit and mip start true"
  		./main $probl $timeLimit 0 -1 -1
	  done
done


