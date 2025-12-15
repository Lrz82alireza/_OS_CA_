#include <iostream>
#include <vector>
#include <algorithm>
#include <queue>
#include <iomanip>

using namespace std;

struct Process {
    string pid;
    int arrival_time;
    int burst_time;
    int start_time;
    int finish_time;
    int waiting_time;
    int turnaround_time;
    int remaining_time;
};

vector<Process> get_scenario_1() {
    return {
        {"P1", 0, 8, 0, 0, 0, 0, 8},
        {"P2", 0, 4, 0, 0, 0, 0, 4},
        {"P3", 0, 1, 0, 0, 0, 0, 1},
        {"P4", 0, 3, 0, 0, 0, 0, 3}
    };
}

vector<Process> get_scenario_2() {
    return {
        {"P1", 0, 7, 0, 0, 0, 0, 7},
        {"P2", 2, 4, 0, 0, 0, 0, 4},
        {"P3", 4, 1, 0, 0, 0, 0, 1},
        {"P4", 5, 4, 0, 0, 0, 0, 4}
    };
}

vector<Process> get_scenario_3() {
    return {
        {"P1", 0, 20, 0, 0, 0, 0, 20},
        {"P2", 0, 3, 0, 0, 0, 0, 3},
        {"P3", 0, 3, 0, 0, 0, 0, 3}
    };
}

void print_results(const vector<Process>& procs, string alg_name) {
    double total_wt = 0;
    double total_tat = 0;
    
    cout << "--------------------------------------------------" << endl;
    cout << "Algorithm: " << alg_name << endl;
    cout << "PID\tArrival\tBurst\tFinish\tWait\tTurnaround" << endl;
    
    for (const auto& p : procs) {
        cout << p.pid << "\t" 
             << p.arrival_time << "\t" 
             << p.burst_time << "\t" 
             << p.finish_time << "\t" 
             << p.waiting_time << "\t" 
             << p.turnaround_time << endl;
             
        total_wt += p.waiting_time;
        total_tat += p.turnaround_time;
    }
    
    cout << fixed << setprecision(2);
    cout << "\nAverage Waiting Time: " << total_wt / procs.size() << endl;
    cout << "Average Turnaround Time: " << total_tat / procs.size() << endl;
    cout << "--------------------------------------------------\n" << endl;
}

bool compareArrival(const Process& a, const Process& b) {
    if (a.arrival_time != b.arrival_time)
        return a.arrival_time < b.arrival_time;
    return a.pid < b.pid; 
}

void run_FCFS(vector<Process> procs) {
    sort(procs.begin(), procs.end(), compareArrival);
    
    int current_time = 0;
    
    for (auto& p : procs) {
        if (current_time < p.arrival_time) {
            current_time = p.arrival_time;
        }
        
        p.start_time = current_time;
        p.finish_time = p.start_time + p.burst_time;
        p.turnaround_time = p.finish_time - p.arrival_time;
        p.waiting_time = p.turnaround_time - p.burst_time;
        
        current_time = p.finish_time;
    }
    
    print_results(procs, "FCFS");
}

void run_SJF(vector<Process> procs) {
    int n = procs.size();
    int current_time = 0;
    int completed = 0;
    vector<bool> is_completed(n, false);
    vector<Process> result_order;
    
    while (completed < n) {
        int idx = -1;
        int min_burst = 1e9;
        
        for (int i = 0; i < n; ++i) {
            if (!is_completed[i] && procs[i].arrival_time <= current_time) {
                if (procs[i].burst_time < min_burst) {
                    min_burst = procs[i].burst_time;
                    idx = i;
                }
            }
        }
        
        if (idx != -1) {
            procs[idx].start_time = current_time;
            procs[idx].finish_time = current_time + procs[idx].burst_time;
            procs[idx].turnaround_time = procs[idx].finish_time - procs[idx].arrival_time;
            procs[idx].waiting_time = procs[idx].turnaround_time - procs[idx].burst_time;
            
            is_completed[idx] = true;
            completed++;
            current_time = procs[idx].finish_time;
            result_order.push_back(procs[idx]);
        } else {
            current_time++;
        }
    }
    print_results(result_order, "SJF (Non-Preemptive)");
}

void run_RR(vector<Process> procs, int quantum) {
    // --- Initialization ---
    int n = procs.size();
    int current_time = 0;
    int completed = 0;
    
    // --- Sort by Arrival Time ---
    sort(procs.begin(), procs.end(), compareArrival);
    
    // --- Setup Queue with First Process ---
    queue<int> q;
    vector<bool> in_queue(n, false);
    
    q.push(0);
    in_queue[0] = true;
    current_time = procs[0].arrival_time; 
    
    // --- Main Loop ---
    while (completed < n) {
        
        // --- Handle Idle CPU ---
        if (q.empty()) {
            for (int i = 0; i < n; i++) {
                if (!in_queue[i] && procs[i].remaining_time > 0) {
                    q.push(i);
                    in_queue[i] = true;
                    current_time = max(current_time, procs[i].arrival_time);
                    break; 
                }
            }
        }

        if (q.empty()) {
             current_time++;
             continue;
        }

        // --- Execute Process ---
        int idx = q.front();
        q.pop();
        
        int exec_time = min(quantum, procs[idx].remaining_time);
        
        procs[idx].remaining_time -= exec_time;
        current_time += exec_time;
        
        // --- Add Newly Arrived Processes ---
        for (int i = 0; i < n; i++) {
            if (!in_queue[i] && procs[i].arrival_time <= current_time && procs[i].remaining_time > 0) {
                q.push(i);
                in_queue[i] = true;
            }
        }
        
        // --- Check Completion or Re-queue ---
        if (procs[idx].remaining_time > 0) {
            q.push(idx);
        } else {
            procs[idx].finish_time = current_time;
            procs[idx].turnaround_time = procs[idx].finish_time - procs[idx].arrival_time;
            procs[idx].waiting_time = procs[idx].turnaround_time - procs[idx].burst_time;
            completed++;
        }
    }
    
    // --- Print Results ---
    print_results(procs, "Round Robin (Q=" + to_string(quantum) + ")");
}
int main() {
    int current_scenario_index = 1; 
    int rr_quantum = 1;             

    vector<Process> selected_scenario;

    if (current_scenario_index == 1) {
        selected_scenario = get_scenario_1();
    } else if (current_scenario_index == 2) {
        selected_scenario = get_scenario_2();
    } else {
        selected_scenario = get_scenario_3();
    }

    cout << "Testing Scenario " << current_scenario_index << endl;

    run_FCFS(selected_scenario);
    
    run_SJF(selected_scenario);
    
    run_RR(selected_scenario, rr_quantum);

    return 0;
}