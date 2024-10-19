#!/bin/bash
source ~/.bashrc

cd ..

exe="wavefront_triangles_ff"
TIMESTAMP=$(date +"%Y%m%d_%H%M%S")

RESULTS_DIR="test_fastflow/$exe/$TIMESTAMP"
mkdir -p $RESULTS_DIR
RESULTS_FILE="$RESULTS_DIR/wavefront_runtimes.csv"

# Define the arrays
MATRIX_SIZES=(2048 2896 4096 5793 8192)
N_TRIANGLES=(1 2 4 8 16 32)
N_WORKERS=(1 2 4 8 16 32)

# Iterate over all executable files
echo "Name,Matrix size,Workers number,Triangles number,Time,Value" >> $RESULTS_FILE
for n_mat in "${MATRIX_SIZES[@]}"; do
    for n_w in "${N_WORKERS[@]}"; do
      echo "Running executable: $exe, work: $n_w n_mat: $n_mat n_tri: $n_w"

      command_output=$(srun "$exe" "$n_mat" "$n_w")
      time=$(echo "$command_output" | grep "time:" | awk '{print $2}')
      value=$(echo "$command_output" | grep "last:" | awk '{print $2}')

      if ((time != -10)); then
        echo "time: $time"
        echo "file: $RESULTS_FILE"
        echo "$exe,$n_mat,$n_w,$n_w,$time,$value" >> $RESULTS_FILE
      fi
      # Check if execution was successful
      if [ $? -ne 0 ]; then
        echo "Execution failed for $exe with N_MATRIX=$n_mat, N_TRIANGLES=$n_w, N_WORKERS=$n_w"
      fi
  done
done