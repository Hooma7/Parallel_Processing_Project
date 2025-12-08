#!/bin/bash
EXECUTABLE="./implement_allgather"

PROC_COUNTS=(2 4 8 16 32 64)

echo ""
echo "Running INTRA-NODE tests..."
for n in "${PROC_COUNTS[@]}"; do
    echo "Testing with $n processes (intra-node)..."
    srun -n $n --nodes=1 $EXECUTABLE > intra_${n}.txt
    echo "  Output saved to intra_${n}.txt"
done

echo ""
echo "Running INTER-NODE tests..."
for n in "${PROC_COUNTS[@]}"; do
    ppn=$((n / 2))    
    echo "Testing with $n processes ($ppn per node, inter-node)..."
    srun -n $n --nodes=2 --ntasks-per-node=$ppn $EXECUTABLE > inter_${n}.txt
    echo "  Output saved to inter_${n}.txt"
done

echo ""
echo "=================================="
echo "All tests completed!"
