for mesh in ../meshes/data_benching/*.obj; do  
	echo $mesh
	for CPU in 32 16 8 4 2 1; do
		export OMP_NUM_THREADS=$CPU
		echo "$CPU CPUs"
		for D in 6; do
			../build/bench_loop_cpu $mesh $D 0 50;
		done
	done
done

