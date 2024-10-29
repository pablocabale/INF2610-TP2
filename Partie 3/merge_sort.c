#include "merge_sort.h"

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <num_processes> <array_size>\n", argv[0]);
        exit(1);
    }

    int num_processes = atoi(argv[1]);
    int array_size = atoi(argv[2]);

    if (num_processes <= 0 || array_size <= 0 || array_size > MAX_ARRAY_SIZE) {
        fprintf(stderr, "Invalid arguments. Ensure 1 <= num_processes <= %d and 1 <= array_size <= %d\n", MAX_ARRAY_SIZE, MAX_ARRAY_SIZE);
        exit(1);
    }

    shared_data = malloc(sizeof(SharedData) + array_size * sizeof(int));
    shared_data->size = array_size;
    shared_data->array = (int *) (shared_data + 1);

    sem_unlink(SEM_NAME);
    mutex = sem_open(SEM_NAME, O_CREAT | O_EXCL, 0666, 0);
    
    /* Populate the array to test the sort */
    srand(time(NULL));
    for (int i = 0; i < array_size; i++) {
        shared_data->array[i] = rand() % MAX_NUM_SIZE;
    }

    printf("Unsorted array: ");
    show_array();

    execute_merge_sort(0, array_size - 1, num_processes);

    printf("Sorted array:\n");
    show_array();

    write_array_into_file();

    sem_close(mutex);
    sem_unlink(SEM_NAME);
    free(shared_data);

    return 0;
}

void execute_merge_sort(int start, int end, int num_processes) {
    int array_size = end - start + 1;
    if (num_processes > array_size) {
        num_processes = array_size;
    }

    pid_t pid;
    int segment_size = array_size / num_processes;
    int rest = array_size % num_processes;

    int pipes[num_processes][2];
    for (int i = 0; i < num_processes; i++) {
        pipe(pipes[i]);
    }

    for (int i = 0; i < num_processes; i++) {
        pid = fork();
        if (pid == 0) {
            int left = start + i * segment_size;
            int right = left + segment_size - 1;
            if (i == num_processes - 1) {
                right += rest;
            }

            // Close all pipes except the one for the current process
            for (int j = 0; j < num_processes; j++) {
                close(pipes[j][0]);
                if (j != i) {
                    close(pipes[j][1]);
                }
            }
            
            printf("Child PID: %d sorting %d to %d\n", getpid(), left, right);
            merge_sort(left, right);
            printf("Child PID: %d finished.\n", getpid());

            int segment_length = right - left + 1;
            write(pipes[i][1], &segment_length, sizeof(int));
            write(pipes[i][1], &shared_data->array[left], segment_length * sizeof(int));

            close(pipes[i][1]);

            sem_post(mutex);

            exit(0);
        }
    }

    for (int i = 0; i < num_processes; i++) {
        close(pipes[i][1]);
    }

    for (int i = 0; i < num_processes; i++) {
        sem_wait(mutex);
    }

    for (int i = 0; i < num_processes; i++) {
        wait(NULL);
    }

    int** sorted_segments = malloc(num_processes * sizeof(int*));
    int* segment_lengths = malloc(num_processes * sizeof(int));

    for (int i = 0; i < num_processes; i++) {
        ssize_t bytes_read = read(pipes[i][0], &segment_lengths[i], sizeof(int));

        sorted_segments[i] = malloc(segment_lengths[i] * sizeof(int));

        bytes_read = 0;
        ssize_t result;
        char* buffer = (char*) sorted_segments[i];
        ssize_t bytes_to_read = segment_lengths[i] * sizeof(int);

        while (bytes_read < bytes_to_read) {
            result = read(pipes[i][0], buffer + bytes_read, bytes_to_read - bytes_read);
            if (result == 0) {
                break;
            }
            bytes_read += result;
        }

        close(pipes[i][0]);
    }

    int* indices_seg = calloc(num_processes, sizeof(int));
    int* temp_array = malloc(array_size * sizeof(int));

    for (int i = 0; i < array_size; i++) {
        int min_val = MAX_NUM_SIZE + 1;
        int min_seg = -1;

        for (int j = 0; j < num_processes; j++) {
            if(indices_seg[j] < segment_lengths[j]) {
                if (sorted_segments[j][indices_seg[j]] < min_val) {
                    min_val = sorted_segments[j][indices_seg[j]];
                    min_seg = j;
                }
            }
        }

        if (min_seg == -1) {
            break;
        }

        temp_array[i] = min_val;
        indices_seg[min_seg]++;
    }

    memcpy(shared_data->array, temp_array, array_size * sizeof(int));

    free(sorted_segments);
    free(segment_lengths);
    free(indices_seg);
    free(temp_array);
}

void merge_sort( int left, int right) {
    if (left < right) {
        int mid = left + (right - left) / 2;
        merge_sort(left, mid);
        merge_sort(mid + 1, right);
        merge(left, mid, right);
    }
}

void merge(int left, int mid, int right) {
    int i, j, k;
    int n1 = mid - left + 1;
    int n2 = right - mid;

    int L[n1], R[n2];

    for (i = 0; i < n1; i++)
        L[i] = shared_data->array[left + i];
    for (j = 0; j < n2; j++)
        R[j] = shared_data->array[mid + 1 + j];

    i = 0;
    j = 0;
    k = left;

    while (i < n1 && j < n2) {
        if (L[i] <= R[j]) {
            shared_data->array[k] = L[i];
            i++;
        } else {
            shared_data->array[k] = R[j];
            j++;
        }
        k++;
    }

    while (i < n1) {
        shared_data->array[k] = L[i];
        i++;
        k++;
    }

    while (j < n2) {
        shared_data->array[k] = R[j];
        j++;
        k++;
    }
}

void show_array(){
    printf("Sorted array: ");
    for (int i = 0; i < shared_data->size; i++) {
        printf("%d ", shared_data->array[i]);
    }
    printf("\n");
}

void write_array_into_file(){
    FILE *file = fopen("sorted_array.txt", "w");
    if (file == NULL) {
        fprintf(stderr, "Error in fopen\n");
        exit(1);
    }

    for (int i = 0; i < shared_data->size; i++) {
        fprintf(file, "%d\n", shared_data->array[i]);
    }

    fclose(file);
}