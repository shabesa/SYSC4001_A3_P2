/*
* TA_marking.cpp
* SYSC 4001 - Assignment 3 - Part 2 A
* By: Shabesa Kohilavani Arunprakash (101258619) & Manoj Kuppuswamy Thayagarajan (101166589)
* Implementation of TA marking system using shared memory.
* Each TA process reviews and corrects the rubric, then marks exam questions.
*/

#include "TA_marking_101258619_101166589.hpp"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <dirent.h>
#include <time.h>
#include <fcntl.h>

using namespace std;

//=====================================================================================
// Function Declarations are the common functions that are used in all the parts of the
// assignment. Helps to reduce code duplication.
//=====================================================================================

/*
 * Reviews the rubric and potentially corrects it.
 * Each line takes 0.5-1.0 seconds to review.
 * Corrections are saved to file.
 */
void review_and_correct_rubric(int ta_id, pid_t pid, int *chance_of_correction, SharedMemoryAccess *shm_ptr, const std::string &rubric_file) {
    cout << "[TA " << ta_id << "] (PID " << pid << ") Reviewing rubric..." << endl;
    fflush(stdout);
    
    // Iterate through all 5 rubric entries
    for (int i = 0; i < MAX_EXCERCISES; i++) {
        // Simulate review time (0.5-1.0 seconds)
        random_delay(0.5, 1.0);
        
        // Chance rubric needs correction based on chance_of_correction
        int needs_correction = (rand() % 100) < *chance_of_correction;
        
        if (needs_correction) {
            cout << "[TA " << ta_id << "] (PID " << pid << ") Detected error in rubric for Question " 
                 << (i + 1) << ", correcting..." << endl;
            fflush(stdout);

            // Multiple TAs can write simultaneously (race condition)
            char *rubric_text = shm_ptr->rubric.rubrics[i].text;
            if (strlen(rubric_text) > 0) {
                // Increment first character (e.g., 'C' -> 'D')
                rubric_text[0] = rubric_text[0] + 1;
                
                cout << "[TA " << ta_id << "] (PID " << pid << ") Corrected Question " << (i + 1) 
                     << " rubric to: " << rubric_text[0] << endl;
                fflush(stdout);
                
                // Save corrected rubric to file
                save_rubric(shm_ptr->rubric, rubric_file);
            }
        }
    }
    
    cout << "[TA " << ta_id << "] (PID " << pid << ") Finished reviewing rubric" << endl;
    fflush(stdout);
}

/*
 * Marks a single question on the exam.
 * Takes 1.0-2.0 seconds to mark.
 */
void mark_question(int ta_id, pid_t pid, SharedMemoryAccess *shm_ptr, int question_num) {
    cout << "[TA " << ta_id << "] (PID " << pid << ") Marking student " << shm_ptr->exam.student_id 
         << ", Question " << (question_num + 1) << "..." << endl;
    fflush(stdout);
    
    // Mark question as in progress with Race condition
    shm_ptr->exam.questions_in_progress[question_num] = 1;
    
    // Simulate marking time (1.0-2.0 seconds)
    random_delay(1.0, 2.0);
    
    // Mark as completed
    shm_ptr->exam.questions_marked[question_num] = 1;
    shm_ptr->exam.questions_in_progress[question_num] = 0;
    
    cout << "[TA " << ta_id << "] (PID " << pid << ") Completed marking student " 
         << shm_ptr->exam.student_id 
         << ", Question " << (question_num + 1) << endl;
    fflush(stdout);
}

/*
 * Main TA process function.
 * Each TA runs this as a separate process.
 */
void ta_process(int ta_id, pid_t pid, 
                int *chance_of_correction, SharedMemoryAccess *shm_ptr, 
                const std::string exam_files[], int num_exams,
                const std::string &rubric_file) {
    
    // Seed random number generator uniquely for each TA
    srand(time(NULL) + ta_id);
    
    cout << "[TA " << ta_id << "] (PID " << pid << ") Started working" << endl;
    fflush(stdout);
    
    // Continue until all exams are processed
    while (!shm_ptr->is_processed) {
        // Review and correct the rubric
        review_and_correct_rubric(ta_id, pid, chance_of_correction, shm_ptr, rubric_file);
        
        // Mark questions on current exam
        int found_work = 0;
        
        // Try to find an unmarked question
        for (int i = 0; i < MAX_EXCERCISES; i++) {
            // Race condition - multiple TAs can pick same question
            if (!shm_ptr->exam.questions_marked[i] && 
                !shm_ptr->exam.questions_in_progress[i]) {
                
                mark_question(ta_id, pid, shm_ptr, i); // Mark the question
                found_work = 1;
                break; // Review rubric again before next question
            }
        }
        
        // If no work found, check if all questions are done
        if (!found_work) {
            int all_marked = 1;
            for (int i = 0; i < MAX_EXCERCISES; i++) {
                if (!shm_ptr->exam.questions_marked[i]) {
                    all_marked = 0;
                    break;
                }
            }
            
            // If all questions marked, load next exam
            if (all_marked) {
                // Race condition - multiple TAs might load next exam
                shm_ptr->exam_idx++;
                
                // Check if all exams are processed
                if (shm_ptr->exam_idx >= num_exams) {
                    shm_ptr->is_processed = 1;
                    cout << "[TA " << ta_id << "] (PID " << pid << ") All exams completed!" << endl;
                    fflush(stdout);
                    break;
                }
                
                // Load next exam
                cout << "[TA " << ta_id << "] (PID " << pid << ") Loading next exam: " 
                     << exam_files[shm_ptr->exam_idx] << endl;
                fflush(stdout);
                
                // Load exam into shared memory
                if (!load_exam(shm_ptr->exam, exam_files[shm_ptr->exam_idx])) {
                    cout << "[TA " << ta_id << "] (PID " << pid << ") Failed to load exam" << endl;
                    fflush(stdout);
                    shm_ptr->is_processed = 1;
                    break;
                }
                
                // Check for termination exam (9999)
                if (strcmp(shm_ptr->exam.student_id, "9999") == 0) {
                    cout << "[TA " << ta_id << "] (PID " << pid << ") Found termination exam (9999), finishing..." << endl;
                    fflush(stdout);
                    shm_ptr->is_processed = 1;
                    break;
                }
                
                cout << "[TA " << ta_id << "] (PID " << pid << ") Now marking exam for student " 
                     << shm_ptr->exam.student_id << endl;
                fflush(stdout);
            } else {
                // Wait before checking again
                usleep(100000); // 100ms
            }
        }
    }
    
    cout << "[TA " << ta_id << "] (PID " << pid << ") Finished working" << endl;
    fflush(stdout);
}

/*
 * Main function - creates shared memory and spawns TA processes.
 */
int main(int argc, char *argv[]) {
    // Check command line arguments
    if (argc < 3) {
        cout << "Usage: " << argv[0] << " <number_of_TAs> <chance_of_correction>" << endl;
        fflush(stdout);
        return 1;
    }
    
    // Number of TAs has to be at least 2
    int num_tas = atoi(argv[1]);
    if (num_tas < 2) {
        cout << "Error: Must have at least 2 TAs" << endl;
        fflush(stdout);
        return 1;
    }

    // Chance of correction between 0 and 100
    int chance_of_correction = atoi(argv[2]);
    if (chance_of_correction < 0 || chance_of_correction > 100) {
        cout << "Error: chance_of_correction must be between 0 and 100" << endl;
        fflush(stdout);
        return 1;
    }
    
    cout << "Starting TA marking system with " << num_tas << " TAs and " << chance_of_correction << "% chance of correction to the rubric" << endl;
    fflush(stdout);
    
    // File paths
    std::string rubric_file = "rubric.txt";
    std::string exam_folder = "exams";
    
    // Get list of exam files
    std::string exam_files[MAX_EXAM_FILES];
    int num_exams = get_exam_files(exam_folder, exam_files, MAX_EXAM_FILES);
    
    // Check if any exam files were found
    if (num_exams == 0) {
        cout << "Error: No exam files found in " << exam_folder << endl;
        fflush(stdout);
        return 1;
    }
    
    cout << "Found " << num_exams << " exam files" << endl;
    fflush(stdout);
    
    // Create shared memory segment using System V IPC
    cout << "Creating shared memory segment..." << endl;
    int shm_id = shmget(SHM_KEY, SHM_SIZE, IPC_CREAT | 0666);
    if (shm_id < 0) {
        perror("shmget failed");
        return 1;
    }
    cout << "Shared memory created with ID: " << shm_id << endl;
    fflush(stdout);
    
    // Attach to shared memory
    SharedMemoryAccess *shm_ptr = (SharedMemoryAccess *)shmat(shm_id, NULL, 0);
    if (shm_ptr == (void *)-1) {
        perror("shmat failed");
        return 1;
    }
    cout << "Attached to shared memory" << endl;
    fflush(stdout);
    
    // Initialize shared memory
    memset(shm_ptr, 0, SHM_SIZE);
    shm_ptr->exam_idx = 0;
    shm_ptr->is_processed = 0;
    
    // Load initial rubric into shared memory
    if (!load_rubric(shm_ptr->rubric, rubric_file)) {
        cout << "Error: Failed to load rubric" << endl;
        fflush(stdout);
        shmdt(shm_ptr);
        shmctl(shm_id, IPC_RMID, NULL);
        return 1;
    }
    cout << "Loaded rubric into shared memory" << endl;
    fflush(stdout);
    
    // Load first exam into shared memory
    if (!load_exam(shm_ptr->exam, exam_files[0])) {
        cout << "Error: Failed to load first exam" << endl;
        fflush(stdout);
        shmdt(shm_ptr);
        shmctl(shm_id, IPC_RMID, NULL);
        return 1;
    }
    cout << "Loaded first exam: student " << shm_ptr->exam.student_id << endl;
    fflush(stdout);
    
    // Create TA processes
    pid_t ta_pids[num_tas];
    
    for (int i = 0; i < num_tas; i++) {
        pid_t pid = fork();
        
        if (pid < 0) {
            perror("fork failed");
            // Clean up created processes
            for (int j = 0; j < i; j++) {
                kill(ta_pids[j], SIGTERM);
            }
            shmdt(shm_ptr);
            shmctl(shm_id, IPC_RMID, NULL);
            return 1;
        }
        else if (pid == 0) {
            // Child process - TA
            ta_process(i + 1, getpid(), &chance_of_correction, shm_ptr, exam_files, num_exams, rubric_file);
            
            // Detach from shared memory before exiting
            shmdt(shm_ptr);
            exit(0);
        }
        else {
            // Parent process
            ta_pids[i] = pid;
        }
    }
    
    // Parent waits for all TAs to finish
    cout << "[MAIN] Waiting for all TAs to finish..." << endl;
    fflush(stdout);
    
    for (int i = 0; i < num_tas; i++) {
        int status;
        waitpid(ta_pids[i], &status, 0);
    }
    
    cout << "[MAIN] All TAs have finished. Cleaning up..." << endl;
    fflush(stdout);
    
    // Detach from shared memory
    if (shmdt(shm_ptr) == -1) {
        perror("shmdt failed");
        return 1;
    }
    
    // Remove shared memory segment
    if (shmctl(shm_id, IPC_RMID, NULL) == -1) {
        perror("shmctl failed");
        return 1;
    }
    
    cout << "[MAIN] Program completed successfully" << endl;
    return 0;
}