#!/bin/bash
source ~/.bashrc

cd ..
echo "Compiling programs..."
if [ $? -ne 0 ]; then
    echo "Compilation failed. Exiting."
    exit 1
fi
echo "Compilation successful."

exe="wavefront_diagonal_ff"
RESULTS_DIR="fastflow_test/$exe"
mkdir -p $RESULTS_DIR
RESULTS_FILE="$RESULTS_DIR/wavefront_runtimes.csv"

# Define the arrays
MATRIX_SIZES=(2048 2896 4096 5793 8192)
N_WORKERS=(1 2 4 8 16 32)

# Iterate over all executable files
echo "Name,Matrix size,Workers number,Map threads,Farm threads,Time,Value" >> $RESULTS_FILE
for n_mat in "${MATRIX_SIZES[@]}"; do
  for n_w in "${N_WORKERS[@]}"; do
    echo "Running executable: $exe, map: $n_map, farm: $n_farm, work: $n_w n_mat: $n_mat"

    command_output=$(./"$exe" "$n_mat" "$n_w")
    time=$(echo "$command_output" | grep "time:" | awk '{print $2}')
    value=$(echo "$command_output" | grep "last:" | awk '{print $2}')
    echo "$command_output"
    if ((time != -10)); then
      echo "$exe,$n_mat,$n_w,_,_,$time,$value" >> $RESULTS_FILE
    fi
    # Check if execution was successful
    if [ $? -ne 0 ]; then
      echo "Execution failed for $exe with N_TRIANGLES=$n_tri, N_WORKERS=$n_workers, N_MAP=$n_map, N_FARM=$n_farm"
    fi
  done
done