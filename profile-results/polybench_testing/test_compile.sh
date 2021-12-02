#!/bin/bash

if [[ "$#" -lt 4 ]]
then
	echo "ERROR: Invalid number of script arguments"
	echo "Arguments must be: CLANGPATH BENCHNAME TESTRUNS IS_O2"
	exit 1
fi

clangpath=$1
benchname=$2
testruns=$3
typeflag=$4
passfile=0

if [[ $typeflag == 0 ]]
then
	if [[ "$#" -lt 5 ]]
	then
		echo "ERROR: Invalid number of script arguments"
		echo "Arguments must be: CLANGPATH BENCHFOLDER TESTRUNS IS_O2 PASS_FILE"
		exit 1
	fi
	passfile=$5
fi

echo "--------------------Compiling Program--------------------"

if [[ -d $benchname/ ]]
then
	benchcompile=${benchname/%/_compile}

	rm $benchcompile -rf

	mkdir $benchcompile
		
	$clangpath/clang -I utilities $benchname/main.c -c -o main.o
			

	if [[ $typeflag == 1 ]]
	then
	echo "Compiling Baseline Run"
		$clangpath/clang -O0 -S -I utilities -emit-llvm $benchname/func.c -c -o - | sed s/optnone// > func.bc
	
		$clangpath/opt -O2 -print-changed=quiet func.bc > change.out 2>&1 
		
		grep -hnr "IR Dump After" --text change.out > sparse.out

		echo "" > active.out 
	
		while IFS= read -r line;
		do
			IFS=" " read -ra element <<< "$line"
			if ! grep -Fxq "${element[4]}" active.out;
			then 	
				echo -e "${element[4]}" >> active.out
			fi
		done < sparse.out

		sort active.out > $benchcompile/${benchname/%/_active.out}

		echo -e "\nActive Passes:"

		cat $benchcompile/${benchname/%/_active.out}

		echo ""

		rm change.out sparse.out active.out -rf
	fi

	mv func.bc old.bc

	counter=0
	compavg=0
	while [ $counter -lt $testruns ]
	do	
		rm func.bc -rf
	
		if [[ $typeflag == 1 ]]
		then
			$clangpath/opt -O2 old.bc -time-passes -o func.bc > $benchcompile/${benchname/%/_comptime.out} 2>&1
		fi
	
		grep -hnr "Total Execution Time:" $benchcompile/${benchname/%/_comptime.out} > time.out	

		IFS=$'\n' read -d '' -r -a timelines < time.out
		comptime=${timelines[0]}
		IFS=" " read -ra comptime <<< "$comptime"
		compavg=`echo "scale=6; ${comptime[4]} + $compavg" | bc -l`

		rm time.out -rf
	
		((counter++))
	done
	
	IFS=" "

	compavg=`echo "scale=6; $compavg / $testruns" | bc -l`
	echo -e "Average Compile Time ($testruns Runs): $compavg seconds\n"	

	$clangpath/llc func.bc -o func.o -filetype=obj

	$clangpath/clang main.o func.o polybench.o -o $benchcompile/$benchname.x

	rm main.o func.bc func.o -rf

	exit 0
else
	echo "ERROR: Unable to compile benchmark, invalid name"
	exit 1
fi
