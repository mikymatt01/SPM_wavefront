#!/bin/bash
source ~/.bashrc

cd ..

exe="wavefront_diagonal_allg_mpi"
TIMESTAMP=$(date +"%Y%m%d_%H%M%S")

RESULTS_DIR="test_mpi/$exe/$TIMESTAMP"
mkdir -p $RESULTS_DIR
RESULTS_FILE="$RESULTS_DIR/wavefront_runtimes.csv"

# Define the arrays
MATRIX_SIZES=(2048 2896 4096 5793 8192)
N_WORKERS=(1 2 4 8 16 32)

# Iterate over all executable files
echo "Name,Matrix size,Workers number,Time,Value" >> $RESULTS_FILE
for n_mat in "${MATRIX_SIZES[@]}"; do
  for n_w in "${N_WORKERS[@]}"; do
    echo "Running executable: $exe n_mat: $n_mat work: $n_w"

    command_output=$(mpirun -np $n_w $exe $n_mat)
    time=$(echo "$command_output" | grep "time:" | awk '{print $2}')
    value=$(echo "$command_output" | grep "last:" | awk '{print $2}')
    echo "$command_output"
    if ((time != -10)); then
      echo "$exe,$n_mat,$n_w,$time,$value" >> $RESULTS_FILE
    fi
    # Check if execution was successful
    if [ $? -ne 0 ]; then
      echo "Execution failed for $exe with N_MATRIX=$n_mat, N_WORKERS=$n_w"
    fi
  done
done