#!/bin/bash
source ~/.bashrc

cd ..

exe="wavefront_diagonal"

RESULTS_DIR="test_sequential/$exe"
mkdir -p $RESULTS_DIR
RESULTS_FILE="$RESULTS_DIR/wavefront_runtimes.csv"

# Define the arrays
MATRIX_SIZES=(2048 2896 4096 5793 8192)

# Iterate over all executable files
echo "Name,Matrix size,Time,Value" >> $RESULTS_FILE
for n_mat in "${MATRIX_SIZES[@]}"; do
  echo "Running executable: $exe, n_mat: $n_mat"

  command_output=$(./"$exe" "$n_mat")
  time=$(echo "$command_output" | grep "time:" | awk '{print $2}')
  value=$(echo "$command_output" | grep "last:" | awk '{print $2}')
  echo "$command_output"
  if ((time != -10)); then
    echo "$exe,$n_mat,$time,$value" >> $RESULTS_FILE
  fi
  # Check if execution was successful
  if [ $? -ne 0 ]; then
    echo "Execution failed for $exe with N_TRIANGLES=$n_tri"
  fi
done