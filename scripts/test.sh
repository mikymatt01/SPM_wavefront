#!/bin/bash

source ~/.bashrc

TIMESTAMP=$(date +"%Y%m%d_%H%M%S")
RESULTS_DIR="results/$TIMESTAMP"
mkdir -p $RESULTS_DIR
RESULTS_FILE="$RESULTS_DIR/wavefront_runtimes.csv"

SIZES=(2048 2896 4096 5793 8192)

PROCS=(1 2 4 8 16)

compile_programs() {
    echo "Compiling programs..."
    make clean
    make all
    if [ $? -ne 0 ]; then
        echo "Compilation failed. Exiting."
        exit 1
    fi
    echo "Compilation successful."
}

run_test() {
    local version=$1
    local size=$2
    local procs=$3
    local runtime=0

    case $version in
        sequential)
            if [ "$procs" -eq 1 ]; then
                runtime=$(./wavefront $size | grep "time:" | awk '{print $2}')
            else
                runtime="N/A"
            fi
            ;;
        mpi)
            runtime=$(mpirun -np $procs ./wavefront_mpi $size | grep "time:" | awk '{print $2}')
            ;;
        fastflow)
            runtime=$(OMP_NUM_THREADS=$procs ./wavefront_ff $size $procs | grep "time:" | awk '{print $2}')
            ;;
    esac

    echo "$version,$size,$procs,$runtime"
}

compile_programs

echo "Version,Size,Processes,Runtime" > $RESULTS_FILE

# Run tests
for version in sequential mpi fastflow; do
    for size in "${SIZES[@]}"; do
        for procs in "${PROCS[@]}"; do
            if [ "$version" = "sequential" ] && [ "$procs" -ne 1 ]; then
                continue
            fi
            result=$(run_test $version $size $procs)
            echo "$result" >> $RESULTS_FILE
        done
    done
done

echo "Tests completed. Runtime results saved in $RESULTS_FILE"
