// TA_marking_2B_101258619_101166589.cpp

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
#include <semaphore.h>
#include <signal.h>

using namespace std;

//=====================================================================================
// Function Declarations are from TA_marking_101258619_101166589 are used here
// Only the functions that need to changed for Part 2B are implemented in this file
//=====================================================================================

// Global semaphore pointers
sem_t *sem_rubric_mutex = NULL;        // Mutex for rubric writing
sem_t *sem_reader_count_mutex = NULL;  // Mutex for reader count
sem_t *sem_exam_mutex = NULL;          // Mutex for exam loading
sem_t *sem_questions[MAX_EXCERCISES];  // Array of semaphores for questions

/*
 * Initializes all semaphore.
 */
int init_semaphores() {
    // Initialize rubric mutex semaphore
    cout << "Initializing rubric mutex semaphore..." << endl;
    fflush(stdout);

    // Clean up any existing semaphores
    sem_unlink(SEM_RUBRIC_MUTEX);
    sem_unlink(SEM_RUBRIC_READER_COUNT);
    sem_unlink(SEM_EXAM_MUTEX);
    for (int i = 0; i < MAX_EXCERCISES; i++)
    {
        char sem_name[50];
        snprintf(sem_name, sizeof(sem_name), "%s%d", SEM_QUESTION_PREFIX, i);
        sem_unlink(sem_name);
    }

    // Create rubric mutex semaphore
    sem_rubric_mutex = sem_open(SEM_RUBRIC_MUTEX, O_CREAT | O_EXCL, 0644, 1);
    if (sem_rubric_mutex == SEM_FAILED)
    {
        cerr << "Error creating rubric mutex semaphore" << endl;
        fflush(stdout);
        return 0;
    }
    
    
    // Create reader count mutex semaphore
    sem_reader_count_mutex = sem_open(SEM_RUBRIC_READER_COUNT, O_CREAT | O_EXCL, 0644, 1);
    if (sem_reader_count_mutex == SEM_FAILED)
    {
        cerr << "Error creating reader count mutex semaphore" << endl;
        fflush(stdout);
        return 0;
    }

    // Create exam mutex semaphore
    sem_exam_mutex = sem_open(SEM_EXAM_MUTEX, O_CREAT | O_EXCL, 0644, 1);
    if (sem_exam_mutex == SEM_FAILED)
    {
        cerr << "Error creating exam mutex semaphore" << endl;
        fflush(stdout);
        return 0;
    }

    // Create question semaphores
    for (int i = 0; i < MAX_EXCERCISES; i++){
        char sem_name[50];
        snprintf(sem_name, sizeof(sem_name), "%s%d", SEM_QUESTION_PREFIX, i);
        sem_questions[i] = sem_open(sem_name, O_CREAT | O_EXCL, 0644, 1);
        if (sem_questions[i] == SEM_FAILED)
        {
            cerr << "Error creating semaphore for question " << i << endl;
            fflush(stdout);
            return 0;
        }
    }

    cout << "All semaphores initialized successfully." << endl;
    fflush(stdout);
    return 1;
}

/*
 * Cleans up and removes all semaphores.
 */
void cleanup_semaphores() {
    cout << "Cleaning up semaphores..." << endl;
    fflush(stdout);

    // Close and unlink rubric mutex semaphore
    if (sem_rubric_mutex != NULL) {
        sem_close(sem_rubric_mutex);
        sem_unlink(SEM_RUBRIC_MUTEX);
    }

    // Close and unlink reader count mutex semaphore
    if (sem_reader_count_mutex != NULL) {
        sem_close(sem_reader_count_mutex);
        sem_unlink(SEM_RUBRIC_READER_COUNT);
    }

    // Close and unlink exam mutex semaphore
    if (sem_exam_mutex != NULL) {
        sem_close(sem_exam_mutex);
        sem_unlink(SEM_EXAM_MUTEX);
    }

    // Close and unlink question semaphores
    for (int i = 0; i < MAX_EXCERCISES; i++) {
        if (sem_questions[i] != NULL) {
            sem_close(sem_questions[i]);
            char sem_name[50];
            snprintf(sem_name, sizeof(sem_name), "%s%d", SEM_QUESTION_PREFIX, i);
            sem_unlink(sem_name);
        }
    }

    cout << "Semaphores cleaned up successfully." << endl;
    fflush(stdout);
}


void review_and_correct_rubric(int ta_id, pid_t pid, int *chance_of_correction,
                               SharedMemoryAccess *shm_ptr, const std::string &rubric_file) {
    cout << "[TA " << ta_id << "] (PID " << pid << ") Reviewing rubric..." << endl;
    fflush(stdout);

    for (int i = 0; i < MAX_EXCERCISES; i++)
    {
        cout << "[TA " << ta_id << "] (PID " << pid << ") Locked reader count mutex" << endl;
        fflush(stdout);
        sem_wait(sem_reader_count_mutex);
        shm_ptr->reader_count++;

        if (shm_ptr->reader_count == 1)
        {
            cout << "[TA " << ta_id << "] (PID " << pid << ") Locked rubric mutex" << endl;
            fflush(stdout);
            sem_wait(sem_rubric_mutex); // First reader locks the rubric for writing
        }

        cout << "[TA " << ta_id << "] (PID " << pid << ") Released reader count mutex" << endl;
        fflush(stdout);
        sem_post(sem_reader_count_mutex); // Release reader count mutex

        // Read rubric line
        cout << "[TA " << ta_id << "] (PID " << pid << ") Reading rubric for Exercise " 
             << shm_ptr->rubric.rubrics[i].exercise_num << ": " 
             << shm_ptr->rubric.rubrics[i].text << endl;
        fflush(stdout);

        random_delay(0.5, 1.0); // Simulate time taken to read
        int needs_correction = (rand() % 100) < *chance_of_correction;

        // Finished reading
        cout << "[TA " << ta_id << "] (PID " << pid << ") Locked reader count mutex" << endl;
        fflush(stdout);
        sem_wait(sem_reader_count_mutex); // Lock reader count mutex
        shm_ptr->reader_count--;
        if (shm_ptr->reader_count == 0)
        {
            cout << "[TA " << ta_id << "] (PID " << pid << ") Released rubric mutex" << endl;
            fflush(stdout);
            sem_post(sem_rubric_mutex); // Last reader releases the rubric for writing
        }
        cout << "[TA " << ta_id << "] (PID " << pid << ") Released reader count mutex" << endl;
        fflush(stdout);
        sem_post(sem_reader_count_mutex); // Release reader count mutex

        // Do correction if needed
        if (needs_correction)
        {
            cout << "[TA " << ta_id << "] (PID " << pid << ") Detected error in rubric for Question " 
                 << (i + 1) << ", correcting..." << endl;
            fflush(stdout);

            cout << "[TA " << ta_id << "] (PID " << pid << ") Locked rubric mutex for writing" << endl;
            fflush(stdout);
            sem_wait(sem_rubric_mutex); // Lock rubric for writing
            // TAs need to wait to write one at a time
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

            cout << "[TA " << ta_id << "] (PID " << pid << ") Released rubric mutex after writing" << endl;
            fflush(stdout);
            sem_post(sem_rubric_mutex); // Release rubric mutex after writing
        }
    }
    
    cout << "[TA " << ta_id << "] (PID " << pid << ") Finished reviewing rubric" << endl;
    fflush(stdout);
}

/*
 * Marks a question with semaphore protection.
 * Prevents multiple TAs from marking the same question.
 */
void mark_question(int ta_id, pid_t pid, SharedMemoryAccess *shm_ptr, int question_num) {
    cout << "[TA " << ta_id << "] (PID " << pid << ") Marking student " << shm_ptr->exam.student_id 
         << ", Question " << (question_num + 1) << "..." << endl;
    fflush(stdout);
    
    // Wait on question semaphore to mark
    cout << "[TA " << ta_id << "] (PID " << pid << ") Locked question " << (question_num + 1) << " semaphore" << endl;
    fflush(stdout);
    sem_wait(sem_questions[question_num]);

    // Mark question as in progress
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

    // Release question semaphore after marking
    sem_post(sem_questions[question_num]);
    cout << "[TA " << ta_id << "] (PID " << pid << ") Released question " << (question_num + 1) << " semaphore" << endl;
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
    cout << "[TA " << ta_id << "] (PID " << pid << ") Started TA process." << endl;
    fflush(stdout);
    
    srand(time(NULL) + ta_id); // generate random number for each TA

    // Semaphore initialization for child processes
    sem_rubric_mutex = sem_open(SEM_RUBRIC_MUTEX, 0);
    sem_reader_count_mutex = sem_open(SEM_RUBRIC_READER_COUNT, 0);
    sem_exam_mutex = sem_open(SEM_EXAM_MUTEX, 0);
    for (int i = 0; i < MAX_EXCERCISES; i++) {
        char sem_name[50];
        snprintf(sem_name, sizeof(sem_name), "%s%d", SEM_QUESTION_PREFIX, i);
        sem_questions[i] = sem_open(sem_name, 0);
    }
    cout << "[TA " << ta_id << "] (PID " << pid << ") Started working" << endl;
    fflush(stdout);

    while (!shm_ptr->is_processed) {
        review_and_correct_rubric(ta_id, pid, chance_of_correction, shm_ptr, rubric_file);

        int found_work = 0;
        for (int i = 0; i < MAX_EXCERCISES; i++) {
            // Try to mark unmarked question
            if (!shm_ptr->exam.questions_marked[i] && 
                !shm_ptr->exam.questions_in_progress[i]) {
                
                mark_question(ta_id, pid, shm_ptr, i); // Mark the question
                found_work = 1;
                break; // Review rubric again before next question
            }
        }
        if (!found_work) {
            int all_marked = 1;
            for (int i = 0; i < MAX_EXCERCISES; i++) {
                if (!shm_ptr->exam.questions_marked[i]) {
                    all_marked = 0;
                    break;
                }
            }
            if (all_marked) {
                // Lock exam loading
                cout << "[TA " << ta_id << "] (PID " << pid << ") Locked exam mutex" << endl;
                fflush(stdout);
                sem_wait(sem_exam_mutex);

                if (!shm_ptr->is_processed) {
                    shm_ptr->exam_idx++;
                    if(shm_ptr->exam_idx >= num_exams) {
                        shm_ptr->is_processed = 1;
                        cout << "[TA " << ta_id << "] (PID " << pid << ") All exams completed!" << endl;
                        fflush(stdout);
                        cout << "[TA " << ta_id << "] (PID " << pid << ") Released exam mutex" << endl;
                        fflush(stdout);
                        sem_post(sem_exam_mutex); // Release exam mutex
                        break;
                    }

                    cout << "[TA " << ta_id << "] (PID " << pid << ") Loading next exam: " 
                         << exam_files[shm_ptr->exam_idx] << endl;
                    fflush(stdout);

                    // Load exam into shared memory
                    if (!load_exam(shm_ptr->exam, exam_files[shm_ptr->exam_idx])) {
                        cout << "[TA " << ta_id << "] (PID " << pid << ") Failed to load exam" << endl;
                        fflush(stdout);
                        shm_ptr->is_processed = 1;
                        cout << "[TA " << ta_id << "] (PID " << pid << ") Released exam mutex" << endl;
                        fflush(stdout);
                        sem_post(sem_exam_mutex); // Release exam mutex
                        break;
                    }
                    
                    // Check for termination exam (9999)
                    if (strcmp(shm_ptr->exam.student_id, "9999") == 0) {
                        cout << "[TA " << ta_id << "] (PID " << pid << ") Found termination exam (9999), finishing..." << endl;
                        fflush(stdout);
                        shm_ptr->is_processed = 1;
                        cout << "[TA " << ta_id << "] (PID " << pid << ") Released exam mutex" << endl;
                        fflush(stdout);
                        sem_post(sem_exam_mutex); // Release exam mutex
                        break;
                    }
                    
                    cout << "[TA " << ta_id << "] (PID " << pid << ") Now marking exam for student " 
                        << shm_ptr->exam.student_id << endl;
                    fflush(stdout);
                }
                cout << "[TA " << ta_id << "] (PID " << pid << ") Released exam mutex" << endl;
                fflush(stdout);
                sem_post(sem_exam_mutex); // Release exam mutex
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
 * Main function with semaphore initialization.
 */
int main(int argc, char *argv[]) {
    if (argc < 3) {
        cout << "Usage: " << argv[0] << " <number_of_TAs> <chance_of_correction>" << endl;
        fflush(stdout);
        return 1;
    }
    
    int num_tas = atoi(argv[1]);
    if (num_tas < 2) {
        cout << "Error: Must have at least 2 TAs" << endl;
        fflush(stdout);
        return 1;
    }

    int chance_of_correction = atoi(argv[2]);
    if (chance_of_correction < 0 || chance_of_correction > 100) {
        cout << "[Main] Error: chance_of_correction must be between 0 and 100" << endl;
        fflush(stdout);
        return 1;
    }
    
    cout << "[Main] Starting TA marking system (Part 2B with Semaphores)" << endl;
    cout << "[Main] TAs: " << num_tas << ", Correction chance: " << chance_of_correction << "%" << endl;
    fflush(stdout);
    
    // Initialize semaphores FIRST
    if (!init_semaphores()) {
        cout << "[Main] Error: Failed to initialize semaphores" << endl;
        fflush(stdout);
        return 1;
    }
    
    std::string rubric_file = "rubric.txt";
    std::string exam_folder = "exams";
    
    std::string exam_files[MAX_EXAM_FILES];
    int num_exams = get_exam_files(exam_folder, exam_files, MAX_EXAM_FILES);
    
    if (num_exams == 0) {
        cout << "[Main] Error: No exam files found in " << exam_folder << endl;
        fflush(stdout);
        cleanup_semaphores();
        return 1;
    }
    
    cout << "[Main] Found " << num_exams << " exam files" << endl;
    fflush(stdout);
    
    int shm_id = shmget(SHM_KEY, SHM_SIZE, IPC_CREAT | 0666);
    if (shm_id < 0) {
        cerr << "[Main] Error: shmget failed" << endl;
        fflush(stdout);
        cleanup_semaphores();
        return 1;
    }
    
    SharedMemoryAccess *shm_ptr = (SharedMemoryAccess *)shmat(shm_id, NULL, 0);
    if (shm_ptr == (void *)-1) {
        cerr << "[Main] Error: shmat failed" << endl;
        fflush(stdout);
        cleanup_semaphores();
        return 1;
    }
    
    memset(shm_ptr, 0, SHM_SIZE);
    shm_ptr->exam_idx = 0;
    shm_ptr->is_processed = 0;
    shm_ptr->reader_count = 0;
    
    if (!load_rubric(shm_ptr->rubric, rubric_file)) {
        cout << "[Main] Error: Failed to load rubric" << endl;
        fflush(stdout);
        shmdt(shm_ptr);
        shmctl(shm_id, IPC_RMID, NULL);
        cleanup_semaphores();
        return 1;
    }
    
    if (!load_exam(shm_ptr->exam, exam_files[0])) {
        cout << "[Main] Error: Failed to load first exam" << endl;
        fflush(stdout);
        shmdt(shm_ptr);
        shmctl(shm_id, IPC_RMID, NULL);
        cleanup_semaphores();
        return 1;
    }
    
    cout << "[Main] Loaded first exam: student " << shm_ptr->exam.student_id << endl;
    fflush(stdout);
    
    pid_t ta_pids[num_tas];
    
    for (int i = 0; i < num_tas; i++) {
        pid_t pid = fork();
        
        if (pid < 0) {
            cerr << "[Main] Error: fork failed" << endl;
            fflush(stdout);
            for (int j = 0; j < i; j++) {
                kill(ta_pids[j], SIGTERM);
            }
            shmdt(shm_ptr);
            shmctl(shm_id, IPC_RMID, NULL);
            cleanup_semaphores();
            return 1;
        }
        else if (pid == 0) {
            ta_process(i + 1, getpid(), &chance_of_correction, shm_ptr, exam_files, num_exams, rubric_file);
            shmdt(shm_ptr);
            exit(0);
        }
        else {
            ta_pids[i] = pid;
        }
    }
    
    cout << "[MAIN] Waiting for all TAs to finish..." << endl;
    fflush(stdout);
    
    for (int i = 0; i < num_tas; i++) {
        int status;
        waitpid(ta_pids[i], &status, 0);
    }
    
    cout << "[MAIN] All TAs have finished. Cleaning up..." << endl;
    fflush(stdout);
    
    shmdt(shm_ptr);
    shmctl(shm_id, IPC_RMID, NULL);
    cleanup_semaphores();
    
    cout << "[MAIN] Program completed successfully" << endl;
    return 0;
}