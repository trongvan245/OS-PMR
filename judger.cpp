#include "judger.h"

#include <atomic>
#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

using namespace std;
namespace fs = std::filesystem;

const string PARTICIPANT_CODE = "Submit/probA_AC.cpp";
const string EXECUTABLE = "participant_executable";
const string INPUT_DIR = "problem/probA/testcases/";
const string OUTPUT_DIR = "problem/probA/expected_outputs/";

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

bool compile(int task_id) {
    string compile_command =
        "g++ " + PARTICIPANT_CODE + " -o " + EXECUTABLE + to_string(task_id);
    int result = system(compile_command.c_str());
    return result == 0;
}

void monitorExecution(thread &programThread, atomic<bool> &finished,
                      int timeLimit) {
    using namespace chrono;

    // Start the timer
    auto start = steady_clock::now();

    while (true) {
        if (finished) {
            auto now = steady_clock::now();
            auto elapsed = duration_cast<seconds>(now - start).count();
            cout << "Program has finished in" << elapsed << '\n';
            // Program finished within the time limit
            return;
        }

        // Check the elapsed time
        auto now = steady_clock::now();
        auto elapsed = duration_cast<seconds>(now - start).count();

        if (elapsed >= timeLimit) {
            // Time limit exceeded, terminate the program
            cout << elapsed << '\n';
            cout << "Time limit exceeded. Terminating the program...\n";
            if (programThread.joinable()) {
                // Terminate the external process (this might need adjusting
                // based on the OS)
                // system(
                //     "pkill -f 'your_program_name'"); // This is for Unix-like
                //                                      // systems
                // For Windows, you might use something like:
                system("taskkill /F /IM simpleprob.exe");
            }
            return;
        }

        // Sleep for a short period to avoid busy-waiting
        this_thread::sleep_for(milliseconds(100));
    }
}

void runExternalProgram(const std::string &command,
                        std::atomic<bool> &finished) {
    // Run the command and set finished to true when done
    std::system(command.c_str());
    finished = true;
}

void runTestCase(const string &input_file, const string &expected_output_file,
                 string &exit_code, int task_id) {
    // Construct the command to run the participant's executable with the given

    string par_output = "output" + to_string(task_id) + ".txt";
    string par_errorlog = "errorlog" + to_string(task_id) + ".txt";
    string par_EXECUTABLE = EXECUTABLE + to_string(task_id);
    string run_command = "./" + par_EXECUTABLE + " < " + INPUT_DIR +
                         input_file + " > " + par_output;

    // Execute the command
    int run_result = system(run_command.c_str());
    // cout << run_command << '\n';

    // Check if the execution was successful
    if (run_result != 0) {
        cerr << "Error running " << EXECUTABLE << endl;
        exit_code = "CERR";
        return; // Execution failed
    }

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
            cerr << "Time limit exceeded. Terminating the program...\n";
            if (programThread.joinable()) {
                system(("pkill -f '" + EXECUTABLE + "'")
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
    string diff_command = "diff -w " + par_output + " " + OUTPUT_DIR +
                          expected_output_file + " > " + par_errorlog;

    // cout << diff_command << '\n';

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

void judge(int task_id, int problem_id, string dir_code, string &exit_code,
           string &message) {
    if (!compile(task_id)) {
        exit_code = "CERR";
        message = "Compile error";
        return;
        //  cerr << "Compilation failed." << endl;
        // return 1;
    }
    cout << "Judge ID: " << this_thread::get_id() << endl;

    vector<string> test_cases = getTestCases(INPUT_DIR);

    int i = 0;
    for (string test_case : test_cases) {
        string expected_output_file = "output";
        string exit_code;
        expected_output_file +=
            test_case[5]; // Assuming output files match input files
        expected_output_file += ".out";
        runTestCase(test_case, expected_output_file, exit_code, task_id);

        cout << "Task " << task_id << ": " << ++i << ". test is " << exit_code
             << '\n';
        // if (runTestCase(test_case, expected_output_file, exit_code)) {
        //     cout << ++i << ". Passed TEST " << test_case << endl;
        // } else {
        //     cout << ++i << ". Failed TEST " << test_case << endl;
        // }
    }
    // Cleanup
    string par_output = "output" + to_string(task_id) + ".txt";
    string par_errorlog = "errorlog" + to_string(task_id) + ".txt";
    string par_EXECUTABLE = EXECUTABLE + to_string(task_id);

    fs::remove(par_output);
    fs::remove(par_errorlog);
    fs::remove(par_EXECUTABLE);

    return;
}