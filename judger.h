// judger.h
#ifndef JUDGER_H
#define JUDGER_H

#include <string>

extern std::string PARTICIPANT_CODE;    // = "Submit/probA_AC.cpp";
extern std::string INPUT_DIR;           // "problem/probA/testcases/";
extern std::string OUTPUT_DIR;          // "problem/probA/expected_outputs/";

void judge(int task_id, int problem_id, std::string dir_code, std::string &exit_code, std::string &message);

// dir_code: /Submit/probA_Accept.cpp
// ProbA: probA_AC.cpp, probA_WA.cpp, probA_TLE.cpp

#endif // JUDGER_H