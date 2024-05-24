#include "judger.h"

#include <atomic>
#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <functional> // for std::hash
#include <iostream>
#include <string>
#include <thread>
#include <vector>

using namespace std;
namespace fs = std::filesystem;

const string EXECUTABLE = "participant_executable";

vector<string> getTestCases(const string &dir) {
    // For simplicity, hardcode the test case names
    // Ideally, this should dynamically read the filenames in the directory
    vector<string> testCases;
    for (const auto &entry : fs::directory_iterator(dir)) {
        if (entry.path().extension() == ".inp") {
            testCases.push_back(entry.path().filename().string());
        }
    }
    return testCases;
}

bool compile(int task_id, string dir_code) {
    string compile_command =
        "g++ " + dir_code + " -o " + EXECUTABLE + to_string(task_id);
    int result = system(compile_command.c_str());
    return result == 0;
}

void runExternalProgram(const std::string &command,
                        std::atomic<bool> &finished) {
    // Run the command and set finished to true when done
    std::system(command.c_str());
    finished = true;
}

void runTestCase(const string &input_file, const string &expected_output_file,
                 string &exit_code, int task_id, string INPUT_DIR) {
    // Construct the command to run the participant's executable with the given
    string par_output = "output" + to_string(task_id) + ".txt";
    string par_errorlog = "errorlog" + to_string(task_id) + ".txt";
    string par_EXECUTABLE = EXECUTABLE + to_string(task_id);
    string run_command = "./" + par_EXECUTABLE + " < " + INPUT_DIR +
                         input_file + " > " + par_output;

    int TIME_LIMIT = 2;

    // Atomic flag to indicate whether the program has finished
    atomic<bool> finished(false);

    // Start the external program in a separate thread
    thread programThread(runExternalProgram, run_command, ref(finished));

    // Monitor the execution time
    using namespace chrono;
    auto start = steady_clock::now();

    while (true) {
        if (finished) {
            // Program finished within the time limit
            programThread.join();
            break;
        }

        // Check the elapsed time
        auto now = steady_clock::now();
        auto elapsed = duration_cast<seconds>(now - start).count();

        if (elapsed >= TIME_LIMIT) {
            // Time limit exceeded, terminate the program
            // cerr << "Time limit exceeded. Terminating the program...\n";
            if (programThread.joinable()) {
                system(("pkill -f '" + EXECUTABLE + to_string(task_id) + "'")
                           .c_str()); // For Unix-like systems
                programThread.join();
            }
            exit_code = "TLE";
            return;
        }

        // Sleep for a short period to avoid busy-waiting
        this_thread::sleep_for(milliseconds(100));
    }

    // Construct the command to compare the output with the expected output
    string diff_command = "diff -w " + par_output + " " + expected_output_file +
                          " > " + par_errorlog;

    int diff_result = system(diff_command.c_str()); // Execute the diff command
    exit_code = (diff_result ? "WA" : "AC");
    return; // Return true if diff found no differences, false otherwise
}

/*
 * Errorcode:
 *      Accept - AC
 *      Wrong Answer - WA
 *      Time Limit Exit - TLEE
 *      Compile Error - CERR
 */

void judge(int task_id, string dir_code, string INPUT_DIR, string OUTPUT_DIR) {
    if (!compile(task_id, dir_code)) {
        cerr << "Compilation failed." << endl;
        return;
    }

    vector<string> test_cases = getTestCases(INPUT_DIR);

    string result = "AC";

    for (string test_case : test_cases) {
        string expected_output_file = OUTPUT_DIR + "output";
        string exit_code;
        expected_output_file +=
            test_case[5]; // Assuming output files match input files
        if (test_case[6] != '.')
            expected_output_file += test_case[6];
        expected_output_file += ".out";
        runTestCase(test_case, expected_output_file, exit_code, task_id,
                    INPUT_DIR);

        if (exit_code != "AC") {
            result = exit_code;
            break;
        }
    }

    std::hash<std::thread::id> hasher;
    auto hashed_id = hasher(this_thread::get_id());
    cout << "\n===================================================\n";
    cout << "Judge ID: " << hashed_id % 1000 << endl;
    cout << "Task " << task_id << ": " << result << '\n';
    cout << "===================================================\n\n";
    // Cleanup
    string par_output = "output" + to_string(task_id) + ".txt";
    string par_errorlog = "errorlog" + to_string(task_id) + ".txt";
    string par_EXECUTABLE = EXECUTABLE + to_string(task_id);

    fs::remove(par_output);
    fs::remove(par_errorlog);
    fs::remove(par_EXECUTABLE);

    return;
}