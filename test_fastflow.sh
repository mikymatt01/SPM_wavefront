#!/bin/bash
source ~/.bashrc

echo "Compiling programs..."
make clean
make fastflow
if [ $? -ne 0 ]; then
    echo "Compilation failed. Exiting."
    exit 1
fi
echo "Compilation successful."

RESULTS_DIR="fastflow_test/$TIMESTAMP"
mkdir -p $RESULTS_DIR
RESULTS_FILE="$RESULTS_DIR/wavefront_runtimes.csv"

# Define the arrays
MATRIX_SIZES=(2048 2896 4096 5793)
N_TRIANGLES=(1 2 4 8 16 32 64)
N_WORKERS=(1 2 4 8 16 32 64)
N_MAP=(1 2 4 8 16 32 64)
N_FARM=(1 2 4 8 16 32 64)

# List of executables
EXECUTABLES=("wavefront_diagonal_ff" "wavefront_triangles_ff" "wavefront_triangles_map_ff_comm" "wavefront_triangles_map_ff")

# Iterate over all executable files

for n_mat in "${MATRIX_SIZES[@]}"; do
  for n_tri in "${N_TRIANGLES[@]}"; do
    for n_w in "${N_WORKERS[@]}"; do
      for n_map in "${N_MAP[@]}"; do
        for n_farm in "${N_FARM[@]}"; do
          for exe in "${EXECUTABLES[@]}"; do
            echo "Running executable: $exe"
            time=0
            case $exe in
              "wavefront_diagonal_ff")
                if (( n_map == 1)) && (( n_farm == 1 )) && ((n_tri == 1)); then
                  time=$(mpirun -np 1 ./"$exe" "$n_mat" "$n_w" | grep "time:" | awk '{print $2}')
                fi
                ;;
              "wavefront_triangles_ff")
                if (( n_map == 1)) && (( n_farm == 1 )); then
                  time=$(mpirun -np 1 ./"$exe" "$n_mat" "$n_tri" "$n_w" | grep "time:" | awk '{print $2}')
                fi
                ;;
              "wavefront_triangles_map_ff_comm")
                time=$(mpirun -np 1 ./"$exe" "$n_mat" "$n_tri" "$n_map" "$n_farm" | grep "time:" | awk '{print $2}')
                ;;
              "wavefront_triangles_map_ff")
                time=$(mpirun -np 1 ./"$exe" "$n_mat" "$n_tri" "$n_map" "$n_farm" | grep "time:" | awk '{print $2}')
                ;;
              *)
                echo "Unknown exe."
                ;;
            esac
            if ((time != 0)); then
              echo "$exe,$n_mat,$n_w,$n_map,$n_farm,$time" > $RESULTS_FILE
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