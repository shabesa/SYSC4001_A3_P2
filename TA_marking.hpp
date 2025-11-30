#ifndef TA_MARKING_HPP
#define TA_MARKING_HPP

#include <string>

// Maximum number of excercises per exam
#define MAX_EXCERCISES 5

// Length of the Student ID
#define STUDENT_ID_LENGTH 5

// Rubric Texts
#define MAX_RUBRIC_TEXT_LENGTH 10

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



#endif // TA_MARKING_HPP