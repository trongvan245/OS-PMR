#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
using namespace std;

const string PARTICIPANT_CODE = "participant_code.cpp";
const string EXECUTABLE = "participant_executable";
const string INPUT_DIR = "../testcases/";
const string OUTPUT_DIR = "../expected_outputs/";

vector<string> getTestCases(const string &dir) {
    // For simplicity, hardcode the test case names
    // Ideally, this should dynamically read the filenames in the directory
    return {"input1.inp", "input2.inp"};
}

bool compile() {
    string compile_command = "g++ " + PARTICIPANT_CODE + " -o " + EXECUTABLE;
    int result = system(compile_command.c_str());
    return result == 0;
}

string readFile(const string &filename) {
    ifstream file(filename);
    string content((istreambuf_iterator<char>(file)),
                   istreambuf_iterator<char>());
    return content;
}

bool runTestCase(const string &input_file, const string &expected_output_file) {
    string run_command =
        ".\\" + EXECUTABLE + " < " + INPUT_DIR + input_file + " > output.txt";
    system(run_command.c_str());

    string par_out_dir = "output.txt";
    string judge_out_dir = OUTPUT_DIR + expected_output_file;

    return system(
               ("fc " + par_out_dir + " " + judge_out_dir + " > errorlog.txt")
                   .c_str()) == 0;

    // string participant_output = readFile("output.txt");
    // string expected_output = readFile(OUTPUT_DIR + expected_output_file);

    // return participant_output == expected_output;
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
            cout << test_case << ": Passed" << endl;
        } else {
            cout << test_case << ": Failed" << endl;
        }
        // Cleanup
        // string cleanup_command = "rm output.txt" + EXECUTABLE;
        // system(cleanup_command.c_str());
    }
    return 0;
}
