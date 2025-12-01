#!/bin/bash

# Create or clean bin directory
if [ ! -d "bin" ]; then
    mkdir bin
else
    rm -f bin/*
fi

# Compile the program
echo "Compiling TA_marking_2A..."
g++ -g -O0 -I . -o bin/TA_marking_2A TA_marking_101258619_101166589.cpp TA_marking_2A_101258619_101166589.cpp

# Check if compilation was successful
if [ $? -ne 0 ]; then
    echo "Compilation failed!"
    exit 1
fi

echo "Compilation successful!"
echo ""

# Ask user for input
echo "=== TA Marking System - Part 2A ==="
read -p "Enter number of TAs (minimum 2): " num_tas
read -p "Enter correction percentage (0-100): " correction_percent

# Run the program with user input
echo ""
echo "Running: ./bin/TA_marking_2A $num_tas $correction_percent"
echo "==========================================="
./bin/TA_marking_2A $num_tas $correction_percent
