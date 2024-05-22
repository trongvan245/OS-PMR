#include <atomic>
#include <chrono>
#include <csignal>
#include <cstdlib>
#include <iostream>
#include <thread>

// Function to run the external program
void runExternalProgram(const std::string &command,
                        std::atomic<bool> &finished) {
    // Run the command and set finished to true when done
    std::system(command.c_str());
    finished = true;
}

// Function to monitor the execution time
void monitorExecution(std::thread &programThread, std::atomic<bool> &finished,
                      int timeLimit) {
    using namespace std::chrono;

    // Start the timer
    auto start = steady_clock::now();

    while (true) {
        if (finished) {
            auto now = steady_clock::now();
            auto elapsed = duration_cast<seconds>(now - start).count();
            std::cout << "Program has finished in" << elapsed << '\n';
            // Program finished within the time limit
            return;
        }

        // Check the elapsed time
        auto now = steady_clock::now();
        auto elapsed = duration_cast<seconds>(now - start).count();

        if (elapsed >= timeLimit) {
            // Time limit exceeded, terminate the program
            std::cout << elapsed << '\n';
            std::cout << "Time limit exceeded. Terminating the program...\n";
            if (programThread.joinable()) {
                // Terminate the external process (this might need adjusting
                // based on the OS)
                // std::system(
                //     "pkill -f 'your_program_name'"); // This is for Unix-like
                //                                      // systems
                // For Windows, you might use something like:
                std::system("taskkill /F /IM simpleprob.exe");
            }
            return;
        }

        // Sleep for a short period to avoid busy-waiting
        std::this_thread::sleep_for(milliseconds(100));
    }
}

int main() {
    std::string command =
        ".\\simpleprob > output.txt"; // Replace with the actual command to run
                                      // your program
    int timeLimit = 5;                // Time limit in seconds

    // Atomic flag to indicate whether the program has finished
    std::atomic<bool> finished(false);

    // Start the external program in a separate thread
    std::thread programThread(runExternalProgram, command, std::ref(finished));

    // Monitor the execution time
    monitorExecution(programThread, finished, timeLimit);

    // Join the program thread if it is still running
    if (programThread.joinable()) {
        programThread.join();
    }

    std::cout << "Program finished or terminated.\n";
    return 0;
}