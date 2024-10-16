#!/bin/bash
source ~/.bashrc

cd ..

exe="wavefront_triangles_map_ff_comm"

RESULTS_DIR="test_fastflow/$exe"
mkdir -p $RESULTS_DIR
RESULTS_FILE="$RESULTS_DIR/wavefront_runtimes.csv"

# Define the arrays
MATRIX_SIZES=(2048 2896 4096 5793 8192)
N_TRIANGLES=(1 2 4 8 16 32)
N_WORKERS=(1 2 4 8 16 32)
N_MAP=(1 2 4 8 16 32)
N_FARM=(1 2 4 8 16 32)

# List of executables

# Iterate over all executable files
echo "Name,Matrix size,Workers number,Map threads,Farm threads,Triangles number,Time,Value" >> $RESULTS_FILE
for n_mat in "${MATRIX_SIZES[@]}"; do
  for n_tri in "${N_TRIANGLES[@]}"; do
    for n_w in "${N_WORKERS[@]}"; do
      for n_map in "${N_MAP[@]}"; do
        for n_farm in "${N_FARM[@]}"; do
            echo "Running executable: $exe, map: $n_map, farm: $n_farm, work: $n_w n_mat: $n_mat"
            time=-10
            if (( n_map * n_farm == n_w)); then
              command_output=$(./"$exe" "$n_mat" "$n_tri" "$n_map" "$n_farm")
              time=$(echo "$command_output" | grep "time:" | awk '{print $2}')
              value=$(echo "$command_output" | grep "last:" | awk '{print $2}')
            fi
            if ((time != -10)); then
              echo "time: $time"
              echo "file: $RESULTS_FILE"
              echo "$exe,$n_mat,$n_w,$n_map,$n_farm,$n_tri,$time,$value" >> $RESULTS_FILE
            fi
            # Check if execution was successful
            if [ $? -ne 0 ]; then
              echo "Execution failed for $exe with N_TRIANGLES=$n_tri, N_WORKERS=$n_workers, N_MAP=$n_map, N_FARM=$n_farm"
            fi
          done
        done
      done
    done
  done
done