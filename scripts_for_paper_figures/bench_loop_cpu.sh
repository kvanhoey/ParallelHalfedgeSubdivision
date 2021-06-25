
for CPU in 1 2 4 8 16 24; do
	export OMP_NUM_THREADS=$CPU
	echo "$CPU CPUs"
	for D in {1..6}; do
		./bench_loop bigguy.obj $D;
	done
done
