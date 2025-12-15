#include <iostream>
#include <pthread.h>
#include <unistd.h>
#include <cstdlib>
#include <ctime>

using namespace std;

struct ThreadArgs {
    int thread_id;
    int total_threads;
};

void* thread_function(void* arg) {
    ThreadArgs* args = (ThreadArgs*)arg;

    srand(time(NULL));
    int sleep_time = (rand() % 4);

    sleep(sleep_time);

    cout << "I'm thread " << args->thread_id << " of " << args->total_threads << endl;

    delete args;

    return NULL;
}

int main() {
    int N;

    cout << "Enter the number of threads (N): ";
    cin >> N;

    pthread_t threads[N];

    for (int i = 0; i < N; ++i) {
        ThreadArgs* args = new ThreadArgs;
        args->thread_id = i + 1; 
        args->total_threads = N;

        pthread_create(&threads[i], NULL, thread_function, args);
    }

    for (int i = 0; i < N; ++i) {
        pthread_join(threads[i], NULL);
    }

    cout << "Main thread finished waiting for all " << N << " threads." << endl;

    return 0;
}