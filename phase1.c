#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#define NUM_RECORDS 2
#define NUM_WORKERS 4
#define UPDATES_PER_WORKER 10000
#define LOGGED_UPDATES 3

typedef struct {
    int record_id;
    int value;
    int update_count;
} Record;

typedef struct {
    int worker_id;
} WorkerData;

Record records[NUM_RECORDS];

double elapsed_seconds(struct timespec start, struct timespec end)
{
    return (end.tv_sec - start.tv_sec) +
           (end.tv_nsec - start.tv_nsec) / 1000000000.0;
}

void *database_worker(void *arg)
{
    WorkerData *data = (WorkerData *)arg;
    int worker_id = data->worker_id;

    printf("Worker %d started with pthread ID %lu\n",
           worker_id, (unsigned long)pthread_self());

    for (int i = 0; i < UPDATES_PER_WORKER; i++) {
        int record_index = i % NUM_RECORDS;

        int old_value = records[record_index].value;
        int new_value = old_value + 1;

        if (i % 25 == 0) {
            usleep(1);
        }

        records[record_index].value = new_value;
        records[record_index].update_count++;

        if (i < LOGGED_UPDATES) {
            printf(
                "Worker %d: Record %d changed from %d to %d\n",
                worker_id,
                records[record_index].record_id,
                old_value,
                new_value
            );
        }
    }

    printf("Worker %d finished\n", worker_id);
    return NULL;
}

int main(void)
{
    pthread_t workers[NUM_WORKERS];
    WorkerData worker_data[NUM_WORKERS];
    struct timespec start_time;
    struct timespec end_time;

    printf("========================================\n");
    printf("Phase 1: Unsynchronized Database Updates\n");
    printf("========================================\n\n");

    for (int i = 0; i < NUM_RECORDS; i++) {
        records[i].record_id = i + 1;
        records[i].value = 0;
        records[i].update_count = 0;
    }

    printf("Workers: %d\n", NUM_WORKERS);
    printf("Updates per worker: %d\n", UPDATES_PER_WORKER);
    printf("Database records: %d\n\n", NUM_RECORDS);

    clock_gettime(CLOCK_MONOTONIC, &start_time);

    for (int i = 0; i < NUM_WORKERS; i++) {
        worker_data[i].worker_id = i + 1;

        int result = pthread_create(
            &workers[i],
            NULL,
            database_worker,
            &worker_data[i]
        );

        if (result != 0) {
            fprintf(stderr, "Failed to create worker %d\n", i + 1);
            return EXIT_FAILURE;
        }
    }

    for (int i = 0; i < NUM_WORKERS; i++) {
        int result = pthread_join(workers[i], NULL);

        if (result != 0) {
            fprintf(stderr, "Failed to join worker %d\n", i + 1);
            return EXIT_FAILURE;
        }
    }

    clock_gettime(CLOCK_MONOTONIC, &end_time);

    int expected_per_record =
        NUM_WORKERS * (UPDATES_PER_WORKER / NUM_RECORDS);

    int expected_total =
        NUM_WORKERS * UPDATES_PER_WORKER;

    int actual_total = 0;

    printf("\n========================================\n");
    printf("Phase 1 Results\n");
    printf("========================================\n");

    for (int i = 0; i < NUM_RECORDS; i++) {
        actual_total += records[i].value;

        printf("\nRecord %d\n", records[i].record_id);
        printf("  Expected value:      %d\n", expected_per_record);
        printf("  Actual value:        %d\n", records[i].value);
        printf("  Recorded updates:    %d\n", records[i].update_count);

        if (records[i].value != expected_per_record) {
            printf("  Result: RACE CONDITION DETECTED\n");
        } else {
            printf("  Result: Correct in this run\n");
        }
    }

    printf("\nExpected total updates: %d\n", expected_total);
    printf("Actual total value:     %d\n", actual_total);
    printf("Lost updates:           %d\n", expected_total - actual_total);
    printf("Execution time:         %.6f seconds\n",
           elapsed_seconds(start_time, end_time));

    if (actual_total != expected_total) {
        printf("\nRace condition demonstrated successfully.\n");
    } else {
        printf("\nNo incorrect result appeared in this run.\n");
        printf("Run the program again because thread scheduling varies.\n");
    }

    return EXIT_SUCCESS;
}
