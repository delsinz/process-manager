/*
 * username: mingyangz
 * student_id: 650242
 * */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include "swap.h"

#define FF "first"
#define BF "best"
#define WF "worst"

extern char* optarg;

int timer;
int completed_processes = 0;
Process* disk = NULL; // A list of processes representing the disk
Process* rrq = NULL;
//int quantum;

int main(int argc, char** argv) {
    char input;
    char* filename;
    char* algorithm;
    int mem_size;
    int quantum;

    // Read input form command line
    while((input = getopt(argc, argv, "f:a:m:q:")) != EOF) {
        switch(input) {
            case 'f':
                filename = optarg;
                break;
            case 'a':
                if(strcmp(optarg, FF) == 0) {
                    algorithm = optarg;
                } else if(strcmp(optarg, BF) == 0) {
                    algorithm = optarg;
                } else if(strcmp(optarg, WF) == 0) {
                    algorithm = optarg;
                } else {
                    fprintf(stderr, "Invalid algorithm: %s\n", optarg);
                    exit(EXIT_FAILURE);
                }
                break;
            case 'm':
                mem_size = atoi(optarg);
                break;
            case 'q':
                quantum = atoi(optarg);
                break;
            default:
                return -1;
        }
    }

    // Read all processes into a list
    int process_count = 0; // Num of total processes in this simulation. Need this to check if simulation finished.
    Process* processes = read_processes(filename, &process_count);
//    Process* curr = processes;
//    while(curr != NULL) {
//        printf("%i, %i, %i, %i\n", curr->time_created, curr->process_id, curr->size, curr->job_time);
//        curr = curr->next;
//    }
//    printf("%i processes read\n", process_count);
    simulate(processes, algorithm, mem_size, quantum, process_count);
    return 0;
}

// The main simulation function.
void simulate(Process* processes, char* algo, int mem_size, int quantum, int process_count) {

    int memory[mem_size]; // Array of int representing memory
    memset(memory, 0, sizeof(memory));

    for(timer = 0; completed_processes < process_count /*&& timer < 30*/; timer ++) {
        // Every unit time, first update the processes on disk
        disk = update_disk_on_create(processes, disk, timer);
        // Manage memory
        if(mem_empty(memory, mem_size) || (rrq!=NULL && rrq->quantum <= 0) || (rrq!=NULL && rrq->remaining_time <= 0)) {
            // 1. Remove process that has been terminated from memory
            rrq = remove_terminated(rrq, memory, mem_size);
            // 2. Swap in a process from disk to memory if disk not empty
            if(disk != NULL) {
                // 1) Get longest waiting process on disk
                Process* target = get_process_from_disk();
                // 2) Check if there is a hole large enough for this process, if not, swap out processes.
                while(largest_hole_size(memory, mem_size) < (target->size)) {
                    swap_out(memory, mem_size);
                }
                // 3) Insert process to memory
                if(strcmp(algo, FF) == 0) { // First fit insertion
                    rrq = first_fit_insert(memory, mem_size, rrq, target, quantum);
                } else if (strcmp(algo, BF) == 0) { // Best fit insertion
                    rrq = best_fit_insert(memory, mem_size, rrq, target, quantum);
                } else { // Worst fit insertion
                    rrq = worst_fit_insert(memory, mem_size, rrq, target, quantum);
                }
                printf("time %d, %d loaded, numprocesses=%d, numholes=%d, memusage=%d%%\n",
                       timer, target->process_id, count_processes(rrq), count_holes(memory, mem_size), get_memusage(memory, mem_size));
            }
            // 3. Schedule round robin queue
            rrq = schedule(rrq, quantum);
        }
        // 4. Update all data by one unit time. i.e. running process
        if(rrq != NULL) {
            rrq->quantum = rrq->quantum - 1;
            rrq->remaining_time = rrq->remaining_time - 1;
        }
    }
    printf("time %d, simulation finished.\n", timer - 1); // TODO: why -1 gives correct result?
    return;
}

// Count the num of processes in round robin queue
int count_processes(Process* rrq) {
    int count = 0;
    Process* curr = rrq;
    while(curr != NULL) {
        count++;
        curr = curr->next;
    }
    return count;
}

// Get memory usage percentage
int get_memusage(int* mem, int mem_size) {
    int count = 0;
    for(int i = 0; i < mem_size; i++) {
        if(mem[i] == 1) {
            count++;
        }
    }
    if((count*100) % mem_size == 0) {
        return count*100/mem_size;
    } else {
        return count*100/mem_size + 1; // Round up
    }
}

// Schedule processes in round robin queue
Process* schedule(Process* rrq, int quantum) {
    // All terminated processes should already be removed at this stage
    if(rrq != NULL) {
        Process* head = rrq;
        // If head of round robin queue expired,
        if(head->quantum <= 0) {
            if(rrq->next == NULL) {
                // If this is the only proc in queue, simply refresh quantum
                head->quantum = quantum;
            } else {
                // Move to end of queue, and refresh quantum
                rrq = rrq->next;
                rrq->prev = NULL;
                head->next = NULL;
                Process* curr = rrq;
                while(curr->next != NULL) {
                    curr = curr->next;
                }
                curr->next = head;
                head->prev = curr;
                head->quantum = quantum;
            }
        }
    }
    return rrq;
}

// Insert proc in mem with worst fit
Process* worst_fit_insert(int* mem, int mem_size, Process* rrq, Process* target, int quantum){
    // Get start/end addresses for target in mem
    int start = get_worst_hole(mem, mem_size, target->size);
    int end = start - target->size + 1;
    // Fill memory space and update data related to target
    fill_mem(mem, start, end, mem_size);
    target->start = start;
    target->end = end;
    target->in_mem = 1;
    target->active = 0;
    target->time_in_mem = timer;
    target->time_on_disk = -1;
    target->quantum = quantum;
    // Add process to round robin queue
    if(rrq == NULL) {
        target->prev = NULL;
        rrq = target;
    } else {
        Process* curr = rrq;
        while(curr->next != NULL) {
            curr = curr->next;
        }
        curr->next = target;
        target->prev = curr;
    }
    target->next = NULL;
    return rrq;
}

// Return the start location of worst fit hole in mem
int get_worst_hole(int* mem, int mem_size, int size) {
    assert(largest_hole_size(mem, mem_size) >= size);
    int hole_count = count_holes(mem, mem_size);
    int hole_size[hole_count];
    int hole_address[hole_count];
    int hole_index = 0;
    int prev = -1;
    int start, end;
    for(int i = mem_size - 1; i >= 0; i--) {
        int curr = mem[i];
        if(curr == 0 && curr != prev) {
            start = i;
        }
        if(curr == 1 && prev == 0) {
            end = i;
            hole_size[hole_index] = start - end;
            hole_address[hole_index] = start;
            hole_index++;
            //printf("start: %i, end: %i, size: %i\n", start, end, start-end);
        }
        if(curr == 0 && i == 0) {
            end = i - 1;
            hole_size[hole_index] = start - end;
            hole_address[hole_index] = start;
            hole_index++;
            //printf("start: %i, end: %i, size: %i\n", start, end, start-end);
        }
        prev = curr;
    }
    int worst, j, worst_size = 0;
    for(j = 0; j < hole_index; j++) {
        if(hole_size[j] >= size) {
            if(hole_size[j] > worst_size) {
                worst_size = hole_size[j];
                worst = hole_address[j];
            }
        }
    }
    return worst;
}

// Insert proc in mem with best fit
Process* best_fit_insert(int* mem, int mem_size, Process* rrq, Process* target, int quantum) {
    // Get start/end addresses for target in mem
    int start = get_best_hole(mem, mem_size, target->size);
    int end = start - target->size + 1;
    // Fill memory space and update data related to target
    fill_mem(mem, start, end, mem_size);
    target->start = start;
    target->end = end;
    target->in_mem = 1;
    target->active = 0;
    target->time_in_mem = timer;
    target->time_on_disk = -1;
    target->quantum = quantum;
    // Add process to round robin queue
    if(rrq == NULL) {
        target->prev = NULL;
        rrq = target;
    } else {
        Process* curr = rrq;
        while(curr->next != NULL) {
            curr = curr->next;
        }
        curr->next = target;
        target->prev = curr;
    }
    target->next = NULL;
    return rrq;
}

// Return the start location of best fit hole in mem
int get_best_hole(int* mem, int mem_size, int size){
    assert(largest_hole_size(mem, mem_size) >= size);
    int hole_count = count_holes(mem, mem_size);
    int hole_size[hole_count];
    int hole_address[hole_count];
    int hole_index = 0;
    int prev = -1;
    int start, end;
    for(int i = mem_size - 1; i >= 0; i--) {
        int curr = mem[i];
        if(curr == 0 && curr != prev) {
            start = i;
        }
        if(curr == 1 && prev == 0) {
            end = i;
            hole_size[hole_index] = start - end;
            hole_address[hole_index] = start;
            hole_index++;
            //printf("start: %i, end: %i, size: %i\n", start, end, start-end);
        }
        if(curr == 0 && i == 0) {
            end = i - 1;
            hole_size[hole_index] = start - end;
            hole_address[hole_index] = start;
            hole_index++;
            //printf("start: %i, end: %i, size: %i\n", start, end, start-end);
        }
        prev = curr;
    }
    int best, j, best_size = 9999; // Simply make sure initial best size is large enough.
    for(j = 0; j < hole_index; j++) {
        if(hole_size[j] >= size) {
            if(hole_size[j] < best_size) {
                best_size = hole_size[j];
                best = hole_address[j];
            }
        }
    }
    return best;
}

// Insert proc in mem with first fit
Process* first_fit_insert(int* mem, int mem_size, Process* rrq, Process* target, int quantum) {
    // Get start/end addresses for target in mem
    int start = get_first_hole(mem, mem_size, target->size);
    int end = start - target->size + 1;
    // Fill memory space and update data related to target
    fill_mem(mem, start, end, mem_size);
    target->start = start;
    target->end = end;
    target->in_mem = 1;
    target->active = 0;
    target->time_in_mem = timer;
    target->time_on_disk = -1;
    target->quantum = quantum; // A process is immediately given a quantum once it's in round robin queue
                               // But only consumes quantum when it's actively running.
    // Add process to round robin queue
    if(rrq == NULL) {
        target->prev = NULL;
        rrq = target;
    } else {
        Process* curr = rrq;
        while(curr->next != NULL) {
            curr = curr->next;
        }
        curr->next = target;
        target->prev = curr;
    }
    target->next = NULL;
    return rrq;
}

// Return the start location in mem of first hole large enough for the proc
int get_first_hole(int* mem, int mem_size, int size) {
    assert(largest_hole_size(mem, mem_size) >= size);
    int hole_count = count_holes(mem, mem_size);
    int hole_size[hole_count];
    int hole_address[hole_count];
    int hole_index = 0;
    int prev = -1;
    int start, end;
    for(int i = mem_size - 1; i >= 0; i--) {
        int curr = mem[i];
        if(curr == 0 && curr != prev) {
            start = i;
        }
        if(curr == 1 && prev == 0) {
            end = i;
            hole_size[hole_index] = start - end;
            hole_address[hole_index] = start;
            hole_index++;
            //printf("start: %i, end: %i, size: %i\n", start, end, start-end);
        }
        if(curr == 0 && i == 0) {
            end = i - 1;
            hole_size[hole_index] = start - end;
            hole_address[hole_index] = start;
            hole_index++;
            //printf("start: %i, end: %i, size: %i\n", start, end, start-end);
        }
        prev = curr;
    }
    int j;
    for(j = 0; j < hole_index; j++) {
        if(hole_size[j] >= size) {
            return hole_address[j];
        }
    }
    printf("Error: No hole large enough for first fit insertion.\n");
    exit(EXIT_FAILURE);
}

// Get the largest hole size in current mem
int largest_hole_size(int* mem, int mem_size) {
    int hole_count = count_holes(mem, mem_size);
    int hole_size[hole_count];
    int hole_address[hole_count];
    int hole_index = 0;
    int prev = -1;
    int start, end;
    int max = 0;
    for(int i = mem_size - 1; i >= 0; i--) {
        int curr = mem[i];
        if(curr == 0 && curr != prev) {
            start = i;
        }
        if(curr == 1 && prev == 0) {
            end = i;
            hole_size[hole_index] = start - end;
            hole_address[hole_index] = start;
            hole_index++;
            //printf("start: %i, end: %i, size: %i\n", start, end, start-end);
        }
        if(curr == 0 && i == 0) {
            end = i - 1;
            hole_size[hole_index] = start - end;
            hole_address[hole_index] = start;
            hole_index++;
            //printf("start: %i, end: %i, size: %i\n", start, end, start-end);
        }
        prev = curr;
    }
    for(int j = 0; j < hole_index; j++) {
        if(hole_size[j] >= max) {
            max = hole_size[j];
        }
    }
    return max;
}

// Swap a proc out of mem to make room for new proc from disk
void swap_out(int* mem, int mem_size) {
    assert(mem_empty(mem, mem_size) == 0);
    int longest_time = 0;
    Process* longest_proc = NULL;
    Process* curr = rrq;
    // Get the process being in mem the longest
    while(curr != NULL) {
        assert(curr->time_in_mem >= 0);
        if(timer - curr->time_in_mem >= longest_time) {
            longest_time = timer - curr->time_in_mem;
            longest_proc = curr;
        }
        curr = curr->next;
    }
    assert(longest_proc != NULL); // Make sure we do get the longest_proc
    if(longest_proc->prev == NULL) { // If this is the head of rrq
        rrq = longest_proc->next;
        longest_proc->next = NULL;
    } else if(longest_proc->next == NULL) { // If this is the tail of rrq
        longest_proc->prev->next = NULL;
        longest_proc->prev = NULL;
    } else { // If in the middle of rrq
        longest_proc->prev->next = longest_proc->next;
        longest_proc->next->prev = longest_proc->prev;
        longest_proc->prev = NULL;
        longest_proc->next = NULL;
    }
    // Free the memory space and update data related to this process
    assert(longest_proc->start >= longest_proc->end);
    free_mem(mem, longest_proc->start, longest_proc->end, mem_size);
    longest_proc->start = -1;
    longest_proc->end = -1;
    longest_proc->in_mem = 0;
    longest_proc->active = 0;
    longest_proc->time_in_mem = -1;
    longest_proc->time_on_disk = timer;
    longest_proc->quantum = -1;
    // Append this process to disk
    if(disk == NULL) {
        longest_proc->prev = NULL;
        longest_proc->next = NULL;
        disk = longest_proc;
    } else{
        Process* disk_head = disk;
        while(disk_head->next != NULL) {
            disk_head = disk_head->next;
        }
        disk_head->next = longest_proc;
        longest_proc->prev = disk_head;
        longest_proc->next = NULL;
    }
    return;
}

// Count how many holes in memory
int count_holes(int* mem, int mem_size) {
    int count = 0, prev = -1;
    for(int i = 0; i < mem_size; i++) {
        int curr = mem[i];
        if(curr == 0 && curr != prev) {
            count++;
        }
        prev = curr;
    }
    return count;
}

// Get the next proc to swap into mem
// If this function is called, then disk is not NULL.
Process* get_process_from_disk() {
    Process* target = disk;
    Process* curr = disk;
    // Find the target proc
    while(curr != NULL) {
        if(curr->time_on_disk > target->time_on_disk) {
            break;
        } else if (curr->time_on_disk < target->time_on_disk) {
            printf("Something wrong with time on disk here.\n");
            exit(EXIT_FAILURE);
        } else {
            if(curr->process_id < target->process_id) {
                target = curr;
            }
        }
        curr = curr->next;
    }
    // Extract target proc
    if(target->prev == NULL) { // Target at head of disk
        disk = disk->next;
        if(disk != NULL) { // If target is not the only proc on disk
            disk->prev = NULL;
        }
        target->next = NULL;
    } else if(target->next == NULL) { // Target at end of disk
        if(target->prev == NULL) { // If target is the only proc on disk
            disk = NULL;
        } else {
            target->prev->next = NULL;
        }
        target->prev = NULL;
    } else { // Target at the middle of disk, and not the only proc on disk
        target->prev->next = target->next;
        target->next->prev = target->prev;
        target->prev = NULL;
        target->next = NULL;
    }
    target->time_on_disk = -1;
    return target;
}

// Remove terminated proc from mem
Process* remove_terminated(Process* rrq, int* mem, int mem_size) {
    if(rrq == NULL) { // If round robin queue empty
        assert(mem_empty(mem, mem_size));
        return rrq;
    }
    Process* head = rrq;
    //assert(head->active); // Can we only check head of round robin queue?
    if(head->remaining_time <= 0) {
        completed_processes += 1;
//        printf("completed: %d\n", completed_processes);
        rrq = head->next;
        if(rrq != NULL){
            rrq->prev = NULL;
        }
        free_mem(mem, head->start, head->end, mem_size);
        free_process(head);
    }
    return rrq;
}

// Free spaces virtually represented by the int array
void free_mem(int* mem, int start, int end, int mem_size) {
    assert(start >= 0 && end >= 0);
    assert(start < mem_size && end <= start);
    for(int i = start; i >= end; i--) {
        mem[i] = 0;
    }
}

// Fill spaces virtually represented by the int array
void fill_mem(int* mem, int start, int end, int mem_size) {
    assert(start >= 0 && end >= 0);
    assert(start < mem_size && end <= start);
    for(int i = start; i >= end; i--) {
        mem[i] = 1;
    }
}

// Check if current mem is empty
int mem_empty(int* mem, int mem_size) {
    for(int i = 0; i < mem_size; i++) {
        if(mem[i] == 1) {
            return 0;
        }
    }
    return 1;
}