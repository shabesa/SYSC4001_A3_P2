# SYSC 4001 Assignment 3 - Part 

**By:** Shabesa Kohilavani Arunprakash **(101258619)** & Manoj Kuppuswamy Thayagarajan **(101166589)**

**Note:** The code was developed and tested on a Linux/Unix environment (Ubuntu/WSL). Ensure compatibility when running on different systems.

## Setup Overview

This setup implements a concurrent TA marking system that demonstrates:
- **Part 2A**: Concurrent processing with System V shared memory (race conditions present)
- **Part 2B**: Synchronized concurrent processing using POSIX semaphores (race conditions eliminated)

## Directory Structure

```
SYSC4001_A3_P2/
├── bin/                                         # Compiled executables (auto-generated)
├── exams/                                       # Exam files directory
│   ├── exam_0001.txt through exam_0020.txt      # Student exam files
│   └── exam_9999.txt                            # Termination signal file
├── rubric.txt                                   # Rubric file (5 exercises)
├── TA_marking_101258619_101166589.hpp           # Common header file
├── TA_marking_101258619_101166589.cpp           # Common functions for A & B
├── TA_marking_2A_101258619_101166589.cpp        # Part 2A (no semaphores)
├── TA_marking_2B_101258619_101166589.cpp        # Part 2B (with semaphores)
├── parta_build.sh                               # Build & run script for Part 2A
├── partb_build.sh                               # Build & run script for Part 2B
└── README.md                                    # This file
```

## Prerequisites

- **Operating System**: Linux/Unix (tested on Ubuntu/WSL)
- **Compiler**: g++
- **Libraries**: pthread (for Part 2B semaphores)
- **Permissions**: Executable permissions for build scripts

## Setup Instructions

### 1. File Permissions

Make the build scripts executable:
```bash
chmod +x parta_build.sh
chmod +x partb_build.sh
```

### 2. Rubric File

The `rubric.txt` file should contain:
```
1, A
2, B
3, C
4, D
5, E
```

Format: `<exercise_number>, <rubric_text>`

### 3. Exam Files

The `exams/` directory contains:
- **exam_0001.txt to exam_0020.txt**: Regular exam files (20 exams minimum)
- **exam_9999.txt**: Special termination file

Each exam file format:
```
<4-digit student ID>
```

Example `exam_0001.txt`:
```
0001
```

**Important**: The file `exam_9999.txt` must exist with content `9999` to signal program termination.

## Compilation and Execution

### Part 2A: Without Semaphores (Race Conditions Present)

**Method 1: Using Build Script (Recommended)**
```bash
./parta_build.sh
```

The script will:
1. Create/clean the `bin/` directory
2. Compile Part 2A
3. Prompt for number of TAs (minimum 2)
4. Prompt for correction percentage (0-100)
5. Run the program

**Method 2: Manual Compilation**
```bash
# Create bin directory
mkdir -p bin

# Compile
g++ -g -O0 -I . -o bin/TA_marking_2A \
    TA_marking_101258619_101166589.cpp \
    TA_marking_2A_101258619_101166589.cpp

# Run
./bin/TA_marking_2A <num_TAs> <correction_percent>
```

**Example:**
```bash
./bin/TA_marking_2A 3 30
```
- 3 TAs working concurrently
- 30% chance of rubric correction per exercise

### Part 2B: With Semaphores (Synchronized)

**Method 1: Using Build Script (Recommended)**
```bash
./partb_build.sh
```

The script will:
1. Create/clean the `bin/` directory
2. Compile Part 2B with pthread
3. Prompt for number of TAs (minimum 2)
4. Prompt for correction percentage (0-100)
5. Run the program

**Method 2: Manual Compilation**
```bash
# Create bin directory
mkdir -p bin

# Compile (note: -lpthread required)
g++ -g -O0 -I . -o bin/TA_marking_2B \
    TA_marking_101258619_101166589.cpp \
    TA_marking_2B_101258619_101166589.cpp \
    -lpthread

# Run
./bin/TA_marking_2B <num_TAs> <correction_percent>
```

**Example:**
```bash
./bin/TA_marking_2B 5 50
```
- 5 TAs working concurrently
- 50% chance of rubric correction per exercise

## Test Cases

### Recommended Test Scenarios

#### 1. Basic Functionality Test
```bash
./bin/TA_marking_2A 2 10
./bin/TA_marking_2B 2 10
```
- Minimum TAs (2)
- Low correction rate (10%)
- Tests basic operation

#### 2. Moderate Load Test
```bash
./bin/TA_marking_2A 5 30
./bin/TA_marking_2B 5 30
```
- Multiple TAs (5)
- Moderate correction rate (30%)
- Tests typical workload

#### 3. High Contention Test
```bash
./bin/TA_marking_2A 10 50
./bin/TA_marking_2B 10 50
```
- Many TAs (10)
- High correction rate (50%)
- Stress test for race conditions (2A) vs synchronization (2B)

#### 4. Race Condition Detection (Part 2A)
```bash
./bin/TA_marking_2A 8 70
```
- Many TAs competing
- High correction frequency
- **Expected**: Race conditions visible (duplicate question marking, lost rubric updates)

#### 5. Correctness Verification (Part 2B)
```bash
./bin/TA_marking_2B 8 70
```
- Same parameters as Test 4
- **Expected**: No race conditions (proper synchronization)

#### 6. Edge Case: No Corrections
```bash
./bin/TA_marking_2A 3 0
./bin/TA_marking_2B 3 0
```
- 0% correction rate
- Tests marking without rubric modifications

#### 7. Edge Case: Always Correct
```bash
./bin/TA_marking_2A 3 100
./bin/TA_marking_2B 3 100
```
- 100% correction rate
- Tests heavy rubric contention


## Key Differences: Part 2A vs 2B

| Aspect | Part 2A | Part 2B |
|--------|---------|---------|
| **Synchronization** | None | POSIX Semaphores |
| **Rubric Access** | Race conditions | Readers-Writers pattern |
| **Question Marking** | Duplicates possible | Per-question mutexes |
| **Exam Loading** | Multiple TAs may load | Single TA loads |
| **Performance** | Faster | Slightly slower (sync overhead) |

## Design Details

### Part 2A: Race Conditions

1. **Rubric Corrections**: Multiple TAs can modify simultaneously → lost updates
2. **Question Marking**: Multiple TAs may mark same question → wasted work
3. **Exam Loading**: Multiple TAs may load next exam → inconsistent state

### Part 2B: Synchronization Mechanisms

1. **Readers-Writers Semaphores**:
   - `sem_rubric_mutex`: Exclusive write access
   - `sem_reader_count_mutex`: Protects reader count variable
   - Multiple readers allowed, single writer

2. **Per-Question Mutexes**:
   - `sem_question_0` through `sem_question_4`
   - One semaphore per question
   - Prevents duplicate marking

3. **Exam Loading Mutex**:
   - `sem_exam_mutex`
   - Ensures only one TA transitions to next exam

## Notes

- Both programs use the same shared memory key (1234) but should not run simultaneously
- Part 2A demonstrates why synchronization is necessary
- Part 2B shows proper concurrent programming with semaphores
- The `rubric.txt` file will be modified during execution as TAs processes make corrections
- Reset `rubric.txt` to original state between test runs for consistent results
- Ensure `exam_9999.txt` exists to signal termination, it prevents infinite loops and ensures the correct shutdown of processes