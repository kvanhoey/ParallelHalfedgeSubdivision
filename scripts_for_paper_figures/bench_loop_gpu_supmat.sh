for mesh in ../meshes/data_benching/*.obj; do  
	echo $mesh
	for D in {1..7}; do
		../build/bench_loop_gpu $mesh $D ;
	done
done

