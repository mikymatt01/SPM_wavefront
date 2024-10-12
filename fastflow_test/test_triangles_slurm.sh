#!/bin/bash
#SBATCH --job-name=wavefront          # Job name
#SBATCH --output=output_%j.txt      # Standard output and error log (%j adds the job ID)
#SBATCH --error=error_%j.txt        # Error log
#SBATCH --partition=normal          # Partition (queue)
#SBATCH --nodes=4                   # Number of nodes to allocate
#SBATCH --requeue                   # Requeue job in case of failure

# Run your shell script
sh ./fastflow_test/test_triangles.sh