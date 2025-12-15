#include <iostream>
#include <vector>
#include <pthread.h>
#include <ctime>
#include <cstdlib>
#include <cmath>

using namespace std;

#define DEFAULT_ARRAY_SIZE 100000
#define MAX_RAND_VAL 10 

struct ThreadData {
    const vector<int>* array;
    long long partial_sum;    
    size_t start_index;        
    size_t end_index;         
};


vector<int> generate_array(size_t N) {
    vector<int> arr(N);
    srand(time(NULL));
    for (size_t i = 0; i < N; ++i) {
        arr[i] = rand() % MAX_RAND_VAL; 
    }
    return arr;
}


void* thread_sum_worker(void* arg) {
    ThreadData* data = (ThreadData*)arg;
    long long sum = 0;

    for (size_t i = data->start_index; i < data->end_index; ++i) {
        sum += (*data->array)[i];
    }
    
    data->partial_sum = sum;
    return NULL;
}

long long single_thread_sum(const vector<int>& arr, double& duration_ms) {
    long long total_sum = 0;
    
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    for (int num : arr) {
        total_sum += num;
    }

    clock_gettime(CLOCK_MONOTONIC, &end); 

    duration_ms = (end.tv_sec - start.tv_sec) * 1000.0;
    duration_ms += (end.tv_nsec - start.tv_nsec) / 1000000.0;
    
    return total_sum;
}


long long multi_thread_sum(const vector<int>& arr, int T, double& duration_ms) {
    long long total_sum = 0;
    size_t N = arr.size();
    pthread_t threads[T];
    ThreadData data[T];

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start); 

    size_t chunk_size = N / T;
    size_t remainder = N % T;
    size_t current_index = 0;

    for (int i = 0; i < T; ++i) {
        size_t segment_size = chunk_size + (i < remainder ? 1 : 0);
        
        data[i].array = &arr;
        data[i].start_index = current_index;
        data[i].end_index = current_index + segment_size;
        data[i].partial_sum = 0; 

        pthread_create(&threads[i], NULL, thread_sum_worker, &data[i]);
        
        current_index += segment_size;
    }

    for (int i = 0; i < T; ++i) {
        pthread_join(threads[i], NULL);
        total_sum += data[i].partial_sum;
    }

    clock_gettime(CLOCK_MONOTONIC, &end); 

    duration_ms = (end.tv_sec - start.tv_sec) * 1000.0;
    duration_ms += (end.tv_nsec - start.tv_nsec) / 1000000.0;

    return total_sum;
}

int main() {
    size_t N = DEFAULT_ARRAY_SIZE; 
    vector<int> large_array = generate_array(N);

    int T;
    cout << "Array size N = " << N << endl;
    cout << "Enter the number of threads (T) for multi-threaded version: ";
    cin >> T;
    
    if (T <= 0) {
        cout << "Invalid number of threads." << endl;
        return 1;
    }

    long long single_sum, multi_sum;
    double single_duration, multi_duration;

    cout << "\n--- Single-Threaded Summation ---" << endl;
    single_sum = single_thread_sum(large_array, single_duration);
    cout << "Single-Thread Sum: " << single_sum << endl;
    cout << "Execution Time: " << single_duration << " ms" << endl;

    cout << "\n--- Multi-Threaded Summation (T=" << T << ") ---" << endl;
    multi_sum = multi_thread_sum(large_array, T, multi_duration);
    cout << "Multi-Thread Sum: " << multi_sum << endl;
    cout << "Execution Time: " << multi_duration << " ms" << endl;

    // Verification
    if (single_sum == multi_sum) {
        cout << "\nVerification: Sums Match (Correctness Check Passed)." << endl;
    } else {
        cout << "\nVerification: *** ERROR: Sums DO NOT Match. ***" << endl;
    }
    
    // Comparison
    cout << "\n--- Comparison ---" << endl;
    cout << "Speedup Factor (Single/Multi): " << single_duration / multi_duration << endl;

    return 0;
}