#!/bin/bash

if [[ "$#" -lt 6 ]]
then
	echo "ERROR: Invalid number of script arguments"
	echo "Arguments must be: CLANGPATH BENCHNAME TESTRUNS PASSFILE DATASET DEFAULT_OPTLEVEL"
	exit 1
fi

clangpath=$1
benchname=$2
testruns=$3
passfile=$4
passfile=$(cat $passfile | awk '{printf "%s ", $0} END {print ""}' )
IFS=" " read -ra passes <<< "$passfile"
configs=${#passes[@]}
dataset=-D$5_DATASET
optlevel=$6

echo "--------------------Compiling Program--------------------"

if [[ -d $benchname/ ]]
then
	benchrun=${benchname/%/_run}
	
	$clangpath/clang $dataset -I utilities utilities/polybench.c -c -o polybench.o

	$clangpath/clang $dataset -I utilities $benchname/main.c -c -o main.o
		
	$clangpath/clang $dataset -O0 -S -I utilities -emit-llvm $benchname/func.c -c -o - | sed s/optnone// > func.bc
				
	$clangpath/opt -$optlevel -print-changed=quiet func.bc > change.out 2>&1 
	
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

	echo -e "\nActive Passes:"
	echo -e "\nActive Passes:" > $benchrun/run.results

	cat active.out
	cat active.out >> $benchrun/run.results

	echo ""
	echo "" >> $benchrun/run.results

	rm change.out sparse.out active.out -rf

	mv func.bc old.bc

	icounter=-1
	while [ $icounter -le $configs ]
	do
		jcounter=0
		resfile=$benchrun/${benchname/%/_gmon.sum}

		if [[ $icounter -lt 0 ]]
		then
			$clangpath/opt -$optlevel old.bc -o func.bc
			$clangpath/llc func.bc -o func.o -filetype=obj
			$clangpath/clang $dataset -lm -pg main.o func.o polybench.o -o $benchrun/$benchname.x
			exefile=$benchrun/$benchname.x
		else
			resfile=$benchrun/${benchname/%/_gmon$icounter.sum}	
			currpasses=()
			for i in ${!passes[@]};
			do
				let "index = $i + 1"
				if [[ $index == $icounter ]]
				then
					echo "Excluding: ${passes[i]}"
					echo "Excluding: ${passes[i]}" >> $benchrun/run.results
				else
					currpasses+=(${passes[i]})
				fi
			done
			$clangpath/opt -enable-new-pm=0 ${currpasses[@]} old.bc -o func.bc
			$clangpath/llc func.bc -o func.o -filetype=obj
			$clangpath/clang $dataset -lm -pg main.o func.o polybench.o -o $benchrun/${benchname}$icounter.x
			exefile=$benchrun/${benchname}$icounter.x
		fi

		echo "Testing: $exefile" 
		echo "Testing: $exefile" >> $benchrun/run.results
	
		while [ $jcounter -lt $testruns ]
		do	
			sleep 1
		
			./$exefile

			gprof -s $exefile gmon.out	

			((jcounter++))
		done

		gprof -b $exefile gmon.sum > gprof.out

                grep -hr "kernel_$benchname" gprof.out > time.out
		IFS=$'\n' read -d '' -r -a timelines < time.out
		IFS=" " read -ra runtime <<< "$timelines"
		echo -e "Average Compile Time $icounter ($testruns Runs): ${runtime[2]} seconds\n"	
		echo -e "Average Compile Time $icounter ($testruns Runs): ${runtime[2]} seconds\n" >> $benchrun/run.results	
		rm gprof.out time.out -rf

		mv gmon.sum $resfile

		((icounter++))
	done	

	IFS=" "
	
	rm main.o old.bc func.bc func.o polybench.o *.x *.out -rf

	exit 0
else
	echo "ERROR: Unable to compile benchmark, invalid name"
	exit 1
fi
