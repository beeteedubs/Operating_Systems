
allThreads=(1 2 3 5 10 20 50 100)
for t in ${allThreads[@]}; do
  ./vector_multiply $t
done
