// judger.h
#ifndef JUDGER_H
#define JUDGER_H

#include <string>

extern std::string PARTICIPANT_CODE; // = "Submit/probA_AC.cpp";
extern std::string INPUT_DIR;        // "problem/probA/testcases/";
extern std::string OUTPUT_DIR;       // "problem/probA/expected_outputs/";

void judge(int task_id, std::string dir_code, std::string INPUT_DIR,
           std::string OUTPUT_DIR);

#endif // JUDGER_H