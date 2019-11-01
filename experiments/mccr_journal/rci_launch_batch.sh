#!/bin/bash

lines=`wc -l commands`;
jobs=`echo "($lines + 1000 - 1)/1000" | bc`

sbatch --array=1-$jobs rci_array_batch_run.sh
