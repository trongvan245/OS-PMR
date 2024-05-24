#include "judger.h"

#include <chrono>             // for time measurement
#include <condition_variable> // for condition_variable
#include <fstream>            // for read request file
#include <functional>         // for std::ref
#include <iostream>           // for std::cout, std::cerr
#include <mutex>              // for std::mutex
#include <queue>              // for std::queue
#include <string>             // for std::string
#include <thread>             // for multithreading
#include <vector>             // for std::vector

using namespace std;

std::string PARTICIPANT_CODE = "Submit/probA_AC.cpp";
std::string INPUT_DIR = "problem/probA/testcases/";
std::string OUTPUT_DIR = "problem/probA/expected_outputs/";

// The print function represents a task that takes a string reference as input
// and prints it.
void print(string &s) {
    cout << "=====" << s << " is running on thread " << this_thread::get_id()
         << endl;
    // Simulate some work
    this_thread::sleep_for(chrono::milliseconds(100));
}

class ThreadPool {
  private:
    // A list of threads in the pool
    vector<thread> threads;
    vector<bool> idle_thread;

    // A queue of tasks
    queue<function<void()>> tasks;

    // Mutex and condition variable for synchronization
    mutex queue_mutex;
    condition_variable condition;

    // A flag to stop the pool
    bool stop;

  public:
    /*
    Constructor:
    - init stop flag to False, meaning the pool is running
    - create a number of threads and add them to the pool, each thread
    have a while loop, that will keep the thread running until the pool
    is stopped. The thread will wait for a task to be added to the queue
    and then execute the task.
    */
    ThreadPool(int num_threads) : stop(false) {
        // set up idle_thread
        for (int i = 0; i < num_threads; ++i) idle_thread.push_back(true);

        // For each thread, create a lambda function that will keep the
        // thread running
        for (int i = 0; i < num_threads; i++) {
            // Create a thread, add to the list of threads
            threads.emplace_back([this, i] {
                while (true) {
                    // Create a task variable, which has no task initially
                    function<void()> task;

                    {
                        // Lock the mutex in this scope, to protect the tasks
                        // queue
                        unique_lock<mutex> lock(this->queue_mutex);

                        // Wait for a task to be added to the queue, or the pool
                        // to be stopped
                        this->condition.wait(lock, [this] {
                            return this->stop || !this->tasks.empty();
                        });

                        // If the pool is stopped and the queue is empty, then
                        // return
                        if (this->stop && this->tasks.empty())
                            return;

                        // Get the nearest task from the queue
                        task = move(this->tasks.front());

                        // thread gets task => idle = false
                        this->idle_thread[i] = false;

                        // Remove the task from the queue
                        this->tasks.pop();
                    }

                    // Execute the task
                    task();

                    // finish task => idle = true
                    this->idle_thread[i] = true;
                }
            });
        }
    }

    /*
    add_task function: add a task (function) to the task queue
    */
    void add_task(function<void()> task) {
        {
            // Lock the mutex to protect the tasks queue
            unique_lock<mutex> lock(queue_mutex);

            // If the pool is stopped, throw an exception
            if (stop)
                throw runtime_error("ThreadPool is stopped");

            // Add the task to the queue
            tasks.push(move(task));
        }

        // Notify one of the threads to execute the task
        condition.notify_one();
    }

    // check if finishing all task
    bool finish_all_tasks() {
        for (bool idle_i : idle_thread) {
            if (idle_i == false) return false; 
        }
        if (tasks.empty()) return true;
        return false;
    }

    /*
    Destructor: wait for all threads to finish executing their tasks
    then join the threads and exit the program
    */
    ~ThreadPool() {
        {
            // Lock the mutex to protect the tasks queue
            unique_lock<mutex> lock(queue_mutex);

            // Set the stop flag to true, meaning the pool is stopped
            stop = true;
        }

        // Notify all threads to stop
        condition.notify_all();

        // Join all threads
        for (thread &t : threads) {
            t.join();
        }
    }
};

int main(int argc, char *argv[]) {
    // get start time
    auto start_time = std::chrono::high_resolution_clock::now();

    // Usage: ./main <numOfString> <NumOfThreads>
    if (argc != 2) {
        cerr << "Usage: " << argv[0] << " <request>.txt\n";
        return 1;
    }

    // int num_tasks = stoi(argv[1]);
    ifstream request_file;
    request_file.open("Test/" + string(argv[1]) + ".txt", ios::in);
    int num_threads, num_tasks;
    request_file >> num_tasks >> num_threads;

    // int num_threads = stoi(argv[2]);

    // Create a vector of strings represent for tasks: "Task 1", ...,
    // "Task n" vector<string> v; for (int i = 1; i <= num_tasks; i++) {
    //     v.push_back("Task " + to_string(i));
    // }

    // string dir_code = "../Submit/probA_AC.cpp";
    //  Create a thread pool with num_threads threads
    ThreadPool pool(num_threads);
    for (int i = 0; i < num_tasks; i++) {
        // sleep until new submit
        int time_arrive;
        request_file >> time_arrive;
        auto current_time = std::chrono::high_resolution_clock::now();
        int time_to_next_submit =
            time_arrive - chrono::duration_cast<chrono::milliseconds>(
                              current_time - start_time)
                              .count();
        if (time_to_next_submit > 0)
            this_thread::sleep_for(chrono::milliseconds(time_to_next_submit));

        // problem of submit
        string problem;
        request_file >> problem;
        problem = "../problem/" + problem;

        INPUT_DIR = "problem/" + problem + "/testcases/";
        OUTPUT_DIR = "problem/" + problem + "/expected_outputs/";

        // code directory of submit
        string dir_code;
        request_file >> dir_code;
        PARTICIPANT_CODE = "Submit/" + dir_code + ".cpp";
        dir_code = "Submit/" + dir_code + ".cpp";

        cout << "Adding task " << problem << " for code " << dir_code
             << " to the pool at time " << time_arrive << endl;

        string exit_code, message;
        // judge(task_id=i, problem_id= ('A', 'B', C',...), dir_code, exit_code,
        // message)
        pool.add_task(bind(judge, i, i, dir_code, INPUT_DIR, OUTPUT_DIR));

        // cout << "================== \n" << exit_code << '\n';
        // pool.add_task(bind(print, ref(v[i])));
    }

    // calculate total time to process all tasks
    while (true) {
        if (pool.finish_all_tasks()) {
            auto end_time = std::chrono::high_resolution_clock::now();
            int total_time = chrono::duration_cast<chrono::milliseconds>(
                              end_time - start_time)
                              .count();
            cout << "the OJ system takes " << total_time << " milliseconds to finish\n";
            break;
        }
        this_thread::sleep_for(chrono::milliseconds(100));
    }
    return 0;
}
