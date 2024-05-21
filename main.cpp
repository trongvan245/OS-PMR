#include <iostream> // for std::cout, std::cerr
#include <vector> // for std::vector
#include <thread> // for multithreading
#include <chrono> // for time measurement
#include <string> // for std::string
#include <mutex>  // for std::mutex
#include <queue> // for std::queue
#include <functional> // for std::ref
#include <condition_variable> // for condition_variable

using namespace std;

// The print function represents a task that takes a string reference as input and prints it.
void print(string &s) {
    cout << "=====" << s << " is running on thread " << this_thread::get_id() << endl; 
    // Simulate some work 
    this_thread::sleep_for(chrono::milliseconds(100));
}

class ThreadPool {
    private:
        // A list of threads in the pool
        vector<thread> threads;

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
            // For each thread, create a lambda function that will keep the
            // thread running
            for (int i = 0; i < num_threads; i++) {
                // Create a thread, add to the list of threads
                threads.emplace_back([this] {
                    while (true) {
                        // Create a task variable, which has no task initially
                        function<void()> task;

                        {
                            // Lock the mutex in this scope, to protect the tasks queue
                            unique_lock<mutex> lock(this->queue_mutex);

                            // Wait for a task to be added to the queue, or the pool to be stopped
                            this->condition.wait(lock, [this] { return this->stop || !this->tasks.empty(); });

                            // If the pool is stopped and the queue is empty, then return
                            if (this->stop && this->tasks.empty()) return;

                            // Get the nearest task from the queue
                            task = move(this->tasks.front());

                            // Remove the task from the queue
                            this->tasks.pop();
                        }

                        // Execute the task
                        task();
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
                if (stop) throw runtime_error("ThreadPool is stopped");

                // Add the task to the queue
                tasks.push(move(task));
            }

            // Notify one of the threads to execute the task
            condition.notify_one();
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

int main(int argc, char* argv[]) {
    // Usage: ./main <numOfString> <NumOfThreads>
    if (argc != 3) {
        cerr << "Usage: " << argv[0] << " <numOfString> <NumOfThreads>\n";
        return 1;
    }

    int num_tasks = stoi(argv[1]);
    int num_threads = stoi(argv[2]);

    // Create a vector of strings represent for tasks: "Task 1", ..., "Task n"
    vector<string> v;
    for (int i = 1; i <= num_tasks; i++) {
        v.push_back("Task " + to_string(i));
    }

    // Create a thread pool with num_threads threads
    ThreadPool pool(num_threads);
    for (int i = 0; i < num_tasks; i++) {
        cout << "Adding task " << v[i] << " to the pool" << endl;
        pool.add_task(bind(print, ref(v[i])));

        // Wait for a while before adding the next task
        this_thread::sleep_for(chrono::milliseconds(50));
    }

    // Give some time for all tasks to finish before the main function exits.
    this_thread::sleep_for(chrono::seconds(1));

    return 0;
}