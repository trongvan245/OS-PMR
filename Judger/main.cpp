#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <filesystem>

using namespace std;
namespace fs = std::filesystem;

const string PARTICIPANT_CODE = "participant_code.cpp";
const string EXECUTABLE = "participant_executable";
const string INPUT_DIR = "../testcases/";
const string OUTPUT_DIR = "../expected_outputs/";

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

bool compile() {
    string compile_command = "g++ " + PARTICIPANT_CODE + " -o " + EXECUTABLE;
    int result = system(compile_command.c_str());
    return result == 0;
}

bool runTestCase(const string &input_file, const string &expected_output_file) {
    // Construct the command to run the participant's executable with the given input
    string run_command = "./" + EXECUTABLE + " < " + INPUT_DIR + input_file + " > output.txt";
    
    // Execute the command
    int run_result = system(run_command.c_str());

    // Check if the execution was successful
    if (run_result != 0) {
        cerr << "Error running " << EXECUTABLE << endl;
        return false; // Execution failed
    } 
    
    // Construct the command to compare the output with the expected output
    string diff_command = "diff -w output.txt " + OUTPUT_DIR + expected_output_file + " > errorlog.txt";
    int diff_result = system(diff_command.c_str()); // Execute the diff command
    return diff_result == 0;    // Return true if diff found no differences, false otherwise
}

int main() {
    if (!compile()) {
        cerr << "Compilation failed." << endl;
        return 1;
    }

    vector<string> test_cases = getTestCases(INPUT_DIR);

    for (string test_case : test_cases) {
        string expected_output_file = "output";
        expected_output_file +=
            test_case[5]; // Assuming output files match input files
        expected_output_file += ".out";
        if (runTestCase(test_case, expected_output_file)) {
            cout << test_case << ": Passed TEST " << test_case << endl;
        } else {
            cout << test_case << ": Failed TEST " << test_case << endl;
        }

    }
    // Cleanup
    fs::remove("output.txt");
    fs::remove("errorlog.txt");
    fs::remove(EXECUTABLE);
    return 0;
}
