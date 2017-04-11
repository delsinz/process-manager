/*
 * username: mingyangz
 * student_id: 650242
 * */

#include "process.h"

void simulate(Process* processes, char* algo, int mem_size, int quantum, int process_count);
int mem_empty(int* mem, int mem_size);
Process* remove_terminated(Process* rrq, int* mem, int mem_size);
void free_mem(int* mem, int start, int end, int mem_size);
void fill_mem(int* mem, int start, int end, int mem_size);
Process* get_process_from_disk();
int count_holes(int* mem, int mem_size);
int largest_hole_size(int* mem, int mem_size);
void swap_out(int* mem, int mem_size);
Process* first_fit_insert(int* mem, int mem_size, Process* rrq, Process* target, int quantum);
int get_first_hole(int* mem, int mem_size, int size);
Process* best_fit_insert(int* mem, int mem_size, Process* rrq, Process* target, int quantum);
int get_best_hole(int* mem, int mem_size, int size);
Process* worst_fit_insert(int* mem, int mem_size, Process* rrq, Process* target, int quantum);
int get_worst_hole(int* mem, int mem_size, int size);
Process* schedule(Process* rrq, int quantum);
int count_processes(Process* rrq);
int get_memusage(int* mem, int mem_size);
