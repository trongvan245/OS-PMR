#include "judger.h"
#include <thread> // for multithreading
#include <random>

void judge(int problem_id, string dir_code, string &exit_code) {
    // Simulate some work
    cout << "Judging problem " << problem_id << " with code in " << dir_code << " in thread " << this_thread::get_id() << endl; 
    int random = rand() % 3;
    switch (random) {
        case 0:
            exit_code = "AC";
            break;
        case 1:
            exit_code = "WA";
            break;
        case 2:
            exit_code = "TLE";
            break;
    }
    this_thread::sleep_for(chrono::milliseconds(100));
}