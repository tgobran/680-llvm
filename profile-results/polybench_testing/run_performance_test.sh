#!/bin/bash

if [[ "$#" -lt 6 ]]
then
	echo "ERROR: Invalid number of script arguments"
	exit 1
fi

clangpath=$1
testfolder=$2
tuningfile=$3
pipelinefile=$4
configurationfile=$5
testruns=$6
shared=0
compthreads=0
if [[ "$#" -gt 6 ]]
then
	shared=$7
fi
if [[ "$#" -gt 7 ]]
then
	compthreads=$8
fi

if ! [[ -d $testfolder ]]
then
	echo "ERROR: Test folder wasn't a folder."
	exit 1
fi

if ! [[ -f $tuningfile ]]
then
	echo "ERROR: Tuning file wasn't a file."
	exit 1
fi

if ! [[ -f $pipelinefile ]]
then
	echo "ERROR: Pipeline file wasn't a file."
	exit 1
fi

if ! [[ -f $configurationfile ]]
then
	echo "ERROR: Configurations file wasn't a file."
	exit 1
fi

tunings=()
pipelines=()
configurations=()

function verify_test () {
	jitc="./jit.x $1 $2 1"
	nonc="./non.x $1 $2 1"
	
	exec 3>&2
	exec 2> /dev/null	

	eval "$jitc" > jit.data

	eval "$nonc" > non.data

	exec 2>&3
	
	cmp --silent -- "jit.data" "non.data"

	if [[ $? -ne 0 ]]
	then
		echo "ERROR: Inconsistent jit to non-jit results."
		rm *.data -f
		rm *.x -f
		exit 1
	fi

	echo "Verification Success"

	rm *.data -f
}

function run_test () {
	jitc="TIMEFORMAT='JIT PROGRAM TIME: %3Rs'; time ( ./jit.x $1 $2 0 )"
	#jitc="/usr/bin/time -f 'JIT PROGRAM TIME: %Es' ./jit.x $1 $2 0"
	
	nonc="TIMEFORMAT='NON PROGRAM TIME: %3Rs'; time ( ./non.x $1 $2 0 )"
	#nonc="/usr/bin/time -f 'NON PROGRAM TIME: %Es' ./non.x $1 $2 0"
	
	eval "$jitc" &> result

	sleep 1

	eval "$nonc" &>> result
}	

echo "-------------------Building Test Settings-------------------"

echo "Reading Tunings"

while IFS= read -r tuning
do
	tunings+=( $tuning )
done < $tuningfile

echo "Registered Tunings:"
echo "${tunings[@]}"

echo "Reading Pipelines"

while IFS= read -r pipeline
do
	pipelines+=( $pipeline )
done < $pipelinefile

echo "Registered Pipelines:"
echo "${pipelines[@]}"

echo "Reading Configurations"

while IFS= read -r configuration
do
	configurations+=( $configuration )
done < $configurationfile

echo "Registered Configurations:"
echo "${configurations[@]}"

echo "------------------Running Performance Test------------------"

echo -n "tuning,pipe,config" > jitstat.csv
echo -n "tuning,pipe,config" > jitvsnon.csv
counter=1
while [ $counter -le $testruns ]
do
	echo -n ",jit-ini-$counter,jit-res-$counter,jit-req-$counter,jit-fun-$counter,non-func-$counter" >> jitstat.csv
	echo -n ",jit-$counter,non-$counter" >> jitvsnon.csv
	((counter++))
done
echo ",ini-avg,res-avg,req-avg,jit-fun-avg,non-fun-avg" >> jitstat.csv
echo ",jit-avg,non-avg,comparison" >> jitvsnon.csv

for t in ${tunings[@]}
do
	for p in ${pipelines[@]}
	do
		for c in ${configurations[@]}
		do
			echo ""
			echo "Testing Configuration: $t $p $c"

			IFS=","
			set -- $c
			buildCmd="./build_performance_test.sh $clangpath $testfolder $t $p $shared $compthreads"
		
			eval $buildCmd
			if [[ $? -ne 0 ]]
			then
				exit 1
			fi

			echo -n "$t,$p,$1|$2" >> jitstat.csv
			echo -n "$t,$p,$1|$2" >> jitvsnon.csv
		
			echo ""

                        echo "------------------Verifying Compiled Program------------------"

	                export IJIT_OPT_PASSES="default<$p>"
	
			verify_test $1 $2

                        echo "-------------------Running Compiled Program------------------"
			counter=0
			initavg=0
			resolveavg=0
			requestavg=0
			jitfuncavg=0
			nonfuncavg=0
			jitavg=0
			nonavg=0
			while [ $counter -lt $testruns ]
			do
				echo "Run Number: $((counter + 1))"

				run_test $1 $2

				cat result

				sed -Ei 's/([+-]?[0-9.]+)[eE]\+?(-?)([0-9]+)/(\1*10^\2\3)/g' result
			
				IFS=$'\n' read -d '' -r -a resultlines < result
				
				jitinit=${resultlines[0]}
				IFS=" " read -ra jitinit <<< "${jitinit/s/}"
				initavg=`echo "scale=6; ${jitinit[3]} + $initavg" | bc -l`
				echo -n ",${jitinit[3]}" >> jitstat.csv
				
				jitresolve=${resultlines[1]}
				IFS=" " read -ra jitresolve <<< "${jitresolve/s/}"
				resolveavg=`echo "scale=6; ${jitresolve[3]} + $resolveavg" | bc -l`
				echo -n ",${jitresolve[3]}" >> jitstat.csv
				
				jitrequest=${resultlines[2]}
				IFS=" " read -ra jitrequest <<< "${jitrequest/s/}"	
				requestavg=`echo "scale=6; ${jitrequest[3]} + $requestavg" | bc -l`
				echo -n ",${jitrequest[3]}" >> jitstat.csv
				
				jitfunction=${resultlines[3]}
				IFS=" " read -ra jitfunction <<< "${jitfunction/s/}"
				jitfuncavg=`echo "scale=6; ${jitfunction[3]} + $jitfuncavg" | bc -l`
				echo -n ",${jitfunction[3]}" >> jitstat.csv
				
				nonfunction=${resultlines[4]}
				IFS=" " read -ra nonfunction <<< "${nonfunction/s/}"
				nonfuncavg=`echo "scale=6; ${nonfunction[3]} + $nonfuncavg" | bc -l`
				echo -n ",${nonfunction[3]}" >> jitstat.csv
				
				jitelapsed=${resultlines[5]}
				IFS=" " read -ra jitelapsed <<< "${jitelapsed/s/}"
				jitavg=`echo "scale=6; ${jitelapsed[3]} + $jitavg" | bc -l`
				echo -n ",${jitelapsed[3]}" >> jitvsnon.csv
				
				nonelapsed=${resultlines[6]}
				IFS=" " read -ra nonelapsed <<< "${nonelapsed/s/}"
				nonavg=`echo "scale=6; ${nonelapsed[3]} + $nonavg" | bc -l`
				echo -n ",${nonelapsed[3]}" >> jitvsnon.csv

				((counter++))
			done

			IFS=" "

			initavg=`echo "scale=6; $initavg / $counter" | bc -l`
			resolveavg=`echo "scale=6; $resolveavg / $counter" | bc -l`
			requestavg=`echo "scale=6; $requestavg / $counter" | bc -l`
			jitfuncavg=`echo "scale=6; $jitfuncavg / $counter" | bc -l`
			nonfuncavg=`echo "scale=6; $nonfuncavg / $counter" | bc -l`
			jitavg=`echo "scale=6; $jitavg / $counter" | bc -l`
			nonavg=`echo "scale=6; $nonavg / $counter" | bc -l`
			cmp=`echo "scale=6; $jitavg / $nonavg" | bc -l`
			
			echo ",$initavg,$resolveavg,$requestavg,$jitfuncavg,$nonfuncavg" >> jitstat.csv
			echo ",$jitavg,$nonavg,$cmp" >> jitvsnon.csv

			rm *.x -f
			if [[ $shared -ne 0 ]]
			then
				rm *.so -f
			fi
		done
	done
done

echo "-----------------JIT vs NON JIT CSV------------------"

cat jitvsnon.csv

echo "--------------------JIT DATA CSV---------------------"

cat jitstat.csv
