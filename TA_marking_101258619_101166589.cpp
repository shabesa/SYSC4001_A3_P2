// TA_marking.cpp
#include "TA_marking_101258619_101166589.hpp"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>

using namespace std;

//=====================================================================================
// Function Declarations are from TA_marking_101258619_101166589 are used here
// Only the functions that need to changed for Part 2A are implemented in this file
//=====================================================================================

/*
* Loads the rubric from a file into shared memory.
*/
int load_rubric(SharedMemoryRubric &rubric, const std::string &filename) {
    FILE *file = fopen(filename.c_str(), "r");
    if (file == NULL) {
        cout << "Error opening rubric file: " << filename << endl;
        fflush(stdout);
        return 0;
    }

    char line[256];
    int index = 0;

    while (fgets(line, sizeof(line), file) != NULL && index < MAX_EXCERCISES) {
        char *comma_pos = strchr(line, ',');
        if (comma_pos != NULL) {
            *comma_pos = '\0';
            rubric.rubrics[index].exercise_num = atoi(line);

            char *rubric_text = comma_pos + 1;
            while (*rubric_text == ' ' || *rubric_text == '\t') rubric_text++;

            char *newline_pos = strchr(rubric_text, '\n');
            if (newline_pos != NULL) {
                *newline_pos = '\0';
            }
            
            strncpy(rubric.rubrics[index].text, rubric_text, MAX_RUBRIC_TEXT_LENGTH - 1);
            rubric.rubrics[index].text[MAX_RUBRIC_TEXT_LENGTH - 1] = '\0';
            index++;
        }
    }

    fclose(file);
    return (index == MAX_EXCERCISES) ? 1 : 0;
}

/*
* Loads an exam from a file into shared memory.
*/
int load_exam(SharedMemoryExam &exam, const std::string &filename) {
    FILE *file = fopen(filename.c_str(), "r");
    if (file == NULL) {
        cout << "Error opening exam file: " << filename << endl;
        fflush(stdout);
        return 0;
    }

    char student_id[256];
    if (fgets(student_id, sizeof(student_id), file) != NULL) {
        char *newline_pos = strchr(student_id, '\n');
        if (newline_pos != NULL) {
            *newline_pos = '\0';
        }

        strncpy(exam.student_id, student_id, STUDENT_ID_LENGTH - 1);
        exam.student_id[STUDENT_ID_LENGTH - 1] = '\0';

        for (int i = 0; i < MAX_EXCERCISES; i++) {
            exam.questions_marked[i] = 0;
            exam.questions_in_progress[i] = 0;
        }

        fclose(file);
        return 1;
    }

    fclose(file);
    return 0;
}

/*
 * Saves the rubric from shared memory back to a file.
 */
int save_rubric(const SharedMemoryRubric &rubric, const std::string &filename) {
    FILE *file = fopen(filename.c_str(), "w");
    if (file == NULL) {
        cout << "Error opening rubric file for writing: " << filename << endl;
        fflush(stdout);
        return 0;
    }
    
    for (int i = 0; i < MAX_EXCERCISES; i++) {
        fprintf(file, "%d, %s\n", 
                rubric.rubrics[i].exercise_num,
                rubric.rubrics[i].text);
    }

    fclose(file);
    return 1;
}

/*
 * Gets list of exam files from directory.
 */
int get_exam_files(const std::string &exam_folder, std::string exam_files[], int max_files) {
    DIR *dir = opendir(exam_folder.c_str());
    
    if (dir == NULL) {
        cout << "Error opening exam folder: " << exam_folder << endl;
        fflush(stdout);
        return 0;
    }

    struct dirent *entry;
    int count = 0;

    while ((entry = readdir(dir)) != NULL && count < max_files) {
        std::string filename = entry->d_name;
        
        if (filename.find(".txt") != std::string::npos && 
            filename != "." && filename != "..") {
            exam_files[count] = exam_folder + "/" + filename;
            count++;
        }
    }

    closedir(dir);
    return count;
}

/*
 * Generates a random delay between min and max seconds.
 */
void random_delay(double min_seconds, double max_seconds) {
    double delay = min_seconds + ((double)rand() / RAND_MAX) * (max_seconds - min_seconds);
    usleep((useconds_t)(delay * 1000000));
}
