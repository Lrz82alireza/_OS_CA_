#include <iostream>
#include <vector>
#include <pthread.h>

using namespace std;

long long global_counter = 0;
int K_increments = 0;

pthread_mutex_t lock_counter;

void* worker_thread(void* arg) {
    for (int i = 0; i < K_increments; ++i) {
        pthread_mutex_lock(&lock_counter);
        global_counter++;
        pthread_mutex_unlock(&lock_counter);
    }
    return NULL;
}

int main() {
    pthread_mutex_init(&lock_counter, NULL);

    int M_threads;
    
    cout << "Enter number of threads (M): ";
    cin >> M_threads;
    cout << "Enter number of increments per thread (K): ";
    cin >> K_increments;

    vector<pthread_t> threads(M_threads);

    for (int i = 0; i < M_threads; ++i) {
        pthread_create(&threads[i], NULL, worker_thread, NULL);
    }

    for (int i = 0; i < M_threads; ++i) {
        pthread_join(threads[i], NULL);
    }

    long long expected_value = (long long)M_threads * K_increments;
    
    cout << "Final Counter Value: " << global_counter << endl;
    cout << "Expected Value (M*K): " << expected_value << endl;

    pthread_mutex_destroy(&lock_counter);

    return 0;
}