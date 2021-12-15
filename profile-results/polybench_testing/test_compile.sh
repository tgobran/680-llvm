#!/bin/bash

if [[ "$#" -lt 5 ]]
then
	echo "ERROR: Invalid number of script arguments"
	echo "Arguments must be: CLANGPATH BENCHNAME TESTRUNS IS_BASE OPTLEVEL/PASSFILE"
	exit 1
fi

clangpath=$1
benchname=$2
testruns=$3
typeflag=$4
configs=0

if [[ $typeflag == 0 ]]
then
	passfile=$5
	passfile=$(cat $passfile | awk '{printf "%s ", $0} END {print ""}' )
	IFS=" " read -ra passes <<< "$passfile"
	configs=${#passes[@]}
else
	optlevel=$5
fi

echo "--------------------Compiling Program--------------------"

if [[ -d $benchname/ ]]
then
	benchcompile=${benchname/%/_compile}

	$clangpath/clang -I utilities $benchname/main.c -c -o main.o
		
	$clangpath/clang -O0 -S -I utilities -emit-llvm $benchname/func.c -c -o - | sed s/optnone// > func.bc
			
	if [[ $typeflag == 1 ]]
	then
		echo "Compiling Baseline Run"
	
		$clangpath/opt -$optlevel -print-changed=quiet func.bc > change.out 2>&1 
	else
		echo "Compiling Custom Pass Runs"	
	
		$clangpath/opt ${passes[@]} -print-changed=quiet func.bc > change.out 2>&1 
	fi

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
	echo -e "\nActive Passes:" > $benchcompile/run.results

	cat active.out
	cat active.out >> $benchcompile/run.results

	echo ""
	echo "" >> $benchcompile/run.results

	rm change.out sparse.out active.out -rf

	mv func.bc old.bc

	icounter=0
	while [ $icounter -le $configs ]
	do
		jcounter=0
		compavg=0
		resfile=$benchcompile/${benchname/%/_comptime.out}

		if [[ $typeflag == 0 ]]
		then
			resfile=$benchcompile/${benchname/%/_comptime$icounter.out}	
			currpasses=()
			for i in ${!passes[@]};
			do
				let "index = $i + 1"
				if [[ $index == $icounter ]]
				then
					echo "Excluding: ${passes[i]}"
					echo "Excluding: ${passes[i]}" >> $benchcompile/run.results
				else
					currpasses+=(${passes[i]})
				fi
			done
		fi
	
		while [ $jcounter -lt $testruns ]
		do	
			sleep 1

			rm func.bc -rf
	
			if [[ $typeflag == 1 ]]
			then
				$clangpath/opt -$optlevel old.bc -time-passes -o func.bc > $resfile 2>&1
			else
				$clangpath/opt ${currpasses[@]} old.bc -time-passes -o func.bc > $resfile 2>&1
			fi
	
			grep -hr "Total Execution Time:" $resfile > time.out	

			IFS=$'\n' read -d '' -r -a timelines < time.out
			comptime=${timelines[0]}
			IFS=" " read -ra comptime <<< "$comptime"
			compavg=`echo "scale=6; ${comptime[3]} + $compavg" | bc -l`

			rm time.out -rf
	
			((jcounter++))
		done

		compavg=`echo "scale=6; $compavg / $testruns" | bc -l`
		echo -e "Average Compile Time $icounter ($testruns Runs): $compavg seconds\n"	
		echo -e "Average Compile Time $icounter ($testruns Runs): $compavg seconds\n" >> $benchcompile/run.results	
	
		((icounter++))
	done	

	IFS=" "
	
	rm main.o old.bc func.bc func.o -rf

	exit 0
else
	echo "ERROR: Unable to compile benchmark, invalid name"
	exit 1
fi
