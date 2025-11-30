#include "ta_marking.hpp"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <dirent.h>
#include <time.h>

using namespace std;

/*
* Loads the rubric from a file into shared memory.
* format of the file: "exercise_num, rubric_text"
*/
int load_rubric(SharedMemoryRubric &rubric, const std::string &filename) {
    FILE *file = fopen(filename.c_str(), "r"); // Open the file for reading
    if (file == NULL) {
        cout << "Error opening rubric file: " << filename << endl;
        fflush(stdout);
        return 0;
    }

    char line[256];
    int index = 0;

    // Read each line from the file
    while (fgets(line, sizeof(line), file) != NULL && index < MAX_EXCERCISES) {
        // Parse the exercise number and rubric text
        char *comma_pos = strchr(line, ',');
        if (comma_pos != NULL) {
            *comma_pos = '\0'; // Split the string at the comma
            rubric.rubrics[index].exercise_num = atoi(line);

            char *rubric_text = comma_pos + 1;
            // Remove leading whitespace
            while (*rubric_text == ' ' || *rubric_text == '\t') rubric_text++;

            // Remove trailing newline character
            char *newline_pos = strchr(rubric_text, '\n');
            if (newline_pos != NULL) {
                *newline_pos = '\0';
            }
            
            strncpy(rubric.rubrics[index].text, rubric_text, MAX_RUBRIC_TEXT_LENGTH - 1);
            rubric.rubrics[index].text[MAX_RUBRIC_TEXT_LENGTH - 1] = '\0'; // Ensure null-termination

            index++; // Move to the next rubric entry
        }
    }

    fclose(file); // Close the file
    return (index == MAX_EXCERCISES) ? 1 : 0; // Return 1 if all rubrics were loaded, else 0
}


/*
* Loads an exam from a file into shared memory.
*/
int load_exam(SharedMemoryExam &exam, const std::string &filename) {
    FILE *file = fopen(filename.c_str(), "r"); // Open the file for reading
    if (file == NULL) {
        cout << "Error opening exam file: " << filename << endl;
        fflush(stdout);
        return 0;
    }

    char student_id[256]; // FIXED: Made buffer larger to read full line
    if (fgets(student_id, sizeof(student_id), file) != NULL) {
        // Remove trailing newline character
        char *newline_pos = strchr(student_id, '\n');
        if (newline_pos != NULL) {
            *newline_pos = '\0';
        }

        strncpy(exam.student_id, student_id, STUDENT_ID_LENGTH - 1); // Copy student ID to shared memory
        exam.student_id[STUDENT_ID_LENGTH - 1] = '\0'; // Ensure null-termination

        // Initialize questions_marked and questions_in_progress to 0
        for (int i = 0; i < MAX_EXCERCISES; i++) {
            exam.questions_marked[i] = 0;
            exam.questions_in_progress[i] = 0;
        }

        fclose(file); // Close the file
        return 1; // Successfully loaded the exam
    }

    fclose(file); // Close the file
    return 0; // Failed to load the exam
}

/*
 * Main function to test load_rubric and load_exam
 */
int main() {
    cout << "=== Testing load_rubric and load_exam ===" << endl << endl;

    // Test 1: Load Rubric
    cout << "Test 1: Loading rubric from 'rubric.txt'" << endl;
    SharedMemoryRubric test_rubric;
    
    int rubric_result = load_rubric(test_rubric, "rubric.txt");
    
    if (rubric_result == 1) {
        cout << "SUCCESS: Rubric loaded successfully!" << endl;
        cout << "Rubric contents:" << endl;
        for (int i = 0; i < MAX_EXCERCISES; i++) {
            cout << test_rubric.rubrics[i].exercise_num 
                << ": " << test_rubric.rubrics[i].text << endl;
        }
    } else {
        cout << "FAILED: Could not load rubric" << endl;
    }
    cout << endl;

    // Test 2: Load Exam
    cout << "Test 2: Loading exam from 'exams/exam_0001.txt'" << endl;
    SharedMemoryExam test_exam;
    
    int exam_result = load_exam(test_exam, "exams/exam_0001.txt");
    
    if (exam_result == 1) {
        cout << "SUCCESS: Exam loaded successfully!" << endl;
        cout << "Student ID: " << test_exam.student_id << endl;
        cout << "Questions marked: ";
        for (int i = 0; i < MAX_EXCERCISES; i++) {
            cout << test_exam.questions_marked[i] << " ";
        }
        cout << endl;
        cout << "Questions in progress: ";
        for (int i = 0; i < MAX_EXCERCISES; i++) {
            cout << test_exam.questions_in_progress[i] << " ";
        }
        cout << endl;
    } else {
        cout << "FAILED: Could not load exam" << endl;
    }
    cout << endl;

    // Test 3: Load multiple exams
    cout << "Test 3: Loading multiple exams" << endl;
    string exam_files[] = {
        "exams/exam_0001.txt",
        "exams/exam_0002.txt",
        "exams/exam_0003.txt"
    };
    
    for (int i = 0; i < 3; i++) {
        SharedMemoryExam exam;
        if (load_exam(exam, exam_files[i]) == 1) {
            cout << "  Loaded exam " << (i+1) << " - Student ID: " << exam.student_id << endl;
        } else {
            cout << "  Failed to load exam " << (i+1) << ": " << exam_files[i] << endl;
        }
    }
    cout << endl;

    cout << "=== Testing Complete ===" << endl;
    return 0;
}