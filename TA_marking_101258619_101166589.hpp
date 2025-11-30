#ifndef TA_MARKING_HPP
#define TA_MARKING_HPP

#include <string>
#include <sys/ipc.h>
#include <sys/types.h>

// Maximum number of excercises per exam
#define MAX_EXCERCISES 5

// Length of the Student ID
#define STUDENT_ID_LENGTH 5

// Rubric Texts
#define MAX_RUBRIC_TEXT_LENGTH 10

// Shared memory keys
#define SHM_KEY 1234
#define SHM_SIZE sizeof(SharedMemoryAccess)

// Maximum number of exam files
#define MAX_EXAM_FILES 100

/*
 * Structure for the rubric of an exercise
 */
struct Rubric {
    int exercise_num; // Exercise number (1 - 5)
    char text[MAX_RUBRIC_TEXT_LENGTH]; // Rubric text
};

/*
 * Structure for rubric in shared memory
 */
struct SharedMemoryRubric {
    Rubric rubrics[MAX_EXCERCISES]; // Array of rubrics for each exercise
};

/*
 * Structure for storing exam in shared memory
 */
struct SharedMemoryExam {
    char student_id[STUDENT_ID_LENGTH]; // Student ID
    int questions_marked[MAX_EXCERCISES]; // Keep track of marked questions
    int questions_in_progress[MAX_EXCERCISES]; // Keep track of questions in progress
};

/*
 * Structure for Shared Memory Access
*/
struct SharedMemoryAccess {
    SharedMemoryRubric rubric; // Pointer to rubric in shared memory
    SharedMemoryExam exam; // Pointer to exam in shared memory
    int exam_idx; // Index of the current exam
    int is_processed; // Flag to indicate if the exams are processed
};

// Function Declarations

/*
 * Loads the rubric from a file into shared memory.
 * @param rubric Reference to the SharedMemoryRubric structure.
 * @param filename Name of the file containing the rubric.
 * @return 1 if the rubric was loaded successfully, 0 otherwise.
 */
int load_rubric(SharedMemoryRubric &rubric, const std::string &filename);

/*
 * Loads an exam from a file into shared memory.
 * @param exam Reference to the SharedMemoryExam structure.
 * @param filename Name of the file containing the exam.
 * @return 1 if the exam was loaded successfully, 0 otherwise.
 */
int load_exam(SharedMemoryExam &exam, const std::string &filename);

/*
 * Saves the rubric from shared memory back to a file.
 * @param rubric Reference to the SharedMemoryRubric structure.
 * @param filename Name of the file to save the rubric to.
 * @return 1 if the rubric was saved successfully, 0 otherwise.
 */
int save_rubric(const SharedMemoryRubric &rubric, const std::string &filename);

/*
 * Gets a list of all exam files in a directory.
 * @param exam_folder Path to the folder containing exam files.
 * @param exam_files Array to store the filenames.
 * @param max_files Maximum number of files to retrieve.
 * @return Number of exam files found.
 */
int get_exam_files(const std::string &exam_folder, std::string exam_files[], int max_files);

/*
 * Reviews the rubric and potentially corrects it.
 * Each line takes 0.5-1.0 seconds to review.
 * @param ta_id Unique identifier for the TA.
 * @param pid Process ID of the TA process.
 * @param chance_of_correction Pointer to the chance of correction percentage.
 * @param shm_ptr Pointer to the shared memory structure.
 * @param rubric_file Path to the rubric file.
 */
void review_and_correct_rubric(int ta_id, pid_t pid, int *chance_of_correction, SharedMemoryAccess *shm_ptr, const std::string &rubric_file);

/*
 * Marks a single question on the exam.
 * Takes 1.0-2.0 seconds to mark.
 * @param ta_id Unique identifier for the TA.
 * @param shm_ptr Pointer to the shared memory structure.
 * @param question_num Question number to mark (0-4).
 */
void mark_question(int ta_id, pid_t pid, SharedMemoryAccess *shm_ptr, int question_num);

/*
 * Main TA process function.
 * Each TA runs this function as a separate process.
 * @param ta_id Unique identifier for the TA.
 * @param pid Process ID of the TA process.
 * @param chance_of_correction Pointer to the chance of correction percentage.
 * @param shm_ptr Pointer to the shared memory structure.
 * @param exam_files Array of exam filenames.
 * @param num_exams Number of exam files.
 * @param rubric_file Path to the rubric file.
 */
void ta_process(int ta_id, pid_t pid, 
                int *chance_of_correction, SharedMemoryAccess *shm_ptr,
                const std::string exam_files[], int num_exams,
                const std::string &rubric_file);

/*
 * Generates a random delay between min and max seconds.
 * Uses usleep to simulate time taken for operations.
 * @param min_seconds Minimum delay in seconds.
 * @param max_seconds Maximum delay in seconds.
 */
void random_delay(double min_seconds, double max_seconds);

#endif // TA_MARKING_HPP