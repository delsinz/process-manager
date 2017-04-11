/*
 * username: mingyangz
 * student_id: 650242
 * */

typedef struct process_s {
    // Initial data
    int time_created;
    int process_id;
    int size;
    int job_time;

    // Once process inside memory, the following data is computed and tracked.
    int start;
    int end; // end = start - size + 1
    int in_mem; // Whether process in memory. 0=n, 1=y
    int active; // Whether process is being run by CPU. 0=n, 1=y
    int time_in_mem; // Time when process is put into mem. -1 if not in mem.
    int time_on_disk; // The timestamp when last time the process is swapped onto disk.
                      // Equals time_created on creation. -1 if not created yet.
    int remaining_time; // Equals job_time on creation.
    int quantum; // Set to quantum when swap into memory, -1 if not in mem;

    struct process_s* prev;
    struct process_s* next;
} Process;

Process* create_process(int time_created, int process_id, int size, int job_time);
Process* read_processes(char* filename, int* process_count);
Process* update_disk_on_create(Process* processes, Process* disk, int timer);
Process* add_to_disk(Process* disk, Process* process);
void free_process(Process* p);