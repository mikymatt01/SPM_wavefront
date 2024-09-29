#!/bin/bash

source ~/.bashrc

TIMESTAMP=$(date +"%Y%m%d_%H%M%S")
RESULTS_DIR="tests/$TIMESTAMP"
RESULTS_FILE="$RESULTS_DIR/wavefront_runtimes.csv"
LOG_FILE="$RESULTS_DIR/wavefront_runtimes.log"

SIZES=()
for i in {8192..8192}; do
    SIZES+=($i)
done
echo "SIZES=(${SIZES[@]})"

PROCS=(2 4 8 16)

compile_programs() {
    echo "Compiling programs..."
    make clean
    make sequential
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
    local value_diagonal=0
    local value_triangles=0

    case $version in
        sequential)
                value_diagonal=$(./wavefront $size | grep "last:" | awk '{print $2}')
                value_triangles=$(./wavefront_triangles $size $procs | grep "last:" | awk '{print $2}')
            ;;
    esac

    if [ "$(echo "$value_diagonal == $value_triangles" | bc)" -eq 1 ]; then
        echo " "
    else
        echo "$version,$size,$procs,$value_diagonal,$value_triangles"
    fi
}

compile_programs

mkdir -p $RESULTS_DIR

echo "Version,Size,Processes,Correct Value,Triangular Value" > $RESULTS_FILE

# Run tests
cmp=" "
for version in sequential; do
    for size in "${SIZES[@]}"; do
        for procs in "${PROCS[@]}"; do
            result=$(run_test $version $size $procs)

            if [ "$result" != "$cmp" ]; then
                echo "$result" >> $RESULTS_FILE
            fi

            echo "$size, $procs" >> $LOG_FILE
        done
    done
done

echo "Tests completed. Runtime results saved in $RESULTS_FILE"
