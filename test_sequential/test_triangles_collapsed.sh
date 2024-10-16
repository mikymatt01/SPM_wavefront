#!/bin/bash
source ~/.bashrc

cd ..

exe="wavefront_triangles_collapsed"
TIMESTAMP=$(date +"%Y%m%d_%H%M%S")

RESULTS_DIR="test_sequential/$exe/$TIMESTAMP"
mkdir -p $RESULTS_DIR
RESULTS_FILE="$RESULTS_DIR/wavefront_runtimes.csv"

# Define the arrays
MATRIX_SIZES=(2048 2896 4096 5793 8192)
N_TRIANGLES=(1 2 4 8 16 32)

# List of executables

# Iterate over all executable files
echo "Name,Matrix size,Triangles number,Time,Value" >> $RESULTS_FILE
for n_mat in "${MATRIX_SIZES[@]}"; do
  for n_tri in "${N_TRIANGLES[@]}"; do
    echo "Running executable: $exe, n_mat: $n_mat n_tri: $n_tri"
    time=-10
    command_output=$(./"$exe" "$n_mat" "$n_tri")
    time=$(echo "$command_output" | grep "time:" | awk '{print $2}')
    value=$(echo "$command_output" | grep "last:" | awk '{print $2}')
    if ((time != -10)); then
      echo "time: $time"
      echo "file: $RESULTS_FILE"
      echo "$exe,$n_mat,$n_tri,$time,$value" >> $RESULTS_FILE
    fi
    # Check if execution was successful
    if [ $? -ne 0 ]; then
      echo "Execution failed for $exe with N_TRIANGLES=$n_tri, N_MATRIX=$n_mat"
    fi
  done
done