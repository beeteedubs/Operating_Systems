make test3

counter=1
while [ $counter -le 10 ]
do
	echo THREADS: $counter
	./test3 $counter
	((counter++))
done
