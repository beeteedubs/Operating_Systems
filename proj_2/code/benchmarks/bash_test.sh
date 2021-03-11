
allThreads=(1 2 3 444 3)
for t in ${allThreads[@]}; do
  ./vector_multiply $t
done
