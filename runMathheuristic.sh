#!/bin/bash

PS=" 
lowerLayerT5
lowerLayerT7
lowerLayerT10
lowerLayerT15
lowerLayerT30
"

TIMELIMIT="5 10 20 30 40"

for probl in $PS 
do 
	  for timeLimit in $TIMELIMIT
	  do
   		echo "Resolvendo o problema $probl with time limit $timeLimit and mip start true"
  		./main $probl $timeLimit 1 -1 -1
	  done
done


