#!/bin/bash
clear
WMIC CPU Get NumberOfLogicalProcessors
MAX_THREADCOUNT=$NUMBER_OF_PROCESSORS
# echo $MAX_THREADCOUNT

LAST_THREADCOUNT=MAX_THREADCOUNT
for ((i=2; i <= $MAX_THREADCOUNT; i = i*2))
do
	printf "\nRunning on $i Threads\n"
	*x64/Release/PraktikumVGJS.exe $i
	LAST_THREADCOUNT=$i
done

if [[ $LAST_THREADCOUNT -lt $MAX_THREADCOUNT ]]
then
	printf "\nRunning on $MAX_THREADCOUNT Threads\n"
	*x64/Release/PraktikumVGJS.exe $MAX_THREADCOUNT
fi

read -rn1 -p "Press any key to continue" x
exit 0
