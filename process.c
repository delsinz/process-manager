/*
 * username: mingyangz
 * student_id: 650242
 * */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "process.h"

#define BUFFSIZE 32

Process* create_process(int time_created, int process_id, int size, int job_time) {
    Process* process = malloc(sizeof(Process));
    assert(process);

    process->time_created = time_created;
    process->process_id = process_id;
    process->size = size;
    process->job_time = job_time;
    process->remaining_time = job_time;
    process->quantum = -1;

    process->in_mem = 0;
    process->active = 0;
    process->time_in_mem = -1;
    process->time_on_disk = -1;

    process->prev = NULL;
    process->next = NULL;

    return process;
}



// Initial code based off code from Andrew Turpin
// Written Wed 29 Apr 2015 06:32:22 AEST
Process* read_processes(char* filename, int* process_count) {
    int empty = 1; // Indicates the list is initially empty.
    Process* head; // Head of list of processes
    Process* recent; // The process recently added to the list

    FILE* fp;
    fp = fopen(filename, "r");
    char buff[BUFFSIZE];

    while((fgets(buff, BUFFSIZE, fp) != NULL)) {
        int time_created, process_id, size, job_time;
        sscanf(buff, "%d %d %d %d", &time_created, &process_id, &size, &job_time);
        if(empty) {
            // If this is the first process read
            head = create_process(time_created, process_id, size, job_time);
            head->prev = NULL;
            recent = head;
        } else {
            // If there are processes in the list
            recent->next = create_process(time_created, process_id, size, job_time);
            recent->next->prev = recent;
            recent = recent->next;
        }
        empty = 0;
        *process_count += 1; // Just to keep track of how big the list is
    }

    fclose(fp);
    return head;
}



Process* update_disk_on_create(Process* processes, Process* disk, int timer) {
    Process* curr = processes;
    while(curr != NULL) {
        if(curr->time_created == timer) {
//            printf("Process %i created, time %i\n", curr->process_id, timer);
            Process* created_proc = create_process(curr->time_created, curr->process_id, curr->size, curr->job_time);
            created_proc->time_on_disk = timer;
            disk = add_to_disk(disk, created_proc);
        }
        curr = curr->next;
    }
    return disk;
}



Process* add_to_disk(Process* disk, Process* process) {
    if(disk == NULL) { // If disk is empty
        disk = process;
    } else {
        // Traverse to the end of disk
        Process* curr = disk;
        while(curr->next != NULL) {
            curr = curr->next;
        }
        assert(curr->next == NULL);
        // Append process
        curr->next = process;
        process->prev = curr;
    }
    return disk;
}

void free_process(Process* p) {
    p->prev = NULL;
    p->next = NULL;
    free(p->prev);
    free(p->next);
    free(p);
}