allThreads=(1 2 3 5 10 20 50 100)
for t in ${allThreads[@]}; do
	echo $t
  ./parallel_cal $t
done
