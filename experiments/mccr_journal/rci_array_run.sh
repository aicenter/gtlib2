#!/bin/bash
#SBATCH -c 1 # 1 core per task
#SBATCH -e slurm-%A_%a.err
#SBATCH -o slurm-%A_%a.out

cd /home/sustr/GTLib2

line=$SLURM_ARRAY_TASK_ID
evalLine=$(head -$line commands | tail -1)

ml CMake/3.13.3-GCCcore-8.2.0
srun -N 1 -n 1 $evalLine
