#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/queue.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

typedef uint32_t u32;
typedef int32_t i32;

struct process {
    u32 pid;
    u32 arrival_time;
    u32 burst_time;
    TAILQ_ENTRY(process) pointers;

    /* Additional fields here */
    u32 remaining_time;
    bool has_started;
    /* End of "Additional fields here" */
};

TAILQ_HEAD(process_queue, process);

u32 parse_next_int(const char **data, const char *data_end) {
    u32 value = 0;
    bool is_started = false;
    while (*data != data_end) {
        char c = **data;
        if (c < '0' || c > '9') {
            if (is_started) {
                return value;
            }
        } else {
            if (!is_started) {
                value = (c - '0');
                is_started = true;
            } else {
                value = value * 10 + (c - '0');
            }
        }
        ++(*data);
    }
    printf("Reached end of file while looking for another integer\n");
    exit(EINVAL);
}

u32 parse_int_from_c_str(const char *data) {
    char c;
    u32 index = 0;
    u32 value = 0;
    bool is_started = false;
    while ((c = data[index++])) {
        if (c < '0' || c > '9') {
            exit(EINVAL);
        }
        if (!is_started) {
            value = (c - '0');
            is_started = true;
        } else {
            value = value * 10 + (c - '0');
        }
    }
    return value;
}

void initialize_processes(const char *path, struct process **process_data, u32 *process_count) {
    int fd = open(path, O_RDONLY);
    if (fd == -1) {
        perror("open");
        exit(errno);
    }

    struct stat file_stat;
    if (fstat(fd, &file_stat) == -1) {
        perror("stat");
        exit(errno);
    }

    u32 file_size = file_stat.st_size;
    const char *file_start = mmap(NULL, file_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (file_start == MAP_FAILED) {
        perror("mmap");
        exit(errno);
    }

    const char *file_end = file_start + file_size;
    const char *cursor = file_start;

    *process_count = parse_next_int(&cursor, file_end);

    *process_data = calloc(*process_count, sizeof(struct process));
    if (*process_data == NULL) {
        perror("calloc");
        exit(errno);
    }

    for (u32 i = 0; i < *process_count; ++i) {
        (*process_data)[i].pid = parse_next_int(&cursor, file_end);
        (*process_data)[i].arrival_time = parse_next_int(&cursor, file_end);
        (*process_data)[i].burst_time = parse_next_int(&cursor, file_end);
    }

    munmap((void *)file_start, file_size);
    close(fd);
}

bool all_processes_completed(struct process *processes, u32 count) {
    for (u32 i = 0; i < count; i++) {
        if (processes[i].remaining_time != 0) {
            return false;
        }
    }
    return true;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        return EINVAL;
    }

    struct process *processes;
    u32 process_count;
    initialize_processes(argv[1], &processes, &process_count);

    u32 time_quantum = parse_int_from_c_str(argv[2]);

    struct process_queue process_queue;
    TAILQ_INIT(&process_queue);

    u32 total_waiting_time = 0;
    u32 total_response_time = 0;
    
    /* Your code here */

    /* Initialization of process data */
    struct process *current_process;
    u32 current_time = processes[0].arrival_time;
    for (u32 i = 0; i < process_count; i++) {
        current_process = &processes[i];
        current_process->remaining_time = current_process->burst_time;
        current_process->has_started = false;
        if (current_process->arrival_time < current_time) {
            current_time = current_process->arrival_time;
        }
    }

    u32 quantum_counter = 1;
    u32 total_runtime = current_time;
    bool all_done = (time_quantum == 0);

    while (!all_done) {
        for (u32 i = 0; i < process_count; i++) {
            struct process *new_process = &processes[i];
            if (new_process->arrival_time == current_time) {
                TAILQ_INSERT_TAIL(&process_queue, new_process, pointers);
            }
        }

        if (quantum_counter == time_quantum + 1 && current_process->remaining_time > 0) {
            TAILQ_INSERT_TAIL(&process_queue, current_process, pointers);
            quantum_counter = 1;
        }

        if (quantum_counter == 1) {
            if (TAILQ_EMPTY(&process_queue)) {
                return -1;
            }
            current_process = TAILQ_FIRST(&process_queue);
            TAILQ_REMOVE(&process_queue, current_process, pointers);
        }

        if (!current_process->has_started) {
            total_response_time += total_runtime - current_process->arrival_time;
            current_process->has_started = true;
        }

        if (quantum_counter < time_quantum + 1) {
            if (current_process->remaining_time > 0) {
                current_process->remaining_time--;
                total_runtime++;
            }

            if (current_process->remaining_time == 0) {
                total_waiting_time += total_runtime - current_process->arrival_time - current_process->burst_time;
                quantum_counter = 0;
            }
        }

        quantum_counter++;
        current_time++;
        all_done = all_processes_completed(processes, process_count);
    }
    
    
    /* End of "Your code here" */


    printf("Average waiting time: %.2f\n", (float)total_waiting_time / (float)process_count);
    printf("Average response time: %.2f\n", (float)total_response_time / (float)process_count);

    free(processes);
    return 0;
}
